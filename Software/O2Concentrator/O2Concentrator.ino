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

const int defaultSequence[sequenceSteps] = {1, 3, -1, -3, 2, 4, 5, -2, -4};
const long defaultTimes[sequenceSteps] = {1000, 5000, 0, 0, 0, 0, 5000, 0, 0};

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

// Load the default values.

void LoadDefaults(ZeolitePath* zp1)
{
  if(debug)
    Serial.println("Loading default sequence into both paths.");

  // Setting one automatically sets the other too
  
  zp1->SetSequenceAndTimes(defaultSequence, defaultTimes); 
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
  Serial.println("Current O2 Concentrator sequences (*...* is the current state):");
  left->PrintSequence();
  right->PrintSequence();
  Serial.println();  
}

// Print prompts to refresh the user's memory

void Help()
{
  Serial.println("\nMenu:");
  Serial.println(" d - Load the default sequence");
  Serial.println(" p - Print the state of the machine");
  Serial.println(" s - Set a new sequence");
  Serial.println(" e - Erase EEPROM & so force defaults on next boot");   
  Serial.println(" ? - Print this list\n");
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
    
  int c = tolower(Serial.read());
  switch(c)
  {
    case 'd':
      LoadDefaults(left);
      break;

    case 'p':
      PrintState();
      break;
      
    case 's':
      left->SetSequenceAndTimes();
      break;

    case 'e':
      EepromWriteLong(0, 0);
      if(debug)
        Serial.println("EEPROM erased.");
      break;

    case ' ':
    case '\r':
    case '\n':
      break;
      
    case '?':
    default:
      Help();
  }
}

// Control O2 generation

void Control()
{
  // Keep the left and right paths working
  
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

  // Loading one side also loads the other
  
  if(!left->LoadFromEEPROM())
    LoadDefaults(left);

  Help();
}

void loop()
{
  Command();
  Control();
}
