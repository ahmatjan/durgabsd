source 'https://rubygems.org'

# Specify your gem's dependencies in dradis-vulndbhq.gemspec
gemspec

if Dir.exists?('../dradis-plugins')
  gem 'dradis-plugins', path: '../dradis-plugins'
else
  gem 'dradis-plugins', github: 'dradis/dradis-plugins'
end
