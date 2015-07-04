openPort();

treshold = 310;

tic

fprintf(s, '%c', '3');
fwrite(s, treshold, 'int16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

closePort();

toc