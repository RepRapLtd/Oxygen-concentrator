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

// ZeolitePath is one half of the Oxygen Concentrator - either the left or the right arm.

#ifndef ZEOLITEPATH_H
#define ZEOLITEPATH_H

// Number of valves in one arm of the machine

const int numberOfValves = 4;

// Each valve has an on and an off, plus we have to tell the other arm to start

const int sequenceSteps = 2*numberOfValves + 1;

// The names of the valves. "start-other-arm" isn't a valve; it's
// the point in one arm's sequence that it tells the other arm to start its sequence.
// We also have an enum for this to make the code easier to read.

const char* valveNames[numberOfValves+1] = { "feed_in", "purge_in", "o2_out", "purge_out", "start_other_arm" };

class ZeolitePath
{
  public:

    // The constructor needs to know the valve pins and name
    
    ZeolitePath(const int pns[], const char* n);

    // Set the valve sequence and timings
    
    void SetSequenceAndTimes(const int seq[], const long tims[]);

    // Tell this path about the opposite path in the machine
    
    void SetOtherPath(ZeolitePath* op);

    // Called in the main loop to run the valve sequence.  This should neither call delay()
    // nor call any functions that call delay(); that is it should always return after a few microseconds.
    
    void Spin();

    // Start the sequence for this path
    
    void StartSequence();

    // Return the current name

    const char* GetName();

    // Return the current sequence

    int* GetSequence();

    // Return the current timings

    long* GetTimes();

    // Are we busy?

    bool Active();

    // Print the current sequence in human-understandable form

    void PrintSequence();

    // Print the valve names and indices

    void PrintValves();

  private:

    // One step forward in the sequence
    
    void StepSequence();

    // Do this step in the sequence

    void DoThisStep();

    // The Arduino pins that drive the solenoid valves

    int pins[numberOfValves];

    // The sequence
        
    int sequence[sequenceSteps];

    // The timings
    
    long times[sequenceSteps];

    // The other half of the machine

    ZeolitePath* otherPath;

    // Timing. lastTime is the time this bit of the sequence started. 

    unsigned long lastTime;

    // For messages etc.

    const char* name;

    // Are we active?

    bool active;

    // What stage in the process this arm is at.

    int pointInSequence;
  
};

// Tell this path about the opposite path in the machine
    
inline void ZeolitePath::SetOtherPath(ZeolitePath* op) { otherPath = op; }

// Busy?

inline bool ZeolitePath::Active() { return active; }

// Who?

inline const char* ZeolitePath::GetName() { return name; }

// The whole sequence

inline int* ZeolitePath::GetSequence() { return sequence; }

// All the timings

inline long* ZeolitePath::GetTimes() { return times; }

#endif
