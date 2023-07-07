# original file : sony's calibration software
import serial
import re
from serial.tools import list_ports

###########################################################
# Error generated by Arduino Controller.
class ArduinoError(Exception):

    def __init__(self,value):
        self.value = value

    def __str__(self):
        err = {1: "Unknown 1character command",
               2: "config-empty or not end with !",
               3: "Unknown config parameter",
               4: "Unknown error"}
        if self.value <= 4:
            e = err[self.value]
        else:
            e = err[4]
        e += ' (%d)' % self.value
        return e
###########################################################




###########################################################
# Lv.Low control of the Arduino

class Arduino(object):

    def __init__(self,port=None):
        """
        - port==None
         1. attempt to find one
         2. no serial port found --> IOError
         3. port is busy --> SerialException
        """

        self.device_state = {}
        print("1) find available port : _ser is found")
        self._ser = self.__get__serial_com(port)    #port available found.
        print("2) detect Arduino~")
        self.__detect_arduino()                     #arduino available found.
        print("3) read the state")
        self.read_device_state()                    #

    def __get__serial_com(self,port):
        ser = serial.Serial()
        if port is None:
            arduino_list= list_ports.grep("VID:PID=(2341|2A03):")
            try:
                port,_,_ = arduino_list.next()
            except StopIteration:
                raise IOError('No Arduino Found')
        ser.port = port
        ser.baudrate = 115200
        ser.bytesize = 8
        ser.stopbits = 1
        ser.parity = 'N'
        ser.xonxoff = True
        ser.timeout = 5
        ser.open()
        print(port)
        return ser


    def __detect_arduino(self):
        retry = 10
        while retry>0:
            rx0 = self._ser.readline()
            rx = rx0.decode('UTF-8')
            print("RX : "+rx)
            if rx.endswith('READY\r\n'): #python3
                return
            retry -=1
        raise IOError('Cannot initialize communication with Arduino')


    def read_device_state(self):
        print("3-1) send question")
        rx = self.send_command('?')
        print("\t: rx = "+rx)
        regex = '(([\w.]+): ([^;]+)[;]*)+?'
        match = re.findall(regex, rx)
        self.device_state = dict(x[1:] for x in match)

    def __del__(self):
        self.close()

    def __str__(self):
        human_name = [n for p, n, a in list_ports.comports() if p == self._ser.port]
        txt = "%s on $s\n" %(human_name[0], self._ser.port)
        width = max([len(key) for key in self.device_state])
        for i, (key,value) in enumerate(self.device_state.iteritems()):
            if i <len(self.device_state)-1:
                txt+='\n'
        return txt

    def close(self):
        self._ser.close()

    @property
    def version(self):
        return self.device_state['Controller.Version']

    def send_command(self, cmd):
        self._ser.write(cmd.encode('UTF-8'))
        rx = self._ser.readline().rstrip().decode('UTF-8')
        if not rx.startswith('<'+cmd.upper()+'>'):
            self._unexpected(rx)
        rx = rx[3:]
        self._check_for_error(rx)
        return rx

    def send_value(self, name, value):
        content = name + ' ' + repr(value) + '!'
        self._ser.write(content.encode('UTF-8'))
        rx = self._ser.readline().rstrip().decode('UTF-8')
        self._check_for_error(rx)
        if rx != 'OK':
            self._unexpected(rx)

    def _unexpected(self, rx):
        raise IOError('Unexpected response: \'' + rx + '\'')

    def _check_for_error(self,rx):
        if rx.startswith('ERR:'):
            raise ArduinoError(int(rx[4:1]))

    def update_property(self, name, value):
        if type(value)(self.device_state[name]) != value:    # type(str1)(str2) : returns strings commonly found in both strings
            self.send_command('C')
            self.send_value(name, value)
            self.read_device_state()
