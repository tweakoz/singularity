#pragma once

#include "controller.h"
#include "filters.h"
#include "PolyBLEP.h"

float shaper(float inp, float adj);
float wrap(float inp, float adj);

struct outputBuffer;
struct layer;
struct DspBlock;

///////////////////////////////////////////////////////////////////////////////

struct dspBlockBuffer
{
    dspBlockBuffer();
    void resize(int inumframes);

    float* _upperBuffer;
    float* _lowerBuffer;
    int _maxframes;
    int _numframes;
};

///////////////////////////////////////////////////////////////////////////////

struct DspKeyOnInfo
{
    int _key;
    int _vel;
    DspBlock* _prv;
    layer* _layer;
};

///////////////////////////////////////////////////////////////////////////////

struct DspBlock
{
    DspBlock(const DspBlockData& dbd);
    void keyOn(const DspKeyOnInfo& koi);
    void keyOff(layer*l);

    virtual void compute(dspBlockBuffer& obuf) = 0;

    virtual void doKeyOn(const DspKeyOnInfo& koi) {}
    virtual void doKeyOff() {}
    const DspBlockData _dbd;
    int _baseIndex = -1;
    int _numParams = 0;
    int _numOutputs = 1;
    int _numInputs = 1;
    layer* _layer = nullptr;

    float _fval[3];
    FPARAM _ctrl[3];
};  

///////////////////////////////////////////////////////////////////////////////
// oscils
///////////////////////////////////////////////////////////////////////////////

struct SWPLUSSHP : public DspBlock
{
    SWPLUSSHP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct SAWPLUS : public DspBlock
{
    SAWPLUS( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct SINE : public DspBlock
{
    SINE( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct SAW : public DspBlock
{
    SAW( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct SQUARE : public DspBlock
{
    SQUARE( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct SINEPLUS : public DspBlock
{
    SINEPLUS( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    PolyBLEP _pblep;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};

///////////////////////////////////////////////////////////////////////////////
// nonlinear blocks
///////////////////////////////////////////////////////////////////////////////

struct AMP : public DspBlock
{
    AMP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _filt;
    float _upan, _lpan;
};
struct PLUSAMP : public DspBlock
{
    PLUSAMP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _filt;
};
struct XAMP : public DspBlock
{
    XAMP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _filt;
};
struct GAIN : public DspBlock
{
    GAIN( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    float _filt;
};

struct AMPU_AMPL : public DspBlock
{
    AMPU_AMPL( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _filtU, _filtL;
    float _upan, _lpan;
};

struct BAL_AMP : public DspBlock
{
    BAL_AMP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    float _filt;
};

struct XGAIN : public DspBlock
{
    XGAIN( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    float _filt;
};

struct XFADE : public DspBlock
{
    XFADE( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _pumix, _plmix; // for smoothing
};
struct PANNER : public DspBlock
{
    PANNER( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
    float _plmix, _prmix; // for smoothing
};

///////////////////////////////////////////////////////////////////////////////
// nonlinear blocks
///////////////////////////////////////////////////////////////////////////////

struct SHAPER : public DspBlock
{
    SHAPER( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
};
struct SHAPE2 : public DspBlock
{
    SHAPE2( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
};
struct TWOPARAM_SHAPER : public DspBlock
{
    TWOPARAM_SHAPER( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
    float ph1, ph2;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct WRAP : public DspBlock
{
    WRAP( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
};
struct DIST : public DspBlock
{
    DIST( const DspBlockData& dbd );
    void compute(dspBlockBuffer& obuf) final;
};

///////////////////////////////////////////////////////////////////////////////
// filter blocks
///////////////////////////////////////////////////////////////////////////////

struct PARABASS : public DspBlock
{
    PARABASS( const DspBlockData& dbd );
    BiQuad _biquad;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct PARAMID : public DspBlock
{
    PARAMID( const DspBlockData& dbd );
    BiQuad _biquad;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct PARATREBLE : public DspBlock
{
    PARATREBLE( const DspBlockData& dbd );
    BiQuad _biquad;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct PARAMETRIC_EQ : public DspBlock
{
    PARAMETRIC_EQ( const DspBlockData& dbd );
    BiQuad _biquad;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct BANDPASS_FILT : public DspBlock
{
    BANDPASS_FILT( const DspBlockData& dbd );
    TrapSVF _filter;
    BiQuad _biquad;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct BAND2 : public DspBlock
{
    BAND2( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct NOTCH_FILT : public DspBlock
{
    NOTCH_FILT( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct NOTCH2 : public DspBlock
{
    NOTCH2( const DspBlockData& dbd );
    TrapSVF _filter1;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct DOUBLE_NOTCH_W_SEP : public DspBlock
{
    DOUBLE_NOTCH_W_SEP( const DspBlockData& dbd );
    TrapSVF _filter1;
    TrapSVF _filter2;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct TWOPOLE_LOWPASS : public DspBlock
{
    TWOPOLE_LOWPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    float _smoothFC;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct LOPAS2 : public DspBlock
{   LOPAS2( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct LP2RES : public DspBlock
{   LP2RES( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct FOURPOLE_LOPASS_W_SEP : public DspBlock
{
    FOURPOLE_LOPASS_W_SEP( const DspBlockData& dbd );
    TrapSVF _filter1;
    TrapSVF _filter2;
    float _filtFC;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct FOURPOLE_HIPASS_W_SEP : public DspBlock
{
    FOURPOLE_HIPASS_W_SEP( const DspBlockData& dbd );
    TrapSVF _filter1;
    TrapSVF _filter2;
    float _filtFC;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct LOPASS : public DspBlock
{   LOPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct HIPASS : public DspBlock
{   HIPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct ALPASS : public DspBlock
{   
    ALPASS( const DspBlockData& dbd );
    TrapAllpass _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct TWOPOLE_ALLPASS : public DspBlock
{
    TWOPOLE_ALLPASS( const DspBlockData& dbd );
    TrapAllpass _filterL;
    TrapAllpass _filterH;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};
struct HIFREQ_STIMULATOR : public DspBlock
{
    HIFREQ_STIMULATOR( const DspBlockData& dbd );
    TrapSVF _filter1;
    TrapSVF _filter2;
    float _smoothFC;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(const DspKeyOnInfo& koi) final;
};

///////////////////////////////////////////////////////////////////////////////

struct Alg 
{   
    Alg();
    void keyOn(DspKeyOnInfo& koi);
    void keyOff();

    virtual bool hasPitchBlock() const { return true; }
    virtual int f1BlockCount() const { return 0; }
    virtual int f2BlockCount() const { return 0; }
    virtual int f3BlockCount() const { return 0; }
    virtual int f4BlockCount() const { return 0; }
    virtual void compute(outputBuffer& obuf);
    virtual void doKeyOn(DspKeyOnInfo& koi) {}

    void intoDspBuf(const outputBuffer& obuf, dspBlockBuffer& dspbuf);
    void intoOutBuf(outputBuffer& obuf, const dspBlockBuffer& dspbuf);
    DspBlock* lastBlock() const;

    DspBlock* _block[4];

    dspBlockBuffer _blockBuf;
};

struct Alg1 : public Alg
{
    int f1BlockCount() const final { return 3; }

};
struct Alg2 : public Alg
{
    int f1BlockCount() const final { return 2; }
    int f3BlockCount() const final { return 1; }
    void doKeyOn(DspKeyOnInfo& koi) final;
    void compute(outputBuffer& obuf) final;
};
struct Alg24 : public Alg
{
    int f1BlockCount() const final { return 1; }
    int f3BlockCount() const final { return 1; }
    void doKeyOn(DspKeyOnInfo& koi) final;
    void compute(outputBuffer& obuf) final;
};
struct Alg3 : public Alg
{
    int f1BlockCount() const final { return 2; }
    int f3BlockCount() const final { return 2; }
    void compute(outputBuffer& obuf) final;
};