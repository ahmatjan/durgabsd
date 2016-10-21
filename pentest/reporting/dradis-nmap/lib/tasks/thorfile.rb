class NmapTasks < Thor
  include Core::Pro::ProjectScopedTask if defined?(::Core::Pro)

  namespace "dradis:plugins:nmap"

  desc      "upload FILE", "upload the results of an Nmap scan"
  long_desc "Upload an Nmap scan to create nodes and notes for the hosts and "\
            "ports discovered during scanning."
  def upload(file_path)
    require 'config/environment'

    unless File.exists?(file_path)
      $stderr.puts "** the file [#{file_path}] does not exist"
      exit(-1)
    end

    # Set project scope from the PROJECT_ID env variable:
    detect_and_set_project_scope if defined?(::Core::Pro)

    plugin = Dradis::Plugins::Nmap

    Dradis::Plugins::Nmap::Importer.new(
      logger:           logger,
      content_service:  service_namespace::ContentService.new(plugin: plugin),
      template_service: service_namespace::TemplateService.new(plugin: plugin)
    ).import(file: file_path)

    logger.close
  end


  private

  def logger
    @logger ||= Logger.new(STDOUT).tap { |l| l.level = Logger::DEBUG }
  end

  def service_namespace
    defined?(Dradis::Pro) ? Dradis::Pro::Plugins : Dradis::Plugins
  end

end
