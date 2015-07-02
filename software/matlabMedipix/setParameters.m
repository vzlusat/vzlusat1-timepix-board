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
exposure = 100;
bias = 240;
filtering = 0;
mode = 1;
outputForm = 2;

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