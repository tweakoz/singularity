
#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////

RateLevelEnvData::RateLevelEnvData(std::string n)
	: _name(n)
{
	
}

///////////////////////////////////////////////////////////////////////////////

layerData* programData::newLayer()
{
	auto l = new layerData;
	_layerDatas.push_back(l);
	return l;
}

///////////////////////////////////////////////////////////////////////////////

bool RateLevelEnvData::isBiPolar() const
{
	bool rval = false;
	for( auto& item : _segments )
		if( item._level<0.0f )
			rval = true;
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

LfoData::LfoData()
	: _initialPhase(0.0f)
	, _minRate(1.0f)
	, _maxRate(1.0f)
	, _shape("Sine")
{

}

///////////////////////////////////////////////////////////////////////////////

kmregion* keymap::getRegion(int note, int vel) const
{
	int k2vel = vel/18; // map 0..127 -> 0..7

	for( auto r : _regions )
	{
		//printf( "testing region<%p> for note<%d>\n", r, note );
		//printf( "lokey<%d>\n", r->_lokey );
		//printf( "hikey<%d>\n", r->_hikey );
		//printf( "lovel<%d>\n", r->_lovel );
		//printf( "hivel<%d>\n", r->_hivel );
		if( note >= r->_lokey && note <= r->_hikey )
		{
			if( k2vel >= r->_lovel && k2vel <= r->_hivel )
			{
				return r;
			}
		}
	}


	return nullptr;
}
