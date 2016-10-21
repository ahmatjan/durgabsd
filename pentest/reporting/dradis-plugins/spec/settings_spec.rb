#
# This spec must be ran from Dradis root dir
#
require 'spec_helper'

class TestEngine < ::Rails::Engine
  include ::Dradis::Plugins::Base
  addon_settings :test_engine do
    settings.default_host     = 'localhost'
    settings.default_port     = 80
    settings.default_protocol = 'http'
  end
end

describe Dradis::Plugins::Settings do

  before(:each) do
    TestEngine::settings.reset_defaults!
  end

  it "sets and return default values" do
    expect(TestEngine::settings.host).to eq('localhost')
    expect(TestEngine::settings.port).to eq(80)
  end

  it "sets and returns user defined values" do
    expect(TestEngine::settings.host).to eq('localhost')
    TestEngine::settings.host = '127.0.0.1'
    expect(TestEngine::settings.host).to eq('127.0.0.1')
    expect(TestEngine::settings.port).to eq(80)
  end

  it "sets and returns new value even if it equals default value" do
    expect(TestEngine::settings.host).to eq('localhost')
    TestEngine::settings.host = '127.0.0.1'
    expect(TestEngine::settings.host).to eq('127.0.0.1')
    TestEngine::settings.host = 'localhost'
    expect(TestEngine::settings.host).to eq('localhost')
  end

  it "saves to db and returns persisted values" do
    expect(TestEngine::settings.host).to eq('localhost')
    TestEngine::settings.host = '127.0.0.1'
    expect_any_instance_of(TestEngine::settings::send(:configuration_class)).to receive(:update_attribute)
    expect(TestEngine::settings.save).to eq( { host: '127.0.0.1'} )
    expect(TestEngine::settings.host).to eq('127.0.0.1')
  end

  it "reads from db after saving" do
    expect(TestEngine::settings.host).to eq('localhost')
    TestEngine::settings.host = '127.0.0.1'
    expect(TestEngine::settings.save).to eq( { host: '127.0.0.1'} )
  end

end

describe Dradis::Plugins::Settings, '#is_default?' do
  it 'knows if a string value equals its default integer value' do
    TestEngine::settings.is_default?(:port, '80')
  end
end

describe Dradis::Plugins::Settings, '#all' do
  it 'returns values from db, dirty state or default as needed and tells which one is default' do
    TestEngine::settings.host = '127.0.0.1'
    TestEngine::settings.save
    TestEngine::settings.protocol = 'https'
    expect(TestEngine::settings.all).to eq([
      {
        name: :host,
        value: '127.0.0.1',
        default: false
      },
      {
        name: :port,
        value: 80,
        default: true
      },
      {
        name: :protocol,
        value: 'https',
        default: false
      },
    ])
  end
end


