//
// Created by jona on 02.07.2023.
//

#include "FlatpackMCP.h"


const uint32_t tx_login = 0x05004800; // 0x050048XX XX is ID << 2
const uint32_t tx_set_out = 0x05FF4000; // 0x05FF40XX XX is walkin
const uint32_t tx_set_default_volt = 0x05009C00; // 0x05XX9C00 XX is ID
const uint32_t tx_diagnostic_request = 0x0500BFFC; // 0x05XXBFFC XX is ID

const uint32_t rx_can_intro = 0x05000000; // 0x0500XXYY XXYY is last 14bits of serial
const uint32_t rx_status = 0x05004000; // 0x05XX40YY XX is ID, YY is state
const uint32_t rx_login_request = 0x05004400; // 0x05XX4400 XX is ID
const uint32_t rx_diagnostic_response = 0x0500BFFC; // 0x05XXBFFC XX is ID

typedef enum{
    DIAGNOSTIC_WARNINGS = 0x04,
    DIAGNOSTIC_ALARMS = 0x08
} DiagnosticType;

const unsigned long login_period = 10000;


long unsigned int msg_id;
uint8_t len = 0;
uint8_t buf[8];
uint8_t id_ext;

FlatpackMCP::FlatpackMCP() : serial(), id(1), over_voltage_protection(), walkin(FLATPACK_WALKIN_QUICK), last_login(0), logged_in(false), state(), warnings(), alarms(), meas_voltage(), meas_current(), voltage_in(), temp_in(), temp_out(), can_driver(){

}

FlatpackResult FlatpackMCP::init_MCP() const{
  if(CAN_OK != can_driver->begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ)){
    return FLATPACK_ERROR;
  }
  can_driver->setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted

  return FLATPACK_OK;
}

FlatpackResult FlatpackMCP::discover() {
  if(CAN_MSGAVAIL != can_driver->checkReceive()) {
    return FLATPACK_ERROR;
  }
  can_driver->readMsgBuf(&msg_id, &id_ext, &len, buf);

  if((msg_id & ~0b111111111111UL) == rx_can_intro){
    for(int serdex = 0; serdex < 6; serdex++){
      serial[serdex] = buf[serdex + 1];
    }
    return FLATPACK_OK;
  }
  if((msg_id & ~(0xFFUL << 16)) == rx_login_request){
    for(int serdex = 0; serdex < 6; serdex++){
      serial[serdex] = buf[serdex];
    }
    return FLATPACK_OK;
  }
  return FLATPACK_ERROR;
}

FlatpackResult FlatpackMCP::update(unsigned long int timeout) {
  if(!logged_in || millis() - last_login > login_period){
    if(CAN_OK != login()){
      return FLATPACK_ERROR;
    }
    last_login = millis();
    logged_in = true;
  }

  FlatpackResult return_status = FLATPACK_OK;

  while(CAN_MSGAVAIL == can_driver->checkReceive()) {
    can_driver->readMsgBuf(&msg_id, &id_ext, &len, buf);
    if((msg_id & ~0xFFUL) != (rx_status | (uint32_t) id << 16)){
      continue;
    }
    state = static_cast<FlatpackState>((uint8_t) msg_id);
    parse_status(buf);
    return_status = FLATPACK_NEW_DATA;
  }
  if(return_status == FLATPACK_NEW_DATA){
    get_diagnostics(DIAGNOSTIC_WARNINGS, timeout);
    get_diagnostics(DIAGNOSTIC_ALARMS, timeout);
  }
  return return_status;
}

int FlatpackMCP::login() {
  uint32_t login_id = tx_login | id << 2;

  for(int serdex = 0; serdex < 6; serdex++){
    buf[serdex] = serial[serdex];
  }
  buf[6] = 0;
  buf[7] = 0;

  return can_driver->sendMsgBuf(login_id, 1, 8, buf);
}

void FlatpackMCP::parse_status(const uint8_t buf[8]) {
  temp_in = buf[0];

  uint16_t current = buf[1] + ((uint16_t) buf[2] << 8);
  meas_current = (float) current / 10;

  uint16_t voltage = buf[3] + ((uint16_t) buf[4] << 8);
  meas_voltage = (float) voltage / 100;

  voltage_in = buf[5] + ((uint16_t) buf[6] << 8);

  temp_out = buf[7];
}

FlatpackResult FlatpackMCP::set_output(float current_limit, float target_voltage,
                            float voltage_measured) const {
  if(over_voltage_protection < target_voltage){
    return FLATPACK_ERROR;
  }
  if(voltage_measured == -1){
    voltage_measured = target_voltage;
  }

  uint16_t cur_lim = (uint16_t) current_limit * 10;
  buf[0] = lowByte(cur_lim);
  buf[1] = highByte(cur_lim);

  uint16_t m_volt = (uint16_t) voltage_measured * 100;
  buf[2] = lowByte(m_volt);
  buf[3] = highByte(m_volt);

  uint16_t t_volt = (uint16_t) target_voltage * 100;
  buf[4] = lowByte(t_volt);
  buf[5] = highByte(t_volt);

  uint16_t ovp = (uint16_t) over_voltage_protection * 100;
  buf[6] = lowByte(ovp);
  buf[7] = highByte(ovp);

  uint32_t set_out_id = tx_set_out | walkin;
  return static_cast<FlatpackResult>(can_driver->sendMsgBuf(set_out_id, 1, 8, buf) == CAN_OK);
}

FlatpackResult FlatpackMCP::set_default_voltage(float voltage) const {

  buf[0] = 0x29;
  buf[1] = 0x15;
  buf[2] = 0x00;

  uint16_t d_volt = (uint16_t) voltage * 100;
  buf[3] = lowByte(d_volt);
  buf[4] = highByte(d_volt);

  uint32_t set_default_id = tx_set_default_volt | ((uint32_t) id << 16);

  return static_cast<FlatpackResult>(can_driver->sendMsgBuf(set_default_id, 1, 5, buf) == CAN_OK);
}

FlatpackResult FlatpackMCP::get_diagnostics(uint8_t type, unsigned long int timeout) {
  unsigned long int start_time = millis();
  if(type == DIAGNOSTIC_WARNINGS && state != FLATPACK_STATE_WARNING && state != FLATPACK_STATE_ALARM){
    warnings.canBytes.can_byte_1 = 0;
    warnings.canBytes.can_byte_2 = 0;
    return FLATPACK_OK;
  }
  if(type == DIAGNOSTIC_ALARMS && state != FLATPACK_STATE_ALARM){
    alarms.canBytes.can_byte_1 = 0;
    alarms.canBytes.can_byte_2 = 0;
    return FLATPACK_OK;
  }

  buf[0] = 0x08;
  buf[1] = type;
  buf[2] = 0x00;

  uint32_t rq_diag_id = tx_diagnostic_request | ((uint32_t) id << 16);

  if(can_driver->sendMsgBuf(rq_diag_id, 1, 3, buf) != CAN_OK){
    return FLATPACK_ERROR;
  }

  while(true) {
    if(millis() - start_time > timeout){
      return FLATPACK_ERROR;
    }

    if (CAN_MSGAVAIL == can_driver->checkReceive()) {
      can_driver->readMsgBuf(&msg_id, &id_ext, &len, buf);
      if (msg_id == (rx_diagnostic_response | (uint32_t) id << 16)) {
        break;
      }
    }
  }
  if(type == DIAGNOSTIC_WARNINGS) {
    warnings.canBytes.can_byte_1 = buf[3];
    warnings.canBytes.can_byte_2 = buf[4];
  } else{
    alarms.canBytes.can_byte_1 = buf[3];
    alarms.canBytes.can_byte_2 = buf[4];
  }
  return FLATPACK_OK;
}
