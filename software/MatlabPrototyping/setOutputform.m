openPort();

tic

outputForm = 1;

fprintf(s, '%c', '8');
fwrite(s, outputForm, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();