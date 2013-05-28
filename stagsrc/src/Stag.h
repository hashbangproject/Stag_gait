/* 
 * File:   stag.h
 * Author: toner
 *
 * Created on 21 March 2013, 11:52 AM
 */

#ifndef STAG_H
#define	STAG_H
//#include "hklx.h"
#include "HKLX.h"
#include "sixmotion.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define LEG_0				0
#define LEG_1				8
#define LEG_2				16
#define LEG_3				20
#define NECK				13

#define ELLIPSE_GAIT 0
#define COS_GAIT 1
#define OBLONG_GAIT 2
#define OBLONG_GAIT_COS 3
#define JUMP_GAIT 4

typedef struct {
    float phi[4];
    float w[4];
    float a[4];


    uint8_t root_id;

    float build_mode;

    float p0f;
    float p1f;

    Mat target;
    //Mat read;

    float l1;
    float l2;
    float l3;

    uint8_t LED[4];

} _leg;

typedef struct {
    float bx;//body x dim in m
    float by;//body y dim in m
    float h;//body height from ground
    float x;//body center x shift
    float y;//body center y shift
    float yaw;//body yaw, NED -- about z axis
    float pitch;//body pitch, NED --about y axit 
    float roll;//body roll, NED --about x axis


} _body;

class Stag {
public:
    Stag();
    Stag(const Stag& orig);
    virtual ~Stag();


    double tic_time;

    float pgh;//phase gait height, height of gait profile
    float l1;//leg lengths
    float l2;
    float l3;
    float stride;//gait stride, full stride is 2*stride

    float neck_yaw;
    float neck_pitch;

    float phase1;//leg phase leg 1 is front right
    float phase2;//leg phase leg 2 is front left
    float phase3;//leg phase leg 3 is bottom left
    float phase4;//leg phase leg 4 is bottom right

    int fs[4];
    float dp;//Dphase/Dtick

    int tic;//
    float phase;//global phase

    float gait_ratio;//gait ratio or duty


    HKLX hklx;

    _leg leg[4];
    _hklx DRS[254];

    _body body;

    Mat lnp; //leg neutral position

    void sjb(uint8_t time);
    float get_phase(int id);
    Mat phase_gait(int gait_id);
    void body_jogn(Mat leg_xyz, int t);
    void ikb(int t);


private:

};

#endif	/* STAG_H */

