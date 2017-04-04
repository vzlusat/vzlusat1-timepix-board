import pickle
from src.HouseKeeping import HouseKeeping
import os.path
from src.baseMethods import getHkFileName

def loadHouseKeeping(images_taken, time_since_boot=0):

    # deduce the filename
    if time_since_boot == 0:
        file_name = "images/"+images_taken
    else:
        file_name = getHkFileName(images_taken, time_since_boot)

    # ask OS to locate the file
    if os.path.isfile(file_name):

        # if the file exists, open it
        with open(file_name, 'rb') as input:
        
            # load the object from the file
            try:
                housekeeping = pickle.load(input)
            except:
                print "file \"{}\" is corrupted".format(file_name)
                return 0
    
            return housekeeping

    else:

        return 0
