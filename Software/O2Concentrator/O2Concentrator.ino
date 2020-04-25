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

// The two arms of the machine; output pins and names

const int lPins[numberOfValves] = { 2, 3, 4, 5 };
const int rPins[numberOfValves] = { 6, 7, 8, 9 };
ZeolitePath* left = new ZeolitePath(lPins, "Left");
ZeolitePath* right = new ZeolitePath(rPins, "Right");

// The default sequence

const int defaultSequence[sequenceSteps] = {};
const long defaultTimes[sequenceSteps] = {};

// Print debugging information when true.

bool debug = true;

// Which side to start up

bool leftStart = true;

// Tag string to see if EEPROM contents are valid

const char* eTag = "uAuAuA";

// O2 level sensor pins

const int o2LevelLow = 11;
const int o2LevelHigh = 12;

// Read and write longs into EEPROM, 
// returning an updated pointer in each case.

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

void SaveToEeprom(ZeolitePath* zp)
{
  if(debug)
  {
    Serial.print("Saving ");
    Serial.print(zp->Name());
    Serial.println(" sequence to EEPROM.");
  }
    
  int ptr = 0;
  while(eTag[ptr])
  {
      EEPROM.write(ptr, eTag[ptr]);
      ptr++;
  }

  int sequence[] = zp->GetSequence();
  long times[] = zp->GetTimes();

  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    ptr = EepromWriteLong(ptr, (long)sequence[ss]);
    ptr = EepromWriteLong(ptr, times[ss]);
  }
}

// Load the default values.

void LoadDefaults(ZeolitePath* zp1, ZeolitePath* zp2)
{
  if(debug)
    Serial.println("Loading default variables.");

  zp1->SetSequenceAndTimes(defaultSequence, defaultTimes);
  zp2->SetSequenceAndTimes(defaultSequence, defaultTimes); 
  SaveToEeprom(zp1);
}


// Check if the EEPROM starts with a valid tag string.  If not, load the default data.
// If it does, load from the EEPROM.

void LoadDataFromEepromOrSetDefaults(ZeolitePath* zp1, ZeolitePath* zp2)
{
  int ptr = 0;
  while(eTag[ptr])
  {
    if(EEPROM.read(ptr) != eTag[ptr])
    {
      LoadDefaults(zp1, zp2);
      return;
    }
    ptr++;
  }

  if(debug)
    Serial.println("Loading variables from EEPROM.");

  int sequence[sequenceSteps];
  long s;
  long times[sequenceSteps];  

  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    ptr = EepromReadLong(ptr, s);
    sequence[ss] = (int)s;
    ptr = EepromReadLong(ptr, times[ss]);
  }

  zp1->SetSequenceAndTimes(sequence, times);
  zp2->SetSequenceAndTimes(sequence, times);  
}

// Print deciseconds with a decimal point

void PrintDeciSeconds(long d)
{
  long i = d/10;
  Serial.print(i);
  d = d - i*10;
  Serial.print('.');
  Serial.print(d);
}

// Print the state of the machine

void PrintState()
{
  
}

// Print prompts to refresh the user's memory

void Help()
{
  Serial.print("\nConcentrator state:\n ");
  Serial.print(left->GetName());
  Serial.print(" is ");
  PrintState(left->GetState());
  Serial.print(".\n ");
  Serial.print(right->GetName());
  Serial.print(" is ");
  PrintState(right->GetState());
  Serial.println(".");
  Serial.println("Concentrator variables:");
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

// Get an integer from what the user types

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
  // Keep the left and right arms working
  
  left->Spin();
  right->Spin();
  
  // Do we need to do anything else?

  if(!O2Demanded())
    return;

  // Yes; if either side it active return.
  // (If at least one is active the machine is already working.)

  if(left->Active() || right->Active())
    return;

  if(leftStart)
    left->StartSequence();
  else
    right->StartSequence();

  leftStart = !leftStart;
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
