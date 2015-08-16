% Energy histogram
if (image.outputForm == 6)

     image.data = zeros(1, 16);
    
     % wait for next packet    
     while (s.BytesAvailable <= 0)
     end
     
     packetType = fread(s, 1, 'uchar');
     imageId = fread(s, 1, 'uint16');
     
     for i=1:16
        image.data(1, i) = fread(s, 1, 'uint16');
     end
     
     % send ack      
     fprintf(s, '%c', 'h');
         
    figure(6);
    title('Energy histogram');
    bar(image.data);
     
% Histograms 
elseif (image.outputForm == 5)

    image.data = zeros(2, 256);

    while true

        % wait for next packet    
        while (s.BytesAvailable <= 0)
        end

        packetType = fread(s, 1, 'uchar');
        imageId = fread(s, 1, 'uint16');
        packetId = fread(s, 1, 'uchar');

        for i=1:64

            % the first histogram         
            if (packetType == 'h')

                image.data(1, packetId*64 + i) = fread(s, 1, 'uchar');

            elseif (packetType == 'H')

                image.data(2, packetId*64 + i) = fread(s, 1, 'uchar');

            end

        end

        fread(s, 1, 'uint16');

        fprintf(s, '%c', 'h');

        if (packetType == 'H' && packetId == 3)
            break;
        end

    end
    
    figure(5);
    subplot(2, 1, 1);
    title('Line histogram');
    plot(image.data(1, :)');
    axis auto
    subplot(2, 1, 2);
    title('Row histogram');
    plot(image.data(2, :)');
    axis auto
    
else
   
    if (image.outputForm == 2)
       
        numPerLine = 32; 
        numerOfPackets = 16;
        figure(2);
       
    elseif (image.outputForm == 3)
        
        numPerLine = 16;
        numerOfPackets = 4;
        figure(3);
        
    elseif (image.outputForm == 4)
        
        numPerLine = 8;
        numerOfPackets = 1;
        figure(4);
    
    end
    
    image.data = zeros(numPerLine, numPerLine);
           
    bytesReceived = 0;
    
    for i=1:numerOfPackets
        
        % wait for next packet    
        while (s.BytesAvailable <= 0)
        end

        packetType = fread(s, 1, 'uchar');
        imageId = fread(s, 1, 'uint16');
        packetId = fread(s, 1, 'uchar');
        
        % read bytes in the packet         
        for j=1:64
            
            image.data(floor(bytesReceived/numPerLine)+1, mod(bytesReceived, numPerLine)+1) = fread(s, 1, 'uchar');
            bytesReceived = bytesReceived + 1;
            
        end
        
        fread(s, 1, 'uint16');

        fprintf(s, '%c', 'h');
        
    end
    imagesc(image.data);
    axis equal;
    axis tight;
    colorbar;
    colormap(hot)
    
end