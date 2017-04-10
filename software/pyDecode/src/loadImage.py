import pickle
from src.Image import Image
import os.path
from src.baseMethods import getFileName

def loadImage(image_id, image_type=0):

    # deduce the filename
    if image_type == 0:
        file_name = "images/"+str(image_id)
    else:
        file_name = getFileName(image_id, image_type)

    # ask OS to locate the file
    if os.path.isfile(file_name):

        # if the file exists, open it
        with open(file_name, 'rb') as input:
        
            # load the object from the file
            try:
                image = pickle.load(input)
            except:
                print("file \"{}\" is corrupted".format(file_name))
                return 0
    
            return image

    else:

        return 0
