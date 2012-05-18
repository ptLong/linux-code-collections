/************************************************************************
 *
 * NAME
 *  cruise-sm.h
 *
 * State machine: cruise_control
 *
 * This file was automatically generated from: cruise.smc
 *
 * DO NOT EDIT THIS FILE. YOUR CHANGES WILL BE
 * OVER-WRITTEN THE NEXT TIME THAT DDL IS RUN.
 *
 * GENERATED
 *  Tue May 22 16:18:00 2007
 *
 ************************************************************************/

#if !defined( STM_H )
#    include "stm.h"
#endif

typedef enum
{
    ccin_enable_cruise                  = 0,
    ccin_disable_cruise                 = 1,
    ccin_start_incr_speed               = 2,
    ccin_stop_incr_speed                = 3,
    ccin_apply_brake                    = 4,
    ccin_resume_speed                   = 5,
    ccin_prev_speed_reached             = 6

} cruise_controlInput;

void ccac_read_rotation_rate( void );
void ccac_dis_maint_const_spd( void );
void ccac_enab_increase_spd( void );
void ccac_dis_increase_spd( void );
void ccac_enab_maint_const_spd( void );
void ccac_do_nothing( void );
void ccac_enab_ret_to_prev_spd( void );
void ccac_dis_ret_to_prev_spd( void );

extern STM_StateMachine cruise_controlSm;