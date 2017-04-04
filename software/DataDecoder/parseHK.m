function [] = parseHK(data)

    hk.bootCount = swapbytes(typecast(data(1:2), 'uint16'));
    hk.imagesTaken = swapbytes(typecast(data(3:4), 'uint16'));
    hk.temperature = typecast(data(5), 'uint8');
    hk.framStatus = typecast(data(6), 'uint8');
    hk.medipixStatus = typecast(data(7), 'uint8');
    hk.bootTime = swapbytes(typecast(data(8:11), 'uint32'));
    hk.TIR_max = swapbytes(typecast(data(12:13), 'int16'));
    hk.TIR_min = swapbytes(typecast(data(14:15), 'int16'));
    hk.IR_max = swapbytes(typecast(data(16:17), 'int16'));
    hk.IR_min = swapbytes(typecast(data(18:19), 'int16'));
    hk.UV1_max = swapbytes(typecast(data(20:21), 'int16'));
    hk.UV1_min = swapbytes(typecast(data(22:23), 'int16'));
    hk.UV2_max = swapbytes(typecast(data(24:25), 'int16'));
    hk.UV2_min = swapbytes(typecast(data(26:27), 'int16'));
    hk.temp_max = typecast(data(28), 'int8');
    hk.temp_min = typecast(data(29), 'int8');
    
    disp(hk);
    
end
