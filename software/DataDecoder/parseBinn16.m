function [] = parseBinn16(data)

    imageId = typecast(data(1:2), 'uint16');
    packetId = uint16(data(3));
    fileName = [int2str(imageId) '_4'];
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 16) || (size(image.data, 2) ~= 16))
       image.data = zeros(16, 16); 
    end
    
    image_reshaped = reshape(image.data', 1, []);
    
    image_reshaped((packetId*64+1):((packetId+1)*64)) = data(4:67);
    
    image.data = reshape(image_reshaped, 16, [])';
    
    image.outputForm = 4;
    
    saveFile(image, fileName);
    
end

