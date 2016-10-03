#include <stdio.h>
//#include <istream.h>
#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <istream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <stdint.h>

using namespace std;


static const int ksize = 1<<20;

struct chip
{
	uint8_t* bytes;

	chip()
	{
		bytes = new uint8_t[ksize];
		for( int i=0; i<ksize; i++ )
		{
			bytes[i]=0;
		}
	}
	void compute_checksum() const
	{
		uint16_t checksum = 0;
		for( int i=0; i<ksize; i++ )
		{
			checksum += bytes[i];
		}
		printf( "chip<%p> checksum<0x%04x>\n", this, checksum );
	}
};

int main( int argc, char** argv )
{
	std::string fname = argv[1];
	FILE* fin = fopen( fname.c_str(), "rb" );
	chip the_chip;
	fread( the_chip.bytes, 1, ksize, fin );
	fclose(fin);
	the_chip.compute_checksum();
	///////////////////////////////////
	return 0;
}
