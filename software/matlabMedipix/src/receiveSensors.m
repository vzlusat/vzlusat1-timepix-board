% wait for data       
while (s.BytesAvailable <= 0)
end

IR = swapbytes(int16(fread(s, 1, 'int16')));
TIR = swapbytes(int16(fread(s, 1, 'int16')));
UV1 = swapbytes(int16(fread(s, 1, 'int16')));
UV2 = swapbytes(int16(fread(s, 1, 'int16')));

fread(s, 1, 'int16');

disp(image);