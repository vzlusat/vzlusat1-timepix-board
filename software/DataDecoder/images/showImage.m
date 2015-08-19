function [] = showImage( name )

    image = load([name '.mat']);
    image = image.image;

    if (image.outputForm <= 8)

        figure(1);
        imagesc(image.data);
        axis equal;
        axis tight;
        colorbar;
        colormap(hot);
        drawnow;

    elseif (image.outputForm == 32)
        
        figure(3);
        bar(image.data);
        drawnow;
        
    elseif (image.outputForm == 16)
        
        figure(2);
        subplot(2, 1, 1);
        title('Line histogram');
        plot(image.data(1, :)');
        axis auto
        subplot(2, 1, 2);
        title('Row histogram');
        plot(image.data(2, :)');
        axis auto
        drawnow;
        
    end

    disp(image);
    
end

