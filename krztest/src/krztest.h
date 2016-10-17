#pragma once

#include "krztypes.h"
#include "krzdata.h"
#include "layer.h"

///////////////////////////////////////////////////////////////////////////////

struct programInst
{
	programInst(synth& syn);
	~programInst();

	void keyOn( int note, const programData* pd );
	void keyOff();

	void compute();

	const programData* _progdata;
	synth& _syn;

	std::vector<layer*> _layers;
};

///////////////////////////////////////////////////////////////////////////////

struct SynthData;

struct hudsample
{
	float _time;
	float _value;
};

struct synth
{
	synth(float sr);
	~synth();

	typedef std::vector<hudsample> hudsamples_t;

	void compute(int inumframes,const void* inputbuffer);

	programInst* keyOn(int note, const programData* pd);
	void keyOff(programInst* p);

	layer* allocLayer();
	void freeLayer(layer* l);
	void deactivateVoices();

	void onDrawHud(float w, float h);
	void onDrawHudPage1(float w, float h);
	void onDrawHudPage2(float w, float h);
	void onDrawHudPage3(float w, float h);

	void resetFenables();

	outputBuffer _ibuf;
	outputBuffer _obuf;
	float _sampleRate;
	float _dt;

	std::set<layer*> _freeVoices;
	std::set<layer*> _activeVoices;
	std::queue<layer*> _deactiveateVoiceQ;
	std::set<programInst*> _freeProgInst;
	std::set<programInst*> _activeProgInst;
	std::map<std::string,hudsamples_t> _hudsample_map;

	int _soloLayer = -1;
	bool _fblockEnable[4] = { true, true, true, true };
	int _lnoteframe;
	float _lnotetime;
	float _testtonepi;
	float _testtoneph;
	float _testtoneamp;
	float _testtoneampps;
	int _hudpage;
	int _genmode = 0;
	float _ostrack = 0.0f;
	float _ostrackPH = 0.0f;
	bool _bypassDSP = false;
	bool _doModWheel = false;
	bool _doPressure = false;
	bool _doInput = false;
	float _masterGain = 0.25f;

	VastObjectsDB* _objectDB;

	SynthData* _SD = nullptr;
	layer* _hudLayer = nullptr;
	bool _clearhuddata = true;

};

///////////////////////////////////////////////////////////////////////////////

std::string formatString( const char* formatstring, ... );
void SplitString(const std::string& s, char delim, std::vector<std::string>& tokens);
std::vector<std::string> SplitString(const std::string& instr, char delim);


///////////////////////////////////////////////////////////////////////////////

float softsat(float x, float a);
float smoothstep(float edge0, float edge1, float x);
float log_base( float base, float inp );
float pow_base( float base, float inp );
int round_to_nearest( float in );
float linear_time_to_timecent( float time );
float timecent_to_linear_time( float timecent );
float decibel_to_linear_amp_ratio( float decibel );
float linear_amp_ratio_to_decibel( float linear );
float centibel_to_linear_amp_ratio( float centibel );
float linear_amp_ratio_to_centibel( float linear );
float linear_freq_ratio_to_cents( float freq_ratio );
float cents_to_linear_freq_ratio( float cents );
float midi_note_to_frequency( float midinote );
float frequency_to_midi_note( float frequency );
float clip_float( float in, float minn, float maxx );
float slopeDBPerSample(float dbpsec,float samplerate);
float lerp( float from,
			float to,
			float index );

void drawtext( const std::string& str, float x, float y, float scale, float r, float g, float b );

