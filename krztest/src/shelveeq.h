#pragma once

struct ShelveEq
{
    void init();
    float SPN;
};
struct LoShelveEq : public ShelveEq
{
    void init();
    void set( float fc, float peakg );
    float compute(float inp);
    float yl_ls,x1l_ls,x2l_ls,y1l_ls,y2l_ls,yr_ls,x1r_ls,x2r_ls,y1r_ls,y2r_ls;
    float a0_ls, a1_ls, a2_ls, b1_ls, b2_ls;
};
struct HiShelveEq : public ShelveEq
{
    void init();
    void set( float cf, float peakg );
    float compute(float inp);
    float yl_hs,x1l_hs,x2l_hs,y1l_hs,y2l_hs,yr_hs,x1r_hs,x2r_hs,y1r_hs,y2r_hs;
    float a0_hs, a1_hs, a2_hs, b1_hs, b2_hs;
};