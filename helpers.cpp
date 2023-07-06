//
// Created by jona on 02.07.2023.
//
#include "helpers.h"


void print_hex_array_serial(uint8_t array[], int len) {
  for(int i = 0; i<len; i++)
  {
    if( array[i] < 0x10){ Serial.print("0");} Serial.print(array[i],HEX);
    Serial.print(" ");
  }
}

void print_hex_array_tft(uint8_t array[], int len,U8g2_for_TFT_eSPI *u8f) {
  for(int i = 0; i<len; i++)
  {
    if( array[i] < 0x10){ u8f->print("0");} u8f->print(array[i], HEX);
    u8f->print(" ");
  }
}