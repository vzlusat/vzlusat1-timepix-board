def getFileName(image_id, image_type):

    return "images/"+str(image_id)+"_{0:02d}.pkl".format(image_type) 

def getHkFileName(images_taken, time_since_boot):

    return "images/"+str(images_taken)+"_99_{0:05d}_h.pkl".format(time_since_boot) 

def getExportDataName(image_id, image_type):

    return "export/"+str(image_id)+"_{0:02d}.txt".format(image_type) 

def getExportMetadataName(image_id, image_type):

    return "export/"+str(image_id)+"_{0:02d}.metadata.txt".format(image_type) 
