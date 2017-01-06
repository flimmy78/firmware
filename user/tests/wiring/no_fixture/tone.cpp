/**
 ******************************************************************************
  Copyright (c) 2013-2015 IntoRobot Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#include "application.h"
#include "tone_hal.h"
#include "unit-test/unit-test.h"

#if defined(STM32F2XX)
static pin_t pin = D1;//pin under test
#else
static pin_t pin = A1;//pin under test
#endif

test(TONE_NoGenerateWhenPinSelectedIsNotTimerChannel) {
    uint8_t pin = D5;//pin under test
    uint32_t frequency = 500;
    uint32_t duration = 100;
    // when
    tone(pin, frequency, duration);
    // then
    assertNotEqual(HAL_Tone_Is_Stopped(pin), false);
    //To Do : Add test for remaining pins if required
}

test(TONE_GeneratedOnPinResultsInCorrectFrequency) {
    uint32_t frequency = 500;
    uint32_t duration = 100;
    // when
    tone(pin, frequency, duration);
    // then
    assertEqual(HAL_Tone_Get_Frequency(pin), frequency);
    //To Do : Add test for remaining pins if required
}

test(TONE_GeneratedOnPinResultsInCorrectDuration) {
    uint32_t frequency = 500;
    uint32_t duration = 100;
    // when
    tone(pin, frequency, duration);
    // then
    assertEqual(HAL_Tone_Is_Stopped(pin), false);
    // when
    delay(110);//delay for > 100 ms
    // then
    assertEqual(HAL_Tone_Is_Stopped(pin), true);
    //To Do : Add test for remaining pins if required
}

test(TONE_GeneratedOnPinStopsWhenStopped) {
    uint32_t frequency = 500;
    uint32_t duration = 100;
    // when
    tone(pin, frequency, duration);
    // then
    assertEqual(HAL_Tone_Is_Stopped(pin), false);
    // when
    noTone(pin);
    // then
    assertEqual(HAL_Tone_Is_Stopped(pin), true);
    //To Do : Add test for remaining pins if required
}

