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

// Serial comms speed

#define BAUD 9600

// Print debugging information when true.

const bool debug { true } ;

// --------------------------------------------------
// pins
typedef const uint8_t t_pin_number ;
// --------------------------------------------------
// Solenoid valve control pins: [0] - left; [1] - right
t_pin_number zeoliteIn   [2] {2, 5} ;
t_pin_number zeoliteOut  [2] {3, 6} ;
t_pin_number airVent     [2] {4, 7} ;
// O2 level sensor pins
t_pin_number o2LevelLow         {8} ;
t_pin_number o2LevelHigh        {9} ;
// --------------------------------------------------

// --------------------------------------------------
// Timings (in ms) and other variables
typedef uint16_t t_value  ;
// --------------------------------------------------
#define TAG_CONSTANTS 'u', 'A' , 'u', 'A' , 'u', 'A' 
constexpr byte k_size_tag_eeprom = 6 ;
constexpr char k_valid_marker[k_size_tag_eeprom] { TAG_CONSTANTS };

class t_eeprom_data //https://www.arduino.cc/en/Reference/EEPROMPut
{
  public:
  const char f_marker[k_size_tag_eeprom] { TAG_CONSTANTS } ;
  
  #define VOT 100
  t_value valveOpenTime   {VOT};

  #define VCT 100
  t_value valveCloseTime  {VCT};
  
  #define ZIT 1000
  t_value zeoliteInTime   {ZIT};
  
  #define ZHT 3000
  t_value zeoliteHoldTime {ZHT};
  
  #define ZOT 500
  t_value zeoliteOutTime  {ZOT};
  
  #define PT 1000
  t_value purgeTime       {PT };

  #define CTF 1
  t_value cyclesToFull    {CTF}; 

  
  void load_defaults ( )
  {
    valveOpenTime   = VOT ; 
    valveCloseTime  = VCT ;
    zeoliteInTime   = ZIT ;
    zeoliteHoldTime = ZHT ;
    zeoliteOutTime  = ZOT ;
    purgeTime       = PT  ;
    cyclesToFull    = CTF ; 
  } 
} ;
// declare variable of eeprom data
// internal values will be initialsed here..
t_eeprom_data my_eeprom_data ;
// --------------------------------------------------

// --------------------------------------------------
t_value cycleCount { my_eeprom_data.cyclesToFull + 1 } ;

uint8_t arm  { 0 } ; // assuming arm can work with this type
//int arm = 0; 
// --------------------------------------------------

// --------------------------------------------------
// Save all the settings to the EEPROM, starting with the validation tag string
// --------------------------------------------------
void debug_msg_saving_to_eeprom ( ) {if(debug) Serial.println ( "Saving variables to EEPROM." ) ; }
void SaveToEeprom()
{
  debug_msg_saving_to_eeprom (                    ) ;
  EEPROM.put                 ( 0 , my_eeprom_data ) ; 
}
// --------------------------------------------------

// --------------------------------------------------
// Load the default values.
// --------------------------------------------------
void debug_msg_loading_defaults ( ) { if(debug) Serial.println ( "Loading default variables." ) ; } 
void LoadDefaults ( )
{
  debug_msg_loading_defaults   ( ) ;
  my_eeprom_data.load_defaults ( ) ;
  SaveToEeprom                 ( ) ; // these will already be initialised in my_eeprom_data
}
// --------------------------------------------------


// --------------------------------------------------
// Check if the EEPROM starts with a valid tag string.  If not, load the default data.
// If it does, load from the EEPROM.
// --------------------------------------------------
void debug_msg_loading_from_eeprom ( ) { if (debug) Serial.println ( "Loading variables from EEPROM." ) ; }
void load_from_eeprom ( )
{
  debug_msg_loading_from_eeprom (                    ) ;
  EEPROM.get                    ( 0 , my_eeprom_data ) ; 
}
bool is_valid_eeprom ( )
{
  bool my_result { true } ;
  char my_char   {      } ;
  
  for ( byte idx = 0 ; idx < k_size_tag_eeprom ; ++idx )
  {
    my_char = EEPROM.read ( idx ) ;
    // debug test, intended for developer only: 
    // Serial.print ( my_char ) ;
    //--
    if ( my_char != k_valid_marker[idx] )
    {
      my_result = false ;
      break ;  
    }
  }
  return my_result ;
}
void LoadDataFromEepromOrSetDefaults ( )
{
  if ( is_valid_eeprom ( ) ) { load_from_eeprom ( ) ; }
  else                       { LoadDefaults     ( ) ; }
}
// --------------------------------------------------

// --------------------------------------------------
// see https://en.wikipedia.org/wiki/Printf_format_string
// %[parameter][flags][width][.precision][length]type
// --------------------------------------------------
void print_formatted ( const char* a_text , t_value a_value ) 
{
  char my_buffer[100]                             ;
  sprintf        ( my_buffer , a_text , a_value ) ;
  Serial.println ( my_buffer                    ) ; 
} 
void print_line_header  ( ) { Serial.println ( "================================================" ) ; }
void print_line_plitter ( ) { Serial.println ( "------------------------------------------------" ) ; }
// --------------------------------------------------

// --------------------------------------------------
void send_help_info_to_serial_port ( )
{
  // formats ok in PuTTY Serial Monitor
  print_line_header  (                  ) ;
  Serial.println     ( "Current Values" ) ;
  print_line_header  (                  ) ;
  print_formatted    ( "-o Time  Valve   open   (%5u) milliseconds" , my_eeprom_data.valveOpenTime   ) ;
  print_formatted    ( "-c Time  Valve   close  (%5u) milliseconds" , my_eeprom_data.valveCloseTime  ) ;
  print_formatted    ( "-i Time  Zeolite in     (%5u) milliseconds" , my_eeprom_data.zeoliteInTime   ) ;
  print_formatted    ( "-h Time  Zeolite hold   (%5u) milliseconds" , my_eeprom_data.zeoliteHoldTime ) ;
  print_formatted    ( "-e Time  Zeolite exit   (%5u) milliseconds" , my_eeprom_data.zeoliteOutTime  ) ;
  print_formatted    ( "-p Time  Zeolite purge  (%5u) milliseconds" , my_eeprom_data.purgeTime       ) ;
  print_formatted    ( "-y Count Cycles To Full (%5u)             " , my_eeprom_data.cyclesToFull    ) ;
  print_line_plitter (                           ) ;
  Serial.println     ( "-d Load default values " ) ;
  Serial.println     ( "-? Print this list     " ) ;
  print_line_header  (                           ) ;
}
// --------------------------------------------------

// --------------------------------------------------
// return a value
// --------------------------------------------------
long ReadInteger ( )
{
  return (t_value)Serial.parseInt ( ) ; 
}
// --------------------------------------------------


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   todo:
   replace with non blocking / non timeout 
   serial input routines
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/ 
// --------------------------------------------------
// Allow the user to change values.
// Save all the values to EEPROM whenever one changes.
// --------------------------------------------------
bool is_comand_ready    ( ) { return ( Serial.available ( ) > 0 ) ; }
void do_execute_command ( )
{
  bool save { true } ;

  int c = Serial.read ( ) ;
  
  switch ( c )
  {
    case 'o': my_eeprom_data.valveOpenTime   = ReadInteger ( ) ; break ; 
    case 'c': my_eeprom_data.valveCloseTime  = ReadInteger ( ) ; break ; 
    case 'i': my_eeprom_data.zeoliteInTime   = ReadInteger ( ) ; break ; 
    case 'h': my_eeprom_data.zeoliteHoldTime = ReadInteger ( ) ; break ; 
    case 'e': my_eeprom_data.zeoliteOutTime  = ReadInteger ( ) ; break ; 
    case 'p': my_eeprom_data.purgeTime       = ReadInteger ( ) ; break ; 
    case 'y': my_eeprom_data.cyclesToFull    = ReadInteger ( ) ; break ; 
    case 'd': 
      LoadDefaults ( ) ;
      save = false     ;
      break            ;
    case '?':
    default:
      /* will fire for each End-of-line and Newline character
      Serial.println(c, DEC);
      Serial.println("** default **" );
      */
      send_help_info_to_serial_port ( ) ;
      save = false                      ;
  }

  if ( save )
  {
    SaveToEeprom ( ) ;  
  }
  else
  {
    // do nothing  
  }
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 
void Command ( )
{
  if ( is_comand_ready ( ) ) 
  {
    do_execute_command ( ) ;
  } 
  else
  {
    // carry on as normal  
  }
}
// --------------------------------------------------

// --------------------------------------------------
// Control O2 generation
// --------------------------------------------------
//private functions
// --------------------------------------------------------------------------------------------------
// debug messages
void debug_msg_concentrator_is_active ( )
{
  if ( debug )
  {
    // .. causes exception fault in gcc compiler.
    print_formatted ( "Concentrator is active. Shutting valves on side (%u) " , arm   ) ;
    // so stick with this for now
    /*
    Serial.print   ( "Concentrator is active. Shutting valves on " ) ;
    Serial.print   ( arm                                           ) ;
    Serial.println ( " side."                                      ) ;
    */
  }  
}
void debug_msg_air_in              ( ) { if ( debug ) Serial.println ( "Air - in."                    ) ; }
void debug_msg_adsorbing_N2        ( ) { if ( debug ) Serial.println ( "N2  - Adsorbing."             ) ; }
void debug_msg_sending_o2_to_output( ) { if ( debug ) Serial.println ( "O2  - Sending to the output." ) ; }
void debug_msg_purge_N2            ( ) { if ( debug ) Serial.println ( "N2  - Purging."               ) ; } 

void debug_msg_cycle_complete      ( ) { if ( debug ) Serial.println ( "Cycle complete.\n"            ) ; }

// --------------------------------------------------------------------------------------------------
bool is_inactive          (                           ) { return ( cycleCount >= my_eeprom_data.cyclesToFull ) ; } 
bool is_need_to_be_active (                           ) { return ( !digitalRead ( o2LevelLow )               ) ; }

void reset_cycle_count    (                           ) 
{ 
  Serial.println("Resetting Cycle Count");
  cycleCount = 0                        ; 
}
void open_valve           ( t_pin_number a_pin_number ) { digitalWrite ( a_pin_number, HIGH )   ; }
void close_valve          ( t_pin_number a_pin_number ) { digitalWrite ( a_pin_number, LOW  )   ; }

// Sends '....' to serial port, appropriate for visual debugging
// where exact  millis timings are not critical.
void show_progress_delay( t_value a_delay_time ) 
{
  constexpr uint8_t k_interval {50} ;
  unsigned long my_millis_now { millis() } ;

  // relying on modular arithmetic to handle timer rollover
  //https://en.wikipedia.org/wiki/Modular_arithmetic
  while ( ( millis() - my_millis_now ) < a_delay_time )
  {
    Serial.print ('.');
    delay ( k_interval ) ;   
  }
  delay ( a_delay_time % k_interval ) ; // delay for any remaing millis
  Serial.println ("");
}

void open_valve_and_wait ( t_pin_number a_pin_number )
{
  open_valve          ( a_pin_number                 ) ;  
  show_progress_delay ( my_eeprom_data.valveOpenTime ) ;
}
void close_valve_and_wait ( t_pin_number a_pin_number )
{
 close_valve         ( a_pin_number                  ) ;
 show_progress_delay ( my_eeprom_data.valveCloseTime ) ;
}
void close_all_valves ( )
{ //Serial.println("closing all valves"); // this was firing often
  for ( byte idx = 0 ; idx < 2 ; ++idx )
  {
    close_valve ( zeoliteIn  [idx] ) ;
    close_valve ( zeoliteOut [idx] ) ;
    close_valve ( airVent    [idx] ) ;
  }
}
void close_all_valves_and_wait     ()
{
  close_all_valves    (                               ) ;
  show_progress_delay ( my_eeprom_data.valveCloseTime ) ;
}
void let_compressed_air_into_chamber()
{
  debug_msg_air_in    (                               ) ;
  open_valve_and_wait ( zeoliteIn[arm]                ) ;
  show_progress_delay ( my_eeprom_data.zeoliteInTime  ) ; // Wait for the air to fill the chamber
}
void wait_for_zeolite_absorb()
{ 
  debug_msg_adsorbing_N2         (                                ) ;
  show_progress_delay            ( my_eeprom_data.zeoliteHoldTime ) ;
  debug_msg_sending_o2_to_output (                                ) ; // Let the O2 go to the output
  open_valve_and_wait            ( zeoliteOut[arm]                ) ;  
  show_progress_delay            ( my_eeprom_data.zeoliteOutTime  ) ; // Wait for it to get there
}
void open_purge_and_blow_through()
{ 
  debug_msg_purge_N2  (                          ) ;
  open_valve          ( airVent  [arm]           ) ;
  open_valve_and_wait ( zeoliteIn[arm]           ) ;
  show_progress_delay ( my_eeprom_data.purgeTime ) ;  // Wait for the purge
}

// --------------------------------------------------

// --------------------------------------------------
// routines for setup
// --------------------------------------------------
void set_pin_modes_for_valves ( )
{
  // Solenoid Valve control pins
  for ( byte idx = 0; idx < 2; ++idx )
  {
    pinMode     ( zeoliteIn  [idx], OUTPUT ) ;
    pinMode     ( zeoliteOut [idx], OUTPUT ) ;
    pinMode     ( airVent    [idx], OUTPUT ) ;
  }
}
void set_pin_mode_for_o2_level ( )
{
  // O2 level sensor pins
  pinMode ( o2LevelLow  , INPUT_PULLUP ) ;
  pinMode ( o2LevelHigh , INPUT_PULLUP ) ;
}
void serial_init ( )
{
  // ---------------------------
  // init serial and show notice
  // ---------------------------
  Serial.begin  ( BAUD ) ;
  // some devices may require a check for serial to be ready. (i.e. non UNO)
  // see https://www.arduino.cc/reference/en/language/functions/communication/serial/ifserial/
  while ( !Serial ) 
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println ( "RepRap Ltd Oxygen Concentrator Starting" ) ;
  // ---------------------------
  //confirm the initialisation is ok
  debug_msg_cycle_count    ( ) ;
}


// ///////////////////////////////////////////////////////
// oxygen concentrator setup and loop 
// ///////////////////////////////////////////////////////

void setup ( ) 
{
  set_pin_modes_for_valves        ( ) ; 
  arm = 0                             ; //global variable used by control, should be zero at this point anyway
  
  close_all_valves_and_wait       ( ) ;
  set_pin_mode_for_o2_level       ( ) ;
  serial_init                     ( ) ;
  LoadDataFromEepromOrSetDefaults ( ) ;
  send_help_info_to_serial_port   ( ) ;
}
// --------------------------------------------------



// --------------------------------------------------
// Routines required for Control
// --------------------------------------------------
void do_state_inactive ( )
{
  static bool is_closed_all_valves { false } ;
  
  if ( is_need_to_be_active ( ) ) 
  { 
    reset_cycle_count ( ) ; // resetting the cycle count will automatically make that happen.
  } 
  else                            
  { 
    if ( ! is_closed_all_valves )
    {
      close_all_valves  ( ) ; // Inactive and not needed. Make sure all valves are closed 
                              // (not needed for function, but reduces current consumption). 
      is_closed_all_valves = true ;                        
    }
  } 
                                                              
}
// --------------------------------------------------
void debug_msg_cycle_count ( ) { if ( debug ) { print_formatted ( "Cycle Count = (%u)" , cycleCount ) ; } }
void do_one_cycle          ( )
{
  debug_msg_concentrator_is_active ( ) ;
  
  close_valve                     ( zeoliteIn  [arm] ) ; // We are active; start by shutting all valves
  close_valve                     ( zeoliteOut [arm] ) ;
  close_valve_and_wait            ( airVent    [arm] ) ;
  //------------------------------- 
  let_compressed_air_into_chamber (                  ) ;  // Let the compressed air into the chamber
  close_valve_and_wait            ( zeoliteIn  [arm] ) ;  // Close the input valve
  wait_for_zeolite_absorb         (                  ) ;  // Wait for the zeolite to adsorb the N2
  close_valve_and_wait            ( zeoliteOut [arm] ) ;  // Close the output valve
  open_purge_and_blow_through     (                  ) ;  // Open the purge valve and the compressed air to blow through
  close_valve_and_wait            ( zeoliteIn  [arm] ) ;  // Close the input valve; leave the purge valve open

  debug_msg_cycle_complete ( ) ;
  cycleCount++                 ;
  debug_msg_cycle_count    ( ) ;
}
void do_state_active ( )
{
  do_one_cycle ( ) ; // That was one cycle 
  arm = 1 - arm    ;  // Swap to the other arm for the next cycle
}
// --------------------------------------------------

// --------------------------------------------------
// Control
// --------------------------------------------------
void Control ( )
{
  if ( is_inactive ( ) ) { do_state_inactive ( ) ; }
  else                   { do_state_active   ( ) ; }
}
// --------------------------------------------------

// --------------------------------------------------
// loop
// --------------------------------------------------
void loop ( )
{
  Command ( ) ; // change any values that arrive from serial port
  Control ( ) ; // control the oxygen concentrator
}
// --------------------------------------------------
