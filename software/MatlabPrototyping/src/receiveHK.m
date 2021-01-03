% wait for data       
while (s.BytesAvailable <= 0)
end

hk.packetType = fread(s, 1, 'uchar');
hk.bootCount = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.imagesTaken = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.temperature = fread(s, 1, 'uchar');
hk.framStatus = fread(s, 1, 'uchar');
hk.medipixStatus = fread(s, 1, 'uchar');
hk.time = swapbytes(uint32(fread(s, 1, 'uint32')));
hk.TIR_max = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.TIR_min = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.IR_max = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.IR_min = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.UV1_max = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.UV1_min = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.UV2_max = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.UV2_min = swapbytes(uint16(fread(s, 1, 'uint16')));
hk.temperatur_max = fread(s, 1, 'uchar');
hk.temperature_min = fread(s, 1, 'uchar');

disp(hk);
fread(s, 1, 'int16');