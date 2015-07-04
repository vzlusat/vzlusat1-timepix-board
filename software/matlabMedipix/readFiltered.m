openPort();

tic

% pozadej o filtrovany obrazek
fprintf(s, '%c', 'r');
    
receiveMetadata;

receiveCompressed;

toc

closePort();