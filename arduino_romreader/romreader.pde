
const int poOEL = 23; // OUTP ENABLE (/G) (input)

const int poA0 = 24;    // 24-31 : A0-A7
const int poA8 = 32;    // 32-39 : A8-A15
const int poA16 = 40;   // 40-43 : A16-A19

const int piD0 = 44;    // 44-51 : D0-D7

const int piD8 = 2;    // 44-51 : D0-D7

const int kmicrodelay = 2;
const int kmicrodelayS = 1;

const int poCELA = 13; // CHIP ENABLE (/E) (input)
const int poCELB = 12; // CHIP ENABLE (/E) (input)
const int poCELC = 11; // CHIP ENABLE (/E) (input)
const int poCELD = 10; // CHIP ENABLE (/E) (input)

long Address = 0L;

uint8_t MembufferL[16];
uint8_t MembufferH[16];
////////////////////////////////////////////////////////////

void setup()
{                

  pinMode(poCELA, OUTPUT);     
  pinMode(poCELB, OUTPUT);     
  pinMode(poCELC, OUTPUT);     
  pinMode(poCELD, OUTPUT);     
  
  pinMode(poOEL, OUTPUT);     

  for( int i=0; i<8; i++ ) pinMode(poA0+i, OUTPUT);     
  for( int i=0; i<8; i++ ) pinMode(poA8+i, OUTPUT);     
  for( int i=0; i<4; i++ ) pinMode(poA16+i, OUTPUT);     
  for( int i=0; i<8; i++ ) pinMode(piD0+i, INPUT);     
  //for( int i=0; i<8; i++ ) pinMode(piD8+i, INPUT);     

  Serial.begin(115200);

  delayMicroseconds(1<<10);              // wait for a second

  digitalWrite(poCELA, HIGH);
  digitalWrite(poCELB, HIGH);
  digitalWrite(poCELC, HIGH);
  digitalWrite(poCELD, HIGH);
  digitalWrite(poOEL, HIGH);
}

////////////////////////////////////////////////////////////

void PrintAddress( long ia )
{
   Serial.print("ad<");
   long iaL = ia&0xffffL;
   long iaH = (ia&0xffff0000L)>>16L;
   Serial.print(iaH,HEX);
   Serial.print(".");
   Serial.print(iaL,HEX);
   Serial.print(">");
}

////////////////////////////////////////////////////////////

void WriteAddress( long iaddr )
{
  long a0007 = (iaddr&0x000ff);
  long a0815 = (iaddr&0x0ff00)>>8;
  long a1619 = (iaddr&0xf0000)>>16;

  for( int i=0; i<8; i++ )
  { int pin = poA0+i;
    int istate = (a0007&(1<<i))?HIGH:LOW;
    digitalWrite(pin,istate);
  }
  for( int i=0; i<8; i++ )
  { int pin = poA8+i;
    int istate = (a0815&(1<<i))?HIGH:LOW;
    digitalWrite(pin,istate);
  }
  for( int i=0; i<4; i++ )
  { int pin = poA16+i;
    int istate = (a1619&(1<<i))?HIGH:LOW;
    digitalWrite(pin,istate);
  }
}
//////////////////////////////////////////
int gicurchip = 0;
//////////////////////////////////////////
void ChipSelSig( int ichip, int val )
{
    switch(ichip)
    {
        case 0:
          digitalWrite(poCELA, val);
          break;
        case 1:
          digitalWrite(poCELB, val);
          break;
        case 2:
          digitalWrite(poCELC, val);
          break;
        case 3:
          digitalWrite(poCELD, val);
          break;
        default:
          break;
    }
    delayMicroseconds(1);              // wait for a second
}
//////////////////////////////////////////
void ChipSelLow()
{
    ChipSelSig(gicurchip, LOW);
}
void ChipSelHigh()
{
    ChipSelSig(gicurchip, HIGH);
}
//////////////////////////////////////////
void PreRead()
{
  ChipSelLow();
  digitalWrite(poOEL, LOW);
  delayMicroseconds(1);              // wait for a second
}
void PostRead()
{
  digitalWrite(poOEL, HIGH);
  delayMicroseconds(1);              // wait for a second
  ChipSelHigh();
}
//////////////////////////////////////////
uint8_t ReadByteL()
{
    uint8_t iret = 0;
    iret |= digitalRead(piD0+0);  
    iret |= digitalRead(piD0+1)<<1;  
    iret |= digitalRead(piD0+2)<<2;  
    iret |= digitalRead(piD0+3)<<3;  
    iret |= digitalRead(piD0+4)<<4;  
    iret |= digitalRead(piD0+5)<<5;  
    iret |= digitalRead(piD0+6)<<6;  
    iret |= digitalRead(piD0+7)<<7;  
    return iret;
}
//////////////////////////////////////////
uint8_t ReadByteH()
{
    uint8_t iret = 0;
    iret |= digitalRead(piD8+0);  
    iret |= digitalRead(piD8+1)<<1;  
    iret |= digitalRead(piD8+2)<<2;  
    iret |= digitalRead(piD8+3)<<3;  
    iret |= digitalRead(piD8+4)<<4;  
    iret |= digitalRead(piD8+5)<<5;  
    iret |= digitalRead(piD8+6)<<6;  
    iret |= digitalRead(piD8+7)<<7;  
    return iret;
}
//////////////////////////////////////////
void PrintLine(int ichip)
{
     Serial.print("chip<"); 
     Serial.print(ichip,HEX); 
     Serial.print("L>"); 
     Serial.print(" : "); 
     PrintAddress(Address&0x000ffff0);
     for( int i=0; i<16; i++ )
     {
        uint8_t byt_e = MembufferL[i];
        Serial.print(byt_e,HEX); 
        Serial.print("."); 
     }
    Serial.print("\n");

   /*Serial.print("chip<"); 
   Serial.print(ichip,HEX); 
   Serial.print("H>"); 
   Serial.print(" : "); 
   PrintAddress(Address&0x000ffff0);
   for( int i=0; i<16; i++ )
   {
      uint8_t byt_e = MembufferH[i];
      Serial.print(byt_e,HEX); 
      Serial.print("."); 
   }
   Serial.print("\n");*/
}
//////////////////////////////////////////
int iloop = 0;
int iloopaddr = 0x300;
void loop()
{    
    ////////////////////////////////////
    int ichip = (Address>>20)&3;
    int icol = Address&0xf;
    ////////////////////////////////////
    // write out address
    WriteAddress( Address );   
    ////////////////////////////////////
    // read data
    PreRead();
    int byt_eL = ReadByteL();
    //int byt_eH = ReadByteH();
    PostRead();
    MembufferL[icol] = byt_eL;
    //MembufferH[icol] = byt_eH;
    ////////////////////////////////////
    gicurchip = ichip;
    ////////////////////////////////////
    // wait for full line
    if(icol==15)
    {
      PrintLine(ichip);
    }
    ////////////////////////////////////
    Address++;
    
    /*if( Address >= (iloopaddr+16) )
    {  Address = iloopaddr;
      
      iloop++;
      if( iloop > 256 )
        {  iloop = 0;
          iloopaddr += 16;
        }
    }
    
    if( iloopaddr==0x320 )
      iloopaddr = 0x300;
     */
 }
//////////////////////////////////////////
//////////////////////////////////////////

