openPort();

tic

% pozadej o metadata z posledniho mereni
fprintf(s, '%c', 'x');
   
receiveSensors;

IR
TIR
UV1
UV2

toc

closePort();