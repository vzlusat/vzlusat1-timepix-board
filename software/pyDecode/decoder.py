import matplotlib
matplotlib.use('TkAgg')

from numpy import arange, sin, pi
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
# implement the default mpl key bindings
from matplotlib.backend_bases import key_press_handler

from matplotlib.figure import Figure

import os
import numpy
from src.Image import Image 
from src.loadImage import loadImage
from src.parseInputFile import parseInputFile

import sys
if sys.version_info[0] < 3:
    import Tkinter as Tk
else:
    import tkinter as Tk

import tkFileDialog

root = Tk.Tk()
root.resizable(width=1, height=1)
root.geometry('{}x{}'.format(700, 700))
root.wm_title("VZLUSAT-1 X-Ray data decoder")

#log = Tk.Tk()
#log.resizable(width=1, height=1)
#log.geometry('{}x{}'.format(700, 200))
#log.wm_title("Log")
#
#text_log = Tk.Text(log)
#text_log.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)

# plot
f = Figure(facecolor='none')

frame_main = Tk.Frame(root);
frame_main.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)

v = Tk.StringVar()
log = Tk.Label(root, anchor=Tk.W, justify=Tk.LEFT, textvariable=v, height=1, bg="white", bd=2, highlightbackground="black")
log.pack(side=Tk.BOTTOM, fill=Tk.X, expand=0)

frame_left1 = Tk.Frame(frame_main, bd=1);
frame_left1.pack(side=Tk.LEFT, fill=Tk.Y, expand=0, padx=5, pady=5)

frame_right1 = Tk.Frame(frame_main);
frame_right1.pack(side=Tk.RIGHT, fill=Tk.BOTH, expand=1, padx=5, pady=5)

frame_above_canvas = Tk.Frame(frame_right1, bd=1);
frame_above_canvas.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1, padx=5, pady=5)

metadata = Tk.Label(frame_above_canvas, anchor=Tk.W, justify=Tk.LEFT, height=20, bg="white", bd=2, highlightbackground="black")
metadata.pack(side=Tk.TOP, fill=Tk.BOTH, expand=0)

# a tk.DrawingArea
frame_canvas = Tk.Frame(frame_right1);
frame_canvas.pack(side=Tk.BOTTOM, fill=Tk.BOTH, expand=0)

canvas = FigureCanvasTkAgg(f, master=frame_canvas)
canvas.show()
canvas.get_tk_widget().pack(side=Tk.TOP)
canvas._tkcanvas.pack(side=Tk.TOP)

# toolbar
frame_toolbar = Tk.Frame(frame_canvas);
frame_toolbar.pack(side=Tk.BOTTOM, fill=Tk.Y, expand=1)

toolbar = NavigationToolbar2TkAgg(canvas, frame_toolbar)
toolbar.pack(side=Tk.LEFT)
toolbar.update()

def loadFiles():

    file_names = os.listdir("images")
    file_names.sort()
    list_files = []

    for file in file_names:
        image = loadImage(file)

        if image != 0:
           list_files.append(str(image.id)+"_"+str(image.type)) 
        else:
            print "could not open file "+file

    v.set("All images loaded")

    return list_files

list_files = loadFiles()

frame_list = Tk.Frame(frame_left1);
frame_list.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)

frame_list2 = Tk.Frame(frame_list);
frame_list2.pack(side=Tk.BOTTOM, fill=Tk.BOTH, expand=1)

scrollbar = Tk.Scrollbar(master=frame_list2, orient=Tk.VERTICAL)
listbox = Tk.Listbox(master=frame_list2, yscrollcommand=scrollbar.set, selectmode=Tk.SINGLE)
scrollbar.config(command=listbox.yview)
scrollbar.pack(side=Tk.LEFT, fill=Tk.Y, expand=0)
listbox.pack(side=Tk.LEFT, fill=Tk.Y, expand=0)
for item in list_files:
    listbox.insert(Tk.END, item)

def showImage(image):

    if image.got_data == 1:

        print "pes"

    if image.got_data == 1:

        if image.type >= 1 and image.type <= 8:

            # plot the image
            f.clf()
            a = f.add_subplot(111)
            a.imshow(image.data, interpolation='none')

        elif image.type == 16:

            f.clf()
            a1 = f.add_subplot(211)
            a2 = f.add_subplot(212)

            x = numpy.linspace(1, 256, 256)

            a1.plot(x, image.data[0, :])
            a2.plot(x, image.data[1, :])
            a1.axis([1, 256, numpy.min(image.data[0, :]), numpy.max(image.data[0, :])])
            a2.axis([1, 256, numpy.min(image.data[1, :]), numpy.max(image.data[1, :])])

        elif image.type == 32:

            f.clf()
            a = f.add_subplot(111)

            x = numpy.linspace(1, 16, 16)

            a.plot(x, image.data[0, :])

    else:

        f.clf()

    canvas.show()

def onselect(evt):
    # Note here that Tkinter passes an event object to onselect()
    w = evt.widget
    index = int(w.curselection()[0])
    value = w.get(index)

    # load the image
    file_names = os.listdir("images")
    file_names.sort()
    image = loadImage(file_names[index])

    showImage(image)

listbox.bind('<<ListboxSelect>>', onselect)

# quit button
def _quit():
    root.quit()   
    root.destroy()
 
button = Tk.Button(master=frame_left1, text='Quit', command=_quit)
button.pack(side=Tk.BOTTOM)

def _loadNewImages():

    file_name = tkFileDialog.askopenfilename()
    print "Openning file \"{}\"".format(file_name)
    parseInputFile(file_name, v, root)
    list_files = loadFiles()

    listbox.delete(0, Tk.END)
    for item in list_files:
        listbox.insert(Tk.END, item)

load_button = Tk.Button(master=frame_list, text='Load new images', command=_loadNewImages)
load_button.pack(side=Tk.TOP)

# detecting keyboard keypresses
def on_key_event(event):
    print('you pressed %s' % event.key)

    if event.key == 'q':
        _quit()

canvas.mpl_connect('key_press_event', on_key_event)

Tk.mainloop()
# If you put root.destroy() here, it will cause an error if
# the window is closed with the window manager.

