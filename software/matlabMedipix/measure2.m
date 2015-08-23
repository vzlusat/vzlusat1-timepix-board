setParameters;

closePort();

openPort();

tic

% pozadej o mereni    
fprintf(s, '%c', 'q');

for g=0:5

    if ((bitand(outputForm, 2^g) > 0))

        receiveMetadata;

        % receive compressed image
        if (image.outputForm == 1)

            receiveCompressed;
            disp('Compressed');

        else

            receivePostprocessed;
            disp('Postprocessed');
            
        end
        
        save(['images/image_' int2str(image.imageId) '_' int2str(image.outputForm)], 'image');
        disp(['Image saved as ' int2str(image.imageId) '_' int2str(image.outputForm)]);
        disp(image);
        
    end

end

toc

closePort();

drawnow;

tightfig;