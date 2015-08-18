function [] = parseBinn32(data)

    imageId = typecast(data(1:2), 'uint16');
    packetId = uint16(data(3));
    fileName = [int2str(imageId) '_8'];
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 8) || (size(image.data, 2) ~= 8))
       image.data = zeros(8, 8); 
    end
    
    image_reshaped = reshape(image.data', 1, []);
    
    image_reshaped((packetId*64+1):((packetId+1)*64)) = data(4:67);
    
    image.data = reshape(image_reshaped, 8, [])';
    
    saveFile(image, fileName);
    
end

