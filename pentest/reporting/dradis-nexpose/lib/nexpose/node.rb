module Nexpose
  # This class represents each of the /NexposeReport[@version='1.0']/nodes/node
  # elements in the Nexpose Full XML document.
  #
  # It provides a convenient way to access the information scattered all over
  # the XML in attributes and nested tags.
  #
  # Instead of providing separate methods for each supported property we rely
  # on Ruby's #method_missing to do most of the work.
  class Node
    # Accepts an XML node from Nokogiri::XML.
    def initialize(xml_node)
      @xml = xml_node
    end

    # List of supported tags. They can be attributes, simple descendans or
    # collections (e.g. <references/>, <tags/>)
    def supported_tags
      [
        # attributes
        :address, :status, :device_id, :hardware_address,

        # simple tags

        # multiple tags
        :names, :fingerprints,

        # compounded tags
        :software, :tests, :endpoints
      ]
    end

    # Convert each ./test/test entry into a simple hash
    def tests(*args)
      @xml.xpath('./tests/test').collect do |xml_test|
        {
          :id => xml_test.attributes['id'],
          :status => xml_test.attributes['status'],
          # in some cases Nexpose doesn't include text content (e.g. udp-ipid-zero)
          :content => if (xml_para = xml_test.xpath('./Paragraph').first)
                        xml_para.text.split("\n").collect(&:strip).reject{|line| line.empty?}.join("\n")
                      else
                        ""
                      end
        }
      end
    end

    # Each of the endpoints associated with this node. Returns an array of
    # Nexpose::Endpoint objects
    def endpoints
      @xml.xpath('./endpoints/endpoint').collect { |xml_endpoint| Endpoint.new(xml_endpoint) }
    end


    # This allows external callers (and specs) to check for implemented
    # properties
    def respond_to?(method, include_private=false)
      return true if supported_tags.include?(method.to_sym)
      super
    end

    # This method is invoked by Ruby when a method that is not defined in this
    # instance is called.
    #
    # In our case we inspect the @method@ parameter and try to find the
    # attribute, simple descendent or collection that it maps to in the XML
    # tree.
    def method_missing(method, *args)

      # We could remove this check and return nil for any non-recognized tag.
      # The problem would be that it would make tricky to debug problems with
      # typos. For instance: <>.potr would return nil instead of raising an
      # exception
      unless supported_tags.include?(method)
        super
        return
      end

      # First we try the attributes. In Ruby we use snake_case, but in XML
      # hyphenated-case is used for some attributes
      translations_table = {
        :device_id => 'device-id',
        :hardware_address => 'hardware-address'
      }

      method_name = translations_table.fetch(method, method.to_s)
      return @xml.attributes[method_name].value if @xml.attributes.key?(method_name)

      # Then we try simple children tags: description, solution

      # Finally the enumerations: names
      if method_name == 'names'
        @xml.xpath("./names/name").collect(&:text)

      elsif ['fingerprints', 'software'].include?(method_name)

        xpath_selector = {
          'fingerprints' => './fingerprints/os',
          'software' => './software/fingerprint'
        }[method_name]

        @xml.xpath(xpath_selector).collect do |xml_os|
          Hash[
            xml_os.attributes.collect do |name, xml_attribute|
              next if name == 'arch'
              [name.sub(/-/,'_').to_sym, xml_attribute.value]
            end
          ]
        end
      else
        # nothing found, the tag is valid but not present in this ReportItem
        return nil
      end
    end
  end
end
