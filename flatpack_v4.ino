#include "TFT_HX8347.h" // Hardware-specific library
#include <SPI.h>
#include "XPT2046_Touchscreen.h"
#include "U8g2_for_TFT_eSPI.h"
#include "FlatpackMCP.h"
#include "mcp_can.h"
#include "helpers.h"
#include "draw.h"

#define CAN_CS_PIN 6
#define TOUCH_CS_PIN  4
#define BKL_PIN 9

#define DISPLAY_HEIGHT 320
#define DISPLAY_WIDTH 240

#define MAX_CURRENT 40
#define MAX_VOLTAGE 4.2
#define MIN_VOLTAGE 3.0
#define MAX_CELLS 15
#define MIN_CELLS 10

XPT2046_Touchscreen ts(TOUCH_CS_PIN);

float currentSet = 5;
float cellVoltageSet = 4.20;
int cells = 15;
float voltageTotalSet = cells * cellVoltageSet;
float Ah = 20.50;

char tmp[12];

MCP_CAN mcp_can(CAN_CS_PIN);
FlatpackMCP flatpack;

TFT_HX8347 tft = TFT_HX8347();
U8g2_for_TFT_eSPI u8f;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

  pinMode(BKL_PIN, OUTPUT);
  digitalWrite(BKL_PIN,1);

  tft.init();
  u8f.begin(tft);

  ts.begin();
  ts.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  u8f.setFontMode(1); //transparent
  u8f.setFontDirection(0);
  u8f.setForegroundColor(TFT_WHITE);
  u8f.setFont(u8g2_font_6x10_mr);

  flatpack.can_driver = &mcp_can; //must be set before any library function
  flatpack.id = 1; //must be set before first calling update(), cannot be changed later
  flatpack.over_voltage_protection = 55; //must be set before first calling set_output, gets written to device with every set_output
  flatpack.walkin = FLATPACK_WALKIN_QUICK; //default is FLATPACK_WALKIN_QUICK, gets written to device with every set_output

  u8f.println();
  Serial.println("initializing MCP");
  u8f.println("initializing MCP");
  while(!flatpack.init_MCP());
  Serial.println(u8f.getFontAscent());
  Serial.println(u8f.getFontDescent());

  Serial.println("looking for flatpack");
  u8f.println("looking for flatpack");
  while(!flatpack.discover()); //can also set the serial manually instead of calling this

  Serial.print("found flatpack with serial ");
  u8f.println("found flatpack with serial ");
  print_hex_array_serial(flatpack.serial, 6);
  print_hex_array_tft(flatpack.serial, 6, &u8f);
  Serial.println("");

  flatpack.update(); //must be called at least once before calling set_output
  flatpack.set_output(0, 42);


  tft.fillScreen(TFT_BLACK);
  drawStatic();
}


const int left_margin = 20;
const int bottom_margin = 15;
const int vert_spacing = 60;
const int label_height = 40;

int GridTextWidget::display_width = DISPLAY_WIDTH;
int GridTextWidget::left_margin_ = left_margin;
int GridTextWidget::bottom_margin_ = bottom_margin;
int GridTextWidget::vert_spacing_ = vert_spacing;
int GridTextWidget::label_height_ = label_height;

void drawStatic() {
  tft.drawFastVLine(DISPLAY_WIDTH / 2, 0, DISPLAY_HEIGHT, TFT_CYAN);
  tft.drawFastHLine(0, label_height, DISPLAY_WIDTH, TFT_CYAN);
  tft.drawFastHLine(0, label_height+vert_spacing, DISPLAY_WIDTH, TFT_CYAN);
  tft.drawFastHLine(0, label_height+vert_spacing*2, DISPLAY_WIDTH, TFT_CYAN);
  tft.drawFastHLine(0, label_height+vert_spacing*3, DISPLAY_WIDTH, TFT_CYAN);
  tft.drawFastHLine(0, label_height+vert_spacing*4, DISPLAY_WIDTH, TFT_CYAN);

  u8f.setFont(u8g2_font_logisoso18_tr);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall

  u8f.setForegroundColor(TFT_WHITE);
  u8f.setCursor(40, 28);
  u8f.print("Set");
  u8f.setCursor(15+DISPLAY_WIDTH/2, 28);
  u8f.print("Measured");
}

void updateWidgets(){
  static GridTextWidget CurrentMesWid(&u8f, TFT_WHITE, TFT_BLACK, 1, 0, u8g2_font_logisoso26_tr);
  static GridTextWidget CurrentSetWid(&u8f,TFT_WHITE, TFT_BLACK, 0, 0, u8g2_font_logisoso26_tr);
  static GridTextWidget CellVoltageMesWid(&u8f,TFT_WHITE, TFT_BLACK, 1, 1, u8g2_font_logisoso26_tr);
  static GridTextWidget CellVoltageSetWid(&u8f,TFT_WHITE, TFT_BLACK, 0, 1, u8g2_font_logisoso26_tr);
  static GridTextWidget VoltageSetWid(&u8f,TFT_WHITE, TFT_BLACK, 0, 2, u8g2_font_logisoso26_tr);
  static GridTextWidget VoltageMesWid(&u8f,TFT_WHITE, TFT_BLACK, 1, 2, u8g2_font_logisoso26_tr);
  static GridTextWidget CellsWid(&u8f,TFT_WHITE, TFT_BLACK, 0, 3, u8g2_font_logisoso26_tr);
  static GridTextWidget AhWid(&u8f,TFT_WHITE, TFT_BLACK, 1, 3, u8g2_font_logisoso26_tr);

  CurrentMesWid.update(strcat(dtostrf(flatpack.meas_current,0,1,tmp), "A"));
  CurrentSetWid.update(strcat(dtostrf(currentSet,0,1,tmp), "A"));
  CellVoltageMesWid.update(strcat(dtostrf(flatpack.meas_voltage/cells,0,2,tmp), "V"));
  CellVoltageSetWid.update(strcat(dtostrf(cellVoltageSet, 0, 2, tmp), "V"));
  VoltageSetWid.update(strcat(dtostrf(voltageTotalSet,0,1,tmp), "V"));
  VoltageMesWid.update(strcat(dtostrf(flatpack.meas_voltage,0,1,tmp), "V"));
  CellsWid.update(strcat(dtostrf(cells,0,0,tmp), "S"));
  AhWid.update(strcat(dtostrf(Ah,0,1,tmp), "Ah"));

  static TextWidget TempWid(&u8f, TFT_WHITE, TFT_BLACK, 140, 312, u8g2_font_logisoso18_tf);
  static TextWidget VACWid(&u8f, TFT_WHITE, TFT_BLACK, 20, 312, u8g2_font_logisoso18_tf);

  TempWid.update(strcat(dtostrf(flatpack.temp_out,0,0,tmp), "°C"));
  VACWid.update(strcat(dtostrf(flatpack.voltage_in,0,0,tmp), "V"));
}


void getTouch() {

  TS_Point p = ts.getPoint();
  Serial.print(p.z);
  Serial.print(", ");
  Serial.print(p.x);
  Serial.print(", ");
  Serial.println(p.y);

  if (p.z > 800) {
    if (p.x > 2800 and p.y < 700) {
      if (currentSet < MAX_CURRENT) {
        currentSet++;
      }
      else {
        currentSet = 1;
      }
    }
    if (p.x > 2800 and p.y > 700) {
      if (currentSet <= 1) {
        currentSet = MAX_CURRENT;
      }
      else {
        currentSet--;
      }
    }
    if (p.x < 2600 and p.x > 2000 and p.y < 700) {
      if (cellVoltageSet < MAX_VOLTAGE) {
        cellVoltageSet = cellVoltageSet + 0.05;
      }
      else {
        cellVoltageSet = MAX_VOLTAGE;
      }
    }
    if (p.x < 2600 and p.x > 2000 and p.y > 700) {
      if (cellVoltageSet <= MIN_VOLTAGE) {
        cellVoltageSet = MIN_VOLTAGE;
      }
      else {
        cellVoltageSet = cellVoltageSet - 0.05;
      }
    }

    if (p.x < 1900 and p.y < 1100) {
      if (cells < MAX_CELLS) {
        cells ++;
      }
      else {
        cells = MAX_CELLS;
      }
    }
    if (p.x < 1900 and p.y > 1100) {
      if (cells <= MIN_CELLS) {
        cells = MIN_CELLS;
      }
      else {
        cells --;
      }
    }
  }
}

int issue_line_len = 0;
const int max_issue_line_len = 180;
void show_issue(bool condition, const char *message) {
  if (condition) {
    int msg_len = u8f.getUTF8Width(message);
    issue_line_len += msg_len;

    if(issue_line_len > max_issue_line_len) {
      tft.println();
      issue_line_len = msg_len;
    }
    tft.print(message);
  }
}

void show_issues() {
  static FlatpackIssue prevWarnings;
  static FlatpackIssue prevAlarms;

  if(flatpack.warnings.canBytes.can_byte_1 == prevWarnings.canBytes.can_byte_1
  && flatpack.warnings.canBytes.can_byte_2 == prevWarnings.canBytes.can_byte_2
  && flatpack.alarms.canBytes.can_byte_1 == prevAlarms.canBytes.can_byte_1
  && flatpack.alarms.canBytes.can_byte_2 == prevAlarms.canBytes.can_byte_2){
    return;
  }
  prevWarnings = flatpack.warnings;
  prevAlarms = flatpack.alarms;

  tft.fillRect(0, 300, 40, 40, TFT_BLACK);

  u8f.setCursor(0, 300);
  u8f.setFont(u8g2_font_mozart_nbp_tr);
  issue_line_len = 0;

  show_issue(flatpack.warnings.issueBits.current_limit, " W CC");
  show_issue(flatpack.warnings.issueBits.fan1_speed_low, " W Fan1 Slow");
  show_issue(flatpack.warnings.issueBits.output_voltage_low, " W Out Low");
  show_issue(flatpack.warnings.issueBits.fan3_speed_low, " W Fan3 Slow");
  show_issue(flatpack.warnings.issueBits.low_mains, " W Low Mains");
  show_issue(flatpack.warnings.issueBits.high_mains, " W High Mains");
  show_issue(flatpack.warnings.issueBits.low_temp, " W Low Temp");
  show_issue(flatpack.warnings.issueBits.high_temp, " W High Temp");
  show_issue(flatpack.warnings.issueBits.inner_volt, " W Inner Volt");
  show_issue(flatpack.warnings.issueBits.internal_voltage, " W Int Volt");
  show_issue(flatpack.warnings.issueBits.mod_fail_primary, " W Mod Fail Prim");
  show_issue(flatpack.warnings.issueBits.mod_fail_secondary, " W Mod Fail Sec");
  show_issue(flatpack.warnings.issueBits.mod_fail_secondary_2, " W Mod Fail Sec2");
  show_issue(flatpack.warnings.issueBits.module_fail, " W Mod Fail");
  show_issue(flatpack.warnings.issueBits.ovs_lockout, " W OVS Lock");
  show_issue(flatpack.warnings.issueBits.sub_mod1_fail, " W Sub Mod Fail");
  show_issue(flatpack.alarms.issueBits.current_limit, " A CC");
  show_issue(flatpack.alarms.issueBits.fan1_speed_low, " A Fan1 Slow");
  show_issue(flatpack.alarms.issueBits.output_voltage_low, " A Out Low");
  show_issue(flatpack.alarms.issueBits.fan3_speed_low, " A Fan3 Slow");
  show_issue(flatpack.alarms.issueBits.low_mains, " A Low Mains");
  show_issue(flatpack.alarms.issueBits.high_mains, " A High Mains");
  show_issue(flatpack.alarms.issueBits.low_temp, " A Low Temp");
  show_issue(flatpack.alarms.issueBits.high_temp, " A High Temp");
  show_issue(flatpack.alarms.issueBits.inner_volt, " A Inner Volt");
  show_issue(flatpack.alarms.issueBits.internal_voltage, " A Int Volt");
  show_issue(flatpack.alarms.issueBits.mod_fail_primary, " A Mod Fail Prim");
  show_issue(flatpack.alarms.issueBits.mod_fail_secondary, " A Mod Fail Sec");
  show_issue(flatpack.alarms.issueBits.mod_fail_secondary_2, " A Mod Fail Sec2");
  show_issue(flatpack.alarms.issueBits.module_fail, " A Mod Fail");
  show_issue(flatpack.alarms.issueBits.ovs_lockout, " A OVS Lock");
  show_issue(flatpack.alarms.issueBits.sub_mod1_fail, " A Sub Mod Fail");
}

void loop() {
  flatpack.update();
  getTouch();
  voltageTotalSet = cells * cellVoltageSet;

  updateWidgets();
  show_issues();

  delay(10);
}
