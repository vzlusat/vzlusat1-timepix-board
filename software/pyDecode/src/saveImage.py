import pickle
from src.Image import Image

def saveImage(image):

    # deduce the file name
    file_name = "images/"+str(image.id)+".pkl"

    with open(file_name, 'wb') as output:
    
        pickle.dump(image, output, pickle.HIGHEST_PROTOCOL)
