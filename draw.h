//
// Created by jona on 05.07.2023.
//

#ifndef FLATPACK_V3_DRAW_H
#define FLATPACK_V3_DRAW_H

#include <Arduino.h>
#include "U8g2_for_TFT_eSPI.h"

class TextWidget{
public:
    TextWidget(U8g2_for_TFT_eSPI *u8f_, uint16_t foreground_color_, uint16_t background_color_, int16_t pos_x_, int16_t pos_y_, const uint8_t *font_);
    void update(char new_string[]);
    void update_color(uint16_t new_foreground_color);

private:
    uint16_t foreground_color;
    uint16_t background_color;
    int16_t pos_x;
    int16_t pos_y;
    const uint8_t *font;
    U8g2_for_TFT_eSPI *u8f;

    char prev_string[8];

    void draw(char string[]);
    void remove();
};

class GridTextWidget: public TextWidget{
public:
    static int display_width;
    static int left_margin_;
    static int label_height_;
    static int vert_spacing_;
    static int bottom_margin_;

    GridTextWidget(U8g2_for_TFT_eSPI *u8f, uint16_t foreground_color_, uint16_t background_color_,
                   uint8_t pos_x_, uint8_t pos_y_, const uint8_t *font_);
};

#endif //FLATPACK_V3_DRAW_H
