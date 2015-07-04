% wait for data       
while (s.BytesAvailable <= 0)
end

image.packetType = fread(s, 1, 'uchar');
image.imageId =  fread(s, 1, 'uint16');
image.mode = fread(s, 1, 'uchar');
image.treshold =  fread(s, 1, 'uint16');
image.bias = fread(s, 1, 'uchar');
image.exposure =  fread(s, 1, 'uint16');
image.filtering = fread(s, 1, 'uchar');
image.outputForm = fread(s, 1, 'uchar');
image.nonzeropixelsfiltered =  fread(s, 1, 'uint16');
image.nonzeropixelsoriginal =  fread(s, 1, 'uint16');
image.minValueOriginal = fread(s, 1, 'uchar');
image.maxValueOriginal = fread(s, 1, 'uchar');
image.minValueFiltered = fread(s, 1, 'uchar');
image.maxValueFiltered = fread(s, 1, 'uchar');

fread(s, 1, 'int16');

disp(image);