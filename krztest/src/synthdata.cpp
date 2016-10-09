#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krzdata.h"
#include "krztest.h"
#include "objectdb.h"

///////////////////////////////////////////////////////////////////////////////

float SynthData::seqTime(float dur)
{
	float rval = _seqCursor;
	_seqCursor += dur;
	return rval;

}

///////////////////////////////////////////////////////////////////////////////

const programData* SynthData::getProgram(int progID)
{
	auto ObjDB = _syn->_objectDB;
	return ObjDB->findProgram(progID);
}

///////////////////////////////////////////////////////////////////////////////

const programData* SynthData::getKmTestProgram(int kmID)
{
	programData* rval = nullptr;
	auto it = _testKmPrograms.find(kmID);
	if( it == _testKmPrograms.end() )
	{
		auto ObjDB = _syn->_objectDB;
		rval = new programData;
		rval->_role = "KmTest";
		auto km = ObjDB->findKeymap(kmID);
		if( km )
		{
			auto lyr = rval->newLayer();
			lyr->_keymap = km;
			//lyr->_useNatEnv = false;
			rval->_name = formatString("%s", km->_name.c_str());
		}
		else
			rval->_name = formatString("\?\?\?\?");

	}
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

const programData* SynthData::getTestProgram(int progid)
{
	int inumtests = _testPrograms.size();
	int testid = progid%inumtests;
	printf( "test<%d>\n", testid );
	auto test = _testPrograms[testid];
	return test;
}


///////////////////////////////////////////////////////////////////////////////

SynthData::SynthData(synth* syn)
	: _syn(syn)
	, _timeaccum(0.0f)
	, _seqCursor(0.0f)
{
	_synsr = syn->_sampleRate;

	auto ObjDB = syn->_objectDB;

	ObjDB->loadJson("krzdump.json");
	_objects = ObjDB;

	// C4 = 72
	float t1, t2;

	///////////////////////////////////////
	// timbreshift scan
	///////////////////////////////////////

	if( false ) for( int i=1; i<128; i++ )
	{
		auto prg = getKmTestProgram(1);
		auto l0 = prg->getLayer(0);

		for( int n=0; n<72; n+=1 )
		{
			for( int tst=0; tst<18; tst++ )
			{
				t1 = seqTime(.15f);

				addEvent( t1 ,[=]()
				{
					//l0->_km_timbreshift = (tst%9)*4;
					//l0->_km_transposeTS = (tst/9);
					auto pi = _syn->keyOn(12+n,prg);

					addEvent( t1+1.5 ,[=]()
					{
						_syn->keyOff(pi);
					} );
				});
			}
		}
	}

	///////////////////////////////////////
	// keymap scan
	///////////////////////////////////////

	if( false ) for( int i=1; i<128; i++ )
	{
		auto prg = getKmTestProgram(i);

		for( int n=0; n<48; n++ )
		{
			t1 = seqTime(0.15f);

			addEvent( t1 ,[=]()
			{
				auto pi = _syn->keyOn(24+n,prg);

				addEvent( t1+2 ,[=]()
				{
					_syn->keyOff(pi);
				} );
			});
		}
	}

	///////////////////////////////////////
	// program scan
	///////////////////////////////////////

	if( false ) for( int i=1; i<128; i++ )
	{
		for( int n=0; n<36; n+=1 )
		{
			t1 = seqTime(0.5f);
			//t2 = seqTime(0.1f);

			addEvent( t1 ,[=]()
			{
				auto prg = getProgram(i);
				auto pi = _syn->keyOn(48+n,prg);

				addEvent( t1+0.5 ,[=]()
				{
					_syn->keyOff(pi);
				} );
			});
		}
	}

	///////////////////////////////////////

	genTestPrograms();

	//_lpf.setup(330.0f,_synsr);
}

///////////////////////////////////////////////////////////////////////////////
void SynthData::addEvent(float time, void_lamda_t ev)

{
	_eventmap.insert(std::make_pair(time,ev));
}

///////////////////////////////////////////////////////////////////////////////

void SynthData::tick(float dt)
{
	bool done = false;
	while( false == done )
	{
		done = true;
		auto it = _eventmap.begin();
		if( it != _eventmap.end() && it->first <= _timeaccum )
		{
			auto& event = it->second;
			event();
			done = false;
			_eventmap.erase(it);
		}
	}
	_timeaccum += dt;
}

///////////////////////////////////////////////////////////////////////////////

void SynthData::genTestPrograms()
{
	auto t1 = new programData;
	t1->_role = "PrgTest";
	_testPrograms.push_back(t1);
	t1->_name = "YO";

	auto l1 = t1->newLayer();
	const int keymap_sine = 163;
	const int keymap_saw = 151;

	l1->_keymap = _objects->findKeymap(keymap_saw);

	auto& EC = l1->_envCtrlData;
	EC._useNatEnv = false;

	auto ampenv = new RateLevelEnvData("AMPENV");
	l1->_userAmpEnv = ampenv;
	auto& aesegs = ampenv->_segments;
	aesegs.push_back(EnvPoint{0,1});
	aesegs.push_back(EnvPoint{0,0});
	aesegs.push_back(EnvPoint{0,0});
	aesegs.push_back(EnvPoint{0,1});
	aesegs.push_back(EnvPoint{0,0});
	aesegs.push_back(EnvPoint{0,0});
	aesegs.push_back(EnvPoint{1,0});

	auto env2 = new RateLevelEnvData("ENV2`");
	l1->_env2 = env2;
	auto& e2segs = env2->_segments;
	e2segs.push_back(EnvPoint{2,1});
	e2segs.push_back(EnvPoint{0,1});
	e2segs.push_back(EnvPoint{0,1});
	e2segs.push_back(EnvPoint{0,0.5});
	e2segs.push_back(EnvPoint{0,0});
	e2segs.push_back(EnvPoint{0,0});
	e2segs.push_back(EnvPoint{1,0});

	auto env3 = new RateLevelEnvData("ENV3");
	l1->_env3 = env3;
	auto& e3segs = env3->_segments;
	e3segs.push_back(EnvPoint{8,1});
	e3segs.push_back(EnvPoint{0,1});
	e3segs.push_back(EnvPoint{0,1});
	e3segs.push_back(EnvPoint{0,0.5});
	e3segs.push_back(EnvPoint{0,0});
	e3segs.push_back(EnvPoint{0,0});
	e3segs.push_back(EnvPoint{1,0});

	auto lfo1 = new LfoData;
	lfo1->_name = "LFO1";
	l1->_lfo1 = lfo1;
	lfo1->_controller = "ON";
	lfo1->_maxRate = 0.1;

	auto& ALGD = l1->_algData;

	ALGD._name = "ALG1";
	ALGD._algID = 1;

	if( 1 )
	{
		auto& F1 = l1->_f1Block;
		F1._dspBlock = "2PARAM SHAPER";
		F1._paramScheme = "EVN";
		F1._blockIndex = 0;
		F1._units = "dB";
		F1._coarse = -60.0;
	}
	else if( 1 )
	{
		auto& F1 = l1->_f1Block;
		F1._blockIndex = 0;
		F1._dspBlock = "SHAPER";
		F1._paramScheme = "AMT";
		F1._units = "x";
		F1._coarse = 0.1;
	}
	if( 0 )
	{
		auto& F2 = l1->_f2Block;
		F2._paramScheme = "ODD";
		F2._blockIndex = 1;
		F2._units = "dB";
		F2._coarse = -96.0;
	}
}



