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

	void compute(int inumframes);

	programInst* keyOn(int note, const programData* pd);
	void keyOff(programInst* p);

	layer* allocLayer();
	void freeLayer(layer* l);
	void deactivateVoices();

	void onDrawHud(float w, float h);
	void onDrawHudPage1(float w, float h);
	void onDrawHudPage2(float w, float h);
	void onDrawHudPage3(float w, float h);

	const programInst* _curProgram;
	outputBuffer _obuf;
	float _sampleRate;
	float _dt;

	std::set<layer*> _freeVoices;
	std::set<layer*> _activeVoices;
	std::queue<layer*> _deactiveateVoiceQ;
	std::set<programInst*> _freeProgInst;
	std::set<programInst*> _activeProgInst;
	int _soloLayer;
	std::map<std::string,hudsamples_t> _hudsample_map;
	int _lnoteframe;
	float _lnotetime;
	float _testtonepi;
	float _testtoneph;
	float _testtoneamp;
	float _testtoneampps;
	int _hudpage;
	bool _sinerep = false;
	float _ostrack = 0.0f;
	float _ostrackPH = 0.0f;
	bool _bypassDSP = false;
	bool _doModWheel = false;
	bool _doPressure = false;

	VastObjectsDB* _objectDB;

	SynthData* _SD = nullptr;
	layer* _hudLayer = nullptr;
	bool _clearhuddata = true;
	bool _testtone = false;

};

struct SynthData
{
	SynthData(synth* syn);
	float seqTime(float dur);
	const programData* getProgram(int progID);
	const programData* getKmTestProgram(int kmID);
	void addEvent(float time, void_lamda_t ev);
	void tick(float dt);

	std::multimap<float,void_lamda_t> _eventmap;
	programInst* _prog;
	float _synsr;
	std::map<int,programData*> _testKmPrograms;
    VastObjectsDB* _objects;
    synth* _syn;
    float _timeaccum;
    float _seqCursor;

};

///////////////////////////////////////////////////////////////////////////////

std::string formatString( const char* formatstring, ... );
void SplitString(const std::string& s, char delim, std::vector<std::string>& tokens);
std::vector<std::string> SplitString(const std::string& instr, char delim);


///////////////////////////////////////////////////////////////////////////////

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
