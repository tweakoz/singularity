#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////
// 7-seg rate/level envelopes
///////////////////////////////////////////////////////////////////////////////

AsrInst::AsrInst()
    : _data(nullptr)
    , _curval(0.0f)
    , _curslope_persamp(0.0f)
    , _curseg(-1)
    , _released(false)
{

}

///////////////////////////////////////////////////////////////////////////////

void AsrInst::initSeg(int iseg)
{
    printf( "AsrInst<%p> _data<%p> initSeg<%d>\n", this, _data, iseg );

    if(!_data) return;

    _curseg = iseg;
    assert(_curseg>=0);
    assert(_curseg<=3);
    const auto& edata = *_data;
    float segtime = 0.0f;
    float dstlevl = 0.0f;

    switch( iseg )
    {
        case 0: // delay
            segtime = edata._delay;
            dstlevl = 0;
            break;
        case 1: // atk
            segtime = edata._attack;
            segtime /= _atkAdjust;
            dstlevl = 1;
            break;
        case 2: // hold
            _curval = 1;
            _curslope_persamp = 0.0f;
            return;
            break;
        case 3: // release
            segtime = edata._release;
            segtime /= _relAdjust;
            dstlevl = 0;
            break;
    }

    if( segtime == 0.0f )
    {
        _curval = dstlevl;
        _curslope_persamp = 0.0f;
    }
    else
    {   float deltlev = (dstlevl-_curval);
        float slope = (deltlev/segtime);
        _curslope_persamp = slope/48000.0f;
    }
    _framesrem = segtime* 48000.0f; // / 48000.0f;
}

///////////////////////////////////////////////////////////////////////////////

float AsrInst::compute()
{
    if( nullptr == _data )
        return 0.5f;

    const auto& edata = *_data;
    _framesrem--;
    if( _curseg==0 && _framesrem<=0 )
        initSeg(1);

    _curval += _curslope_persamp;
    _curval = clip_float(_curval,0.0f,1.0f);
    return _curval;
}

///////////////////////////////////////////////////////////////////////////////

void AsrInst::keyOn(const layerData* ld, const AsrData* data)
{   
    const auto& EC = ld->_envCtrlData;
    _atkAdjust = EC._atkAdjust;
    _relAdjust = EC._relAdjust;
    _ignoreRelease = ld->_ignRels;
    _data = data;
    assert(_data);
    _curval = 0.0f;
    _released = false;
    initSeg(0);
}

///////////////////////////////////////////////////////////////////////////////

void AsrInst::keyOff()
{
    _released = true;
    if( false == _ignoreRelease )
        initSeg(3);
}

bool AsrInst::isValid() const
{
    return _data and _data->_name.length();
}


///////////////////////////////////////////////////////////////////////////////
// 7-seg rate/level envelopes
///////////////////////////////////////////////////////////////////////////////

RateLevelEnvInst::RateLevelEnvInst()
    : _data(nullptr)
    , _curval(0.0f)
    , _curslope_persamp(0.0f)
    , _curseg(-1)
    , _released(false)
{

}

///////////////////////////////////////////////////////////////////////////////

void RateLevelEnvInst::initSeg(int iseg)
{
    if(!_data) return;

    _curseg = iseg;
    assert(_curseg>=0);
    assert(_curseg<7);
    const auto& edata = *_data;
    auto curseg = edata._segments[_curseg];
    float segtime = curseg._rate;

    switch( iseg )
    {
        case 0:
        case 1:
        case 2: // atk
            segtime /= _atkAdjust;
            break;
        case 3: // decay
            segtime /= _decAdjust;
            break;
        case 4:
        case 5:
        case 6: // atk
            segtime /= _relAdjust;
            break;
    }

   // segtime *= 0.5f;

    if( segtime == 0.0f )
    {   if( iseg==1 or iseg==2 or iseg==4 or iseg==5 ){
            // attack segss 2 and 3 only have effect
            // if their times are not 0
        }
        else{
            _dstval = curseg._level;
            _curval = _dstval;
            _curslope_persamp = 0.0f;
        }
    }
    else
    {   _dstval = curseg._level;
        float deltlev = (_dstval-_curval);
        float slope = (deltlev/segtime);
        _curslope_persamp = slope/48000.0f;
    }
    _framesrem = segtime* 48000.0f; // / 48000.0f;
}

///////////////////////////////////////////////////////////////////////////////

float RateLevelEnvInst::compute()
{
    if( nullptr == _data )
        return 0.0f;

    const auto& edata = *_data;
    const auto& segs = edata._segments;
    bool done = false;
    _framesrem--;
    if( _framesrem<=0 )
    {   if( _released )
        {   if( _curseg <= 5  )
            {
                initSeg(_curseg+1);
            }
            else
            {   done = true;
                _curval = 0.0f;
                //_data = nullptr;
            }
        }
        else // go up to decay
        {   if( _curseg == 3 )
            {   float declev = segs[3]._level;
                if( _curval < declev )
                {   _curslope_persamp = 0.0f;
                    _curval = declev;
                }
            }
            else if( _curseg <= 2  )
                initSeg(_curseg+1);
        }
    }
    if( ! done )
    {
        _curval += _curslope_persamp;
        if( _curslope_persamp>0.0f && _curval>_dstval )
            _curval = _dstval;
        else if( _curslope_persamp<0.0f && _curval<_dstval )
            _curval = _dstval;
    }

    _filtval = _filtval*0.995f + _curval*0.005f;

    return clip_float(powf(_filtval,4.0f),-1.0f,1.0f);
}

///////////////////////////////////////////////////////////////////////////////

void RateLevelEnvInst::keyOn(int ikey, const layerData* ld, const RateLevelEnvData*data)
{   
    const auto& EC = ld->_envCtrlData;
    const auto& DKT = EC._decKeyTrack;
    const auto& RKT = EC._relKeyTrack;
    printf( "ikey<%d> DKT<%f>\n", ikey, DKT );

    float fkl = -1.0f+float(ikey)/63.5f;

    _atkAdjust = EC._atkAdjust;
    _decAdjust = EC._decAdjust;
    _relAdjust = EC._relAdjust;

    if( ikey>60 )
    {
        float flerp = float(ikey-60)/float(127-60);
        _decAdjust = lerp(_decAdjust,DKT,flerp);
        _relAdjust = lerp(_relAdjust,RKT,flerp);
    }
    else if( ikey<60 )
    {
        float flerp = float(59-ikey)/59.0f;
        _decAdjust = lerp(_decAdjust,1.0/DKT,flerp);
        _relAdjust = lerp(_relAdjust,1.0/RKT,flerp);
    }
    //printf( "flerp<%f> _decAdjust<%f>\n", flerp,_decAdjust);

    printf( "_relAdjust<%f>\n", _relAdjust );
    _ignoreRelease = ld->_ignRels;
    _data = data;
    assert(_data);
    _curval = 0.0f;
    _released = false;
    initSeg(0);
}

///////////////////////////////////////////////////////////////////////////////

void RateLevelEnvInst::keyOff()
{
    _released = true;
    if( false == _ignoreRelease )
        initSeg(4);
}

///////////////////////////////////////////////////////////////////////////////

bool RateLevelEnvInst::done() const
{
    float dbatten = linear_amp_ratio_to_decibel(_curval);
    return (_curseg==6)&&(dbatten<-96.0f);
}

///////////////////////////////////////////////////////////////////////////////
// Natural envelopes
///////////////////////////////////////////////////////////////////////////////

NatEnv::NatEnv(const synth& syn)
    : _curseg(0)
    , _numseg(0)
    , _prvseg(0)
    , _slopePerSecond(0.0f)
    , _slopePerSample(0.0f)
    , _SR(syn._sampleRate)
    , _framesrem(0)
    , _released(false)
{

}

///////////////////////////////////////////////////////////////////////////////

void NatEnv::keyOn(int ikey, const layerData* ld,const sample* s)
{
    _natenv = s->_natenv;
    _numseg = _natenv.size();
    _curseg = 0;
    _prvseg = -1;
    _curamp = 1.0f;
    _released = false;
    _ignoreRelease = ld->_ignRels;

    const auto& EC = ld->_envCtrlData;
    const auto& DKT = EC._decKeyTrack;
    const auto& RKT = EC._relKeyTrack;

    _decAdjust = EC._decAdjust;
    _relAdjust = EC._relAdjust;

    if( ikey>60 )
    {
        float flerp = float(ikey-60)/float(127-60);
        _decAdjust = lerp(_decAdjust,DKT,flerp);
        _relAdjust = lerp(_relAdjust,RKT,flerp);
    }
    else if( ikey<60 )
    {
        float flerp = float(59-ikey)/59.0f;
        _decAdjust = lerp(_decAdjust,1.0/DKT,flerp);
        _relAdjust = lerp(_relAdjust,1.0/RKT,flerp);
    }

    initSeg(0);
}

///////////////////////////////////////////////////////////////////////////////

void NatEnv::keyOff()
{
    _released=true;
    if( _ignoreRelease )
        return;

    if( _numseg-1 >= 0 )
        initSeg(_numseg-1);
}

///////////////////////////////////////////////////////////////////////////////

const natenvseg& NatEnv::getCurSeg() const
{
    assert(_curseg>=0);
    assert(_curseg<_numseg);
    return _natenv[_curseg];
}

///////////////////////////////////////////////////////////////////////////////

void NatEnv::initSeg(int iseg)
{
    _curseg = iseg;
    const auto& seg = getCurSeg();

    _slopePerSecond = seg._slope;
    if( _released )
    {
        _slopePerSecond*=_relAdjust;
    }
    else
    {
        _slopePerSecond*=_decAdjust;
    }



    _slopePerSample = slopeDBPerSample(_slopePerSecond,192000.0);
    _segtime = seg._time;
    _framesrem = seg._time; ///16.0f;// * _SR / 48000.0f;
    //printf( "SEG<%d/%d> CURAMP<%f> SLOPE<%f> SEGT<%f>\n", _curseg+1, _numseg, _curamp,_slopePerSecond,seg._time );
}

///////////////////////////////////////////////////////////////////////////////

float NatEnv::compute()
{
    bool doslope = _released;

    if( false == _released )
    {
        doslope = true;
        _framesrem--;
        if( _framesrem<=0 )
        {
            if( _curseg+2 < _numseg )
                initSeg(_curseg+1);
            else // 
            {
                //doslope = false;
                //printf( "decay.. dbps<%f>\n",_slopePerSecond);
            }
        }
    }

    if( doslope )
        _curamp *= _slopePerSample;

    assert(_curamp<=1.0f);
    assert(_curamp>=-1.0f);

    return _curamp;
}

///////////////////////////////////////////////////////////////////////////////

bool NatEnv::done() const
{
    float dbatten = linear_amp_ratio_to_decibel(_curamp);
    return ((_curseg+1)==_numseg)&&(dbatten<-96.0f);
}
