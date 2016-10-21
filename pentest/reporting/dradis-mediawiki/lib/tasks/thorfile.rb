class MediawikiTasks < Thor
  namespace "dradis:plugins:wiki"

  desc "search QUERY", "perform a general search against a remote MediaWiki"
  def search(query)
    require 'config/environment'

    results = Dradis::Plugins::Mediawiki::Filters::FullTextSearch.new.query(query: query)

    puts "Wiki Search\n==========="
    puts "#{results.size} results"

    results.each do |record|
      puts "#{record.title}\n\t#{record.description}"
    end
  end
end
