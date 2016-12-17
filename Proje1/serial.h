#ifndef SERIAL_H
#define SERIAL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#define COM_PORT 4
#else
#include <unistd.h>
#define COM_PORT 24
#endif

#define BD_RATE 9600

#include "rs232.h"

//serial portu initilize eder
bool init_serial();
/**
 * serial porttan buffer okur.
 * @param buf: karakter bufferı
 * @param size: okunmak istenen boyut
 */
bool readBuf(char *buf, int size);
/**
 * 
 * @param buf karakter bufferı
 * @param size gönderilmek istenen verinin boyutu
 */
bool sendBuf(char *buf, int size);

/**
 * @param pid pid katsayılarını içeren array
 * indexler:
 * 0 -> Px
 * 1 -> Ix
 * 2 -> Dx
 * 3 -> Py
 * 4 -> Iy
 * 5 -> Dy
 */
bool sendPID(float* pid);
/**
 */
bool getCoordinates(int16_t* x, int16_t* y, float* servo_x, float* servo_y);
/**
 */
bool sendSetpoints(float x, float y);
#endif //Serial.h
