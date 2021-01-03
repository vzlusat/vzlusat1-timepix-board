function [] = showImage( name )

    image = load(['image_' name '.mat']);
    image = image.image;

    if (size(image.data, 1) > 3)

        figure(1);
        imagesc(image.data);
        axis equal;
        axis tight;
        colorbar;
        colormap(hot)

    elseif (size(image.data, 1) == 1)
        
        figure(6);
        bar(image.data);
        
    else

        figure(3);
        subplot(2, 1, 1);
        title('Line histogram');
        plot(image.data(1, :)');
        axis auto
        subplot(2, 1, 2);
        title('Row histogram');
        plot(image.data(2, :)');
        axis auto
        
    end

    disp(image);
    
end

