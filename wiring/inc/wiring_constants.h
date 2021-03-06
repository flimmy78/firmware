/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

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

#ifndef WIRING_CONSTANTS_H_
#define WIRING_CONSTANTS_H_


#include <stdlib.h>
#include <stdint.h>
#include <type_traits>

enum PinState {
    LOW = 0,
    HIGH = 1
};

#if !defined(INTOROBOT_WIRING_ARDUINO_COMPATIBILTY) || !INTOROBOT_WIRING_ARDUINO_COMPATIBILTY
#define INTOROBOT_WIRING_ARDUINO_TEMPLATES 1

template <typename T, typename U>
static inline
typename std::common_type<T, U>::type
max (T a, U b) { return ((a)>(b)?(a):(b)); }

template <typename T, typename U>
static inline
typename std::common_type<T, U>::type
min (T a, U b) { return static_cast<typename std::common_type<T, U>::type>((a)<(b)?(a):(b)); }

template <typename T, typename U, typename V>
static inline
T constrain (T amt, U low, V high) { return ((amt)<(low)?(low):((amt)>(high)?(high):(amt))); }

template <typename T>
static inline
T round (T x) { return ((x)>=0?(long)((x)+0.5):(long)((x)-0.5)); }
#else
#define INTOROBOT_WIRING_ARDUINO_TEMPLATES 0
#endif // #ifndef INTOROBOT_WIRING_ARDUINO_COMPATIBILTY

typedef bool boolean;
typedef uint8_t byte;

#ifndef FALSE
#define FALSE					(0x00)
#endif
#ifndef TRUE
#define TRUE					(!FALSE)
#endif

#endif
