openPort();

tic

% pozadej o vypnuti mpx      
fprintf(s, '%c', '1');
    
% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();