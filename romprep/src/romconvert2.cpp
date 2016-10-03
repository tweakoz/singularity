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
#include <map>
#include <set>
#include <assert.h>
#include <stdint.h>

using namespace std;

static const int ksize = 1<<20;

struct chip
{
	uint8_t* bytes;
	std::map<uint8_t,int>* bsets;

	chip()
	{
		bytes = new uint8_t[ksize];
		bsets = new std::map<uint8_t,int>[ksize];
		for( int i=0; i<ksize; i++ )
		{
			bytes[i]=0;
		}
	}
	void set_byte( int addr, uint8_t byt )
	{
		assert(addr<ksize);
		assert(addr>=0);
		std::map<uint8_t,int>& bset = bsets[addr];
		bytes[addr] = byt;
		
		std::map<uint8_t,int>::iterator it = bset.find(byt);
		if( it == bset.end() )
		{
			bset.insert(std::make_pair<uint8_t,int>(byt,1));
		}
		else
		{
			it->second++;
		}

	}
	uint16_t compute_checksum() const
	{
		uint16_t checksum = 0;
		for( int i=0; i<ksize; i++ )
		{
			checksum += bytes[i];
		}
		printf( "chip<%p> checksum<0x%04x>\n", this, checksum );
		return checksum;
	}
};
std::map<int,chip*> chipmap;
chip* GetChip(int ichip)
{
	chip* rval = 0;
	std::map<int,chip*>::iterator it=chipmap.find(ichip);
	if( it==chipmap.end() )	
	{
		rval = new chip;
		chipmap[ichip] = rval;
	}
	else
		rval = it->second;
	return rval;
}

int main( int argc, char** argv )
{
	std::string fname = argv[1];
	std::ifstream input( fname.c_str() );
	std::string lhsel;
	std::string chipsel;
	std::string addrh,addrl,bytestr;
	int ichipsel;
	int ilhsel;
	chip the_chips[8];

	int ilastchipsel = -1;

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

		chip* the_chip = GetChip(ichipidx);

		uint32_t addr = (addrH<<16)|addrL;
		assert(addr<(1<<20));
		//printf( "chip<%d> lh<%d> addr<0x%08x> : ", ichipsel, ilhsel, addr );
		for( int ib=0; ib<16; ib++ )
		{	//printf( "%02x ", byterow[ib] );

			the_chip->set_byte(addr+ib,byterow[ib]);
		}
		static int inumchips = 0;
		if( ilastchipsel!=ichipsel )
		{
			printf( "OnChip<%d> NumChipsProcessed<%d>\r", ichipidx, inumchips );
			fflush(stdout);
			ilastchipsel=ichipsel;
			inumchips++;
		}
		//printf("\n");
	}
	///////////////////////////////////
	for( std::map<int,chip*>::const_iterator it=chipmap.begin(); it!=chipmap.end(); it++ )
	{
		chip* pchip = it->second;
		//pchip->bytes[0] = 1;
		for( int i=0; i<ksize; i++ )
		{
			const std::map<uint8_t,int>& bset = pchip->bsets[i];
			if( bset.size()>1)
			{
				uint8_t setbyte = pchip->bytes[i];
				int icnt = 1;
				printf( "chip<%p> : addr<0x%08x> ", pchip, i );

				for( std::map<uint8_t,int>::const_iterator it=bset.begin(); it!=bset.end(); it++ )
				{
					int val = it->first;
					int cnt = it->second;
					if( cnt>icnt )
					{
						icnt=cnt;
						setbyte=uint8_t(val);
					}	

					printf( "<%02x:%d> ", val, cnt );
				}
				pchip->bytes[i] = setbyte;
				printf("\n");
			}
		}
	}
	///////////////////////////////////
	for( std::map<int,chip*>::const_iterator it=chipmap.begin(); it!=chipmap.end(); it++ )
	{
		int ichipID = it->first;
		chip* pchip = it->second;

		uint16_t cksum = pchip->compute_checksum();
		char fnamebuf[256];
		sprintf( fnamebuf, "k2ksamplerom_%04x.bin", cksum );
		FILE* fout = fopen( fnamebuf, "wb" );
		for( int ib=0; ib<(1<<20); ib++ )
		{
			uint8_t b0 = pchip->bytes[ib];
			fwrite(&b0,1,1,fout);
		}
		fclose(fout);
		//////////////////////
	}
	///////////////////////////////////
	return 0;
}
