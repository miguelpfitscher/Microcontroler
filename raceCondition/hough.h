#ifndef __HOUGH
#define __HOUGH 1

#include <math.h>
#include <avr/pgmspace.h>

#include "image10.h"

char houghTransform( uint16_t rPtr, uint16_t gPtr, uint16_t bPtr );

#endif