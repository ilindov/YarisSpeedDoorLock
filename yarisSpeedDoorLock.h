/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   yarisSpeedDoorLock.h
 * Author: ilia
 *
 * Created on May 9, 2018, 2:45 PM
 */

#ifndef YARISSPEEDDOORLOCK_H
#define YARISSPEEDDOORLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <bcm2835.h>

#define TRUE 1
#define FALSE 0

#define PIN_LOC RPI_BPLUS_GPIO_J8_38
#define PIN_UNL RPI_BPLUS_GPIO_J8_40

#define DOOR_OPEN_NONE 0x40 // - all closed             0100 0000
#define DOOR_OPEN_DRVR 0x20 // - driver’s open          0010 0000
#define DOOR_OPEN_PSNG 0x10 // - passenger’s open       0001 0000
#define DOOR_OPEN_REAR 0x0C // - rear open              0000 1100
#define DOOR_OPEN_TRNK 0x02 // - trunk open             0000 0010

#define DOOR_UNL_DRVR 0x10  // - driver’s unlocked      0001 0000
#define DOOR_UNL_PSNG 0x08  // - passenger’s unlocked   0000 1000
#define DOOR_UNL_REAR 0x06  // - rear unlocked          0000 0110
#define DOOR_UNL_TRNK 0x01  // - trunk unlocked         0000 0001
#define DOOR_UNL_NONE 0x00  // - all locked             0000 0000

#define CAN_ID_SPEED         0x610
#define CAN_ID_DOOR_OC_PWR15 0x620
#define CAN_ID_DOOR_LU       0x638

#define PWR_15_ON  0x50
#define PWR_15_OFF 0x80

#define CAN_BYTE_PWR15   7
#define CAN_BYTE_DOOR_OC 5
#define CAN_BYTE_DOOR_LU 2
#define CAN_BYTE_SPEED   2


#define LOCK_SPEED_THRESHOLD 15 // km/h

struct car_status {
    int doors_open;
    int doors_unlocked;
    int pwr15;
    int speed_above_threshold;
};

int init_gpio();
int init_can(const char *ifname);
void deinitialize();
void evaluate_status();
void lock_doors();
void unlock_doors();

#endif /* YARISSPEEDDOORLOCK_H */

