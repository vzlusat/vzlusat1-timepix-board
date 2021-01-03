openPort();

bias = 240;

tic

fprintf(s, '%c', '4');
fwrite(s, bias, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();