# -*- encoding: utf-8 -*-
require File.expand_path('../lib/dradis/plugins/pdf_export/version', __FILE__)
version = Dradis::Plugins::PdfExport::version

Gem::Specification.new do |spec|
  spec.platform = Gem::Platform::RUBY
  spec.name = "dradis-pdf_export"
  spec.version = version
  spec.required_ruby_version = '>= 1.9.3'
  spec.license = 'GPL-2'

  spec.authors = ['Daniel Martin']
  spec.email = ["<etd@nomejortu.com>"]
  spec.description = %q{Export to PDF plugin for the Dradis Framework}
  spec.summary = %q{Dradis PDF export plugin}
  spec.homepage = 'http://dradisframework.org'

  spec.files = `git ls-files`.split($\)
  spec.executables = spec.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  spec.test_files = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  # gem.add_dependency 'dradis_core', version
  spec.add_dependency 'dradis-plugins', '~> 3.4.1'
  spec.add_dependency 'prawn', '~> 0.15.0'

  spec.add_development_dependency 'capybara', '~> 1.1.3'
  spec.add_development_dependency 'database_cleaner'
  spec.add_development_dependency 'factory_girl_rails'
  spec.add_development_dependency 'rspec-rails', '~> 2.11.0'
  spec.add_development_dependency 'sqlite3'
end
