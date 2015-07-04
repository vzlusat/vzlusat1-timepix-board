openPort();

tic

% pozadej o zpracovany obrazek
fprintf(s, '%c', 't');
    
receiveMetadata;

receivePostprocessed;

toc

closePort();