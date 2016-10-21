class CSVTasks < Thor
  include Core::Pro::ProjectScopedTask if defined?(::Core::Pro)

  namespace "dradis:plugins:csv"

  desc "export", "export issues to a CSV file"
  def export
    require 'config/environment'

    logger = Logger.new(STDOUT)
    logger.level = Logger::DEBUG
    content_service = nil


    if defined?(Dradis::Pro)
      detect_and_set_project_scope
      content_service = 'Dradis::Pro::Plugins::ContentService'
    else
      content_service = 'Dradis::Plugins::ContentService'
    end

    csv = Dradis::Plugins::CSV::Exporter.new.export({
      content_service: content_service
    })

    filename = "dradis_report-#{Time.now.to_i}.csv"
    File.open(filename, 'w') { |f| f.write csv }

    logger.info "File written to ./#{ filename }"

    logger.close
  end

end
