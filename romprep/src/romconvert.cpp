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

uint16_t kcsums[8] = 
{
	0x25da,
	0xde34,
	0x0e87,
	0x3f2d,
	0x20fe,
	0x1bdf,
	0x3cde,
	0x2e6d,
};

struct chip
{
	static const int ksize = 1<<20;
	uint8_t* bytes;

	chip()
	{
		bytes = new uint8_t[ksize];
		for( int i=0; i<ksize; i++ )
		{
			bytes[i]=0;
		}
	}
	void compute_checksum(int ichip) const
	{
		const uint16_t desired = kcsums[ichip];
		uint16_t checksum = 0;
		for( int i=0; i<ksize; i++ )
		{
			checksum += bytes[i];
		}
		printf( "chip<%p> checksum<0x%04x> desired<0x%04x>\n", this, checksum, desired );
	}
};

int main( int argc, char** argv )
{
	std::ifstream input( "DUMP1" );
	std::string lhsel;
	std::string chipsel;
	std::string addrh,addrl,bytestr;
	int ichipsel;
	int ilhsel;
	chip the_chips[8];

	for( std::string line; getline( input, line ); )
	{
		int i=0;
		int ipass = 0;
        std::stringstream strstrline(line);

        uint8_t byterow[16];
        uint32_t addrL, addrH;
        int ibytecol = 0;

		for( std::string tok; std::getline( strstrline, tok, '.' ); )
		{
			//printf( "tok<%d><%s>\n", i, tok.c_str() );
			std::stringstream sstok(tok);
			uint32_t ubyt;
			switch( i )
			{
				case 0: // chip/addrh
				{
					std::string tokl2;
					std::getline( sstok, tokl2, ':' );
					chipsel = tokl2.substr(5,1);
					lhsel = tokl2.substr(6,1);
					ichipsel = atoi( chipsel.c_str() );
					ilhsel = (lhsel=="H");
					//printf( "CHIPSEL:%d LHSEL:%d\n", ichipsel, ilhsel );
					/////////////////////////////////////////
					std::getline( sstok, tokl2, ':' );
					addrh = "0x"+tokl2.substr(4,10);
					std::transform(addrh.begin(), addrh.end(), addrh.begin(), ::tolower);
					sscanf( addrh.c_str(), "0x%x", & addrH );
					//printf( "ADDRH<0x%04x>\n", addrH );
					break;
				}
				case 1: // addrl/b1
				{
					std::getline( sstok, addrl, '>' );
					//addrl = tokl2.substr(3,2);
					std::transform(addrl.begin(), addrl.end(), addrl.begin(), ::tolower);
					sscanf( addrl.c_str(), "%x", & addrL );
					//printf( "ADDRL<0x%04x>\n", addrL );
					std::string tokl2;
					std::getline( sstok, tokl2, '>' );
					bytestr = "0x"+tokl2;
					sscanf( bytestr.c_str(), "0x%x", & ubyt );
					assert(ubyt<=0xff);
					std::transform(bytestr.begin(), bytestr.end(), bytestr.begin(), ::tolower);
					//printf( "BYTE<0x%02x>\n", ubyt );
					byterow[0] = uint8_t(ubyt);
					ibytecol=1;
					break;
				}
				default:
				{
					bytestr = "0x"+tok;
					std::transform(bytestr.begin(), bytestr.end(), bytestr.begin(), ::tolower);
					sscanf( bytestr.c_str(), "0x%x", & ubyt );
					assert(ubyt<=0xff);
					//printf( "BYTE<0x%02x>\n", ubyt );
					byterow[ibytecol] = uint8_t(ubyt);
					ibytecol++;
					assert(i<17);
					break;
				}
			}
			i++;
		} // for line
		int ichipidx = (ilhsel<<2)|ichipsel;
		chip& out_chip = the_chips[ichipidx];

		uint32_t addr = (addrH<<16)|addrL;
		assert(addr<(1<<20));
		//printf( "chip<%d> lh<%d> addr<0x%08x> : ", ichipsel, ilhsel, addr );
		for( int ib=0; ib<16; ib++ )
		{	//printf( "%02x ", byterow[ib] );
			if( ipass > 0 )
			{
				if( byterow[ib] != out_chip.bytes[addr+ib] )
					printf( "mismatch\n" );
			}

			out_chip.bytes[addr+ib] = byterow[ib];
		}
		if( (ichipidx==7) && (addr == 0xfffff) )
		{
			ipass++;
			printf( "started pass <%d>\n", ipass+1 );
		}
		printf("\n");
	}
	///////////////////////////////////
	for( int i=0; i<8; i++ )
	{
		const chip& chip = the_chips[i];
		chip.compute_checksum(i);
		char fnamebuf[256];
		sprintf( fnamebuf, "k2ksamplerom_chip%d.bin", i );
		FILE* fout = fopen( fnamebuf, "wb" );
		for( int ib=0; ib<(1<<20); ib++ )
		{

			uint8_t b0 = chip.bytes[ib];
			fwrite(&b0,1,1,fout);
		}
		
		fclose(fout);
	}
	///////////////////////////////////
	FILE* fout = fopen( "k2kintsamplerom.raw","wb");
	if(0)
	for( int ic=0; ic<8; ic++ )
	{
		const chip& chip = the_chips[ic];
		for( int ib=0; ib<(1<<20); ib++ )
		{

			uint8_t b0 = chip.bytes[ib];
			fwrite(&b0,1,1,fout);
		}
	}
	else
	for( int ic=0; ic<4; ic++ )
	{
		const chip& chipA = the_chips[ic];
		const chip& chipB = the_chips[ic+4];
		for( int ib=0; ib<(1<<20); ib++ )
		{

			uint8_t bA = chipA.bytes[ib];
			uint8_t bB = chipB.bytes[ib];
			fwrite(&bB,1,1,fout);
			fwrite(&bA,1,1,fout);
		}
	}
	fclose(fout);
	///////////////////////////////////
	return 0;
}
