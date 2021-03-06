/******************************************************************************
  Copyright (2022), Brilliant Labs Limited (Hong Kong)
  Licensed under the MIT License
*******************************************************************************/

/** @file touch.h
 *	@brief Function prototypes for touch interface. Hardware dependencies should be in board.h
 *
 *	@author Nathan Ashelman for Brilliant Labs Limited
 *	
 *	@bug No known bugs.
 */

#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "board.h"           // for TOUCH_CAPSENSE

// ===== Logging for this module =====
#define TOUCH_LOG_INFO_ON
//#define TOUCH_LOG_DEBUG_ON

// ===== "Gestures" detected by this module =====
typedef enum {
#ifdef TOUCH_CAPSENSE         // gestures requiring 2 sensors:
    TOUCH_GESTURE_SLIDELR,    //   "slide" left to right = Tap left then right buttons
    TOUCH_GESTURE_SLIDERL,    //   "slide" right to left = Tap right then left buttons
    TOUCH_GESTURE_PRESSBOTH,  //   push both buttons for >0.5s & <10s then release
    TOUCH_GESTURE_LONGBOTH,   //   push both buttons for >10s then release
#endif
    TOUCH_GESTURE_TAP,        // button push & release in <0.5s, with no push in following 0.25s
    TOUCH_GESTURE_DOUBLETAP,  // Tap, followed by another Tap <0.25s later
    TOUCH_GESTURE_PRESS,      // push for >0.5s & <10s then release
    TOUCH_GESTURE_LONGPRESS   // push for >10s then release
} touch_gesture_t;

/**@brief Touch gesture handler type. */
typedef void (*touch_gesture_handler_t)(touch_gesture_t gesture);

void touch_quick_init(void);
bool touch_init(touch_gesture_handler_t handler);
bool touch_reprogram(void);

#endif // TOUCH_H
