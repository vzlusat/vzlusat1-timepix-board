import binascii
from src.parseMetadata import parseMetadata
from src.parseMetadata import parseBinning8
from src.parseMetadata import parseBinning16
from src.parseMetadata import parseBinning32
from src.parseMetadata import parseColsSums
from src.parseMetadata import parseRowsSums
from src.parseMetadata import parseEnergyHist
from src.parseMetadata import parseRaw

def parseInputFile(file_path):

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
                parseMetadata(bin_data[1:])

            elif data[0] == 'B':

                print("Parsing image (raw)")
                parseRaw(bin_data[1:])
                
            elif data[0] == 'D':

                print("Parsing image (binning-8)")
                parseBinning8(bin_data[1:])

            elif data[0] == 'E':

                print("Parsing image (binning-16)")
                parseBinning16(bin_data[1:])

            elif data[0] == 'F':

                print("Parsing image (binning-32)")
                parseBinning32(bin_data[1:])

            elif data[0] == 'h':

                print("Parsing image (rows summ)")
                parseRowsSums(bin_data[1:])

            elif data[0] == 'H':

                print("Parsing image (cols summ)")
                parseColsSums(bin_data[1:])

            elif data[0] == 'e':

                print("Parsing image (energy hist.)")
                parseEnergyHist(bin_data[1:])

            elif data[0] == 'H':

                print("Parsing house keeping")

            else:

                print("UNKNOWN packet");
                print(hex_data);

parseInputFile("input.txt")
