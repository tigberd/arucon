require 'serialport'
require 'yaml'

$LOAD_PATH.unshift(File.dirname(__FILE__) + '/lib')

require 'arucon'

config = YAML.load(File.open('config.yml').read)
serial_port = SerialPort.new(
  config['serial_port']['port'],
  config['serial_port']['bps'],
  8,
  1,
  SerialPort::NONE
)

parser  = Arucon::Parser::Parser.new(config['button'])
adapter = Arucon::Adapter.new(serial_port)

require 'sinatra'

lock = false

get '/index' do
  erb :test
end

post '/index' do
  loop do
    break unless lock
    sleep 0.1
  end
  lock = true
  parsed_step = parser.parse("frame,lever,button\r\n" + params[:command])
  adapter.send(parsed_step)
  lock = false
  parsed_step.map { |step| step.to_serial}
end
