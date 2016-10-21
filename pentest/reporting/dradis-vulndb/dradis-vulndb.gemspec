# -*- encoding: utf-8 -*-
$:.push File.expand_path('../lib', __FILE__)
require 'dradis/plugins/vulndb/version'
version = Dradis::Plugins::Vulndb::VERSION::STRING


Gem::Specification.new do |spec|
  spec.platform      = Gem::Platform::RUBY
  spec.name          = 'dradis-vulndb'
  spec.version       = version
  spec.summary       = %q{VulnDB HQ add-on for Dradis}
  spec.description   = %q{Import entries from your VulnDB HQ library into the Dradis Framework}

  spec.required_ruby_version = '>= 1.9.3'
  spec.license       = 'GPL-2'

  spec.authors       = ['Daniel Martin']
  spec.email         = ['<daniel@nomejortu.com>']
  spec.homepage      = 'http://dradisframework.org'

  spec.files         = `git ls-files`.split($\)
  spec.executables   = spec.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})

  # spec.require_paths = ["lib"]

  spec.add_dependency 'dradis-plugins', '~> 3.2'
  spec.add_dependency 'vulndbhq', '~> 0.1'

  spec.add_development_dependency 'bundler', '~> 1.6'
  spec.add_development_dependency 'rake', '~> 10.0'
end
