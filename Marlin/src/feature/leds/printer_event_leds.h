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
#pragma once

/**
 * feature/leds/printer_event_leds.h - LED color changing based on printer status
 */

#include "leds.h"
#include "../../inc/MarlinConfig.h"

class PrinterEventLEDs {
private:
  static uint8_t old_intensity;

  #if HAS_LEDS_OFF_FLAG
    static bool leds_off_after_print;
  #endif

  static void set_done() { TERN(LED_COLOR_PRESETS, leds.set_default(), leds.set_off()); }

public:
//  #if HAS_TEMP_HOTEND
//    static LEDColor onHotendHeatingStart() { old_intensity = 0; return leds.get_color(); }
//    static void onHotendHeating(const celsius_t start, const celsius_t current, const celsius_t target);
//  #endif
  #if HAS_TEMP_HOTEND
    static LEDColor onHotendHeatingStart() { old_intensity = 0; return leds.get_color(); }
    static void onHotendHeating(const celsius_t start, const celsius_t current, const celsius_t target);
  #endif

//  #if HAS_HEATED_BED
//    static LEDColor onBedHeatingStart() { old_intensity = 127; return leds.get_color(); }
//    static void onBedHeating(const celsius_t start, const celsius_t current, const celsius_t target);
//  #endif
  #if HAS_HEATED_BED
    static LEDColor onBedHeatingStart() { old_intensity = 255; return leds.get_color(); }
    static void onBedHeating(const celsius_t start, const celsius_t current, const celsius_t target);
  #endif

  #if HAS_HEATED_CHAMBER
    static LEDColor onChamberHeatingStart() { old_intensity = 127; return leds.get_color(); }
    static void onChamberHeating(const celsius_t start, const celsius_t current, const celsius_t target);
  #endif

  #if HAS_TEMP_HOTEND || HAS_HEATED_BED || HAS_HEATED_CHAMBER
    static void onHeatingDone() {
      leds.set_red(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_red(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_red(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_red(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_red(); 
    }
    static void onPIDTuningDone(LEDColor c) { leds.set_color(c); }
  #endif

  #if HAS_MEDIA

    static void onPrintCompleted() {
      leds.set_green(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_green(); safe_delay(200); 
      leds.set_off(); safe_delay(200);
      leds.set_green(); safe_delay(200); 
      leds.set_off();  safe_delay(200);
      leds.set_green(); safe_delay(200); 
      leds.set_off();  safe_delay(200);
      leds.set_green(); safe_delay(200); 
      leds.set_off();  safe_delay(200);
      Marlin_NeoPixel::rainbow(10);
      leds.set_bright_green(); 
      #if HAS_LEDS_OFF_FLAG
        leds_off_after_print = true;
      #else
        safe_delay(2000);
        set_done();
      #endif
    }

    static void onResumeAfterWait() {
      #if HAS_LEDS_OFF_FLAG
        if (leds_off_after_print) {
          set_done();
          leds_off_after_print = false;
        }
      #endif
    }

  #endif // HAS_MEDIA
};

extern PrinterEventLEDs printerEventLEDs;
