    outputForm = data(1);
    imageId = typecast(data(2:3), 'uint16');

    fileName = [int2str(imageId) '_' int2str(outputForm)];
    
    image = openFile(fileName);
    
    image.outputForm = outputForm;
    image.imageId = imageId;
    image.mode = data(4);
    image.treshold = typecast(data(5:6), 'uint16');
    image.bias = data(7);
    image.exposure = typecast(data(8:9), 'uint16');
    image.filtering = data(10);
    image.nonzeropixelsfiltered = typecast(data(11:12), 'uint16');
    image.nonzeropixelsoriginal = typecast(data(13:14), 'uint16');
    image.minValueOriginal = data(15);
    image.maxValueOriginal = data(16);
    image.minValueFiltered = data(17);
    image.maxValueFiltered = data(18);
    image.temperature = data(19);
    image.tempLimit = data(20);
    image.pxlLimit = typecast(data(21:22), 'uint16');
    image.uv1thr = typecast(data(23:24), 'uint16');

    image.chunkId = typecast(data(25:28), 'uint32');

    for u=1:7
        image.attitude(u) = int16(typecast(data(((u-1)*2+29):((u-1)*2+1+29)), 'int16'));
    end

    for u=1:3
        image.position(u) = int16(typecast(data(((u-1)*2+43):((u-1)*2+1+43)), 'int16'));
    end
    
    image.time = typecast(data(49:52), 'uint32');
    
    saveFile(image, fileName);
    
end
