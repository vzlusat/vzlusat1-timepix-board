function [] = showImage( name )
%UNTITLED2 Summary of this funcition goes here
%   Detailed explanation goes here

    load([name '.mat']);
    
    mask = load('mask.mat');
    
    im.image = im.image - mask.image;
    
    maximum = max(im.image);
    
    imshow(im.image./255, [0, maximum(1)/255]);

end

