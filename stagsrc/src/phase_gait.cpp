/* 
 * File:   stag.cpp
 * Author: toner
 * 
 * Created on 21 March 2013, 11:52 AM
 */

#include "Stag.h"
#include <cstdlib>

Mat Stag::phase_gait(int gait_id) {

    float h = pgh; //phase gait height

    Mat lrb_xyz = Mat::zeros(3, 4, CV_32F);

    //float *Gr_p = Gr.ptr<float>(0);
    float *lrb_xyz_p = lrb_xyz.ptr<float>(0);

    for (int i = 0; i < 4; i++) {
        float Phase = get_phase(i);
        /*
        
        if (fs[i] == 0) {
            if (Phase < gait_ratio) {
                //lrb_xyz_p[4*2+i] = 0;
                //lrb_xyz_p[] = 0;

            } else {
                lrb_xyz_p[4*2+i] = -(h * (1.0 + cos(-M_PI + 2.0 * M_PI * (Phase - gait_ratio) / (1 - gait_ratio))) / 2.0);
                //lrb_xyz_p[4*2+i] = -(h * (1.0 + cos(-M_PI + 2.0 * M_PI * (Phase - gait_ratio) / (1 - gait_ratio))) / 2.0);
            }
        }
         */

        switch (gait_id) {


            case COS_GAIT:

                if (Phase < gait_ratio) {

                    if (fs[i] == 0) {
                        lrb_xyz_p[i] = stride - stride * 2 * Phase / gait_ratio;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    } else {
                        lrb_xyz_p[i] = 0;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    }
                } else {



                    if (fs[i] == 0) {
                        lrb_xyz_p[i] = -stride + stride * 2 * (Phase - gait_ratio) / (1 - gait_ratio);
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = -(h * (1.0 + cos(-M_PI + 2.0 * M_PI * (Phase - gait_ratio) / (1 - gait_ratio))) / 2.0);
                    } else {
                        if (Phase > (1.0f - gait_ratio) / 2 + gait_ratio) {
                            lrb_xyz_p[i] = -stride + stride * 2 * (Phase - gait_ratio) / (1 - gait_ratio);

                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -(h * (1.0 + cos(-M_PI + 2.0 * M_PI * (Phase - gait_ratio) / (1 - gait_ratio))) / 2.0);
                        } else {

                            //lrb_xyz_p[i] = -stride + stride * 2 * (Phase - gait_ratio) / (1 - gait_ratio);
                            lrb_xyz_p[i] = 0;
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -(h * (1.0 + cos(-M_PI + 2.0 * M_PI * (Phase - gait_ratio) / (1 - gait_ratio))) / 2.0);
                        }
                    }
                    //printf("%f\n",sin(M_PI * (Phase - gait_ratio) / (1 - gait_ratio))-cos(-M_PI_2 + M_PI * (Phase - gait_ratio) / (1 - gait_ratio)));
                }

                break;

            case OBLONG_GAIT:

                if (Phase < gait_ratio) {

                    if (fs[i] == 0) {
                        lrb_xyz_p[i] = stride - stride * 2 * Phase / gait_ratio;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    } else {
                        lrb_xyz_p[i] = 0;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    }

                } else {

                    //((1.0 - gait_ratio)/3.0) 
                    if (fs[i] == 0) {
                        //lift
                        if (gait_ratio <= Phase && Phase < gait_ratio + (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = -stride;
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0);

                        }

                        //move
                        if (gait_ratio + (1.0 - gait_ratio) / 3.0 <= Phase && Phase < gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = -stride + stride * 2.0 * (Phase - gait_ratio - (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h;

                        }

                        //drop
                        if (gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0 <= Phase) {

                            lrb_xyz_p[i] = stride;
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h + h * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);

                        }
                    } else {

                        //lift
                        if (gait_ratio <= Phase && Phase < gait_ratio + (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = 0;
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0);

                        }

                        //move
                        if (gait_ratio + (1.0 - gait_ratio) / 3.0 <= Phase && Phase < gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = stride * (Phase - gait_ratio - (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h;

                        }

                        //drop
                        if (gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0 <= Phase) {

                            lrb_xyz_p[i] = stride;
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h + h * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);

                        }


                    }



                }

                break;

            case OBLONG_GAIT_COS:

                if (Phase < gait_ratio) {

                    if (fs[i] == 0) {
                        lrb_xyz_p[i] = stride - stride * 2 * Phase / gait_ratio;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    } else {
                        lrb_xyz_p[i] = 0;
                        lrb_xyz_p[4 + i] = 0;
                        lrb_xyz_p[4 * 2 + i] = 0;
                    }

                } else {

                    //((1.0 - gait_ratio)/3.0) 
                    if (fs[i] == 0) {
                        //lift
                        if (gait_ratio <= Phase && Phase < gait_ratio + (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = -stride;
                            lrb_xyz_p[4 + i] = 0;

                            lrb_xyz_p[4 * 2 + i] = h * (-1 + cos(M_PI * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0))) / 2.0;

                            //lrb_xyz_p[4*2+i] = -h * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0);

                        }

                        //move
                        if (gait_ratio + (1.0 - gait_ratio) / 3.0 <= Phase && Phase < gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = -stride + stride * 2.0 * (Phase - gait_ratio - (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h;

                        }

                        //drop
                        if (gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0 <= Phase) {

                            lrb_xyz_p[i] = stride;
                            lrb_xyz_p[4 + i] = 0;
                            //lrb_xyz_p[4*2+i] = -h + h * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);

                            lrb_xyz_p[4 * 2 + i] = -h * (1 + cos(M_PI * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0))) / 2.0;

                        }
                    } else {

                        //lift
                        if (gait_ratio <= Phase && Phase < gait_ratio + (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = 0;
                            lrb_xyz_p[4 + i] = 0;
                            //lrb_xyz_p[4*2+i] = -h * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 * 2 + i] = h * (-1 + cos(M_PI * (Phase - gait_ratio) / ((1.0 - gait_ratio) / 3.0))) / 2.0;

                        }

                        //move
                        if (gait_ratio + (1.0 - gait_ratio) / 3.0 <= Phase && Phase < gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0) {

                            lrb_xyz_p[i] = stride * (Phase - gait_ratio - (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 + i] = 0;
                            lrb_xyz_p[4 * 2 + i] = -h;

                        }

                        //drop
                        if (gait_ratio + 2.0 * (1.0 - gait_ratio) / 3.0 <= Phase) {

                            lrb_xyz_p[i] = stride;
                            lrb_xyz_p[4 + i] = 0;
                            //lrb_xyz_p[4*2+i] = -h + h * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0);
                            lrb_xyz_p[4 * 2 + i] = -h * (1 + cos(M_PI * (Phase - gait_ratio - 2.0 * (1.0 - gait_ratio) / 3.0) / ((1.0 - gait_ratio) / 3.0))) / 2.0;

                        }


                    }



                }

                break;


        }






    }



    //first step stuff

    if (floor(phase + phase1) != floor(phase + dp + phase1))
        fs[0] = 0;

    if (floor(phase + phase2) != floor(phase + dp + phase2))
        fs[1] = 0;

    if (floor(phase + phase3) != floor(phase + dp + phase3))
        fs[2] = 0;

    if (floor(phase + phase4) != floor(phase + dp + phase4))
        fs[3] = 0;


    return lrb_xyz;

}
