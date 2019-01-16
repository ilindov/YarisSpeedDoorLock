#ifndef YARISSPEEDDOORLOCK_H
#define YARISSPEEDDOORLOCK_H

#define TRUE 1
#define FALSE 0

#define PIN_REL 3
#define PIN_LOC 6
#define PIN_UNL 11

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

#endif /* YARISSPEEDDOORLOCK_H */
