#include <portmidi.h>
#include <porttime.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <queue>

///////////////////////////////////////////////////////////////////////////////

const int INPUT_BUFFER_SIZE = 100;
const int OUTPUT_BUFFER_SIZE = 0;
void* DRIVER_INFO = nullptr;
#define TIME_PROC ((int32_t (*)(void *)) Pt_Time)
void* TIME_INFO = nullptr;
typedef uint8_t u8;

const u8 MIDI_EOX = 0xf7;

///////////////////////////////////////////////////////////////////////////////

struct MidiInpPort
{
	MidiInpPort(int pid)
	{
	    Pm_OpenInput(&_midi, 
	                 pid,
	                 DRIVER_INFO, 
	                 INPUT_BUFFER_SIZE, 
	                 TIME_PROC, 
	                 TIME_INFO);


	    printf("Midi Input opened.\n");
    	Pm_SetFilter(_midi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

	}
	~MidiInpPort()
	{
	    Pm_Close(_midi);
	}
	int poll()
	{
		int count = 0;
	    while (Pm_Poll(_midi)) {
	    	PmEvent event;
    	    //Pm_Read(_midi, &ev, 1);
    	    _evq.push(event);
    	    count++;
    	    u8 data = 0;

			while (data != MIDI_EOX)
			{
	            int count = Pm_Read(_midi, &event, 1);
	            if (count == 0) {
	                //Sleep(1); /* be nice: give some CPU time to the system */
	                continue; /* continue polling for input */
	            }
	            printf( "read count<%d>\n", count );

	            /* printf("read %lx ", event.message);
	               fflush(stdout); */
	            

	            /* compare 4 bytes of data until you reach an eox */
	            for (int shift = 0; shift < 32 && (data != MIDI_EOX); shift += 8) {
	                data = (event.message >> shift) & 0xFF;
	                printf( "inp: shft<%d> data<%02x>\n", shift, data );
	                //i++;
	            }
	        }

                    //printf("Got message %d: time %ld, %2lx %2lx %2lx\n",
                   //_msgcount,
                   //(long) ev.timestamp,
                   //(long) Pm_MessageStatus(ev.message),
                   //(long) Pm_MessageData1(ev.message),
                   //(long) Pm_MessageData2(ev.message));

            _msgcount++;
    	}
    	return count;
	}

    PmStream* _midi = nullptr;
    int _msgcount = 0;
    std::queue<PmEvent> _evq;
};

///////////////////////////////////////////////////////////////////////////////

struct MidiOutPort
{
	MidiOutPort(int pid)
	{
    /* open output device -- since PortMidi avoids opening a timer
       when latency is zero, we will pass in a NULL timer pointer
       for that case. If PortMidi tries to access the time_proc,
       we will crash, so this test will tell us something. */

	    Pm_OpenOutput(&_midi, 
	                  pid, 
	                  DRIVER_INFO,
	                  OUTPUT_BUFFER_SIZE, 
	                  (_latency == 0 ? NULL : TIME_PROC),
	                  (void*)(_latency == 0 ? NULL : TIME_INFO), 
	                  _latency);		

	    printf("Midi Output opened with %ld ms latency.\n", (long) _latency);
	}
	~MidiOutPort()
	{
	    Pm_Close(_midi);
	}
	void send(std::vector<PmEvent> events )
	{
	    Pm_Write(_midi, events.data(), events.size());
	}
	void CC(int chan, int ccnum, int val)
	{
		PmEvent ev;
		ev.timestamp = 0;
	    ev.message = Pm_Message(0xb0+chan, ccnum, val);
	    Pm_Write(_midi, &ev, 1);
	}
	void noteOn(int chan, int note, int vel)
	{
		PmEvent ev;
		ev.timestamp = 0;
	    ev.message = Pm_Message(0x90+chan, note, vel);
	    Pm_Write(_midi, &ev, 1);
	}
	void noteOff(int chan, int note)
	{
		PmEvent ev;
		ev.timestamp = 0;
	    ev.message = Pm_Message(0x90+chan, note, 0);
	    Pm_Write(_midi, &ev, 1);
	}
	void stdProgramChange(int chan, int pid)
	{
    	assert(pid>=0);
    	assert(pid<=128);
		PmEvent ev;
		ev.timestamp = 0;
	    ev.message = Pm_Message(0xC0+chan, pid, 0);
	    Pm_Write(_midi, &ev, 1);
	}
	void krzProgramChange(int chan, int pid)
	{
		int bank = pid/100;
		int subprg = pid%100;

		printf( "krzprg<%d> -> bank<%d> subp<%d>\n", pid, bank, subprg );
		// select bank
		CC(chan,0,bank);
		stdProgramChange(chan,subprg);
	}
	std::vector<u8> split14(int inp14)
	{
		std::vector<u8> rval;
		rval.push_back((inp14>>7)&0x7f);	
		rval.push_back(inp14&0x7f);	
		return rval;
	}
	std::vector<u8> split21(int inp21)
	{
		std::vector<u8> rval;
		rval.push_back((inp21>>14)&0x7f);	
		rval.push_back((inp21>>7)&0x7f);	
		rval.push_back(inp21&0x7f);	
		return rval;
	}
	void krzObjectDir(int type)
	{
		//f0 07 00 78 04 01 05 00 0a f7

		auto typeb = split14(133);
		auto idnob = split14(10);
		std::vector<u8> DIR;
		DIR.insert(DIR.end(), typeb.begin(), typeb.end());
		DIR.insert(DIR.end(), idnob.begin(), idnob.end());
		krzSysEx(0x04,DIR);
	}
	void krzObjectDump(int type)
	{
		// types 
		// 132 program
		// 133 keymap
		// 134 sndblk

		//DUMP = 00h type(2) idno(2) offs(3) size(3) form(1)

		auto typeb = split14(133);
		auto idnob = split14(10);
		auto offsb = split21(0);
		auto sizeb = split21(16);


		std::vector<u8> DUMP;
		DUMP.insert(DUMP.end(), typeb.begin(), typeb.end());
		DUMP.insert(DUMP.end(), idnob.begin(), idnob.end());
		DUMP.insert(DUMP.end(), offsb.begin(), offsb.end());
		DUMP.insert(DUMP.end(), sizeb.begin(), sizeb.end());
		DUMP.push_back(0); // form nybblepack
		krzSysEx(0x00,DUMP);

	}
	void krzPanel(int button)
	{
		// bid 

		// 0x00 0
		// 0x09 9
		// 0x0a +/-
		// 0x0b cancel
		// 0x0d enter

		// 0x10 cursup
		// 0x11 cursdown
		// 0x12 cursleft
		// 0x13 cursright
		// 0x14 CHAN+
		// 0x15 CHAN-
		// 0x16 +
		// 0x17 -

		// 0x20 edit
		// 0x21 exit
		// 0x22 SOFT-A
		// 0x27 SOFT-F

		// 0x40 program
		// 0x43 master
		// 0x44 midi

		std::vector<u8> PANEL;
		PANEL.push_back(0x09); // button down
		PANEL.push_back(button); // button #
		PANEL.push_back(0x08); // button up
		krzSysEx(0x14,PANEL);
	}
	void krzSysEx(u8 msgtype, std::vector<u8> message)
	{
		message.insert(message.begin(),msgtype); // msgtype
		message.insert(message.begin(),0x78); // pid
		message.insert(message.begin(),0x00); // devid
		message.insert(message.begin(),0x07); // kid
		message.insert(message.begin(),0xf0); // sox
		message.push_back(0xf7); // eox

		printf( "sending bytes [" );
		for( auto b : message )
		{
			printf( "%02x ", int(b) );
		}
		printf( "]\n" );

		Pm_WriteSysEx(_midi, 0, message.data());

		//sox(1) kid(1) dev-id(1) pid(1) msg-type(1) message(n) eox(1)		
	}
	void panic()
	{
		printf( "sending panic...\n");
	    for( int n=0; n<127; n++ )
		{
			PmEvent ev;
			ev.timestamp = 0;
		    ev.message = Pm_Message(0x90, n, 0);
		    Pm_Write(_midi, &ev, 1);
		}		
		printf( "panic sent...\n");
	}
    PmStream* _midi = nullptr;
	int32_t _latency = 0;
};

///////////////////////////////////////////////////////////////////////////////

void iodriver() {
	char line[80];
    int32_t off_time;
    std::vector<PmEvent> events;
    //PmTimestamp timestamp;

    MidiInpPort inpport(1);
    MidiOutPort outport(3);

    /* It is recommended to start timer before PortMidi */
	Pt_Start(1, 0, 0); // timer started w/millisecond accuracy 

    //////////////

    outport.panic();

    if(0)
    {
		std::vector<int> buttons = 
		{	
			0x40,0x41,0x42,0x43,0x44,0x45,0x40,
			0x16, 0x16, 0x16,
		};

		for( auto button : buttons )
		{
			outport.krzPanel(button);
			usleep(1<<20);
		}
	}

	//outport.krzObjectDir(1);

    if(0) while( true )
    {
		int count = inpport.poll();
		printf( "count<%d>\n", count );
		usleep(1<<20);
    }

    /* output note on/off w/latency offset; hold until user prompts */
    printf("starting sequence in 2 secs\n");
    usleep(2<<20);


    //////////////

	outport.krzProgramChange(0,196);
	outport.noteOn(0,60,127);
    usleep(8<<20);
	outport.noteOff(0,60);
    usleep(2<<20);

    //////////////

    events.push_back(PmEvent());
    auto& ev = events[0];

    ev.timestamp = 0;


    //for( int p=50; p<52; p++ )
    if(1)
    {	int p = 32;
	    ev.message = Pm_Message(0xC0, p, 0);
    	outport.send(events);

	    for( int n=36; n<72; n++ )
    	{
    		outport.noteOn(0,n,127);
		    usleep(1<<18);

    		outport.noteOff(0,n);
		    usleep(1<<10);
    	}

    }


    usleep(1<<20);
	
}

///////////////////////////////////////////////////////////////////////////////

void printiolist()
{
    /* list device information */
    auto default_in = Pm_GetDefaultInputDeviceID();
    auto default_out = Pm_GetDefaultOutputDeviceID();
    for (int i = 0; i < Pm_CountDevices(); i++) {
        const char *deflt;
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) {
            deflt = (i == default_in ? "default " : "");
            printf(" (%sinput)", deflt);
        }
        if (info->output) {
            deflt = (i == default_out ? "default " : "");
            printf(" (%soutput)", deflt);
        }
        printf("\n");
    }

}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	printiolist();
	iodriver();

    Pm_Terminate();
	usleep(1<<20);
    return 0;
}
