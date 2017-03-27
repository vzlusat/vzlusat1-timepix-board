import struct
from src.Image import Image 
from src.loadImage import loadImage
from src.saveImage import saveImage

def parseMetadata(bin_data):

    output_form = bin_data[0]

    imageid = ord(bin_data[2])<<8 | ord(bin_data[1])

    image = loadImage(imageid) 

    if image == 0:
        
        print("creating new image")
        image = Image(imageid)
        saveImage(image);



