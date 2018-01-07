/*
 *  Seeeduino V4.2 
 *  It is compatible with Arduino UNO R3.
 *  
 *  Additional Library Timer
*/
//#include <Metro.h> //Include Metro library

//#include "Timer.h"
#include <PololuLedStrip.h>

/*
  Const definition 
*/
// LED pin
#define LED 13

// Mode Key input pin 
#define MODEKEY 2

// Maximum PWM duty (when maximum darkening)
#define MAXCONT 127

// transition time of LCD contrast 50ms unit
#define TRANSITIONTIME 10 // 50 * 10 = 500(ms)

// Holding time of dark and transparent status - 50ms unit
#define HOLDINGTIME 40 // 50 * 40 = 2000(ms)

// Status Count
#define STATUSCNT 28

// WS2812B Data input Pin
#define WS2812B 12

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 12


// definition of PWM pins
#define ShutterPin1 11
#define ShutterPin2 10
#define ShutterPin3 9
#define ShutterPin4 6
#define ShutterPin5 5
#define ShutterPin6 3

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<LED_COUNT> ledStrip;

// RGB color structure
rgb_color colors[LED_COUNT];

// Condition transition time interval 50ms unit
// Now it has total 28 state transition points.
const unsigned short ntimeCnt[STATUSCNT] = {
        TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME,
        TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME,
        TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME,
        TRANSITIONTIME, HOLDINGTIME, TRANSITIONTIME, HOLDINGTIME
                                            };

// definition of actions in LCD moduule
#define TRANSPARENING 0 // transition time of darkening to transparent 
#define TRANSPARENT   1 // transparent status
#define DARKENNING    2 // transition time of transparent to darkening
#define DARKNED       3 // darked status
// it would be able to added.

// Scan Mode Const 
#define OPAQUEMODE 0 // All LCDs dark/opaque (No LED).
#define LARSONNOLED 1 // LCD Larson scanner (No LED)
#define SINGLERAIN 2 // LCD Larson scanner with Rainbow LED active behind transparent LCD
#define REDLCDMODE 3 // LCD Larson scanner with Red LED active behind transparent LCD
#define ALLRAINBOW 4 // All LCDs Transparent with all Rainbow LED
//

// Status Count
#define ModuleCnt 6
// Action group of 6 LCD modules if you needed to 
struct Actions
{
  unsigned short ModuleAction[ModuleCnt];
};

// Definition of Action of each module at any time
struct Actions nModuleAction[STATUSCNT] = {
  {TRANSPARENING, DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED}, //0
  {TRANSPARENT,   DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED}, //1
  {DARKENNING,    TRANSPARENING,  DARKNED,        DARKNED,        DARKNED,        DARKNED}, //2
  {DARKNED,       TRANSPARENT,    DARKNED,        DARKNED,        DARKNED,        DARKNED}, //3
  {DARKNED,       DARKENNING,     TRANSPARENING,  DARKNED,        DARKNED,        DARKNED}, //4
  {DARKNED,       DARKNED,        TRANSPARENT,    DARKNED,        DARKNED,        DARKNED}, //5
  {DARKNED,       DARKNED,        DARKENNING,     TRANSPARENING,  DARKNED,        DARKNED}, //6
  {DARKNED,       DARKNED,        DARKNED,        TRANSPARENT,    DARKNED,        DARKNED}, //7
  {DARKNED,       DARKNED,        DARKNED,        DARKENNING,     TRANSPARENING,  DARKNED}, //8
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        TRANSPARENT,    DARKNED}, //9
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKENNING,     TRANSPARENING}, //10 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        TRANSPARENT}, //11 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKENNING}, //12 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED}, //13 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        TRANSPARENING}, //14 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        TRANSPARENT}, //15 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        TRANSPARENING,  DARKENNING}, //16 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        TRANSPARENT,    DARKNED}, //17 
  {DARKNED,       DARKNED,        DARKNED,        TRANSPARENING,  DARKENNING,     DARKNED}, //18 
  {DARKNED,       DARKNED,        DARKNED,        TRANSPARENT,    DARKNED,        DARKNED}, //19 
  {DARKNED,       DARKNED,        TRANSPARENING,  DARKENNING,     DARKNED,        DARKNED}, //20 
  {DARKNED,       DARKNED,        TRANSPARENT,    DARKNED,        DARKNED,        DARKNED}, //21 
  {DARKNED,       TRANSPARENING,  DARKENNING,     DARKNED,        DARKNED,        DARKNED}, //22 
  {DARKNED,       TRANSPARENT,    DARKNED,        DARKNED,        DARKNED,        DARKNED}, //23 
  {TRANSPARENING, DARKENNING,     DARKNED,        DARKNED,        DARKNED,        DARKNED}, //24 
  {TRANSPARENT,   DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED}, //25 
  {DARKENNING,    DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED}, //26 
  {DARKNED,       DARKNED,        DARKNED,        DARKNED,        DARKNED,        DARKNED} //27 
};


// Contrast value of each LCD at a time
static unsigned short nShutterCont[6];

// Device status structure variable                                            
struct
{
  unsigned short nCurrPos;
  unsigned short nMode;
}Larsonlogic;

/*
 * Function definition
*/ 
// LCD scanning process function
void LCDScanTask();

// Initialize varibale and status function
void InitVars();

//
void WS2812_SCAN();

// function that it converts HSV color sysyetm to RGB color system.
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v);

/*
    In order to prevent the shake of Key input signal, I use time delay function block.
    It is called that TON FBD in IEC61131-5 standard.

    Only IN signal is maintained for specified time, Q output become 1.
    Otherwise Q is 0.
*/
typedef struct timeronblock TON;
struct timeronblock
{
  unsigned IN: 1; // IN option
  unsigned Q: 1; // Output
  unsigned PT: 10; // Set point
  unsigned ET: 10; // Elapsed time
};

/*
 *  
 *  In order to detect rising edge of input signal, I use Rising trigger function block.
 *  It is called that R_trig FBD in IEC61131-5.
 *  
 *  function: I have used it in order to detect key pressed signal so that I noticed to program that mode is changed.
*/
typedef struct RisingTrg RTtrg;
struct RisingTrg
{
  unsigned IN : 1;
  unsigned PRE : 1;
  unsigned Q : 1;
};
RTtrg ModeRTrg;

// FBD processing routine
void RTrgFBD(RTtrg *pTrg)
{
    pTrg->Q = 0;
    if(pTrg->IN != pTrg->PRE)
    {
        pTrg->PRE = pTrg->IN;
        if(pTrg->IN)
          pTrg->Q = 1;    
    }
}

TON KeyTON;
// FBD process function
void TONFBD(TON *pTP)
{
  if(pTP->IN)
  {
    if(pTP->ET < pTP->PT)
      pTP->ET++;  
    else
      pTP->Q = 1;  
  }
  else
      pTP->Q = 0;
}

// Timer object
//Timer mytimer;
//Metro MetroTimer = Metro(100); 

// Setup routine
void setup() {

  // Set LED Pin on Seeeduino
  pinMode(LED, OUTPUT);

  // Set Mode button( push button input)
  pinMode(MODEKEY, INPUT);

  // Set PWM Output
  pinMode(ShutterPin1, OUTPUT);
  pinMode(ShutterPin2, OUTPUT);
  pinMode(ShutterPin3, OUTPUT);
  pinMode(ShutterPin4, OUTPUT);
  pinMode(ShutterPin5, OUTPUT);  
  pinMode(ShutterPin6, OUTPUT); 

  // 500ms cycle LED flusing - it means that program running
  //mytimer.oscillate(LED, 500, LOW);

  // Set cycle running routine per 50 ms 
  //mytimer.every(50, LCDScanTask); // LCD Scan task routine 
  // Now it is called per 50ms, if you need, it could be changed.

  // for testing 
  Serial.begin(9600);  
  Serial.println("Work started!");

  //InitVars(); // Local var initialize

}



void loop() {
    // put your main code here, to run repeatedly:

    //mytimer.update();
    //if (MetroTimer.check() == 1) { // check if the metro has passed its interval .
    //  MetroTimer.interval(100);
      //LCDScanTask();
    //}
    //WS2812_SCAN();
    delay(10);
}



// Local variable init function
void InitVars()
{
    unsigned short index = 0;
    // Set all of LCD contrasts set Maximum contrast - Maximum PWM dutys etting
    for(index = 0; index < 6; index++)
      nShutterCont[index] = MAXCONT;  

    // 
    KeyTON.IN = 0;
    KeyTON.Q = 0;
    KeyTON.PT = 5; // Holding time is set to 5 * 50 = 250ms.
    KeyTON.ET = 0;

    // Mode init
    Larsonlogic.nMode = OPAQUEMODE;

    // 
    ModeRTrg.IN = 0;
    ModeRTrg.PRE = 0;
}

// 
void WS2812_SCAN()
{
    unsigned short index = 0;
    uint16_t time = millis() >> 2;

    switch(Larsonlogic.nMode)
    {
        case OPAQUEMODE:
        case LARSONNOLED:
        {
            for(uint16_t index = 0; index < LED_COUNT; index++)
            {
                colors[index] = hsvToRgb(0, 0, 0);
            }
               
        }
        break;  

        case SINGLERAIN:
        {
            for(uint16_t index = 0; index < ModuleCnt; index++)
            {
                if(nModuleAction[Larsonlogic.nCurrPos].ModuleAction[index] == TRANSPARENT)
                {
                    byte x = (time >> 2) - (0 << 2);
                    colors[index * 2] = hsvToRgb((uint32_t)x * 359 / 256, 255, 255);
                    x = (time >> 2) - (1 << 6);
                    colors[index * 2 + 1] = hsvToRgb((uint32_t)x * 359 / 256, 255, 255);
                }
                else
                {
                    colors[index * 2] = hsvToRgb(0, 0, 0);
                    colors[index * 2 + 1] = hsvToRgb(0, 0, 0);
                }
            }
        }
        break;  

        case REDLCDMODE:
        {
            for(uint16_t index = 0; index < ModuleCnt; index++)
            {
                if(nModuleAction[Larsonlogic.nCurrPos].ModuleAction[index] == TRANSPARENT)
                {
                    byte x = (time >> 2) - (0 << 2);
                    colors[index * 2] = hsvToRgb(0, x, 255); // only S value is changed.
                    x = (time >> 2) - (1 << 6);
                    //byte x = (time >> 2) - (index << 2);
                    colors[index * 2 + 1] = hsvToRgb(0, x, 255); // only S value is changed.
                }
                else
                {
                    colors[index * 2] = hsvToRgb(0, 0, 0);
                    colors[index * 2 + 1] = hsvToRgb(0, 0, 0);
                }
            }
        }
        break; 
        
        case ALLRAINBOW:
        {
            for(uint16_t index = 0; index < LED_COUNT; index++)
            {
              /*
               * setting of scan speed and color spectrum width.
              */
              // time >> 2 : speed setting - the bigger, the more slow, and the smaller, the faster.
              // index * 21 : color spectum width - the smaller, the spectrum more narrow, and the bigger, the wider.
              byte x = (time >> 2) - (index * 21); 
              colors[index] = hsvToRgb((uint32_t)x * 359 / 256, 255, 255);
            }
               
        }
        break;  
    }
    
    
    // Write the colors to the LED strip.
    ledStrip.write(colors, LED_COUNT);    
}

// 
void LCDScanTask()
{ // periodically called by timer
  
    static unsigned short nIntCnt = 0;
    unsigned short index = 0;
    unsigned int ntmp = 0;

    KeyTON.IN = (digitalRead(MODEKEY) == 0);
    TONFBD(&KeyTON);
    ModeRTrg.IN = KeyTON.Q;
    RTrgFBD(&ModeRTrg);
    if(ModeRTrg.Q)
    {
        switch(Larsonlogic.nMode)
        {
            case OPAQUEMODE:
            {
              Larsonlogic.nMode = LARSONNOLED;
              Serial.println("In LARSONNOLED.");
            }
            break;
            
            case LARSONNOLED:
            {
              Larsonlogic.nMode = SINGLERAIN;
              Serial.println("In SINGLERAIN.");
            }
            break;

            case SINGLERAIN:
            {
              Larsonlogic.nMode = REDLCDMODE;
              Serial.println("In REDLCDMODE.");
            }
            break;

            case REDLCDMODE:
            {
              Larsonlogic.nMode = ALLRAINBOW;
              Serial.println("In ALLRAINBOW.");
            }
            break;

            case ALLRAINBOW:
            {
              Larsonlogic.nMode = OPAQUEMODE;
              Serial.println("In OPAQUEMODE.");
            }
            break;
        }
    }

    switch(Larsonlogic.nMode)
    {
        case OPAQUEMODE:
        {
            Larsonlogic.nCurrPos = 0; // Set curr state variable to 0.
            nIntCnt = 0;
            // Set maximum contrast.
            for(index = 0; index < ModuleCnt; index++)
                nShutterCont[index] = MAXCONT;    
        }
        break;

        case ALLRAINBOW:
        {
            Larsonlogic.nCurrPos = 0; // Set curr state variable to 0.
    
            // Set minium contrast.
            for(index = 0; index < ModuleCnt; index++)
                nShutterCont[index] = 0;    
        }
        break;

        default:
        {
            // timer count
            nIntCnt++;
            if(nIntCnt >= ntimeCnt[Larsonlogic.nCurrPos])
            {
                //Serial.println(Larsonlogic.nCurrPos);
                nIntCnt = 0;
        
                Larsonlogic.nCurrPos++;
                if(Larsonlogic.nCurrPos >= STATUSCNT)
                {
                    Larsonlogic.nCurrPos = 0;
                }
            }
    
            // It take appropriate PWM duty for each module according to the current state.
            for(index = 0; index < ModuleCnt; index++)
            {
                //
                switch(nModuleAction[Larsonlogic.nCurrPos].ModuleAction[index])
                {
                    case TRANSPARENING:
                    {// It should be gradually transparented in accordance with the elapsed time.
                        ntmp = (ntimeCnt[Larsonlogic.nCurrPos] - nIntCnt) * MAXCONT;
                        nShutterCont[index] = ntmp / ntimeCnt[Larsonlogic.nCurrPos];    
                    }
                    break;                
            
                    case TRANSPARENT:
                    { // completely transparent
                        nShutterCont[index] = 0;    
                    }
                    break;                
            
                    case DARKENNING:
                    {// It should be gradually darkened in accordance with the elapsed time.
                        ntmp = nIntCnt * MAXCONT;
                        nShutterCont[index] = ntmp / ntimeCnt[Larsonlogic.nCurrPos];    
                    }
                    break;                
            
                    case DARKNED:
                    {// completely darkened
                        nShutterCont[index] = MAXCONT;    
                    }
                    break;                
                }
            
            }        
        }
        break;
    }
    
    // Set PWM duty as calculated value. 
    analogWrite(ShutterPin1, nShutterCont[0]);
    analogWrite(ShutterPin2, nShutterCont[1]);
    analogWrite(ShutterPin3, nShutterCont[2]);
    analogWrite(ShutterPin4, nShutterCont[3]);
    analogWrite(ShutterPin5, nShutterCont[4]);
    analogWrite(ShutterPin6, nShutterCont[5]);
}

// Converts a color from HSV to RGB.
// h is hue, as a number between 0 and 360.
// s is the saturation, as a number between 0 and 255.
// v is the value, as a number between 0 and 255.
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return (rgb_color){r, g, b};
}

/*
 * Now unused

typedef struct RisingTrg RTtrg;
struct RisingTrg
{
  unsigned IN : 1;
  unsigned PRE : 1;
  unsigned Q : 1;
};

typedef struct FallingTrg FTtrg;
struct FallingTrg
{
  unsigned IN : 1;
  unsigned PRE : 1;
  unsigned Q : 1;
};

RTtrg ModeRTrg;
FTtrg ModeFTrg;

void RTrgFBD(RTtrg *pTrg)
{
    pTrg->Q = 0;
    if(pTrg->IN != pTrg->PRE)
    {
        pTrg->PRE = pTrg->IN;
        if(pTrg->IN)
          pTrg->Q = 1;    
    }
}

void FTrgFBD(FTtrg *pTrg)
{
    pTrg->Q = 0;
    if(pTrg->IN != pTrg->PRE)
    {
        pTrg->PRE = pTrg->IN;
        if(pTrg->IN == 0)
          pTrg->Q = 1;    
    }
}
*/
