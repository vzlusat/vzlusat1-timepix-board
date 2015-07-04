image.data = zeros(256, 256);

if (image.nonzeropixelsoriginal == 0)
    disp('The image is blank');
else

    while true

        packetType = fread(s, 1, 'uchar');

        if packetType == 'B'

            imageId = fread(s, 1, 'uint16');

            numPixels = fread(s, 1, 'uchar');

            idx = fread(s, 1, 'uint16');

            for i=1:numPixels

                image.data(floor((idx/256))+1, mod(idx, 256)+1) = fread(s, 1, 'uchar');

                idx = fread(s, 1, 'uint16');

            end

            fprintf(s, '%c', 'h');

        elseif packetType == 'C'

            fprintf(s, '%c', 'h');
            break;

        end

    end
    
end

figure(1);
imagesc(image.data);
axis equal;
axis tight;
colorbar;
colormap(hot)