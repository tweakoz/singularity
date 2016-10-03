#include "krzio.h"

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
	rapidjson::Document gconfigdoc;
	//document.Parse(json);

	filescanner scanner(argv[1]);
	Krz			krz;

	scanner._dlsFile.pInfo->Name = "KrzDlsFile";
	/////////////////////////////////////////////	
	u8 u8v;
	u16 u16v;
	u32 u32v;
	bool bOK;
	/////////////////////////////////////////////
	bOK = scanner.GetData( u32v );
	printf( "u32v<%8x>\n", int(u32v) );
	//assert(kKRZMagic==u32v);
	/////////////////////////////////////////////
	bOK = scanner.GetData( krz.miFileHeaderAndPRAMLength );
	printf( "u32v<%8x>\n", int(krz.miFileHeaderAndPRAMLength) );
	/////////////////////////////////////////////
	bOK = scanner.GetData( u32v );
	printf( "u32v<%8x>\n", int(u32v) );
	bOK = scanner.GetData( u32v );
	printf( "u32v<%8x>\n", int(u32v) );
	/////////////////////////////////////////////
	bOK = scanner.GetData( u32v );
	printf( "K2KOSRELEASE<%d>\n", int(u32v) );
	/////////////////////////////////////////////
	bOK = scanner.GetData( u32v );
	printf( "HWTYPE<%08x>\n", int(u32v) );
//	switch( u32v )
//	{}
//	assert( kKRZHwTypeK2000==u32v );
	/////////////////////////////////////////////
	scanner.SkipData(8);
	/////////////////////////////////////////////
	bool bdone = false;
	int iblock = 0;
	printf( "FileHeaderAndPRAMLength<%p>\n", (void*)(krz.miFileHeaderAndPRAMLength) );
	while((iblock<10) && (false==bdone))
	{
		printf("iblock<%d>\n", iblock );
		int blocklen = 0;
		int iseekpos = scanner.mMainIterator.miIndex;
		bOK = scanner.GetData( blocklen );
		blocklen *= -1;
		printf( "SeekPos<%p> Block<%d> Length<%d>\n", (void*) iseekpos, int(iblock), int(blocklen) );
		iblock++;
		if( (iseekpos+4) < krz.miFileHeaderAndPRAMLength )
		{
			datablock newblock;
			newblock.CopyFrom(scanner.mMainDataBlock,scanner.mMainIterator,blocklen-4);
			scanner.mDatablocks.push_back(newblock);
		}
		else
		{
			bdone = true;
		}
	}
	////////////////////////
	// get sample data
	////////////////////////
	int isampledatacount = scanner.miSize - krz.miFileHeaderAndPRAMLength;
	
	printf( "FileSize<%d> SampleDataSize<%d>\n", scanner.miSize, isampledatacount );

	if( isampledatacount )
	{
		krz.mpSampleData = (s16*) malloc( isampledatacount );
		krz.miSampleDataLength = isampledatacount;
		
		void* pdest = (void*) krz.mpSampleData;
		const char* psrcbase = (const char*) scanner.mpData;
		const char* psrc = psrcbase + krz.miFileHeaderAndPRAMLength;
		memcpy( (void*) pdest, (const void*)psrc, isampledatacount );
		
	}
	////////////////////////
	// process objects
	////////////////////////

	scanner.scanAndDump();

	/////////////////////////////

    //scanner._dlsFile.Save("krz.gig");

	/////////////////////////////

	for( auto itsamp : scanner._subSamples )
	{
		SampleItem* si = itsamp.second;
		//si->mpDlsSample->Write(si->_sampleData, si->_numSamples);

	}

	/////////////////////////////

	return 0;

}

///////////////////////////////////////////////////////////////////////////////
