openPort();

treshold = 310;
exposure = 10000;
bias = 240;
filtering = 1;
mode = 1;
outputForm = 0;
tempLimit = 50;
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