require 'spec_helper'

describe 'Nmap upload plugin' do
  describe "Importer" do

    before(:each) do
      # Stub template service
      templates_dir = File.expand_path('../../templates', __FILE__)
      expect_any_instance_of(Dradis::Plugins::TemplateService)
      .to receive(:default_templates_dir).and_return(templates_dir)

      # Init services
      plugin = Dradis::Plugins::Nmap

      @content_service = Dradis::Plugins::ContentService.new(plugin: plugin)
      template_service = Dradis::Plugins::TemplateService.new(plugin: plugin)

      @importer = plugin::Importer.new(
        content_service: @content_service,
        template_service: template_service
      )

      # Stub dradis-plugins methods
      #
      # They return their argument hashes as objects mimicking
      # Nodes, Issues, etc
      allow(@content_service).to receive(:create_node) do |args|
        # puts "create_node: #{ args.inspect }"
        OpenStruct.new(args)
      end
      allow(@content_service).to receive(:create_note) do |args|
        # puts "create_note: #{ args.inspect }"
        OpenStruct.new(args)
      end
      allow(@content_service).to receive(:create_issue) do |args|
        # puts "create_issue: #{ args.inspect }"
        OpenStruct.new(args)
      end
      allow(@content_service).to receive(:create_evidence) do |args|
        # puts "create_evidence: #{ args.inspect }"
        OpenStruct.new(args)
      end
    end

    it "creates an error note when the xml is not valid" do
      expect(@content_service).to receive(:create_note) do |args|
        expect(args[:text]).to include("#[Title]#\nInvalid file format")
        OpenStruct.new(args)
      end.once
      # Run the import
      @importer.import(file: 'spec/fixtures/files/invalid.xml')
    end

    it "creates nodes, issues, notes and an evidences as needed" do
      expect(@content_service).to receive(:create_node) do |args|
        # puts "create_node: #{ args.inspect }"
        expect(args[:label]).to eq('74.207.244.221')
        expect(args[:type]).to eq(:host)
        OpenStruct.new(args)
      end.once
      expect(@content_service).to receive(:create_note) do |args|
        puts "create_note: #{ args.inspect }"
        expect(args[:text]).to include("#[Title]#\nNmap info: 74.207.244.221")
        expect(args[:text]).to_not include("not recognized by the plugin")
        expect(args[:node].label).to eq("74.207.244.221")
        OpenStruct.new(args)
      end.once
      expect(@content_service).to receive(:create_note) do |args|
        puts "create_note: #{ args.inspect }"
        expect(args[:text]).to include("#[Title]#\n22/tcp is open (syn-ack)")
        expect(args[:text]).to_not include("not recognized by the plugin")
        expect(args[:text]).to include("#[Host]#\n74.207.244.221")
        expect(args[:node].label).to eq("74.207.244.221")
        OpenStruct.new(args)
      end.once
      expect(@content_service).to receive(:create_note) do |args|
        puts "create_note: #{ args.inspect }"
        expect(args[:text]).to include("#[Title]#\n80/tcp is open (syn-ack)")
        expect(args[:text]).to_not include("not recognized by the plugin")
        expect(args[:text]).to include("#[Host]#\n74.207.244.221")
        expect(args[:node].label).to eq("74.207.244.221")
        OpenStruct.new(args)
      end.once

      # Run the import
      @importer.import(file: 'spec/fixtures/files/sample.xml')
    end

  end
end
