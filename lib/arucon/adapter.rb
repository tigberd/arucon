require 'logger'

module Arucon
  class Adapter
    SIGNAL = "SIGNAL\r\n"

    def initialize(serial_port, logger: Logger.new(STDOUT))
      @serial_port = serial_port
      @logger = logger
      sleep 2
    end

    def send(steps)
      steps.each do |step|
        @logger.debug(step)
        @logger.debug(step.to_serial)
        @serial_port.puts step.to_serial

        loop do
          break if @serial_port.gets == SIGNAL
        end
      end

      @serial_port.putc "\0"
    end
  end
end
