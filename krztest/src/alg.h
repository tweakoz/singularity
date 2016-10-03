#pragma once

#include "controller.h"
#include "filters.h"

float shaper(float inp, float adj);
float wrap(float inp, float adj);

struct outputBuffer;
struct layer;

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

struct DspBlock
{
    DspBlock(const DspBlockData& dbd);
    void keyOn(layer*l);
    void keyOff(layer*l);

    virtual void compute(dspBlockBuffer& obuf) = 0;

    virtual void doKeyOn(layer*l) {}
    virtual void doKeyOff() {}
    const DspBlockData _dbd;
    int _baseIndex = -1;
    int _numParams = 0;
    int _numOutputs = 1;
    int _numInputs = 1;
    
    float _fval[3];
    FPARAM _ctrl[3];
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

struct PARAMETRIC_EQ : public DspBlock
{
    PARAMETRIC_EQ( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct BANDPASS_FILT : public DspBlock
{
    BANDPASS_FILT( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct NOTCH_FILT : public DspBlock
{
    NOTCH_FILT( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct TWOPOLE_LOWPASS : public DspBlock
{
    TWOPOLE_LOWPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct LOPAS2 : public DspBlock
{   LOPAS2( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct LP2RES : public DspBlock
{   LP2RES( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct FOURPOLE_LOPASS_W_SEP : public DspBlock
{
    FOURPOLE_LOPASS_W_SEP( const DspBlockData& dbd );
    TrapSVF _filter1;
    TrapSVF _filter2;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct LOPASS : public DspBlock
{   LOPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct HIPASS : public DspBlock
{   HIPASS( const DspBlockData& dbd );
    TrapSVF _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};
struct ALPASS : public DspBlock
{   
    ALPASS( const DspBlockData& dbd );
    TrapAllpass _filter;
    void compute(dspBlockBuffer& obuf) final;
    void doKeyOn(layer*l) final;
};

///////////////////////////////////////////////////////////////////////////////

struct Alg 
{   
    Alg();
    void keyOn(layer*l);
    void keyOff();

    virtual bool hasPitchBlock() const { return true; }
    virtual int f1BlockCount() const { return 0; }
    virtual int f2BlockCount() const { return 0; }
    virtual int f3BlockCount() const { return 0; }
    virtual void compute(outputBuffer& obuf);

    void intoDspBuf(const outputBuffer& obuf, dspBlockBuffer& dspbuf);
    void intoOutBuf(outputBuffer& obuf, const dspBlockBuffer& dspbuf);
    DspBlock* lastBlock() const;

    DspBlock* _block[3];

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
    void compute(outputBuffer& obuf) final;
};
struct Alg3 : public Alg
{
    int f1BlockCount() const final { return 2; }
    int f3BlockCount() const final { return 2; }
    void compute(outputBuffer& obuf) final;
};