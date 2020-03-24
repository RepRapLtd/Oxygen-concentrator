/*
 * RepRap Ltd Open-source Oxygen Concentrator Control Program
 * 
 * https://reprapltd.com
 * 
 * Device: Arduino Uno
 * 
 * Adrian Bowyer
 * 24 March 2020
 * 
 * Licence: GPL
 * 
 * See: https://github.com/RepRapLtd/Oxygen-concentrator
 */

// We need to save the parameters for when we reboot.

#include <EEPROM.h>

// Tag string to see if EEPROM contents are valid

const char* eTag = "uAuAuA";

// Solenoid valve control pins: [0] - left; [1] - right

const int zeoliteIn[2] = {2, 5};
const int zeoliteOut[2] = {3, 6};
const int airVent[2] = {4, 7};

// O2 level sensor pins

const int o2LevelLow = 8;
const int o2LevelHigh = 9;

// Timings (in ms)

#define VOT 100
int valveOpenTime = VOT;

#define VCT 100
int valveCloseTime = VCT;

#define ZIT 1000
int zeoliteInTime = ZIT;

#define ZHT 3000
int zeoliteHoldTime = ZHT;

#define ZOT 500
int zeoliteOutTime = ZOT;

#define PT 1000
int purgeTime = PT;


// Save all the settings to the EEPROM, starting with the validation tag string

void SaveToEeprom()
{
  int i = 0;
  while(eTag[i])
  {
      EEPROM.write(i, eTag[i]);
  }
}

// Load the default values.

void LoadDefaults()
{
  valveOpenTime = VOT;
  valveCloseTime = VCT;
  zeoliteInTime = ZIT;
  zeoliteHoldTime = ZHT;
  zeoliteOutTime = ZOT;
  purgeTime = PT;
  SaveToEeprom();
}





// Check if the EEPROM starts with a valid tag string.  If not, load the default data.
// If it does, load from the EEPROM.

void LoadDataFromEepromOrSetDefaults()
{
  int i = 0;
  while(eTag[i])
  {
    if(EEPROM.read(i) != eTag[i])
    {
      LoadDefaults();
      return;
    }
    i++;
  }
 
}

// Allow the user to change values.
// Save all the values to EEPROM whenever one changes.

void Command()
{
}

// Control O2 generation

void Control()
{
}


void setup() 
{
// Solenoid Valve control pins

  for(int lr = 0; lr < 2; lr++)
  {
    pinMode(zeoliteIn[lr], OUTPUT);
    pinMode(zeoliteOut[lr], OUTPUT);
    pinMode(airVent[lr], OUTPUT);
    digitalWrite(zeoliteIn[lr], LOW);
    digitalWrite(zeoliteOut[lr], LOW);
    digitalWrite(airVent[lr], LOW);
  }

// O2 level sensor pins

  pinMode(o2LevelLow, INPUT_PULLUP);
  pinMode(o2LevelHigh, INPUT_PULLUP); 
 
  LoadDataFromEepromOrSetDefaults();

  

}

void loop()
{
  Command();
  Control();
}
