module Nikto
  # This class represents each of the <item> elements in the Nikto
  # XML document.
  #
  # It provides a convenient way to access the information scattered all over
  # the XML in attributes and nested tags.
  #
  # Instead of providing separate methods for each supported property we rely
  # on Ruby's #method_missing to do most of the work.
  class Item
    # Accepts an XML node from Nokogiri::XML.
    def initialize(xml_node)
      @xml = xml_node
    end

    # List of supported tags. They can be attributes, simple descendants or
    # collections (e.g. <references/>, <tags/>)
    def supported_tags
      [
        # attributes
        :id, :request_method, :osvdblink, :osvdbid,

        # simple tags
        :description, :uri, :namelink, :iplink
      ]
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
      # We need the translations_table because 'method' is a reserved word
      translations_table = {
        request_method: 'method'
      }
      method_name = translations_table.fetch(method, method.to_s)

      # First we try the attributes
      return @xml.attributes[method_name].value if @xml.attributes.key?(method_name)

      # Then we try simple children tags
      tag = @xml.xpath("./#{ method_name }").first
      if tag
        return tag.text
      end
    end
  end
end
