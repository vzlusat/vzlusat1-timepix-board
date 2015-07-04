openPort();

tic

% pozadej o nezpracovany obrazek
fprintf(s, '%c', 'e');
    
receiveMetadata;

receiveCompressed;

toc

closePort();
