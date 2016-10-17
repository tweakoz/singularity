#include "shelveeq.h"
#include "filters.h"
/*
slider1:0<0,1,1{Stereo,Mono}>Processing
slider2:50<0,100,0.05>Low Shelf (Scale)
slider3:0<-24,24,0.01>Gain (dB)
slider4:50<0,100,0.05>High Shelf (Scale)
slider5:0<-24,24,0.01>Gain (dB)
slider6:0<-24,24,0.05>Output (dB)
*/

///////////////////////////////////////////
// @init
///////////////////////////////////////////
void ShelveEq::init()
{
    SPN=0;
}
void LoShelveEq::init()
{
    ShelveEq::init();
    yl_ls=x1l_ls=x2l_ls=y1l_ls=y2l_ls=yr_ls=x1r_ls=x2r_ls=y1r_ls=y2r_ls=0;
}


void LoShelveEq::set(float cf, float peakg)
{
    float freq1 = cf;
    cf *= ISR;

    float sa = tanf(pi*(cf-0.25));
    float asq = sa*sa;
    float A = powf(10,(peakg/20.0));
    
    float F = ((peakg < 6.0) && (peakg > -6.0)) 
            ? sqrt(A)
            : (A > 1.0) 
                ? A/sqrt(2.0)
                : A*sqrt(2.0);
    
    float F2 = F*F;
    float tmp = A*A - F2;

    float gammad = (fabs(tmp) <= SPN)
                 ? 1.0
                 : powf((F2-1.0)/tmp,0.25);

    float gamman = sqrt(A)*gammad;
    float gamma2 = gamman*gamman;
    float gam2p1 = 1.0 + gamma2;
    float siggam2 = 2.0*sqrt(2.0)/2.0*gamman;
    float ta0 = gam2p1 + siggam2;
    float ta1 = -2.0*(1.0 - gamma2);
    float ta2 = gam2p1 - siggam2;
          gamma2 = gammad*gammad;
          gam2p1 = 1.0 + gamma2;
          siggam2 = 2.0*sqrt(2.0)/2.0*gammad;
    float tb0 = gam2p1 + siggam2;
    float tb1 = -2.0*(1.0 - gamma2);
    float tb2 = gam2p1 - siggam2;

    float aa1 = sa*ta1;
    float a0 = ta0 + aa1 + asq*ta2;
    float a1 = 2.0*sa*(ta0+ta2)+(1.0+asq)*ta1;
    float a2 = asq*ta0 + aa1 + ta2;

    float ab1 = sa*tb1;
    float b0 = tb0 + ab1 + asq*tb2;
    float b1 = 2.0*sa*(tb0+tb2)+(1.0+asq)*tb1;
    float b2 = asq*tb0 + ab1 + tb2;

    float recipb0 = 1.0/b0;
    a0 *= recipb0;
    a1 *= recipb0;
    a2 *= recipb0;
    b1 *= recipb0;
    b2 *= recipb0;

    a0_ls = a0;  
    a1_ls = a1;
    a2_ls = a2;
    b1_ls = -b1;
    b2_ls = -b2;

}

void HiShelveEq::init()
{
    ShelveEq::init();
    yl_hs=x1l_hs=x2l_hs=y1l_hs=y2l_hs=yr_hs=x1r_hs=x2r_hs=y1r_hs=y2r_hs=0;
}
void HiShelveEq::set(float cf, float peakg )
{
    float freq2 = cf;
    cf *= ISR;
    float boost = -peakg;

    float sa = tan(pi*(cf-0.25));
    float asq = sa*sa;
    float A = powf(10,(boost/20.0));
    float F = ((boost < 6.0) && (boost > -6.0))
            ? sqrt(A)
            : (A > 1.0) 
              ? A/sqrt(2.0) 
              : A*sqrt(2.0);

    float F2 = F*F;
    float tmp = A*A - F2;
    float gammad = (fabs(tmp) <= SPN)
                 ? 1.0
                 : powf((F2-1.0)/tmp,0.25);
    float gamman = sqrt(A)*gammad;
    float gamma2 = gamman*gamman;
    float gam2p1 = 1.0 + gamma2;
    float siggam2 = 2.0*sqrt(2.0)/2.0*gamman;
    float ta0 = gam2p1 + siggam2;
    float ta1 = -2.0*(1.0 - gamma2);
    float ta2 = gam2p1 - siggam2;
          gamma2 = gammad*gammad;
          gam2p1 = 1.0 + gamma2;
          siggam2 = 2.0*sqrt(2.0)/2.0*gammad;
    float tb0 = gam2p1 + siggam2;
    float tb1 = -2.0*(1.0 - gamma2);
    float tb2 = gam2p1 - siggam2;

    float aa1 = sa*ta1;
    float a0 = ta0 + aa1 + asq*ta2;
    float a1 = 2.0*sa*(ta0+ta2)+(1.0+asq)*ta1;
    float a2 = asq*ta0 + aa1 + ta2;

    float ab1 = sa*tb1;
    float b0 = tb0 + ab1 + asq*tb2;
    float b1 = 2.0*sa*(tb0+tb2)+(1.0+asq)*tb1;
    float b2 = asq*tb0 + ab1 + tb2;

    float recipb0 = 1.0/b0;
    a0 *= recipb0;
    a1 *= recipb0;
    a2 *= recipb0;
    b1 *= recipb0;
    b2 *= recipb0;
      
    float gain = powf(10.0f,(boost/20.0));
    a0_hs = a0/gain;
    a1_hs = a1/gain; 
    a2_hs = a2/gain; 
    b1_hs = -b1;
    b2_hs = -b2;

}
///////////////////////////////////////////
// @sample
///////////////////////////////////////////

///////////////////////////////////////////
//LS
///////////////////////////////////////////
float LoShelveEq::compute(float inp)
{
    float xl_ls = inp;
     
    yl_ls = a0_ls*xl_ls + a1_ls*x1l_ls + a2_ls*x2l_ls + b1_ls*y1l_ls + b2_ls*y2l_ls;
    x2l_ls = x1l_ls;
    x1l_ls = xl_ls;
    y2l_ls = y1l_ls;
    y1l_ls = yl_ls;

    return yl_ls;
}

///////////////////////////////////////////
//HS
///////////////////////////////////////////
float HiShelveEq::compute(float inp)
{
    float xl_hs = inp;
     
    yl_hs = a0_hs*xl_hs + a1_hs*x1l_hs + a2_hs*x2l_hs + b1_hs*y1l_hs + b2_hs*y2l_hs;
    x2l_hs = x1l_hs;
    x1l_hs = xl_hs;
    y2l_hs = y1l_hs;
    y1l_hs = yl_hs;

    return yl_hs;
}


