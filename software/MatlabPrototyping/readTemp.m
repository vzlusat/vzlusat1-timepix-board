openPort();

tic

% pozadej o nezpracovany obrazek
fprintf(s, '%c', 'i');
    
while (s.BytesAvailable <= 0)
end

temp = fread(s, 1, 'int8')

toc

closePort();
