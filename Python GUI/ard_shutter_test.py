import tkinter as tk
from ard_shutter_panel import Panel
from ard_shutter import Shutter
import logging, sys


#logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
#logging.basicConfig(stream=sys.stderr, level=logging.INFO)
logging.basicConfig(stream=sys.stderr)


def device_check(event):
    dev_num = shutter.get_num_devices()
    if dev_num is not None:
        panel.set_device_num_shutters(dev_num)


def device_clear(event):
    shutter.clear()


def eeprom_save(event):
    shutter.save()


def state_open(event):
    shutter_sel = panel.get_shutter_selected()
    shutter.set_state(shutter_sel, 1)


def state_close(event):
    shutter_sel = panel.get_shutter_selected()
    shutter.set_state(shutter_sel, 0)


def state_manual(event):
    shutter_sel = panel.get_shutter_selected()
    manualPos = panel.get_manual_position()
    shutter.set_position(shutter_sel,manualPos)


def state_check(event):
    shutter_sel = panel.get_shutter_selected()
    num = shutter.check_state(shutter_sel)
    if num is not None:
        panel.status_add_line(f'Device number {shutter_sel} state is {num}')

def param_get(event):
    shutter_sel = panel.get_shutter_selected()
    params = shutter.get_parameters(shutter_sel)
    if params:
        panel.set_panel_params(params)


def param_set(event):
    shutter_sel = panel.get_shutter_selected()
    params = panel.get_panel_params()
    if params:
        shutter.set_parameters(shutter_sel, params)


# setup
window = tk.Tk()
panel = Panel(window)

# bind the buttons from the panel to the functions above
panel.bind_button('deviceCheck', device_check)
panel.bind_button('deviceClear', device_clear)
panel.bind_button('eepromSave', eeprom_save)
panel.bind_button('stateOpen', state_open)
panel.bind_button('stateClose', state_close)
panel.bind_button('stateManual', state_manual)
panel.bind_button('stateCheck', state_check)
panel.bind_button('paramGet', param_get)
panel.bind_button('paramSet', param_set)

# open the shutter
shutter = Shutter('ASRL4::INSTR')

# run the GUI
window.mainloop()
