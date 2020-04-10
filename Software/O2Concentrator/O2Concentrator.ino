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
 * 
 * Version 2
 * 9 April 2020
 * 
 */

#include "O2Concentrator.h"

// The two arms of the machine

ZeolitePath* left = new ZeolitePath(2, 3, 4, 5, "Left");
ZeolitePath* right = new ZeolitePath(6, 7, 8, 9, "Right");

// Print debugging information when true.

const bool debug = true;

// Tag string to see if EEPROM contents are valid

const char* eTag = "uAuAuA";

// O2 level sensor pins

const int o2LevelLow = 11;
const int o2LevelHigh = 12;

// Timings (in ms) and other variables

#define FEED_T 30
long o2FeedTime = FEED_T;

#define PURGE_T 30
long purgingTime = PURGE_T;

#define SHUT_T 1000;
long shuttingDownTime = SHUT_T;

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
  
  ptr = EepromWriteLong(ptr, o2FeedTime);
  ptr = EepromWriteLong(ptr, purgingTime);
  ptr = EepromWriteLong(ptr, shuttingDownTime);
}

// Load the default values.

void LoadDefaults()
{
  if(debug)
    Serial.println("Loading default variables.");
    
  o2FeedTime = FEED_T;
  purgingTime = PURGE_T;
  shuttingDownTime = SHUT_T;
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
  
  ptr = EepromReadLong(ptr, o2FeedTime);
  ptr = EepromReadLong(ptr, purgingTime);
  ptr = EepromReadLong(ptr, shuttingDownTime);
}


void Help()
{
  Serial.println("\nConcentrator variables");
  Serial.println(" All times in milliseconds. Current values in brackets.");
  Serial.print(" Oxygen feed time (");  
  Serial.print(o2FeedTime);
  Serial.print(") - o\n Purging time (");
  Serial.print(purgingTime);
  Serial.print(") - p\n Shutting down delay (");
  Serial.print(shuttingDownTime);
  Serial.println(") - s\n Load default values - d");
  Serial.println(" Print this list - ?\n");
}

long ReadInteger()
{
  return (long)Serial.parseInt(); 
}


// True when the machine needs to produce O2.

bool O2Demanded()
{
  return !digitalRead(o2LevelLow);
}

// Allow the user to change values.
// Save all the values to EEPROM whenever one changes.

void Command()
{

  if(Serial.available() <= 0)
    return;

  bool save = true;
    
  int c = Serial.read();
  switch(c)
  {
    case 'o':
      o2FeedTime = ReadInteger();
      break;

    case 'p':
      purgingTime = ReadInteger();
      break;
      
    case 's':
      shuttingDownTime = ReadInteger();
      break;

    case 'd':
      LoadDefaults();
      save = false;
      break;
      
    case '?':
    default:
      Help();
      save = false;
  }

  if(save)
    SaveToEeprom();
}

// Control O2 generation

void Control()
{
  // Do we need to do anything?

  if(!O2Demanded())
    return;

  // Yes; if both sides are inactive turn one on.
  // (If at least one is active the machine is already working.)

  if(left->Inactive() && right->Inactive())
    left->StartFeed();
}


void setup() 
{

  // O2 level sensor pins

  pinMode(o2LevelLow, INPUT_PULLUP);
  pinMode(o2LevelHigh, INPUT_PULLUP);

  // Tell each side about the other
  
  left->SetOtherPath(right);
  right->SetOtherPath(left);

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
