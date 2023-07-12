#include "TFT_HX8347.h" // Hardware-specific library
#include <SPI.h>
#include "XPT2046_Touchscreen.h"
#include "U8g2_for_TFT_eSPI.h"
#include "FlatpackMCP.h"
#include "mcp_can.h"
#include "helpers.h"
#include "draw.h"

#define TFT_CS_PIN   10  //These two are set in User_Setup.h
#define TFT_DC_PIN   7   // only here for reference

#define CAN_CS_PIN 6
#define TOUCH_CS_PIN  4
#define BKL_PIN 9
#define ACTIVATE_PIN A0

#define DISPLAY_HEIGHT 320
#define DISPLAY_WIDTH 240

#define MAX_CURRENT 40
#define MAX_CELL_VOLTAGE 4.2
#define MIN_CELL_VOLTAGE 3.0
#define MAX_CELLS 18
#define MIN_CELLS 10

#define MIN_VOLTAGE_TOTAL 39.9
#define IDLE_CLIM 0

class AhCounter{
public:
    // use double, since the smallest increment of float at a value of 20 is 2e-6, but 1Ams is just 5e-6, so we might get truncation errors
    double Ah;

    void init();
    void update();

private:
    unsigned long int last_update;
};

XPT2046_Touchscreen ts(TOUCH_CS_PIN);

float currentSet = 5;
float cellVoltageSet = 4.20;
int cells = 15;
float voltageTotalSet = cells * cellVoltageSet;
AhCounter ah_counter;

bool active = false;

char tmp[12];
char tmp2[12];

MCP_CAN mcp_can(CAN_CS_PIN);
FlatpackMCP flatpack;

TFT_HX8347 tft = TFT_HX8347();
U8g2_for_TFT_eSPI u8f;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

  pinMode(ACTIVATE_PIN, INPUT_PULLUP);
  pinMode(BKL_PIN, OUTPUT);
  digitalWrite(BKL_PIN,1);

  ah_counter.init();

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
  flatpack.set_output(IDLE_CLIM, MIN_VOLTAGE_TOTAL);


  tft.fillScreen(TFT_BLACK);
  drawStatic();
}


const int left_margin = 15;
const int bottom_margin = 10;
const int vert_spacing = 50;
const int label_height = 40;
const int temp_height = 40;
const int status_square_width = 80;
const int issue_y_pos = label_height + vert_spacing * 4 + temp_height + 1;

int GridTextWidget::display_width = DISPLAY_WIDTH;
int GridTextWidget::left_margin_ = left_margin;
int GridTextWidget::bottom_margin_ = bottom_margin;
int GridTextWidget::vert_spacing_ = vert_spacing;
int GridTextWidget::label_height_ = label_height;

void drawStatic() {
  tft.drawFastVLine(DISPLAY_WIDTH / 2, 0, label_height+vert_spacing*4, TFT_DARKGREY);
  tft.drawFastVLine(status_square_width, label_height+vert_spacing*4, temp_height, TFT_DARKGREY);
  tft.drawFastHLine(0, label_height, DISPLAY_WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, label_height+vert_spacing, DISPLAY_WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, label_height+vert_spacing*2, DISPLAY_WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, label_height+vert_spacing*3, DISPLAY_WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, label_height+vert_spacing*4, DISPLAY_WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, issue_y_pos-1, DISPLAY_WIDTH, TFT_DARKGREY);

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

  if(flatpack.warnings.issueBits.output_voltage_low || flatpack.alarms.issueBits.output_voltage_low){
    VoltageMesWid.update_color(TFT_RED);
  }else if(flatpack.state == FLATPACK_STATE_WALKIN){
    VoltageMesWid.update_color(TFT_CYAN);
  }else{
    VoltageMesWid.update_color(TFT_WHITE);
  }
  CurrentMesWid.update_color(flatpack.warnings.issueBits.current_limit ? TFT_CYAN : TFT_WHITE);

  CurrentMesWid.update(strcat(dtostrf(flatpack.meas_current,0,1,tmp), "A"));
  CurrentSetWid.update(strcat(dtostrf(currentSet,0,1,tmp), "A"));
  CellVoltageMesWid.update(strcat(dtostrf(flatpack.meas_voltage/cells,0,2,tmp), "V"));
  CellVoltageSetWid.update(strcat(dtostrf(cellVoltageSet, 0, 2, tmp), "V"));
  VoltageSetWid.update(strcat(dtostrf(voltageTotalSet,0,1,tmp), "V"));
  VoltageMesWid.update(strcat(dtostrf(flatpack.meas_voltage,0,1,tmp), "V"));
  CellsWid.update(strcat(dtostrf(cells,0,0,tmp), "S"));
  AhWid.update(strcat(dtostrf(ah_counter.Ah,0,1,tmp), "Ah"));

  static TextWidget TempWid(&u8f, TFT_WHITE, TFT_BLACK, 100, 270, u8g2_font_logisoso18_tf);
  static TextWidget VACWid(&u8f, TFT_WHITE, TFT_BLACK, 15, 270, u8g2_font_logisoso18_tf);

  bool bad_temp = flatpack.warnings.issueBits.high_temp || flatpack.warnings.issueBits.low_temp || flatpack.alarms.issueBits.high_temp || flatpack.alarms.issueBits.low_temp;
  TempWid.update_color(bad_temp ? TFT_RED : TFT_WHITE);
  bool bad_mains = flatpack.warnings.issueBits.high_mains || flatpack.warnings.issueBits.low_mains || flatpack.alarms.issueBits.high_mains || flatpack.alarms.issueBits.low_mains;
  VACWid.update_color(bad_mains ? TFT_RED : TFT_WHITE);

  TempWid.update(strcat(strcat(strcat(strcat(dtostrf(flatpack.temp_in,0,0,tmp), "°C"), "->"), dtostrf(flatpack.temp_out, 0, 0, tmp2)), "°C"));
  VACWid.update(strcat(dtostrf(flatpack.voltage_in,0,0,tmp), "V"));
}


void getTouch() {
  if(active){
    return;
  }

  TS_Point p = ts.getPoint();
  Serial.print(p.z);
  Serial.print(", ");
  Serial.print(p.x);
  Serial.print(", ");
  Serial.println(p.y);

  if (p.z > 800) {
    if (p.x > 2900 and p.y < 700) {
      if (currentSet < MAX_CURRENT) {
        currentSet++;
      }
      else {
        currentSet = 1;
      }
    }
    if (p.x > 2900 and p.y > 700) {
      if (currentSet <= 1) {
        currentSet = MAX_CURRENT;
      }
      else {
        currentSet--;
      }
    }
    if (p.x < 2800 and p.x > 2300 and p.y < 700) {
      if (cellVoltageSet < MAX_CELL_VOLTAGE) {
        cellVoltageSet = cellVoltageSet + 0.05;
      }
      else {
        cellVoltageSet = MAX_CELL_VOLTAGE;
      }
    }
    if (p.x < 2800 and p.x > 2300 and p.y > 700) {
      if (cellVoltageSet <= MIN_CELL_VOLTAGE) {
        cellVoltageSet = MIN_CELL_VOLTAGE;
      }
      else {
        cellVoltageSet = cellVoltageSet - 0.05;
      }
    }

    if (p.x < 2100 and p.y < 1100) {
      if (cells < MAX_CELLS) {
        cells ++;
      }
      else {
        cells = MAX_CELLS;
      }
    }
    if (p.x < 2100 and p.y > 1100) {
      if (cells <= MIN_CELLS) {
        cells = MIN_CELLS;
      }
      else {
        cells --;
      }
    }
  }
}

class IssueShower {
public:
    const int width;
    int issue_line_len = 0;
    int issue_line_num = 0;
    int start_point_x;
    int start_point_y;
    int line_height;

    IssueShower(int start_point_x_, int start_point_y_, int width_, int height) : width(width_), start_point_x(start_point_x_), start_point_y(start_point_y_) {
      tft.fillRect(start_point_x, start_point_y, width, height, TFT_BLACK);

      u8f.setFont(u8g2_font_mozart_nbp_tr);
      line_height = u8f.getFontAscent() - u8f.getFontDescent() + 2;

      u8f.setCursor(start_point_x, start_point_y + line_height);
      u8f.setForegroundColor(TFT_WHITE);
    }

    void show_issue(bool condition, const char *message) {
      if (condition) {
        int msg_len = u8f.getUTF8Width(message);
        issue_line_len += msg_len;

        if (issue_line_len > width) {
          issue_line_num++;
          u8f.setCursor(start_point_x, start_point_y + line_height * (issue_line_num+1));
          issue_line_len = msg_len;
        }
        u8f.print(message);
      }
    }
};

void show_issues() {
  IssueShower issues(status_square_width+1, issue_y_pos, DISPLAY_WIDTH - status_square_width, DISPLAY_HEIGHT-issue_y_pos);

  issues.show_issue(flatpack.warnings.issueBits.current_limit, " (W) CC");
  issues.show_issue(flatpack.warnings.issueBits.fan1_speed_low, " (W) Fan1 Slow");
  issues.show_issue(flatpack.warnings.issueBits.output_voltage_low, " (W) Out Low");
  issues.show_issue(flatpack.warnings.issueBits.fan3_speed_low, " (W) Fan3 Slow");
  issues.show_issue(flatpack.warnings.issueBits.low_mains, " (W) Low Mains");
  issues.show_issue(flatpack.warnings.issueBits.high_mains, " (W) High Mains");
  issues.show_issue(flatpack.warnings.issueBits.low_temp, " (W) Low Temp");
  issues.show_issue(flatpack.warnings.issueBits.high_temp, " (W) High Temp");
  issues.show_issue(flatpack.warnings.issueBits.inner_volt, " (W) Inner Volt");
  issues.show_issue(flatpack.warnings.issueBits.internal_voltage, " (W) Int Volt");
  issues.show_issue(flatpack.warnings.issueBits.mod_fail_primary, " (W) Mod Fail Prim");
  issues.show_issue(flatpack.warnings.issueBits.mod_fail_secondary, " (W) Mod Fail Sec");
  issues.show_issue(flatpack.warnings.issueBits.mod_fail_secondary_2, " (W) Mod Fail Sec2");
  issues.show_issue(flatpack.warnings.issueBits.module_fail, " (W) Mod Fail");
  issues.show_issue(flatpack.warnings.issueBits.ovs_lockout, " (W) OVS Lock");
  issues.show_issue(flatpack.warnings.issueBits.sub_mod1_fail, " (W) Sub Mod Fail");
  issues.show_issue(flatpack.alarms.issueBits.current_limit, " (A) CC");
  issues.show_issue(flatpack.alarms.issueBits.fan1_speed_low, " (A) Fan1 Slow");
  issues.show_issue(flatpack.alarms.issueBits.output_voltage_low, " (A) Out Low");
  issues.show_issue(flatpack.alarms.issueBits.fan3_speed_low, " (A) Fan3 Slow");
  issues.show_issue(flatpack.alarms.issueBits.low_mains, " (A) Low Mains");
  issues.show_issue(flatpack.alarms.issueBits.high_mains, " (A) High Mains");
  issues.show_issue(flatpack.alarms.issueBits.low_temp, " (A) Low Temp");
  issues.show_issue(flatpack.alarms.issueBits.high_temp, " (A) High Temp");
  issues.show_issue(flatpack.alarms.issueBits.inner_volt, " (A) Inner Volt");
  issues.show_issue(flatpack.alarms.issueBits.internal_voltage, " (A) Int Volt");
  issues.show_issue(flatpack.alarms.issueBits.mod_fail_primary, " (A) Mod Fail Prim");
  issues.show_issue(flatpack.alarms.issueBits.mod_fail_secondary, " (A) Mod Fail Sec");
  issues.show_issue(flatpack.alarms.issueBits.mod_fail_secondary_2, " (A) Mod Fail Sec2");
  issues.show_issue(flatpack.alarms.issueBits.module_fail, " (A) Mod Fail");
  issues.show_issue(flatpack.alarms.issueBits.ovs_lockout, " (A) OVS Lock");
  issues.show_issue(flatpack.alarms.issueBits.sub_mod1_fail, " (A) Sub Mod Fail");
}

void update_status(){
  FlatpackIssue onlyConstantCurrent = {0};
  onlyConstantCurrent.issueBits.current_limit = 1;

  uint16_t status_color = TFT_GREEN;
  if(flatpack.state == FLATPACK_STATE_ALARM){
    status_color = TFT_RED;
  } else if(flatpack.state == FLATPACK_STATE_WARNING
            && (flatpack.warnings.canBytes.can_byte_1 != onlyConstantCurrent.canBytes.can_byte_1
                || flatpack.warnings.canBytes.can_byte_2 != onlyConstantCurrent.canBytes.can_byte_2)){
    status_color = TFT_ORANGE;
  } else if (active){
    status_color = TFT_CYAN;
  }
  static uint16_t last_status_color = 0;
  if(last_status_color != status_color) {
    tft.fillRoundRect(5, issue_y_pos + 1, status_square_width - 10, DISPLAY_HEIGHT - issue_y_pos - 2, 10, status_color);
    last_status_color = status_color;
  }


  static FlatpackIssue prevWarnings = {1,1,1,1,1,1,1,1,1,1,1};
  static FlatpackIssue prevAlarms;

  if(flatpack.warnings.canBytes.can_byte_1 == prevWarnings.canBytes.can_byte_1
     && flatpack.warnings.canBytes.can_byte_2 == prevWarnings.canBytes.can_byte_2
     && flatpack.alarms.canBytes.can_byte_1 == prevAlarms.canBytes.can_byte_1
     && flatpack.alarms.canBytes.can_byte_2 == prevAlarms.canBytes.can_byte_2){
    return;
  }
  prevWarnings = flatpack.warnings;
  prevAlarms = flatpack.alarms;

  //recovery from an alarm sometimes causes the flatpack to forget its voltage
  set_output();

  show_issues();
}

void getActivation(){
  const unsigned int activate_delay = 1000;
  const unsigned int debounce_period = 100;

  static bool last_state = false;
  static unsigned long int last_change = 0;
  static bool press_processed = false;

  bool state = !digitalRead(ACTIVATE_PIN);

  if(state){
    if(!last_state){
      last_change = millis();
      last_state = true;
    }
    if(!press_processed){
      if(active){
        active = false;
        set_output();
        press_processed = true;
      }else if(millis() - last_change > activate_delay){
        active = true;
        set_output();
        press_processed = true;
      }
    }
  }else{
    if(last_state){
      last_change = millis();
      last_state = false;
    }
    if(press_processed && millis() - last_change > debounce_period){
      press_processed = false;
    }
  }
}

void set_output(){
  float voltage = active ? voltageTotalSet : MIN_VOLTAGE_TOTAL;
  float current = active ? currentSet : IDLE_CLIM;

  // for full range use 48V mode below 66 and 60V mode above 66. Never use the stupid 48V NiCd mode
  flatpack.over_voltage_protection = voltage < 66 ? 70 : 80;
  flatpack.set_output(current, voltage);
}

void set_output_sometimes(){
  //just in case the supply somehow forgets its settings
  const unsigned int period = 1000;

  static unsigned long int last_set = 0;
  if(millis() - last_set > period){
    set_output();
    last_set = millis();
  }
}

void AhCounter::init(){
  last_update = millis();
}

void AhCounter::update(){
  unsigned int period = millis() - last_update;
  last_update = millis();

  Ah += period * (double) flatpack.meas_current / 1000 / 60 / 60;
}

void loop() {
  flatpack.update();
  getTouch();
  getActivation();
  set_output_sometimes();
  ah_counter.update();

  voltageTotalSet = cells * cellVoltageSet;

  updateWidgets();
  update_status();

  delay(10);
}
