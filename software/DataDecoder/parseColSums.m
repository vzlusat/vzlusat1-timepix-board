function [] = parseColSums(data)

    imageId = typecast(data(1:2), 'uint16');
    packetId = uint16(data(3));
    fileName = [int2str(imageId) '_16'];
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 2) || (size(image.data, 2) ~= 256))
       image.data = zeros(2, 256); 
    end
    
    image.data(2, (packetId*64+1):((packetId+1)*64)) = data(4:67);
    
    saveFile(image, fileName);
    
end