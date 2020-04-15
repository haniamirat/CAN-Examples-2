
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main(int argc, char **argv){

    int i;
    int s;
    int nbytes;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct can_frame frame;

    /* Creation of a socket */
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket");
        return 1;
    }

    /* I/O control call and pass an ifreq structure */
    /* containing the interface name */
    strcpy(ifr.ifr_name, "vcan0");
    ioctl(s, SIOCGIFINDEX, &ifr);

    /* Bind the socket to the CAN interface */
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }


    while(1){ /* Permet de garder le test.c en "attente" */

        /* We initialise a CAN frame with an ID of 0x8123, a payload of 8 bytes containing “hello” and send it using the write() system call */
        frame.can_id = 0x8123;
        frame.can_dlc = 8;
        sprintf(frame.data, "Hello");

        if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("Write");
            return 1;
        }

        /* Set up of a filter which allows to print only frames with an ID included between 0x100 and 0x1FF */
        struct can_filter rfilter[1];

        rfilter[0].can_id   = 0x100;
        rfilter[0].can_mask = 0xF00;

        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        /* Part which allows to read a frame */
        /* We display  here the ID, data lenght code and playload */
        nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("Read");
            return 1;
        }
        printf("0x%03X [%d] ",frame.can_id, frame.can_dlc);
        for (i = 0; i < frame.can_dlc; i++){
            printf("%02X ",frame.data[i]);
            printf("\r\n");
        }

    }

}