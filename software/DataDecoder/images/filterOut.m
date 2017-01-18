
image = load(['115_1.mat']);
image = image.image;

image_filtered = image.data;

for i=1:256
   for j=1:256
        if (getPixel(image.data, i, j) && ~getPixel(image.data, i-1, j) && ~getPixel(image.data, i+1, j) && ~getPixel(image.data, i, j-1));
            image_filtered(i, j) = image.data(i, j);
        else
            image_filtered(i, j) = 0;
        end
   end
end

geg = jet;
geg(1, :) = [0, 0, 0];
geg(end, :) = [1, 0, 0];

figure(1);
clf
imagesc(image.data);
axis equal;
axis tight;
colorbar;
colormap(geg)
drawnow;

disp(image);

figure(2);
clf
imagesc(image_filtered);
axis equal;
axis tight;
colorbar;
colormap(geg)
drawnow;

