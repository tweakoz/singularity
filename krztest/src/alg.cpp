#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg_eq.h"

DspBlock* createDspBlock( const DspBlockData& dbd );

extern synth* the_synth;

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

void DspBlock::keyOn(const DspKeyOnInfo& koi)
{
    _layer = koi._layer;

    for( int i=0; i<_numParams; i++ )
    {
        _ctrl[i] = _layer->_fp[_baseIndex+i];
        _ctrl[i].keyOn(koi._key,koi._vel);
    }

    doKeyOn(koi);
}

///////////////////////////////////////////////////////////////////////////////

Alg::Alg()
{
    for( int i=0; i<4; i++ )
        _block[i] = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

DspBlock* Alg::lastBlock() const
{
    DspBlock* r = nullptr;
    for( int i=0; i<4; i++ )
        if( _block[i] )
            r = _block[i];
    return r;
}

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOn(DspKeyOnInfo& koi)
{
    auto l = koi._layer;
    assert(l!=nullptr);

    for( int i=0; i<4; i++ )
        _block[i] = nullptr;

    const auto ld = l->_layerData;    
    const auto& F1D = ld->_f1Block;
    const auto& F2D = ld->_f2Block;
    const auto& F3D = ld->_f3Block;
    const auto& F4D = ld->_f4Block;

    if( F1D._dspBlock.length() )
        _block[0] = createDspBlock(F1D);
    if( F2D._dspBlock.length() )
        _block[1] = createDspBlock(F2D);
    if( F3D._dspBlock.length() )
        _block[2] = createDspBlock(F3D);
    if( F4D._dspBlock.length() )
        _block[3] = createDspBlock(F4D);

    doKeyOn(koi);
}

void Alg::doKeyOn(DspKeyOnInfo& koi)
{
    auto l = koi._layer;
    assert(l!=nullptr);

    koi._prv = nullptr;
    int numoutputs = 1;
    auto procblock = [&numoutputs,&koi](DspBlock* thisblock,layer*l)
    {
        if( thisblock )
        {
            thisblock->keyOn(koi);
            //numoutputs = block->_numOutputs;
            koi._prv = thisblock;
        }


    };

    if( _block[0] )
        procblock( _block[0],l );
    if( _block[1] )
        procblock( _block[1],l );
    if( _block[2] )
        procblock( _block[2],l );
    if( _block[3] )
        procblock( _block[3],l );


}

void Alg10::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[1] )
        _block[1]->_useLowerInput = true;

    Alg::doKeyOn(koi);
}

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOff()
{
    for( int i=0; i<4; i++ )
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
    memcpy( lwrbuf, lefbuf, inumframes*4 );
}
void Alg::intoOutBuf(outputBuffer& obuf, const dspBlockBuffer& dspbuf,int inumo)
{
    int inumframes = obuf._numframes;
    _blockBuf.resize(inumframes);
    float* lefbuf = obuf._leftBuffer;
    float* rhtbuf = obuf._rightBuffer;
    float* uprbuf = _blockBuf._upperBuffer;
    float* lwrbuf = _blockBuf._lowerBuffer;
    if( inumo == 1 )
    {
        memcpy( lefbuf, uprbuf, inumframes*4 );
        memcpy( rhtbuf, uprbuf, inumframes*4 );
    }
    else if( inumo == 2)
    {
        memcpy( lefbuf, uprbuf, inumframes*4 );
        memcpy( rhtbuf, lwrbuf, inumframes*4 );
    }
}

void Alg::compute(outputBuffer& obuf)
{
    intoDspBuf(obuf,_blockBuf);

    bool touched = false;

    int inumoutputs = 1;

    for( int i=0; i<4; i++ )
    {
        auto b = _block[i];
        if( b )
        {   
            bool ena = the_synth->_fblockEnable[i];

            if( ena )
            {
                b->compute(_blockBuf);
                inumoutputs = b->_numOutputs;
                touched = true;
            }
        }
    }

    intoOutBuf(obuf,_blockBuf,inumoutputs);
}

void Alg2::doKeyOn(DspKeyOnInfo& koi) // final
{
    Alg::doKeyOn(koi);
    /*if( _block[2] )
        _block[2]->_numOutputs=2;
    if( _block[3] )
    {
        _block[3]->_numInputs=2;
        _block[3]->_numOutputs=2;
    }*/
}

/*void Alg2::compute(outputBuffer& obuf) // final
{
    intoDspBuf(obuf,_blockBuf);
    auto b1 = _block[0];
    auto b2 = _block[1];
    auto b3 = _block[2];
    auto b4 = _block[3];
    if( b1 && the_synth->_fblockEnable[0] )
        b1->compute(_blockBuf);
    if( b2 && the_synth->_fblockEnable[1] )
        b2->compute(_blockBuf);
    if( b3 && the_synth->_fblockEnable[2] )
        b3->compute(_blockBuf);
    if( b4 && the_synth->_fblockEnable[3] )
        b4->compute(_blockBuf);
    intoOutBuf(obuf,_blockBuf);
} */
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
            return new Alg1;
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

    ////////////////////////
    // amp/mix
    ////////////////////////

    if( dbd._dspBlock == "XFADE")
        rval = new XFADE(dbd);
    if( dbd._dspBlock == "x GAIN")
        rval = new XGAIN(dbd);
    if( dbd._dspBlock == "GAIN")
        rval = new GAIN(dbd);
    if( dbd._dspBlock == "AMP")
        rval = new AMP(dbd);
    if( dbd._dspBlock == "+ AMP")
        rval = new PLUSAMP(dbd);
    if( dbd._dspBlock == "x AMP")
        rval = new XAMP(dbd);
    if( dbd._dspBlock == "PANNER" )
        rval = new PANNER(dbd);
    if( dbd._dspBlock == "AMP U   AMP L")
        rval = new AMPU_AMPL(dbd);
    if( dbd._dspBlock == "! AMP" )
        rval = new BANGAMP(dbd);

    ////////////////////////
    // osc/gen
    ////////////////////////

    if( dbd._dspBlock == "SINE")
        rval = new SINE(dbd);
    if( dbd._dspBlock == "LF SIN")
        rval = new SINE(dbd);
    if( dbd._dspBlock == "SAW")
        rval = new SAW(dbd);
    if( dbd._dspBlock == "SQUARE")
        rval = new SQUARE(dbd);
    if( dbd._dspBlock == "SINE+")
        rval = new SINEPLUS(dbd);
    if( dbd._dspBlock == "SAW+")
        rval = new SAWPLUS(dbd);
    if( dbd._dspBlock == "SW+SHP" )
        rval = new SWPLUSSHP(dbd);
    if( dbd._dspBlock == "+ SHAPEMOD OSC" )
        rval = new PLUSSHAPEMODOSC(dbd);
    if( dbd._dspBlock == "SHAPE MOD OSC" )
        rval = new SHAPEMODOSC(dbd);



    ////////////////////////
    // EQ
    ////////////////////////

    if( dbd._dspBlock == "PARA BASS" )
        rval = new PARABASS(dbd);
    if( dbd._dspBlock == "PARA MID" )
        rval = new PARAMID(dbd);
    if( dbd._dspBlock == "PARA TREBLE" )
        rval = new PARATREBLE(dbd);
    if( dbd._dspBlock == "PARAMETRIC EQ" )
        rval = new PARAMETRIC_EQ(dbd);

    ////////////////////////
    // filter
    ////////////////////////

    if( dbd._dspBlock == "2POLE ALLPASS" )
        rval = new TWOPOLE_ALLPASS(dbd);
    if( dbd._dspBlock == "2POLE LOWPASS" )
        rval = new TWOPOLE_LOWPASS(dbd);

    if( dbd._dspBlock == "STEEP RESONANT BASS" )
        rval = new STEEP_RESONANT_BASS(dbd);
    if( dbd._dspBlock == "4POLE LOPASS W/SEP" )
        rval = new FOURPOLE_LOPASS_W_SEP(dbd);
    if( dbd._dspBlock == "4POLE HIPASS W/SEP" )
        rval = new FOURPOLE_HIPASS_W_SEP(dbd);
    if( dbd._dspBlock == "NOTCH FILTER" )
        rval = new NOTCH_FILT(dbd);
    if( dbd._dspBlock == "NOTCH2" )
        rval = new NOTCH2(dbd);
    if( dbd._dspBlock == "DOUBLE NOTCH W/SEP" )
        rval = new DOUBLE_NOTCH_W_SEP(dbd);
    if( dbd._dspBlock == "BANDPASS FILT" )
        rval = new BANDPASS_FILT(dbd);
    if( dbd._dspBlock == "BAND2" )
        rval = new BAND2(dbd);

    if( dbd._dspBlock == "LOPAS2" )
        rval = new LOPAS2(dbd);
    if( dbd._dspBlock == "LP2RES" )
        rval = new LOPAS2(dbd);
    if( dbd._dspBlock == "LOPASS" )
        rval = new LOPASS(dbd);
    if( dbd._dspBlock == "LPCLIP" )
        rval = new LPCLIP(dbd);
    if( dbd._dspBlock == "LPGATE" )
        rval = new LPGATE(dbd);

    if( dbd._dspBlock == "HIPASS" )
        rval = new HIPASS(dbd);
    if( dbd._dspBlock == "ALPASS" )
        rval = new ALPASS(dbd);

    if( dbd._dspBlock == "HIFREQ STIMULATOR" )
        rval = new HIFREQ_STIMULATOR(dbd);


    ////////////////////////
    // nonlin
    ////////////////////////

    if( dbd._dspBlock == "SHAPER" )
        rval = new SHAPER(dbd);
    if( dbd._dspBlock == "2PARAM SHAPER" )
        rval = new TWOPARAM_SHAPER(dbd);
    if( dbd._dspBlock == "WRAP" )
        rval = new WRAP(dbd);
    if( dbd._dspBlock == "DIST" )
        rval = new DIST(dbd);

    return rval;
}


