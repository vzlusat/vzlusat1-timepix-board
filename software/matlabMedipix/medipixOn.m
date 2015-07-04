openPort();

tic

% pozadej o zapnuti mpx      
fprintf(s, '%c', '0');
    
% wait for data       
while (s.BytesAvailable <= 0)
end

disp(fgets(s));

toc

closePort();