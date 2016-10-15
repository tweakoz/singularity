#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*
As the SHAPER receives input signals, it evaluates the signal’s level
according to its own internal scale. When the SHAPER’s Adjust value is
at .25, an input signal moving from negative full scale to positive
full scale (a sawtooth) will map to an output curve with a single-
cycle sine wave shape. At an adjust value of .5, the same input signal
would map to a 2-cycle sine wave output signal. Adjust values of .75
and 1.0 for the SHAPER would map to 3-and 4-cycle sine wave output
signals, respectively. Beyond values of 1.0, some portions of the
output will pin at zero-scale.
*/

float shaper(float inp, float adj)
{
	float index = pi*3.0f*inp*adj;

	//adj = 0.85f;
	//adj *= 5.0f;

	//float absinp = fabs(inp);
	//float inpsqu = inp*inp;
	//float N = inp * (absinp + adj); 
	//float D = (inpsqu+(adj-1.0f)*absinp+1.0f); 
	//float rval = N/D;
	//printf( "adj<%g> N<%g> D<%g> rv<%g>\n", adj, N, D, rval );
	return sinf(index); ///adj;
}

///////////////////////////////////////////////////////////////////////////////

float wrap(float inp, float adj)
{
	float wrapamp = 30.0f+adj;
	//printf( "wrap adj<%f> amp<%f>\n", adj, wrapamp );
	float amped = decibel_to_linear_amp_ratio(wrapamp);
	return fmod(inp*amped,1.0f);
}

///////////////////////////////////////////////////////////////////////////////





