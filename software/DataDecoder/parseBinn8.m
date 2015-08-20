function [] = parseBinn8(data)

    imageId = typecast(data(1:2), 'uint16');
    packetId = uint16(data(3));
    fileName = [int2str(imageId) '_2'];
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 32) || (size(image.data, 2) ~= 32))
       image.data = zeros(32, 32); 
    end
    
    image_reshaped = reshape(image.data', 1, []);
    
    image_reshaped((packetId*64+1):((packetId+1)*64)) = data(4:67);
    
    image.data = reshape(image_reshaped, 32, [])';
    
    image.outputForm = 2;
    
    saveFile(image, fileName);
    
end