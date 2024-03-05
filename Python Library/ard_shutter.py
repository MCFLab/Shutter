import pyvisa
import sys
import re
import logging

class InstrumentError(Exception):
    """Exception to indicate an error while communicating with the Arduino"""
    pass


class Shutter:
    """Represents an Arduino shutter controller

    Instance variables (private):
      _rm: visa handle to resource manager
      _inst: visa handle to instrument
    Methods:
      get_num_devices: get # of shutters. This can change during operation
      get(set)_state(dev): get(set) the state (open-1, close-0) of shutter # dev
      get_device_label(dev): get the label of shutter # dev
      get_transit_delay(dev): get the transit delay in ms of shutter # dev
      set_position(dev): set the PWM position of shutter # dev
      get(set)_parameters(dev): get(set) the device parms of shutter # dev
      save: saves parameters to EEPROM
      clear: clears the device paramters and sets the num sutters to zero
    """
    
    def __init__(self, address):
        """ Connects to the shutter controller

        Opens the resource manager, then the device. Terminates if the device cannot 
        be found. Sets the communication attributes. Then checks the ID response and
        throws an exception if it's the wrong device.
        Arguments:
          address: a VISA Resource ID, like 'ASRL3::INSTR'
        """
        logging.info('Initializing instrument.')
        try:
            self._rm = pyvisa.ResourceManager('@py')
        except Exception as e:
            logging.error('Could not open resource manager.')
            print(e)
            sys.exit(1)
        try:
            self._inst = self._rm.open_resource(address)
        except Exception as e:
            self._rm.close()
            logging.info('Could not open instrument')
            print(e)
            sys.exit(1)
        logging.info(f'Instrument: {self._inst}')
        self._inst.read_termination = '\n'
        self._inst.write_termination = '\n'
        self._inst.baud_rate = 9600
        logging.info('Requesting ID from instrument')
        resp = self._inst.query('*IDN?').rstrip('\r\n')
        if not resp.startswith('Arduino Uno Shutter'):
            self._inst.close()
            self._rm.close()
            raise InstrumentError(
                f"Wrong ID response. Expected 'Arduino Uno Shutter', got '{resp}'.")


    def __del__(self):
        """ Closes the shutter controller

        Closes the instrument (if open) and the resource manager (if open).
        The deletes the attributes (just in case).
        """
        if hasattr(self, '_inst'):
            logging.info('Closing instrument.')
            self._inst.close()
            delattr(self, '_inst')
        else:
            logging.info('Instrument not open. Do nothing.')
        if hasattr(self, '_rm'):
            logging.info('Closing resource manager.')
            self._rm.close()
            delattr(self, '_rm')
        else:
            logging.info('Resource manager not open. Do nothing.')


    def get_num_devices(self):
        """ Gets the numer of attached shutters

        Sends the query and interprets the response. Sends back an integer.
        """
        logging.info('Checking the number of devices.')
        resp = self._inst.query('GND').rstrip('\r\n')
        if resp.startswith('Error'):
            logging.error(f"Invalid response. Expected a number, got '{resp}'.")
            return    
        return int(re.search(r'\d+', resp).group())


    def check_state(self, device):
        """ Gets the state of the given device

        Returns the state of the device: -1->inactive, 0->closed, 1->open, 2-> manual set
        Arguments:
          device: the selected shutter number (zero-based index)
        """
        logging.info('Checking the device state.')
        resp = self._inst.query(f'GST{device}').rstrip('\r\n')
        if resp.startswith('Error'):
            logging.error(f"Invalid response. Expected a number, got '{resp}'.")
            return    
        num = resp.split('=')[1]
        return int(num)


    def get_parameters(self, device):
        """ Gets the parameter for the given device

        Returns the parameters as a dictionary.
        Arguments:
          device: the selected shutter number (zero-based index)
        """
        logging.info('Getting device parameters.')
        resp = self._inst.query(f'GPR{device}').rstrip('\r\n').rstrip('\r\n')
        if not resp.startswith('PR'):
            logging.error(f"Invalid response. Expected 'PR...', got '{resp}'.")
            return {}
        parts = resp.rsplit(',', 1)
        label = parts[1]
        numbers = re.findall(r'[-+]?\d+', parts[0])
        if len(numbers)!=6:
            logging.error(f"Wrong response. Expected 6 parameters, got {len(numbers)}.")
            return {}
        return_dict = {}
        return_dict['PWMChannel'] = int(numbers[1])
        return_dict['digInput'] = int(numbers[2])
        return_dict['openPos'] = int(numbers[3])
        return_dict['closedPos'] = int(numbers[4])
        return_dict['transDelay_ms'] = int(numbers[5])
        return_dict['label'] = label
        return return_dict
    
       
    def set_parameters(self, device, params):
        """ Sets the parameter for the given device

        Arguments:
          device: the selected shutter number (zero-based index)
          params: The parameters as a dictionary.
        """
        logging.info('Setting device parameters.')
        resp = self._inst.query(f'SPR{device},'\
                                +f'{params['PWMChannel']},'\
                                +f'{params['digInput']},'\
                                +f'{params['openPos']},'\
                                +f'{params['closedPos']},'\
                                +f'{params['transDelay_ms']},'\
                                +f'{params['label']}').rstrip('\r\n')
        if resp!='OK':
            logging.error(f"Invalid response. Expected 'OK', got '{resp}'.")


    def set_state(self, device, state):
        """ Sets the state of the given device

        Opens/closes the shutter.
        Arguments:
          device: the selected shutter number (zero-based index)
          state: 0->close, 1->open
        """
        logging.info('Setting shutter state.')
        resp = self._inst.query(f'SST{device},{state}').rstrip('\r\n')
        if resp!='OK':
            logging.error(f"Invalid response. Expected 'OK', got '{resp}'.")


    def set_position(self, device, position):
        """ Sets the position of the servo for a given device

        Sets the position of the RC servo manually (can be different from the open/close poes).
        Arguments:
          device: the selected shutter number (zero-based index)
          position: PWM value for the RC servo
        """
        logging.info('Setting servo position.')
        resp = self._inst.query(f'SSP{device},{position}').rstrip('\r\n')
        if resp!='OK':
            logging.error(f"Invalid response. Expected 'OK', got '{resp}'.")


    def clear(self):
        """ Clears the device parameters

        Sets the number of devices to zero
        """
        logging.info('Clearing the device parameters.')
        resp = self._inst.query('CLR').rstrip('\r\n')
        if resp!='OK':
            logging.error(f"Invalid response. Expected 'OK', got '{resp}'.")


    def save(self):
        """ Saves the current parameters to EEPROM

        Only writes to the EEPROM if the number of devices is > 0
        """
        logging.info('Saving the device parameters to EEPROM.')
        resp = self._inst.query('SAV').rstrip('\r\n')
        if resp!='OK':
            logging.error(f"Invalid response. Expected 'OK', got '{resp}'.")