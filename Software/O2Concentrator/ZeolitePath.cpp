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

ZeolitePath::ZeolitePath(int fi, int pi, int oo, int po)
{  
  feedIn = fi;
  pinMode(feedIn, OUTPUT);
  digitalWrite(feedIn, LOW);
  
  purgeIn = pi;
  pinMode(purgeIn, OUTPUT);
  digitalWrite(purgeIn, LOW);
  
  o2Out = oo;
  pinMode(o2Out, OUTPUT);
  digitalWrite(o2Out, LOW);
  
  purgeOut = po;
  pinMode(purgeOut, OUTPUT);
  digitalWrite(purgeOut, LOW);

// The other half of the machine; not set yet

  otherPath = nullptr;

// Timings and state

  state = idle;
  lastTime = millis();
  interval = 0;
  o2Requested = false;
}

// Called in the main loop to run the valve sequence.  This should neither call delay()
// nor call any functions that call delay(); that is it should always return after a few microseconds.
    
void ZeolitePath::Spin()
{
  // Are we due to do anything?

  unsigned long now = millis();

  // Maybe. What?
  
  switch(state)
  {
    case idle:
      return;

    case o2Feed:
      if(now - lastTime <= interval)
        return;
      SwitchToPurge();
      break;
      
    case purging:
      if(now - lastTime <= interval)
        return;
      EndPurge();
      break;

    case shuttingDown:
     if(now - lastTime <= interval)
        return;
     ShutDown();
     break;    

    default:
      Serial.println("ERROR - dud state in ZeolitePath::Spin().");
      state = idle;
      interval = 0;
  }

  // State has changed
  
  lastTime = now; 
}

// Start the O2 flow from this path
    
void ZeolitePath::StartFeed()
{

  // If we are purging just set the O2 request flag and return.

  if(state == purging)
  {
    o2Requested = true;
    return;
  }
  
  // Make sure the purging valves are closed.
  
  digitalWrite(purgeIn, LOW);
  digitalWrite(purgeOut, LOW);

  // Open the air in and O2 out valves
  
  digitalWrite(feedIn, HIGH);
  digitalWrite(o2Out, HIGH);

  // Set the state and the time to feed O2

  state = o2Feed;
  interval = o2FeedTime;
}

// Switch from O2 flow to purging
    
void ZeolitePath::SwitchToPurge()
{
  // Make sure the O2 valves are closed.
  
  digitalWrite(feedIn, LOW);
  digitalWrite(o2Out, LOW);

  // Open the purging valves
  
  digitalWrite(purgeIn, HIGH);
  digitalWrite(purgeOut, HIGH);

  // Set the state and the time to purge

  state = purging;
  interval = purgingTime;

  // Set the other path feeding O2 if there is demand.

  if(O2Demanded())
    otherPath->StartFeed();
}

// Finish purging
    
void ZeolitePath::EndPurge()
{
  // Is there an O2 request waiting?

  if(o2Requested)
  {
    // Yes - switch straight back to feed.
    
    o2Requested = false;
    StartFeed();
    return;
  }
  
  // No. Make sure the purging valves are closed.
  
  digitalWrite(purgeIn, LOW);
  digitalWrite(purgeOut, LOW);
  
  // Set the state and the time to shut down
  // If the other path requests us to start in the
  // mean time we never actually
  // shut down, but switch back to feeding O2.

  state = shuttingDown;
  interval = shuttingDownTime;    
}

// Shut down (i.e. close all valves)

void ZeolitePath::ShutDown()
{
  digitalWrite(purgeIn, LOW);
  digitalWrite(purgeOut, LOW);
  digitalWrite(feedIn, LOW);
  digitalWrite(o2Out, LOW);
  state = idle;
  interval = 0;  
}
