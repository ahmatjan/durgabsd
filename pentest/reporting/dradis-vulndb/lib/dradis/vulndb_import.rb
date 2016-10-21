# VulndbImport

require 'dradis/vulndb_import/filters'
require 'dradis/vulndb_import/meta'

module Dradis
  module VulndbImport
    class Configuration < Core::Configurator
      configure :namespace => 'vulndb'
      setting :rest_url, :default => 'https://localhost/'
      setting :hq_rest_url, :default => 'https://youremail%40emaildomain.com:password@user.vulndbhq.com'
    end       

    class Page < ActiveResource::Base
      self.site = Configuration.rest_url
    end  
  end
end

module Plugins
  module Import
    include Dradis::VulndbImport
  end
end
