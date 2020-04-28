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


#ifndef O2CONCENTRATOR_H
#define O2CONCENTRATOR_H

//#include <ctype.h>
#include <Arduino.h>

// We need to save the parameters for when we reboot.

#include <EEPROM.h>

#include "ZeolitePath.h"

// True when O2 is needed

bool O2Demanded();

// Print deciseconds with a decimal point

void PrintDeciSeconds(long d);

// Serial comms speed

#define BAUD 9600

// Globals

extern bool debug;
extern const int numberOfValves;
extern const int sequenceSteps;


#endif
