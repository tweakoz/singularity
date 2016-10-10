#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////

layer::layer(synth& syn)
    : _layerData(nullptr)
    , _syn(syn)
    , _spOsc(syn, *this)
    , _layerGain(0.25)
    , _curPitchOffsetInCents(0.0f)
    , _centsPerKey(100.0f)
    , _lfo1(syn)
    , _lfo2(syn)
    , _lyrPhase(-1)
    , _phCounter(0)
    , _kmregion(nullptr)
    , _curnote(0)
    , _natAmpEnv(syn)
    , _useNatEnv(false)
    , _curcents(0)
    , _baseCents(0)
    , _FPA(0.0f)
    , _sampleRoot(0)
    , _alg(nullptr)
    , _preDSPGAIN(1.0f)
    , _postDSPGAIN(1.0f)
    , _doNoise(false)
{
   _pchControl1 = [](){return 0.0f;};
   _pchControl2 = [](){return 0.0f;};
    //printf( "Layer Init<%p>\n", this );
}

///////////////////////////////////////////////////////////////////////////////

void layer::compute(outputBuffer& obuf)
{
    ///////////////////////
    // leave some time at the end to bview ENV hud
    ///////////////////////
    if( _postdone>1024 )
    {
        _syn.freeLayer(this);
        return;
    }

    ///////////////////////

    if( nullptr == _layerData )
    {
        printf( "gotnull ld layer<%p>\n", this );
        return;
    }

    int inumframes = obuf._numframes;
    float* outl = obuf._leftBuffer;
    float* outr = obuf._rightBuffer;

    float dt = float(inumframes)/_syn._sampleRate;

    _lfo1.compute(dt);
    _lfo2.compute(dt);

    _fun1.compute(dt);
    _fun2.compute(dt);
    _fun3.compute(dt);
    _fun4.compute(dt);

    _pchc1 = _pchControl1();
    _pchc2 = _pchControl2();

    _pchc1 = clip_float( _pchc1, -6400,6400 );
    _pchc2 = clip_float( _pchc2, -6400,6400 );

    _curPitchOffsetInCents = _pchc1+_pchc2;

    _curPitchOffsetInCents = clip_float( _curPitchOffsetInCents, -6400,6400 );

    _curcents = _baseCents + _curPitchOffsetInCents;
    _curcents = clip_float( _curcents, -0,12700 );

    updateSampSRRatio();
    float currat = _curSampSRratio;

    _spOsc.setSrRatio(currat);
    //printf( "pchc1<%f> pchc2<%f> poic<%f> currat<%f>\n", _pchc1, _pchc2, _curPitchOffsetInCents, currat );
    ////////////////////////////////////////

    bool bypassDSP = _syn._bypassDSP;
    DspBlock* lastblock = _alg 
                        ? _alg->lastBlock() 
                        : nullptr;
    bool doBlockStereo = bypassDSP 
                       ? false 
                       : lastblock ? (lastblock->_numOutputs==2) 
                                   : false;
    //printf( "doBlockStereo<%d>\n", int(doBlockStereo) );

    ////////////////////////////////////////

    float ogain = _layerData->_outputGain;

    if( ogain != 0.0f )
    {
        float synsr = _syn._sampleRate;

        outputBuffer laybuf;

        _layerObuf.resize(inumframes);
        float* lyroutl = _layerObuf._leftBuffer;
        float* lyroutr = _layerObuf._rightBuffer;

        ///////////////////////////////////
        // AMPENV
        ///////////////////////////////////

        for( int i=0; i<inumframes; i++ )
        {
            float env2 = _env2.compute();
            float env3 = _env3.compute();

            float asr1 = _asr1.compute();
            float asr2 = _asr2.compute();

            float ampenv = _useNatEnv 
                         ? _natAmpEnv.compute()
                         : _userAmpEnv.compute(); 
            _AENV[i] = ampenv;
        }

        ///////////////////////////////////
        // sample osc
        ///////////////////////////////////

        if( _doNoise )
        {
            for( int i=0; i<inumframes; i++ )
            {
                //float ampenv = _AENV[i];
                float o = ((rand()&0xffff)/32768.0f)-1.0f;
                lyroutl[i] = o;//*ampenv;
                lyroutr[i] = 0.0f;//o*ampenv;
            }
        }
        else if( _syn._sinerep )
        {
            float F = midi_note_to_frequency(float(_curcents)*0.01);
            float phaseinc =  pi2*F/synsr;

            for( int i=0; i<inumframes; i++ )
            {
                //float ampenv = _AENV[i];
                float o = sinf(_sinrepPH)*_preDSPGAIN;
                _sinrepPH += phaseinc;
                lyroutl[i] = o;
                lyroutr[i] = 0.0f;
            }

        }
        else
        for( int i=0; i<inumframes; i++ )
        {
            float rawsamp = _spOsc.compute();
            //float ampenv = _AENV[i];
            float kmpblockOUT = rawsamp*_preDSPGAIN;
            lyroutl[i] = kmpblockOUT;
            lyroutr[i] = 0.0f;//kmpblockOUT;
        }
        ///////////////////////////////////
        // DSP F1-F3
        ///////////////////////////////////

        if(_alg and (false==bypassDSP) )
            _alg->compute(_layerObuf);

        ///////////////////////////////////
        // amp / out
        ///////////////////////////////////
        if( doBlockStereo )
        {
            for( int i=0; i<inumframes; i++ )
            {
                float ampenv = _AENV[i];
                //float tgain = ampenv*_postDSPGAIN*_preDSPGAIN;
                float tgain = _postDSPGAIN*_masterGain;//*ampenv;
                outl[i] += lyroutl[i]*tgain;
                outr[i] += lyroutr[i]*tgain;
            }
        }
        else if( bypassDSP )
        {
            for( int i=0; i<inumframes; i++ )
            {
                float ampenv = _AENV[i];
                //float tgain = ampenv*_postDSPGAIN*_preDSPGAIN;
                float tgain = _postDSPGAIN*_masterGain;//*ampenv;
                float inp = lyroutl[i]; 
                outl[i] += inp*tgain*0.5f;
                outr[i] += inp*tgain*0.5f;
            }
        }
        else
        {
            for( int i=0; i<inumframes; i++ )
            {
                float ampenv = _AENV[i];
                //float tgain = ampenv*_postDSPGAIN*_preDSPGAIN;
                float tgain = _postDSPGAIN*_masterGain;//*ampenv;
                float inp = lyroutl[i]; 
                outl[i] += inp*tgain;
                outr[i] += inp*tgain;
            }            
        }
    }

    ////////////////////////////////////////

    _phCounter++;

    if( _lyrPhase==1 && _phCounter>2048 )
    {
        //_layerData = nullptr;
        _postdone++;
        //_syn.freeLayer(this);
    }
    else if( _useNatEnv && _natAmpEnv.done() )
    {
        //printf( "natenv done..\n" );
        _postdone++;
        //_syn.freeLayer(this);
    }
    else if( (!_useNatEnv) && _userAmpEnv.done() )
    {
        //printf( "ampenv done..\n" );
        _postdone++;
    }

    /////////////////
    // oscope
    /////////////////

    int tailbegin = koscopelength-inumframes;
    memcpy( _oscopebuffer, _oscopebuffer+inumframes, tailbegin*4 );
    float* tailb = _oscopebuffer+tailbegin;

    for( int i=0; i<inumframes; i++ )
    {
        float ampenv = _AENV[i];
        float l = _layerObuf._leftBuffer[i];
        float r = _layerObuf._rightBuffer[i];
        tailb[i] = doBlockStereo ? l+r : l;
        tailb[i] *= ampenv;
    }
    //memcpy( _oscopebuffer+tailbegin, _layerObuf._leftBuffer, inumframes*4 );

}

controller_t layer::getController(const std::string& srcn) const
{
    if( srcn == "LFO1" )
        return [this]()
        {   float fv = this->_lfo1.getval();
            return fv;
        };
    else if( srcn == "LFO2" )
        return [this]()
        {   float fv = this->_lfo2.getval();
            return fv;
        };
    else if( srcn == "ASR1" )
        return [this]()
        {   float fv = this->_asr1._curval;
            return fv;
        };
    else if( srcn == "ASR2" )
        return [this]()
        {   float fv = this->_asr2._curval;
            return fv;
        };
    else if( srcn == "ENV2" )
        return [this]()
        {   float fv = this->_env2._curval;
            //printf( "ENV2<%f>\n", fv );
            return fv;
        };
    else if( srcn == "ENV3" )
        return [this]()
        {   float fv = this->_env3._curval;
            //printf( "ENV3<%f>\n", fv );
            return fv;
        };
    else if( srcn == "FUN1" )
        return [this]()
        {   float fv = this->_fun1._curval;
            return fv;
        };
    else if( srcn == "FUN2" )
        return [this]()
        {   float fv = this->_fun2._curval;
            return fv;
        };
    else if( srcn == "FUN3" )
        return [this]()
        {   float fv = this->_fun3._curval;
            return fv;
        };
    else if( srcn == "FUN4" )
        return [this]()
        {   float fv = this->_fun4._curval;
            return fv;
        };
    else if( srcn == "OFF" )
        return [this]()
        {   return 0.0f;
        };
    else if( srcn == "MPress" )
        return [this]()
        {
            if( _syn._doPressure )
            {
                static float fph = 0.0f;
                fph += 0.01f;
                float fv = 0.5f+sinf(fph)*0.5f;;
                return fv;
            }
            else
                return 0.0f;
        };
    else if( srcn == "MWheel" )
        return [this]()
        {   
            if( _syn._doModWheel )
            {
                static float fph = 0.0f;
                fph += 0.003f;
                float fv = 0.5f+sinf(fph)*0.5f;;
                return fv;
            }
            else
                return 0.0f;

        };
    else if( srcn == "ON" )
        return [this]()
        {   return 1.0f;
        };
    else if( srcn == "KeyNum" )
        return [this]()
        {   return this->_curnote;
        };
    else
    {
        float fv = atof(srcn.c_str());
        if(fv!=0.0f)
        {   return [=]()
            {   //printf( "fv<%f>\n", fv);
                return fv;
            };
        }

    }

    return [](){return 0.0f;};
  
}

///////////////////////////////////////////////////////////////////////////////

controller_t layer::getSRC1(const BlockModulationData& mods)
{
    auto src1 = this->getController(mods._src1);
    float src1depth = mods._src1Depth;

    auto it = [=]()->float
    {   
        float out = src1()*src1depth;
        //printf( "src1out<%f>\n", out );
        return out;
    };

    return it;

}

controller_t layer::getSRC2(const BlockModulationData& mods)
{
    auto src2 = this->getController(mods._src2);
    auto depthcon = this->getController(mods._src2DepthCtrl);
    float mindepth = mods._src2MinDepth;
    float maxdepth = mods._src2MaxDepth;
   
    auto it = [=]()->float
    {   
        float dc = clip_float(depthcon(),0,1);
        float depth = lerp(mindepth,maxdepth,dc);
        float out = src2()*depth;
        return out;
    };

    return it;
}

///////////////////////////////////////////////////////////////////////////////

void layer::keyOn( int note, const layerData* ld )
{
    std::lock_guard<std::mutex> lock(_mutex);
    assert(ld!=nullptr);
    const auto& KMP = ld->_kmpBlock;
    const auto& PCH = ld->_pchBlock;
    const auto& F1 = ld->_f1Block;
    const auto& F2 = ld->_f2Block;
    const auto& F3 = ld->_f3Block;
    const auto& F4 = ld->_f4Block;
    const auto& ENVC = ld->_envCtrlData;

    _useNatEnv = ENVC._useNatEnv;
    _ignoreRelease = ld->_ignRels;
    _curnote = note;
    _layerData = ld;
    _layerGain = ld->_outputGain;
    _postdone = 0;
    _masterGain = _syn._masterGain;

    _lfo1.reset();
    _lfo2.reset();

    for( int i=0; i<koscopelength/2; i++ )
    {
        _fftbuffer[i] = 0.0f;
    }

    /////////////////////////////////////////////
    
    _pchControl1 = getSRC1(PCH._mods);
    _pchControl2 = getSRC2(PCH._mods);


    _fp[0] = initFPARAM(F1);
    _fp[1] = initFPARAM(F2);
    _fp[2] = initFPARAM(F3);
    _fp[3] = initFPARAM(F4);

    /////////////////////////////////////////////


    auto km = ld->_keymap;
    assert(km);

    ///////////////////////////////////////

    const int kNOTEC4 = 60;

    int timbreshift = KMP._timbreShift; // 0
    int kmtrans = KMP._transpose+timbreshift; // -20
    int pchtrans = PCH._coarse-timbreshift; // 8
    int kmkeytrack = KMP._keyTrack; // 100
    int pchkeytrack = PCH._keyTrack; // 0
    // expect 48-20+8 = 28+8 = 36*100 = 3600 total cents

    int kmpivot = (kNOTEC4+kmtrans); // 48-20 = 28
    int kmdeltakey = (note+kmtrans-kmpivot); // 48-28 = 28
    int kmdeltacents = kmdeltakey*kmkeytrack; // 8*0 = 0
    int kmfinalcents = (kmpivot*100)+kmdeltacents; // 4800

    _sampselnote = (kmfinalcents/100); // 4800/100=48

    int pchpivot = (kNOTEC4+pchtrans); // 48-0 = 48
    int pchdeltakey = (note+pchtrans-pchpivot); // 48-48=0 //possible minus kmorigin?
    int pchdeltacents = pchdeltakey*pchkeytrack;//0*0=0
    int pchfinalcents = (pchtrans*100)+pchdeltacents;//0*100+0=0

    _doNoise = KMP._pbMode == "Noise";


    auto region = km->getRegion(_sampselnote,64);
    if( region )
    {
        assert(region);

        _kmregion = region;

        ///////////////////////////////////////
        auto sample = region->_sample;
        float sampsr = sample->_sampleRate;
        int highestP = sample->_highestPitch;
        _sampleRoot = sample->_rootKey;
        _keydiff = note-_sampleRoot;
        ///////////////////////////////////////
        
        _kmcents = kmfinalcents+region->_tuning;
        _pchcents = pchfinalcents;

        ///////////////////////////////////////
        // just a guess that the lowest 2 decimal 
        // digits in the HP represent
        // +/- 50 cents for pitch adjust
        ///////////////////////////////////////

        float SRratio = 96000.0f/sampsr;
        int RKcents = (_sampleRoot)*100;
        int delcents = highestP-RKcents;
        int frqerc = linear_freq_ratio_to_cents(SRratio);
        int pitchadjx = (frqerc-delcents); //+1200;
        int pitchadj = sample->_pitchAdjust; //+1200;

        //if( SRratio<3.0f )
        //    pitchadj >>=1;
        printf( "sampsr<%f> srrat<%f> rkc<%d> hp<%d> delc<%d> frqerc<%d> pitchadjx<%d>\n", sampsr, SRratio, RKcents, highestP, delcents, frqerc, pitchadjx );

        _curpitchadj = pitchadj;
        _curpitchadjx = pitchadjx;

        _baseCents = _kmcents+_pchcents+pitchadjx-1200;
        if(pitchadj)
            _baseCents = _kmcents+_pchcents+pitchadj;
        _curcents = _baseCents;

        updateSampSRRatio();

        //float outputPAD = decibel_to_linear_amp_ratio(F4._pad);
        //float ampCOARSE = decibel_to_linear_amp_ratio(F4._coarse);

        //_totalGain = sample->_linGain* region->_linGain; // * _layerGain * outputPAD * ampCOARSE;

        _preDSPGAIN = sample->_linGain* region->_linGain;
        _postDSPGAIN = _layerGain;// * outputPAD * ampCOARSE;

        ///////////////////////////////////////
        //  trigger sample playback oscillator
        ///////////////////////////////////////

        _spOsc._sample = sample;
        _spOsc.keyOn(_curSampSRratio);
    }

    /////////////////////////////////////////////
    // trigger lfos
    /////////////////////////////////////////////

    _lfo1.keyOn(ld->_lfo1);
    _lfo2.keyOn(ld->_lfo2);

    /////////////////////////////////////////////
    // trigger funs
    /////////////////////////////////////////////

    _fun1.keyOn(this,ld->_fun1);
    _fun2.keyOn(this,ld->_fun2);
    _fun3.keyOn(this,ld->_fun3);
    _fun4.keyOn(this,ld->_fun4);

    ///////////////////////////////////////
    //  trigger envelopes (todo - delay)
    ///////////////////////////////////////

    if( _useNatEnv && region->_sample )
        _natAmpEnv.keyOn(note,ld,region->_sample);
    else
        _userAmpEnv.keyOn(note, ld, ld->_userAmpEnv);

    if( ld->_env2 )
        _env2.keyOn(note, ld, ld->_env2);
    if( ld->_env3 )
        _env3.keyOn(note, ld, ld->_env3);

    if( ld->_asr1 )
        _asr1.keyOn(ld,ld->_asr1);
    if( ld->_asr2 )
        _asr2.keyOn(ld,ld->_asr2);

    ///////////////////////////////////////

    _alg = _layerData->_algData.createAlgInst();
    //assert(_alg);
    if(_alg)
        _alg->keyOn(this);

    ///////////////////////////////////////

    _lyrPhase = 0;
    _phCounter = 0;
    _sinrepPH = 0.0f;

}

void layer::updateSampSRRatio()
{
    float desired_cents =  _curcents;

    ///////////////////////////////////////

    int cents_at_root = (_sampleRoot*100)+(_FPA);
    int delta_cents = _curcents-cents_at_root;

    //printf( "DESCENTS<%d>\n", _curcents );
    //printf( "SROOTCENTS<%d>\n", cents_at_root );
    //printf( "DELCENTS<%d>\n", delta_cents );
    //printf( "FPA<%f>\n", _FPA );

    _samppbnote = _sampleRoot+(delta_cents/100);

    _curSampSRratio = cents_to_linear_freq_ratio(delta_cents);

}

///////////////////////////////////////////////////////////////////////////////

void layer::keyOff()
{
    _lyrPhase = 1;
    _phCounter = 0;
    _postdone = 0;

    ///////////////////////////////////////
    //  release envelopes
    ///////////////////////////////////////

    if( _useNatEnv )
        _natAmpEnv.keyOff();
    else
        _userAmpEnv.keyOff();

    _env2.keyOff();
    _env3.keyOff();

    _asr1.keyOff();
    _asr2.keyOff();

    _fun1.keyOff();
    _fun2.keyOff();
    _fun3.keyOff();
    _fun4.keyOff();

    ///////////////////////////////////////

    if( _ignoreRelease )
        return;

    ///////////////////////////////////////

    if(_alg)
        _alg->keyOff();

    _spOsc.keyOff();

}

///////////////////////////////////////////////////////////////////////////////

void layer::reset()
{
    _layerData = nullptr;
    _kmregion = nullptr;
    _useNatEnv = false;
    _curnote = 0;
}