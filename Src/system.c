/**********************************************************************************************************************
 * @file    system.c
 * @author  Simon Benoit
 * @date    02-12-2017
 * @brief
 *********************************************************************************************************************/

/* Includes ---------------------------------------------------------------------------------------------------------*/

#include "stm32l0xx_hal.h"
#include "pid.h"
#include "menu.h"

/* Local Defines ----------------------------------------------------------------------------------------------------*/
#define NB_OF_TEMP_POINTS   128
/* Local Typedefs ---------------------------------------------------------------------------------------------------*/

typedef struct _SYS_Profile_e
{
    uint16_t PreHeatTime;
    uint16_t PreHeatTemp;
    uint16_t SoakTime;
    uint16_t SoakTemp;
    uint16_t ReflowTime;
    uint16_t ReflowTemp;
    uint16_t CoolingTime;
    uint16_t TotalTime;
    uint16_t SetpointArray[NB_OF_TEMP_POINTS];
}SYS_Profile_e;

/* Forward Declarations ---------------------------------------------------------------------------------------------*/

/* Local Constants --------------------------------------------------------------------------------------------------*/

/* Local Variables --------------------------------------------------------------------------------------------------*/

static uint16_t SYS_PreHeatTime = 30;
static uint16_t SYS_PreHeatTemp = 80;
static uint16_t SYS_SoakTime = 60;
static uint16_t SYS_SoakTemp = 120;
static uint16_t SYS_ReflowTime = 180;
static uint16_t SYS_ReflowTemp = 245;
static uint16_t SYS_CoolingTime = 60;
static bool     SYS_Started;

SYS_Profile_e profile1;


// Structure to strore PID data and pointer to PID structure
struct pid_controller ctrldata;
PID_t pid;

// Control loop input,output and setpoint variables
float input = 0, output = 0;
float setpoint = 0;

// Control loop gains
float kp = 2.5, ki = 1.0, kd = 1.0;

/* Local Functions --------------------------------------------------------------------------------------------------*/

void SYS_GenerateProfile(SYS_Profile_e *profile)
{
    uint16_t x0,x1;
    uint16_t y0,y1;
    float ratio;
    float a, b;

    profile->PreHeatTemp = SYS_PreHeatTemp;
    profile->PreHeatTime = SYS_PreHeatTime;
    profile->SoakTemp = SYS_SoakTemp;
    profile->SoakTime = SYS_SoakTime;
    profile->ReflowTemp = SYS_ReflowTemp;
    profile->ReflowTime = SYS_ReflowTime;
    profile->CoolingTime = SYS_CoolingTime;
    profile->TotalTime = profile->PreHeatTime + profile->SoakTime + profile->ReflowTime + profile->CoolingTime;

    ratio = (float)profile->PreHeatTime/(float)profile->TotalTime;
    x0 = 0;
    y0 = 0;
    x1 = ratio *NB_OF_TEMP_POINTS;
    y1 = profile->PreHeatTemp;
    a = (float)(y1-y0)/(float)(x1-x0);
    b = y1 - (a*x1);
    for ( int x=x0; x<x1; x++ )
    {
        profile->SetpointArray[x] = a*x + b;
    }

    ratio = (float)profile->SoakTime/(float)profile->TotalTime;
    x0 = x1;
    y0 = profile->PreHeatTemp;
    x1 = ratio * NB_OF_TEMP_POINTS + x0;
    y1 = profile->SoakTemp;
    a = (float)(y1-y0)/(float)(x1-x0);
    b = y1 - (a*x1);
    for ( int x=x0; x<x1; x++ )
    {
        profile->SetpointArray[x] = a*x + b;
    }

    ratio = (float)profile->ReflowTime/(float)profile->TotalTime;
    x0 = x1;
    y0 = profile->SoakTemp;
    x1 = ratio * NB_OF_TEMP_POINTS /2 + x0;
    y1 = profile->ReflowTemp;
    a = (float)(y1-y0)/(float)(x1-x0);
    b = y1 - (a*x1);
    for ( int x=x0; x<x1; x++ )
    {
        profile->SetpointArray[x] = a*x + b;
    }
    x0 = x1;
    x1 += (x1/2);
    a = (float)(y0 - y1)/(float)(x1-x0);
    b = y0 - (a*x1);
    for ( int x=x0; x<x1; x++ )
    {
        profile->SetpointArray[x] = a*x + b;
    }

    x0 = x1;
    x1 = 128;
    y1 = 0;
    a = (float)(y1-y0)/(float)(x1-x0);
    b = y1 - (a*x1);
    for ( int x=x0; x<x1; x++ )
    {
        profile->SetpointArray[x] = a*x + b;
    }
}

/* Global Functions -------------------------------------------------------------------------------------------------*/

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_Start()
{
    SYS_GenerateProfile(&profile1);
    MENU_PrintDots(profile1.SetpointArray, 128);
    SYS_Started = true;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_Stop()
{
    SYS_Started = false;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetPreHeatTimePtr (uint16_t** val)
{
    *val = &SYS_PreHeatTime;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetPreHeatTempPtr (uint16_t** val)
{
  *val = &SYS_PreHeatTemp;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetSoakTimePtr    (uint16_t** val)
{
  *val = &SYS_SoakTime;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetSoakTempPtr    (uint16_t** val)
{
  *val = &SYS_SoakTemp;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetReflowTimePtr  (uint16_t** val)
{
  *val = &SYS_ReflowTime;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetReflowTempPtr  (uint16_t** val)
{
  *val = &SYS_ReflowTemp;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void SYS_GetCoolingTimePtr (uint16_t** val)
{
  *val = &SYS_CoolingTime;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
uint16_t SYS_GetTotalTime()
{
    return  (SYS_PreHeatTime + SYS_SoakTime +  SYS_ReflowTime + SYS_CoolingTime);
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  * @brief
  *
  * @param  none
  *
  * @retval none
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
bool SYS_IsSystemStarted()
{
    return SYS_Started;
}

void SYS_Init()
{
    SYS_Started = false;

	// Prepare PID controller for operation
	pid = pid_create(&ctrldata, &input, &output, &setpoint, kp, ki, kd);
	// Set controler output limits from 0 to 100
	pid_limits(pid, 0, 100);
	// Allow PID to compute and change output
	pid_auto(pid);

}

void SYS_Process()
{
    float actualTemp;
    if(SYS_Started)
    {
        // Get actual temperature

		// Check if need to compute PID
		if (pid_need_compute(pid)) {
			// Read process feedback
			input = actualTemp;
			// Compute new PID output value
			pid_compute(pid);
			//Change actuator value
			//Output is updated here so use it !
		}
    }
}

uint16_t SYS_GetPreHeatTime() { return SYS_PreHeatTime; }
uint16_t SYS_GetPreHeatTemp() { return SYS_PreHeatTemp; }
uint16_t SYS_GetSoakTime()    { return SYS_SoakTime; }
uint16_t SYS_GetSoakTemp()    { return SYS_SoakTemp; }
uint16_t SYS_GetReflowTime()  { return SYS_ReflowTime; }
uint16_t SYS_GetReflowTemp()  { return SYS_ReflowTemp ; }
uint16_t SYS_GetCoolingTime() { return SYS_CoolingTime; }
