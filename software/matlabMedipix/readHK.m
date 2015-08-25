openPort();

tic

% pozadej o metadata z posledniho mereni
fprintf(s, '%c', 'l');
   
receiveHK;

toc

closePort();