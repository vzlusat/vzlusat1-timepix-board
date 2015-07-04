openPort();

mode = 1;

tic

fprintf(s, '%c', '7');
fwrite(s, mode, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();