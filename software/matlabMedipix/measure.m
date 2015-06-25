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

image = zeros(256, 256);

% pozadej o mereni    
fprintf(s, '%c', '4');

for i=1:7

% wait for last line data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));
  
end
    
