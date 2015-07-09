% pozadej o mereni    
fprintf(s, '%c', 'q');

receiveMetadata;

% receive compressed image
if image.outputForm == 0
    
    receiveCompressed;
    disp(['Image saved as ' int2str(image.imageId) 'r']);
    
else
   
    receivePostprocessed;
    disp(['Image saved as ' int2str(image.imageId) 'p']);
    
end