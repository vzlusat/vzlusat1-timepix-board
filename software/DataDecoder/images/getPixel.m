function [ occupied ] = getPixel( image, x, y )

occupied = false;

if (x < 1) || (x > 256)
    return;
end
if (y < 1) || (y > 256)
    return;
end

if (image(x, y) > 0)
    occupied = true;
   
end

