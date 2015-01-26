require 'csv'

module Arucon
  module Parser
    class Parser
      def initialize(config)
        @config = config
      end

      def parse(str)
        CSV.parse(str, headers: :first_row).map { |row|
          row.map { |key, value| [key, value.nil? ? '' : value] }.to_h
        }.select { |row|
          row.map { |_, value| value[0] != '#' }.reduce(&:&)
        }.map { |row|
          Step.new(
            row['frame'].strip.empty? ? '0' : row['frame'].strip,
            row['lever'].strip,
            row['button'].strip.gsub(' ', ''),
            config: @config
          )
        }
      end
    end
  end
end
