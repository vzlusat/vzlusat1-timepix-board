% generuju c soubor
data = fopen('data.c', 'w');

% generuju h soubor
headers = fopen('data_headers.h', 'w');

fprintf(data, '#include "data_headers.h"\n\n');

% for all images
for i=1:26
    
    fileName = ['image' num2str(i) '.mat'];
    
    image = load(fileName);
    
    printMatrixC(data, ['int8_t image' num2str(i) '[256*256]'], '%d', image.im.image);

    fprintf(headers, 'int8_t image%d[256*256];\n\n', i);
    
end
