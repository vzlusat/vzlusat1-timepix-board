import pickle
from src.Image import Image
import os.path
from src.baseMethods import getFileName

def loadImage(image_id, image_type):

    # deduce the filename
    file_name = getFileName(image_id, image_type)

    # ask OS to locate the file
    if os.path.isfile(file_name):

        # if the file exists, open it
        with open(file_name, 'rb') as input:
        
            # load the object from the file
            image = pickle.load(input)
    
            return image

    else:

        return 0
