openPort();

tic

exposure = 100;

fprintf(s, '%c', '5');
fwrite(s, exposure, 'uint16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();