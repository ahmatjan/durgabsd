require 'spec_helper'

describe WikiImport do
  let(:source){ "=Title=\nEigen titel\n\n=Impact=\nbla\n\n=Reccommendation=\nniets\n\n\n=Description=\nleeg" }
  let(:result){ "#[Title]#\nEigen titel\n\n#[Impact]#\nbla\n\n#[Reccommendation]#\nniets\n\n\n#[Description]#\nleeg" }

  it "parses wiki markup to Dradis fields" do
    config = Configuration.create(:name=>'wikiimport:fields', :value=>'Title,Impact,Reccommendation,Description')
    WikiImport::fields_from_wikitext(source).should eq(result)
    config.destroy
  end

  it "strips whitespace from the wikiimport:fields setting" do
    config = Configuration.create(:name=>'wikiimport:fields', :value=>'Title, Impact,Reccommendation ,Description')
    WikiImport::fields_from_wikitext(source).should eq(result)
    config.destroy
  end
end