#!/usr/bin/env rake

# Kudos for the dummy app tasks to:
#   https://coderwall.com/p/743kfg
#   https://gist.github.com/4131823

require 'bundler/gem_tasks'
require 'rake'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:spec => :dummy_app) do |t|
  t.pattern =  File.expand_path('../spec/**/*_spec.rb', __FILE__)
end
task :default => :spec


desc 'Generates a dummy app for testing'
task :dummy_app => [:setup, :migrate]

task :setup do
  require 'rails'
  require 'dradis_core'
  require 'dradis/generators/dummy/dummy_generator'

  dummy = File.expand_path('../spec/dummy', __FILE__)

  sh "rm -rf #{dummy}"
  Dradis::DummyGenerator.start(
    %W(. --quiet --force --skip-bundle --dummy-path=#{dummy})
  )
end

task :migrate do
  rakefile = File.expand_path('../spec/dummy/Rakefile', __FILE__)

  # dradis_core migrations
  sh("rake -f #{rakefile} dradis:install:migrations")

  # this gem's migrations
  if Dir[ File.expand_path('../db/migrate/*', __FILE__) ].empty?
    puts "Skipping :migrate as this plugin doesn't define any migrations"
  else
    # TODO: hard-coded plugin name ahead!
    sh("rake -f #{rakefile} dradis_html_export:install:migrations")
  end
  sh("rake -f #{rakefile} db:create db:migrate db:test:prepare")
end