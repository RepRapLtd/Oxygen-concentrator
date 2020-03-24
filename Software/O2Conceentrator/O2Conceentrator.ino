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

// Timings (in ms) and other variables

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

#define CTF 1
int cyclesToFull = CTF;

int cycleCount = cyclesToFull + 1;
int arm = 0;


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
  cyclesToFull = CTF;
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
  // Are we inactive?
  
  if(cycleCount >= cyclesToFull)
  {
    
    // Yes, inactive; do we need to become active?
    
    if(!digitalRead(o2LevelLow))
    {
      // Yes - resetting the cycle count will automatically make that happen.
      
      cycleCount = 0;
      return;
    }

    // Inactive and not needed. Make sure all valves are closed (not needed 
    // for function, but reduces current consumption).

    for(arm = 0; arm < 2; arm++)
    {
      digitalWrite(zeoliteIn[arm], LOW);
      digitalWrite(zeoliteOut[arm], LOW);
      digitalWrite(airVent[arm], LOW);
    }
    delay(valveCloseTime);
    
    return;
  }

  // We are active; start by shutting all valves

  digitalWrite(zeoliteIn[arm], LOW);
  digitalWrite(zeoliteOut[arm], LOW);
  digitalWrite(airVent[arm], LOW);
  delay(valveCloseTime);

  // Let the compressed air into the chamber

  digitalWrite(zeoliteIn[arm], HIGH);  
  delay(valveOpenTime);

  // Wait for the air to fill the chamber

  delay(zeoliteInTime);

  // Close the input valve

  digitalWrite(zeoliteIn[arm], LOW);  
  delay(valveCloseTime);

  // Wait for the zeolite to adsorb the N2

  delay(zeoliteHoldTime);

  // Let the O2 go to the output

  digitalWrite(zeoliteOut[arm], HIGH);
  delay(valveOpenTime);

  // Wait for it to get there

  delay(zeoliteOutTime);

  // Close the output valve

  digitalWrite(zeoliteOut[arm], LOW);
  delay(valveCloseTime);

  // Open the purge valve and the compressed air to blow through

  digitalWrite(airVent[arm], HIGH);
  digitalWrite(zeoliteIn[arm], HIGH); 
  delay(valveOpenTime);

  // Wait for the purge

  delay(purgeTime);

  // Close the input valve; leave the purge valve open

  digitalWrite(zeoliteIn[arm], LOW);  
  delay(valveCloseTime);

  // That was one cycle
  
  cycleCount++;

  // Swap to the other arm for the next cycle
  
  arm = 1 - arm;
}


void setup() 
{
// Solenoid Valve control pins

  for(arm = 0; arm < 2; arm++)
  {
    pinMode(zeoliteIn[arm], OUTPUT);
    pinMode(zeoliteOut[arm], OUTPUT);
    pinMode(airVent[arm], OUTPUT);
    digitalWrite(zeoliteIn[arm], LOW);
    digitalWrite(zeoliteOut[arm], LOW);
    digitalWrite(airVent[arm], LOW);
  }

  arm = 0;

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
