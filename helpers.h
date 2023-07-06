//
// Created by jona on 02.07.2023.
//

#ifndef FLATPACK_V3_HELPERS_H
#define FLATPACK_V3_HELPERS_H

#include <Arduino.h>
#include "U8g2_for_TFT_eSPI.h"

void print_hex_array_serial(uint8_t array[], int len);
void print_hex_array_tft(uint8_t array[], int len, U8g2_for_TFT_eSPI *u8f);

#endif //FLATPACK_V3_HELPERS_H
