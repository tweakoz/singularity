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
		rval->_iskmtest = true;
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

SynthData::SynthData(synth* syn)
	: _syn(syn)
	, _timeaccum(0.0f)
	, _seqCursor(0.0f)
{
	_synsr = syn->_sampleRate;

	auto ObjDB = syn->_objectDB;

	ObjDB->loadJson("krzdump.json");

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

/*
float compute()
{
	float s1 = _sosc1.sample();
	float s2 = _sosc2.sample();
	float s3 = _sosc3.sample();

	float adj = 0.5f-cosf(phase)*0.4f;
	float adj2 = 0.5f-cosf(phase*5.3f)*0.5f;
	float adj3 = 0.5f-cosf(phase*20	)*0.5f;
	float plfo3 = 0.5f-cosf(phase*0.3)*0.4f;

	//_lpf.setup(adj*3300.0f,_synsr);
	_lpf.setup(4000.0f*fmod(phase,1),_synsr);

	_notch.setup(880.0f*adj3*2.0f,0.8,_synsr);

	//printf( "phase<%f>\n", phase );
	//printf( "adj2<%f>\n", adj2 );
	s2 = shaper(s2,.9);
	s2 = wrap(s2,-25.25f+adj2*23.0f);
	s2 = _lpf.compute(s2)*0.6f;

	s3 = _notch.compute(s3);

	phase += 0.0000005f;

	return (s1+s2+s3)*0.33f;
	//return s3;
}*/


