openPort();

tic

% pozadej o filtrovany obrazek
fprintf(s, '%c', 'r');
    
receiveMetadata;

receiveCompressed;

save(['images/image_' int2str(image.imageId) 'f'], 'image');
disp(['Image saved as ' int2str(image.imageId) 'f']);

toc

closePort();