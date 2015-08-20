function [] = parseRaw(data)

    imageId = typecast(data(1:2), 'uint16');
    packetId = uint16(data(3));
    fileName = [int2str(imageId) '_1'];
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 256) || (size(image.data, 2) ~= 256))
       image.data = zeros(256, 256); 
    end
    
    data = data(4:end);
    
    for i=1:3:length(data)
        idx = typecast(data(i:i+1), 'uint16');
        image.data(floor((idx/256))+1, mod(idx, 256)+1) = uint16(data(i+2));
    end
    
    image.outputForm = 1;
    
    saveFile(image, fileName);
    
end