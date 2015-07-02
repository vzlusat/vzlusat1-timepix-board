if exist('s')
    fclose(s);
    delete(s);
    clear s;
end

clear all

s = serial('COM29');
s.BaudRate = 230400;
s.BytesAvailableFcnMode = 'terminator';
s.Terminator = 'CR/LF';
s.DataBits = 8;
s.Parity = 'none';
s.StopBits = 1;

fopen(s);

treshold = 310;
exposure = 60000;
bias = 70;
filtering = 1;
mode = 0;
outputForm = 1;

tic

fprintf(s, '%c', '2');

fwrite(s, treshold, 'int16');
fwrite(s, exposure, 'int16');
fwrite(s, bias, 'uchar');
fwrite(s, filtering, 'uchar');
fwrite(s, mode, 'uchar');
fwrite(s, outputForm, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

fclose(s);

toc