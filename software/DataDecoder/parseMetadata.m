function [] = parseMetadata(data)

    imageId = typecast(data(1:2), 'uint16');
    
    image.imageId = imageId;
    image.mode = data(3);
    image.treshold = typecast(data(4:5), 'uint16');
    image.bias = data(6);
    image.exposure = typecast(data(7:8), 'uint16');
    image.filtering = data(9);
    image.outputForm = data(10);
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

    fileName = [int2str(imageId) '_2'];
    
    image = openFile(fileName);
    
    saveFile(image, fileName);
    
end