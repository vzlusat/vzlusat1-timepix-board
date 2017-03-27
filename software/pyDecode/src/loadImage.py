import pickle
from src.Image import Image
import os.path

def loadImage(image_name):

    file_name = "images/"+str(image_name)+".pkl"

    if os.path.isfile(file_name):

        with open(file_name, 'rb') as input:
        
            image = pickle.load(input)
    
            return image

    else:

        return 0
