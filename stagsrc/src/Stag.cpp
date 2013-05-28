/* 
 * File:   stag.cpp
 * Author: toner
 * 
 * Created on 21 March 2013, 11:52 AM
 */

#include "Stag.h"
#include <cstdlib>

Mat rot_xyz(Mat in, float phi, int ax) {

    float* u = in.ptr<float>(0);
    float u1 = u[ax];
    float u2 = u[3 + ax];
    float u3 = u[6 + ax];

    Mat Ux = (Mat_<float>(3, 3) <<
            0.0f, -u3, u2,
            u3, 0, -u1,
            -u2, u1, 0);
    Mat UxU = (Mat_<float>(3, 3) <<
            u1*u1, u1*u2, u1*u3,
            u1*u2, u2*u2, u2*u3,
            u1*u3, u2*u3, u3 * u3);

    Mat I = Mat::eye(3, 3, CV_32F);

    Mat R = I * cos(phi) + sin(phi) * Ux + (1 - cos(phi)) * UxU;

    return R*in;

}

void Stag::ikb(int t) {

    float *x, *y, *z;

    float old_phi[4];
    float old_w[4];

    for (int i = 0; i < 4; i++) {

        //xyz = leg[i].target.ptr<float>(0);
        old_phi[0] = leg[i].phi[0];
        old_phi[1] = leg[i].phi[1];
        old_phi[2] = leg[i].phi[2];
        old_phi[3] = leg[i].phi[3];


        old_w[0] = leg[i].w[0];
        old_w[1] = leg[i].w[1];
        old_w[2] = leg[i].w[2];
        old_w[3] = leg[i].w[3];

        //leg[i].x[3] = 
        x = leg[i].target.row(0).ptr<float>(0);
        y = leg[i].target.row(1).ptr<float>(0);
        z = leg[i].target.row(2).ptr<float>(0);


        if (sqrt((x[0] - x[3])*(x[0] - x[3])+(y[0] - y[3])*(y[0] - y[3])+(z[0] - z[3])*(z[0] - z[3])) >= sqrt(l1 * l1 + (l2 + l3)*(l2 + l3))*.99) {
            printf("OUT OF RANGE\n");
            //waitKey(0);
            usleep(1000000000000);
        }

        leg[i].phi[1] = atan2(z[3], y[3]) - acos(l1 / (sqrt(y[3] * y[3] + z[3] * z[3])));

        y[1] = l1 * cos(leg[i].phi[1]);
        x[1] = 0.0f;
        z[1] = l1 * sin(leg[i].phi[1]);

        //x[2] = x[1] + 

        float l24 = sqrt((x[3] - x[1])*(x[3] - x[1])+(y[3] - y[1])*(y[3] - y[1])+(z[3] - z[1])*(z[3] - z[1]));

        leg[i].phi[3] = leg[i].build_mode * M_PI - leg[i].build_mode * acos((l2 * l2 + l3 * l3 - l24 * l24) / (2.0 * l2 * l3));


        float yz24 = (z[3] - z[1]) / cos(leg[i].phi[1]);
        float yz24_alt = sqrt((z[3] - z[1])*(z[3] - z[1])+(y[3] - y[1])*(y[3] - y[1])); //this should equal above

        leg[i].phi[2] = atan2(x[3], yz24) + leg[i].build_mode * acos((l2 * l2 + l24 * l24 - l3 * l3) / (2.0 * l2 * l24));
        leg[i].phi[2] = atan2(sin(leg[i].phi[2]), cos(leg[i].phi[2]));

        x[2] = x[1] + l2 * sin(leg[i].phi[2]);
        y[2] = y[1] - l2 * sin(leg[i].phi[1]) * cos(leg[i].phi[2]);
        z[2] = z[1] + l2 * cos(leg[i].phi[1]) * cos(leg[i].phi[2]);

        float x4 = x[2] + l3 * sin(-leg[i].phi[3] + leg[i].phi[2]);
        float y4 = y[2] - l3 * sin(leg[i].phi[1]) * cos(-leg[i].phi[3] + leg[i].phi[2]);
        float z4 = z[2] + l3 * cos(leg[i].phi[1]) * cos(-leg[i].phi[3] + leg[i].phi[2]);



        leg[i].phi[0] = leg[i].phi[0] * leg[i].p0f;
        leg[i].phi[1] = leg[i].phi[1] * leg[i].p1f;

        for (int j = 0; j < 4; j++) {

            leg[i].w[j] = (leg[i].phi[j] - old_phi[j]) / ((float) t * 11200.0f / 1000000.0f);
            leg[i].a[j] = (leg[i].w[j] - old_w[j]) / ((float) t * 11200.0f / 1000000.0f);

        }
    }
}


//s_jog body

void Stag::sjb(uint8_t time) {//dynamic tics?? time == tic == 11.2ms

    hklx_packet_t *hklx_packet = hklx_packet_allocate(sizeof (s_jog_t)*16 + 1, BROADCAST_ID, CMD_S_JOG);
    s_jog_t *s_jog = (s_jog_t*) malloc(sizeof (s_jog_t) * 16 * sizeof (uint8_t));


    for (int i = 0; i < 4; i++) {

        s_jog[4 * i + 0].id = (uint8_t) (leg[i].root_id + 0);
        s_jog[4 * i + 1].id = (uint8_t) (leg[i].root_id + 1);
        s_jog[4 * i + 2].id = (uint8_t) (leg[i].root_id + 2);
        s_jog[4 * i + 3].id = (uint8_t) (leg[i].root_id + 3);

        s_jog[4 * i + 0].jog.value = (uint16_t) round((leg[i].phi[0]*180.0f / M_PI + 166.650f) / 0.3258064516f);
        s_jog[4 * i + 1].jog.value = (uint16_t) round((leg[i].phi[1]*180.0f / M_PI + 166.650f) / 0.3258064516f);
        s_jog[4 * i + 2].jog.value = (uint16_t) round((leg[i].phi[2]*180.0f / M_PI + 166.650f) / 0.3258064516f);
        s_jog[4 * i + 3].jog.value = (uint16_t) round((leg[i].phi[3]*180.0f / M_PI + 166.650f) / 0.3258064516f);


        s_jog[4 * i + 0].set.mode = 0;
        s_jog[4 * i + 1].set.mode = 0;
        s_jog[4 * i + 2].set.mode = 0;
        s_jog[4 * i + 3].set.mode = 0;

        s_jog[4 * i + 0].set.joginvalid = 0;
        s_jog[4 * i + 1].set.joginvalid = 0;
        s_jog[4 * i + 2].set.joginvalid = 0;
        s_jog[4 * i + 3].set.joginvalid = 0;


        s_jog[4 * i + 0].set.stopflag = 0;
        s_jog[4 * i + 1].set.stopflag = 0;
        s_jog[4 * i + 2].set.stopflag = 0;
        s_jog[4 * i + 3].set.stopflag = 0;

        s_jog[4 * i + 0].set.LedRed = (RED & leg[i].LED[0]) > 0;
        s_jog[4 * i + 1].set.LedRed = (RED & leg[i].LED[1]) > 0;
        s_jog[4 * i + 2].set.LedRed = (RED & leg[i].LED[2]) > 0;
        s_jog[4 * i + 3].set.LedRed = (RED & leg[i].LED[3]) > 0;

        s_jog[4 * i + 0].set.LedGreen = (GREEN & leg[i].LED[0]) > 0;
        s_jog[4 * i + 1].set.LedGreen = (GREEN & leg[i].LED[1]) > 0;
        s_jog[4 * i + 2].set.LedGreen = (GREEN & leg[i].LED[2]) > 0;
        s_jog[4 * i + 3].set.LedGreen = (GREEN & leg[i].LED[3]) > 0;

        s_jog[4 * i + 0].set.LedBlue = (BLUE & leg[i].LED[0]) > 0;
        s_jog[4 * i + 1].set.LedBlue = (BLUE & leg[i].LED[1]) > 0;
        s_jog[4 * i + 2].set.LedBlue = (BLUE & leg[i].LED[2]) > 0;
        s_jog[4 * i + 3].set.LedBlue = (BLUE & leg[i].LED[3]) > 0;


    }

    memcpy(hklx_packet->data + 1, s_jog, sizeof (s_jog_t) * 16 * sizeof (uint8_t));

    hklx_packet->data[0] = time;
    hklx.send(hklx_packet);
    hklx_packet_free(&hklx_packet);
    free(s_jog);

}

void Stag::body_jogn(Mat leg_xyz, int t) {//kinematic model

    Mat bcm = Mat::eye(3, 3, CV_32F);

    leg_xyz += lnp;

    float* leg_xyz_p = leg_xyz.ptr<float>(0);


    body.x = 0;
    body.y = 0;


    Mat bc = (Mat_<float>(3, 1) <<
            body.x,
            body.y,
            -body.h);

    Mat lxyz = (Mat_<float>(3, 4) <<
            body.bx, body.bx, -body.bx, -body.bx,
            body.by, -body.by, -body.by, body.by,
            0, 0, 0, 0);


    bcm = rot_xyz(bcm, body.yaw, 2); //yaw  in gcf?? NED??
    bcm = rot_xyz(bcm, body.pitch, 1); //pitch        
    bcm = rot_xyz(bcm, body.roll, 0); //roll

    lxyz = bcm*lxyz;

    lxyz.row(0) = lxyz.row(0) + bc.row(0);
    lxyz.row(1) = lxyz.row(1) + bc.row(1);
    lxyz.row(2) = lxyz.row(2) + bc.row(2);

    //float ang = 20.0 * M_PI / 180.0;
    float ang = 0.0 * M_PI / 180.0;

    leg[0].phi[0] = ang;
    leg[1].phi[0] = ang;
    leg[2].phi[0] = ang;
    leg[3].phi[0] = ang;

    Mat l1cf = rot_xyz(bcm, ang, 2);
    Mat l2cf = rot_xyz(bcm, M_PI - ang, 2);
    Mat l3cf = rot_xyz(bcm, M_PI - ang, 2);
    Mat l4cf = rot_xyz(bcm, ang, 2);

    leg[0].target.col(3) = l1cf.inv()*(leg_xyz.col(0) - lxyz.col(0)); //NAMING //SHOULDRS OR SOMETHING
    leg[1].target.col(3) = l2cf.inv()*(leg_xyz.col(1) - lxyz.col(1));
    leg[2].target.col(3) = l3cf.inv()*(leg_xyz.col(2) - lxyz.col(2));
    leg[3].target.col(3) = l4cf.inv()*(leg_xyz.col(3) - lxyz.col(3));


    ikb(t);


    while ((double) tic * 11200.0 > (((double) getTickCount() - tic_time)*1000000. / getTickFrequency())) {
        //while((double)tic * 10000.0 > (((double) getTickCount() - tic_time)*1000000. / getTickFrequency())){
    }
    //NOTHING CAN HAPPEN HERE EVER

    sjb(t);
    tic_time = (double) getTickCount();


}

float Stag::get_phase(int id) {

    switch (id) {
        case 0:
            return phase + phase1 - floor(phase + phase1);
            break;
        case 1:
            return phase + phase2 - floor(phase + phase2);
            break;
        case 2:
            return phase + phase3 - floor(phase + phase3);
            break;
        case 3:
            return phase + phase4 - floor(phase + phase4);
            break;
    }
}

int main() {


    sixmotion six;

    Stag stag;


    hklx_packet_t *hklx_packet;
    hklx_packet = hklx_packet_allocate(3, BROADCAST_ID, CMD_RAM_WRITE);

    hklx_packet->data[0] = 0x34;
    hklx_packet->data[1] = 0x01;
    hklx_packet->data[2] = 0x60; //torque on
    stag.hklx.send(hklx_packet);
    hklx_packet_free(&hklx_packet);


    Mat leg_xyz = Mat::zeros(3, 4, CV_32F);
    stag.tic_time = (double) getTickCount();

    stag.body_jogn(leg_xyz, 200);
    usleep(400 * 10000);
    _sixmotion sixm;
    stag.tic_time = (double) getTickCount();

    float v = 0.1; //velocity in m/s

    stag.dp = (float) stag.tic * (v / (2.0f * stag.stride / stag.gait_ratio)) * 11200.0f / 1000000.0f;
    //dp is the velocity

    while (1) {
        sixm = six.state();


        leg_xyz = stag.phase_gait(COS_GAIT);
        Mat bcm = Mat::eye(3, 3, CV_32F);
        float direction =0.0f;//walking direction in RAD anti clockwise positive about Z
        leg_xyz = rot_xyz(bcm, direction, 2)*leg_xyz;
        
        
        
        //stag.body_rj(leg_xyz, stag.tic);
        stag.body_jogn(leg_xyz, stag.tic);
        stag.phase += stag.dp;
    }
    return 0;
}