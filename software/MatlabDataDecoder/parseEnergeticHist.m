function [] = parseEnergeticHist(data)

    imageId = typecast(data(1:2), 'uint16');
    fileName = [int2str(imageId) '_32'];
    data = data(3:end);
    
    image = openFile(fileName);
    
    if ((size(image.data, 1) ~= 1) || (size(image.data, 2) ~= 16))
       image.data = -ones(1, 16); 
    end
    
    for i=1:16
        image.data(1, i) = typecast(data((i*2-1):(i*2)), 'uint16');
    end
    
    image.outputForm = 32;
        
    saveFile(image, fileName);
    
end
