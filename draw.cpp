//
// Created by jona on 05.07.2023.
//

#include "draw.h"
#include "U8g2_for_TFT_eSPI.h"

TextWidget::TextWidget(U8g2_for_TFT_eSPI *u8f_, uint16_t foreground_color_, uint16_t background_color_, int16_t pos_x_, int16_t pos_y_,
                       const uint8_t *font_) : foreground_color(foreground_color_), background_color(background_color_), pos_x(pos_x_), pos_y(pos_y_), font(font_), u8f(u8f_), prev_string() {
}

void TextWidget::update(char *new_string) {
  if (strcmp(new_string, prev_string) != 0) {
    u8f->setFont(font);

    u8f->setForegroundColor(background_color);
    u8f->setCursor(pos_x, pos_y);
    u8f->print(prev_string);
    u8f->setForegroundColor(foreground_color);
    u8f->setCursor(pos_x, pos_y);
    u8f->print(new_string);

    strcpy(prev_string, new_string);
  }
}

GridTextWidget::GridTextWidget(U8g2_for_TFT_eSPI *u8f_, uint16_t foreground_color_, uint16_t background_color_,
                               uint8_t pos_x_, uint8_t pos_y_, const uint8_t *font_) : TextWidget(u8f_, foreground_color_, background_color_, display_width/2*pos_x_ + left_margin_, label_height_ + vert_spacing_ * (pos_y_ + 1) - bottom_margin_, font_) {
}
