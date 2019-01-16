#include <Canbus.h>  // don't forget to include these
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <time.h>
#include "YarisSpeedDoorLock.h"

int last_pwr_update;
int data_ready_flag;
struct car_status status;

void setup() {
  pinMode(PIN_REL, OUTPUT);
  digitalWrite(PIN_REL, LOW);
  pinMode(PIN_LOC, OUTPUT);
  digitalWrite(PIN_LOC, LOW);
  pinMode(PIN_UNL, OUTPUT);
  digitalWrite(PIN_UNL, LOW);
  
  
  if (!Canbus.init(CANSPEED_500)) {
    halt();
  }

  //  Turn on the relay, i.e. source the device from +30
  digitalWrite(PIN_REL, HIGH);

  last_pwr_update = time(NULL);
  data_ready_flag = 0b000;
  
  delay(1000);
}

void loop() {
  tCAN frame;

  if (mcp2515_check_message()) {
    if (mcp2515_get_message(&frame)) {
      if (frame.id == CAN_ID_SPEED) {
        if (frame.data[CAN_BYTE_SPEED] >= LOCK_SPEED_THRESHOLD) {
          status.speed_above_threshold = TRUE;
        }
        else {
          status.speed_above_threshold = FALSE;
        }
        data_ready_flag |= 0b001;
      }
      else if (frame.id == CAN_ID_DOOR_OC_PWR15) {
        status.doors_open = frame.data[CAN_BYTE_DOOR_OC];
        status.pwr15 = frame.data[CAN_BYTE_PWR15];
        last_pwr_update = time(NULL);
        data_ready_flag |= 0b010;
      }
      else if (frame.id == CAN_ID_DOOR_LU) {
        if (frame.data[1] == 0x80)
          return;
                
        status.doors_unlocked = frame.data[CAN_BYTE_DOOR_LU];
        data_ready_flag |= 0b100;
      }
      evaluate_status();
    }
  }
}

void evaluate_status() {
    if (         time(NULL) > (last_pwr_update + 3) 
              || (status.pwr15 == PWR_15_OFF && data_ready_flag == 0b111)) {
        if ((status.doors_unlocked & DOOR_UNL_DRVR) != DOOR_UNL_DRVR) {
            unlock_doors();
        }
        halt();
    }
    
    if (       status.doors_open == DOOR_OPEN_NONE
            && status.doors_unlocked != DOOR_UNL_NONE
            && status.pwr15 != PWR_15_OFF
            && status.speed_above_threshold == TRUE
            && data_ready_flag == 0b111) {
        lock_doors();
    }
    
    if (data_ready_flag == 0b111)
        data_ready_flag = 0b000;
}

void lock_doors() {
    digitalWrite(PIN_LOC, HIGH);
    delay(300);
    digitalWrite(PIN_LOC, LOW);
}

void unlock_doors() {
    digitalWrite(PIN_UNL, HIGH);
    delay(300);
    digitalWrite(PIN_UNL, LOW);
}

void halt() {
  digitalWrite(PIN_LOC, LOW);
  digitalWrite(PIN_UNL, LOW);
  digitalWrite(PIN_REL, LOW);
  
  noInterrupts();
  while(1);  
}
