clear all

%% open the input file 

fileName = 'input3.txt';
fid = fopen(fileName,'r');

%% 

line = fgets(fid);
while ischar(line)
    
    % parse the line
    if (line(1:4) == 'data') % if it is the data line
        
        line = line(8:end);
        while true
            if ((line(end) == 10) || (line(end) == 13))
                line = line(1:(end-1));
            else
               break; 
            end
        end
        binaryData = 0;
        
        for (i=1:2:(length(line)-1))
            binaryData(round(i/2)) = hex2dec(line(i:(i+1)));
        end
        
        binaryData = uint8(binaryData);
        
        % parse the binary data         
        
        % Metadata         
        if (binaryData(1) == uint8('A'))
            
            disp('Reading image Metadata');
            parseMetadata(binaryData(2:end));
        
        % Raw image       
        elseif (binaryData(1) == uint8('B'))
            
            disp('Reading image (Raw)');
            parseRaw(binaryData(2:end));
            
        % Binning8  
        elseif (binaryData(1) == uint8('D'))
            
            disp('Reading image (Binn8)');
            parseBinn8(binaryData(2:end));
        
        % Binning16
        elseif (binaryData(1) == uint8('E'))
         
            disp('Reading image (Binn16)');
            parseBinn16(binaryData(2:end));
            
        % Binning32
        elseif (binaryData(1) == uint8('F'))
         
            disp('Reading image (Binn32)');
            parseBinn32(binaryData(2:end));
            
        % Rows summed
        elseif (binaryData(1) == uint8('h'))
            
            disp('Reading image (Row sum)');
            parseRowSums(binaryData(2:end));
            
        % Cols summed
        elseif (binaryData(1) == uint8('H'))
            
            disp('Reading image (Col sum)');
            parseColSums(binaryData(2:end));
            
        % Energy histograms
        elseif (binaryData(1) == uint8('e'))
            
            disp('Reading image (En. hist)');
            parseEnergeticHist(binaryData(2:end));
            
        % houskeeping
        elseif (binaryData(1) == uint8('Z'))
            
            disp('Reading HK data');
            parseHK(binaryData(2:end));
            
        else
            
           disp('UNKNOWN PACKET'); 
           disp(line);
        end
    end
    
    line = fgets(fid);
end

%% 