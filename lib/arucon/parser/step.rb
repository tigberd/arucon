module Arucon
  module Parser
    class Step
      LEVER = {
        '1' => '0101',
        '2' => '0100',
        '3' => '1100',
        '4' => '0001',
        '5' => '0000',
        '6' => '1000',
        '7' => '0011',
        '8' => '0010',
        '9' => '1010'
      }.freeze

      attr_reader :frame, :lever, :button

      def initialize(frame, lever, button, config: {})
        @config  = config
        @frame   = frame
        @lever   = lever
        @button  = to_button(button)
      end

      def to_serial
        command = frame + ','
        command += @button.include?('circle')   ? '1' : '0'
        command += @button.include?('triangle') ? '1' : '0'
        command += @button.include?('cross')    ? '1' : '0'
        command += @button.include?('square')   ? '1' : '0'

        command += LEVER[@lever].nil? ? '0000' : LEVER[@lever]

        command += @button.include?('l1')     ? '1' : '0'
        command += @button.include?('l2')     ? '1' : '0'
        command += @button.include?('r1')     ? '1' : '0'
        command += @button.include?('r2')     ? '1' : '0'
        command += @button.include?('start')  ? '1' : '0'
        command += @button.include?('ps')     ? '1' : '0'
        command += @button.include?('select') ? '1' : '0'

        command
      end

      private

      def to_button(button)
        list = []
        while button.size > 0
          button_to_array = @config.select { |_, aliaz|
            !aliaz.nil? && button[0..aliaz.size - 1].downcase == aliaz.downcase
          }.max { |x, y| x[1] <=> y[1] }

          fail TypeError.new('not match') if button_to_array.nil?
          button.slice!(0..(button_to_array[1].size - 1))
          list.push(button_to_array[0])
        end
        list
      end
    end
  end
end
