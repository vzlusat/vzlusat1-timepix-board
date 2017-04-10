def install_and_import(package):
    import importlib
    try:
        importlib.import_module(package)
    except ImportError:
        import pip
        pip.main(['install', package])
    finally:
        globals()[package] = importlib.import_module(package)


install_and_import('matplotlib')

matplotlib.use('TkAgg')

from numpy import arange, sin, pi
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
# implement the default mpl key bindings
from matplotlib.backend_bases import key_press_handler

from matplotlib.figure import Figure

import os
import numpy
from src.Image import Image 
from src.HouseKeeping import HouseKeeping 
from src.loadImage import loadImage
from src.loadHouseKeeping import loadHouseKeeping
from src.parseInputFile import parseInputFile

import sys
if sys.version_info[0] < 3:
    import Tkinter as Tk
    import tkFileDialog
else:
    import tkinter as Tk
    import tkinter.filedialog

import datetime
import matplotlib.patches as patches

if not os.path.exists("images"):
    os.makedirs("images")

if not os.path.exists("export"):
    os.makedirs("export")

root = Tk.Tk()
root.resizable(width=1, height=1)
root.geometry('{}x{}'.format(1300, 600))
root.wm_title("VZLUSAT-1 X-Ray data decoder")

# plot
f = Figure(facecolor='none')
f.clf()
frame_main = Tk.Frame(root);
frame_main.pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)

global v
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
text_labels = []
text_labels_var = []
for i in range(0, len(Image.metadata_labels)): #Rows
    text_labels_var.append(Tk.StringVar())
    text_labels.append(Tk.Label(frame_left_to_canvas, textvariable=text_labels_var[i]).grid(row=i, column=0, sticky=Tk.E))

    metadatas_var.append(Tk.StringVar())
    metadatas.append(Tk.Label(frame_left_to_canvas, textvariable=metadatas_var[i]).grid(row=i, column=1, sticky=Tk.W))

housekeeping_values = []
housekeeping_labels = []

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

a = f.add_subplot(111)
a.axes.get_xaxis().set_visible(False)
a.axes.get_yaxis().set_visible(False)
a.patch.set_visible(False)
a.axis('off')
canvas.show()

import re
numbers = re.compile(r'(\d+)')
def numericalSort(value):
    parts = numbers.split(value)
    parts[1::2] = map(int, parts[1::2])
    return parts

def loadFiles():

    # updated the filename list
    global file_names
    file_names = os.listdir("images")

    file_names = sorted(file_names, key=numericalSort)

    list_files = []

    # create the list of files for the listbox
    for file in file_names:

        if file[-5] == 'h':

            housekeeeping = loadHouseKeeping(file)

            if housekeeeping != 0:
               list_files.append(str(housekeeeping.images_taken)+"_"+str(housekeeeping.time_since_boot)+"s_hk") 
            else:
                print("could not open file "+file)

        else:

            image = loadImage(file)

            if image != 0:
               list_files.append(str(image.id)+"_"+str(image.type)) 
            else:
                print("could not open file "+file)

    v.set("All images loaded")

    return list_files

# preload and sort file names from "images" directories
global file_names
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

def showHouseKeeping(housekeeping):

    f.clf()
    a = f.add_subplot(111)
    a.text(0.5, 0.5, 'No data', horizontalalignment='center',verticalalignment='center', transform = a.transAxes)
    a.axes.get_xaxis().set_visible(False)
    a.axes.get_yaxis().set_visible(False)
    a.patch.set_visible(False)
    a.axis('off')

    canvas.show()

    # clear the text
    for i in range(0, len(Image.metadata_labels)):
        text_labels_var[i].set("")

    for i in range(0, len(Image.metadata_labels)):
        metadatas_var[i].set("")

    # fill the labels
    for i in range(0, len(HouseKeeping.housekeeping_labels)):
        text_labels_var[i].set(HouseKeeping.housekeeping_labels[i])

    metadatas_var[0].set(str(housekeeping.boot_count))
    metadatas_var[1].set(str(housekeeping.images_taken))
    metadatas_var[2].set(str(housekeeping.temperature)+" C")

    if housekeeping.fram_status == 1:
        fram_status = "OK"
    else:
        fram_status = "ERROR"

    metadatas_var[3].set(str(fram_status))

    if housekeeping.medipix_status == 1:
        medipix_status = "OK"
    else:
        medipix_status = "ERROR"

    metadatas_var[4].set(str(medipix_status))
    metadatas_var[5].set(str(housekeeping.time_since_boot)+" s")

    metadatas_var[6].set(str(housekeeping.TIR_max))
    metadatas_var[7].set(str(housekeeping.TIR_min))
    metadatas_var[8].set(str(housekeeping.IR_max))
    metadatas_var[9].set(str(housekeeping.IR_min))
    metadatas_var[10].set(str(housekeeping.UV1_max))
    metadatas_var[11].set(str(housekeeping.UV1_min))
    metadatas_var[12].set(str(housekeeping.UV2_max))
    metadatas_var[13].set(str(housekeeping.UV2_min))
    metadatas_var[14].set(str(housekeeping.temp_max))
    metadatas_var[15].set(str(housekeeping.temp_min))

def showImage(image):

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

    if image.got_metadata == 1:

        if image.mode == 0:
            mode = "Medipix (Counting)"
        else:
            mode = "Timepix (Energy)"

        metadatas_var[2].set(mode)

        metadatas_var[3].set(image.threshold)
        metadatas_var[4].set(image.bias)

        exposure = image.exposure

        if image.exposure <= 60000:
            exposure = image.exposure*0.001
        else:
            exposure = 60 + image.exposure%60000

        metadatas_var[5].set("{0:3.3f} s".format(exposure))

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

        human_readible_time = datetime.datetime.fromtimestamp(image.time).strftime('%Y-%m-%d %H:%M:%S')
        metadatas_var[19].set(human_readible_time)

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

        for i in range(0, len(Image.metadata_labels)):
            text_labels_var[i].set(Image.metadata_labels[i])

    if image.got_data == 1:

        if image.type >= 1 and image.type <= 8:

            # plot the image
            f.clf()
            a = f.add_subplot(111)

            a.set_xlabel("Column [-]")
            a.set_ylabel("Row [-]")

            if image.got_metadata == 1:
                a.set_title(img_type+" n.{0}, {1} s exposure, ".format(image.id, exposure)+mode+" mode", fontsize=13, y=1.02)
            else:
                a.set_title(img_type+" n.{0}, ??? s exposure, ".format(image.id)+"??? mode", fontsize=13, y=1.02)

            cax = a.imshow(image.data, interpolation='none', cmap=colormap)
            cbar = f.colorbar(cax)

            cbar.ax.get_yaxis().labelpad = 20
            if image.type == 1:
                if image.mode == 0:
                    cbar.ax.set_ylabel('[counts]', rotation=270)
                else:
                    cbar.ax.set_ylabel('[keV]', rotation=270)
            else:
                cbar.ax.set_ylabel('[active pixel in the bin]', rotation=270)

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

            if image.got_metadata == 1:
                a1.set_title("Row summs n.{0}, {1} s exposure ".format(image.id, exposure), fontsize=13, y=1.02)
            else:
                a1.set_title("Row summs n.{0}, ??? s exposure ".format(image.id), fontsize=13, y=1.02)
            a1.set_xlabel("Row [-]")
            a1.set_ylabel("Active pixel count [-]")

            if image.got_metadata == 1:
                a2.set_title("Column summs n.{0}, {1} s exposure ".format(image.id, exposure), fontsize=13, y=1.02)
            else:
                a2.set_title("Column summs n.{0}, ??? s exposure ".format(image.id), fontsize=13, y=1.02)
            a2.set_xlabel("Column [-]")
            a2.set_ylabel("Active pixel count [-]")

            f.tight_layout(pad=2)

        elif image.type == 32:

            f.clf()
            a = f.add_subplot(111)

            x = [2.9807, 4.2275, 6.4308, 10.3875, 16.6394, 24.7081, 33.7833, 43.3679, 53.2233, 63.2344, 73.3415, 83.5115, 93.7248, 103.9691, 114.2361, 124.5204, 134.8182]
            
            for i in range(0, 16):

               # rectangle('Position', [x(i), 0, x(i+1)-x(i), image.data(i)], 'FaceColor', [0 0.5 0.5], 'EdgeColor', 'b','LineWidth',1);
               a.add_patch(
                   patches.Rectangle(
                       (x[i], 0),            # (x,y)
                       x[i+1]-x[i],          # width
                       image.data[0, i],                  # height
                   )
               )

            # a.plot(x, image.data[0, :])

            a.relim()
            a.autoscale_view()

            if image.mode == 0:
                a.set_xlabel("Particle counts [-]")
            else:
                a.set_xlabel("Energy [keV]")

            a.set_ylabel("Counts [-]")

            if image.got_metadata == 1:
                a.set_title("Image histogram n.{0}, {1} s exposure, ".format(image.id, exposure)+mode+" mode", fontsize=13, y=1.02)
            else:
                a.set_title("Image histogram n.{0}, ??? s exposure, ".format(image.id)+"??? mode", fontsize=13, y=1.02)

            f.tight_layout(pad=1)

    else:

        f.clf()
        a = f.add_subplot(111)
        a.text(0.5, 0.5, 'No data', horizontalalignment='center',verticalalignment='center', transform = a.transAxes)
        a.axes.get_xaxis().set_visible(False)
        a.axes.get_yaxis().set_visible(False)
        a.patch.set_visible(False)
        a.axis('off')

    canvas.show()

# callback function for showing an image after clicking the listbox
def onselect(evt):

    global file_names

    w = evt.widget

    if len(w.curselection()) == 0:
        return

    index = int(w.curselection()[0])
    value = w.get(index)

    file_name = file_names[index]
    if file_name[-5] == 'h':
        housekeeping = loadHouseKeeping(file_names[index])
        showHouseKeeping(housekeeping)
    else:
        image = loadImage(file_names[index])
        showImage(image)

# bind onselect() callback function to listbox, so we can 
# show images after clicking on their name
listbox.bind('<<ListboxSelect>>', onselect)

# listbox.selection_clear()
listbox.after(10, lambda: listbox.focus_force())
listbox.after(10, lambda: listbox.selection_set("end"))
listbox.after(10, lambda: listbox.see(Tk.END))

# autoselect the last item in the listbox after start
if len(file_names) > 0:
    file_name = file_names[-1]
    if file_name[-5] == 'h':
        print "HK selected"
    else:
        image = loadImage(file_names[-1])
        showImage(image)

# really we want the scrollabar to be down
listbox.after(100, lambda: listbox.see(Tk.END))

# quit button callback
def _quit():
    root.quit()   
    root.destroy()
 
# spawn quit button
button = Tk.Button(master=frame_left1, text='Quit', command=_quit)
button.pack(side=Tk.BOTTOM)

# callback for loading new images from a text file
def _loadNewImages():

    if sys.version_info[0] < 3:
        file_name = tkFileDialog.askopenfilename()
    else:
        file_name = tkinter.filedialog.askopenfilename()

    if file_name == "":
        return

    print("Openning file \"{}\"".format(file_name))
    parseInputFile(file_name, v, root)

    list_files = loadFiles()

    listbox.delete(0, Tk.END)

    for item in list_files:
        listbox.insert(Tk.END, item)

    # listbox.selection_clear()
    listbox.after(10, lambda: listbox.selection_set("end"))
    listbox.after(10, lambda: listbox.see(Tk.END))

    # autoselect the last item in the listbox after start
    if len(file_names) > 0:
        file_name = file_names[-1]
        if file_name[-5] == 'h':
            print "HK selected"
        else:
            image = loadImage(file_names[-1])
            showImage(image)

# spawn button for loading new images
load_button = Tk.Button(master=frame_list, text='Load new images', command=_loadNewImages)
load_button.pack(side=Tk.TOP)

# callback for detecting keypresses
def on_key_event(event):
    if event.key == 'q' or event.key == 'esc':
        _quit()

canvas.mpl_connect('key_press_event', on_key_event)

Tk.mainloop()
# If you put root.destroy() here, it will cause an error if
# the window is closed with the window manager.
