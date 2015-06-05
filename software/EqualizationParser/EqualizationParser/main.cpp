#include <iostream>
#include <stdio.h>
#include <stdint.h>

using namespace std;

struct PixelCfg{
		uint8_t lowTh: 3;          // low threshold (3 bits, low (0) is ACTIVE)
		uint8_t highTh: 3;         // high threshold (3 bits, low (0) is ACTIVE)
		uint8_t testBit: 1;        // test bit (1 bit, low (0) is ACTIVE)
		uint8_t maskBit: 1;        // mask bit (1 bit, low (0) is ACTIVE)
};

int main()
{

    FILE * pFile;
				pFile = fopen ("myfile.txt","w");
				FILE * inFile;
				inFile = fopen ("equalization2","rb");

				uint8_t val;

				for (int i = 0; i < 65536; i++) {

						if (fread(&val,1,1,inFile))
								fprintf (pFile, "%u,\n", val);

						if (((i+1) % 8192) == 0)
								fprintf (pFile, "konec\n");
				}

    fclose (pFile);
				fclose (inFile);

    return 0;
}
