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

// The names of the valves. "start-other-path" isn't a valve; it's
// the point in one arm's sequence that it tells the other arm to start its sequence.
// We also have an enum for this to make the code easier to read.

const char* valveNames[numberOfValves+1] = { "feed_in", "purge_in", "o2_out", "purge_out", "start_other_path" };

// ZeolitePath is one half of the Oxygen Concentrator - either the left or the right arm.

// The constructor needs to know the valve pins and its name
    
ZeolitePath::ZeolitePath(const int pns[], const char* n)
{

  for(int valve = 0; valve < numberOfValves; valve++)
  {
    pins[valve] = pns[valve];
    pinMode(pins[valve], OUTPUT);
    digitalWrite(pins[valve], LOW);
  }

  // For messages etc.

  name = n;

// The other half of the machine; not set yet

  otherPath = nullptr;

// Timings and state

  lastTime = millis();

  active = false;

// What stage in the process this arm is at.

  pointInSequence = 0;
}

// Save all the settings to the EEPROM, starting with the validation tag string

void ZeolitePath::SaveToEEPROM()
{
  if(debug)
  {
    Serial.print("Saving ");
    Serial.print(GetName());
    Serial.println(" sequence to EEPROM.");
  }
    
  int ptr = 0;
  while(eTag[ptr])
  {
      EEPROM.write(ptr, eTag[ptr]);
      ptr++;
  }

  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    ptr = EepromWriteLong(ptr, (long)sequence[ss]);
    ptr = EepromWriteLong(ptr, times[ss]);
  }
}

bool ZeolitePath::LoadFromEEPROM()
{
  // Check if the EEPROM is valid
  
  int ptr = 0;
  while(eTag[ptr])
  {
    if(EEPROM.read(ptr) != eTag[ptr])
      return false;
    ptr++;
  }

  if(debug)
  {
    Serial.print(GetName());
    Serial.println(" loading sequence from EEPROM.");
  }

  long s; 

  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    ptr = EepromReadLong(ptr, s);
    sequence[ss] = (int)s;
    ptr = EepromReadLong(ptr, times[ss]);
  }

  CopySequenceToOtherPath();
  active = false;
  lastTime = millis();
  pointInSequence = 0;
  CloseAllValves();
  return true;
}


// Set the valve sequence and timings
    
void ZeolitePath::SetSequenceAndTimes(const int seq[], const long tims[])
{
  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    sequence[ss] = seq[ss];
    times[ss] = tims[ss]; 
  }
  SaveToEEPROM();
  CopySequenceToOtherPath();
  active = false;
  lastTime = millis();
  pointInSequence = 0;
  CloseAllValves();
}

// Set the valve sequence and timings from serial input

void ZeolitePath::SetSequenceAndTimes()
{
  // TODO put some code in here
  SaveToEEPROM();
  CopySequenceToOtherPath();
}

// Copy the sequence to the other path.  We can't call SetSequenceAndTimes for it
// as that would give infinite recursion.

void ZeolitePath::CopySequenceToOtherPath()
{
  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    otherPath->sequence[ss] = sequence[ss];
    otherPath->times[ss] = times[ss]; 
  } 
  otherPath->active = false;
  otherPath->lastTime = millis();
  otherPath->pointInSequence = 0;
  if(debug)
  {
    Serial.print("Sequence copied to ");
    Serial.println(otherPath->GetName());
  }
  otherPath->CloseAllValves();
}

// Execute one step in the sequence.

void ZeolitePath::DoThisStep()
{
  bool open = true;
  int valve = sequence[pointInSequence];
  if(valve < 0)
  {
    open = false;
    valve = abs(valve);
  }

  // Valves cannot have a 0 index; pins[] starts at [0]
    
  valve--;

  lastTime = millis();

  // Trigger the other arm?
  
  if(valve == numberOfValves)
  {
    if(debug)
    {
      Serial.print(GetName());
      Serial.print(" is triggering the other path at t = ");
      PrintDeciSeconds(millis()/100);
      Serial.println();
    }
    otherPath->StartSequence();
    return;
  }

  digitalWrite(pins[valve], open);  
  if(debug)
  {
    Serial.print(GetName());
    Serial.print(" is ");
    if(open)
      Serial.print("opening ");
    else
      Serial.print("closing "); 
    Serial.print(valveNames[valve]);
    Serial.print(" at t = ");
    PrintDeciSeconds(millis()/100);
    Serial.println();  
  }
}


// Start the sequence for this path
    
void ZeolitePath::StartSequence()
{
  if(debug)
    Serial.print(GetName());
  
  pointInSequence = 0;
  lastTime = millis();
  if(O2Demanded())
  {
    active = true;
    Serial.println(" is starting its sequence.");
    DoThisStep();
  } else
  {
    active = false;
    Serial.println(" is now idle.");
    CloseAllValves();
  }  
}

// Go to the next step

void ZeolitePath::StepSequence()
{
  pointInSequence++;
  if(pointInSequence >= sequenceSteps)
    StartSequence();
  else
    DoThisStep();
}

// Make sure all valves are closed

void ZeolitePath::CloseAllValves()
{
  if(debug)
  {
    Serial.print(GetName());
    Serial.println(" is closing all its valves.");
  }
  for(int valve = 0; valve < numberOfValves; valve++)
    digitalWrite(pins[valve], LOW);  
}

// Called in the main loop to run the valve sequence.  This should neither call delay()
// nor call any functions that call delay(); that is it should always return after a few microseconds.
    
void ZeolitePath::Spin()
{
  if(!Active())
    return;

  if(millis() - lastTime <= times[pointInSequence])
    return;

  StepSequence();
}

// Print the current sequence in human-understandable form

void ZeolitePath::PrintSequence()
{
  bool open;
  Serial.print("\n The ");
  Serial.print(name);
  Serial.println(" sequence is: ");
  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    Serial.print(" ");
    Serial.print(ss);
    Serial.print(": ");
    int valve = sequence[ss];
    long t = times[ss];
    open = true;
    if(valve < 0)
    {
      open = false;
      valve = abs(valve);
    }
    valve--;
    if(ss == pointInSequence)
      Serial.print('*');
    Serial.print(valveNames[valve]);
    if(ss == pointInSequence)
      Serial.print('*');
    if(valve != numberOfValves)
    {
       if(open)
        Serial.print(" opens");
       else
        Serial.print(" closes");
    }
    Serial.print(" <wait ");
    Serial.print(t);
    Serial.println("ms>");
  }
  Serial.print(" ");
  Serial.print(GetName());
  Serial.print(" path is ");
  if(Active())
    Serial.println("active.");
  else
    Serial.println("idle.");  
}

// Print the valve names and indices

void ZeolitePath::PrintValves()
{
  Serial.println("\nValve numbers:");
  for(int valve = 0; valve <= numberOfValves; valve++)
  {
    Serial.print(" ");
    Serial.print(valve + 1);
    Serial.print(": ");
    Serial.println(valveNames[valve]);   
  }
  Serial.println();
}
