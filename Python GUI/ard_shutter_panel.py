import tkinter as tk
from tkinter import ttk


class NumCtrl(ttk.Frame):
    def __init__(self, parent, text, minVal, maxVal, inc):
        super().__init__(parent)
        self.numInt = tk.IntVar(value = 0)
        ttk.Label(self, text=text).pack(anchor='w')
        ttk.Spinbox(self, width=10, 
                    from_=minVal, 
                    to=maxVal, 
                    increment=inc,
                    textvariable=self.numInt).pack(pady=1)


class NumDisplay(ttk.Frame):
    def __init__(self, parent, text):
        super().__init__(parent)
        self.numInt = tk.IntVar()
        ttk.Label(self, text=text).pack(anchor='w')
        tk.Entry(self, width=10, textvariable=self.numInt).pack(pady=1)


class StringCtrl(ttk.Frame):
    def __init__(self, parent, text):
        super().__init__(parent)
        self.valStr = tk.StringVar()
        ttk.Label(self, text=text).pack(anchor='w')
        tk.Entry(self, width=10, textvariable=self.valStr).pack()


class Panel:
    def __init__(self, window):
        window.geometry('950x400')
        window.title('Shutter Test Panel')
		
        labelFont = ('Helvetica',11,'bold');
        device_label = ttk.Label(window, text='Device:', font=labelFont)
        eeprom_label = ttk.Label(window, text='EEPROM:', font=labelFont)
        shutter_label = ttk.Label(window, text='Shutter:', font=labelFont)
        state_label = ttk.Label(window, text='State:', font=labelFont)
        prm_label = ttk.Label(window, text='Param:', font=labelFont)
        status_label = ttk.Label(window, text='Status:', font=labelFont)
        self.status_text_box = tk.Text(window, width=5, height=5)

        # define the frames
        deviceFrame = ttk.Frame(window)
        self.device_check_button = ttk.Button(deviceFrame, text='Check')
        self.device_num_shutters = NumDisplay(deviceFrame, 'Num shutters')
        self.device_clear_button = ttk.Button(deviceFrame, text='Clear')

        eepromFrame = ttk.Frame(window)
        self.eeprom_save_button = ttk.Button(eepromFrame, text='Save')

        shutterFrame = ttk.Frame(window, relief='ridge', borderwidth=3)
        self.shutter_selected =  NumCtrl(shutterFrame, 'Shutter selected', 0, 15, 1)

        stateFrame = ttk.Frame(window)
        self.state_open_button = ttk.Button(stateFrame, text='Open')
        self.state_close_button = ttk.Button(stateFrame, text='Close')
        self.state_manual_pos =  NumCtrl(stateFrame, 'Manual Pos', 200, 1000, 10)
        self.state_manual_button = ttk.Button(stateFrame, text='Set Manual')
        self.state_check_button = ttk.Button(stateFrame, text='Check')

        paramFrame = ttk.Frame(window)
        self.param_pwm_channel =  NumCtrl(paramFrame, 'PWM Channel', 0, 15, 1)
        self.param_dig_input =  NumCtrl(paramFrame, 'DigInput', -1, 3, 1)
        self.param_open_pos =  NumCtrl(paramFrame, 'Open Pos', 200, 1000, 10)
        self.param_closed_pos =  NumCtrl(paramFrame, 'Closed Pos', 200, 1000, 10)
        self.param_trans_delay =  NumCtrl(paramFrame, 'Trans Delay', 0, 500, 10)
        self.param_label = StringCtrl(paramFrame, 'Label')
        self.param_get_button = ttk.Button(paramFrame, text='Get')
        self.param_set_button = ttk.Button(paramFrame, text='Set')

        # place stuff into the frames
        self.device_check_button.pack(side='left', anchor='s', padx=10)
        self.device_num_shutters.pack(side='left', anchor='s', padx=10)
        self.device_clear_button.pack(side='left', anchor='s', padx=10)

        self.eeprom_save_button.pack(side='left', anchor='s', padx=10)

        self.shutter_selected.pack(side='left', anchor='s', padx=10)

        self.state_open_button.pack(side='left', anchor='s', padx=10)
        self.state_close_button.pack(side='left', anchor='s', padx=10)
        self.state_manual_pos.pack(side='left', anchor='s', padx=(30,10))
        self.state_manual_button.pack(side='left', anchor='s', padx=10)
        self.state_check_button.pack(side='left', anchor='s', padx=(30,10))

        self.param_pwm_channel.pack(side='left', anchor='s', padx=10)
        self.param_dig_input.pack(side='left', anchor='s', padx=10)
        self.param_open_pos.pack(side='left', anchor='s', padx=10)
        self.param_closed_pos.pack(side='left', anchor='s', padx=10)
        self.param_trans_delay.pack(side='left', anchor='s', padx=10)
        self.param_label.pack(side='left', anchor='s', padx=10)
        self.param_get_button.pack(side='left', anchor='s', padx=10)
        self.param_set_button.pack(side='left', anchor='s', padx=10)

        # place the panel grid
        window.columnconfigure(0, weight=1)
        window.columnconfigure(1, weight=6)
        window.rowconfigure((0,1,2,3,4,5), weight=1)

        device_label.grid(row=0, column=0, sticky='se')
        eeprom_label.grid(row=1, column=0, sticky='se')
        shutter_label.grid(row=2, column=0, sticky='se')
        state_label.grid(row=3, column=0, sticky='se')
        prm_label.grid(row=4, column=0, sticky='se')
        status_label.grid(row=5, column=0, sticky='e')

        deviceFrame.grid(row=0, column=1, sticky='sw')
        eepromFrame.grid(row=1, column=1, sticky='sw')
        shutterFrame.grid(row=2, column=1, sticky='sw')
        stateFrame.grid(row=3, column=1, sticky='sw')
        paramFrame.grid(row=4, column=1, sticky='sw')
        self.status_text_box.grid(row=5, column=1, padx=10, pady=10, sticky="nsew")


    def bind_button(self, button_name, func):
        if button_name == 'deviceCheck':
            self.device_check_button.bind('<Button>', func)
        elif button_name == 'deviceClear':
            self.device_clear_button.bind('<Button>', func)
        elif button_name == 'eepromSave':
            self.eeprom_save_button.bind('<Button>', func)
        elif button_name == 'stateOpen':
            self.state_open_button.bind('<Button>', func)
        elif button_name == 'stateClose':
            self.state_close_button.bind('<Button>', func)
        elif button_name == 'stateManual':
            self.state_manual_button.bind('<Button>', func)
        elif button_name == 'stateCheck':
            self.state_check_button.bind('<Button>', func)
        elif button_name == 'paramGet':
            self.param_get_button.bind('<Button>', func)
        elif button_name == 'paramSet':
            self.param_set_button.bind('<Button>', func)


    def set_device_num_shutters(self, val):
        return self.device_num_shutters.numInt.set(val)


    def get_shutter_selected(self):
        return self.shutter_selected.numInt.get()


    def status_add_line(self, text):
        self.status_text_box.insert('end', text+'\n')
        self.status_text_box.see('end')


    def get_manual_position(self):
        return self.state_manual_pos.numInt.get()


    def set_panel_params(self, params):
        self.param_pwm_channel.numInt.set(params['PWMChannel'])
        self.param_dig_input.numInt.set(params['digInput'])
        self.param_open_pos.numInt.set(params['openPos'])
        self.param_closed_pos.numInt.set(params['closedPos'])
        self.param_trans_delay.numInt.set(params['transDelay_ms'])
        self.param_label.valStr.set(params['label'])


    def get_panel_params(self):
        params = {}
        params['PWMChannel'] = self.param_pwm_channel.numInt.get()
        params['digInput'] = self.param_dig_input.numInt.get()
        params['openPos'] = self.param_open_pos.numInt.get()
        params['closedPos'] = self.param_closed_pos.numInt.get()
        params['transDelay_ms'] = self.param_trans_delay.numInt.get()
        params['label'] = self.param_label.valStr.get()
        return params
