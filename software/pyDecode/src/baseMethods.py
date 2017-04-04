def getFileName(image_id, image_type):

    return "images/"+str(image_id)+"_{0:02d}.pkl".format(image_type) 

def getHkFileName(images_taken, time_since_boot):

    return "housekeeping/"+str(images_taken)+"_{0:05d}.pkl".format(time_since_boot) 

def getExportDataName(image_id, image_type):

    return "export/"+str(image_id)+"_{0:02d}.txt".format(image_type) 

def getExportMetadataName(image_id, image_type):

    return "export/"+str(image_id)+"_{0:02d}.metadata.txt".format(image_type) 
