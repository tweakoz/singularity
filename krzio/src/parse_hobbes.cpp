#include "krzio.h"
#include <fstream>

using namespace rapidjson;

///////////////////////////////////////////////////////////////////////////////

void filescanner::parseHobbes(	const datablock& db,
								datablock::iterator& it,
								u8 code )
{
	assert(_curLayer);
	assert( code>=0x50);
	assert( code<=0x53);

	auto hobbes = _curLayer->_hobbes[code-0x50];
	auto& hfp = hobbes->_hobbesFP;

	auto algCFG = getAlgConfig(_curALG );

	hfp._inputALG = db.GetTypedData<u8>(it);
	hfp._inputCourse = db.GetTypedData<u8>(it);
	hfp._inputFine = db.GetTypedData<u8>(it);
	hfp._inputKeyTrack = db.GetTypedData<u8>(it);
	hfp._inputVelTrack = db.GetTypedData<u8>(it);
	hfp._inputSrc1 = db.GetTypedData<u8>(it);
	hfp._inputDepth = db.GetTypedData<u8>(it);
	hfp._inputDptCtl = db.GetTypedData<u8>(it);
	hfp._inputMinDepth = db.GetTypedData<u8>(it);
	hfp._inputMaxDepth = db.GetTypedData<u8>(it);
	hfp._inputSrc2 = db.GetTypedData<u8>(it);
	hfp._inputFiltAlg = db.GetTypedData<u8>(it);
	hfp._inputMoreTSCR = db.GetTypedData<u8>(it);
	hfp._inputRESERVED = db.GetTypedData<u8>(it);
	hfp._inputFineHZKST = db.GetTypedData<u8>(it);

	hfp._blockScheme = getDspBlockScheme( hfp._inputALG );

	hfp._blockName = "???";

	bool continuation_block = false;

	switch(code)
	{	case 0x50: // hobbes f1
		{	hfp._blockName = "F1";
			break;
		}
		case 0x51: // hobbes f2
		{	hfp._blockName = "F2";
			if( algCFG._w2==0 )
				continuation_block = true;
			break;
		}
		case 0x52:  // hobbes f3
		{	hfp._blockName = "F3";
			if( algCFG._w3==0 )
				continuation_block = true;
			break;
		}
		case 0x53: // hobbes f4 (amp)
		{	hfp._blockName = "F4AMP";
			if( algCFG._wa==0 )
				continuation_block = true;
			break;
		}
		default:
			assert(false);
	}

	auto rawalgschm = hfp._blockScheme;

	if( false==continuation_block )
	{
		hfp._algName = getDspBlockName( hfp._inputALG );

		std::vector<std::string> split_algs;
		SplitString( hfp._blockScheme, split_algs, " " );

		//printf( "algschm<%s>\n", hfp._blockScheme.c_str() );

		for( const auto& item : split_algs )
		{
			this->_algschmq.push(item);
			//printf( "split_algs<%s>\n", item.c_str() );
		}
	}

	if( this->_algschmq.size() )
	{
		hfp._blockScheme = this->_algschmq.front();
		this->_algschmq.pop();
		//printf( "algschmpop<%s>\n", hfp._blockScheme.c_str() );
	}
	else
	{
		hfp._blockScheme = "???";
	}

	switch( int(hfp._inputFiltAlg&3) )
	{
		case 0:
			hfp._outputPAD = 0; // dB
			break;
		case 1:
			hfp._outputPAD = -6; // dB
			break;
		case 2:
			hfp._outputPAD = -12; // dB
			break;
		case 3:
			hfp._outputPAD = -18; // dB
			break;
	}
	hfp._outputFiltAlg = formatString("%d",hfp._inputFiltAlg>>2);


	if( hfp._blockScheme == "AMP" )
		getFParamAMP(hfp);
	else if( hfp._blockScheme == "FRQ" )
		getFParamFRQ(hfp);
	else if( hfp._blockScheme == "DRV" )
		getFParamDRV(hfp);
	else if( hfp._blockScheme == "PCH" )
		getFParamPCH(hfp);
	else if( hfp._blockScheme == "POS" )
		getFParamPOS(hfp);
	else if( hfp._blockScheme == "RES" )
		getFParamRES(hfp);
	else if( hfp._blockScheme == "WRP" )
		getFParamWRP(hfp);
	else if( hfp._blockScheme == "DEP" )
		getFParamDEP(hfp);
	else if( hfp._blockScheme == "AMT" )
		getFParamAMT(hfp);
	else if( hfp._blockScheme == "WID" )
		getFParamWID(hfp);
	else if( hfp._blockScheme == "SEP" )
		getFParamSEP(hfp);
	else if( hfp._blockScheme == "EVN" )
		getFParamEVNODD(hfp);
	else if( hfp._blockScheme == "ODD" )
		getFParamEVNODD(hfp);
	else if( hfp._blockScheme == "XFD" )
		getFParamXFD(hfp);

	_algschmap[hfp._blockName] = hfp._blockScheme;

	const auto& blkname = hfp._blockName;

	hobbes->_ok2emit = (0!=_curALG);

	hobbes->_blockName = blkname;
}

///////////////////////////////////////////////////////////////////////////////

void filescanner::emitHobbes(const Hobbes* h, rapidjson::Value& parent)
{
	if( h->_ok2emit )
	{
		Value hseg(kObjectType);

		const auto& hfp = h->_hobbesFP;

		if( hfp._algName.length() )
			AddStringKVMember(hseg,"BLOCK_ALG ", hfp._algName );

		AddStringKVMember(hseg,"PARAM_SCHEME", hfp._blockScheme );

		fparamOutput(hfp,h->_blockName,hseg);
		AddMember(parent,h->_blockName,hseg);
	}
}

Hobbes::Hobbes()
	: _ok2emit(false)
{

}

