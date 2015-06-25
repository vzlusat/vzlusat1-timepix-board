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

fopen(s);

figure(2);

image = zeros(256, 256);

% pozadej o mereni    
fprintf(s, '%c', '6');
    
% precti 256 radku
for i=1:256
    
    % wait for data       
    while (s.BytesAvailable < 34)
    end
   
    % precti 8 packetu na radek    
    for j=1:8
        
        % pres 32 bytu na radku         
        for k=1:32
            
            image(i, 32*(j-1)+k) = fread(s, 1, 'uchar');
        end
        
        % read end of line         
        fread(s, 1, 'int16');
    end
    
    if (mod(i, 10) == 0)
        sprintf('Line %d', i)
    end
end

% wait for last line data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

while (s.BytesAvailable <= 0)
end

params = fgets(s);

sprintf(['Parameters: ' params])

fclose(s);

mask = load('mask.mat');

image = image - mask.image;

maximum = max(max(image, [], 1));

imshow(image./255, [0, maximum(1)/255], 'InitialMagnification', 'fit');

im = struct('params', params, 'image', image);

load('fileCounter.mat');
fileCounter = fileCounter + 1;
save('fileCounter.mat', 'fileCounter');

filename = ['image' num2str(fileCounter)];
save(filename, 'im');

