% wait for data       
while (s.BytesAvailable <= 0)
end

IR = swapbytes(uint16(fread(s, 1, 'uint16')));
TIR = swapbytes(uint16(fread(s, 1, 'uint16')));
UV1 = swapbytes(uint16(fread(s, 1, 'uint16')));
UV2 = swapbytes(uint16(fread(s, 1, 'uint16')));

fread(s, 1, 'int16');

disp(image);