setParameters;

closePort();
clear all;

openPort();

tic

% pozadej o mereni    
fprintf(s, '%c', 'q');

receiveMetadata;

% receive compressed image
if image.outputForm == 0
    
    receiveCompressed;
    save(['images/image_' int2str(image.imageId) 'r'], 'image');
    disp(['Image saved as ' int2str(image.imageId) 'r']);
    
else
   
    receivePostprocessed;
    save(['images/image_' int2str(image.imageId) 'p'], 'image');
    disp(['Image saved as ' int2str(image.imageId) 'p']);
    
end

toc

closePort();