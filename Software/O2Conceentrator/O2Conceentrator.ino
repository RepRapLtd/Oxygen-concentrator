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

#define BAUD 9600

const bool debug = true;

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
long valveOpenTime = VOT;

#define VCT 100
long valveCloseTime = VCT;

#define ZIT 1000
long zeoliteInTime = ZIT;

#define ZHT 3000
long zeoliteHoldTime = ZHT;

#define ZOT 500
long zeoliteOutTime = ZOT;

#define PT 1000
long purgeTime = PT;

#define CTF 1
long cyclesToFull = CTF;

long cycleCount = cyclesToFull + 1;
int arm = 0;

// Read and write longs into EEPROM, 
// returning an opdated pointer in each case.

int EepromWriteLong(int ptr, long v)
{
  unsigned char c;
  for(int b = 0; b < 4; b++)
  {
    c = v & 0xff;
    EEPROM.write(ptr, c);
    ptr++;
    v = v >> 8;
  }
  return ptr;
}

int EepromReadLong(int ptr, long& v)
{
  v = 0;
  long r = 0;
  for(int b = 0; b < 4; b++)
  {
    r = EEPROM.read(ptr);
    r = r << 8*b;
    v = v | r;
    ptr++;
  }
  return ptr;
}

// Save all the settings to the EEPROM, starting with the validation tag string

void SaveToEeprom()
{
  if(debug)
    Serial.println("Saving variables to EEPROM.");
    
  int ptr = 0;
  while(eTag[ptr])
  {
      EEPROM.write(ptr, eTag[ptr]);
      ptr++;
  }
  
  ptr = EepromWriteLong(ptr, valveOpenTime);
  ptr = EepromWriteLong(ptr, valveCloseTime);
  ptr = EepromWriteLong(ptr, zeoliteInTime);
  ptr = EepromWriteLong(ptr, zeoliteHoldTime);
  ptr = EepromWriteLong(ptr, zeoliteOutTime);
  ptr = EepromWriteLong(ptr, purgeTime);
  ptr = EepromWriteLong(ptr, cyclesToFull);
}

// Load the default values.

void LoadDefaults()
{
  if(debug)
    Serial.println("Loading default variables.");
    
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
  int ptr = 0;
  while(eTag[ptr])
  {
    if(EEPROM.read(ptr) != eTag[ptr])
    {
      LoadDefaults();
      return;
    }
    ptr++;
  }

  if(debug)
    Serial.println("Loading variables from EEPROM.");
  
  ptr = EepromReadLong(ptr, valveOpenTime);
  ptr = EepromReadLong(ptr, valveCloseTime);
  ptr = EepromReadLong(ptr, zeoliteInTime);
  ptr = EepromReadLong(ptr, zeoliteHoldTime);
  ptr = EepromReadLong(ptr, zeoliteOutTime);
  ptr = EepromReadLong(ptr, purgeTime);
  ptr = EepromReadLong(ptr, cyclesToFull); 
}


void Help()
{
  Serial.println("\nConcentrator variables");
  Serial.println(" All times in milliseconds. Current values in brackets.");
  Serial.print(" Valve open time (");  
  Serial.print(valveOpenTime);
  Serial.print(") - o\n Valve close time (");
  Serial.print(valveCloseTime);
  Serial.print(") - c\n Zeolite in time (");
  Serial.print(zeoliteInTime);
  Serial.print(") - i\n Zeolite hold time (");
  Serial.print(zeoliteHoldTime);
  Serial.print(") - h\n Zeolite exit time (");
  Serial.print(zeoliteOutTime);
  Serial.print(") - e\n Purge time (");
  Serial.print(purgeTime);
  Serial.print(") - p\n Cycles to full (");
  Serial.print(cyclesToFull);
  Serial.println(") - y");
  Serial.println(" Load default values - d");
  Serial.println(" Print this list - ?\n");
}

long ReadInteger()
{
  return (long)Serial.parseInt(); 
}

// Allow the user to change values.
// Save all the values to EEPROM whenever one changes.

void Command()
{

  if(Serial.available() <= 0)
    return; 
    
  int c = Serial.read();
  switch(c)
  {
    case 'o':
      valveOpenTime = ReadInteger();
      break;

    case 'c':
      valveCloseTime = ReadInteger();
      break;
      
    case 'i':
      zeoliteInTime = ReadInteger();
      break;
      
    case 'h':
      zeoliteHoldTime = ReadInteger();
      break;
      
    case 'e':
      zeoliteOutTime = ReadInteger();
      break;
      
    case 'p':
      purgeTime = ReadInteger();
      break;
      
    case 'y':
      cyclesToFull = ReadInteger();
      break;

    case 'd':
      LoadDefaults();
      break;
      
    case '?':
    default:
      Help();
  }
  
  SaveToEeprom();
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

    for(int a = 0; a < 2; a++)
    {
      digitalWrite(zeoliteIn[a], LOW);
      digitalWrite(zeoliteOut[a], LOW);
      digitalWrite(airVent[a], LOW);
    }
    delay(valveCloseTime);
    
    return;
  }

  if(debug)
  {
    Serial.print("Concentrator is active. Shutting valves on ");
    Serial.print(arm);
    Serial.println(" side.");
  }

  // We are active; start by shutting all valves

  digitalWrite(zeoliteIn[arm], LOW);
  digitalWrite(zeoliteOut[arm], LOW);
  digitalWrite(airVent[arm], LOW);
  delay(valveCloseTime);

  // Let the compressed air into the chamber

  if(debug)
    Serial.println("Air in.");

  digitalWrite(zeoliteIn[arm], HIGH);  
  delay(valveOpenTime);

  // Wait for the air to fill the chamber

  delay(zeoliteInTime);

  // Close the input valve

  digitalWrite(zeoliteIn[arm], LOW);  
  delay(valveCloseTime);

  // Wait for the zeolite to adsorb the N2
  
  if(debug)
    Serial.println("Adsorbing N2.");
    
  delay(zeoliteHoldTime);

  // Let the O2 go to the output

  if(debug)
    Serial.println("Sending O2 to the output.");
    
  digitalWrite(zeoliteOut[arm], HIGH);
  delay(valveOpenTime);

  // Wait for it to get there

  delay(zeoliteOutTime);

  // Close the output valve

  digitalWrite(zeoliteOut[arm], LOW);
  delay(valveCloseTime);

  // Open the purge valve and the compressed air to blow through

  if(debug)
    Serial.println("Purge N2.");

  digitalWrite(airVent[arm], HIGH);
  digitalWrite(zeoliteIn[arm], HIGH); 
  delay(valveOpenTime);

  // Wait for the purge

  delay(purgeTime);

  // Close the input valve; leave the purge valve open

  digitalWrite(zeoliteIn[arm], LOW);  
  delay(valveCloseTime);

  // That was one cycle

  if(debug)
    Serial.println("Cycle complete.\n");
  
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

  Serial.begin(BAUD);

  Serial.println("RepRap Ltd Oxygen Concentrator Starting");
 
  LoadDataFromEepromOrSetDefaults();

  Help();

}

void loop()
{
  Command();
  Control();
}
