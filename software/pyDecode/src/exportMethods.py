import numpy
import math
from src.Image import Image 
from src.baseMethods import getExportDataName
from src.baseMethods import getExportMetadataName
import datetime
import csv

def exportMetadata(image):

    if image.got_metadata == 0:
        return

    metadatas_array = [None] * 21;

    with open(getExportMetadataName(image.id, image.type), "w") as metadata_file:

        metadatas_array[0] = str(image.id)

        if image.type == 1:
            img_type = "RAW"
        elif image.type == 2:
            img_type = "Binning 8"
        elif image.type == 4:
            img_type = "Binning 16"
        elif image.type == 8:
            img_type = "Binning 32"
        elif image.type == 16:
            img_type = "Sums"
        elif image.type == 32:
            img_type = "Energy histogram"

        metadatas_array[1] = str(img_type)

        if image.mode == 0:
            mode = "Medipix (Counting)"
        else:
            mode = "Timepix (Energy)"

        metadatas_array[2] = str(mode)
        metadatas_array[3] = str(image.threshold)
        metadatas_array[4] = str(image.bias)

        exposure = image.exposure

        if image.exposure <= 60000:
            exposure = image.exposure*0.001
        else:
            exposure = 60 + image.exposure%60000

        metadatas_array[5] = "{0:3.3f}".format(exposure)

        metadatas_array[6] = str(image.filtering)
        metadatas_array[7] = str(image.filtered_pixels)
        metadatas_array[8] = str(image.original_pixels)
        metadatas_array[9] = str(image.min_original)
        metadatas_array[10] = str(image.max_original)
        metadatas_array[11] = str(image.min_filtered)
        metadatas_array[12] = str(image.max_filtered)
        metadatas_array[13] = str(image.temperature)
        metadatas_array[14] = str(image.temp_limit)
        metadatas_array[15] = str(image.pxl_limit)
        metadatas_array[16] = str(image.uv1_thr)

        attitude = ""
        for att in image.attitude:
            attitude += str(att)+" "

        metadatas_array[17] = str(attitude)

        position = ""
        for pos in image.position:
            position += str(pos)+" "

        metadatas_array[18] = str(position)

        metadatas_array[19] = str(image.time)

        if image.type == 2:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+15)
        elif image.type == 4:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+3)
        elif image.type == 8:
            chunk_id = str(image.chunk_id)
        elif image.type == 16:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+7)
        elif image.type == 32:
            chunk_id = str(image.chunk_id)
        elif image.type == 1:
            chunk_id = str(image.chunk_id)+" to "+str(image.chunk_id+int(numpy.floor(image.filtered_pixels/20)))

        metadatas_array[20] = str(chunk_id)

        for i in range(0, 21):

            metadata_file.write(image.metadata_labels[i]+" "+metadatas_array[i])

            if i < 20:

                metadata_file.write("\n")

def exportBinning(image):

    if image.got_data == 1:
        if image.type == 2:
            size = 32
        elif image.type == 4:
            size = 16
        elif image.type == 8:
            size = 8
        
        with open(getExportDataName(image.id, image.type), "w") as data_file:

            writer = csv.writer(data_file, quoting=csv.QUOTE_NONE)
        
            for i in range(0, size):
                writer.writerow(['{0:d}'.format(math.trunc(x)) for x in image.data[i, :]])

    exportMetadata(image)

def exportSums(image):

    if image.got_data == 1:
        with open(getExportDataName(image.id, image.type), "w") as data_file:

            writer = csv.writer(data_file, quoting=csv.QUOTE_NONE)
        
            writer.writerow(['{0:d}'.format(math.trunc(x)) for x in image.data[0, :]])
            writer.writerow(['{0:d}'.format(math.trunc(x)) for x in image.data[1, :]])

    exportMetadata(image)

def exportHistogram(image):

    if image.got_data == 1:
        with open(getExportDataName(image.id, image.type), "w") as data_file:

            writer = csv.writer(data_file, quoting=csv.QUOTE_NONE)
        
            writer.writerow(['{0:d}'.format(math.trunc(x)) for x in image.data[0, :]])
        
    exportMetadata(image)

def exportRaw(image):

    if image.got_data == 1:
        with open(getExportDataName(image.id, image.type), "w") as data_file:

            writer = csv.writer(data_file, quoting=csv.QUOTE_NONE)
        
            for i in range(0, 256):
                writer.writerow(["{0:.2f}".format(x) for x in image.data[i, :]])

    exportMetadata(image)

def exportImage(image):

    if image.type == 1:

        exportRaw(image)

    if image.type >= 2 and image.type <= 8:

        exportBinning(image)

    elif image.type == 16:

        exportSums(image)

    elif image.type == 32:

        exportHistogram(image)
