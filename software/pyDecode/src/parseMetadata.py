import numpy
from src.Image import Image 
from src.loadImage import loadImage
from src.saveImage import saveImage
from src.numberConversion import bytesToInt16
from src.numberConversion import bytesToInt32

def parseImageHeader(bin_data, image_type):

    image_id = bytesToInt16(bin_data[0], bin_data[1])
    packet_id = bin_data[2]

    # try to load already saved image
    image = loadImage(image_id, image_type) 

    if image == 0: # if the image file does not exist

        # instantiate new image object
        image = Image(image_id, image_type)

        # save it to its apropriate file
        saveImage(image);

    return image

def parseMetadata(bin_data):

    image_type = bin_data[0]
    image_id = bytesToInt16(bin_data[1], bin_data[2])

    # try to load already saved image
    image = loadImage(image_id, image_type) 

    if image == 0: # if the image file does not exist

        # instantiate new image object
        image = Image(image_id, image_type)

        # save it to its apropriate file
        saveImage(image);

    # fill all the particular image properties

    image.mode = bin_data[3]
    image.threshold = bytesToInt16(bin_data[4], bin_data[5])
    image.bias = bin_data[6]
    image.exposure = bytesToInt16(bin_data[7], bin_data[8])
    image.filtering = bin_data[9]
    image.filtered_pixels = bytesToInt16(bin_data[10], bin_data[11])
    image.original_pixels = bytesToInt16(bin_data[12], bin_data[13])
    image.min_original = bin_data[14]
    image.max_original = bin_data[15]
    image.min_filtered = bin_data[16]
    image.max_filtered = bin_data[17]
    image.temperature = bin_data[18]
    image.temp_limit = bin_data[19]
    image.pxl_limit = bytesToInt16(bin_data[20], bin_data[21])
    image.uv1_thr = bytesToInt16(bin_data[22], bin_data[23])
    image.chunk_id = bytesToInt32(bin_data[24], bin_data[25], bin_data[26], bin_data[27])

    for x in range(0, 7):
        image.attitude[x] = bytesToInt16(bin_data[28+x*2], bin_data[29+x*2])

    for x in range(0, 3):
        image.position[x] = bytesToInt16(bin_data[42+x*2], bin_data[43+x*2])

    image.time = bytesToInt32(bin_data[48], bin_data[49], bin_data[50], bin_data[51])

    image.got_metadata = 1

    saveImage(image)

def parseBinning8(bin_data):

    image = parseImageHeader(bin_data, 2)

    packet_id = bin_data[2]

    if (image.data.shape[0] != 32) or (image.data.shape[1] != 32):
        image.data = numpy.ones(shape=[32, 32]) * -1        

    image_reshaped = image.data.reshape((1, 32*32))

    image_reshaped[:, (packet_id*64):((packet_id+1)*64)] = bin_data[3:67]
    
    image.data = image_reshaped.reshape((32, 32))
    
    image.type = 2

    image.got_data = 1

    saveImage(image)

