if exist('s')
    fclose(s);
    delete(s);
    clear s;
end

clear all

s = serial('COM29');
s.BaudRate = 230400;
s.BytesAvailableFcnMode = 'terminator';
s.Terminator = 'CR/LF';
s.DataBits = 8;
s.Parity = 'none';
s.StopBits = 1;

fopen(s);

% // structure that hold all parameters of the measurement
% typedef struct {
% 	
% 	uint16_t imageId;
% 	uint8_t mode;
% 
% 	uint16_t threshold;
% 	
% 	uint8_t bias;
% 	
% 	uint16_t exposure;
% 	uint8_t filtering;
% 	uint8_t outputForm;
% 	
% 	uint16_t nonZeroPixelsOutput;
% 	uint16_t nonZeroPixelsOriginal;
% 	
% 	uint8_t maxValue;
% 	uint8_t minValue;
% } imageParameters_t;

% pozadej o zapnuti mpx      
fprintf(s, '%c', 'z');
    
% wait for data       
while (s.BytesAvailable <= 0)
end

packetType = fread(s, 1, 'uchar')
imageId =  fread(s, 1, 'uint16')
mode = fread(s, 1, 'uchar')
treshold =  fread(s, 1, 'uint16')
bias = fread(s, 1, 'uchar')
exposure =  fread(s, 1, 'uint16')
filtering = fread(s, 1, 'uchar')
outputForm = fread(s, 1, 'uchar')
nonzeropixelsfiltered =  fread(s, 1, 'uint16')
nonzeropixelsoriginal =  fread(s, 1, 'uint16')
minValueOriginal = fread(s, 1, 'uchar')
maxValueOriginal = fread(s, 1, 'uchar')
minValueFiltered = fread(s, 1, 'uchar')
maxValueFiltered = fread(s, 1, 'uchar')

fread(s, 1, 'int16');

fclose(s);