openPort();

treshold = 310;

tic

fprintf(s, '%c', '3');
fwrite(s, swapbytes(uint16(treshold)), 'uint16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

closePort();

toc