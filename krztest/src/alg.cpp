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

float* DspBlock::getInpBuf1(dspBlockBuffer& obuf)
{
    assert((_inputMask==1) || (_inputMask==2));
    return (_inputMask==1)
           ? obuf._upperBuffer
           : obuf._lowerBuffer;
}
float* DspBlock::getOutBuf1(dspBlockBuffer& obuf)
{
    assert((_outputMask==1) || (_outputMask==2));
    return (_outputMask==1)
           ? obuf._upperBuffer
           : obuf._lowerBuffer;
}

void DspBlock::output1(dspBlockBuffer& obuf,int index,float val)
{
    float* U = obuf._upperBuffer;
    float* L = obuf._lowerBuffer;

    switch(_inputMask)
    {
        case kmaskUPPER:
            U[index] = val;
            break;
        case kmaskLOWER:
            L[index] = val;
            break;
        case kmaskBOTH:
            U[index] = val;
            L[index] = val;
            break;

    } 
}

float DspBlock::outputGainU()
{
    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;
    float f3gain = decibel_to_linear_amp_ratio(F3._v14Gain);
    //float f4gain = decibel_to_linear_amp_ratio(F4._v14Gain);
    return f3gain;
}
float DspBlock::outputGainL()
{
    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;
    //float f3gain = decibel_to_linear_amp_ratio(F3._v14Gain);
    float f4gain = decibel_to_linear_amp_ratio(F4._v14Gain);
    return f4gain;
}
float DspBlock::outputGainSINGLE()
{
    const auto& layd = _layer->_layerData;
    const auto& F4 = layd->_f4Block;
    //assert(_numOutputs==1);
    float f4gain = decibel_to_linear_amp_ratio(F4._v14Gain);
    return f4gain;
}

int DspBlock::numOutputs() const
{
    return (_outputMask==kmaskBOTH) ? 2 : 1;
}
int DspBlock::numInputs() const
{
    return (_inputMask==kmaskBOTH) ? 2 : 1;
}

///////////////////////////////////////////////////////////////////////////////

Alg::Alg()
{
    for( int i=0; i<kmaxblocks; i++ )
        _block[i] = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

DspBlock* Alg::lastBlock() const
{
    DspBlock* r = nullptr;
    for( int i=0; i<kmaxblocks; i++ )
        if( _block[i] )
            r = _block[i];
    return r;
}

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOn(DspKeyOnInfo& koi)
{
    auto l = koi._layer;
    assert(l!=nullptr);

    for( int i=0; i<kmaxblocks; i++ )
        _block[i] = nullptr;

    const auto ld = l->_layerData;    
    const auto& F1D = ld->_f1Block;
    const auto& F2D = ld->_f2Block;
    const auto& F3D = ld->_f3Block;
    const auto& F4D = ld->_f4Block;

    if( F1D._dspBlock.length() )
        _block[1] = createDspBlock(F1D);
    if( F2D._dspBlock.length() )
        _block[2] = createDspBlock(F2D);
    if( F3D._dspBlock.length() )
        _block[3] = createDspBlock(F3D);
    if( F4D._dspBlock.length() )
        _block[4] = createDspBlock(F4D);

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

    for( int i=0; i<kmaxblocks; i++ )
        if( _block[i] )
            procblock( _block[i],l );


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
    memcpy( lefbuf, uprbuf, inumframes*4 );
    memcpy( rhtbuf, lwrbuf, inumframes*4 );
}

void Alg::compute(outputBuffer& obuf)
{
    intoDspBuf(obuf,_blockBuf);

    bool touched = false;

    int inumoutputs = 1;

    for( int i=0; i<kmaxblocks; i++ )
    {
        auto b = _block[i];
        if( b )
        {   
            bool ena = the_synth->_fblockEnable[i-1];

            if( ena )
            {
                b->compute(_blockBuf);
                inumoutputs = b->numOutputs();
                touched = true;
            }
        }
    }

    intoOutBuf(obuf,_blockBuf,inumoutputs);
}

///////////////////////////////////////////////////////////////////////////////

void Alg::keyOff()
{
    for( int i=0; i<kmaxblocks; i++ )
    {
        auto b = _block[i];
        if( b ) 
            b->doKeyOff();
        //_block[i] = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Alg2::doKeyOn(DspKeyOnInfo& koi) // final
{
    if( _block[3] )
        _block[3]->_outputMask = kmaskBOTH;

    if( _block[4] )
    {
        _block[4]->_inputMask = kmaskBOTH;
        _block[4]->_outputMask = kmaskBOTH;
    }

    Alg::doKeyOn(koi);
}
void Alg3::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[3] )
    {
        _block[3]->_inputMask = kmaskBOTH;
        _block[3]->_outputMask = kmaskBOTH;
    }

    Alg::doKeyOn(koi);
}
void Alg6::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[4] )
        _block[4]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg7::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[3] )
    {
        _block[3]->_inputMask = kmaskLOWER;
        _block[3]->_outputMask = kmaskLOWER;
    }
    if( _block[4] )
        _block[4]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}

void Alg10::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[2] )
    {
        _block[2]->_inputMask = kmaskLOWER;
        _block[2]->_outputMask = kmaskLOWER;
    }
    if( _block[4] )
        _block[4]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg11::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[1] )
        _block[1]->_outputMask = 3;
    if( _block[2] )
        _block[2]->_outputMask = kmaskLOWER;
    if( _block[4] )
        _block[4]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg12::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[1] )
        _block[1]->_outputMask = kmaskBOTH;
    if( _block[4] )
        _block[4]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg14::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[2] )
        _block[2]->_inputMask = kmaskLOWER;

    Alg::doKeyOn(koi);
}
void Alg15::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[1] )
        _block[1]->_inputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg22::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[2] )
        _block[2]->_outputMask = kmaskLOWER;

    Alg::doKeyOn(koi);
}
void Alg23::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[2] )
        _block[2]->_outputMask = kmaskBOTH;

    Alg::doKeyOn(koi);
}
void Alg24::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[2] )
        _block[2]->_inputMask = kmaskBOTH;
    if( _block[3] )
        _block[2]->_outputMask = kmaskBOTH;
    if( _block[4] )
    {
        _block[4]->_inputMask = kmaskBOTH;
        _block[4]->_outputMask = kmaskBOTH;
    }

    Alg::doKeyOn(koi);
}
void Alg26::doKeyOn(DspKeyOnInfo& koi)
{
    if( _block[3] )
        _block[3]->_outputMask = kmaskBOTH;
    if( _block[4] )
    {
        _block[4]->_inputMask = kmaskBOTH;
        _block[4]->_outputMask = kmaskBOTH;
    }

    Alg::doKeyOn(koi);
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
        case 6:
            return new Alg6;
            break;
        case 7:
            return new Alg7;
            break;
        case 10:
            return new Alg10;        
            break;
        case 11:
            return new Alg11;        
            break;
        case 12:
            return new Alg12;        
            break;
        case 14:
            return new Alg14;        
            break;
        case 15:
            return new Alg15;        
            break;
        case 22:
            return new Alg22;        
            break;
        case 23:
            return new Alg23;        
            break;
        case 24:
            return new Alg24;        
            break;
        case 26:
            return new Alg26;        
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

    if( dbd._dspBlock == "SYNC M" )
        rval = new SYNCM(dbd);
    if( dbd._dspBlock == "SYNC S" )
        rval = new SYNCS(dbd);
    if( dbd._dspBlock == "PWM" )
        rval = new PWM(dbd);


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


