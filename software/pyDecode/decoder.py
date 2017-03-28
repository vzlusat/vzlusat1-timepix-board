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
root.geometry('{}x{}'.format(1200, 600))
root.wm_title("VZLUSAT-1 X-Ray data decoder")

# plot
f = Figure(facecolor='none')
f.clf()

frame_main = Tk.Frame(root);
frame_main.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)

v = Tk.StringVar()
log = Tk.Label(root, anchor=Tk.W, justify=Tk.LEFT, textvariable=v, height=1, bg="white", bd=2, highlightbackground="black")
log.pack(side=Tk.BOTTOM, fill=Tk.X, expand=0)

frame_left1 = Tk.Frame(frame_main, bd=1);
frame_left1.pack(side=Tk.LEFT, fill=Tk.Y, expand=0, padx=5, pady=5)

frame_right1 = Tk.Frame(frame_main);
frame_right1.pack(side=Tk.RIGHT, fill=Tk.BOTH, expand=1, padx=5, pady=5)

frame_left_to_canvas = Tk.Frame(frame_right1, bd=1);
frame_left_to_canvas.pack(side=Tk.LEFT, fill=Tk.BOTH, expand=0, padx=10, pady=30)

metadatas = []
metadatas_var = []
for i in range(len(Image.metadata_labels)): #Rows
    b = Tk.Label(frame_left_to_canvas, text=Image.metadata_labels[i]).grid(row=i, column=0, sticky=Tk.E)
    metadatas_var.append(Tk.StringVar())
    metadatas.append(Tk.Label(frame_left_to_canvas, textvariable=metadatas_var[i]).grid(row=i, column=1, sticky=Tk.W))

# a tk.DrawingArea
frame_canvas = Tk.Frame(frame_right1);
frame_canvas.pack(side=Tk.RIGHT, fill=Tk.BOTH, expand=0, padx=5, pady=5)

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

colormap = "hot"

def showImage(image):

    if image.got_metadata == 1:

        metadatas_var[0].set(str(image.id))

        if image.type == 1:
            img_type = "RAW"
        elif image.type == 2:
            img_type = "Binning 8"
        elif image.type == 4:
            img_type = "Binning 16"
        elif image.type == 8:
            img_type = "Binning 32"
        elif image.type == 16:
            img_type = "Sums"
        elif image.type == 32:
            img_type = "Energy histogram"

        metadatas_var[1].set(img_type)

        if image.mode == 0:
            mode = "Medipix (Counting)"
        else:
            mode = "Timepix (Energy)"

        metadatas_var[2].set(mode)

        metadatas_var[3].set(image.threshold)
        metadatas_var[4].set(image.bias)
        metadatas_var[5].set(image.exposure)

        if image.filtering == 0:
            filtering = "OFF"
        else:
            filtering = "ON"

        metadatas_var[6].set(filtering)
        metadatas_var[7].set(image.filtered_pixels)
        metadatas_var[8].set(image.original_pixels)
        metadatas_var[9].set(image.min_original)
        metadatas_var[10].set(image.max_original)
        metadatas_var[11].set(image.min_filtered)
        metadatas_var[12].set(image.max_filtered)
        metadatas_var[13].set(str(image.temperature)+" C")
        metadatas_var[14].set(str(image.temp_limit)+" C")
        metadatas_var[15].set(image.pxl_limit)
        metadatas_var[16].set(image.uv1_thr)

        attitude = ""
        for att in image.attitude:
            attitude += str(att)+" "

        metadatas_var[17].set(attitude)

        position = ""
        for pos in image.position:
            position += str(pos)+" "

        metadatas_var[18].set(position)
        metadatas_var[19].set(image.time)

        if image.type == 2:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+15)
        elif image.type == 4:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+3)
        elif image.type == 8:
            chunk_id = str(image.chunk_id)
        elif image.type == 16:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+7)
        elif image.type == 32:
            chunk_id = str(image.chunk_id)
        elif image.type == 1:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+int(numpy.floor(image.filtered_pixels/20)))

        metadatas_var[20].set(chunk_id)

    if image.got_data == 1:

        if image.type >= 1 and image.type <= 8:

            # plot the image
            f.clf()
            a = f.add_subplot(111)
            a.imshow(image.data, interpolation='none', cmap=colormap)
            f.tight_layout(pad=1)

        elif image.type == 16:

            f.clf()
            a1 = f.add_subplot(211)
            a2 = f.add_subplot(212)

            x = numpy.linspace(1, 256, 256)

            a1.plot(x, image.data[0, :])
            a2.plot(x, image.data[1, :])
            a1.axis([1, 256, numpy.min(image.data[0, :]), numpy.max(image.data[0, :])])
            a2.axis([1, 256, numpy.min(image.data[1, :]), numpy.max(image.data[1, :])])
            f.tight_layout(pad=2)

        elif image.type == 32:

            f.clf()
            a = f.add_subplot(111)

            x = numpy.linspace(1, 16, 16)

            a.plot(x, image.data[0, :])
            f.tight_layout(pad=1)

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
