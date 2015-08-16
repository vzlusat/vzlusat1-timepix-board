openPort();

treshold = 310;
exposure = 100;
bias = 240;
filtering = 0;
mode = 1;
outputForm = 1 + 2 + 4 + 8 + 16 + 32;
tempLimit = 55;
pxlCount = 0;

fprintf(s, '%c', '2');

fwrite(s, treshold, 'uint16');
fwrite(s, exposure, 'uint16');
fwrite(s, bias, 'uchar');
fwrite(s, filtering, 'uchar');
fwrite(s, mode, 'uchar');
fwrite(s, outputForm, 'uchar');
fwrite(s, tempLimit, 'int8');
fwrite(s, pxlCount, 'uint16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

closePort();