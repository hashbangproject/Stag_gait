/* 
 * File:   HKLX.cpp
 * Author: toner
 * 
 * Created on 22 March 2013, 10:43 AM
 */

#include "HKLX.h"
#include "stdint.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <cstdlib>
#include "opencv2/core/core.hpp"



using namespace std;
using namespace cv;

void hklx_decoder_initialise(hklx_decoder_t *hklx_decoder) {
    hklx_decoder->buffer_length = 0;
    hklx_decoder->crc_errors = 0;
}

hklx_packet_t *hklx_packet_allocate(uint8_t length, uint8_t id, uint8_t cmd) {
    hklx_packet_t *hklx_packet = (hklx_packet_t*) malloc(sizeof (hklx_packet_t) + length * sizeof (uint8_t));
    if (hklx_packet != NULL) {

        hklx_packet->header[0] = HEADER;
        hklx_packet->header[1] = HEADER;
        hklx_packet->packetsize = length + MIN_PACKET_SIZE;
        hklx_packet->id = id;
        hklx_packet->cmd = cmd;

    }
    return hklx_packet;
}

void hklx_packet_free(hklx_packet_t **hklx_packet) {
    free(*hklx_packet);
    *hklx_packet = NULL;
}

uint16_t calculate_crc(const void *data)
//uint16_t calculate_crc(uint8_t *data)
{

    uint8_t *bytes = (uint8_t *) data;
    uint16_t crc;

    uint8_t CheckSum1 = bytes[0] ^ bytes[1] ^ bytes[2];
    for (int i = 0; i < ((int) bytes[0] - MIN_PACKET_SIZE); i++)
        CheckSum1 ^= bytes[5 + i];

    uint8_t CheckSum2 = ~(CheckSum1);
    CheckSum1 &= CHKSUM_MASK;
    CheckSum2 &= CHKSUM_MASK;

    crc = CheckSum1;
    crc |= CheckSum2 << 8;


    return crc;
}

/*
void hklx_send(hklx_packet_t *hklx_packet) {

    hklx_packet->CRC = calculate_crc(&hklx_packet->packetsize);

    int sentb=0;

    sentb = SendBuf(HKLX_COM, &hklx_packet->header[0], hklx_packet->packetsize);
    
    if (sentb != hklx_packet->packetsize)
        printf("THEFUCK??\n");
    

    return;
}
 */

/*
void HKLX::hklx_send(hklx_packet_t *hklx_packet) {

    pthread_mutex_lock(&m_mut);
    hklx_packet->CRC = calculate_crc(&hklx_packet->packetsize);
    SendBuf(HKLX_COM, &hklx_packet->header[0], hklx_packet->packetsize);
    pthread_mutex_unlock(&m_mut);
  
}*/

void HKLX::send(hklx_packet_t *hklx_packet) {

    pthread_mutex_lock(&m_mut);
    hklx_packet->CRC = calculate_crc(&hklx_packet->packetsize);
    SendBuf(HKLX_COM, &hklx_packet->header[0], hklx_packet->packetsize);
    pthread_mutex_unlock(&m_mut);

}

hklx_packet_t *hklx_packet_decode_new(hklx_decoder_t *hklx_decoder) {

    hklx_packet_t *hklx_packet = NULL;
    uint8_t id, length, cmd, header1, header2;
    uint16_t crc;



    if (MIN_PACKET_SIZE <= hklx_decoder->buffer_length) {
        header1 = hklx_decoder->buffer[0];
        header2 = hklx_decoder->buffer[1];
        if (header1 == HEADER && header2 == HEADER) {//good

            length = hklx_decoder->buffer[2];
            id = hklx_decoder->buffer[3];
            cmd = hklx_decoder->buffer[4];
            crc = hklx_decoder->buffer[5];
            crc |= hklx_decoder->buffer[6] << 8;

            if (length == hklx_decoder->buffer_length) {//good

                if (crc == calculate_crc(&hklx_decoder->buffer[2])) {


                    hklx_packet = hklx_packet_allocate(length - MIN_PACKET_SIZE, id, cmd);
                    if (hklx_packet != NULL) {
                        memcpy(hklx_packet->data, &hklx_decoder->buffer[7], (length - MIN_PACKET_SIZE) * sizeof (uint8_t));
                    }
                    hklx_decoder->buffer_length = 0;
                    return hklx_packet; //must also flush com port 
                } else {
                    //CRC ERROR
                    //printf("CRC ERROR\n");
                    printf("crc FAIL\n");
                    hklx_decoder->buffer_length = 0;
                    return hklx_packet; //must also flush com port
                }



            } else {//length fail



                printf("%i %i length FAIL\n", length, hklx_decoder->buffer_length);
                hklx_decoder->buffer_length = 0;
                return hklx_packet; //must also flush com port 
            }


        } else {//header error



            printf("head FAIL\n");
            hklx_decoder->buffer_length = 0;
            return hklx_packet; //must also flush com port 
        }

    }
    printf("gen FAIL\n");
    //printf("%i\n",hklx_decoder->buffer_length);
    hklx_decoder->buffer_length = 0;
    return hklx_packet;
}

hklx_packet_t *hklx_packet_decode(hklx_decoder_t *hklx_decoder) {


    //THIS IS SUPER FUCKED
    uint16_t decode_iterator = 0;
    hklx_packet_t *hklx_packet = NULL;
    uint8_t id, length, cmd, header1, header2;
    uint16_t crc;


    while (decode_iterator + MIN_PACKET_SIZE <= hklx_decoder->buffer_length) {
        //header_lrc = hklx_decoder->buffer[decode_iterator++];

        header1 = hklx_decoder->buffer[decode_iterator++];
        header2 = hklx_decoder->buffer[decode_iterator];

        if (header1 == HEADER && header2 == HEADER) {

            decode_iterator++;
            length = hklx_decoder->buffer[decode_iterator++];
            id = hklx_decoder->buffer[decode_iterator++];
            cmd = hklx_decoder->buffer[decode_iterator++];

            //crc = hklx_decoder->buffer[decode_iterator++];
            //crc |= hklx_decoder->buffer[decode_iterator++] << 8;

            crc = hklx_decoder->buffer[decode_iterator++];
            crc |= hklx_decoder->buffer[decode_iterator++] << 8;
            //printf("%i\n",length);

            if (decode_iterator + length - MIN_PACKET_SIZE > hklx_decoder->buffer_length) {
                decode_iterator -= MIN_PACKET_SIZE;
                break;
            }

            if (crc == calculate_crc(&hklx_decoder->buffer[decode_iterator - MIN_PACKET_SIZE + 2])) {
                hklx_packet = hklx_packet_allocate(length - MIN_PACKET_SIZE, id, cmd);
                if (hklx_packet != NULL) {
                    memcpy(hklx_packet->data, &hklx_decoder->buffer[decode_iterator], (length - MIN_PACKET_SIZE) * sizeof (uint8_t));
                }
                decode_iterator += length;
                //decode_iterator += length- MIN_PACKET_SIZE;
                break;
            } else {
                printf("CRC ERROR\n");
                //decode_iterator -= (MIN_PACKET_SIZE);
                //decode_iterator -= (2);
                //decode_iterator += length- MIN_PACKET_SIZE;

                ////decode_iterator += length;
                ////hklx_decoder->crc_errors++;

                printf("%i %i %i %i %i\n", length, decode_iterator, hklx_decoder->buffer_length, id, cmd);
                hklx_packet = hklx_packet_allocate(length - MIN_PACKET_SIZE, id, cmd);
                if (hklx_packet != NULL) {
                    memcpy(hklx_packet->data, &hklx_decoder->buffer[decode_iterator], (length - MIN_PACKET_SIZE) * sizeof (uint8_t));
                }
                decode_iterator += length;
                //decode_iterator += length- MIN_PACKET_SIZE;

                break;
            }
        }
    }

    if (decode_iterator < hklx_decoder->buffer_length) {
        if (decode_iterator > 0) {
            memmove(&hklx_decoder->buffer[0], &hklx_decoder->buffer[decode_iterator], (hklx_decoder->buffer_length - decode_iterator) * sizeof (uint8_t));
            hklx_decoder->buffer_length -= decode_iterator;
        }
    } else hklx_decoder->buffer_length = 0;



    //printf("%i\n",hklx_decoder->buffer_length);

    return hklx_packet;
}

void HKLX::hklx_packet_parse(hklx_packet_t *hklx_packet) {

    switch (hklx_packet->cmd) {

        case CMD_RAM_READ_ACK:

            /*double exec_time = (double) getTickCount();
        usleep(1000000);
        //stag.hklx.update();
        exec_time = ((double) getTickCount() - exec_time) / getTickFrequency();*/


            memcpy(&id[hklx_packet->id].RAM.ID + (int) (hklx_packet->data[0]), &hklx_packet->data[2], hklx_packet->data[1]);
            double new_tc = (double) getTickCount();
            float new_w = ((float) id[hklx_packet->id].RAM.Differential_Position)*29.09f * M_PI / 180.0f;

            id[hklx_packet->id].RAM.Calibrated_Position = (id[hklx_packet->id].RAM.Calibrated_Position & CALIBRATE_MASK);
            id[hklx_packet->id].abs_phi = ((float) id[hklx_packet->id].RAM.Absolute_Position)*0.3258064516f - 166.650f;
            //id[hklx_packet->id].phi = ((float) id[hklx_packet->id].RAM.Calibrated_Position)*0.3258064516f - 166.650f;

            float newphi = ((float) id[hklx_packet->id].RAM.Calibrated_Position)*0.3258064516f - 166.650f;
            //id[hklx_packet->id].W = ((newphi-id[hklx_packet->id].phi)/((new_tc-id[hklx_packet->id].tc)/ getTickFrequency()))*M_PI/180.0f;            
            float newW = ((newphi - id[hklx_packet->id].phi) / ((new_tc - id[hklx_packet->id].tc) / getTickFrequency())) * M_PI / 180.0f;

            id[hklx_packet->id].A = (newW - id[hklx_packet->id].W) / ((new_tc - id[hklx_packet->id].tc) / getTickFrequency());
            id[hklx_packet->id].a = (new_w - id[hklx_packet->id].w) / ((new_tc - id[hklx_packet->id].tc) / getTickFrequency());

            id[hklx_packet->id].W = newW;
            id[hklx_packet->id].phi = newphi;
            id[hklx_packet->id].w = new_w;
            id[hklx_packet->id].tc = new_tc;


            id[hklx_packet->id].vin = ((float) id[hklx_packet->id].RAM.Voltage)*0.0740745098f;
            id[hklx_packet->id].temp = ((float) id[hklx_packet->id].RAM.Temperature)*1.497559055f - 79.47f;

            //id[hklx_packet->id].RAM.PWM = (id[hklx_packet->id].RAM.PWM & T_MASK);
            id[hklx_packet->id].pwm = ((float) id[hklx_packet->id].RAM.PWM) / 1023.0;
            //id[hklx_packet->id].w = ((float)id[hklx_packet->id].RAM.Differential_Position)*29.09f*M_PI/180.0f;



            //if(hklx_packet->id==2){
            //if(fabs(id[hklx_packet->id].pwm)>0.3){
            //printf("%i\n",hklx_packet->data[1]);
            //printf("%i\n",id[hklx_packet->id].RAM.PWM);

            //for(int i=0;i<hklx_packet->data[1];i++){
            //    printf("%02x",hklx_packet->data[2+i]);
            //for(int i=0;i<hklx_packet->packetsize-7;i++){
            //    printf("%02x",hklx_packet->data[i]);
            //}
            //printf("\n");
            // }
            //}

            /*
            if(hklx_packet->id==3){
                //printf("%i\n",hklx_packet->data[1]);
                for(int i=0;i<hklx_packet->data[1];i++){
                    printf("%02x",hklx_packet->data[2+i]);
                }
                printf("\n");
                
            }*/

            /*
            if(hklx_packet->id==2){
                printf("%i\n",hklx_packet->data[1]);
                for(int i=0;i<hklx_packet->data[1];i++){
                    printf("%x",hklx_packet->data[2+i]);
                }
                printf("\n");
                
            }*/

            //printf("%i\n",hklx_packet->packetsize);
            //printf("%f\n",id[hklx_packet->id].phi);

            break;



            /*
            case 1:


                break;
            case 2:


                break;
            case 3:


                break;
             */
    }



}

/*
int HKLX::recive() {

    hklx_packet_t *hklx_packet;
    int bytes_received = 0;
    //while (1) {//needs timeout

    
    //fd_set fds;
    //FD_ZERO(&fds);
    //FD_SET(hklx_fd, &fds);
    //struct timeval timeout = {0, 3000}; 
    //int ret = select(hklx_fd + 1, &fds, NULL, NULL, &timeout);
    
    //printf("%i\n",hklx_decoder_size(&hklx_decoder));
    //if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
    //int ret=1;
    //if (ret > 0)
        while (1) {
            //while ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
            //if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
            if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), 1)) > 0) {
                //printf("%i\n",bytes_received);
                hklx_decoder_increment(&hklx_decoder, bytes_received);
                while ((hklx_packet = hklx_packet_decode(&hklx_decoder)) != NULL) {
                    hklx_packet_parse(hklx_packet);

                    hklx_packet_free(&hklx_packet);
                    return 1;
                }
                //usleep(2000);
            }
        }

    return 0;//lolz
}
 */


int HKLX::recive() {

    hklx_packet_t *hklx_packet;
    int bytes_received = 0;



    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(hklx_fd, &fds);
    struct timeval timeout = {1, 0};
    int ret = select(hklx_fd + 1, &fds, NULL, NULL, &timeout);

    if (ret > 0)
        while (1) {

            if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
                //printf("%i\n",bytes_received);
                hklx_decoder_increment(&hklx_decoder, bytes_received);
                while ((hklx_packet = hklx_packet_decode(&hklx_decoder)) != NULL) {
                    hklx_packet_parse(hklx_packet);

                    hklx_packet_free(&hklx_packet);
                    return 1;
                }

            }
        }

    return 0;
}


//hklx_packet->CRC = calculate_crc(&hklx_packet->packetsize);
//SendBuf(HKLX_COM, &hklx_packet->header[0], hklx_packet->packetsize);

int return_length(hklx_packet_t *query_packet) {
    return 7 + 4 + query_packet->data[1];
}

int HKLX::query(hklx_packet_t *query_packet) {

    //query_packet->CRC = calculate_crc(&query_packet->packetsize);
    SendBuf(HKLX_COM, &query_packet->header[0], query_packet->packetsize);

    int length = return_length(query_packet);

    hklx_packet_t *hklx_packet;
    int bytes_received = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(hklx_fd, &fds);
    struct timeval timeout = {0, 3000};
    int ret = select(hklx_fd + 1, &fds, NULL, NULL, &timeout); //CHANGE TO PSELECT

    //printf("%i\n",length);

    while (hklx_decoder.buffer_length < length) {//here in lies the rub
        if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
            hklx_decoder_increment(&hklx_decoder, bytes_received);
        }
    }

    if ((hklx_packet = hklx_packet_decode_new(&hklx_decoder)) != NULL) {
        hklx_packet_parse(hklx_packet);

        hklx_packet_free(&hklx_packet);
        return 1;
    }



    /*
    while (1) {
        
        if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), hklx_decoder_size(&hklx_decoder))) > 0) {
        //if ((bytes_received = PollComport(HKLX_COM, hklx_decoder_pointer(&hklx_decoder), 1)) > 0) {
            //printf("%i\n",bytes_received);
            hklx_decoder_increment(&hklx_decoder, bytes_received);
            
            
            while ((hklx_packet = hklx_packet_decode(&hklx_decoder)) != NULL) {
                //hklx_packet_parse(hklx_packet);

                hklx_packet_free(&hklx_packet);
                return 1;
            }
            
            
        }
    }*/

    return 0; //lolz
}

HKLX::HKLX() {

    //hklx_fd = OpenComport(HKLX_COM, BAUD,500000);
    hklx_fd = OpenComport(HKLX_COM, BAUD, 666666);
    hklx_decoder_initialise(&hklx_decoder);

    //hklx_packet_t *hklx_packet = hklx_packet_allocate(3, BROADCAST_ID, CMD_EEP_WRITE);

    //hklx_packet->data[0] = 0x04;
    //hklx_packet->data[1] = 0x01;
    //hklx_packet->data[2] = 0x02;
    //hklx_send(hklx_packet);

    //hklx_send(hklx_packet);
    //hklx_packet_free(&hklx_packet);

    //printf("done\n");
    //usleep(100000000000000);
    queue = NULL;
    discover();

    active = 1;




    //usleep(100000000000000);

    pthread_mutex_init(&m_mut, NULL);

    start = (double) getTickCount();
    fr = 0;
    //pthread_create(&thread, 0, &HKLX::loop, this);


}

HKLX::HKLX(const HKLX& orig) {
}

HKLX::~HKLX() {

    if (active == 0) {

        pthread_mutex_destroy(&m_mut);
        pthread_join(thread, 0);
    }

}

void HKLX::discover() {


    //update shoud be 54 then 12

    //hklx_packet_t *hklx_packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
    //hklx_packet->data[0] = 0;
    //hklx_packet->data[1] = 74;

    //hklx_packet_free(&hklx_packet);

    /*
    for (int i = 0; i < 254; i++) {

        hklx_packet->id = (uint8_t) (i);
        hklx_send(hklx_packet);
        //int rval = recive();
        id[i].connected=recive();
        
        if(id[i].connected){
            connected_id.push_back(i);
        }
        
        //printf("%i\n", rval);

        //usleep(10000);
    }*/

    //id[i].connected=recive();


    connected_id.push_back(0);
    connected_id.push_back(1);
    connected_id.push_back(2);
    connected_id.push_back(3);

    connected_id.push_back(8);
    connected_id.push_back(9);
    connected_id.push_back(10);
    connected_id.push_back(11);

    connected_id.push_back(16);
    connected_id.push_back(17);
    connected_id.push_back(18);
    connected_id.push_back(19);

    connected_id.push_back(20);
    connected_id.push_back(21);
    connected_id.push_back(22);
    connected_id.push_back(23);

    //connected_id.push_back(13);
    //connected_id.push_back(14);

    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);

    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);

    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);

    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);
    state_ready.push_back(0);

    //state_ready.push_back(0);
    //state_ready.push_back(0);


    /*
    connected_id.push_back(2);
    connected_id.push_back(3);
    state_ready.push_back(0);
    state_ready.push_back(0);    
     */

    //hklx_packet_t *hklx_packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
    //hklx_packet_t *hklx_packet = hklx_packet_allocate(2, BROADCAST_ID, CMD_RAM_READ);
    //hklx_packet->data[0] = 54;
    //hklx_packet->data[0] = 1;
    //hklx_packet->data[1] = 1;
    //hklx_send(hklx_packet);

    //int rval = recive();

    //queue = (struct test_struct*)malloc(sizeof(struct test_struct));

    //queue->next=queue;


    //hklx_packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
    //hklx_packet->data[0] = 0;
    //hklx_packet->data[1] = 74;


    //printf("%i\n",sizeof(test_struct));

    for (int i = 0; i < connected_id.size(); i++) {

        if (queue == NULL) {
            queue = (test_struct*) malloc(sizeof (test_struct));

            //hklx_packet->id=connected_id[i];
            queue->packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
            queue->packet->data[0] = 58;
            queue->packet->data[1] = 8;
            //queue->packet->data[0] = 54;
            //queue->packet->data[1] = 12;
            //queue->packet->data[0] = 45;
            //queue->packet->data[1] = 23;

            queue->packet->id = connected_id[i];
            queue->packet->CRC = calculate_crc(&queue->packet->packetsize);

            queue->val = i;
            queue->del = 0;
            prv = queue;
            queue->next = queue;
        } else {

            test_struct *ptr = (test_struct*) malloc(sizeof (test_struct));
            //hklx_packet->id=connected_id[i];

            ptr->packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
            ptr->packet->data[0] = 58;
            ptr->packet->data[1] = 8;
            //ptr->packet->data[0] = 54;
            //ptr->packet->data[1] = 12;
            //ptr->packet->data[0] = 45;
            //ptr->packet->data[1] = 23;
            ptr->packet->id = connected_id[i];
            ptr->packet->CRC = calculate_crc(&ptr->packet->packetsize);


            ptr->val = i;
            ptr->del = 0;
            ptr->next = queue->next;
            queue->next = ptr;
            prv = queue;
            queue = queue->next;


        }

    }

}

/*
void HKLX::update() {

    
    //update shoud be 54 then 12

    hklx_packet_t *hklx_packet = hklx_packet_allocate(2, 0x00, CMD_RAM_READ);
    //hklx_packet_t *hklx_packet = hklx_packet_allocate(2, BROADCAST_ID, CMD_RAM_READ);
    //hklx_packet->data[0] = 54;
    //hklx_packet->data[1] = 7;
    hklx_packet->data[0] = 1;
    hklx_packet->data[1] = 1;
    //hklx_send(hklx_packet);
   
    //int rval = recive();
    
    
    for (int i = 0; i < connected_id.size(); i++) {

        hklx_packet->id = (uint8_t) (connected_id[i]);
        
        
        
        double exec_time = (double) getTickCount();

        
        //hklx_send(hklx_packet);
        //int rval = recive();


        exec_time = ((double) getTickCount() - exec_time)*1000000. / getTickFrequency();
            
            
        //printf("%f\n",exec_time);        
        
        //id[i].connected=recive();
        
        //printf("con %i\n", id[connected_id[i]].RAM.ACK_Policy);
        
        //printf("%i\n", rval);

        //usleep(10000);
    }
    
    
    hklx_packet_free(&hklx_packet);

}
 */


void* HKLX::loop(void *obj) {
    while (reinterpret_cast<HKLX *> (obj)->active) reinterpret_cast<HKLX *> (obj)-> poll();
}

void HKLX::add2queue() {


    //printf("TRYLOCKA\n");
    pthread_mutex_lock(&m_mut);
    //printf("lock a\n");
    test_struct *ptr;
    ptr = (test_struct*) malloc(sizeof (test_struct));

    ptr->val = 244;
    ptr->del = 1;
    ptr->next = queue->next;
    queue->next = ptr;
    pthread_mutex_unlock(&m_mut);

}

void HKLX::poll() {


    pthread_mutex_lock(&m_mut);
    //double exec_time = (double) getTickCount();
    //query(queue->packet);
    //exec_time = ((double) getTickCount() - exec_time) / getTickFrequency();
    //printf("%f\n", exec_time);
    //state_ready[queue->val]=1;

    if (query(queue->packet)) {
        state_ready[queue->val] = 1;
    }

    //usleep(1000);
    pthread_mutex_unlock(&m_mut);

    //if(queue->val==0){
    //fr++;
    //printf("%f\n", fr / (((double) getTickCount() - start) / getTickFrequency()));
    //}

    switch (queue->del) {

        case 0:
            prv = queue;
            queue = queue->next;
            break;

        case 1:


            test_struct *del = queue;
            prv->next = del->next;
            queue = queue->next;
            hklx_packet_free(&del->packet);
            free(del);

            break;


    }


    //usleep(2200);

    usleep(20);
    //usleep(2000);
    //usleep(1);
    //add2queue();

    //usleep(500000);



}

void HKLX::state(_hklx out[]) {


    pthread_mutex_lock(&m_mut);
    for (int i = 0; i < connected_id.size(); i++) {
        if (state_ready[i]) {
            memcpy(&out[connected_id[i]].RAM.ID, &id[connected_id[i]].RAM.ID, sizeof (_hklx));
            state_ready[i] = 0;
        }
    }
    pthread_mutex_unlock(&m_mut);


    //_hklx id[254];
    //if(connected)
    //return id;

    //return NULL;
}