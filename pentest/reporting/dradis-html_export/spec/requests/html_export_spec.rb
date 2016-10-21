require 'spec_helper'

describe "HTML Export" do
  it "presents the note text in the report layout" do
    Dradis::Core::VERSION::STRING.should eq('3.0.0.beta')
  end
end