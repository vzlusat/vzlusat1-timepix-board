openPort();

tic

exposure = 200;

fprintf(s, '%c', '5');
fwrite(s, swapbytes(uint16(exposure)), 'uint16');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();