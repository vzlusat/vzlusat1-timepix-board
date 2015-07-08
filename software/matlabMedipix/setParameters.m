openPort();

treshold = 430;
exposure = 1000;
bias = 240;
filtering = 0;
mode = 1;
outputForm = 0;

fprintf(s, '%c', '2');

fwrite(s, treshold, 'uint16');
fwrite(s, exposure, 'uint16');
fwrite(s, bias, 'uchar');
fwrite(s, filtering, 'uchar');
fwrite(s, mode, 'uchar');
fwrite(s, outputForm, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

closePort();