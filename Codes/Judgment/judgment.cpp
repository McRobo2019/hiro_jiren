/******************************************************************************
 *  Author: Koaru Ota

--- basic structure ---

*****************************************************************************/

#include <stdlib.h>
#include "ev3api.h"
#include "judgment.hpp"

Judgment::Judgment() {

}

void Judgment::init() {

  ZONE               = START_ZONE;
  DRIVE_MODE         = LINE_TRACE;
  ON_LINE_MODE       = ON_THE_LEFT_EDGE;
  PRE_ON_LINE_MODE   = ON_THE_LEFT_EDGE;
  TEST_MODE          = MODE_00;

  on_line            = true;
  left_line          = false;
  right_line         = false;
  lost_line          = false;
  line_to_map        = false; //181108
  line_trace_mode    = true;

  mMax_Forward = 10;
  det_navi_log = 0;
  re_start     = false; //181112


  gAve_line_val->init();
  gAve_yaw_angle_500->init(); //20181108
  gNavi->init();


}


void Judgment::set_drive_mode_LT(){
  DRIVE_MODE = LINE_TRACE;
  line_trace_mode = true;
}

void Judgment::set_drive_mode_TK(){
  DRIVE_MODE = TRACK;
  line_trace_mode = false;
}

void Judgment::set_drive_mode_DB(){
  DRIVE_MODE = DEBUG;
  line_trace_mode = false;
}



void Judgment::run() {
  //  float yaw_time;

  static float ref_odo;
  //  static float ref_angle;
  /*
  static float dif_odo;

  static int   ref_forward;
  static float acl_forward;
  */
  static int ref_clock;


  //  LINE_VAL = 50; //for debug
  ave_line_val = gAve_line_val->average_500(LINE_VAL);
  ave_yaw_angle_500 = gAve_yaw_angle_500->average_500(YAW_ANGLE);


  if(DRIVE_MODE == LINE_TRACE){
    line_trace_mode = true;

    //    gNavi->run(mLinevalue, mOdo, (int)mVelocity, mYawrate, mAve_yaw_angle, X_POS, Y_POS, (int)mPre_50mm_x, (int)mPre_50mm_y);
   gNavi->run(LINE_VAL, ODO, (int)mVelocity, YAW_ANGLE, X_POS, Y_POS, PRE_X_POS, PRE_Y_POS);
    
    mRef_Omega      = gNavi->ref_omega;
    mMax_Omega      = gNavi->max_omega;
    mMin_Omega      = gNavi->min_omega;
    target_velocity = gNavi->target_velocity;

    target_omega = gLine_Trace->line_trace_omega(LINE_VAL, mRef_Omega, mMax_Omega, mMin_Omega);
  }
  else if(DRIVE_MODE == TRACK){
    line_trace_mode    = false;

    switch(TEST_MODE){

    case MODE_00:
      det_navi_log = 0;
      target_velocity = 0;
      target_omega    = 0.0;

      ref_clock = SYS_CLK + 499; //0.5sec
      ref_odo   = ODO + 4399;
      TEST_MODE = MODE_01;

      det_navi_log = ref_odo;
      break;

    case MODE_01:
      det_navi_log = 100000+ref_odo;
      target_velocity = 0;
      target_omega    = 0.0;

      if(SYS_CLK > ref_clock){
	TEST_MODE = MODE_02;
	ref_clock = SYS_CLK;
      }
      break;

    case MODE_02:
      det_navi_log = 300000+ref_odo;

      target_omega    = 0.0;
      //      target_velocity = 200*(SYS_CLK - ref_clock);
      target_velocity = 0.2*(SYS_CLK - ref_clock);
      if(target_velocity > 399){
	target_velocity = 400;
	TEST_MODE = MODE_03;
      }
      break;

    case MODE_03:
      det_navi_log = 300000+ref_odo;
      target_velocity = 400;
      target_omega    = 0.0;
      if(ODO > ref_odo){
	TEST_MODE = MODE_04;
	ref_odo   = ODO + 401;
      }
      break;

    case MODE_04:
      det_navi_log = 400000+ref_odo;
      target_velocity = ref_odo - ODO;
      target_omega    = 0.0;

      if(target_velocity > 400){
	target_velocity = 400;
      }else if(target_velocity < 0){
	target_velocity = 0;
      }

      if(ODO > ref_odo){
	TEST_MODE = MODE_05;
      }
      break;

    case MODE_05:
      det_navi_log = 500000+ref_odo;
      target_velocity = 0;
      target_omega    = 0.0;
      break;

    default:
      break;
    }


    
  }
  else if(DRIVE_MODE == DEBUG){
    line_trace_mode    = false;

    switch(TEST_MODE){
    case MODE_00:
      det_navi_log = 0;
      target_velocity = 0;
      target_omega    = 0.0;

      ref_clock = SYS_CLK + 500; //0.5sec
      ref_odo   = ODO + 1200;
      TEST_MODE = MODE_01;

      det_navi_log = ref_odo;
      break;

    case MODE_01:
      det_navi_log = 1;
      target_velocity = 0;
      target_omega    = 0.0;

      if(SYS_CLK > ref_clock){
	TEST_MODE = MODE_02;
	ref_clock = SYS_CLK;
      }
      break;

    case MODE_02:
      det_navi_log = 2;
      target_omega    = 0.0;
      target_velocity = 0.2*(SYS_CLK - ref_clock);
      if(target_velocity >= 400){
	target_velocity = 400;
	TEST_MODE = MODE_03;
      }

      break;

    case MODE_03:
      det_navi_log = 3;
      target_velocity = 400;
      target_omega    = 0.0;

      if(ODO > ref_odo){
	TEST_MODE = MODE_04;
	ref_odo   = ODO;
      }

      break;

    case MODE_04:
      det_navi_log = 4;
      target_velocity = 400;
      target_omega    = 0.4 * PAI * (ODO - ref_odo)/800.0;
      if (target_omega >= 0.4 * PAI){
	TEST_MODE = MODE_05;	
	ref_odo = 3600;
      }

      break;

    case MODE_05:
      det_navi_log = 5;
		target_velocity = 400;
		target_omega = 0.4 * PAI;
		
		if (ODO > ref_odo) {
			TEST_MODE = MODE_06;
			ref_odo = ODO + 800;
		}
      break;

    case MODE_06:
      det_navi_log = 6;
		target_velocity = 400;
		target_omega = 0.4 * PAI * (-ODO + ref_odo)/800.0;
		if (target_omega <= 0) {
			TEST_MODE = MODE_07;
			ref_odo = 5200;
		}

      break;

    case MODE_07:
      det_navi_log = 7;
		target_velocity = 400;
		target_omega = 0;
		if (ODO > ref_odo) {
			TEST_MODE = MODE_08;
			ref_odo = ODO + 400;
			ref_clock = SYS_CLK + 2000;
		}

      break;

    case MODE_08:
      det_navi_log = 8;
		target_velocity = 0.2 * (ref_clock - SYS_CLK);
		target_omega = 0;
		if (target_velocity <= 0) {
			TEST_MODE = MODE_09;
		}

      break;

    case MODE_09:
      det_navi_log = 9;
		target_velocity = 0;
		target_omega = 0.0;

      break;




    default:
      break;
    }
  }
}

void Judgment::set_in_data(bool    green_flag,
			   float   velocity,
			   float   pre_velo_0p5sec,
			   float   yawrate,
			   float   ave_angle,
			   int     robo_tail_angle,
			   bool    robo_stop,
			   bool    robo_forward,
			   bool    robo_back,
			   bool    robo_turn_left,
			   bool    robo_turn_right,
			   int16_t sonar_dis){

  mGreen_flag      = green_flag;
  mVelocity        = velocity;
  mPre_velo_0p5sec = pre_velo_0p5sec;
  mYawrate         = yawrate;
  mAve_yaw_angle   = ave_angle;

  mTail_angle      = robo_tail_angle;
  mRobo_stop       = robo_stop;
  mRobo_forward    = robo_forward;
  mRobo_back       = robo_back;
  mRobo_turn_left  = robo_turn_left;
  mRobo_turn_right = robo_turn_right;
  mSonar_dis       = sonar_dis;
}



