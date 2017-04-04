import numpy
import math
from src.Image import Image 
from src.loadImage import loadImage
from src.saveImage import saveImage
from src.saveHouseKeeping import saveHouseKeeping
from src.numberConversion import bytesToInt16
from src.numberConversion import bytesToInt32
from src.calibration import *
from src.exportMethods import exportImage
from src.HouseKeeping import HouseKeeping

def parseHouseKeeping(bin_data):

    new_hk = HouseKeeping()

    new_hk.boot_count = numpy.uint16(bytesToInt16(bin_data[1], bin_data[0]))
    new_hk.images_taken = numpy.uint16(bytesToInt16(bin_data[3], bin_data[2]))
    new_hk.temperature = bin_data[4]
    new_hk.fram_status = bin_data[5]
    new_hk.medipix_status = bin_data[6]
    new_hk.time_since_boot = numpy.uint32(bytesToInt32(bin_data[10], bin_data[9], bin_data[8], bin_data[8]))
    new_hk.TIR_max = numpy.int16(bytesToInt16(bin_data[12], bin_data[11]))
    new_hk.TIR_min = numpy.int16(bytesToInt16(bin_data[14], bin_data[13]))
    new_hk.IR_max = numpy.int16(bytesToInt16(bin_data[16], bin_data[15]))
    new_hk.IR_min = numpy.int16(bytesToInt16(bin_data[18], bin_data[17]))
    new_hk.UV1_max = numpy.int16(bytesToInt16(bin_data[20], bin_data[19]))
    new_hk.UV1_min = numpy.int16(bytesToInt16(bin_data[22], bin_data[21]))
    new_hk.UV2_max = numpy.int16(bytesToInt16(bin_data[24], bin_data[23]))
    new_hk.UV2_min = numpy.int16(bytesToInt16(bin_data[26], bin_data[25]))
    new_hk.temp_max = bin_data[27]
    new_hk.temp_min = bin_data[28]

    saveHouseKeeping(new_hk);

    return new_hk

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
        image.attitude[x] = numpy.int16(bytesToInt16(bin_data[28+x*2], bin_data[29+x*2]))

    for x in range(0, 3):
        image.position[x] = numpy.int16(bytesToInt16(bin_data[42+x*2], bin_data[43+x*2]))

    image.time = bytesToInt32(bin_data[48], bin_data[49], bin_data[50], bin_data[51])

    image.got_metadata = 1

    saveImage(image)
    
    exportImage(image)

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
    
    exportImage(image)

def parseBinning16(bin_data):

    image = parseImageHeader(bin_data, 4)

    packet_id = bin_data[2]

    if (image.data.shape[0] != 16) or (image.data.shape[1] != 16):
        image.data = numpy.ones(shape=[16, 16]) * -1        

    image_reshaped = image.data.reshape((1, 16*16))

    image_reshaped[:, (packet_id*64):((packet_id+1)*64)] = bin_data[3:67]
    
    image.data = image_reshaped.reshape((16, 16))
    
    image.type = 4

    image.got_data = 1

    saveImage(image)
    
    exportImage(image)

def parseBinning32(bin_data):

    image = parseImageHeader(bin_data, 8)

    packet_id = bin_data[2]

    if (image.data.shape[0] != 8) or (image.data.shape[1] != 8):
        image.data = numpy.ones(shape=[8, 8]) * -1        

    image_reshaped = image.data.reshape((1, 8*8))

    image_reshaped[:, (packet_id*64):((packet_id+1)*64)] = bin_data[3:67]
    
    image.data = image_reshaped.reshape((8, 8))
    
    image.type = 8

    image.got_data = 1

    saveImage(image)

    exportImage(image)

def parseColsSums(bin_data):

    image = parseImageHeader(bin_data, 16)

    packet_id = bin_data[2]

    if (image.data.shape[0] != 2) or (image.data.shape[1] != 256):
        image.data = numpy.ones(shape=[2, 256]) * -1        

    image.data[1, (packet_id*64):((packet_id+1)*64)] = bin_data[3:67]
    
    image.type = 16

    image.got_data = 1

    saveImage(image)
    
    exportImage(image)
    
def parseRowsSums(bin_data):

    image = parseImageHeader(bin_data, 16)

    packet_id = bin_data[2]

    if (image.data.shape[0] != 2) or (image.data.shape[1] != 256):
        image.data = numpy.ones(shape=[2, 256]) * -1        

    image.data[0, (packet_id*64):((packet_id+1)*64)] = bin_data[3:67]
    
    image.type = 16

    image.got_data = 1

    saveImage(image)
    
    exportImage(image)

def parseEnergyHist(bin_data):

    image = parseImageHeader(bin_data, 32)

    if (image.data.shape[0] != 1) or (image.data.shape[1] != 16):
        image.data = numpy.ones(shape=[1, 16])

    for i in range(0, 16):
        image.data[0, i] = bytesToInt16(bin_data[2 + 2*i], bin_data[3 + 2*i])
    
    image.type = 32

    image.got_data = 1

    saveImage(image)
    
    exportImage(image)

def parseRaw(bin_data):

    image = parseImageHeader(bin_data, 1)

    if image.data.shape[0] != 256 or image.data.shape[1] != 256:
        image.data = numpy.zeros((256, 256))

    payload = bin_data[3:]

    i = 0
    while i < (len(payload)-2):
        idx = bytesToInt16(payload[i], payload[i+1])
        newx = int(math.floor(idx/256))
        newy = idx%256

        calibrated_pixel = payload[i+2]

        if image.mode == 1:
            calibrated_pixel = (calibration_t[0, idx]*calibration_a[0, idx] + payload[i+2] - calibration_b[0, idx] + math.sqrt(math.pow(calibration_b[0, idx] + calibration_t[0, idx]*calibration_a[0, idx] - payload[i+2], 2) + 4*calibration_a[0, idx]*calibration_c[0, idx]))/(2*calibration_a[0, idx]);

        if newx > 255 or newx < 0 or newy > 255 or newy < 0:
            print("Index out of bounds: {0}, {1}".format(idx))
        else:
            image.data[newx, newy] = calibrated_pixel

        i += 3

    image.type = 1

    image.got_data = 1

    saveImage(image)
    
    exportImage(image)
