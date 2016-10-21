ENV["RAILS_ENV"] ||= 'test'
require File.expand_path("../../../../../config/environment", __FILE__)
require 'rspec/rails'

# Requires supporting ruby files with custom matchers and macros, etc,
# in spec/support/ and its subdirectories.
# require 'support/fixture_loader'

RSpec.configure do |config|

  # uncomment to use :focus => true to run a single Spec
  # config.filter_run :focus => true

  # If you're not using ActiveRecord, or you'd prefer not to run each of your
  # examples within a transaction, remove the following line or assign false
  # instead of true.
  config.use_transactional_fixtures = true

  config.before(:suite) do
    Configuration.create(:name=>'revision', :value=>'0')
  end
end
