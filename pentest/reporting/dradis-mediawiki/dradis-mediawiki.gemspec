$:.push File.expand_path('../lib', __FILE__)
require 'dradis/plugins/mediawiki/version'
version = Dradis::Plugins::Mediawiki::VERSION::STRING

# Describe your gem and declare its dependencies:
Gem::Specification.new do |spec|
  spec.platform    = Gem::Platform::RUBY
  spec.name        = 'dradis-mediawiki'
  spec.version     = version
  spec.summary     = 'MediaWiki add-on for the Dradis Framework.'
  spec.description = 'Use a MediaWiki instance as source of re-usable issue entries for Dradis Framework.'

  spec.license     = 'GPL-2'

  spec.authors     = ['Daniel Martin']
  spec.email       = ['etd@nomejortu.com']
  spec.homepage    = 'http://dradisframework.org'

  spec.files       = `git ls-files`.split($\)
  spec.executables = spec.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  spec.test_files  = spec.files.grep(%r{^(test|spec|features)/})

  # By not including Rails as a dependency, we can use the gem with different
  # versions of Rails (a sure recipe for disaster, I'm sure), which is needed
  # until we bump Dradis Pro to 4.1.
  # s.add_dependency 'rails', '~> 4.1.1'
  spec.add_dependency 'dradis-plugins', '~> 3.0'

  spec.add_development_dependency 'bundler', '~> 1.6'
  spec.add_development_dependency 'rake', '~> 10.0'
end
