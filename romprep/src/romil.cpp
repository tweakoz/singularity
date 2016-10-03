#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main( int argc, char** argv )
{
	FILE* fina = fopen(argv[1],"rb");
	FILE* finb = fopen(argv[2],"rb");
	FILE* fout = fopen(argv[3],"wb");

	int ila = 0;
	int ilb = 0;

	fseek( fina, 0, SEEK_END );
	fseek( finb, 0, SEEK_END );
	ila = ftell(fina);
	ilb = ftell(finb);
	fseek( fina, 0, SEEK_SET );
	fseek( finb, 0, SEEK_SET );

	printf( "ila<%d>\n", ila );
	printf( "ilb<%d>\n", ilb );

	assert(ila==ilb);

	for( int i=0; i<ila; i++ )
	{
		uint8_t cha, chb;
		fread( &cha, 1, 1, fina );
		fread( &chb, 1, 1, finb );
		fwrite( &cha, 1, 1, fout );
		fwrite( &chb, 1, 1, fout );
	}

	fclose(fout);
	fclose(finb);
	fclose(fina);

	return 0;

}