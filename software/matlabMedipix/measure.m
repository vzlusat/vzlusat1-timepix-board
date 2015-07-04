setParameters;

closePort();
clear all;

openPort();

tic

% pozadej o mereni    
fprintf(s, '%c', '9');

receiveMetadata;

% receive compressed image
if image.outputForm == 0
    
    receiveCompressed;
    
else
   
    receivePostprocessed;
    
end

toc

closePort();