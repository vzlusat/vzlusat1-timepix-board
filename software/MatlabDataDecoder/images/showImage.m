function [] = showImage( name )

image = load([name '.mat']);
image = image.image
image.data = double(image.data);

if (image.outputForm <= 8)

        if (image.outputForm == 1)
           if (image.mode == 1)

                a = load('_calibration_a.txt');
                b = load('_calibration_b.txt');
                c = load('_calibration_c.txt');
                t = load('_calibration_t.txt');

                for i=1:256
                    for j=1:256
                        if (image.data(i, j) > 0)
                            image.data(i, j) = image.data(i, j) * 1;
                            image.data(i, j) = (t(i, j)*a(i, j) + image.data(i, j) - b(i, j) + sqrt((b(i, j) + t(i, j)*a(i, j) - image.data(i, j))^2 + 4*a(i, j)*c(i, j)))/(2*a(i, j));
                        end
                    end
                end
           end
        end
        geg = jet;
        %             geg(1, :) = [0, 0, 0];
        %             geg(end, :) = [1, 0, 0];

        fig = figure(1);
        clf
        imagesc(image.data);
        axis equal;
        axis tight;
        colorbar;
        colormap(geg)
        hold on                

        %         for i=1:256
        %             for j=1:256
        %                 if (image.data(i, j) > 0)
        %                  strmin = [num2str(image.data(i, j), '%1.1f'), ''];
        %                  text(j-6, i-3, strmin, 'HorizontalAlignment', 'left', 'color', 'w');
        %                 else
        %                     image.data(i, j) = 0;
        %                 end
        %             end
        %         end
        drawnow;
        print(fig, [int2str(image.imageId) '_img'],'-dpng');

        fig = figure(2);
        temp = image.data(find(image.data > 0));
        hist(reshape(temp, length(temp), 1), 300)
        xlabel('E [kev]');
        ylabel('Counts [-]');
        xlim([0, 120]);
        drawnow;
        print(fig, [int2str(image.imageId) '_hist'],'-dpng');

    elseif (image.outputForm == 32)

        figure(33);

        a = load('_calibration_a.txt');
        b = load('_calibration_b.txt');
        c = load('_calibration_c.txt');
        t = load('_calibration_t.txt');

        ma = mean(mean(a));
        mb = mean(mean(b));
        mc = mean(mean(c));
        mt = mean(mean(t));

        x = linspace(0, 256, 17) .* 1;

        for i=1:17
           x(i) = (mt*ma + x(i) - mb + sqrt((mb + mt*ma - x(i))^2 + 4*ma*mc))/(2*ma);
        end

        x

        % x(1:end-1) + (x(2:end)-x(1:end-1))./2

        for i=1:16
           rectangle('Position', [x(i), 0, x(i+1)-x(i), image.data(i)], 'FaceColor', [0 0.5 0.5], 'EdgeColor', 'b','LineWidth',1);
        end

        %         title('Histogram of pixel values');
        xlabel('Energy [keV]');
        ylabel('Pixels [-]');

        drawnow;

    elseif (image.outputForm == 16)

        figure(2);
        subplot(2, 1, 1);
        plot(image.data(1, :)');
        title('Summing rows');
        xlabel('Row [-]');
        ylabel('Number of active pixels [-]');
        axis auto
        xlim([1 256]);
        subplot(2, 1, 2);
        plot(image.data(2, :)');
        title('Summing columns');
        xlabel('Column [-]');
        ylabel('Number of active pixels [-]');
        axis auto
        xlim([1 256]);
        drawnow;

    end

end

