#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

SINE::SINE( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 1;                
}

void SINE::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
//    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    //printf("lc<%f> coff<%f> cin<%f> frq<%f>\n", lyrcents, centoff, cin, frq );
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];

        output1(obuf,i, saw );
    }
}
void SINE::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SAW::SAW( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SAWTOOTH)
{   _numParams = 1;                
}

void SAW::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    //float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        output1(obuf,i, saw );
       // ubuf[i] = saw;
    }
}
void SAW::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SQUARE::SQUARE( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SQUARE)
{   _numParams = 1;                
}

void SQUARE::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    //float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        //ubuf[i] = saw;
        output1(obuf,i, saw );
    }
}
void SQUARE::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SINEPLUS::SINEPLUS( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 1;                
}

void SINEPLUS::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i]*pad;
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float swplus = input+saw;
        ubuf[i] = swplus;
    }
}
void SINEPLUS::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SAWPLUS::SAWPLUS( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SAWTOOTH)
{   _numParams = 1;                
}

void SAWPLUS::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf("lc<%f> coff<%f> cin<%f> frq<%f>\n", lyrcents, centoff, cin, frq );
    //printf( "saw+ pad<%f>\n", pad );

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i]*pad;
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float swplus = input+(saw);
        output1(obuf,i, swplus );
    }
}
void SAWPLUS::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SWPLUSSHP::SWPLUSSHP( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::RAMP)
{   _numParams = 1;    
    _pblep.setAmplitude(1.0f);            
}

void SWPLUSSHP::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = clip_float(ubuf[i]*pad,-1,1);
        float saw = _pblep.getAndInc();
        float xxx = wrap(input+saw,-30.0f);
        float swplusshp = shaper(xxx,.25);
        ubuf[i] = (xxx);
    }
}
void SWPLUSSHP::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SHAPEMODOSC::SHAPEMODOSC( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 2;    
    _pblep.setAmplitude(1.0f);            
}

void SHAPEMODOSC::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    float depth = _ctrl[1].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );

    float depg = decibel_to_linear_amp_ratio(depth);


    const float kc1 = 1.0f/128.0f;
    const float kc2 = 1.0f/32;

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        float inU = ubuf[i]*pad;
        float inL = lbuf[i]*pad;
        float inp = (inU)*depg;

        //First, the SINE value is multiplied by 
        // the sample input value, then multiplied 
        // by a constant—any samples exceeding full scale 
        // will wrap around.

        float sine = _pblep.getAndInc();
        float xxx = wrap(inp*sine*kc1,1.0);

        //The result is added to the wrapped product 
        // of the SINE value times a constant.

        float yyy = xxx + wrap(sine*kc2,1.0);

        //The entire resulting waveform is then passed 
        // through the SHAPER, whose Adjust value is 
        // set by the level of the sample input.

        float swplusshp = shaper(yyy,inp);
        ubuf[i] = (swplusshp);

    }

    _fval[0] = centoff;
    _fval[1] = depth;
}
void SHAPEMODOSC::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

PLUSSHAPEMODOSC::PLUSSHAPEMODOSC( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 2;    
    _pblep.setAmplitude(0.25f);            
}

void PLUSSHAPEMODOSC::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    float depth = _ctrl[1].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );

    float depg = decibel_to_linear_amp_ratio(depth);


    const float kc1 = 1.0f/128.0f;
    const float kc2 = 1.0f/32;

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        float inU = ubuf[i]*pad;
        float inL = lbuf[i]*pad;
        float inp = (inU+inL)*depg;

        //First, the SINE value is multiplied by 
        // the sample input value, then multiplied 
        // by a constant—any samples exceeding full scale 
        // will wrap around.

        float sine = _pblep.getAndInc();
        float xxx = wrap(inp*sine*kc1,1.0);

        //The result is added to the wrapped product 
        // of the SINE value times a constant.

        float yyy = xxx + wrap(sine*kc2,1.0);

        //The entire resulting waveform is then passed 
        // through the SHAPER, whose Adjust value is 
        // set by the level of the sample input.

        float swplusshp = shaper(yyy,inp);
        ubuf[i] = (swplusshp);


        /*
        x SHAPE MOD OSC
        this function is
         similar to SHAPE MOD OSC, except that it multiplies
         its two input signals and uses that result as its input.

        + SHAPE MOD OSC

         + SHAPE MOD OSC is similar to x SHAPE MOD OSC, 
         except that it adds its two input signals and uses 
         that sum as its input. 
*/
    }

    _fval[0] = centoff;
    _fval[1] = depth;
}
void PLUSSHAPEMODOSC::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SYNCM::SYNCM( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;    
}

void SYNCM::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);

    float SR = _layer->_syn._sampleRate;
    _phaseInc = frq/SR;


    //printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   
        ubuf[i] = _phase;
        _phase = fmod(_phase+_phaseInc,1.0f);
    }
}
void SYNCM::doKeyOn(const DspKeyOnInfo& koi) //final
{
    _phaseInc = 0.0f;
    _phase = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

SYNCS::SYNCS( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::RAMP)
{   _numParams = 1;    
    _pblep.setAmplitude(1.0f);   
    _prvmaster = 0.0f;         
}

void SYNCS::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i];

        bool do_sync = ( input < _prvmaster);            
        _prvmaster = input;

        if( do_sync )
            _pblep.sync(0.0f);

        float saw = _pblep.getAndInc();
        ubuf[i] = saw;
    }
}
void SYNCS::doKeyOn(const DspKeyOnInfo& koi) //final
{
    _prvmaster = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

PWM::PWM( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 1;    
    _pblep.setAmplitude(1.0f);   
}

void PWM::compute(dspBlockBuffer& obuf) //final
{
    float offset = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = offset;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcents;
    float cin = (lyrcents)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i]*pad;
        ubuf[i] = input+offset*0.01f;
    }
}
void PWM::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SAMPLEPB::SAMPLEPB( const DspBlockData& dbd )
    : DspBlock(dbd)
{

}
void SAMPLEPB::compute(dspBlockBuffer& obuf) // final
{

}

void SAMPLEPB::doKeyOn(const DspKeyOnInfo& koi) // final
{

}

