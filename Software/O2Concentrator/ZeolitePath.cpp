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

// ZeolitePath is one half of the Oxygen Concentrator - either the left or the right arm.

// The constructor needs to know the valve pins and name
    
ZeolitePath(int pns[], char* n)
{

  for(int v = 0; v < N_VALVES - 1; v++)
  {
    pins[v] = pns[v];
    pinMode(pins[v], OUTPUT);
    digitalWrite(pins[v], LOW);
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
    
void ZeolitePath::SetSequenceAndTimes(int seq[], long tims[])
{
  for(int s = 0; s < N_VALVES - 1; s++)
  {
    sequence[s] = seq[s];
    times[s] = tims[s]; 
  }
}


// Start the sequence from this path
    
void ZeolitePath::void StartSequence()
{
  
}


void ZeolitePath::StepSequence()
{
  pointInSequence++;
  switch(pointInSequence)
  {
     
  }
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
