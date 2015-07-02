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

bias = 240;

fprintf(s, '%c', '4');
fwrite(s, bias, 'uchar');

% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

fclose(s);