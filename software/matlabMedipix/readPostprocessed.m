if exist('s')
    fclose(s);
    delete(s);
    clear s;
end

clear all

s = serial('COM29');
s.BaudRate = 230500;
s.BytesAvailableFcnMode = 'terminator';
s.Terminator = 'CR/LF';
s.DataBits = 8;
s.Parity = 'none';
s.StopBits = 1;
s.ReadAsyncMode = 'continuous';

fopen(s);

% pozadej o mereni    
fprintf(s, '%c', 't');
    
% wait for last line data       
while (s.BytesAvailable <= 0)
end

packetType = fread(s, 1, 'uchar')
imageId =  fread(s, 1, 'uint16')
mode = fread(s, 1, 'uchar')
treshold =  fread(s, 1, 'uint16')
bias = fread(s, 1, 'uchar')
exposure =  fread(s, 1, 'uint16')
filtering = fread(s, 1, 'uchar')
outputForm = fread(s, 1, 'uchar')
nonzeropixelsfiltered =  fread(s, 1, 'uint16')
nonzeropixelsoriginal =  fread(s, 1, 'uint16')
minValueOriginal = fread(s, 1, 'uchar')
maxValueOriginal = fread(s, 1, 'uchar')
minValueFiltered = fread(s, 1, 'uchar')
maxValueFiltered = fread(s, 1, 'uchar')

fread(s, 1, 'uint16');

% Histograms 
if (outputForm == 4)

    histogram = zeros(2, 256);

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

                histogram(1, packetId*64 + i) = fread(s, 1, 'uchar');

            elseif (packetType == 'H')

                histogram(2, packetId*64 + i) = fread(s, 1, 'uchar');

            end

        end

        fread(s, 1, 'uint16');

        fprintf(s, '%c', 'h');

        if (packetType == 'H' && packetId == 3)
            break;
        end

    end

    figure(3);
    subplot(2, 1, 1);
    title('Line histogram');
    plot(histogram(1, :)');
    axis auto
    subplot(2, 1, 2);
    title('Row histogram');
    plot(histogram(2, :)');
    axis auto
    
else
   
    if (outputForm == 1)
       
        numPerLine = 32; 
        numerOfPackets = 16;
       
    elseif (outputForm == 2)
        
        numPerLine = 16;
        numerOfPackets = 4;
        
    elseif (outputForm == 3)
        
        numPerLine = 8;
        numerOfPackets = 1;
    
    end
    
    binnedImage = zeros(numPerLine, numPerLine);
           
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
            
            binnedImage(floor(bytesReceived/numPerLine)+1, mod(bytesReceived, numPerLine)+1) = fread(s, 1, 'uchar');
            bytesReceived = bytesReceived + 1;
            
        end
        
        fread(s, 1, 'uint16');

        fprintf(s, '%c', 'h');
        
    end
    
    figure(4);
    
    maximum = max(max(binnedImage, [], 1));
    
    imshow(binnedImage./255, [0, maximum(1)/255], 'InitialMagnification', 'fit');
    
end

fclose(s);