import os, serial, sys, matplotlib, wx, time, numpy, struct
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg
from matplotlib.figure import Figure
import matplotlib.pyplot as plt
from StringIO import StringIO
from collections import deque

class DataLoggerWindow(wx.Frame):
        def __init__(self):
                wx.Frame.__init__(self, None, -1, "Signal Plotter", (100,100), (640,480))
                
                self.SetBackgroundColour('#ece9d8')
                self.isLogging = False


                self.N = 1000  ##self.N / sampling freq = time window in seconds
                self.n = range(self.N)
                self.M = 1
                self.x = []
                self.x.append(0 * numpy.ones(self.N, numpy.float))

                self.fig = Figure(facecolor='#ece9d8')
                self.canvas = FigureCanvasWxAgg(self, -1, self.fig)
                self.canvas.SetPosition((0,0))
                self.canvas.SetSize((640,320))
                self.ax = self.fig.add_axes([0.08,0.1,0.86,0.8])
                self.ax.autoscale(False)
                self.ax.set_xlim(0, self.N-1)
                self.ax.set_ylim(0.0,3.3)
                self.ax.plot(self.n,self.x[0])


                self.timer = wx.Timer(self)
                self.Bind(wx.EVT_TIMER, self.GetSample, self.timer)


                self.start_stop_button = wx.Button(self, label="Start", pos=(25,320), size=(100,100))
                self.start_stop_button.SetFont(wx.Font(14, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False))
                self.start_stop_button.Bind(wx.EVT_BUTTON, self.onStartStopButton)
    
                a = wx.StaticText(self,1,"Period:",(150,365))
                #b = wx.StaticText(self,2,"Pins:",(380,365))
                self.freq=wx.TextCtrl(self,3,'.001',(240,365),style=wx.TE_PROCESS_ENTER)
                #self.pins=wx.TextCtrl(self,4,'1,0,0,0,0,0',(430,365),style=wx.TE_PROCESS_ENTER)

                self.Bind(wx.EVT_TEXT_ENTER, self.on_freq_enter, self.freq)
                #self.Bind(wx.EVT_TEXT_ENTER, self.on_pins_enter, self.pins)

                #st1 = wx.StaticText(self,5,"Time Window (s):",(235,403))
                #self.textarea = wx.TextCtrl(self, -1,style=wx.TE_READONLY,pos=(355,400),size=(50,25))


        def on_freq_enter(self, event):
            self.value = self.freq.GetValue()
            freq = self.value
            ser.write("f"+freq+"\n")
            ser.flush()
            #self.textarea.SetValue(str(self.N/float(freq)))

        
        """def on_pins_enter(self, event):
            self.value = self.pins.GetValue()
            pins = self.value
            pins_s = pins.split(',')
            bin_num = 0
            for i in range(6):
                if pins_s[5-i] == '1':
                    bin_num += pow(2,i)
            ser.write("p"+str(bin_num)+"\n")"""

            
        def GetSample(self, event=None):
                ser.flush()
                ser.write("t");
                while True:
                        #pream = ser.read(2)
                        #print ord(pream[0])
                        #if pream == "\xA0\xA0":
                        #print "BOOM!!!!!"
                        data = ser.read(100*2)
                        vals1 = numpy.array(struct.unpack(">"+"H"*100,data))
                        vals = vals1*3.3/65536
                        break

                #print len(vals)
                #print self.n
                #print self.x[0]
                text_file.write("%s\n"%str(vals))
                
                self.buff = vals
                d = deque(self.x[0])
                d.rotate(-len(self.buff))
                self.x[0] = list(d)
                self.x[0][self.N-len(self.buff):] = self.buff[:]


                self.ax.cla()
                self.ax.autoscale(False)
                self.ax.set_xlim(0, self.N - 1)
                self.ax.set_ylim(0.0,3.3)
                self.ax.plot(self.n, self.x[0])
                self.canvas.draw()

        def onStartStopButton(self, event):
                if not self.isLogging:
                        self.isLogging = True
                        self.ser = serial.Serial('/dev/tty.usbmodem1412',115200)
                        self.timer.Start(100)
                        self.start_stop_button.SetLabel("Stop")
                else:
                        self.timer.Stop()
                        self.ser.close()
                        self.isLogging = False
                        self.start_stop_button.SetLabel("Start")

        def on_exit(self, event):
                text_file.close()
                self.ser.close()
                self.Destroy()


if __name__ == '__main__':
    myport = '/dev/tty.usbmodem1412'
    mybaud = 115200
    ser = serial.Serial(port=myport, baudrate=mybaud)

    if ser.isOpen() != True:
        ser.open()

    text_file = open("Output.txt", "a+")

    app = wx.App()
    window = DataLoggerWindow()
    window.Show()
    app.MainLoop()

    
