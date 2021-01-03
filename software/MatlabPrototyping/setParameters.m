openPort();

treshold = 310;
exposure = 1000;
bias = 70;
filtering = 0;
mode = 1;
outputForm = 1;
tempLimit = 80;
pxlCount = 0;
uv1thr = 500;

fprintf(s, '%c', '2');

fwrite(s, swapbytes(uint16(treshold)), 'uint16');
fwrite(s, swapbytes(uint16(exposure)), 'uint16');
fwrite(s, bias, 'uchar');
fwrite(s, filtering, 'uchar');
fwrite(s, mode, 'uchar');
fwrite(s, outputForm, 'uchar');
fwrite(s, tempLimit, 'int8');
fwrite(s, swapbytes(uint16(pxlCount)), 'uint16');
fwrite(s, swapbytes(int16(uv1thr)), 'int16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

closePort();