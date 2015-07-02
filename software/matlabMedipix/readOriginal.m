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

figure(1);

image = zeros(256, 256);

% pozadej o mereni    
fprintf(s, '%c', 'e');
    
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

while true

    % wait for next packet    
    while (s.BytesAvailable <= 0)
    end
    
    packetType = fread(s, 1, 'uchar');
    
    if packetType == 'B'
        
        imageId = fread(s, 1, 'uint16');
        
        numPixels = fread(s, 1, 'uchar');
        
        idx = fread(s, 1, 'uint16');
        
        for i=1:numPixels
                       
            image(floor((idx/256))+1, mod(idx, 256)+1) = fread(s, 1, 'uchar');
            
            idx = fread(s, 1, 'uint16');
            
        end
        
        fprintf(s, '%c', 'h');
        
    elseif packetType == 'C'
        
        fprintf(s, '%c', 'h');
        break;
        
    end
    
end

fclose(s);

maximum = max(max(image, [], 1));

imshow(image./255, [0, 0.5*maximum(1)/255], 'InitialMagnification', 'fit');

% im = struct('params', params, 'image', image);
% 
% load('fileCounter.mat');
% fileCounter = fileCounter + 1;
% save('fileCounter.mat', 'fileCounter');
% 
% filename = ['image' num2str(fileCounter)];
% save(filename, 'im');
% 
