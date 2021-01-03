openPort();

tic

% pozadej o zpracovany obrazek
fprintf(s, '%c', 't');
    
receiveMetadata;

receivePostprocessed;

save(['images/image_' int2str(image.imageId) 'p'], 'image');
disp(['Image saved as ' int2str(image.imageId) 'p']);

toc

closePort();