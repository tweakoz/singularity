#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////

sample::sample()
	: _data(nullptr)
	, _numFrames(0)
	, _loopPoint(0)
	, _subid(0)
	, _sampleRate(0.0f)
	, _rootKey(0)
	, _loopMode(0)
	, _linGain(1.0f)
	, _highestPitch(0)
{

}

///////////////////////////////////////////////////////////////////////////////
/*
void sample::load(const std::string& fname)
{
	printf( "loading sample<%s>\n", fname.c_str() );

	auto af_file = afOpenFile(fname.c_str(), "r", nullptr);


	_numFrames = afGetFrameCount(af_file, AF_DEFAULT_TRACK);
	float frameSize = afGetVirtualFrameSize(af_file, AF_DEFAULT_TRACK, 1);
	int channelCount = afGetVirtualChannels(af_file, AF_DEFAULT_TRACK);
	_sampleRate = afGetRate(af_file, AF_DEFAULT_TRACK);

	int sampleFormat, sampleWidth;

	afGetVirtualSampleFormat(af_file, AF_DEFAULT_TRACK, &sampleFormat,
		&sampleWidth);

	int numbytes = _numFrames*frameSize;

	printf( "frameCount<%d>\n", _numFrames );
	printf( "frameSize<%f>\n", frameSize );
	printf( "numbytes<%d>\n", numbytes );


	auto buffer = malloc(numbytes);
	int count = afReadFrames(af_file, AF_DEFAULT_TRACK, buffer, _numFrames);
	printf( "readcount<%d>\n", count );

	_data = (s16*) buffer;

	////////////////////////////
	// get loop
	////////////////////////////

	int numloopids = afGetLoopIDs(af_file, AF_DEFAULT_INST, NULL);
	printf( "numloopids<%d>\n", numloopids );
	
	auto loopids = new int[numloopids];
	afGetLoopIDs(af_file, AF_DEFAULT_INST, loopids);
	for (int i=0; i<numloopids; i++)
		printf( "loopid<%d> : %d\n", i, loopids[i] );

	int nummkrids = afGetMarkIDs(af_file, AF_DEFAULT_TRACK, NULL);
	printf( "nummkrids<%d>\n", nummkrids );
	auto mkrids = new int[nummkrids];
	afGetMarkIDs(af_file, AF_DEFAULT_TRACK, mkrids);
	for (int i=0; i<nummkrids; i++)
	{
		const char* mkrname = afGetMarkName(af_file, AF_DEFAULT_TRACK, mkrids[i]);
		auto mkrpos = afGetMarkPosition(af_file, AF_DEFAULT_TRACK, mkrids[i]);

		printf( "mkrid<%d> : %d : pos<%d> name<%s>\n", i, mkrids[i], int(mkrpos), mkrname );
	}

	int lpa_startMKR = afGetLoopStart(af_file, AF_DEFAULT_INST, 1);
	int lpa_endMKR = afGetLoopEnd(af_file, AF_DEFAULT_INST, 1);

	printf( "loopa_markers start<%d> end<%d>\n", lpa_startMKR,lpa_endMKR );

	auto lptrack1 = afGetLoopTrack(af_file, AF_DEFAULT_INST, 1);

	int lpstartfr = afGetLoopStartFrame(af_file, AF_DEFAULT_INST, 1);
	int lpendfr = afGetLoopEndFrame(af_file, AF_DEFAULT_INST, 1);

	_loopPoint = lpstartfr;

	printf( "loopa_frames start<%d> end<%d>\n", lpstartfr,lpendfr );

	afCloseFile(af_file);

}*/

///////////////////////////////////////////////////////////////////////////////

sampleOsc::sampleOsc(synth& syn, layer& l)
	: _syn(syn)
	, _lyr(l)
	, _sample(nullptr)
	, _sampleData(getSoundBlock())
	, _active(false)
	, _pbindex(0.0f)
	, _pbindexNext(0.0f)
	, _pbincrem(0.0f)
	, _isLooped(false)
	, _curratio(1.0f)
{

}

void sampleOsc::setSrRatio(float pbratio)
{
	_curratio = pbratio;
	if( _sample )
	{
		_playbackRate = _sample->_sampleRate*_curratio;
		_pbincrem = (_dt*_playbackRate*65536.0f);
	}
	else
	{
		_playbackRate = 0.0f;
		_pbincrem = 0.0f;
	}
}

///////////////////////////////////////////////////////////////////////////////

void sampleOsc::keyOn(float pbratio)
{

	const layerData* ld = _lyr._layerData;

	assert(_sample);

	_blk_start = int64_t(_sample->_blk_start)<<16;
	_blk_alt = int64_t(_sample->_blk_alt)<<16;
	_blk_loop = int64_t(_sample->_blk_loop)<<16;
	_blk_end = int64_t(_sample->_blk_end)<<16;

	_pbindex = _blk_start;
	_pbindexNext = _blk_start;

	_isLooped = (_sample->_loopMode!=0); // && (_loopPoint<_numFrames);
	_pbincrem = 0;
	_dt = _syn._dt;

	_loopMode = _sample->_loopMode;

	_synsr = _syn._sampleRate;

	/*04-05 Pitch at Highest Playback Rate: unsigned word (affected by
           change in Root Key Number and Pitch Adjust) -- value is
           some kind of computed highest pitch plus Pitch Adjust
           entered on Sample Editor MISC page: value in cents*/



    setSrRatio(pbratio);

	//printf( "osc<%p> sroot<%d> SR<%d> ratio<%f> PBR<%d> looped<%d>\n", this, _sample->_rootKey, int(_sample->_sampleRate), _curratio, int(_playbackRate), int(_isLooped) );
	//printf( "sample<%s>\n", _sample->_name.c_str() );

	_active = true;

	_forwarddir = true;

	_loopCounter = 0;
	_released = false;
}

void sampleOsc::keyOff()
{
	_released = true;
	printf( "osc<%p> beginRelease\n", this );
}


///////////////////////////////////////////////////////////////////////////////

float sampleOsc::compute()
{
	if( false == _active ) return 0.0f;

	float lyrpocents = _lyr._curPitchOffsetInCents;

	//float ratio = cents_to_linear_freq_ratio(_keyoncents+lyrpocents);

	//printf( "curratio<%f>\n", _curratio );

	_playbackRate = _sample->_sampleRate*_curratio;
	
	_pbincrem = (_dt*_playbackRate*65536.0f);

	/////////////////////////////

	float sampleval = _isLooped 
					? (_loopMode==2)
					? sampleLoopedBid() 
					: sampleLoopedFwd() 
					: sampleNotLooped();

	return sampleval;
}

///////////////////////////////////////////////////////////////////////////////

float sampleOsc::sampleNotLooped()
{
	_pbindexNext = _pbindex + _pbincrem;

	///////////////

	double fract = double(int64_t(_pbindex*65536.0)&0xffff)/65536.0;
	double whole = _pbindex-fract;

	int iiA = int(whole);
	if( iiA >= (_blk_end-1) )
		iiA = _blk_end-1;

	int iiB = int(whole+1.0);
	if( iiB >= (_blk_end-1) )
		iiB = _blk_end-1;

	float sampA = float(_sampleData[iiA] );
	float sampB = float(_sampleData[iiB] );
	float samp = (sampB*fract+sampA*(1.0-fract))/32768.0;

	printf( "NL: iiA<%d> sampA<%f> iiB<%d> sampB<%f> samp<%f>\n", iiA, sampA, iiB, sampB, samp );

	///////////////

	_pbindex = _pbindexNext;

	return samp;
}

///////////////////////////////////////////////////////////////////////////////

float sampleOsc::sampleLoopedFwd()
{	
	_pbindexNext = _pbindex + _pbincrem;

	bool did_loop = false;

	if( (_pbindexNext>>16) > (_blk_end>>16) )
	{
		int64_t over = (_pbindexNext-_blk_end)-(1<<16);
		_pbindexNext = _blk_loop+over;
		did_loop = true;
		_loopCounter++;
	}

	///////////////

	float fract = float(_pbindex&0xffff)*kinv64k;
	float invfr = 1.0f-fract;

	///////////////

	int64_t iiA = (_pbindex>>16);

	int64_t iiB = iiA+1; 
	if( iiB > (_blk_end>>16) )
		iiB = (_blk_loop>>16);

	///////////////

	float sampA = float(_sampleData[iiA] );
	float sampB = float(_sampleData[iiB] );
	float samp = (sampB*fract+sampA*invfr)*kinv32k;

	///////////////

	_pbindex = _pbindexNext;

	return samp;
}

float sampleOsc::sampleLoopedBid()
{
	assert(false);
	return 0.0f;
	/*
	_pbindexNext = _forwarddir
	             ? _pbindex + _pbincrem
	             : _pbindex - _pbincrem;


	bool did_loop = false;

	if( _forwarddir && int(_pbindexNext) > (_numFrames-1) )
	{
		_pbindexNext = _pbindexNext - _pbincrem;
		printf( "LoopedBIDIR (F)->(B) : _loopPoint<%d> _numFrames<%d> _pbindexNext<%f>\n", (int)_loopPoint, _numFrames, _pbindexNext );

		did_loop = true;

		_forwarddir = false;

	}
	else if( (!_forwarddir) && int(_pbindexNext) < _loopPoint )
	{
		_pbindexNext = _pbindexNext + _pbincrem;
		printf( "LoopedBIDIR (B)->(F) : _loopPoint<%d> _numFrames<%d> _pbindexNext<%f>\n", (int)_loopPoint, _numFrames, _pbindexNext );

		did_loop = true;

		_forwarddir = true;
	}

	///////////////

	double fract = double(int64_t(_pbindex*65536.0)&0xffff)/65536.0;
	double whole = _pbindex-fract;

	int iiA = int(whole);
	if( iiA >= (_numFrames-1) )
	{
		if( _isLooped )
			iiA = _numFrames-2;
	}

	int iiB = int(whole+1.0);
	if( iiB >= _numFrames )
	{
		iiB = _loopPoint;
		printf( "yo\n");
	}

	assert(iiA<_numFrames);
	assert(iiB<_numFrames);
	float sampA = float(_sampleData[iiA] );
	float sampB = float(_sampleData[iiB] );

	//if( false == _forwarddir )
	//	std::swap(sampA,sampB);

	float samp = (sampB*fract+sampA*(1.0-fract))/32768.0;

	if( did_loop )
		printf( "iiA<%d> sampA<%f> iiB<%d> sampB<%f>\n", iiA, sampA, iiB, sampB );
	///////////////

	_pbindex = _pbindexNext;

	return samp;*/
}