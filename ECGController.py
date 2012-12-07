import os, serial, sys, matplotlib, wx
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

myport = '/dev/tty.usbmodemfd122'
mybaud = 9600

ser = serial.Serial(port=myport, baudrate=mybaud)
#ser.write('200 60 1000\n')
#ser.flush()

class ParamWindow(wx.Frame):
        def __init__(self):
                wx.Frame.__init__(self, None, -1, "ECG Generator", (100,100), (220,200))
                
                self.SetBackgroundColour('#ece9d8')
                self.isLogging = False

                self.fig = Figure(facecolor='#ece9d8')

                a = wx.StaticText(self,1,"Frequency:",(20,20))
                b = wx.StaticText(self,2,"BPM:",(20,40))
                c = wx.StaticText(self,3,"Amplitude:",(20,60))
                c = wx.StaticText(self,4,"Gain:",(20,80))
                self.freq=wx.TextCtrl(self,5,'200',(100,20),style=wx.TE_PROCESS_ENTER)
                self.bpm=wx.TextCtrl(self,6,'60',(100,40),style=wx.TE_PROCESS_ENTER)
                self.amp=wx.TextCtrl(self,7,'1000',(100,60),style=wx.TE_PROCESS_ENTER)
                self.gain=wx.TextCtrl(self,8,'1000',(100,80),style=wx.TE_PROCESS_ENTER)
                self.Bind(wx.EVT_TEXT_ENTER, self.on_freq_enter, self.freq)
                self.Bind(wx.EVT_TEXT_ENTER, self.on_bpm_enter, self.bpm)
                self.Bind(wx.EVT_TEXT_ENTER, self.on_amp_enter, self.amp)
                self.Bind(wx.EVT_TEXT_ENTER, self.on_gain_enter, self.gain)

                self.enter_button = wx.Button(self, label="Enter", pos=(50,120), size=(100,30))
                self.enter_button.SetFont(wx.Font(14, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False))
                self.enter_button.Bind(wx.EVT_BUTTON, self.onEnterButton)

        def on_freq_enter(self, event):
            self.value = self.freq.GetValue()
            global freq
            freq = self.value
            #ser.write(freq+"\n")

        def on_bpm_enter(self, event):
            self.value = self.bpm.GetValue()
            global bpm
            bpm = self.value
            #ser.write(bpm+"\n")

        def on_amp_enter(self, event):
            self.value = self.amp.GetValue()
            global amp
            amp = self.value
            #ser.write(amp+"\n")

        def on_gain_enter(self, event):
            self.value = self.gain.GetValue()
            global gain
            gain = self.value

        def onEnterButton(self, event):
            print freq+' '+bpm+' '+amp+' '+gain+"\n"
            ser.write(freq+' '+bpm+' '+amp+' '+gain+"\n")
            ser.flush()

        def on_exit(self, event):
                self.ser.close()
                self.Destroy()


if __name__ == '__main__':
    app = wx.App()
    window = ParamWindow()
    window.Show()
    app.MainLoop()