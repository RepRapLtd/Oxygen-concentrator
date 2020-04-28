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

// The names of the valves. "start-other-arm" isn't a valve; it's
// the point in one arm's sequence that it tells the other arm to start its sequence.
// We also have an enum for this to make the code easier to read.

const char* valveNames[numberOfValves+1] = { "feed_in", "purge_in", "o2_out", "purge_out", "start_other_arm" };

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


// Set the valve sequence and timings
    
void ZeolitePath::SetSequenceAndTimes(const int seq[], const long tims[])
{
  for(int ss = 0; ss < sequenceSteps; ss++)
  {
    sequence[ss] = seq[ss];
    times[ss] = tims[ss]; 
  }
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
      Serial.println(" is triggering the other path.");
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
      Serial.print("opening");
    else
      Serial.print("closing"); 
    Serial.println(valveNames[valve]);  
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
       Serial.print(" < wait: ");
       Serial.print(t);
       Serial.println("ms >");
    }
  }
  Serial.print(" This path is ");
  if(Active())
    Serial.println("active.\n");
  else
    Serial.println("idle.\n");  
}

// Print the valve names and indices

void ZeolitePath::PrintValves()
{
  Serial.println("\nValve numbers:");
  for(int valve = 0; valve < numberOfValves; valve++)
  {
    Serial.print(" ");
    Serial.print(valve);
    Serial.print(": ");
    Serial.println(valveNames[valve]);   
  }
  Serial.println();
}
