//
// Created by jona on 02.07.2023.
//

#ifndef FLATPACK_V3_FLATPACKMCP_H
#define FLATPACK_V3_FLATPACKMCP_H

#include <Arduino.h>
#include "mcp_can.h"

typedef enum {
    FLATPACK_WALKIN_QUICK = 0x04, //should be like 5s
    FLATPACK_WALKIN_SLOW = 0x05   //reported to be 60s, more like 100s for me
} FlatpackWalkin;

typedef enum {
    FLATPACK_STATE_NORMAL = 0x04,
    FLATPACK_STATE_WARNING = 0x08,
    FLATPACK_STATE_ALARM = 0x0C,
    FLATPACK_STATE_WALKIN = 0x10
} FlatpackState;

typedef enum {
    FLATPACK_ERROR = 0,
    FLATPACK_OK = 1,
    FLATPACK_NEW_DATA = 2
} FlatpackResult;

union FlatpackIssue{
    struct{
      unsigned int ovs_lockout : 1;
      unsigned int mod_fail_primary : 1;
      unsigned int mod_fail_secondary : 1;
      unsigned int high_mains : 1;
      unsigned int low_mains : 1;
      unsigned int high_temp : 1;
      unsigned int low_temp : 1;
      unsigned int current_limit : 1;
      unsigned int internal_voltage : 1;
      unsigned int module_fail : 1;
      unsigned int mod_fail_secondary_2 : 1;
      unsigned int fan1_speed_low : 1;
      unsigned int output_voltage_low : 1; //supposed to be fan2_speed_low, but testing suggests it's output low on my model
      unsigned int sub_mod1_fail : 1;
      unsigned int fan3_speed_low : 1;
      unsigned int inner_volt : 1;
    } issueBits;
    struct{
        uint8_t can_byte_1 : 8;
        uint8_t can_byte_2 : 8;
    } canBytes;
};


class FlatpackMCP{
public:
    uint8_t serial[6];
    uint8_t id;

    //these next two get sent to the supply whenever set_output is called
    float over_voltage_protection;
    uint8_t walkin;

    FlatpackState state;
    FlatpackIssue warnings;
    FlatpackIssue alarms;
    float meas_voltage;
    float meas_current;
    int voltage_in;
    int temp_in;
    int temp_out;

    MCP_CAN* can_driver;

    FlatpackMCP();
    FlatpackResult init_MCP() const;
    FlatpackResult discover();
    FlatpackResult update(unsigned long int timeout=200);
    FlatpackResult set_output(float current_limit, float target_voltage,
                   float voltage_measured=-1) const;
    FlatpackResult set_default_voltage(float voltage) const;

private:
    unsigned long last_login;
    bool logged_in;

    int login();
    void parse_status(const uint8_t buf[]);
    FlatpackResult get_diagnostics(uint8_t type, unsigned long int timeout);
};


#endif //FLATPACK_V3_FLATPACKMCP_H
