#pragma once

#include "krztypes.h"
#include "controller.h"

///////////////////////////////////////////////////////////////////////////////

struct Alg;
struct AlgData
{
    int _algID = -1;
    std::string _name;
    std::string _blkF1;
    std::string _blkF2;
    std::string _blkF3;
    std::string _blkF4AMP;

   Alg* createAlgInst() const;
};

///////////////////////////////////////////////////////////////////////////////

struct EnvPoint
{
    float _rate;
    float _level;
};

///////////////////////////////////////////////////////////////////////////////

struct RateLevelEnvData 
{
    RateLevelEnvData(std::string n="");
    std::vector<EnvPoint> _segments;
    std::string _name;
    bool isBiPolar() const;
};

///////////////////////////////////////////////////////////////////////////////

struct natenvseg
{
    float _slope;
    float _time;
};

///////////////////////////////////////////////////////////////////////////////

struct AsrData
{
    std::string _name;
    std::string _trigger;
    std::string _mode;
    float _delay;
    float _attack;
    float _release;
};
///////////////////////////////////////////////////////////////////////////////

struct sample
{
    sample();

    std::string _name;
    s16* _data;
    
    int _blk_start;
    int _blk_alt;
    int _blk_loop;
    int _blk_end;

    int  _numFrames;
    int _loopPoint;
    int _subid;
    float _sampleRate;
    float _linGain;
    int _rootKey;
    int _highestPitch;
    int _loopMode = 0;
    std::vector<natenvseg> _natenv;

    int _pitchAdjust = 0;

    //void load(const std::string& fname);
};

///////////////////////////////////////////////////////////////////////////////

struct multisample
{
    std::string _name;
    int _objid;
    std::map<int,sample*> _samples;
};

///////////////////////////////////////////////////////////////////////////////

struct kmregion
{
    int _lokey, _hikey;
    int _lovel, _hivel;
    int _tuning;
    float _volAdj;
    float _linGain;
    int _multsampID, _sampID;
    std::string _sampleName;
    const multisample* _multiSample;
    const sample* _sample;
};

///////////////////////////////////////////////////////////////////////////////

struct keymap
{
    std::string _name;
    std::vector<kmregion*> _regions;
    int _kmID;

    kmregion* getRegion(int note, int vel) const;

};

///////////////////////////////////////////////////////////////////////////////

struct LfoData
{
    LfoData();
    std::string _name;
    float _initialPhase;
    float _minRate;
    float _maxRate;
    std::string _controller;
    std::string _shape;
};

///////////////////////////////////////////////////////////////////////////////

struct FunData
{
    std::string _a, _b, _op, _name;
};

///////////////////////////////////////////////////////////////////////////////

struct BlockModulationData
{
    std::string _src1 = "OFF";
    std::string _src2 = "OFF";
    std::string _src2DepthCtrl = "OFF";

    float _src1Depth = 0.0f;
    float _src2MinDepth = 0.0f;
    float _src2MaxDepth = 0.0f;
    evalit_t _evaluator = [](float c,float,float)->float{return c;};

};

///////////////////////////////////////////////////////////////////////////////

struct KmpBlockData
{
    const keymap* _keymap;
    int _transpose = 0;
    float _keyTrack = 100.0f;
    float _velTrack = 0.0f;
    int _timbreShift = 0;
    std::string _pbMode;

};

///////////////////////////////////////////////////////////////////////////////

struct FBlockData
{
    std::string _units;
    float _coarse = 0.0f;
    float _keyTrack = 0.0f;
    float _velTrack = 0.0f;
    BlockModulationData _mods;
};

///////////////////////////////////////////////////////////////////////////////

struct PchBlockData : public FBlockData
{
    float _fine = 0.0f;
    float _fineHZ = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////

struct DspBlockData : public FBlockData
{
    std::string _dspBlock;
    std::string _paramScheme;
    float _pad = 1.0f;
    int _blockIndex = -1;
};

///////////////////////////////////////////////////////////////////////////////

struct EnvCtrlData
{
    bool _useNatEnv = true;
    float _atkAdjust = 1.0f;
    float _decAdjust = 1.0f;
    float _relAdjust = 1.0f;

    float _atkKeyTrack = 1.0f;
    float _atkVelTrack = 1.0f;
    float _decKeyTrack = 1.0f;
    float _decVelTrack = 1.0f;
    float _relKeyTrack = 1.0f;
    float _relVelTrack = 1.0f;
};
///////////////////////////////////////////////////////////////////////////////

struct layerData
{
    layerData() {}

    AlgData _algData;

    EnvCtrlData _envCtrlData;

    KmpBlockData _kmpBlock;
    PchBlockData _pchBlock;
    DspBlockData _f1Block;
    DspBlockData _f2Block;
    DspBlockData _f3Block;
    DspBlockData _f4Block;

    const keymap* _keymap = nullptr;
    int _loKey = 0;
    int _hiKey = 127;
    int _loVel = 0;
    int _hiVel = 127;
    float _outputGain = 1.0f;
    bool _ignRels = false;
    bool _atk1Hold = false; // ThrAtt
    bool _atk3Hold = false; // TilDec

    RateLevelEnvData* _userAmpEnv = nullptr;
    RateLevelEnvData* _env2 = nullptr;
    RateLevelEnvData* _env3 = nullptr;

    const FunData* _fun1 = nullptr;
    const FunData* _fun2 = nullptr;
    const FunData* _fun3 = nullptr;
    const FunData* _fun4 = nullptr;

    const LfoData* _lfo1 = nullptr;
    const LfoData* _lfo2 = nullptr;

    const AsrData* _asr1 = nullptr;
    const AsrData* _asr2 = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

struct programData
{
    layerData* newLayer();
    layerData* getLayer(int i) const { return _layerDatas[i]; }
    std::string _name;
    std::string _role;
    std::vector<layerData*> _layerDatas;
};

///////////////////////////////////////////////////////////////////////////////

struct programInst;

struct SynthData
{
    SynthData(synth* syn);
    float seqTime(float dur);
    const programData* getProgram(int progID);
    const programData* getKmTestProgram(int kmID);
    const programData* getTestProgram(int progID);

    void addEvent(float time, void_lamda_t ev);
    void tick(float dt);
    void genTestPrograms();

    std::multimap<float,void_lamda_t> _eventmap;
    programInst* _prog;
    float _synsr;
    std::map<int,programData*> _testKmPrograms;
    std::vector<programData*> _testPrograms;
    VastObjectsDB* _objects;
    synth* _syn;
    float _timeaccum;
    float _seqCursor;

};

