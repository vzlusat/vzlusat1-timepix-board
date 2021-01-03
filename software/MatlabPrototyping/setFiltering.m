openPort();

tic

filtering = 1;

fprintf(s, '%c', '6');
fwrite(s, filtering, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();