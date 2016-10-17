////////////////////////////////////////////////////////////////
// Orkid Media Engine
// Copyright 1996-2012, Michael T. Mayers.
// Distributed under the Boost Software License - Version 1.0 - August 17, 2003
// see http://www.boost.org/LICENSE_1_0.txt
//////////////////////////////////////////////////////////////// 

#include <math.h>
#include <cmath>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// decibel - A unit of amplitude ratio corresponding to the twentieth root of ten, approximately 1.122018454.
//
// centibel - A unit of amplitude ratio corresponding to the two hundredth root of ten, or one tenth of a decibel,
//			  approximately 1.011579454.
//
// timecent - A unit of duration ratio corresponding to the twelve hundredth root of two, 
//			  or one twelve hundredth of an octave, approximately 1.000577790.
//
// cent - A unit of pitch ratio corresponding to the twelve hundredth root of two, 
//			  or one hundredth of a semitone, approximately 1.000577790.
///////////////////////////////////////////////////////////////////////////////

float log_base( float base, float inp )
{
	float rval = std::log( inp ) / std::log( base );
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

float pow_base( float base, float inp )
{
	float rval = std::pow( base, inp );
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

int round_to_nearest( float in )
{
	float trval, trval2;
	int urval = int(in);
	int urval2;
	int rval;
	
	if( in >= 0.0f )
		urval2 = urval+1;
	else
		urval2 = urval-1;

	trval = (float) urval;
	trval2 = (float) urval2;
	
	if( std::fabs( trval2-in ) < std::fabs( trval-in ) )
		rval = urval2;
	else
		rval = urval;
	
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

float linear_time_to_timecent( float time )
{
	float timecent = 1200.0f * log_base( 2.0f, time );
	return time;
}

float timecent_to_linear_time( float timecent )
{
	float time = pow_base( 2.0f, (timecent/1200.0f) );
	return time;
}

///////////////////////////////////////////////////////////////////////////////
// note:	96dB Digital Decibels as linear = 65536 (1<<16)
//			96dB true Decibels as linear = 10 to the 4.8th power = 63095.73445

float decibel_to_linear_amp_ratio( float decibel )
{
	float linear = pow_base( 10.0f, (decibel/20.0f) );
	return linear;
}

float linear_amp_ratio_to_decibel( float linear )
{
	float decibel = 20.0f * log_base( 10.0f, linear );
	return decibel;
}

///////////////////////////////////////////////////////////////////////////////

float centibel_to_linear_amp_ratio( float centibel )
{
	float linear = pow_base( 10.0f, (centibel/200.0f) );
	return linear;
}

float linear_amp_ratio_to_centibel( float linear )
{
	float centibel = 200.0f * log_base( 10.0f, linear );
	return centibel;
}

///////////////////////////////////////////////////////////////////////////////

float linear_freq_ratio_to_cents( float freq_ratio )
{
	float cents = log_base( 2.0f, freq_ratio );
	//orkprintf( "log2( %g ) = %g  (* 1200.0f = %g)\n", freq_ratio, cents, (cents*1200.0) );
	return (cents*1200.0f);
}

float cents_to_linear_freq_ratio( float cents )
{
	float freq_ratio = pow_base( 2.0f, (cents/1200.0f) );
	return freq_ratio;
}

///////////////////////////////////////////////////////////////////////////////

//#define TUNING_CONSTANT	32.7033
#define TUNING_CONSTANT	16.3515978307f // derived by tweak given note 57 = 440.00000000 hz
//#define TUNING_CONSTANT	32.7031956614 // derived by tweak given note 57 = 440.00000000 hz

float midi_note_to_frequency( float midinote )
{
	float frequency = (TUNING_CONSTANT) * 0.5f * cents_to_linear_freq_ratio( midinote * 100.0f );
	//orkprintf( "midinote->frequency( note: %g ) = frequency: %g\n", midinote, frequency );
	return frequency;
}

///////////////////////////////////////////////////////////////////////////////

float frequency_to_midi_note( float frequency )
{
	//float frequency = 32.7033 * cents_to_linear_freq_ratio( midinote * 100.0 );
	float note = linear_freq_ratio_to_cents( (frequency/TUNING_CONSTANT) ) / 100.0f;
	//orkprintf( "frequency->note( %g ) = %g\n", frequency, note );
	return note;
}

///////////////////////////////////////////////////////////////////////////////

float clip_float( float in, float minn, float maxx )
{
	if( in < minn )
		in = minn;
	else if( in > maxx )
		in = maxx;
	return in;
}

float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    float y = clip_float((x - edge0)/(edge1 - edge0), 0.0, 1.0); 
    //printf( "x<%f> y<%f>\n", x, y );
    // Evaluate polynomial
    return y*y*(3.0f - 2.0f*y);
}

float softsat(float x, float a)
{
	float mag = fabs(x);
	bool neg = x<0.0f;
	float rval = 0.0f;

	if(mag < a)
	  rval=mag;
	if(mag > a)
	  rval = a + (mag-a)/(1.0f+powf((mag-a)/(1.0f-a),2.0f));
	if(mag > 1)
	  rval = (a+1.0f)/2.0f;

  return neg ? -rval : rval;
}

///////////////////////////////////////////////////////////////////////////////

float lerp( float from,
			float to,
			float index )
{
	return to*index + from*(1.0f-index);
}
///////////////////////////////////////////////////////////////////////////////

float slopeDBPerSample(float dbpsec,float samplerate)
{
	//float 
	//float rval = powf(powf(10.0f,(dbpsec/20.0f)),1.0f/samplerate);
	float dbrate = dbpsec/samplerate;
	float rval = powf(10.0f,(dbrate/20.0f));
	return rval;
}


