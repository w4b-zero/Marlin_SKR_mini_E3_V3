/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../inc/MarlinConfig.h"

#if HAS_SOUND
#if ENABLED(GCODE_MUSIC)

#include "../gcode.h"
#include "../queue.h"
#include "../parser.h"

#include "../../lcd/marlinui.h" // i2c-based BUZZ
#include "../../libs/buzzer.h"  // Buzzer, if possible

/**
 * M9000_9004: Play a G-code sound macro.
 * M9005_9009: Set/Play a Custom G-code sound macro.
 *
 * Usage:
 *   M9000                 Play G-code sound macro 0
 *   M9001                 Play G-code sound macro 1
 *   M9002                 Play G-code sound macro 2
 *   M9003                 Play G-code sound macro 3
 *   M9004                 Play G-code sound macro 4
 *
 *   M9005 <command>|...   Set Custom G-code sound macro 5 to the given commands, separated by the pipe character
 *   M9005                 Play Custom G-code sound macro 5 
 *   M9006 <command>|...   Set Custom G-code sound macro 6 to the given commands, separated by the pipe character
 *   M9006                 Play Custom G-code sound macro 6 
 *   M9007 <command>|...   Set Custom G-code sound macro 7 to the given commands, separated by the pipe character
 *   M9007                 Play Custom G-code sound macro 7 
 *   M9008 <command>|...   Set Custom G-code sound macro 8 to the given commands, separated by the pipe character
 *   M9008                 Play Custom G-code sound macro 8 
 *   M9009 <command>|...   Set Custom G-code sound macro 9 to the given commands, separated by the pipe character
 *   M9009                 Play Custom G-code sound macro 9 
 * 
 *   <command> = M300 S<frequency> P<duration>
 *   <command> = M300 S1567 P206
 * 
 *********************************************
 * M300: Play a Tone / Add a tone to the queue
 *
 *  S<frequency> - (Hz) The frequency of the tone. 0 for silence.
 *  P<duration>  - (ms) The duration of the tone.
 *
 ********************************************* 
 *
 * M9000 = scooter
 * M9001 = startrek
 * M9002 = starwars
 * M9003 = mario
 * M9004 = mario2
 * M9005 = use for custom music
 * M9006 = use for custom music
 * M9007 = use for custom music
 * M9008 = use for custom music
 * M9009 = use for custom music
 */
// example for the first 5 G-code music macros
//  char gcode_music[GCODE_MUSIC_SLOTS][GCODE_MUSIC_SLOT_SIZE + 1] = {
//    "M300 S1567 P206|M300 S1567 P206|M300 S1396 P103|M300 S1318 P103|M300 S1396 P413|M300 S1174 P310|M300 S1046 P103|M300 S1174 P206|M300 S1567 P206|M300|S1567 P206|M300 S1396 P206|M300 S1318 P206|M300 S1567 P206|M300 S1567 P206|M300 S1396 P103|M300 S1318 P103|M300 S1396 P413|M300 S1174 P413|M300 S1318|P206|M300 S1046 P206|M300 S1174 P827|M300 S0 P206",
//    "M300 S1396 P714|M300 S932 P238|M300 S2489 P1428|M300 S2349 P476|M300 S932 P357|M300 S1567 P357|M300 S2093 P357|M300 S2793 P952",
//    "M300 S1396 P166|M300 S1396 P166|M300 S1396 P166|M300 S932 P1000|M300 S2793 P1000|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300|S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2489 P166|S2093 P666|M300 S0 P333|M300 S1396 P166|M300 S1396 P166|M300 S1396 P166|M300 S932 P1000|M300 S2793 P1000|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2489 P166|M300 S2093 P666",
//    "M300 S0 P600|M300 S1046 P450|M300 S1567 P150|M300 S0 P300|M300 S1318 P300|M300 S0 P150|M300 S880 P300|M300 S987 P150|M300 S0 P150|M300 S932 P150|M300 S880 P300|M300 S1567 P450|M300 S2637 P300|M300 S3135 P300|M300 S1760 P300|M300 S2637 P150|M300 S3135 P150|M300 S0 P150|M300 S2637 P300|M300 S2093 P150|M300 S2349 P150|M300 S987 P450|M300 S2093 P450|M300 S1567 P150|M300 S0 P300|M300 S1318 P300|M300 S0 P150|M300 S880 P300|M300 S987 P150|M300 S0 P150|M300 S932 P150|M300 S880 P300|M300 S1567 P300|M300 S2637 P300|M300 S3135 P300|M300 S1760 P300|M300 S2793 P150|M300 S3135 P150|M300 S0 P150|M300 S2637 P300|M300 S2093 P150|M300 S2349 P150|M300 S987 P300",
//    "M300 S2093 P83|M300 S2093 P83|M300 S2093 P83|M300 S0 P333|M300 S987 P166|M300 S2793 P166|M300 S0 P166|M300 S2793 P166|M300 S2793 P250|M300 S2637 P250|M300 S2349 P166|M300 S2093 P166|M300 S0 P166|M300 S1318 P166|M300 S0 P166|M300 S1046 P166"
//  }

  #define GCODE_MUSIC_SLOTS       5  // Up to 10 may be used
  #define GCODE_MUSIC_SLOT_SIZE  651  // Maximum length of a single macro

    char gcode_music[GCODE_MUSIC_SLOTS][GCODE_MUSIC_SLOT_SIZE + 1] = {
        "M300 S1567 P206|M300 S1567 P206|M300 S1396 P103|M300 S1318 P103|M300 S1396 P413|M300 S1174 P310|M300 S1046 P103|M300 S1174 P206|M300 S1567 P206|M300|S1567 P206|M300 S1396 P206|M300 S1318 P206|M300 S1567 P206|M300 S1567 P206|M300 S1396 P103|M300 S1318 P103|M300 S1396 P413|M300 S1174 P413|M300 S1318|P206|M300 S1046 P206|M300 S1174 P827|M300 S0 P206",
        "M300 S1396 P714|M300 S932 P238|M300 S2489 P1428|M300 S2349 P476|M300 S932 P357|M300 S1567 P357|M300 S2093 P357|M300 S2793 P952",
        "M300 S1396 P166|M300 S1396 P166|M300 S1396 P166|M300 S932 P1000|M300 S2793 P1000|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300|S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2489 P166|S2093 P666|M300 S0 P333|M300 S1396 P166|M300 S1396 P166|M300 S1396 P166|M300 S932 P1000|M300 S2793 P1000|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2093 P166|M300 S1864 P1000|M300 S2793 P500|M300 S2489 P166|M300 S2349 P166|M300 S2489 P166|M300 S2093 P666",
        "M300 S0 P600|M300 S1046 P450|M300 S1567 P150|M300 S0 P300|M300 S1318 P300|M300 S0 P150|M300 S880 P300|M300 S987 P150|M300 S0 P150|M300 S932 P150|M300 S880 P300|M300 S1567 P450|M300 S2637 P300|M300 S3135 P300|M300 S1760 P300|M300 S2637 P150|M300 S3135 P150|M300 S0 P150|M300 S2637 P300|M300 S2093 P150|M300 S2349 P150|M300 S987 P450|M300 S2093 P450|M300 S1567 P150|M300 S0 P300|M300 S1318 P300|M300 S0 P150|M300 S880 P300|M300 S987 P150|M300 S0 P150|M300 S932 P150|M300 S880 P300|M300 S1567 P300|M300 S2637 P300|M300 S3135 P300|M300 S1760 P300|M300 S2793 P150|M300 S3135 P150|M300 S0 P150|M300 S2637 P300|M300 S2093 P150|M300 S2349 P150|M300 S987 P300",
        "M300 S2093 P83|M300 S2093 P83|M300 S2093 P83|M300 S0 P333|M300 S987 P166|M300 S2793 P166|M300 S0 P166|M300 S2793 P166|M300 S2793 P250|M300 S2637 P250|M300 S2349 P166|M300 S2093 P166|M300 S0 P166|M300 S1318 P166|M300 S0 P166|M300 S1046 P166",
    };


void GcodeSuite::M9000_9004() {
  const uint8_t index = parser.codenum - 9000;
  if (index >= GCODE_MUSIC_SLOTS) return;


  const size_t len = strlen(parser.string_arg);

  if (len) {
    // Set a music
    if (len > GCODE_MUSIC_SLOT_SIZE)
      SERIAL_ERROR_MSG("Music too long.");
    else {
      char c, *s = parser.string_arg, *d = gcode_music[index];
      do {
        c = *s++;
        *d++ = c == '|' ? '\n' : c;
      } while (c);
    }
  }
  else {
    // Execute a macro
    char * const cmd = gcode_music[index];
    if (strlen(cmd)) process_subcommands_now(cmd);
  }
}

#endif // GCODE_MUSIC


#if ENABLED(CUSTOM_GCODE_MUSIC)
char custom_gcode_music[CUSTOM_GCODE_MUSIC_SLOTS][CUSTOM_GCODE_MUSIC_SLOT_SIZE + 1] = {{ 0 }};


void GcodeSuite::M9005_9009() {
  const uint8_t index = parser.codenum - 9000;
  if (index >= CUSTOM_GCODE_MUSIC_SLOTS) return;


  const size_t len = strlen(parser.string_arg);

  if (len) {
    // Set a music
    if (len > CUSTOM_GCODE_MUSIC_SLOT_SIZE)
      SERIAL_ERROR_MSG("Music too long.");
    else {
      char c, *s = parser.string_arg, *d = custom_gcode_music[index];
      do {
        c = *s++;
        *d++ = c == '|' ? '\n' : c;
      } while (c);
    }
  }
  else {
    // Execute a macro
    char * const cmd = custom_gcode_music[index];
    if (strlen(cmd)) process_subcommands_now(cmd);
  }
}

#endif // CUSTOM_GCODE_MUSIC



#endif // HAS_SOUND
