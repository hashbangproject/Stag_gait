/* 
 * File:   sixmotion.cpp
 * Author: toner
 * 
 * Created on 21 March 2013, 1:09 PM
 */

#include "sixmotion.h"
#include <math.h>
int force_sixaxis = 1;
int force_ds3 = 0;
int timestamp = 0;
int nostdin = 0;

void be_memcpy(const void *destination, const void *source, uint8_t size) {
    uint8_t copy_count = 0;
    while (size) ((uint8_t *) destination)[copy_count++] = ((uint8_t *) source)[--size];
}

void fatal(char *msg) {
    if (errno) perror(msg);
    else fprintf(stderr, "%s\n", msg);
    exit(1);
}

int l2cap_listen(unsigned short psm) {
    int sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sk < 0) fatal("socket");

    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof (addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_bdaddr = {
        {0, 0, 0, 0, 0, 0}
    };
    addr.l2_psm = htobs(psm);
    if (bind(sk, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
        perror("bind");
        close(sk);
        return -1;
    }

    if (listen(sk, 5) < 0) fatal("listen");
    return sk;
}

struct motion_dev *accept_device(int csk, int isk) {
    fprintf(stderr, "Incoming connection...\n");
    struct motion_dev *dev = (motion_dev*) malloc(sizeof (struct motion_dev));
    if (!dev) fatal("malloc");

    dev->csk = accept(csk, NULL, NULL);
    if (dev->csk < 0) fatal("accept(CTRL)");
    dev->isk = accept(isk, NULL, NULL);
    if (dev->isk < 0) fatal("accept(INTR)");

    struct sockaddr_l2 addr;
    socklen_t addrlen = sizeof (addr);
    if (getpeername(dev->isk, (struct sockaddr *) &addr, &addrlen) < 0)
        fatal("getpeername");
    dev->addr = addr.l2_bdaddr;

    {
        // Distinguish SIXAXIS / DS3 / PSMOVE.
        unsigned char resp[64];
        char get03f2[] = {HIDP_TRANS_GET_REPORT | HIDP_DATA_RTYPE_FEATURE | 8,
            0xf2, sizeof (resp), sizeof (resp) >> 8};
        send(dev->csk, get03f2, sizeof (get03f2), 0); // 0301 is interesting too.
        int nr = recv(dev->csk, resp, sizeof (resp), 0);
        if (nr < 19) dev->type = PSMOVE;
        else if (force_sixaxis) dev->type = SIXAXIS;
        else if (force_ds3) dev->type = DS3;
        else dev->type = (resp[13] == 0x40) ? SIXAXIS : DS3; // My guess.
    }

    return dev;
}

void hidp_trans(int csk, int isk, const char *buf, int len) {

    if (send(csk, buf, len, 0) != len) fatal("send(CTRL)");
    unsigned char ack;
    int nr = recv(csk, &ack, sizeof (ack), 0);
    //int nr = recv(csk, &ack, sizeof (ack), MSG_DONTWAIT);//THIS no longer in same thread needs to be looked at
    //if (nr != 1 || ack != 0) fatal("ack");
}

void setup_device(struct motion_dev *dev) {

    switch (dev->type) {
        case SIXAXIS:
        case DS3:
        {

            char set03f4[] = {HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_FEATURE, 0xf4,
                0x42, 0x03, 0x00, 0x00};

            hidp_trans(dev->csk, dev->isk, set03f4, sizeof (set03f4));

            static const char ledmask[10] = {1, 2, 4, 8, 6, 7, 11, 13, 14, 15};
#define LED_PERMANENT 0xff, 0x27, 0x10, 0x00, 0x32
            char set0201[] = {
                HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUTPUT, 0x01,
                0x00, 0x00, 0x00, 0, 0, 0x00, 0x00, 0x00,
                0x00, ledmask[dev->index % 10] << 1,
                LED_PERMANENT,
                LED_PERMANENT,
                LED_PERMANENT,
                LED_PERMANENT,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            char output_report[] = {
                0x52, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x70, 0x00, 0x00, 0x00,
                0xff, 0x27, 0x10, 0x00, 0x32,
                0xff, 0x27, 0x10, 0x00, 0x32,
                0xff, 0x27, 0x10, 0x00, 0x32,
                0xff, 0x27, 0x10, 0x00, 0x32,
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };


            if (dev->type == SIXAXIS) {
                printf("six\n");
            } else {
                printf("ds3\n");

            }
            printf("start trans\n");
            printf("finish setup\n");
            break;
        }


    }

}

void parse_raw(_sixmotion_hidraw &raw, _sixmotion &sixm) {

    sixm.L3_x = -128.0f + (float) raw.L3_x;
    sixm.L3_y = 128.0f - (float) raw.L3_y;
    sixm.L3_mag = sqrt(sixm.L3_x * sixm.L3_x + sixm.L3_y * sixm.L3_y);

    if (sixm.L3_mag > 128.0f)
        sixm.L3_mag = 128.0f;

    //printf("%i\n",raw.BTN_L1_VALUE);

    sixm.R3_x = -128.0f + (float) raw.R3_x;
    sixm.R3_y = 128.0f - (float) raw.R3_y;
    sixm.R3_mag = sqrt(sixm.R3_x * sixm.R3_x + sixm.R3_y * sixm.R3_y);


    if (sixm.R3_mag > 128.0f)
        sixm.R3_mag = 128.0f;



    uint16_t ax, ay, az, gz;


    be_memcpy(&ax, raw.ax, 2);
    be_memcpy(&ay, raw.ay, 2);
    be_memcpy(&az, raw.az, 2);
    be_memcpy(&gz, raw.gz, 2);

    sixm.aX = ax - 512;
    sixm.aY = ay - 512;
    sixm.aZ = az - 512 - 100;
    sixm.gZ = gz - 512;

    //printf("%f %f\n",sixm.R3_x,sixm.R3_y);

    //printf("%i \n",ax);

    //sixm.aX = raw.ax1*256 + raw.ax2 - 512;
    //sixm.aY = raw.ay1*256 + raw.ay2 - 512;
    //sixm.aZ = raw.az1*256 + raw.az2 - 512;
    //sixm.gZ = raw.gz1*256 + raw.gz2 - 512;

    //printf("%f %f %f %f\n",sixm.aX,sixm.aY,sixm.aZ,sixm.gZ);

    //sqrt(sixm.aX*sixm.aX+sixm.aY*sixm.aY+sixm.aZ+sixm.aZ);


    //printf("%f %f %f %f\n",sixm.aX,sixm.aY,sixm.aZ);
    //printf("%f\n",sqrt(sixm.aX*sixm.aX+sixm.aY*sixm.aY+sixm.aZ*sixm.aZ));

    //printf("%i %i\n",raw.gz1,raw.gz2);

    //printf("%i %i %i %i\n",raw.aX,raw.aY,raw.aZ,raw.gZ);

}

sixmotion::sixmotion() {


    devs = NULL;
    next_devindex = 0;

    csk = l2cap_listen(L2CAP_PSM_HIDP_CTRL);
    isk = l2cap_listen(L2CAP_PSM_HIDP_INTR);
    active = false;
    connected = false;
    if (csk >= 0 && isk >= 0) {
        fprintf(stderr, "Waiting for Bluetooth connection.\n");
        active = true;
        pthread_create(&thread, 0, &sixmotion::loop, this);
    } else {
        fprintf(stderr, "Unable to listen on HID PSMs."
                " Are you root bro?\n");
        active = false;
    }

}

sixmotion::sixmotion(const sixmotion& orig) {
}

sixmotion::~sixmotion() {

    active = false;
    pthread_join(thread, 0);
}

void* sixmotion::loop(void *obj) {

    while (reinterpret_cast<sixmotion *> (obj)->active) reinterpret_cast<sixmotion *> (obj)->readEv();
}

void sixmotion::readEv() {


    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    int fdmax = 0;
    if (csk >= 0) FD_SET(csk, &fds);
    if (isk >= 0) FD_SET(isk, &fds);
    if (csk > fdmax) fdmax = csk;
    if (isk > fdmax) fdmax = isk;
    for (struct motion_dev *dev = devs; dev; dev = dev->next) {
        FD_SET(dev->csk, &fds);
        if (dev->csk > fdmax) fdmax = dev->csk;
        FD_SET(dev->isk, &fds);
        if (dev->isk > fdmax) fdmax = dev->isk;
    }

    //waiting for a controler
    if (select(fdmax + 1, &fds, NULL, NULL, NULL) < 0) fatal("select");

    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //time_t now = tv.tv_sec;
    // Incoming connection ?
    if (csk >= 0 && FD_ISSET(csk, &fds)) {//CONNECTS DEVICE
        struct motion_dev *dev = accept_device(csk, isk);
        dev->index = next_devindex++;
        dev->next = devs;
        devs = dev;
        setup_device(dev);
        connected = true;
    }


    for (struct motion_dev *dev = devs; dev; dev = dev->next)
        if (FD_ISSET(dev->isk, &fds)) {
            unsigned char report[256];

            //select (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout);

            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            //select(fdmax + 1, &fds, NULL, NULL, &tv);
            //int nr = recv(dev->isk, report, sizeof (report), 0);
            int nr = recv(dev->isk, report, sizeof (report), MSG_DONTWAIT);

            if (nr == 50) {
                if (report[0] == 0xa1) {

                    memcpy(&raw_sixm, report, nr);
                    //dump("RECV", report, nr);
                    parse_raw(raw_sixm, sixm);

                }
            }

            /*
            if (nr <= 0) {
                //fprintf(stderr, "%d disconnected\n", dev->index);
                printf("DISCONNECTED\n");
                close(dev->csk);
                close(dev->isk);
                struct motion_dev **pdev;
                for (pdev = &devs; *pdev != dev; pdev = &(*pdev)->next);
             *pdev = dev->next;
                free(dev);
            } else {

                if (report[0] == 0xa1) {

                }
            }*/
        }


    /*
    for (struct motion_dev *dev = devs; dev; dev = dev->next)
        if (FD_ISSET(dev->isk, &fds)) {

            int nr = 1;
            while (nr > 0) {
                unsigned char report[256];

                //nr = recv(dev->isk, report, sizeof (report), MSG_DONTWAIT);
                printf("RCV\n");
                nr = recv(dev->isk, report, sizeof (report), 0);
                //THIS IS THE REPORT
                if (report[0] == 0xa1) {
                    if (nr == 50) {

                        //memcpy(&raw_sixm, report, nr);
                        //dump("RECV", report, nr);
                        //parse_raw(raw_sixm, sixm);

                        printf("REPORT\n");
                    }


                } else {
                    printf("REPORT\n");
                }

            }
        }*/

}

_sixmotion sixmotion::state() {

    return sixm;

}