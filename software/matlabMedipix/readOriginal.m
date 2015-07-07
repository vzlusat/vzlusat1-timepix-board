openPort();

tic

% pozadej o nezpracovany obrazek
fprintf(s, '%c', 'e');
    
receiveMetadata;

receiveCompressed;

save(['images/image_' int2str(image.imageId) 'o'], 'image');
disp(['Image saved as ' int2str(image.imageId) 'o']);

toc

closePort();
