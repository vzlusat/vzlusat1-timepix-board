if exist('s')
    fclose(s);
    delete(s);
    clear s;
end

clear all

s = serial('COM7');
s.BaudRate = 57600;
s.BytesAvailableFcnMode = 'terminator';
s.Terminator = 'CR/LF';
s.DataBits = 8;
s.Parity = 'none';
s.StopBits = 1;

fopen(s);

% pozadej o vypnuti mpx      
fprintf(s, '%c', '2');
    
% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

fclose(s);