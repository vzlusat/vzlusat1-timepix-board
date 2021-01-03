if exist('s')
    fclose(s);
    delete(s);
    clear s;
end

s = serial('COM25');
s.BaudRate = 230500;
s.BytesAvailableFcnMode = 'terminator';
s.Terminator = 'CR/LF';
s.DataBits = 8;
s.Parity = 'none';
s.StopBits = 1;
s.ReadAsyncMode = 'continuous';
s.Timeout = 600;

fopen(s);
