import pickle
from src.Image import Image
from src.baseMethods import getFileName

def saveImage(image):

    # deduce the file name
    file_name = getFileName(image.id, image.type)

    # try to open the file
    with open(file_name, 'wb') as output:
    
        # load the object with pickle
        pickle.dump(image, output, pickle.HIGHEST_PROTOCOL)
