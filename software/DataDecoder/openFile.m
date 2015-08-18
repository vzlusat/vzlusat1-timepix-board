function [image] = openFile(fileName)

    folder = 'images';

    % create it if it does not exist
    if (exist([folder '/' fileName '.mat'], 'file') == 0) 
        
        image.imageId = 0;
        image.mode = 0;
        image.treshold = 0;
        image.bias = 0;
        image.exposure = 0;
        image.filtering = 0;
        image.outputForm = 0;
        image.nonzeropixelsfiltered = 0;
        image.nonzeropixelsoriginal = 0;
        image.minValueOriginal = 0;
        image.maxValueOriginal = 0;
        image.minValueFiltered = 0;
        image.maxValueFiltered = 0;
        image.temperature = 0;
        image.tempLimit = 0;
        image.pxlLimit = 0;
        image.uv1thr = 0;
        image.chunkId = 0;
        for u=1:7
            image.attitude(u) = 0;
        end
        for u=1:3
            image.position(u) = 0;
        end
        image.time = 0;
        image.data = 0;
        
        save([folder '/' fileName '.mat'], 'image');
        
    else
        
        image = load([folder '/' fileName '.mat']);
        image = image.image;
        
    end

end

