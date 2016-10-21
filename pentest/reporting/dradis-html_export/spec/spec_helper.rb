# Configure Rails Envinronment
ENV["RAILS_ENV"] ||= 'test'
require File.expand_path("../dummy/config/environment", __FILE__)

require 'rspec/rails'

# Requires supporting ruby files with custom matchers and macros, etc,
# in spec/support/ and its subdirectories.
Dir["#{File.dirname(__FILE__)}/support/**/*.rb"].each {|f| require f}

require 'database_cleaner'
require 'factory_girl_rails'


RSpec.configure do |config|

  # Capybara javascript drivers require transactional fixtures set to false,
  # and we just use DatabaseCleaner to cleanup after each test instead.
  # Without transactional fixtures set to false none of the records created to
  # setup a test will be available to the browser, which runs under a seperate
  # server instance.
  config.use_transactional_fixtures = false

  config.before(:each) do
    if example.metadata[:js]
      DatabaseCleaner.strategy = :truncation
    else
      DatabaseCleaner.strategy = :transaction
    end
  end

  config.before(:each) do
    DatabaseCleaner.start
  end

  config.after(:each) do
    DatabaseCleaner.clean
  end
end
