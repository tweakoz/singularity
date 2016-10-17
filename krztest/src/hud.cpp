#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "svariant.h"
#include "drawtext.h"

#include "krzdata.h"
#include "krztest.h"
#include "fft.h"

typedef ork::svar256_t svar_t;

void DrawEnv(synth* s, layer* layer, svar_t env, int VPW, int VPH, int X1, int Y1, int W, int H);
void DrawAsr(synth* s, layer* layer, AsrInst* env, int VPW, int VPH, int X1, int Y1, int W, int H);
void DrawLfo(synth* s, layer* layer, LfoInst* lfo, int VPW, int VPH, int X1, int Y1, int W, int H);
void DrawFun(synth* s, layer* layer, FunInst* fun, int VPW, int VPH, int X1, int Y1, int W, int H);

const float fontscale = 0.40;

void PushOrtho(float VPW, float VPH)
{   glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,VPW,VPH,0,0,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}
void PopOrtho()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}
///////////////////////////////////////////////////////////////////////////////

void synth::onDrawHud(float width, float height)
{
    if( _clearhuddata )
    {
        _hudsample_map.clear();
        _clearhuddata = false;
    }


    if(_hudpage==0)
        this->onDrawHudPage1(width,height);
    else if(_hudpage==1)
        this->onDrawHudPage2(width,height);
    else if(_hudpage==2)
        this->onDrawHudPage3(width,height);
}

///////////////////////////////////////////////////////////////////////////////

void synth::onDrawHudPage1(float width, float height)
{
    if( false == (_hudLayer && _hudLayer->_layerData) )
        return;

    std::lock_guard<std::mutex> lock(_hudLayer->_mutex);

    int ldi = _hudLayer->_ldindex+1;
    int ln = _hudLayer->_curnote;
    auto ld = _hudLayer->_layerData;
    auto km = ld->_keymap;
    auto kmr = _hudLayer->_kmregion;
    const auto& KMP = ld->_kmpBlock;
    const auto& PCH = ld->_pchBlock;
    const auto& AMP = ld->_f4Block;
    const auto& ENVCT = ld->_envCtrlData;

    const auto& sosc = _hudLayer->_spOsc;
    const auto& nenv = _hudLayer->_natAmpEnv;
    auto sample = sosc._sample;
    const auto& samname = sample->_name;
    double sampleindex = double(sosc._pbindex)/65536.0f;
    float ampDB = linear_amp_ratio_to_decibel(nenv._curamp);
    int pitchadj = _hudLayer->_curpitchadjx;
    const auto& ampenv = _hudLayer->_userAmpEnv;

    bool useNENV = ENVCT._useNatEnv;

    /////////////////////////////////////////////////
    // basic layer/keymap/sample info
    /////////////////////////////////////////////////
    float segtim = nenv._segtime;
    {
        auto hudstr  =  formatString("lay.curnote<%d>\n",ln);
             hudstr +=  formatString("lay.kmtransp<%d>\n",KMP._transpose);
             hudstr +=  formatString("lay.kmtimbshift<%d>\n",KMP._timbreShift);
             hudstr +=  formatString("lay.kmkeytrk<%0.2f>\n",KMP._keyTrack);
             if( kmr )
             {
                 hudstr +=  formatString("kmr.tuning<%d>\n",kmr->_tuning);
                 hudstr +=  formatString("kmr.voldaj<%f>\n",kmr->_volAdj);
                 hudstr +=  formatString("kmr.lok<%d> hik<%d>\n",kmr->_lokey,kmr->_hikey);
             }
             hudstr +=  formatString("lay.pchcoarse<%0.2f>\n",PCH._coarse);
             hudstr +=  formatString("lay.pchkeytrk<%0.2f>\n",PCH._keyTrack);
             //hudstr +=  formatString("lay.natenv<%d>\n",int(ld->_useNatEnv));
             hudstr +=  formatString("lay.sampselnote<%d>\n",int(_hudLayer->_sampselnote));
             hudstr +=  formatString("lay.samppbnote<%d>\n",int(_hudLayer->_samppbnote));
             hudstr +=  formatString("lay.basecentsOSC<%d>\n",_hudLayer->_basecentsOSC);
             hudstr +=  formatString("lay.curcentsOSC<%d>\n",_hudLayer->_curcentsOSC);
             hudstr +=  formatString("lay.curcents<%d>\n",int(_hudLayer->_curcents));
             hudstr +=  formatString("lay._kmcents<%d>\n",int(_hudLayer->_kmcents));
             hudstr +=  formatString("lay._pchcents<%d>\n",int(_hudLayer->_pchcents));
             hudstr +=  formatString("lay._keydiff<%d>\n",int(_hudLayer->_keydiff));


        drawtext( hudstr, 50,245, .45, 1,1,1 );
    }
    if( sample )
    {
        auto hudstr  =  formatString("samp.rootkey<%d>\n",sample->_rootKey);
             hudstr +=  formatString("samp.name<%s>\n",samname.c_str());
             hudstr +=  formatString("samp.SR<%d>\n",int(sample->_sampleRate));
             hudstr +=  formatString("samp.HP<%d>\n",sample->_highestPitch);
             hudstr +=  formatString("samp.pitchADJX<%d>\n",pitchadj);
             hudstr +=  formatString("samp.pitchADJ<%d>\n",sample->_pitchAdjust);
             hudstr +=  formatString("osc.samidx<%0.1f>\n",sampleindex);
             hudstr +=  formatString("amp.pad<%d>\n",int(AMP._pad));
             hudstr +=  formatString("amp.coarse<%d>\n",int(AMP._coarse));
             hudstr +=  formatString("lay.pchs1<%s>\n",PCH._mods._src1.c_str());
             hudstr +=  formatString("lay.pchd1<%0.2f>\n",PCH._mods._src1Depth);
             hudstr +=  formatString("lay.pchc1<%0.2f>\n",_hudLayer->_pchc1);
             hudstr +=  formatString("lay.pchs2<%s>\n",PCH._mods._src2.c_str());
             hudstr +=  formatString("lay.pchd2s<%s>\n",PCH._mods._src2DepthCtrl.c_str());
             hudstr +=  formatString("lay.pchd2min<%0.2f>\n",PCH._mods._src2MinDepth);
             hudstr +=  formatString("lay.pchd2max<%0.2f>\n",PCH._mods._src2MaxDepth);
             hudstr +=  formatString("lay.pchc2<%0.2f>\n",_hudLayer->_pchc2);

        drawtext( hudstr, 300,245, .45, 1,1,1 );
    }
    {   const auto& l1 = _hudLayer->_lfo1;
        const auto& l2 = _hudLayer->_lfo2;
        std::string hudstr;
        if( l1._data )
        {
             hudstr +=  formatString("lfo1.freq<%0.3f>\n",l1._currate);
             hudstr +=  formatString("lfo1.phas<%0.3f>\n",l1._phase );
             hudstr +=  formatString("lfo1.shap<%s>\n",l1._data->_shape.c_str());
             hudstr +=  formatString("lfo1.cval<%0.3f>\n",l1._curval );
         }
         if( l2._data )
         {
             hudstr +=  formatString("lfo2.freq<%0.3f>\n",l2._currate);
             hudstr +=  formatString("lfo2.phas<%0.3f>\n",l2._phase );
             hudstr +=  formatString("lfo2.shap<%s>\n",l2._data->_shape.c_str());
             hudstr +=  formatString("lfo2.cval<%0.3f>\n",l2._curval );
         }
         hudstr +=  formatString("envc.atkadj<%0.3f>\n",ENVCT._atkAdjust );
         hudstr +=  formatString("envc.decadj<%0.3f>\n",ENVCT._decAdjust );
         hudstr +=  formatString("envc.reladj<%0.3f>\n",ENVCT._relAdjust );
        drawtext( hudstr, 800,45, .45, 1,1,1 );
    }
    {   std::string hudstr = "";
        const auto f1 = ld->_fun1;
        const auto f2 = ld->_fun2;
        const auto f3 = ld->_fun3;
        const auto f4 = ld->_fun4;
         if( f1 ){
             hudstr +=  formatString("fun1.a<%s>\n",f1->_a.c_str() );
             hudstr +=  formatString("fun1.b<%s>\n",f1->_b.c_str() );
             hudstr +=  formatString("fun1.op<%s>\n",f1->_op.c_str() );
         }
         if( f2 ){
             hudstr +=  formatString("fun2.a<%s>\n",f2->_a.c_str() );
             hudstr +=  formatString("fun2.b<%s>\n",f2->_b.c_str() );
             hudstr +=  formatString("fun2.op<%s>\n",f2->_op.c_str() );
         }
         if( f3 ){
             hudstr +=  formatString("fun3.a<%s>\n",f3->_a.c_str() );
             hudstr +=  formatString("fun3.b<%s>\n",f3->_b.c_str() );
             hudstr +=  formatString("fun3.op<%s>\n",f3->_op.c_str() );
         }
         if( f4 ){
             hudstr +=  formatString("fun4.a<%s>\n",f4->_a.c_str() );
             hudstr +=  formatString("fun4.b<%s>\n",f4->_b.c_str() );
             hudstr +=  formatString("fun4.op<%s>\n",f4->_op.c_str() );
         }
        drawtext( hudstr, 1000,45, .45, 1,1,1 );
    }
    float fxb = 50;
    float fh = 300.0f;
    float fw = 600.0f;
    float fyb = height-fh;

    svar_t svar;
    if( useNENV )
    {   auto env = & _hudLayer->_natAmpEnv;
        svar.Set<NatEnv*>(env);
    }
    else
    {   auto env = & _hudLayer->_userAmpEnv;
        svar.Set<RateLevelEnvInst*>(env);
    }
    DrawEnv(this,_hudLayer,svar,width,height,fxb,fyb,fw,fh);

}

///////////////////////////////////////////////////////////////////////////////

void synth::onDrawHudPage2(float width, float height)
{ 
    auto hudl =  _hudLayer;

    if( false == (hudl && hudl->_layerData) )
        return;

    std::lock_guard<std::mutex> lock(hudl->_mutex);

    auto ld = hudl->_layerData;
    const auto& ENVCT = ld->_envCtrlData;
    bool useNENV = ENVCT._useNatEnv;

    float fxb = 0;
    float fh = 360.0f;
    float fw = 800.0f;
    float fyb = (height-1)-fh*3;

    svar_t svar;
    if( useNENV )
    {   auto env = & _hudLayer->_natAmpEnv;
        svar.Set<NatEnv*>(env);
    }
    else
    {   auto env = & _hudLayer->_userAmpEnv;
        svar.Set<RateLevelEnvInst*>(env);
    }
    DrawEnv(this,hudl,svar,width,height,fxb,fyb,fw,fh);

    {   auto env = & _hudLayer->_env2;
        svar.Set<RateLevelEnvInst*>(env);
        fyb += fh;
        DrawEnv(this,hudl,svar,width,height,fxb,fyb,fw,fh);
    }
    {   auto env = & hudl->_env3;
        svar.Set<RateLevelEnvInst*>(env);
        fyb += fh;
        DrawEnv(this,hudl,svar,width,height,fxb,fyb,fw,fh);
    }
    if( hudl->_asr1.isValid() )
    {   fyb = (height-1)-fh*3;
        fxb += fw;
        DrawAsr(this,hudl,& hudl->_asr1,width,height,fxb,fyb,fw,fh);
    }
    if( hudl->_asr2.isValid() )
    {   fyb += fh;
        DrawAsr(this,hudl,& hudl->_asr2,width,height,fxb,fyb,fw,fh);
    }
    //////////////////
    {   fyb = (height-1)-fh*1;
        fxb = fw;
        DrawLfo(this,hudl,& hudl->_lfo1,width,height,fxb,fyb,fw*0.5,fh);
    }
    {   fxb += fw*0.5f;
        DrawLfo(this,hudl,& hudl->_lfo2,width,height,fxb,fyb,fw*0.5,fh);
    }
    //////////////////
    {   fyb = (height-1)-fh*4;
        fxb = fw*2.0;
        DrawFun(this,hudl,& hudl->_fun1,width,height,fxb,fyb,fw*0.5,fh);
    }
    {   fyb = (height-1)-fh*3;
        DrawFun(this,hudl,& hudl->_fun2,width,height,fxb,fyb,fw*0.5,fh);
    }
    {   fyb = (height-1)-fh*2;
        DrawFun(this,hudl,& hudl->_fun3,width,height,fxb,fyb,fw*0.5,fh);
    }
    {   fyb = (height-1)-fh*1;
        DrawFun(this,hudl,& hudl->_fun4,width,height,fxb,fyb,fw*0.5,fh);
    }
    //////////////////


}

///////////////////////////////////////////////////////////////////////////////

void DrawBorder(int X1, int Y1, int X2, int Y2, int color=0)
{
    switch(color)
    {
      case 0:
        glColor4f(0.6,0.3,0.6,1);
        break;
      case 1:
        glColor4f(0.0,0.0,0.0,1);
        break;
      case 2:
        glColor4f(0.9,0.0,0.0,1);
        break;
    }

    glBegin(GL_LINES);

        glVertex3f(X1,Y1,0.0f);
        glVertex3f(X2,Y1,0.0f);

        glVertex3f(X2,Y1,0.0f);
        glVertex3f(X2,Y2,0.0f);

        glVertex3f(X2,Y2,0.0f);
        glVertex3f(X1,Y2,0.0f);

        glVertex3f(X1,Y2,0.0f);
        glVertex3f(X1,Y1,0.0f);

    glEnd();

}

///////////////////////////////////////////////////////////////////////////////

void DrawEnv(synth* s, layer* layer, svar_t env, int VPW, int VPH, int X1, int Y1, int W, int H)
{
    float X2 = X1+W;
    float Y2 = Y1+H;

    int ldi = layer->_ldindex+1;
    auto ld = layer->_layerData;
    const auto& AMP = ld->_f4Block;
    const auto& ENVCT = ld->_envCtrlData;
    bool collsamp = ( s->_lnoteframe%3 == 0 );

    bool useNENV = env.IsA<NatEnv*>();
    auto nenv = useNENV ? env.Get<NatEnv*>() : (NatEnv*) nullptr;
    auto ampenv = useNENV ? (RateLevelEnvInst*) nullptr : env.Get<RateLevelEnvInst*>();

    std::string sampname = "???";

    if( useNENV )
        sampname = "natural";
    else
        sampname = (ampenv&&ampenv->_data) ? ampenv->_data->_name : "blah";
    
    /////////////////////////////////////////////////
    // collect hud samples
    /////////////////////////////////////////////////

    auto& HUDSAMPS = s->_hudsample_map[sampname];

    if( collsamp )
    {
        //printf( "time<%f> ampDB<%f>\n", _lnotetime, ampDB );
        hudsample hs;
        hs._time = s->_lnotetime;

        hs._value = useNENV
                  ? nenv->_curamp
                  : ampenv->_curval;


        HUDSAMPS.push_back(hs);
    }
    /////////////////////////////////////////////////
    // draw envelope values
    /////////////////////////////////////////////////
    float env_by = Y1+20;
    float env_bx = X1+70.0;
    int spcperseg = 70;
    if( nenv )
    {
        const auto& NES = nenv->_natenv;
        int nseg = NES.size();
        int icurseg = nenv->_curseg;


        drawtext( formatString("NATENV(L%d)",ldi), env_bx,env_by, fontscale, 1,0,0 );

        for( int i=0; i<nseg; i++ )
        {
            bool iscurseg = (i==icurseg);
            float r = 1;
            float g = iscurseg ? 0.5 : 1;

            auto hudstr = formatString("seg%d",i);
            drawtext( hudstr, env_bx+spcperseg*i,env_by+20, fontscale, r,g,0 );
        }
        drawtext( "dB/s\ntim", X1+15,env_by+40, .45, 1,1,0 );
        for( int i=0; i<nseg; i++ )
        {
            const auto& seg = NES[i];

            auto hudstr = formatString("%0.1f\n%d",seg._slope, int(seg._time));
            bool iscurseg = (i==icurseg);
            float r = 1;
            float g = iscurseg ? 0 : 1;
            float b = iscurseg ? 0 : 1;
            drawtext( hudstr, env_bx+spcperseg*i,env_by+40, fontscale, r,g,1 );
        }
        
        /*auto hudstr = formatString("amp(dB)<%0.1f>\n",ampDB);
        hudstr += formatString("dB/sec<%0.1f>\n",(nenv._slopePerSecond));
        drawtext( hudstr, 50,height-50, .45, 1,1,1 );

        hudstr = formatString("env seg<%d/%d>\n",(nenv._curseg+1),nenv._numseg);
        hudstr += formatString("env segtim(sec)<%0.1f>\n",segtim);
        drawtext( hudstr, 350,height-50, .45, 1,1,1 );*/
    }
    else if( ampenv && ampenv->_data )
    {           
        auto ENVD = ampenv->_data;
        auto& AE = ENVD->_segments;
        drawtext( formatString("%s(L%d)",sampname.c_str(),ldi), X1,env_by, fontscale, 1,0,.5 );
        int icurseg = ampenv->_curseg;
        for( int i=0; i<7; i++ )
        {
            std::string segname;
            if(i<3)
                segname = formatString("atk%d",i+1);
            else if(i==3)
                segname = "dec";
            else
                segname = formatString("rel%d",i-3);
            auto hudstr = formatString("%s",segname.c_str());

            bool iscurseg = (i==icurseg);
            float r = 1;
            float g = iscurseg ? 0.5 : 1;

            drawtext( hudstr, env_bx+spcperseg*i,env_by+20, fontscale, r,g,0 );
        }
        drawtext( "lev\ntim", X1+15,env_by+40, .45, 1,1,0 );
        for( int i=0; i<7; i++ )
        {
            auto hudstr = formatString("%0.2f\n%0.2f",AE[i]._level, AE[i]._rate);
            bool iscurseg = (i==icurseg);
            float r = 1;
            float g = iscurseg ? 0 : 1;
            float b = iscurseg ? 0 : 1;
            drawtext( hudstr, env_bx+spcperseg*i,env_by+40, fontscale, r,g,1 );
        }
    }
    /////////////////////////////////////////////////
    // draw envelope segments
    /////////////////////////////////////////////////

    float fxb = X1;
    float fyb = Y2;
    float fw = W;
    float fpx = fxb;
    float fpy = fyb;

    PushOrtho(VPW,VPH);

    ///////////////////////
    // draw border
    ///////////////////////

    DrawBorder(X1,Y1,X2,Y2);

    const float ktime = 20.0f;

    float fh = useNENV 
             ? 200
             : 400;

    bool bipolar = ampenv && ampenv->_data && ampenv->_data->isBiPolar();


    ///////////////////////
    // draw grid, origin
    ///////////////////////

    glColor4f(.5,.2,.5,1);
    glBegin(GL_LINES);
    if( bipolar )
    {
        float fy = fyb-(fh*0.5f)*0.5f;
        float x1 = fxb;
        float x2 = x1+fw;

        glVertex3f(x1,fy,0);
        glVertex3f(x2,fy,0);
    }
    glEnd();

    ///////////////////////
    // from hud samples
    ///////////////////////

    glColor4f(1,1,1,1);
    glBegin(GL_LINES);
    for( int i=0; i<HUDSAMPS.size(); i++ )
    {     const auto& hs = HUDSAMPS[i];
          if( fpx >= X1 and fpx <= X2 )
          {
              glVertex3f(fpx,fpy,0);
              fpx = fxb + hs._time*(fw/ktime);
                
              float fval = hs._value;
              if( bipolar )
                fval = 0.5f+(fval*0.5f);
              fpy = fyb - (fh*0.5f)*fval;

              glVertex3f(fpx,fpy,0);
          }
    }
    glEnd();

    ///////////////////////
    // natural env dB slopehull 
    ///////////////////////

    if( useNENV )
    {
        glColor4f(1,0,0,1);
        fpx = fxb;
        fpy = fyb;
        glBegin(GL_LINES);
        for( int i=0; i<HUDSAMPS.size(); i++ )
        {     const auto& hs = HUDSAMPS[i];
              glVertex3f(fpx,fpy,0);

              float val = hs._value;
              val = 96.0 + linear_amp_ratio_to_decibel(val);
              if( val<0.0f ) val = 0.0f;
              val = val/96.0f;

              fpx = fxb + hs._time*(fw/ktime);
              fpy = fyb - fh*val;
              glVertex3f(fpx,fpy,0);
        }
        glEnd();
    }

    PopOrtho();

    /////////////////////////////////////////////////    
}

///////////////////////////////////////////////////////////////////////////////

void DrawAsr(synth* s, layer* layer, AsrInst* env, int VPW, int VPH, int X1, int Y1, int W, int H)
{
    float X2 = X1+W;
    float Y2 = Y1+H;

    int ldi = layer->_ldindex+1;
    auto ld = layer->_layerData;
    const auto& AMP = ld->_f4Block;
    const auto& ENVCT = ld->_envCtrlData;
    bool collsamp = ( s->_lnoteframe%3 == 0 );

    std::string sampname = (env&&env->_data) ? env->_data->_name : "blah";
    
    /////////////////////////////////////////////////
    // collect hud samples
    /////////////////////////////////////////////////

    auto& HUDSAMPS = s->_hudsample_map[sampname];

    if( collsamp )
    {
        //printf( "time<%f> ampDB<%f>\n", _lnotetime, ampDB );
        hudsample hs;
        hs._time = s->_lnotetime;

        hs._value = env->_curval;


        HUDSAMPS.push_back(hs);
    }
    /////////////////////////////////////////////////
    // draw envelope values
    /////////////////////////////////////////////////
    float env_by = Y1+20;
    float env_bx = X1+70.0;
    int spcperseg = 70;

    if( env && env->_data )
    {           
        auto ENVD = env->_data;
        //auto& AE = ENVD->_segments;
        drawtext( formatString("%s(L%d)",sampname.c_str(),ldi), X1,env_by, fontscale, 1,0,.5 );
        int icurseg = env->_curseg;
        float deltime = ENVD->_delay;
        float atktime = ENVD->_attack;
        float reltime = ENVD->_release;
        float sx0 = env_bx+spcperseg*0;
        float sx1 = env_bx+spcperseg*1;
        float sx2 = env_bx+spcperseg*2;
        float by0 = env_by+20;
        float by1 = env_by+40;

        float r = 1;
        float gD = (icurseg==0) ? 0.5 : 1;
        float gA = (icurseg==1) ? 0.5 : 1;
        float gR = (icurseg==3) ? 0.5 : 1;
        drawtext( "del", sx0, by0, .45, r,gD,0 );
        drawtext( "atk", sx1, by0, .45, r,gA,0 );
        drawtext( "rel", sx2, by0, .45, r,gR,0 );

        drawtext( "tim", X1+15,by1, .45, 1,1,0 );

        drawtext( formatString("%0.2f", deltime ), sx0,by1, fontscale, r,gD,1 );
        drawtext( formatString("%0.2f", atktime ), sx1,by1, fontscale, r,gA,1 );
        drawtext( formatString("%0.2f", reltime ), sx2,by1, fontscale, r,gR,1 );

    }
    /////////////////////////////////////////////////
    // draw envelope segments
    /////////////////////////////////////////////////

    float fxb = X1;
    float fyb = Y2;
    float fw = W;
    float fpx = fxb;
    float fpy = fyb;

    PushOrtho(VPW,VPH);

    ///////////////////////
    // draw border
    ///////////////////////

    DrawBorder(X1,Y1,X2,Y2);

    const float ktime = 20.0f;

    fyb -=1;
    float fh = 400-1;

    ///////////////////////
    // from hud samples
    ///////////////////////

    glColor4f(1,1,1,1);
    glBegin(GL_LINES);
    for( int i=0; i<HUDSAMPS.size(); i++ )
    {     const auto& hs = HUDSAMPS[i];
          if( fpx >= X1 and fpx <= X2 )
          {
              glVertex3f(fpx,fpy,0);
              fpx = fxb + hs._time*(fw/ktime);
                
              float fval = hs._value;
              fpy = fyb - (fh*0.5f)*fval;

              glVertex3f(fpx,fpy,0);
          }
    }
    glEnd();

    PopOrtho();

    /////////////////////////////////////////////////    
}

///////////////////////////////////////////////////////////////////////////////

void DrawLfo(synth* s, layer* layer, LfoInst* lfo, int VPW, int VPH, int X1, int Y1, int W, int H)
{
    if( lfo==nullptr ) return; 
    if( lfo->_data==nullptr ) return; 

    auto LFOD = lfo->_data;

    auto lfoname = LFOD->_name;
    bool collsamp = true; //( s->_lnoteframe%1 == 0 );


    auto& HUDSAMPS = s->_hudsample_map[lfoname];

    float ktime = 4.0f / lfo->_currate;
    if( ktime > 4.0f )
        ktime = 4.0f;
    int maxsamps = int(ktime*50.0f);

    while( HUDSAMPS.size() > maxsamps )
    {
        HUDSAMPS.erase(HUDSAMPS.begin());
    }

    if( collsamp )
    {
        //printf( "time<%f> ampDB<%f>\n", _lnotetime, ampDB );
        hudsample hs;
        hs._time = s->_lnotetime;

        hs._value = lfo->_curval;

        HUDSAMPS.push_back(hs);
    }

    int spcperseg = 70;
    int ldi = layer->_ldindex+1;
    float X2 = X1+W;
    float Y2 = Y1+H;
    float env_bx = X1+70.0;
    float env_by = Y1+20;
    float fxb = X1;
    float fyb = Y2-1;
    float fw = W;
    float fpx = fxb;
    float fh = 200;
    float sx0 = env_bx+spcperseg*0;
    float sx1 = env_bx+spcperseg*1;
    float sx2 = env_bx+spcperseg*2;
    float by0 = env_by+20;
    float by1 = env_by+40;
    float by2 = env_by+60;

    ///////////////////////

    drawtext( formatString("%s(L%d)",lfoname.c_str(),ldi), X1,env_by, fontscale, 1,0,.5 );

    drawtext( "shp", X1+15,by0, fontscale, 1,1,0 );
    drawtext( "rat", X1+15,by1, fontscale, 1,1,0 );
    drawtext( "pha", X1+15,by2, fontscale, 1,1,0 );
    drawtext( LFOD->_shape, sx0,by0, fontscale, 1,1,1 );
    drawtext( formatString("%0.2f",lfo->_currate), sx0,by1, fontscale, 1,1,1 );
    drawtext( formatString("%d",int(LFOD->_initialPhase*90.0f)), sx0,by2, fontscale, 1,1,1 );

    ///////////////////////

    PushOrtho(VPW,VPH);

    ///////////////////////
    // draw border, grid, origin
    ///////////////////////

    float fyc = fyb-(fh*0.5f);
    float x1 = fxb;
    float x2 = x1+fw;

    DrawBorder(X1,Y1,X2,Y2);

    glColor4f(.5,.2,.5,1);
    glBegin(GL_LINES);
        glVertex3f(x1,fyc,0);
        glVertex3f(x2,fyc,0);
    glEnd();

    ///////////////////////
    // from hud samples
    ///////////////////////

    glColor4f(1,1,1,1);
    glBegin(GL_LINES);
    float fpy = fyc;
    if( HUDSAMPS.size() )
    {
        const auto& HS0 = HUDSAMPS[0];
        float ftimebase = HS0._time;
        float s0 = HS0._value;
        fpy = fyc-s0*100.0f;

        for( int i=0; i<HUDSAMPS.size(); i++ )
        {     const auto& hs = HUDSAMPS[i];
              if( fpx >= X1 and fpx <= X2 )
              {
                  glVertex3f(fpx,fpy,0);
                  float t = hs._time-ftimebase;
                  fpx = fxb + t*(fw/ktime);
                    
                  float fval = hs._value;
                  fpy = fyc-fval*100.0f;

                  glVertex3f(fpx,fpy,0);
              }
        }
    }
    glEnd();

    PopOrtho();
}

///////////////////////////////////////////////////////////////////////////////

void DrawFun(synth* s, layer* layer, FunInst* fun, int VPW, int VPH, int X1, int Y1, int W, int H)
{
    if( nullptr == fun ) return;
    if( nullptr == fun->_data ) return;

    float X2 = X1+W;
    float Y2 = Y1+H;
    int spcperseg = 70;
    int ldi = layer->_ldindex+1;
    float env_bx = X1+70.0;
    float env_by = Y1+20;
    float fxb = X1;
    float fyb = Y2-1;
    float fw = W;
    float fpx = fxb;
    float fh = 300;
    float sx0 = env_bx+spcperseg*0;
    float sx1 = env_bx+spcperseg*1;
    float sx2 = env_bx+spcperseg*2;
    float by0 = env_by+20;
    float by1 = env_by+40;
    float by2 = env_by+60;

    //printf( "fun<%p>\n", fun );
    if( fun==nullptr )
        return; 

    auto& FUND = *fun->_data;

    auto funname = FUND._name;

    float ktime = 3.0f;
    int maxsamps = int(ktime*35.0f);

    auto& HUDSAMPS = s->_hudsample_map[funname];

    while( HUDSAMPS.size() > maxsamps )
    {
        HUDSAMPS.erase(HUDSAMPS.begin());
    }

    hudsample hs;
    hs._time = s->_lnotetime;
    hs._value = fun->_curval;
    HUDSAMPS.push_back(hs);

    drawtext( formatString("%s(L%d)",funname.c_str(),ldi), X1,env_by, fontscale, 1,0,.5 );

    drawtext( "a", X1+15,by0, fontscale, 1,1,0 );
    drawtext( "b", X1+15,by1, fontscale, 1,1,0 );
    drawtext( "op", X1+15,by2, fontscale, 1,1,0 );
    drawtext( FUND._a, sx0,by0, fontscale, 1,1,1 );
    drawtext( FUND._b, sx0,by1, fontscale, 1,1,1 );
    drawtext( FUND._op, sx0,by2, fontscale, 1,1,1 );

    PushOrtho(VPW,VPH);
    DrawBorder(X1,Y1,X2,Y2);

    ///////////////////////
    // draw border, grid, origin
    ///////////////////////

    float fyc = fyb-(fh*0.5f);
    float x1 = fxb;
    float x2 = x1+fw;

    DrawBorder(X1,Y1,X2,Y2);

    glColor4f(.5,.2,.5,1);
    glBegin(GL_LINES);
        glVertex3f(x1,fyc,0);
        glVertex3f(x2,fyc,0);
    glEnd();

    ///////////////////////
    // from hud samples
    ///////////////////////

    glColor4f(1,1,1,1);
    glBegin(GL_LINES);
    float fpy = fyc;
    if( HUDSAMPS.size() )
    {
        const auto& HS0 = HUDSAMPS[0];
        float ftimebase = HS0._time;
        float s0 = HS0._value;
        fpy = fyc-s0*100.0f;

        for( int i=0; i<HUDSAMPS.size(); i++ )
        {     const auto& hs = HUDSAMPS[i];
              if( fpx >= X1 and fpx <= X2 )
              {
                  glVertex3f(fpx,fpy,0);
                  float t = hs._time-ftimebase;
                  fpx = fxb + t*(fw/ktime);
                    
                  float fval = hs._value;
                  fpy = fyc-fval*100.0f;

                  glVertex3f(fpx,fpy,0);
              }
        }
    }
    glEnd();


    PopOrtho();

}

void synth::onDrawHudPage3(float width, float height)
{
  auto hudl = _hudLayer;

    if( false == (hudl && hudl->_layerData) )
        return;

    std::lock_guard<std::mutex> lock(hudl->_mutex);

    drawtext( formatString("ostrack<%g>",_ostrack), 100,250, fontscale, 1,1,0 );

    _ostrackPH += _ostrack;

    int inumframes = layer::koscopelength;

    if( _ostrack>0 and _ostrackPH>inumframes )
      _ostrackPH = 0;
    if( _ostrack<0 and _ostrackPH<-inumframes )
      _ostrackPH = 0;

    const float* ldata = hudl->_oscopebuffer;

    float OSC_X1 = 100;
    float OSC_Y1 = 250;
    float OSC_W = 1300;
    float OSC_H = 500;
    float OSC_HH = OSC_H*0.5;
    float OSC_CY = OSC_Y1+OSC_HH;
    float OSC_X2 = OSC_X1+OSC_W;
    float OSC_Y2 = OSC_Y1+OSC_H;

    float ANA_X1 = 100;
    float ANA_Y1 = 800;
    float ANA_W = 1300;
    float ANA_H = 600;
    float ANA_HH = ANA_H*0.5;
    float ANA_CY = ANA_Y1+ANA_HH;
    float ANA_X2 = ANA_X1+ANA_W;
    float ANA_Y2 = ANA_Y1+ANA_H;

    PushOrtho(width,height);

    //static array_t fftbins;
    //fftbins.resize(inumframes);

  const size_t fftSize = inumframes; // Needs to be power of 2!

  std::vector<float> input(fftSize, 0.0f);
  std::vector<float> re(audiofft::AudioFFT::ComplexSize(fftSize)); 
  std::vector<float> im(audiofft::AudioFFT::ComplexSize(fftSize)); 
  std::vector<float> output(fftSize);

// OSC centerline
      glBegin(GL_LINES);
        glVertex3f(OSC_X1,OSC_H,0);
        glVertex3f(OSC_X2,OSC_H,0);
      glEnd();

    /////////////////////////////////////////////

    glColor4f(.3,1,.3,1);
    glBegin(GL_LINE_STRIP);

      int i = 0;
      auto mapI = [&](int i)->int
      {
          int j = i+_ostrackPH;

          if( j<0 ) 
            j += inumframes;
          else if( j>=inumframes )
            j -= inumframes;

          assert(j>=0);
          assert(j<inumframes);
          return j;
      };

      float x1 = OSC_X1;
      float y1 = OSC_Y1+OSC_HH+ldata[mapI(i)]*OSC_HH;
      glVertex3f(x1,y1,0);      
      for( i=0; i<inumframes; i++ )
      {   int j = mapI(i);

          float s = ldata[j];

          float win_num = pi2*float(i);
          float win_den = inumframes-1;
          float win = 0.5f*(1-cosf(win_num/win_den));
          //printf( "win<%d> : %f\n", i, win );
          float s2 = ldata[i];
          input[i]= s2*win;

          float x = OSC_W*float(i)/float(inumframes);
          float y = OSC_HH+s*OSC_HH;

          float x2 = x+OSC_X1;
          float y2 = y+OSC_HH;
          glVertex3f(x2,y2,0);      
      }
      glEnd();
      
      //////////////////////////////
      // do the FFT
      //////////////////////////////

      audiofft::AudioFFT fft;
      fft.init(fftSize);
      fft.fft(input.data(), re.data(), im.data());

      //////////////////////////////
      // map bin -> Y
      //////////////////////////////

      auto mapDB = [&](float re, float im) ->float
      {
          float mag = re*re+im*im;
          float dB = 10.0f*log_base( 10.0f, mag )-6.0f;
          return dB;
      };
      auto mapFFTY = [&](float dbin) ->float
      {
        float dbY = (dbin+96.0f)/132.0f;

        float y = ANA_Y2-dbY*ANA_H;
        if(y>ANA_Y2)
          y=ANA_Y2;
        return y;
      };

      //////////////////////////////
      glColor4f(.3,.1,.3,1);
      glBegin(GL_LINES);
      for( int i=36; i>=-96; i-=12 )
      {
        float db0 = i;
        glVertex3f(ANA_X1,mapFFTY(db0),.0f);
        glVertex3f(ANA_X2,mapFFTY(db0),.0f);
      }
      glEnd();

      //////////////////////////////

      glColor4f(.3,.7,1,1);
      glBegin(GL_LINE_STRIP);
      float dB = mapDB(re[0],im[0]);
      float x = ANA_W*float(0)/float(inumframes);
      float y = mapFFTY(dB);
      float xx = x+ANA_X1;
      float yy = y;
      glVertex3f(xx,yy,0);      
      for( int i=0; i<inumframes/2; i++ )
      { 
          float dB = mapDB(re[i],im[i]);
          hudl->_fftbuffer[i] += dB*0.1+0.0001f;
          hudl->_fftbuffer[i] *= 0.9f;
          dB = hudl->_fftbuffer[i];

          //printf( "dB<%f>\n", dB);
          float fi = float(i)/float(inumframes/2);

          float frq = fi*48000.0f;
          float midinote = frequency_to_midi_note(frq);

          float x = ANA_W*(midinote-36.0f)/128.0;
          float y = mapFFTY(dB-12);
          float xx = x+ANA_X1;
          glVertex3f(xx,y,0);      
      }
      //freqbins[index] = complex_t(0,0);
      glEnd();
      //////////////////////////////
      glColor4f(.3,.1,.3,1);
      glBegin(GL_LINES);
      for( int n=0; n<128; n+=12 )
      {
        float db0 = i;
        float x = ANA_X1+ANA_W*float(n)/128.0;
        glVertex3f(x,ANA_Y1,0);      
        glVertex3f(x,ANA_Y2,0);      

      }
      glEnd();
  
      //////////////////////////////

    DrawBorder(OSC_X1,OSC_Y1,OSC_X2,OSC_Y2);
    DrawBorder(ANA_X1,ANA_Y1,ANA_X2,ANA_Y2);

      PopOrtho();

      for( int i=36; i>=-96; i-=12 )
      {
        float db0 = i;
        float y = mapFFTY(db0);

        drawtext( formatString("%g dB",db0), 40,y+10, fontscale, .6,0,.8 );

      }
      for( int n=0; n<128; n+=12 )
      {
        float x = ANA_X1-20+ANA_W*float(n)/128.0;
        float f = midi_note_to_frequency(n+36);
        drawtext( formatString("  midi\n   %d\n(%d hz)",n+36,int(f)), x,ANA_Y2+30, fontscale, .6,0,.8 );

      }

      //////////////////////////////
      // draw DSP blocks
      //////////////////////////////

      auto alg = hudl->_alg;

      float xb = 1500;
      float yb = 90;
      float dspw = 500;
      float dsph = 344;
      float yinc = dsph+3;

      glColor4f(.7,.7,.3,1);
      PushOrtho(width,height);
      for( int i=0; i<4; i++ )
      {
         auto b = alg->_block[i];
         int color = 2;
         if( b )
            color = _fblockEnable[i] ? 0 : 1;
         DrawBorder(xb,yb,xb+dspw,yb+dsph,color);
         yb += yinc;

      }

      PopOrtho();

      yb = 90;
      int ytb = 95;
      auto& layd = hudl->_layerData;
      auto& algd = layd->_algData;

      auto alghdr = formatString("DSP Algorithm: %s", algd._name.c_str() );
      drawtext( alghdr, xb+80, yb-30, fontscale, 1,1,1 );

      //////////////////////

      auto PanPadOut = [](const DspBlockData* dbd, int xt, int yt) -> int
      {
            assert(dbd);
            float v14DB = dbd->_v14Gain;
            float padDB = linear_amp_ratio_to_decibel(dbd->_pad);
            auto text = formatString("PAD<%g dB> V14G<%g dB>",padDB, v14DB);
            drawtext( text, xt, yt, fontscale, 1,.8,.8 );

            int h =20;

            if( dbd->_name == "F3" or dbd->_name == "F4" )
            {
              //text = formatString("V15<0x%02x>",dbd._var15);
              //drawtext( text, xt, yt+=20, fontscale, 0.8,0.8,1 );

              std::string panmode;
              switch( dbd->_panMode )
              {
                  case 0:
                    panmode = "Fixed";
                    break;
                  case 1:
                    panmode = "MIDI+";
                    break;
                  case 2:
                    panmode = "Auto";
                    break;
                  case 3:
                    panmode = "Revrs";
                    break;
              }

              text = formatString("Pan<%d> Mode<%s>",dbd->_pan, panmode.c_str() );
              drawtext( text, xt, yt+20, fontscale, 0.8,1,0.8 );
              h += 20;
            }
            return h;
      };

      //////////////////////

      for( int i=0; i<4; i++ )
      {
         const DspBlockData* dbd = nullptr;
         switch(i)
         {
            case 0:
              dbd = & layd->_f1Block;
              break;
            case 1:
              dbd = & layd->_f2Block;
              break;
            case 2:
              dbd = & layd->_f3Block;
              break;
            case 3:
              dbd = & layd->_f4Block;
              break;
         }

         int xt = xb+10;
         int yt = ytb+30;
         auto name = dbd->_dspBlock;
         if( name == "" )
             name = "none";
         auto text = formatString("%s BLOCK: %s", dbd->_name.c_str(), name.c_str() );
         drawtext( text, xt, yt, fontscale, 1,1,1 );
         yt += 20;
         yt += PanPadOut( dbd, xt, yt );

         auto b = alg->_block[i];

         if( b )
         {
            bool block_ena = _fblockEnable[i];

            int fidx = b->_baseIndex;

            text = formatString("INP<%d> OUT<%d>", b->_numInputs, b->_numOutputs );
            drawtext( text, xt, yt, fontscale, 1,1,1 );
            yt += 20;

            auto drawfhud = [&b,&xt,&yt](int idx)
            {
                float tot = b->_fval[idx];
                float coa = b->_ctrl[idx]._coarse;
                float fin = b->_ctrl[idx]._fine;
                float s1 = b->_ctrl[idx]._C1();
                float s2 = b->_ctrl[idx]._C2();
                float ko = b->_ctrl[idx]._keyOff;
                float kt = b->_ctrl[idx]._keyTrack;
                float kv = b->_ctrl[idx]._kval;
                float vo = b->_ctrl[idx]._unitVel;
                int ks = b->_ctrl[idx]._kstartNote;
                char paramC = 'A'+idx;
                std::string text;

                if( fabs(tot) > 1 )
                  text = formatString("P%c<%0.1f> _UV<%0.2f>",paramC,tot,vo);
                else
                  text = formatString("P%c<%g> _UV<%0.2f>",paramC,tot,vo);

                drawtext( text, xt, yt, fontscale, 1,1,0 );
                yt += 20;

                drawtext( formatString("   _c<%g> _f<%g>",coa,fin), xt, yt, fontscale, 1,1,0 );
                yt += 20;

                text = ( fabs(s1) > 1 ) 
                     ? formatString("   _s1<%0.1f> _s2<%0.1f>",s1,s2)
                     : formatString("   _s1<%g> _s2<%g>",s1,s2);
                drawtext( text, xt, yt, fontscale, 1,1,0 );
                yt += 20;

                text = formatString("   _ks<%d> _ko<%g> _kt<%g> _kv<%g>", ks,ko,kt,kv);
                drawtext( text, xt, yt, fontscale, 1,1,0 );
                yt += 20;
            };

            if( b->_numParams>0 )
                drawfhud(0);
            if( b->_numParams>1 )
                drawfhud(1);
            if( b->_numParams>2 )
                drawfhud(2);

         }

         yb += yinc;
         ytb += yinc;
      }

}