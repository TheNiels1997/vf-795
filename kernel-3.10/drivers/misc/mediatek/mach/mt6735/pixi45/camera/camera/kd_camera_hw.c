#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

#include <mach/mt_pm_ldo.h>
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)    pr_debug(PFX  fmt, ##args)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)   pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   pr_debug(PFX  fmt, ##args); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

/*
#ifndef BOOL
typedef unsigned char BOOL;
#endif
*/

extern char Front_Camera_Name[256];
/* Mark: need to verify whether ISP_MCLK1_EN is required in here //Jessy @2014/06/04*/
extern void ISP_MCLK1_EN(BOOL En);

int cntVCAMD =0;
int cntVCAMA =0;
int cntVCAMIO =0;
int cntVCAMAF =0;
int cntVCAMD_SUB =0;

static DEFINE_SPINLOCK(kdsensor_pw_cnt_lock);


bool _hwPowerOn(MT65XX_POWER powerId, int powerVolt, char *mode_name){

	if( hwPowerOn( powerId,  powerVolt, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD+= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA+= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO+= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF+= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB+= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

bool _hwPowerDown(MT65XX_POWER powerId, char *mode_name){

	if( hwPowerDown( powerId, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD-= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA-= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO-= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF-= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB-= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

void checkPowerBeforClose( char* mode_name)
{

	int i= 0;

	PK_DBG("[checkPowerBeforClose]cntVCAMD:%d, cntVCAMA:%d,cntVCAMIO:%d, cntVCAMAF:%d, cntVCAMD_SUB:%d,\n",
		cntVCAMD, cntVCAMA,cntVCAMIO,cntVCAMAF,cntVCAMD_SUB);


	for(i=0;i<cntVCAMD;i++)
		hwPowerDown(CAMERA_POWER_VCAM_D,mode_name);
	for(i=0;i<cntVCAMA;i++)
		hwPowerDown(CAMERA_POWER_VCAM_A,mode_name);
	for(i=0;i<cntVCAMIO;i++)
		hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name);
	for(i=0;i<cntVCAMAF;i++)
		hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name);
	for(i=0;i<cntVCAMD_SUB;i++)
		hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name);

	 cntVCAMD =0;
	 cntVCAMA =0;
	 cntVCAMIO =0; 

	 cntVCAMAF =0;
	 cntVCAMD_SUB =0;

}



int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{

u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3


u32 pinSet[3][8] = {
                        //for main sensor
                     {  CAMERA_CMRST_PIN, // The reset pin of main sensor uses GPIO10 of mt6306, please call mt6306 API to set
                        CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,              /* ON state */
                        GPIO_OUT_ZERO,             /* OFF state */
                        CAMERA_CMPDN_PIN,
                        CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     },
                     //for sub sensor
                     {  CAMERA_CMRST1_PIN,
                        CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                        CAMERA_CMPDN1_PIN,
                        CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     },
                     //for main_2 sensor
                     {  GPIO_CAMERA_INVALID,
                        GPIO_CAMERA_INVALID,   /* mode */
                        GPIO_OUT_ONE,               /* ON state */
                        GPIO_OUT_ZERO,              /* OFF state */
                        GPIO_CAMERA_INVALID,
                        GPIO_CAMERA_INVALID,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     }
                   };



    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }


    //power ON
    if (On) {

        ISP_MCLK1_EN(1);

        PK_DBG("[PowerON]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);

    
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670_MIPI_RAW, currSensorName)) && (pinSetIdx == 0))
	{
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_OV5670_MIPI_RAW \n");

		if (GPIO_CAMERA_INVALID != CAMERA_CMPDN_PIN) {
	            if(mt_set_gpio_mode(CAMERA_CMPDN_PIN, CAMERA_CMPDN_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR ov5670] set gpio pwdn mode failed!\n");}
	            if(mt_set_gpio_dir(CAMERA_CMPDN_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR ov5670] set gpio pwdn dir failed!\n");}
	            if(mt_set_gpio_out(CAMERA_CMPDN_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR ov5670] set gpio pwdn failed!\n");}
		}

	        if (GPIO_CAMERA_INVALID != CAMERA_CMRST_PIN) {
	            if(mt_set_gpio_mode(CAMERA_CMRST_PIN, CAMERA_CMRST_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR ov5670]set gpio mode failed!\n");}
	            if(mt_set_gpio_dir(CAMERA_CMRST_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR ov5670] set gpio reset dir failed!\n");}
	            if(mt_set_gpio_out(CAMERA_CMRST_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR ov5670] set gpio reset failed!\n");}
		}

	        //DOVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR ov5670] Fail to enable IO_VDD power\n");
	            goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

		if(mt_set_gpio_out(CAMERA_CMPDN_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR ov5670] set gpio pwdn failed!\n");}

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR ov5670] Fail to enable AVDD power\n");
	            goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

	        //DVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200, mode_name))
	        {
	             PK_DBG("[CAMERA SENSOR ov5670] Fail to enable DVDD power\n");
	             goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(5);

	        //enable active sensor
	        if(mt_set_gpio_out(CAMERA_CMRST_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR ov5670] set gpio reset failed!\n");}
	        mdelay(1);
        }
	else  if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV, currSensorName)) && (pinSetIdx == 1))
        {

		//disable inactive sensor

	        if ((GPIO_CAMERA_INVALID != CAMERA_CMRST_PIN) && (GPIO_CAMERA_INVALID != CAMERA_CMPDN_PIN)) {
	             if(mt_set_gpio_mode(CAMERA_CMRST_PIN, CAMERA_CMRST_PIN_M_GPIO)){PK_DBG("[cam sensor gc0329]set gpio mode failed!! \n");}
	             if(mt_set_gpio_mode(CAMERA_CMPDN_PIN, CAMERA_CMPDN_PIN_M_GPIO)){PK_DBG("[cam sensor gc0329]set gpio mode failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMRST_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc0329] set gpio dir failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMPDN_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc0329]set gpio dir failed!! \n");}
	             if(mt_set_gpio_out(CAMERA_CMRST_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc0329] set gpio failed!! \n");}		         
		     if(mt_set_gpio_out(CAMERA_CMPDN_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc0329]set gpio failed!! \n");}
	        }
	
	        if (GPIO_CAMERA_INVALID != CAMERA_CMPDN1_PIN) {
		    if(mt_set_gpio_mode(CAMERA_CMPDN1_PIN, CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR gc0329] set gpio pwdn mode failed!! \n");}
	            if(mt_set_gpio_dir(CAMERA_CMPDN1_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc0329] set gpio pwdn dir failed!! \n");}
	            if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR gc0329] set gpio pwdn failed!! \n");}
                }

                if (GPIO_CAMERA_INVALID != CAMERA_CMRST1_PIN) {
	            if(mt_set_gpio_mode(CAMERA_CMRST1_PIN, CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR gc0329] set gpio reset mode failed!! \n");}
	            if(mt_set_gpio_dir(CAMERA_CMRST1_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc0329] set gpio reset dir failed!! \n");}
		    if(mt_set_gpio_out(CAMERA_CMRST1_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc0329] set gpio reset failed!! \n");}    
	        }

	   	PK_DBG("current open camera is gc0329 sensor,pinSetIdx=%d\n\r",pinSetIdx);
       		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR gc0329] Fail to enable IO_VDD power\n");		            
			goto _kdCISModulePowerOn_exit_;
		} 

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR gc0329] Fail to enable AVDD power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}

	        if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc0329] set gpio pwdn failed!! \n");}
		mdelay(2);

	        if(mt_set_gpio_out(CAMERA_CMRST1_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	        mdelay(2);
	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2145_YUV,currSensorName)) && (pinSetIdx == 1))
        {
		
		PK_DBG("\nTCL: Enter gc2145 sensor,pinSetIdx=%d\n\r",pinSetIdx);

		/*disable inactive sensor*/
		if ((GPIO_CAMERA_INVALID != CAMERA_CMRST_PIN) && (GPIO_CAMERA_INVALID != CAMERA_CMPDN_PIN)) {
	             if(mt_set_gpio_mode(CAMERA_CMRST_PIN, CAMERA_CMRST_PIN_M_GPIO)){PK_DBG("[cam sensor gc2145]set gpio mode failed!! \n");}
	             if(mt_set_gpio_mode(CAMERA_CMPDN_PIN, CAMERA_CMPDN_PIN_M_GPIO)){PK_DBG("[cam sensor gc2145]set gpio mode failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMRST_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc2145] set gpio dir failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMPDN_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc2145]set gpio dir failed!! \n");}
	             if(mt_set_gpio_out(CAMERA_CMRST_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc2145] set gpio failed!! \n");}		         
		     if(mt_set_gpio_out(CAMERA_CMPDN_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc2145]set gpio failed!! \n");}
	        }

		if (GPIO_CAMERA_INVALID != CAMERA_CMPDN1_PIN) {
		    if(mt_set_gpio_mode(CAMERA_CMPDN1_PIN, CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR gc2145] set gpio pwdn mode failed!! \n");}
	            if(mt_set_gpio_dir(CAMERA_CMPDN1_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc2145] set gpio pwdn dir failed!! \n");}
	            if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR gc2145] set gpio pwdn failed!! \n");}
                }

                if (GPIO_CAMERA_INVALID != CAMERA_CMRST1_PIN) {
	            if(mt_set_gpio_mode(CAMERA_CMRST1_PIN, CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR gc2145] set gpio reset mode failed!! \n");}
	            if(mt_set_gpio_dir(CAMERA_CMRST1_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR gc2145] set gpio reset dir failed!! \n");}
		    if(mt_set_gpio_out(CAMERA_CMRST1_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc2145] set gpio reset failed!! \n");}    
	        }
		
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR gc2145] Fail to enable IO_VDD power\n");		            
			goto _kdCISModulePowerOn_exit_;
		} 
		mdelay(1);

		if(TRUE != hwPowerOn(MT6328_POWER_LDO_VGP1, VOL_1800, mode_name))//DVDD
//		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))//DVDD
		{
			PK_DBG("[CAMERA SENSOR gc2145] Fail to enable DVDD power\n");		      
			goto _kdCISModulePowerOn_exit_;
		}
                mdelay(1);

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR gc2145] Fail to enable AVDD power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}
		mdelay(1);

		if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR gc2145] set gpio pwdn failed!! \n");}
		mdelay(1);
    		if(mt_set_gpio_out(CAMERA_CMRST1_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR gc2145] set gpio reset failed!! \n");}    
		mdelay(5);
			
		PK_DBG("TCL: gc2145 sensor,open down \n\r");
	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV2686_YUV,currSensorName))  && (pinSetIdx == 1))
        {
		PK_DBG("\nTCL: Enter OV2686 sensor,pinSetIdx=%d\n\r",pinSetIdx);

		/*disable inactive sensor*/
		if ((GPIO_CAMERA_INVALID != CAMERA_CMRST_PIN) && (GPIO_CAMERA_INVALID != CAMERA_CMPDN_PIN)) {
	             if(mt_set_gpio_mode(CAMERA_CMRST_PIN, CAMERA_CMRST_PIN_M_GPIO)){PK_DBG("[cam sensor OV2686]set gpio mode failed!! \n");}
	             if(mt_set_gpio_mode(CAMERA_CMPDN_PIN, CAMERA_CMPDN_PIN_M_GPIO)){PK_DBG("[cam sensor OV2686]set gpio mode failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMRST_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR OV2686] set gpio dir failed!! \n");}
	             if(mt_set_gpio_dir(CAMERA_CMPDN_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR OV2686]set gpio dir failed!! \n");}
	             if(mt_set_gpio_out(CAMERA_CMRST_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR OV2686] set gpio failed!! \n");}		         
		     if(mt_set_gpio_out(CAMERA_CMPDN_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR OV2686]set gpio failed!! \n");}
	        }
				
		if (GPIO_CAMERA_INVALID != CAMERA_CMPDN1_PIN) {
		    if(mt_set_gpio_mode(CAMERA_CMPDN1_PIN, CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR OV2686] set gpio pwdn mode failed!! \n");}
	            if(mt_set_gpio_dir(CAMERA_CMPDN1_PIN, GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR OV2686] set gpio pwdn dir failed!! \n");}
	            if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR OV2686] set gpio pwdn failed!! \n");}
                }
		
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR OV2686] Fail to enable AVDD power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}

	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR OV2686] Fail to enable IOVDD power\n");		            
			goto _kdCISModulePowerOn_exit_;
		}

		if(TRUE != hwPowerOn(MT6328_POWER_LDO_VGP1, VOL_1800,mode_name))//DVDD
		{
			PK_DBG("[CAMERA SENSOR OV2686] Fail to enable DVDD power\n");		      
			goto _kdCISModulePowerOn_exit_;
		}
		mdelay(1);

		if(mt_set_gpio_out(CAMERA_CMPDN1_PIN, GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		mdelay(5);

	}
        else
        {
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            //VCAM_IO
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }
            //VCAM_D
            if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K2P8_MIPI_RAW, currSensorName)))
            {
                if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }
            }
            else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName)))
            {
                if(pinSetIdx == 0 && TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }
            }
            else { // Main VCAMD max 1.5V
                if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }

            }


             //AF_VCC
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            mdelay(1);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }
        }

    } else {//power OFF
        PK_DBG("[PowerOFF]pinSetIdx:%d\n", pinSetIdx);
        ISP_MCLK1_EN(0);

	if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670_MIPI_RAW, currSensorName)))
	{
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_OV5670_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

	        if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	           if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	           if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	           if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	           if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		   if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
		}
		mdelay(1);

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
      }
      else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV, currSensorName)))
      {

	   if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	   if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
           if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}


	    if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	       
		
            }

	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    }
	       
	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	    {
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    }
	} 
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2145_YUV,currSensorName)))
        {
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}


		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))   //AVDD
                {
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))  //IOVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		if(TRUE != hwPowerDown(MT6328_POWER_LDO_VGP1,mode_name))  //DVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	   	}

	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV2686_YUV,currSensorName)))
        {
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {  //AVDD
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		if(TRUE != hwPowerDown(MT6328_POWER_LDO_VGP1,mode_name))  //DVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	   	}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))  //IOVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}
	}
        else
        {
            //Set Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName)))
	    {
		if(pinSetIdx == 0 && TRUE != _hwPowerDown(CAMERA_POWER_VCAM_D, mode_name))
		 {
		   PK_DBG("[CAMERA SENSOR] main imx220 Fail to OFF core power (VCAM_D), power id = %d \n",CAMERA_POWER_VCAM_D);
		   goto _kdCISModulePowerOn_exit_;
		 }
	    } else {

	         if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
	         {
	            PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), power id = %d \n",CAMERA_POWER_VCAM_D);
	            goto _kdCISModulePowerOn_exit_;
	          }
	   }

            //VCAM_A
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A), power id= (%d) \n", CAMERA_POWER_VCAM_A);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //AF_VCC
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

        }

    }

    return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;

}

EXPORT_SYMBOL(kdCISModulePowerOn);

//!--
//


