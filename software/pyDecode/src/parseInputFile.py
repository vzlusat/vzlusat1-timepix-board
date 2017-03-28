import binascii
from src.parseMethods import parseMetadata
from src.parseMethods import parseBinning8
from src.parseMethods import parseBinning16
from src.parseMethods import parseBinning32
from src.parseMethods import parseColsSums
from src.parseMethods import parseRowsSums
from src.parseMethods import parseEnergyHist
from src.parseMethods import parseRaw

import sys
if sys.version_info[0] < 3:
    import Tkinter as Tk
else:
    import tkinter as Tk

def parseInputFile(file_path, v, root):

    infile = open(file_path, "r")

    # for all lines in the file 
    for line in infile:

        # if the line contains word "data"
        if line.find("data") > -1:

            # select the part with the data
            hex_data = line[7:-1]

            # convert to binary
            data = binascii.unhexlify(hex_data)
            bin_data = [ord(new_int) for new_int in data]

            if data[0] == 'A':

                print("Parsing metadata")
                v.set("Parsing metadata")
                root.update()
                parseMetadata(bin_data[1:])

            elif data[0] == 'B':

                print("Parsing image (raw)")
                v.set("Parsing image (raw)")
                root.update()
                parseRaw(bin_data[1:])
                
            elif data[0] == 'D':

                print("Parsing image (binning-8)")
                v.set("Parsing image (binning-8)")
                root.update()
                parseBinning8(bin_data[1:])

            elif data[0] == 'E':

                print("Parsing image (binning-16)")
                v.set("Parsing image (binning-16)")
                root.update()
                parseBinning16(bin_data[1:])

            elif data[0] == 'F':

                print("Parsing image (binning-32)")
                v.set("Parsing image (binning-32)")
                root.update()
                parseBinning32(bin_data[1:])

            elif data[0] == 'h':

                print("Parsing image (rows summ)")
                v.set("Parsing image (rows summ)")
                root.update()
                parseRowsSums(bin_data[1:])

            elif data[0] == 'H':

                print("Parsing image (cols summ)")
                v.set("Parsing image (cols summ)")
                root.update()
                parseColsSums(bin_data[1:])

            elif data[0] == 'e':

                print("Parsing image (energy hist.)")
                v.set("Parsing image (energy hist.)")
                root.update()
                parseEnergyHist(bin_data[1:])

            elif data[0] == 'H':

                print("Parsing house keeping")
                v.set("Parsing house keeping")
                root.update()

            else:

                print("UNKNOWN packet")
                print(hex_data)
