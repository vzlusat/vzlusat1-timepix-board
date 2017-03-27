import binascii
from src.parseMetadata import parseMetadata

def parseInputFile(file_path):

    infile = open(file_path, "r")

    # for all lines in the file 
    for line in infile:

        # if the line contains word "data"
        if line.find("data") > -1:

            # select the part with the data
            hex_data = line[7:-1]

            # convert to binary
            bin_data = binascii.unhexlify(hex_data)

            if bin_data[0] == 'A':

                print("Parsing metadata")
                parseMetadata(bin_data[1:])

            elif bin_data[0] == 'B':

                print("Parsing image (raw)")
                
            elif bin_data[0] == 'D':

                print("Parsing image (binning-32)")

            elif bin_data[0] == 'E':

                print("Parsing image (binning-16)")

            elif bin_data[0] == 'F':

                print("Parsing image (binning-32)")

            elif bin_data[0] == 'h':

                print("Parsing image (rows summ)")

            elif bin_data[0] == 'H':

                print("Parsing image (cols summ)")

            elif bin_data[0] == 'e':

                print("Parsing image (energy hist.)")

            elif bin_data[0] == 'H':

                print("Parsing house keeping")

            else:

                print("UNKNOWN packet");
                print(hex_data);

parseInputFile("input.txt")
