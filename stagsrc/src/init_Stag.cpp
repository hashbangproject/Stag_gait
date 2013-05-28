/* 
 * File:   stag.cpp
 * Author: toner
 * 
 * Created on 21 March 2013, 11:52 AM
 */

#include "Stag.h"
#include <cstdlib>

Stag::Stag() {
    //OpenComport(STAG_COM, HK_BAUD);
    for (int i = 0; i < 4; i++) {
        leg[i].l1 = 0.045f;
        leg[i].l2 = 0.1156f;
        leg[i].l3 = 0.107f;
        leg[i].target = Mat::zeros(3, 4, CV_32F);
        leg[i].phi[0] = 0.0f;
        for (int j = 0; j < 4; j++) {
            leg[i].LED[j] = BLUE;
        }
    }

    leg[0].root_id = LEG_0;
    leg[1].root_id = LEG_1;
    leg[2].root_id = LEG_2;
    leg[3].root_id = LEG_3;

    leg[0].p0f = -1.0f;
    leg[0].p1f = -1.0f;

    leg[1].p0f = 1.0f;
    leg[1].p1f = 1.0f;

    leg[2].p0f = 1.0f;
    leg[2].p1f = -1.0f;

    leg[3].p0f = -1.0f;
    leg[3].p1f = 1.0f;

    leg[0].build_mode = -1.0f;
    leg[1].build_mode = 1.0f;
    leg[2].build_mode = -1.0f;
    leg[3].build_mode = 1.0f;

    l1 = 0.045f;
    l2 = 0.1156f;
    l3 = 0.107f;

    body.by = 0.09f;
    body.bx = 0.1085f;


    body.x = 0.0f;
    body.y = 0.0f;
    //body.h=0.19f;
    //body.h=0.20f;
    body.h = 0.17f;

    body.yaw = 0.0f;
    body.pitch = 0.0f;
    body.roll = 0.0f;
    //pgh = 0.1;

    //pgh = 0.07;
    pgh = 0.07;

    neck_yaw = 0.0f;
    neck_pitch = -M_PI_2;
    stride = 0.05;
    phase = 0;
    tic = 1;

    //trot gait
    gait_ratio = 0.5;
    phase1 = 0.5;
    phase2 = 0;
    phase3 = 0.5;
    phase4 = 0.0;

    //crawl gait
    //gait_ratio = 0.75;
    //phase1 = 0.5;
    //phase2 = 0;
    //phase3 = 0.25;
    //phase4 = 0.75;  

    dp = 0;

    fs[0] = 1; //first step
    fs[1] = 1;
    fs[2] = 1;
    fs[3] = 1;





    lnp = (Mat_<float>(3, 4) <<
            body.bx, body.bx, -body.bx, -body.bx,
            body.by + 0.04, -body.by - 0.04, -body.by - 0.04, body.by + 0.04,
            0, 0, 0, 0);
}

Stag::Stag(const Stag& orig) {
}

Stag::~Stag() {
}
