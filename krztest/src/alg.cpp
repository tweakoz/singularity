#include "krztest.h"
#include <assert.h>
#include "filters.h"

DspBlock* createDspBlock( const DspBlockData& dbd );

///////////////////////////////////////////////////////////////////////////////


dspBlockBuffer::dspBlockBuffer()
    : _upperBuffer(nullptr)
    , _lowerBuffer(nullptr)
    , _maxframes(0)
    , _numframes(0)
{

}

///////////////////////////////////////////////////////////////////////////////

void dspBlockBuffer::resize(int inumframes)
{
    if( inumframes > _maxframes )
    {
        if( _upperBuffer ) delete[] _upperBuffer;
        if( _lowerBuffer ) delete[] _lowerBuffer;
        _upperBuffer = new float[inumframes];
        _lowerBuffer = new float[inumframes];
        _maxframes = inumframes;
    }
    _numframes = inumframes;

}

///////////////////////////////////////////////////////////////////////////////

DspBlock::DspBlock(const DspBlockData& dbd) 
    : _dbd(dbd)
    , _baseIndex(dbd._blockIndex)
{

}

void DspBlock::keyOn(layer*l)
{
    for( int i=0; i<_numParams; i++ )
        _ctrl[i] = l->_fp[_baseIndex+i];

    doKeyOn(l);
}

///////////////////////////////////////////////////////////////////////////////

struct PANNER : public DspBlock
{   
    PANNER( const DspBlockData& dbd )
        :DspBlock(dbd)
    {   _numParams = 1;
        _numOutputs = 2;
    }
    float _plmix, _prmix;
    void compute(dspBlockBuffer& obuf) final
    {
        int inumframes = obuf._numframes;
        float* ubuf = obuf._upperBuffer;
        float* lbuf = obuf._lowerBuffer;
        float pos = _ctrl[0].eval();
        float pan = pos*0.01f;
        float lmix = (pan>0) 
                   ? lerp(0.5,0,pan)
                   : lerp(0.5,1,-pan);
        float rmix = (pan>0) 
                   ? lerp(0.5,1,pan)
                   : lerp(0.5,0,-pan);

        _fval[0] = pos;
        //_fval[1] = lmix;
        //_fval[2] = rmix;
        //printf( "pan<%f> lmix<%f> rmix<%f>\n", pan, lmix, rmix );
        if(1)for( int i=0; i<inumframes; i++ )
        {
            float input = ubuf[i];
            _plmix = _plmix*0.995f+lmix*0.005f;
            _prmix = _prmix*0.995f+rmix*0.005f;

            ubuf[i] = input*_plmix;
            lbuf[i] = input*_prmix;
        }
    }
    void doKeyOn(layer*l) final
    {   _plmix = 0.0f;
        _prmix = 0.0f;
    }
};

struct AMPU_AMPL : public DspBlock
{   
    AMPU_AMPL( const DspBlockData& dbd ):DspBlock(dbd){}
    FPARAM _controlAMPU;
    void compute(dspBlockBuffer& obuf) final
    {
        int inumframes = obuf._numframes;
        float* lbuf = obuf._upperBuffer;
        float* rbuf = obuf._lowerBuffer;
        float ampu = _controlAMPU.eval(true)*0.01f;

        if(0)for( int i=0; i<inumframes; i++ )
        {
            float input = lbuf[i];

            lbuf[i] = input;//*lmix;
            rbuf[i] = input;//*rmix;
        }
    }
    void doKeyOn(layer*l) final
    {   _controlAMPU = l->_fp[_baseIndex];
    }
};

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOn(layer*l)
{
    assert(l!=nullptr);

    for( int i=0; i<3; i++ )
        _block[i] = nullptr;

    const auto ld = l->_layerData;    
    const auto& F1D = ld->_f1Block;
    const auto& F2D = ld->_f2Block;
    const auto& F3D = ld->_f3Block;
    if( F1D._dspBlock.length() )
    {
        _block[0] = createDspBlock(F1D);
        if( _block[0] )
            _block[0]->keyOn(l);
        printf( "createF1<%s>\n",F1D._dspBlock.c_str() );
    }
    if( F2D._dspBlock.length() )
    {
        _block[1] = createDspBlock(F2D);
        if( _block[1] )
            _block[1]->keyOn(l);
        printf( "createF2<%s>\n",F2D._dspBlock.c_str() );
    }
    if( F3D._dspBlock.length() )
    {
        _block[2] = createDspBlock(F3D);
        if( _block[2] )
            _block[2]->keyOn(l);
        printf( "createF3<%s>\n",F3D._dspBlock.c_str() );
    }
    //const auto& F1D = ld->_ampBlock;


}

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOff()
{
    for( int i=0; i<3; i++ )
    {
        auto b = _block[i];
        if( b ) 
            b->doKeyOff();
        //_block[i] = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Alg::intoDspBuf(const outputBuffer& obuf, dspBlockBuffer& dspbuf)
{
    int inumframes = obuf._numframes;
    _blockBuf.resize(inumframes);
    float* lefbuf = obuf._leftBuffer;
    float* rhtbuf = obuf._rightBuffer;
    float* uprbuf = _blockBuf._upperBuffer;
    float* lwrbuf = _blockBuf._lowerBuffer;
    memcpy( uprbuf, lefbuf, inumframes*4 );
    memcpy( lwrbuf, rhtbuf, inumframes*4 );
}
void Alg::intoOutBuf(outputBuffer& obuf, const dspBlockBuffer& dspbuf)
{
    int inumframes = obuf._numframes;
    _blockBuf.resize(inumframes);
    float* lefbuf = obuf._leftBuffer;
    float* rhtbuf = obuf._rightBuffer;
    float* uprbuf = _blockBuf._upperBuffer;
    float* lwrbuf = _blockBuf._lowerBuffer;
    memcpy( lefbuf, uprbuf, inumframes*4 );
    memcpy( rhtbuf, lwrbuf, inumframes*4 );
}

void Alg::compute(outputBuffer& obuf)
{
    intoDspBuf(obuf,_blockBuf);

    bool touched = false;

    for( int i=0; i<3; i++ )
    {
        auto b = _block[i];
        if( b )
        {   b->compute(_blockBuf);
            touched = true;
        }
    }
    if( touched )
    {
        intoOutBuf(obuf,_blockBuf);

    }
}

void Alg2::compute(outputBuffer& obuf) // final
{
    intoDspBuf(obuf,_blockBuf);
    auto b0 = _block[0];
    auto b1 = _block[2];
    if( b0 )
        b0->compute(_blockBuf);
    if( b1 )
        b1->compute(_blockBuf);
    intoOutBuf(obuf,_blockBuf);
} 
void Alg3::compute(outputBuffer& obuf) // final
{

}

///////////////////////////////////////////////////////////////////////////////

Alg* AlgData::createAlgInst() const
{
    printf( "create Alg<%d>\n", _algID );
    switch( _algID )
    {
        case 1:
            return new Alg1;
            break;
        case 2:
            return new Alg2;
            break;
        case 3:
            return new Alg3;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            return new Alg1;        
            break;
        default:
            return new Alg1;
            break;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

DspBlock* createDspBlock( const DspBlockData& dbd )
{
    DspBlock* rval = nullptr;

    if( dbd._dspBlock == "PARAMETRIC EQ" )
        rval = new PARAMETRIC_EQ(dbd);
    if( dbd._dspBlock == "4POLE LOPASS W/SEP" )
        rval = new FOURPOLE_LOPASS_W_SEP(dbd);
    if( dbd._dspBlock == "NOTCH FILTER" )
        rval = new NOTCH_FILT(dbd);
    if( dbd._dspBlock == "BANDPASS FILT" )
        rval = new BANDPASS_FILT(dbd);
    if( dbd._dspBlock == "2POLE LOWPASS" )
        rval = new TWOPOLE_LOWPASS(dbd);
    if( dbd._dspBlock == "LOPAS2" )
        rval = new LOPAS2(dbd);
    if( dbd._dspBlock == "LP2RES" )
        rval = new LOPAS2(dbd);
    if( dbd._dspBlock == "SHAPER" )
        rval = new SHAPER(dbd);
    if( dbd._dspBlock == "2PARAM SHAPER" )
        rval = new TWOPARAM_SHAPER(dbd);
    if( dbd._dspBlock == "WRAP" )
        rval = new WRAP(dbd);
    if( dbd._dspBlock == "DIST" )
        rval = new DIST(dbd);
    if( dbd._dspBlock == "LOPASS" )
        rval = new LOPASS(dbd);
    if( dbd._dspBlock == "HIPASS" )
        rval = new HIPASS(dbd);
    if( dbd._dspBlock == "ALPASS" )
        rval = new ALPASS(dbd);
    if( dbd._dspBlock == "PANNER" )
        rval = new PANNER(dbd);
    if( dbd._dspBlock == "AMP U  AMPL" )
        rval = new AMPU_AMPL(dbd);

    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Alg::Alg()
{
    for( int i=0; i<3; i++ )
        _block[i] = nullptr;
}

DspBlock* Alg::lastBlock() const
{
    DspBlock* r = nullptr;
    for( int i=0; i<3; i++ )
        if( _block[i] )
            r = _block[i];
    return r;
}
