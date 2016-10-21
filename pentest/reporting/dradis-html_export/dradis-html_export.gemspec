# -*- encoding: utf-8 -*-
require File.expand_path('../lib/dradis/plugins/html_export/version', __FILE__)
version = Dradis::Plugins::HtmlExport::version

Gem::Specification.new do |spec|
  spec.platform      = Gem::Platform::RUBY
  spec.name          = "dradis-html_export"
  spec.version       = version
  spec.required_ruby_version = '>= 1.9.3'
  spec.license       = 'GPL-2'

  spec.authors       = ['Daniel Martin']
  spec.email         = ["<etd@nomejortu.com>"]
  spec.description   = %q{Export to HTML plugin for the Dradis Framework}
  spec.summary       = %q{Dradis HTML export plugin}
  spec.homepage      = 'http://dradisframework.org'

  spec.files         = `git ls-files`.split($\)
  spec.executables   = spec.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  # gem.add_dependency 'dradis_core', version
  spec.add_dependency 'dradis-plugins', '~> 3.3'

  # Note markup
  spec.add_dependency 'rails_autolink', '~> 1.1'
  spec.add_dependency 'RedCloth', '4.2.9'

  # gem.add_development_dependency 'capybara', '~> 1.1.3'
  # gem.add_development_dependency 'database_cleaner'
  # gem.add_development_dependency 'factory_girl_rails'
  # gem.add_development_dependency 'rspec-rails',  '~> 2.11.0'
  # gem.add_development_dependency 'sqlite3'
end
