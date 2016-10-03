#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krztest.h"

FunInst::FunInst()
{
    _a = []()->float{return 0.0f;};
    _b = []()->float{return 0.0f;};
    _op = []()->float{return 0.0f;};
}

float FunInst::compute(float dt)
{
    _curval = _op();
    return _curval;
}

struct Lowpass
{
    Lowpass() 
        :a(0.99f)
        ,b(1.f - a)
        ,z(0)
    {}
    float compute(float in)
    {   z = (in * b) + (z * a);
        return z;
    }

    float a, b, z;
};

void FunInst::keyOn(const layer* l, const FunData* data)
{
    _data = data;

    if(nullptr==data)
        return;

    _a = l->getController(data->_a);   
    _b = l->getController(data->_b);   
    _op = []()->float{return 0.0f;};

    const auto& op = data->_op;
    if( 0 )
        return;
    else if( op=="a+b" )
        _op = [=]()->float{
            float a = this->_a();
            float b = this->_b();
            return (a+b);
        };  
    else if( op=="a*b" )
        _op = [=]()->float{
            float a = this->_a();
            float b = this->_b();
            return (a*b);
        };  
    else if( op=="(a+b)/2" )
        _op = [=]()->float{
            float a = this->_a();
            float b = this->_b();
            return (a+b)*0.5f;
        };  
    else if( op=="Quantize B To A" )
        _op = [=]()->float{
            float a = this->_a();
            float b = this->_b();
            float inva = 1.0f / a;
            float bb = b*inva;
            int ib = int(bb);

            return clip_float(ib,-1,1);///inva;
        };  
    else if( op=="lowpass(f=a,b)" )
    {
        auto lowpass = new Lowpass;
        _op = [=]()->float{
            float a = this->_a();
            float b = this->_b();
            float out = lowpass->compute(b);
            return out;
        };
    }
}
void FunInst::keyOff()
{
}
