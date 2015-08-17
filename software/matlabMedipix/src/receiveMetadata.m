% wait for data       
while (s.BytesAvailable <= 0)
end

image.packetType = fread(s, 1, 'uchar');
image.imageId = swapbytes(uint16(fread(s, 1, 'uint16')));
image.mode = fread(s, 1, 'uchar');
image.treshold = swapbytes(uint16(fread(s, 1, 'uint16')));
image.bias = fread(s, 1, 'uchar');
image.exposure = swapbytes(uint16(fread(s, 1, 'uint16')));
image.filtering = fread(s, 1, 'uchar');
image.outputForm = fread(s, 1, 'uchar');
image.nonzeropixelsfiltered = swapbytes(uint16(fread(s, 1, 'uint16')));
image.nonzeropixelsoriginal = swapbytes(uint16(fread(s, 1, 'uint16')));
image.minValueOriginal = fread(s, 1, 'uchar');
image.maxValueOriginal = fread(s, 1, 'uchar');
image.minValueFiltered = fread(s, 1, 'uchar');
image.maxValueFiltered = fread(s, 1, 'uchar');
image.temperature = fread(s, 1, 'int8');
image.tempLimit = fread(s, 1, 'int8');
image.pxlLimit = swapbytes(uint16(fread(s, 1, 'uint16')));
image.uv1thr = swapbytes(int16(fread(s, 1, 'int16')));

image.chunkId = swapbytes(uint32(fread(s, 1, 'uint32')));

for u=1:7
    image.attitude(u) = swapbytes(int16(fread(s, 1, 'int16')));
end

for u=1:3
    image.position(u) = swapbytes(int16(fread(s, 1, 'int16')));
end

image.time = swapbytes(uint32(fread(s, 1, 'uint32')));

fread(s, 1, 'int16');