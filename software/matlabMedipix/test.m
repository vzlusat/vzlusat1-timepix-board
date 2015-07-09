setParameters;

openPort();

for o=41:100
   
    measure_for_test;
    
    huhl(o) = image.nonzeropixelsfiltered;
    
    figure(5);
    hist(huhl);
    
    o
    
    drawnow;

end

closePort();
