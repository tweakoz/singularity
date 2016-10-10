#pragma once

#include <mutex>
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

struct sampleOsc
{
    sampleOsc(synth& syn, layer& l);
    void keyOn(float pbratio);
    void keyOff();

    void setSrRatio(float r);
    float compute();
    float sampleNotLooped();
    float sampleLoopedFwd();
    float sampleLoopedBid();

    //
    int64_t _blk_start;
    int64_t _blk_alt;
    int64_t _blk_loop;
    int64_t _blk_end;
    
    static constexpr float kinv64k = 1.0f/65536.0f;
    static constexpr float kinv32k = 1.0f/32768.0f;

    const sample* _sample;
    const s16* _sampleData;
    
    float _playbackRate;
    
    int64_t _pbindex;
    int64_t _pbindexNext;

    float _keyoncents;
    int64_t _pbincrem;

    float _dt;
    float _synsr;
    bool _isLooped;
    int _loopMode;
    bool _active;
    bool _forwarddir;
    bool _loopCounter;

    float _curratio;

    synth& _syn;
    layer& _lyr;
    bool _released;

};

///////////////////////////////////////////////////////////////////////////////

struct LfoInst
{
    LfoInst(synth& syn);

    void reset();
    void keyOn(const LfoData* data);

    void compute(float dt);
    float getval() const { return _curval; }

    const LfoData* _data;
    float _phaseInc;
    float _phase;
    float _currate;
    bool _enabled;
    float _curval;
    float _rateLerp;
    float _bias;
    mapper_t _mapper;

    synth& _syn;
};

///////////////////////////////////////////////////////////////////////////////

struct FunInst 
{
    FunInst();
    float compute(float dt);
    void keyOn(const layer* l, const FunData* data);
    void keyOff();

    const FunData* _data = nullptr;
    float _curval = 0.0f;
    controller_t _a;
    controller_t _b;
    controller_t _op;
};

///////////////////////////////////////////////////////////////////////////////

struct AsrInst 
{
    AsrInst();
    float compute();
    void keyOn(const layerData* ld, const AsrData* data);
    void keyOff();
    void initSeg(int iseg);
    bool isValid() const;
    const AsrData* _data;
    int _curseg;
    float _atkAdjust;
    float _relAdjust;
    float _curval;
    float _dstval;
    int _framesrem;
    bool _released;
    bool _ignoreRelease;
    float _curslope_persamp;
};

///////////////////////////////////////////////////////////////////////////////

struct RateLevelEnvInst 
{
    RateLevelEnvInst();
    float compute();
    void keyOn(int ikey, const layerData* ld, const RateLevelEnvData*data);
    void keyOff();
    void initSeg(int iseg);
    bool done() const;
    const RateLevelEnvData* _data;
    int _curseg;
    float _atkAdjust;
    float _decAdjust;
    float _relAdjust;
    float _curval;
    float _dstval;
    int _framesrem;
    bool _released;
    bool _ignoreRelease;
    float _curslope_persamp;
};

///////////////////////////////////////////////////////////////////////////////

struct NatEnv
{
    NatEnv(const synth& syn);
    void keyOn(int ikey, const layerData* ld,const sample* s);
    void keyOff();
    const natenvseg& getCurSeg() const;
    float compute();
    void initSeg(int iseg);
    bool done() const;

    std::vector<natenvseg> _natenv;
    int _curseg;
    int _prvseg;
    int _numseg;
    int _framesrem;
    float _segtime;
    float _curamp;
    float _slopePerSecond;
    float _slopePerSample;
    float _SR;
    bool _ignoreRelease;
    bool _released;
    float _atkAdjust;
    float _decAdjust;
    float _relAdjust;
};

///////////////////////////////////////////////////////////////////////////////

struct layer
{
    layer(synth& syn);

    void compute(outputBuffer& obuf);
    void keyOn( int note, int vel, const layerData* ld );
    void keyOff();
    void reset();
    controller_t getController(const std::string& n) const;

    //
    FPARAM initFPARAM(const FBlockData& fbd);

    controller_t getSRC1(const BlockModulationData& mods);
    controller_t getSRC2(const BlockModulationData& mods);

    void updateSampSRRatio();

    std::mutex _mutex;

    sampleOsc _spOsc;
    synth& _syn;
    int _curnote;
    int _ldindex;
    float _layerGain;
    float _baseCents;
    float _curSampSRratio;
    float _curPitchOffsetInCents;
    float _centsPerKey;
    float _FPA;
    int _sampleRoot;
    int _lyrPhase;
    int _phCounter; 
    bool _useNatEnv;
    bool _ignoreRelease;
    int _postdone;
    int _curcents;
    int _sampselnote;
    int _kmcents;
    int _pchcents;
    int _keydiff;
    int _curpitchadjx;
    int _curpitchadj;
    float _pchc1;
    float _pchc2;
    float _preDSPGAIN;
    float _postDSPGAIN;
    float _sinrepPH = 0.0f;
    bool _doNoise;
    float _masterGain = 0.0f;
    float _AENV[1024];

    FPARAM _fp[4];

    controller_t _pchControl1,_pchControl2;


    RateLevelEnvInst _userAmpEnv, _env2, _env3;
    NatEnv _natAmpEnv;
    LfoInst _lfo1, _lfo2;
    AsrInst _asr1, _asr2;
    FunInst _fun1, _fun2, _fun3, _fun4;
    
    Alg* _alg;
    
    outputBuffer _layerObuf;

    static const int koscopelength = 1024;
    float _oscopebuffer[koscopelength];
    float _fftbuffer[koscopelength/2];


    int _samppbnote;

    const layerData* _layerData;
    const kmregion* _kmregion;

};