#include <string>
#include <portaudio.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <sstream>
#include <GLFW/glfw3.h>
#include "drawtext.h"

#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////

//static const int kdefaultprogID = 191;
static const int kdefaultprogID = 165;
static int programID = 0;
static int octave = 4;
static bool dokeymaps = false;
static bool dotestprogram = false;
const programData* curProg = nullptr;

std::map<int,programInst*> playingNotesMap;

///////////////////////////////////////////////////////////////////////////////

s16* getSoundBlock()
{
    static s16* gdata = nullptr;
    if( nullptr == gdata )
    {
        const char* filename = "krzfiles/k2v3internalsamplerom.bin";
        printf( "Loading Soundblock<%s>\n", filename );
        FILE* fin = fopen(filename, "rb");
        gdata = (s16*) malloc(8<<20);
        fread(gdata,8<<20,1,fin);
        fclose(fin);
      }
    return gdata;
}

///////////////////////////////////////////////////////////////////////////////

synth* the_synth = nullptr;

static int patestCallback(	const void *inputBuffer,
              							void *outputBuffer,
                           	unsigned long framesPerBuffer,
                           	const PaStreamCallbackTimeInfo* timeInfo,
                           	PaStreamCallbackFlags statusFlags,
                           	void *userData )
{
    /* Cast data passed through stream to our structure. */
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    the_synth->compute(framesPerBuffer);
    const auto& obuf = the_synth->_obuf;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = obuf._leftBuffer[i];
        *out++ = obuf._rightBuffer[i];
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

PaStream* pa_stream = nullptr;

void startupAudio()
{
    the_synth = new synth(48000.0f);
    auto err = Pa_Initialize();
    assert( err == paNoError );

    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &pa_stream,
                                0,          // no input channels 
                                2,          // stereo output 
                                paFloat32,  // 32 bit floating point output 
                                the_synth->_sampleRate,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                patestCallback, // this is your callback function 
                                nullptr ); //This is a pointer that will be passed to
                                         //         your callback
    
    assert(err==paNoError);

    err = Pa_StartStream( pa_stream );
    assert(err==paNoError);

}

///////////////////////////////////////////////////////////////////////////////

void tearDownAudio()
{
    auto err = Pa_StopStream( pa_stream );
    assert( err == paNoError );
    err = Pa_Terminate();
    assert( err == paNoError );
}

///////////////////////////////////////////////////////////////////////////////

void loadprog()
{
    auto sd = the_synth->_SD;

  if( dotestprogram )
    curProg = sd->getTestProgram(programID);
  else if( dokeymaps )
    curProg = sd->getKmTestProgram(programID);
  else
    curProg = sd->getProgram(programID);

  the_synth->resetFenables();

  assert(curProg);
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool down = ( action == GLFW_PRESS );
    bool up = ( action == GLFW_RELEASE );

    //  return;

    auto sd = the_synth->_SD;



    auto do_midikb = [&]( int key, bool is_down )
    {
        int note = -1;

        switch(key)
        {
            case 'Z':
              note = 0;
              break;
            case 'S':
              note = 1;
              break;
            case 'X':
              note = 2;
              break;
            case 'D':
              note = 3;
              break;
            case 'C':
              note = 4;
              break;
            case 'V':        
              note = 5;
              break;
            case 'G':        
              note = 6;
              break;
            case 'B':        
              note = 7;
              break;
            case 'H':        
              note = 8;
              break;
            case 'N':        
              note = 9;
              break;
            case 'J':        
              note = 10;
              break;
            case 'M':        
              note = 11;
              break;
            case ',':        
              note = 12;
              break;
            case 'L':        
              note = 13;
              break;
            case '.':        
              note = 14;
              break;
            case ';':        
              note = 15;
              break;
            case '/':        
              note = 16;
              break;
            default:
              break;
        }

        if( note>=0 )
        {
            note = ((octave+1)*12) + note;

            if( is_down )
            {
                sd->addEvent( 0.0f ,[=]()
                {   
                    auto it = playingNotesMap.find(note);
                    if( it == playingNotesMap.end() )
                    {
                      auto pi = the_synth->keyOn(note,curProg);
                      assert(pi);
                      playingNotesMap[note] = pi;
                    }
                } );
            }
            else // must be up
            {
                sd->addEvent( 0.0f ,[=]()
                {
                    auto it = playingNotesMap.find(note);
                    if(it!=playingNotesMap.end())
                    {
                      auto pi = it->second;
                      assert(pi);
                      the_synth->keyOff(pi);
                      playingNotesMap.erase(it);
                    }
                } );
            }
        }

    };

    switch( key )
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case ']':
          if( ! down ) break;
          if( (programID+1)<199 )
            programID++;
          else
            programID=1;
          loadprog();
          break;
        case '[':
          if( ! down ) break;
          if( (programID-1)>0 )
            programID--;
          else
            programID=199;
          loadprog();
          break;
        case '\\':
          if( ! down ) break;
          if( programID+10<=199 )
            programID+=10;
          else
          {
            programID+=10;
            programID-=200;
            if( programID==0 )
                programID = 1;
          }
          loadprog();
          break;
        case '-':
          if( ! down ) break;
          if( (octave-1) > 0 )
            octave--;
          break;
        case '=':
          if( ! down ) break;
          if( (octave+1) < 8 )
            octave++;
          break;
        case 'T':
          if( ! down ) break;
          dotestprogram = ! dotestprogram;
          loadprog();
          break;
        case 'K':
          if( ! down ) break;
          dokeymaps = ! dokeymaps;
          loadprog();
          break;
        case 'Z':
        case 'S':
        case 'X':
        case 'D':
        case 'C':
        case 'V':
        case 'G':
        case 'B':
        case 'H':
        case 'N':
        case 'J':
        case 'M':
        case ',':
        case 'L':
        case '.':
        case ';':
        case '/':
          if( down || up )
            do_midikb( key, down );
          break;
        case 258:
        {
          if( ! down ) break;

          int nl = curProg 
                  ? curProg->_layerDatas.size()
                  : 0;

          the_synth->_soloLayer++;

          if( the_synth->_soloLayer >= nl ) 
              the_synth->_soloLayer = -1;


          the_synth->resetFenables();

          printf( "inclayer: %d nl<%d>\n", the_synth->_soloLayer, nl);

          break;

        }
        case 96: // '`'
          if( ! down ) break;
          the_synth->_testtone = ! the_synth->_testtone;
          break;
        case '1':
          if( ! down ) break;
          the_synth->_fblockEnable[0] = ! the_synth->_fblockEnable[0];
          break;
        case '2':
          if( ! down ) break;
          the_synth->_fblockEnable[1] = ! the_synth->_fblockEnable[1];
          break;
        case '3':
          if( ! down ) break;
          the_synth->_fblockEnable[2] = ! the_synth->_fblockEnable[2];
          break;
        case '4':
          if( ! down ) break;
          the_synth->_fblockEnable[3] = ! the_synth->_fblockEnable[3];
          break;
        case '5':
          if( ! down ) break;
          the_synth->_masterGain *= 0.707f;
          break;
        case '6':
          if( ! down ) break;
          the_synth->_masterGain *= 1.0f/0.707f;
          break;
        case '9':
          if( ! down ) break;
          the_synth->_bypassDSP = ! the_synth->_bypassDSP;
          break;
        case ' ':
          if( ! down ) break;
          the_synth->_doModWheel = ! the_synth->_doModWheel;
          break;        
        case 257:
          if( ! down ) break;
          the_synth->_doPressure = ! the_synth->_doPressure;
          break;        
        case 'Q':
          if( ! down ) break;
          the_synth->_hudpage = 0;
          break;
        case 'W':
          if( ! down ) break;
          the_synth->_hudpage = 1;
          break;
        case 'E':
          if( ! down ) break;
          the_synth->_hudpage = 2;
          break;
        case 'I':
          if( ! down ) break;
          the_synth->_ostrack -= 1.5f;
          break;
        case 'O':
          if( ! down ) break;
          the_synth->_ostrack += 1.5f;
          break;
        case '0':
          if( ! down ) break;
          the_synth->_sinerep = ! the_synth->_sinerep;
          break;
        case 'P':
        {  auto l = the_synth->_hudLayer;
           bool inc = key=='P';
           if( up ) break;
           if( l && l->_kmregion )
           {  auto r = l->_kmregion;
              if( r && r->_sample )
              {
                auto s = (sample*) r->_sample;
                s->_pitchAdjust = inc 
                                ? s->_pitchAdjust+5
                                : s->_pitchAdjust-5;

              }
           }
           break;
        }
        default:
          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

static int width = 0;
static int height = 0;

void drawtext( const std::string& str, float x, float y, float scale, float r, float g, float b )
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,width,height,0,0,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();    
    glTranslatef(x,y, 0);
    glScalef(scale,-scale,1);

    glColor4f(r,g,b,1);
    dtx_string(str.c_str());

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void runUI()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
      return;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 600, "KrzTest", NULL, NULL);
    if (!window)
    {
      glfwTerminate();
      return;
    }

    glfwMakeContextCurrent(window);

    auto font = dtx_open_font("/Library/Fonts/Tahoma.ttf", 48);
    assert(font!=nullptr);
    dtx_use_font(font, 48);

    glfwSetKeyCallback(window, key_callback);

    auto sd = the_synth->_SD;

    ///////////////////////////////////////////////////////

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);

        glClearColor(.20,0,.20,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0,0,width,height);
        glScissor(0,0,width,height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,width,0,height,0,1);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        ///////////////////////////////

        bool modwh = the_synth->_doModWheel;
        bool press = the_synth->_doPressure;
        float mgain = the_synth->_masterGain;
        float mgainDB = linear_amp_ratio_to_decibel(mgain);

        auto hudstr = formatString("Octave<%d> Mod<%d> Press<%d> MGain<%ddB>\n", octave, int(modwh), int(press), int(mgainDB) );

        if( nullptr == curProg )
        {
          programID=kdefaultprogID;
          dokeymaps=false;
          loadprog();
        }

        const char* prghead = curProg->_role.c_str();

        hudstr += formatString("%s<%d:%s>\n", prghead,programID, curProg->_name.c_str() );
        drawtext( hudstr, 50,50, 1, 0,1,0 );

        ///////////////////////////////

        int inumlayers = curProg->_layerDatas.size();
        if( inumlayers > 3 )
            drawtext( "(Drum Program)\n", 80,150, .65, 0,1,0 );
        else for( int i=0; i<inumlayers; i++ )
        {
          auto ld = curProg->_layerDatas[i];
          auto km = ld->_keymap;
          if( km )
          {
              hudstr = formatString(" L%d keymap<%d:%s>\n", i+1, km->_kmID, km->_name.c_str() );
              bool sololayer = the_synth->_soloLayer==i;
              float r = sololayer ? 1 : 0;

              drawtext( hudstr, 80,150+(i*29), .65, r,1,0 );
          }
          else
          {

          }
        }


        ///////////////////////////////

        glLoadIdentity();    
        glColor4f(1,1,0,1);
        the_synth->onDrawHud(width,height);

        ///////////////////////////////

        glFinish();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
    getSoundBlock();
    usleep(1<<19);

  	startupAudio();

    runUI();

  	tearDownAudio();

    printf( "Goodbye...\n");
  	return 0;
}

///////////////////////////////////////////////////////////////////////////////

std::string formatString( const char* formatstring, ... )
{
    std::string rval;
    char formatbuffer[512];

    va_list args;
    va_start(args, formatstring);
    //buffer.vformat(formatstring, args);
    vsnprintf( &formatbuffer[0], sizeof(formatbuffer), formatstring, args );
    va_end(args);
    rval = formatbuffer;
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

void SplitString(const std::string& s, char delim, std::vector<std::string>& tokens)
{ std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
        tokens.push_back(item);
}

std::vector<std::string> SplitString(const std::string& instr, char delim)
{ std::vector<std::string> tokens;
    SplitString(instr, delim, tokens);
    return tokens;
}
