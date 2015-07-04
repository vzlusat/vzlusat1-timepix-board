openPort();

tic

% pozadej o metadata z posledniho mereni
fprintf(s, '%c', 'z');
   
receiveMetadata;

toc

closePort();