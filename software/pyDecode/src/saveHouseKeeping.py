import pickle
from src.HouseKeeping import HouseKeeping
from src.baseMethods import getHkFileName

def saveHouseKeeping(housekeeping):

    # deduce the file name
    file_name = getHkFileName(housekeeping.images_taken, housekeeping.time_since_boot)

    # try to open the file
    with open(file_name, 'wb') as output:
    
        # load the object with pickle
        pickle.dump(housekeeping, output, pickle.HIGHEST_PROTOCOL)
