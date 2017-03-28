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

                v.set("Parsing metadata")
                root.update()
                parseMetadata(bin_data[1:])

            elif data[0] == 'B':

                v.set("Parsing image (raw)")
                root.update()
                parseRaw(bin_data[1:])
                
            elif data[0] == 'D':

                v.set("Parsing image (binning-8)")
                root.update()
                parseBinning8(bin_data[1:])

            elif data[0] == 'E':

                v.set("Parsing image (binning-16)")
                root.update()
                parseBinning16(bin_data[1:])

            elif data[0] == 'F':

                v.set("Parsing image (binning-32)")
                root.update()
                parseBinning32(bin_data[1:])

            elif data[0] == 'h':

                v.set("Parsing image (rows summ)")
                root.update()
                parseRowsSums(bin_data[1:])

            elif data[0] == 'H':

                v.set("Parsing image (cols summ)")
                root.update()
                parseColsSums(bin_data[1:])

            elif data[0] == 'e':

                v.set("Parsing image (energy hist.)")
                root.update()
                parseEnergyHist(bin_data[1:])

            elif data[0] == 'H':

                v.set("Parsing house keeping")
                root.update()

            else:

                print("UNKNOWN packet")
                print(hex_data)
