/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   yarisSpeedDoorLock.c
 * Author: ilia
 *
 * Created on May 9, 2018, 2:26 PM
 */

#include "yarisSpeedDoorLock.h"

int can_socket;
int last_pwr_update;
struct car_status status;
int data_ready_flag;

int init_gpio() {
    if (!bcm2835_init())
	return  0;
 
    bcm2835_gpio_fsel(PIN_LOC, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PIN_UNL, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_LOC, LOW);
    bcm2835_gpio_write(PIN_UNL, LOW);
    
    return 1;
}

int init_can(const char *ifname) {
    struct sockaddr_can addr;
    struct can_filter filter[3];
    struct ifreq ifr;

    if((can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            perror("Error while opening socket");
            return 0;
    }
    
    filter[0].can_id = CAN_ID_SPEED;
    filter[0].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    filter[1].can_id = CAN_ID_DOOR_OC_PWR15;
    filter[1].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    filter[2].can_id = CAN_ID_DOOR_LU;
    filter[2].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    
    if (setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) == -1) {
        perror("Unable to set CAN frame filter. Possible performance hit");
    }

    ifr.ifr_name[0] = ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
    strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
    ioctl(can_socket, SIOCGIFINDEX, &ifr);

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error in socket bind");
        return 0;
    }
    
    return 1;
}

void deinitialize() {
//    printf("\nDe-initialization...\n");
    bcm2835_gpio_write(PIN_LOC, LOW);
    bcm2835_gpio_write(PIN_UNL, LOW);
    
    exit(0);
}

void evaluate_status() {
//    printf("DO: %02x, DU: %02x, PW: %02x, SP: %02x\n",
//            (unsigned int)status.doors_open,
//            (unsigned int)status.doors_unlocked,
//            (unsigned int)status.pwr15,
//            (unsigned int)status.speed_above_threshold);
            
    
    if (        time(NULL) > (last_pwr_update + 3)
            || (status.pwr15 == PWR_15_OFF && data_ready_flag == 0b111)) {
        if ((status.doors_unlocked & DOOR_UNL_DRVR) != DOOR_UNL_DRVR) {
            unlock_doors();
        }
        system("/sbin/shutdown -r now");
        exit(0);
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
//    printf("Locking...\n");
    bcm2835_gpio_write(PIN_LOC, HIGH);
    delay(300);
    bcm2835_gpio_write(PIN_LOC, LOW);
}

void unlock_doors() {
//    printf("Unlocking...\n");
    bcm2835_gpio_write(PIN_UNL, HIGH);
    delay(300);
    bcm2835_gpio_write(PIN_UNL, LOW);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: yarisspeeddoorlock <can_if_name>\n");
        return -1;
    }
    
    if (!init_gpio()) {
        return -2;
    }
    
    if(!init_can(argv[1])) {
        return -3;
    }
    
    if(fcntl(can_socket, F_SETFL, O_NONBLOCK) < 0) {
        perror("Unable to set socket to non-blocking");
        return -4;
    }
    
    signal(SIGINT, deinitialize);
    
//    printf("Initialized...\n");
    
    int bytes_read;
    struct can_frame frame;
    
//    status.doors_open = (DOOR_OPEN_DRVR | DOOR_OPEN_PSNG | DOOR_OPEN_REAR | DOOR_OPEN_TRNK);
//    status.doors_unlocked = (DOOR_UNL_DRVR | DOOR_UNL_PSNG | DOOR_UNL_REAR | DOOR_UNL_TRNK);
//    status.pwr15 = PWR_15_ON;
//    status.speed_above_threshold = FALSE;
    last_pwr_update = time(NULL);
    data_ready_flag = 0b000;
    
    while (1) {
        if ((bytes_read = read(can_socket, &frame, sizeof(struct can_frame))) == sizeof(struct can_frame)) {
            if (frame.can_id == CAN_ID_SPEED) {
//                printf("--- 610\n");
                if (frame.data[CAN_BYTE_SPEED] >= LOCK_SPEED_THRESHOLD) {
                    status.speed_above_threshold = TRUE;
                }
                else {
                    status.speed_above_threshold = FALSE;
                }
//                printf("SPD: %d, THR: %d, SAT: %d\n", frame.data[CAN_BYTE_SPEED], LOCK_SPEED_THRESHOLD, status.speed_above_threshold);
                data_ready_flag |= 0b001;
            }
            else if (frame.can_id == CAN_ID_DOOR_OC_PWR15) {
//                printf("--- 620\n");
                status.doors_open = frame.data[CAN_BYTE_DOOR_OC];
                status.pwr15 = frame.data[CAN_BYTE_PWR15];
                last_pwr_update = time(NULL);
                data_ready_flag |= 0b010;
            }
            else if (frame.can_id == CAN_ID_DOOR_LU) {
                if (frame.data[1] == 0x80)
                    continue;
//                printf("--- 638\n");
                
                status.doors_unlocked = frame.data[CAN_BYTE_DOOR_LU];
//                printf("ID: %x, DT: %02x, ST: %02x\n",
//                        frame.can_id,
//                        (unsigned int)frame.data[CAN_BYTE_DOOR_LU],
//                        (unsigned int)status.doors_unlocked);
                data_ready_flag |= 0b100;
            }
            else {
                fprintf(stderr, "It seems that the can filters do not work. Caught frame with id: %d", frame.can_id);
            }
        }
        else {
            if (errno != EAGAIN)
                fprintf(stderr, "read: incomplete can frame\n");
            usleep(100000);
        }
        
        evaluate_status();
    }

    return 0;
}

