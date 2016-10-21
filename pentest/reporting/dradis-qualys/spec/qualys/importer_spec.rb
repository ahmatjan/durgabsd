require "spec_helper"
require "ostruct"

describe Dradis::Plugins::Qualys::Importer do
  let(:plugin) { Dradis::Plugins::Qualys }

  let(:content_service)  { Dradis::Plugins::ContentService.new(plugin: plugin) }
  let(:template_service) { Dradis::Plugins::TemplateService.new(plugin: plugin) }

  let(:importer) {
    described_class.new(
      content_service: content_service,
      template_service: template_service
    )
  }

  before do
    # Stub template service
    templates_dir = File.expand_path('../../../templates', __FILE__)
    allow_any_instance_of(Dradis::Plugins::TemplateService).to \
      receive(:default_templates_dir).and_return(templates_dir)

    # Stub dradis-plugins methods
    #
    # They return their argument hashes as objects mimicking
    # Nodes, Issues, etc
    %i[node note evidence issue].each do |model|
      allow(content_service).to receive(:"create_#{model}") do |args|
        OpenStruct.new(args)
      end
    end
  end

  let(:example_xml) { 'spec/fixtures/files/simple.xml' }

  pending "collapses INFOS|SERVICES|VULNS|PRACTICES node if only a single element is found"

  def run_import!
    importer.import(file: example_xml)
  end

  it "creates nodes as needed" do
    # Host node
    expect_to_create_node_with(label: '10.0.155.160')

    # Information gathering node
    expect_to_create_node_with(label: 'infos - Information gathering')

    # Services node with its child nodes
    expect_to_create_node_with(label: 'services')
    expect_to_create_node_with(label: 'TCP/IP')
    expect_to_create_node_with(label: 'Web server')

    run_import!
  end


  it "creates notes as needed" do
    # Host node notes
    expect_to_create_note_with(text: "Basic host info")

    # Information gathering node and notes
    expect_to_create_note_with(
      text: "DNS Host Name",
      node_label: "infos - Information gathering"
    )
    expect_to_create_note_with(
      text: "Host Scan Time",
      node_label: "infos - Information gathering"
    )

    # Child notes of Services node
    expect_to_create_note_with(
      text: "Open TCP Services List",
      node_label: "TCP/IP"
    )

    expect_to_create_note_with(
      text: "Web Server Version",
      node_label: "Web server"
    )

    run_import!
  end

  # Issues and evidences from vulns
  # There are 3 vulns in total:
  #   - TCP/IP: Sequence number in both hosts
  #   - Web server: Apache 1.3
  #   - Web server: ETag
  # Each one should create 1 issue and 1 evidence

  it "creates issues from vulns" do
    expect_to_create_issue_with(
      text: "Sequence Number Approximation Based Denial of Service"
    )

    expect_to_create_issue_with(
      text: "Apache 1.3 HTTP Server Expect Header Cross-Site Scripting"
    )

    expect_to_create_issue_with(
      text: "Apache Web Server ETag Header Information Disclosure Weakness"
    )

    run_import!
  end

  it "creates evidence from vulns" do
    expect_to_create_evidence_with(
      content: "Tested on port 80 with an injected SYN/RST offset by 16 bytes.",
      issue: "Sequence Number Approximation Based Denial of Service",
      node_label: "10.0.155.160"
    )

    expect_to_create_evidence_with(
      content: "The expectation given in the Expect request-header",
      issue: "Apache 1.3 HTTP Server Expect Header Cross-Site Scripting",
      node_label: "10.0.155.160"
    )

    expect_to_create_evidence_with(
      content: "bee-4f12-00794aef",
      issue: "Apache Web Server ETag Header Information Disclosure Weakness",
      node_label: "10.0.155.160"
    )

    run_import!
  end

  # A VULN is not required to have a RESULT element.
  # See:
  #   https://github.com/securityroots/dradispro-tracker/issues/8
  #   https://qualysapi.qualys.eu/qwebhelp/fo_help/reports/report_dtd.htm
  context "when an issue has no RESULT element" do
    let(:example_xml) { 'spec/fixtures/files/no_result.xml' }

    it "detects an issue without a RESULT element and applies (n/a)" do
      # 1 node should be created:
      expect_to_create_node_with(label: '10.0.155.160')

      # There is 1 vuln in total:
      #   - TCP/IP: Sequence number in both hosts
      # Each one should create 1 issue and 1 evidence
      expect_to_create_issue_with(
        text: "Sequence Number Approximation Based Denial of Service"
      )

      expect_to_create_evidence_with(
        content: "n/a",
        issue: "Sequence Number Approximation Based Denial of Service",
        node_label: "10.0.155.160"
      )

      run_import!
    end
  end


  def expect_to_create_node_with(label:)
    expect(content_service).to receive(:create_node).with(
      hash_including label: label
    ).once
  end

  def expect_to_create_note_with(node_label: nil, text:)
    expect(content_service).to receive(:create_note) do |args|
      expect(args[:text]).to include text
      expect(args[:node].label).to eq node_label unless node_label.nil?
    end.once
  end

  def expect_to_create_issue_with(text:)
    expect(content_service).to receive(:create_issue) do |args|
      expect(args[:text]).to include text
      OpenStruct.new(args)
    end.once
  end

  def expect_to_create_evidence_with(content:, issue:, node_label:)
    expect(content_service).to receive(:create_evidence) do |args|
      expect(args[:content]).to include content
      expect(args[:issue].text).to include issue
      expect(args[:node].label).to eq node_label
    end.once
  end


end

