function [] = saveFile(image, fileName)

    folder = 'images';
    save([folder '/' fileName '.mat'], 'image');

end

