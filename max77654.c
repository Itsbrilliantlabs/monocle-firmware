/******************************************************************************
  Copyright (2022), Brilliant Labs Limited (Hong Kong)
  Licensed under the MIT License
*******************************************************************************/

/** @file max77654.c
 //*	@brief Code for MAX77654 PMIC interface. MCU hardware dependent (I2C & logging).
 *
 *	@author Brilliant Labs Limited (Hong Kong)
 *	
 *	@bug No known bugs.
 */

#include "max77654.h"
#include "nrf_log.h"
#include "i2c.h"
#include "board.h" // for IO control

// ===== hardware dependent data =====
//MAX77654 I2C Address
#define MAX77654_ADDR    (0x48) // 7-bit address

#define MAX77654_CHG_CC_MAX 140  // max. allowable charge current in mA; to protect battery, disallow any higher setting (even if configurable)
#define MAX77654_CHG_CV_MAX 4300 // max. allowable charge voltage in mV; to protect battery, disallow any higher setting (even if configurable)

#ifdef PMIC_MAX77654_DEV_BRD
#define MAX77654_CID_EXPECTED (0xA) // dev board: MAX77654MENV+T (use for development & testing)
#else
#define MAX77654_CID_EXPECTED (0x2) // MK11: MAX77654BENV+T (use for MK11 code)
#endif

// access permissions:
//   RW read write
//   RC read clears all
//   RO read only
//   WO write only (write 1, auto-resets to 0, will be read as 0)

// ===== Register Definitions =====
// Global Registers
#define MAX77654_INT_GLBL0_REG          0x00 // RC
#define MAX77654_INT_GLBL1_REG          0x04 // RC
#define MAX77654_ERCFLAG_REG            0x05 // RC
#define MAX77654_STAT_GLBL_REG          0x06
#define MAX77654_INTM_GLBL0_REG         0x09
#define MAX77654_INTM_GLBL1_REG         0x08
#define MAX77654_CNFG_GLBL_REG          0x10
#define MAX77654_CNFG_GPIO0_REG         0x11 // RW (except bit 1 is RO)
#define MAX77654_CNFG_GPIO1_REG         0x12 // RW (except bit 1 is RO)
#define MAX77654_CNFG_GPIO2_REG         0x13 // RW (except bit 1 is RO)
#define MAX77654_CID_REG                0x14 // RO
#define MAX77654_CNFG_WDT_REG           0x17 // RW (except bit 0 is RO)

// Charger Registers
#define MAX77654_INT_CHG_REG            0x01 // RC
#define MAX77654_STAT_CHG_A_REG         0x02 // RO
#define MAX77654_STAT_CHG_B_REG         0x03 // RO
#define MAX77654_INT_M_CHG_REG          0x07 // RW
#define MAX77654_CNFG_CHG_A_REG         0x20 // RW
#define MAX77654_CNFG_CHG_B_REG         0x21 // RW
#define MAX77654_CNFG_CHG_C_REG         0x22 // RW
#define MAX77654_CNFG_CHG_D_REG         0x23 // RW
#define MAX77654_CNFG_CHG_E_REG         0x24 // RW
#define MAX77654_CNFG_CHG_F_REG         0x25 // RW
#define MAX77654_CNFG_CHG_G_REG         0x26 // RW
#define MAX77654_CNFG_CHG_H_REG         0x27 // RW
#define MAX77654_CNFG_CHG_I_REG         0x28 // RW

// SBB Registers
#define MAX77654_CNFG_SBB0_A_REG        0x29 // RW
#define MAX77654_CNFG_SBB0_B_REG        0x2A // RW
#define MAX77654_CNFG_SBB1_A_REG        0x2B // RW
#define MAX77654_CNFG_SBB1_B_REG        0x2C // RW
#define MAX77654_CNFG_SBB2_A_REG        0x2D // RW
#define MAX77654_CNFG_SBB2_B_REG        0x2E // RW
#define MAX77654_CNFG_SBB_TOP_REG       0x2F // RW

// LDO Registers
#define MAX77654_CNFG_LDO0_A_REG        0x38 // RW
#define MAX77654_CNFG_LDO0_B_REG        0x39 // RW
#define MAX77654_CNFG_LDO1_A_REG        0x3A // RW
#define MAX77654_CNFG_LDO1_B_REG        0x3B // RW


#ifndef BIT
#define BIT(n) (0x01<<n)
#endif

// === Global Registers ===

/* MAX77654_INT_GLBL0_REG          0x0 */
#define MAX77654_DOD0_R            BIT(7) // RC LDO Dropout Detector Rising Interrupt
#define MAX77654_DOD1_R            BIT(6) // RC LDO Dropout Detector Rising Interrupt
#define MAX77654_TJAL2_R           BIT(5) // RC Thermal Alarm 2 Rising Interrupt
#define MAX77654_TJAL1_R           BIT(4) // RC Thermal Alarm 1 Rising Interrupt
#define MAX77654_nEN_R             BIT(3) // RC nEN Rising Interrupt
#define MAX77654_nEN_F             BIT(2) // RC nEN Falling Interrupt
#define MAX77654_GPI_R             BIT(1) // RC GPI Rising Interrupt
#define MAX77654_GPI_F             BIT(0) // RC GPI Falling Interrupt

/* MAX77654_INT_GLBL1_REG          0x04 */
// Reserved                        bit 7
#define MAX77654_LDO1_F            BIT(6) // LDO1 Fault Interrupt
#define MAX77654_LDO0_F            BIT(5) // LDO0 Fault Interrupt
#define MAX77654_SBB_TO            BIT(4) // SBB Timeout
#define MAX77654_GPI2_R            BIT(3) // GPI Rising Interrupt
#define MAX77654_GPI2_F            BIT(2) // GPI Fallint Interrupt
#define MAX77654_GPI1_R            BIT(1) // GPI Rising Interrupt
#define MAX77654_GPI1_F            BIT(0) // GPI Fallint Interrupt

/* MAX77654_ERCFLAG_REG            0x05 */
#define MAX77654_WDT_RST           BIT(7) // Watchdog Timer Reset Flag, watchdog timer expired & caused power-reset
#define MAX77654_WDT_OFF           BIT(6) // Watchdog Timer OFF Flag, watchdog timer expired & caused power-off
#define MAX77654_SFT_CRST_F        BIT(5) // Software Cold Reset Flag
#define MAX77654_SFT_OFF_F         BIT(4) // Software OFF Flag
#define MAX77654_MRST              BIT(3) // Manual Reset Timer, a manual reset has occurred
#define MAX77654_SYSUVLO           BIT(2) // SYS Domain Undervoltage lockout
#define MAX77654_SYSOVLO           BIT(1) // SYS Domain Overvoltage lockout
#define MAX77654_TOVLD             BIT(0) // Thermal overload, temperature > 165C

/* MAX77654_STAT_GLBL_REG          0x06 */
/* MAX77654_INTM_GLBL0_REG         0x09 */
/* MAX77654_INTM_GLBL1_REG         0x08 */
/* MAX77654_CNFG_GLBL_REG          0x10 */
// TODO

/* MAX77654_CNFG_GPIO0_REG         0x11 */
/* MAX77654_CNFG_GPIO1_REG         0x12 */
/* MAX77654_CNFG_GPIO2_REG         0x13 */
// Reserved                        bits 7:6
#define MAX77654_ALT_GPIO          BIT(5) // RW Alternate mode (meaning depends on GPIO #)
#define MAX77654_DBEN_GPIO         BIT(4) // RW GPI Debounce Timer Enable: 0=no debouce, 1=30ms debounce
#define MAX77654_DO                BIT(3) // RW GPO Data Output: 0=logic low, 1=logic high (DRV=1) or hiZ (DRV=0) (if DIR=1, has no effect)
#define MAX77654_DRV               BIT(2) // RW GPO Driver Type: 0=open-drain, 1=push-pull
#define MAX77654_DI                BIT(1) // RO GPIO Digital Input Value: reflects state of the GPIO (irrespective of GPI or GPO)
#define MAX77654_DIR               BIT(0) // RW GPIO Direction: 0=outupt (GPO), 1=input (GPI)

/* MAX77654_CID_REG                0x14 */
#define MAX77654_CID4              BIT(7) // RO Chip Identification Code, bit 4
// Reserved                        bits 6:4
#define MAX77654_CID               (BIT(3) | BIT(2)| BIT(1) | BIT(0))
#define MAX77654_CID_SHIFT         0x00
#define MAX77654_CID_MASK          0x0F

// === Charger Registers ===

/* MAX77654_INT_CHG_REG            0x01 */
/* MAX77654_STAT_CHG_A_REG         0x02 */
//TODO

/* MAX77654_STAT_CHG_B_REG         0x03 */
#define MAX77654_CHG_DTLS          (BIT(7) | BIT(6) | BIT(5) | BIT(4)) // Charger details (charge state)
#define MAX77654_CHG_DTLS_SHIFT    0x04
#define MAX77654_CHG_DTLS_MASK     0x0F
#define MAX77654_CHGIN_DTLS        (BIT(3) | BIT(2)) // CHGIN Status Detail
#define MAX77654_CHGIN_DTLS_SHIFT  0x02
#define MAX77654_CHGIN_DTLS_MASK   0x03
#define MAX77654_CHG               BIT(1) // Quick Charger Status: 0=not charging, 1=charging
#define MAX77654_TIME_SUS          BIT(0) // Time Suspend Indicator: 0=not active or not suspended, 1=suspended (for any of 3 given reasons)
// CHG_DTLS bits
#define MAX77654_CHG_DTLS_OFF         0x00 // Off
#define MAX77654_CHG_DTLS_PRE_Q       0x01 // Prequalification mode
#define MAX77654_CHG_DTLS_FAST_CC     0x02 // Fast-charge constant-current (CC) mode
#define MAX77654_CHG_DTLS_FAST_CC_J   0x03 // JEITA modified fast-charge constant-current (CC) mode
#define MAX77654_CHG_DTLS_FAST_CV     0x04 // Fast-charge constant-voltage (CV) mode
#define MAX77654_CHG_DTLS_FAST_CV_J   0x05 // JEITA modified fast-charge constant-voltage (CV) mode
#define MAX77654_CHG_DTLS_TOP_OFF     0x06 // Top-off mode
#define MAX77654_CHG_DTLS_TOP_OFF_J   0x07 // JEITA modified top-off mode
#define MAX77654_CHG_DTLS_DONE        0x08 // Done
#define MAX77654_CHG_DTLS_DONE_J      0x09 // JEITA modified done
#define MAX77654_CHG_DTLS_FAULT_PRE_Q 0x0A // Prequalification timer fault
#define MAX77654_CHG_DTLS_FAULT_TIME  0x0B // Fast-charge timer fault
#define MAX77654_CHG_DTLS_FAULT_TEMP  0x0B // Battery temperature fault

/* MAX77654_INT_M_CHG_REG          0x07 */
//TODO

/* MAX77654_CNFG_CHG_A_REG         0x20 */
#define MAX77654_THM_HOT           (BIT(7) | BIT(6) // JEITA Temperature Threshold: HOT
#define MAX77654_THM_HOT_SHIFT     0x06
#define MAX77654_THM_WARM          (BIT(5) | BIT(4) // JEITA Temperature Threshold: WARM
#define MAX77654_THM_WARM_SHIFT    0x04
#define MAX77654_THM_COOL          (BIT(3) | BIT(2) // JEITA Temperature Threshold: COOL
#define MAX77654_THM_COOL_SHIFT    0x02
#define MAX77654_THM_COLD          (BIT(1) | BIT(0) // JEITA Temperature Threshold: COLD
#define MAX77654_THM_COLD_SHIFT    0x00
//values to write to temp bits
#define MAX77654_THM_HOT_45C       0x00
#define MAX77654_THM_HOT_50C       0x01
#define MAX77654_THM_HOT_55C       0x02
#define MAX77654_THM_HOT_60C       0x03
#define MAX77654_THM_WARM_35C      0x00
#define MAX77654_THM_WARM_40C      0x01
#define MAX77654_THM_WARM_45C      0x02
#define MAX77654_THM_WARM_50C      0x03
#define MAX77654_THM_COOL_00C      0x00
#define MAX77654_THM_COOL_05C      0x01
#define MAX77654_THM_COOL_10C      0x02
#define MAX77654_THM_COOL_15C      0x03
#define MAX77654_THM_COLD_N10C     0x00
#define MAX77654_THM_COLD_N05C     0x01
#define MAX77654_THM_COLD_00C      0x02
#define MAX77654_THM_COLD_05C      0x03

/* MAX77654_CNFG_CHG_B_REG         0x21 */
#define MAX77654_VCHGIN_MIN        (BIT(7) | BIT(6) | BIT(5)) // Minimum CHGIN Regulation Voltage
#define MAX77654_VCHGIN_MIN_SHIFT  0x05
#define MAX77654_ICHGIN_LIM        (BIT(4) | BIT(3) | BIT(2)) // CHGIN Input Current Limit
#define MAX77654_ICHGIN_LIM_SHIFT  0x02
#define MAX77654_ICHGIN_LIM_MASK   0x07
#define MAX77654_I_PQ              BIT(1) // Prequalification charge current as % of I_FAST-CHG: 0=10%, 1=20%
#define MAX77654_CHG_EN            BIT(0) // Battery Charger Enable: 0=disable, 1=enable
//values to write to VCHGIN_MIN
#define MAX77654_VCHGIN_MIN_4V0    0x00 // 4.0V
#define MAX77654_VCHGIN_MIN_4V1    0x01 // 4.1V
#define MAX77654_VCHGIN_MIN_4V2    0x02 // 4.2V
#define MAX77654_VCHGIN_MIN_4V3    0x03 // 4.3V
#define MAX77654_VCHGIN_MIN_4V4    0x04 // 4.4V
#define MAX77654_VCHGIN_MIN_4V5    0x05 // 4.5V
#define MAX77654_VCHGIN_MIN_4V6    0x06 // 4.6V
#define MAX77654_VCHGIN_MIN_4V7    0x07 // 4.7V
//values to write to ICHGIN_LIM; assumes CNFG_SBB_TOP.ICHGIN_LIM_DEF = 0
#define MAX77654_ICHGIN_LIM_95MA   0x00 // 95mA
#define MAX77654_ICHGIN_LIM_190MA  0x01 // 190mA
#define MAX77654_ICHGIN_LIM_285MA  0x02 // 285mA
#define MAX77654_ICHGIN_LIM_380MA  0x03 // 380mA
#define MAX77654_ICHGIN_LIM_475MA  0x04 // 475mA

/* MAX77654_CNFG_CHG_C_REG         0x22 */
#define MAX77654_CHG_PQ            (BIT(7) | BIT(6) | BIT(5)) // Battery Prequalification Voltage Threshold (VPQ)
#define MAX77654_CHG_PQ_SHIFT      0x05
#define MAX77654_I_TERM            (BIT(4) | BIT(3)) // Termination current
#define MAX77654_I_TERM_SHIFT      0x03
#define MAX77654_T_TOPOFF          (BIT(2) | BIT(1) | BIT(0)) // Top-Off Timer Value (tTO)
#define MAX77654_T_TOPOFF_SHIFT    0x00
// values foro CHG_PQ
#define MAX77654_CHG_PQ_2V3        0x00 // 2.3V
#define MAX77654_CHG_PQ_2V4        0x01 // 2.4V
#define MAX77654_CHG_PQ_2V5        0x02 // 2.5V
#define MAX77654_CHG_PQ_2V6        0x03 // 2.6V
#define MAX77654_CHG_PQ_2V7        0x04 // 2.7V
#define MAX77654_CHG_PQ_2V8        0x05 // 2.8V
#define MAX77654_CHG_PQ_2V9        0x06 // 2.9V
#define MAX77654_CHG_PQ_3V0        0x07 // 3.0V
// values for I_TERM
#define MAX77654_I_TERM_5P         0x00 // 5% of I_FAST-CHG
#define MAX77654_I_TERM_7P5        0x01 // 7.5% of I_FAST-CHG
#define MAX77654_I_TERM_10P        0x02 // 10% of I_FAST-CHG
#define MAX77654_I_TERM_15P        0x03 // 15% of I_FAST-CHG
// values for T_TOPOFF
#define MAX77654_T_TOPOFF_0M       0x00 // 0 minutes
#define MAX77654_T_TOPOFF_5M       0x01 // 5 minutes
#define MAX77654_T_TOPOFF_10M      0x02 // 10 minutes
#define MAX77654_T_TOPOFF_15M      0x03 // 15 minutes
#define MAX77654_T_TOPOFF_20M      0x04 // 20 minutes
#define MAX77654_T_TOPOFF_25M      0x05 // 25 minutes
#define MAX77654_T_TOPOFF_30M      0x06 // 30 minutes
#define MAX77654_T_TOPOFF_35M      0x07 // 35 minutes

/* MAX77654_CNFG_CHG_D_REG         0x23 */
#define MAX77654_TJ_REG            (BIT(7) | BIT(6) | BIT(5)) //  die junction temperature regulation point
#define MAX77654_TJ_REG_SHIFT      0x05
#define MAX77654_VSYS_REG          (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0)) // System Voltage Regulation (VSYS-REG)
#define MAX77654_VSYS_REG_SHIFT    0x00
// default values are fine

/* MAX77654_CNFG_CHG_E_REG         0x24 */
#define MAX77654_CHG_CC            (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)) // fast-charge constant current value, I_FAST-CHG
#define MAX77654_CHG_CC_SHIFT      0x02
#define MAX77654_CHG_CC_MASK       0x3F
#define MAX77654_T_FAST_CHG        (BIT(1) | BIT(0)) // fast-charge safety timer, tFC
#define MAX77654_T_FAST_CHG_SHIFT  0x00
#define MAX77654_T_FAST_CHG_MASK   0x03
// values for T_FAST_CHG
#define MAX77654_T_FAST_CHG_3H     0x01 // 3 hours
#define MAX77654_T_FAST_CHG_5H     0x02 // 5 hours
#define MAX77654_T_FAST_CHG_7H     0x03 // 7 hours

/* MAX77654_CNFG_CHG_F_REG         0x25 */
#define MAX77654_CHG_CC_JEITA      (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)) // fast-charge JEITA constant current value, if COOL or WARM
#define MAX77654_CHG_CC_JEITA_SHIFT 0x02
#define MAX77654_CHG_CC_JEITA_MASK 0x3F
#define MAX77654_THM_EN            BIT(1)
// Reserved                        bit 0

/* MAX77654_CNFG_CHG_G_REG         0x26 */
#define MAX77654_CHG_CV            (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)) // fast-charge battery regulation voltage V_FAST-CHG
#define MAX77654_CHG_CV_SHIFT      0x02
#define MAX77654_CHG_CV_MASK       0x3F
#define MAX77654_USBS              BIT(1) // USB suspend mode: 0=not suspended, draw current from adapter; 1=suspended, draw no current (charge off)
// Reserved                        bit 0

/* MAX77654_CNFG_CHG_H_REG         0x27 */
#define MAX77654_CHG_CV_JEITA      (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)) // fast-charge battery regulation voltage V_FAST-CHG
#define MAX77654_CHG_CV_JEITA_SHIFT 0x02
#define MAX77654_CHG_CV_JEITA_MASK 0x3F
// Reserved                        bits 1, 0

/* MAX77654_CNFG_CHG_I_REG         0x28 */
#define MAX77654_IMON_DISCHG_SCALE (BIT(7) | BIT(6) | BIT(5) | BIT(4)) // battery discharge current full-scale current; default 300mA
#define MAX77654_IMON_DISCHG_SCALE_SHIFT 0x04
#define MAX77654_IMON_DISCHG_SCALE_MASK 0x0F
#define MAX77654_MUX_SEL           (BIT(3) | BIT(2) | BIT(1) | BIT(0)) // AMUX channel select
#define MAX77654_MUX_SEL_SHIFT     0x00
#define MAX77654_MUX_SEL_MASK      0x0F
// values for MUX_SEL
#define MAX77654_MUX_DISABLE       0x0 // disabled, AMUX is hi-Z
#define MAX77654_MUX_CHGIN_V       0x1 // CHGIN voltage monitor
#define MAX77654_MUX_CHGIN_I       0x2 // CHGIN current monitor
#define MAX77654_MUX_BATT_V        0x3 // BATT voltage monitor
#define MAX77654_MUX_BATT_I        0x4 // BATT current monitor, valid only when charging in progress (CHG=1)
#define MAX77654_MUX_BATT_DIS_I    0x5 // BATT discharge current monitor normal measurement
#define MAX77654_MUX_BATT_NUL_I    0x6 // BATT discharge current monitor nulling measurement
#define MAX77654_MUX_THM_V         0x7 // THM voltage monitor
#define MAX77654_MUX_TBIAS_V       0x8 // TBIAS voltage monitor
#define MAX77654_MUX_AGND_V        0x9 // AGND voltage monitor, through 100 ohm pull-down resistor
#define MAX77654_MUX_SYS_V         0xA // SYS voltage monitor

// === SBB Registers ===

/* MAX77654_CNFG_SBB0_A_REG        0x29 */
/* MAX77654_CNFG_SBB1_A_REG        0x2B */
/* MAX77654_CNFG_SBB2_A_REG        0x2D */
// directly write these values to the register to set voltage (other voltages available, see datasheet)
// Reserved                        bit 7
#define MAX77654_CNFG_SBB_A_TV_1V2 0x08   // 0.8V +  8*50mV = 1.2V
#define MAX77654_CNFG_SBB_A_TV_1V8 0x14   // 0.8V + 20*50mV = 1.8V
#define MAX77654_CNFG_SBB_A_TV_2V7 0x26   // 0.8V + 38*50mV = 2.7V
#define MAX77654_CNFG_SBB_A_TV_2V8 0x28   // 0.8V + 40*50mV = 2.8V

/* MAX77654_CNFG_SBB0_B_REG        0x2A */
/* MAX77654_CNFG_SBB1_B_REG        0x2C */
/* MAX77654_CNFG_SBB2_B_REG        0x2E */
// Reserved                        bit 7
#define MAX77654_CNFG_SBB_B_MD     BIT(6) // Operation Mode: 0=buck-boost, 1=buck
#define MAX77654_CNFG_SBB_B_IP     (BIT(5) | BIT(4)) // Peak Current Limit
#define MAX77654_CNFG_SBB_B_IP_SHIFT 0x04
#define MAX77654_CNFG_SBB_B_IP_MASK  0x03
#define MAX77654_CNFG_SBB_B_ADE    BIT(3) // Active Discharge Enable: 0=disabled, 1=enabled
#define MAX77654_CNFG_SBB_B_EN     (BIT(2)| BIT(1) | BIT(0)) // RW Enable
#define MAX77654_CNFG_SBB_B_EN_SHIFT 0x00
#define MAX77654_CNFG_SBB_B_EN_MASK  0x07
// values to write to IP bits
#define MAX77654_CNFG_SBB_B_IP_1000 0x00  // 1000mA output current limit
#define MAX77654_CNFG_SBB_B_IP_750 0x01   // 750mA output current limit
#define MAX77654_CNFG_SBB_B_IP_500 0x02   // 500mA output current limit
#define MAX77654_CNFG_SBB_B_IP_333 0x03   // 333mA output current limit
// values to write to EN bits
#define MAX77654_CNFG_SBB_B_EN_SLOT0 0x0  // FPS slot 0
#define MAX77654_CNFG_SBB_B_EN_SLOT1 0x1  // FPS slot 1
#define MAX77654_CNFG_SBB_B_EN_SLOT2 0x2  // FPS slot 2
#define MAX77654_CNFG_SBB_B_EN_SLOT3 0x3  // FPS slot 3
#define MAX77654_CNFG_SBB_B_EN_OFF   0x4  // Off irrespective of FPS
#define MAX77654_CNFG_SBB_B_EN_ON    0x6  // On irrespective of FPS

/* MAX77654_CNFG_SBB_TOP_REG       0x2F */
// ICHGIN_LIM_DEF                    bit 7 // Don't use, we always set to 0 for standard definitions
#define MAX77654_CNFG_SBB_TOP_DRV  (BIT(1) | BIT(0)) // Drive strength

// === LDO Registers ===

/* MAX77654_CNFG_LDO0_A_REG        0x38 */
/* MAX77654_CNFG_LDO1_A_REG        0x3A */
// directly write these values to the register to set voltage (other voltages available, see datasheet)
// Reserved                        bit 7
#define MAX77654_CNFG_LDO_A_TV_1V2 0x10   // 0.8V + 16*25mV = 1.2V
#define MAX77654_CNFG_LDO_A_TV_1V8 0x28   // 0.8V + 40*25mV = 1.8V
#define MAX77654_CNFG_LDO_A_TV_2V7 0x4C   // 0.8V + 76*25mV = 2.7V
#define MAX77654_CNFG_LDO_A_TV_2V8 0x50   // 0.8V + 80*25mV = 2.8V

/* MAX77654_CNFG_LDO0_B_REG        0x39 */
/* MAX77654_CNFG_LDO1_B_REG        0x3B */
#define MAX77654_CNFG_LDO_B_MD     BIT(4) // RW Mode: 0=LDO, 1=Load Switch
#define MAX77654_CNFG_LDO_B_ADE    BIT(3) // RW Active Discharge Enable: 0=disabled, 1=enabled
#define MAX77654_CNFG_LDO_B_EN     (BIT(2)| BIT(1) | BIT(0)) // RW Enable
#define MAX77654_CNFG_LDO_B_EN_SHIFT 0x00
#define MAX77654_CNFG_LDO_B_EN_MASK  0x07
// values to write to EN bits
#define MAX77654_CNFG_LDO_B_EN_SLOT0 0x0
#define MAX77654_CNFG_LDO_B_EN_SLOT1 0x1
#define MAX77654_CNFG_LDO_B_EN_SLOT2 0x2
#define MAX77654_CNFG_LDO_B_EN_SLOT3 0x3
#define MAX77654_CNFG_LDO_B_EN_OFF   0x4
#define MAX77654_CNFG_LDO_B_EN_ON    0x6

//Decode the MAX77654_STAT_CHG_B_REG register, MAX77654_CHG_DTLS bits
static const char * status_desc[] = {
	[MAX77654_CHG_DTLS_OFF] = "Off",
	[MAX77654_CHG_DTLS_PRE_Q] = "Prequalification mode",
	[MAX77654_CHG_DTLS_FAST_CC] = "Fast-charge constant-current (CC) mode",
	[MAX77654_CHG_DTLS_FAST_CC_J] = "JEITA modified fast-charge constant-current (CC) mode",
        [MAX77654_CHG_DTLS_FAST_CV] = "Fast-charge constant-voltage (CV) mode",
        [MAX77654_CHG_DTLS_FAST_CV_J] = "JEITA modified fast-charge constant-voltage (CV) mode",
        [MAX77654_CHG_DTLS_TOP_OFF] = "Top-off mode",
        [MAX77654_CHG_DTLS_TOP_OFF_J] = "JEITA modified top-off mode",
        [MAX77654_CHG_DTLS_DONE] = "Done",
        [MAX77654_CHG_DTLS_DONE_J] = "JEITA modified done",
        [MAX77654_CHG_DTLS_FAULT_PRE_Q] = "Prequalification timer fault",
        [MAX77654_CHG_DTLS_FAULT_TIME] = "Fast-charge timer fault",
        [MAX77654_CHG_DTLS_FAULT_TEMP] = "Battery temperature fault",
};

/* Charge current in mA*10 */
static const unsigned int cc_tbl[] = {
	75,  //[0] = 7.5mA
	150, //[1] = 15mA
	225,
	300,
	375,
	450,
	525,
	600,
	675,
	750,
	825, //[10]
	900,
	975,
	1050,
	1125,
	1200,
	1275,
	1350,
	1425,
	1500,
	1575, //[20] = 157.5mA
	1650,
	1725,
	1800,
	1875,
	1950,
	2025,
	2100,
	2175,
	2250,
	2325, //[30 = 011110] = 232.5mA
        2400, //[31 = 011111]
        2475, //[32 = 100000]
	2550,
	2625,
	2700,
	2775,
	2850,
	2925,
	3000  //[39 = 0x27] 300mA
	// [0x28 to 0x3F] also 300mA
};

/* Charge voltage in mV */
static const unsigned int cv_tbl[] = {
	3600, //[0] = 3600mV = 3.6V
	3625, //[1] = 3.625V
	3650,
	3675,
	3700,
	3725,
	3750,
	3775,
	3800,
	3825,
	3850, //[10]
	3875,
	3900,
	3925,
	3950,
	3975,
	4000,
	4025,
	4050,
	4075,
	4100, //[20] = 4.1V
	4125,
	4150,
        4175,
        4200,
	4225,
	4250,
	4275,
	4300,
	4325,
	4350, //[30 = 011110] = 4.35V
	4375,
	4400, //[32 = 100000] = 4.4V
        4425,
        4450,
        4475,
        4500,
        4525,
        4550,
        4575, //[39 = 0x27]
        4600  //[40 = 0x28] = 4.6V
};

static bool max77654_initialized = false; // flag that PMIC has been successfully initialized

// ===== internal functions =====

// for logging
#ifdef MAX77654_LOG_INFO_ON
#define MAX77654_LOG_INFO(...) NRF_LOG_INFO(__VA_ARGS__)
#else
#define MAX77654_LOG_INFO(...) 
#endif

#ifdef MAX77654_LOG_DEBUG_ON
#define MAX77654_LOG_DEBUG(...) NRF_LOG_DEBUG(__VA_ARGS__)
#else
#define MAX77654_LOG_DEBUG(...) 
#endif

bool max77654_write(uint8_t reg, uint8_t data)
{
    uint8_t write_buffer[(2u)];
    bool success = false;

    // Initialize buffer with packet
    write_buffer[0] = reg;
    write_buffer[1] = data;

    if (i2c_write(MAX77654_ADDR, write_buffer, 2))
    {
        success = true;
        MAX77654_LOG_DEBUG("MAX77654 Write 0x%x to register 0x%x", data, reg);
    }
    return success;
}

bool max77654_read(uint8_t reg, uint8_t *value)
{
    uint8_t write_buffer[(1u)];
    uint8_t read_buffer[(1u)];
    uint8_t ret_val = 0;
    bool success = false;

    // Initialize buffer with register address
    write_buffer[0] = reg;

    if (i2c_write(MAX77654_ADDR, write_buffer, 1))
    {
        if (i2c_read(MAX77654_ADDR, read_buffer, 1))
        {
            ret_val = read_buffer[0];
            *value = ret_val;
            success = true;
            MAX77654_LOG_DEBUG("MAX77654 Read register 0x%x = 0x%x", reg, *value);
        }
    }
    return success;
}

// return 0 on read failure or CID value (6-bits) on read success
uint8_t max77654_read_cid(void)
{
    bool success = false;
    uint8_t reg = 0;
    uint8_t bit4 = 0;
    uint8_t cid = 0;

    success = max77654_read(MAX77654_CID_REG, &reg);
    if(!success) {
        MAX77654_LOG_DEBUG("MAX77654 CID read failed");
        return 0;
    }
    bit4 = (reg & MAX77654_CID4) >> 3;
    cid = bit4 | (reg & MAX77654_CID_MASK);
    MAX77654_LOG_DEBUG("MAX77654 CID = 0x%x.", cid);
    return cid;
}

/* Convert charge current to register value using lookup table */
static int cc_to_hw(unsigned int val)
{
	int i;
        int size = sizeof(cc_tbl) / sizeof(cc_tbl[0]);

	for (i = 0; i < size; i++)
		if (val < cc_tbl[i])
			break;
	return i > 0 ? i-1 : 0;
}

/* Convert charge voltage to register value using lookup table */
static int cv_to_hw(unsigned int val)
{
	int i;
        int size = sizeof(cv_tbl) / sizeof(cv_tbl[0]);

	for (i = 0; i < size; i++)
		if (val < cv_tbl[i])
			break;
	return i > 0 ? i-1 : 0;
}

static bool update_register_bits(uint8_t reg, uint8_t newbits, uint8_t bits_to_update, uint8_t shift)
{
    bool success = false;
    uint8_t reg_val = 0;

    success = max77654_read(reg, &reg_val);
    if(success) {
        MAX77654_LOG_DEBUG("MAX77654 update_register_bits() original register value read: 0x%x.", reg_val);
        MAX77654_LOG_DEBUG("MAX77654 update_register_bits() bits to write: 0x%x, shifted <<%d.", newbits, shift);
        reg_val = (reg_val & ~bits_to_update) | (newbits << shift);
        success = max77654_write(reg, reg_val);
        if(success) {
            MAX77654_LOG_DEBUG("MAX77654 update_register_bits() wrote: 0x%x to register 0x%x.", reg_val, reg);
        } else {
            NRF_LOG_ERROR("MAX77654 update_register_bits() failed to write register 0x%x.", reg);
        }
    } else {
        NRF_LOG_ERROR("MAX77654 update_register_bits() failed to read register 0x%x.", reg);
    }
    return success;
}

static bool get_register_bits(uint8_t reg, uint8_t *gotbits, uint8_t bits_to_get, uint8_t shift)
{
    bool success = false;
    uint8_t reg_val = 0;

    success = max77654_read(reg, &reg_val);
    if(success) {
        *gotbits = (reg_val & bits_to_get) >> shift;
        MAX77654_LOG_DEBUG("MAX77654 get_register_bits() got 0x%x.", *gotbits);
    } else {
        NRF_LOG_ERROR("MAX77654 get_register_bits() failed to read register 0x%x.", reg);
    }
    return success;
}


// ===== public functions =====

/**@brief Function for initializing MAX77654
 *
 * Test results: (using resistors to simulate bulk & constant voltage charging)
 *   bulk charging current: 67.4mA, constant voltage: 4.28V
 */
bool max77654_init(void)
{
    uint8_t reg = 0;
    uint8_t cid = 0;

    // === verify MAX77654 on I2C bus by attempting to read Chip ID register ===
    cid = max77654_read_cid();
    if(cid == 0) { // read failure
        NRF_LOG_ERROR("MAX77654 not detected on I2C bus.");
        return false;
    } else if (cid != MAX77654_CID_EXPECTED) {
        NRF_LOG_ERROR("MAX77654 incorrect CID detected: expected 0x%x, read 0x%x.", MAX77654_CID_EXPECTED, cid);
        return false;
    }

    // ===== Power Rail Configuration =====
    // Power Rail: 2.7V
    // set SBB0 to 2.7V, buck, 333mA, active discharge, OFF
    max77654_write(MAX77654_CNFG_SBB0_A_REG, MAX77654_CNFG_SBB_A_TV_2V7);
    max77654_write(MAX77654_CNFG_SBB0_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_OFF));

    // Power Rail: 1.8V always on
    // set SBB1 to 1.8V, buck, 333mA, active discharge, ON
#ifdef PMIC_MAX77654_DEV_BRD  // on MK11, OTP setting is already 1.8V; on dev board, change setting
    max77654_write(MAX77654_CNFG_SBB1_A_REG, MAX77654_CNFG_SBB_A_TV_1V8);
#endif
    max77654_write(MAX77654_CNFG_SBB1_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_ON));

    // Power Rail: 1.2V
    // set SBB2 to 1.2V, buck, 333mA, active discharge, OFF
    max77654_write(MAX77654_CNFG_SBB2_A_REG, MAX77654_CNFG_SBB_A_TV_1V2);
    max77654_write(MAX77654_CNFG_SBB2_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_OFF));

    // Power Rail: 1.8VDC_SW, 1.8V switched
    // set LDO0 as load switch, Active Discharge, OFF
    max77654_write(MAX77654_CNFG_LDO0_A_REG, MAX77654_CNFG_LDO_A_TV_1V8); // not needed; just in case mode set to LDO by mistake
    max77654_write(MAX77654_CNFG_LDO0_B_REG, (MAX77654_CNFG_LDO_B_MD | MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_OFF));

    // Power Rail: VLED, 2.7V
    // set LDO1 to LDO at 2.7V, Active Discharge, OFF
    max77654_write(MAX77654_CNFG_LDO1_A_REG, MAX77654_CNFG_LDO_A_TV_2V7);
    max77654_write(MAX77654_CNFG_LDO1_B_REG, (MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_OFF));

    // ICHGIN_LIM_DEF=0: clear this bit so dev brd "M" OTP matches "B" OTP of 0, that is ICHGIN_LIM scale starts at 95mA
    // Drive strength, slow down to reduce EMI (but reduces efficiency)
    max77654_write(MAX77654_CNFG_SBB_TOP_REG, 0x02); // second slowest

    // ===== GPIO configuration =====
    // Note: OTP "B" version defaults to Alternate functions, not GPO, so must be reconfigured
#ifdef PMIC_MAX77654_DEV_BRD // LEDs connected to transistors: push-pull, logic low
    max77654_write(MAX77654_CNFG_GPIO0_REG, (MAX77654_DRV));
    max77654_write(MAX77654_CNFG_GPIO1_REG, (MAX77654_DRV));
#else // on MK11 hardware
    // GPIO0 (Red LED) : GPO, open-drain, logic low
    max77654_write(MAX77654_CNFG_GPIO0_REG, MAX77654_DO); // set to hi-Z, LED off
    // GPIO1 (Green LED) : GPO, open-drain, logic low
    max77654_write(MAX77654_CNFG_GPIO1_REG, MAX77654_DO); // set to hi-Z, LED off
#endif
    // GPIO2 (DISP_PWR_EN): GPO, push-pull, logic low -> 10V off
    max77654_write(MAX77654_CNFG_GPIO2_REG, MAX77654_DRV);
    //NOTE GPIO2 is broken on our Dev Board: read of bit 1 always returns 0 (even if bit 3 set high)
    //max77654_read(MAX77654_CNFG_GPIO2_REG, &reg); 

    // ===== Charging configuration =====
    // Battery charging parameters from datasheets
    // for Varta CP1254 A4 CoinPower battery * 1
    // Charging:
    //   Voltage: 4.3V (4.0V for rapid)
    //   Current: 35mA (standard), 70mA (fast), 140mA (rapid)
    //   Operating temperature (standard & fast) 0 to 45C
    //   Operating temperature (rapid) 20 to 45C
    //   Cut-off time: 5hr (standard), 3h (fast & rapid)
    //   Cut-off current: 1.4mA
    //   Pre-qualification charging not specified; assume 14mA to 2.5V
    // Discharge
    //   Cut-off voltage: 3.0V
    //   Max current (continuous): 140mA
    //   Operating temperature: -20 to 60C

    // Rapid charging is not currently implemented, because:
    //   It is incompatible with MAX design, which assumes non-JEITA charging to a _higher_ voltage, whereas Varta is to _lower_ voltage.
    //   If implemented rapid charging to 4.0V at temperatures 20-45C, battery would never fully charge (except when temp in range 0-20C).
    //   Could implement rapid charging if willing to accept less-than-full battery, or if
    //     MCU monitors voltage, increasing charge voltage & decreasing charge current when battery reaches 3.95V.
    
    // JEITA Temperatures: COLD=0C, HOT=45C, COOL=20C (not avail, use 15C), WARM=45C (MK11 NTC B is 3380K)
    max77654_write(MAX77654_CNFG_CHG_A_REG, ( (MAX77654_THM_HOT_45C << MAX77654_THM_HOT_SHIFT) | (MAX77654_THM_WARM_45C << MAX77654_THM_WARM_SHIFT) \
                                            | (MAX77654_THM_COOL_15C << MAX77654_THM_COOL_SHIFT) | (MAX77654_THM_COLD_00C << MAX77654_THM_COLD_SHIFT) ));

    // "B" version input current limit is 95mA; increase to allow charging + operation (at least during testing)
    // V_CHGIN_MIN=4.3V, I_CHGIN-LIM=190mA, I_pre-charge = 20% of I_fast, charge enable
    max77654_write(MAX77654_CNFG_CHG_B_REG, ( (MAX77654_VCHGIN_MIN_4V3 << MAX77654_VCHGIN_MIN_SHIFT) \
                                            | (MAX77654_ICHGIN_LIM_190MA << MAX77654_ICHGIN_LIM_SHIFT) \
                                            | (MAX77654_I_PQ) | (MAX77654_CHG_EN) ));

    // pre-charge to 2.5V, termination current = 10% = 6.8mA (so CC6 green LED will turn off), top-off 5 mins (unknown how long before reach 1.4mA, should be safe)
    max77654_write(MAX77654_CNFG_CHG_C_REG, ( (MAX77654_CHG_PQ_2V5 << MAX77654_CHG_PQ_SHIFT) | (MAX77654_I_TERM_10P << MAX77654_I_TERM_SHIFT) \
                                            | (MAX77654_T_TOPOFF_5M << MAX77654_T_TOPOFF_SHIFT)));

    // MAX77654_CNFG_CHG_D_REG: use defaults: die temperature regulation = 60C, V_SYS-REG = 4.5V

    max77654_initialized = true; // place here to allow successful calls to _set_charge_current() and _set_charge_voltage()

    // MAX77654_CNFG_CHG_E_REG: fast/rapid charge current = 67.5mA, safety timer = 3 hours (default)
    max77654_set_charge_current(70); // will result in 67.5mA, since next highest value is 75mA
    //max77654_read(MAX77654_CNFG_CHG_E_REG, &reg); //debug

    // MAX77654_CNFG_CHG_F_REG: JEITA charge current = 70mA (=67.5mA), Thermistor enabled
    max77654_write(MAX77654_CNFG_CHG_F_REG, MAX77654_THM_EN);
    update_register_bits(MAX77654_CNFG_CHG_F_REG, cc_to_hw(70*10), MAX77654_CHG_CC_JEITA, MAX77654_CHG_CC_JEITA_SHIFT);

    // MAX77654_CNFG_CHG_G_REG: charge voltage 4.3V, not in USB suspend mode
    max77654_set_charge_voltage(4300);
    //max77654_read(MAX77654_CNFG_CHG_G_REG, &reg); //debug

    // MAX77654_CNFG_CHG_H_REG: JEITA charge voltage = 4.3V, Thermistor enabled
    update_register_bits(MAX77654_CNFG_CHG_H_REG, cv_to_hw(4300), MAX77654_CHG_CV_JEITA, MAX77654_CHG_CV_JEITA_SHIFT);

    // MAX77654_CNFG_CHG_I_REG: use defaults: AMUX BATT discharge 300mA scale, AMUX disabled (hi-Z)

    // ===== AMUX configuration =====
    // by default AMUX is off (high-impedance), for now just turn on, but this is not most power-efficient
    // TODO implement turning on only when measurements are needed
    // monitor VSYS voltage (=input from CC when plugged in, or from battery when stand-alone)
    update_register_bits(MAX77654_CNFG_CHG_I_REG, MAX77654_MUX_SYS_V, MAX77654_MUX_SEL, MAX77654_MUX_SEL_SHIFT);
    
    return true;
}

bool max77654_rail_1v8sw_on(bool on)
{
    bool success = false;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_LDO0_B_REG, (MAX77654_CNFG_LDO_B_MD | MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_ON));
        } else {
            success = max77654_write(MAX77654_CNFG_LDO0_B_REG, (MAX77654_CNFG_LDO_B_MD | MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_OFF));
        }
    }
    return success;
}

bool max77654_rail_2v7_on(bool on)
{
    bool success = false;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_SBB0_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_ON));
        } else {
            success = max77654_write(MAX77654_CNFG_SBB0_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_OFF));
        }
    }
    return success;
}

bool max77654_rail_1v2_on(bool on)
{
    bool success = false;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_SBB2_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_ON));
        } else {
            success = max77654_write(MAX77654_CNFG_SBB2_B_REG, (MAX77654_CNFG_SBB_B_MD | (MAX77654_CNFG_SBB_B_IP_333 << MAX77654_CNFG_SBB_B_IP_SHIFT) \
                                             | MAX77654_CNFG_SBB_B_ADE | MAX77654_CNFG_SBB_B_EN_OFF));
        }
    }
    return success;
}

bool max77654_rail_10v_on(bool on)
{
    bool success = false;
    uint8_t reg = 0;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_GPIO2_REG, (MAX77654_DRV | MAX77654_DO)); // push-pull, high
        } else {
            success = max77654_write(MAX77654_CNFG_GPIO2_REG, MAX77654_DRV); // push-pull, low
        }
        //max77654_read(MAX77654_CNFG_GPIO2_REG, &reg); //debug
    }
}

bool max77654_rail_vled_on(bool on)
{
    bool success = false;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_LDO1_B_REG, (MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_ON));
        } else {
            success = max77654_write(MAX77654_CNFG_LDO1_B_REG, (MAX77654_CNFG_LDO_B_ADE | MAX77654_CNFG_LDO_B_EN_OFF));
        }
    }
    return success;
}

#ifdef PMIC_MAX77654_DEV_BRD
const uint8_t LED_ON = (MAX77654_DRV | MAX77654_DO); // push-pull, high
const uint8_t LED_OFF = MAX77654_DRV; // push-pull, low
#else
const uint8_t LED_ON = 0x00; // open-drain, set low, LED on
const uint8_t LED_OFF = MAX77654_DO; // open-drain, set to hi-Z, LED off
#endif

bool max77654_led_red_on(bool on)
{
    bool success = false;
    uint8_t reg = 0;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_GPIO0_REG, LED_ON);
        } else {
            success = max77654_write(MAX77654_CNFG_GPIO0_REG, LED_OFF);
        }
        //max77654_read(MAX77654_CNFG_GPIO0_REG, &reg); // debug
    }
}

bool max77654_led_green_on(bool on)
{
    bool success = false;
    uint8_t reg = 0;
    if(max77654_initialized) {
        if(on) {
            success = max77654_write(MAX77654_CNFG_GPIO1_REG, LED_ON);
        } else {
            success = max77654_write(MAX77654_CNFG_GPIO1_REG, LED_OFF);
        }
        //max77654_read(MAX77654_CNFG_GPIO1_REG, &reg); // debug
    }
}

/**
 * max77654_charging_status - returns status of charging
 *
 */
max77654_status max77654_charging_status(void)
{
    uint8_t val;
    bool success = false;

    success = get_register_bits(MAX77654_STAT_CHG_B_REG, &val, MAX77654_CHG_DTLS, MAX77654_CHG_DTLS_SHIFT);
    if (success) {
        MAX77654_LOG_INFO("MAX77654 Status = %s.", status_desc[val]);
        if( (val == MAX77654_CHG_DTLS_OFF) ) return MAX77654_READY;
        if( (val == MAX77654_CHG_DTLS_DONE) || (val == MAX77654_CHG_DTLS_DONE_J) ) return MAX77654_CHARGE_DONE;
        if( (val == MAX77654_CHG_DTLS_FAULT_PRE_Q) || (val == MAX77654_CHG_DTLS_FAULT_TIME) || (val == MAX77654_CHG_DTLS_FAULT_TEMP) ) return MAX77654_FAULT;
        return MAX77654_CHARGING;
    } else {
        NRF_LOG_ERROR("MAX77654 error reading charge status register B.");
        return MAX77654_READ_ERROR;
    }
}

max77654_fault max77654_faults_status(void)
{
    uint8_t val;
    int ret = MAX77654_NORMAL;
    bool success = false;

    success = get_register_bits(MAX77654_STAT_CHG_B_REG, &val, MAX77654_CHG_DTLS, MAX77654_CHG_DTLS_SHIFT);
    if (success && ((val == MAX77654_CHG_DTLS_FAULT_PRE_Q) || (val == MAX77654_CHG_DTLS_FAULT_TIME) || (val == MAX77654_CHG_DTLS_FAULT_TEMP)) )
    {
        MAX77654_LOG_INFO("MAX77654 Fault = %s.", status_desc[val]);
        if( (val == MAX77654_CHG_DTLS_FAULT_PRE_Q) ) ret = MAX77654_FAULT_PRE_Q;
        if( (val == MAX77654_CHG_DTLS_FAULT_TIME) ) ret = MAX77654_FAULT_TIME;
        if( (val == MAX77654_CHG_DTLS_FAULT_TEMP) ) ret = MAX77654_FAULT_TEMP;
    } else {
        NRF_LOG_ERROR("MAX77654 error reading charge status register B.");
    }
    return ret;
}


//WARNING setting this too high could damage the battery
// Range is 7 - 300mA. Input values <7 will be rounded up to 7.5, >300 down to 300, and intermediate values rounded down
//                     to the nearest supported config value (increments of 7.5mA).
bool max77654_set_charge_current(uint16_t current)
{
    bool success = false;
    uint8_t charge_bits = 0;

    if(max77654_initialized) {
        if(current > MAX77654_CHG_CC_MAX) {
            NRF_LOG_ERROR("MAX77654 Charge Current cannot be set above %d mA to protect the battery.", MAX77654_CHG_CC_MAX);
            return false;
        }
        charge_bits = cc_to_hw(current*10);
        success = update_register_bits(MAX77654_CNFG_CHG_E_REG, charge_bits, MAX77654_CHG_CC, MAX77654_CHG_CC_SHIFT);
        if(success) {
            MAX77654_LOG_INFO("MAX77654 Charge Current set to %d mA.", cc_tbl[charge_bits]/10);
        } else {
            NRF_LOG_ERROR("MAX77654 Charge Current FAILED to set to %d mA.", cc_tbl[charge_bits]/10);
        }
    }
    return success;
}

// Set charge voltage (in mV)
// Range is 3600 to 4600 (3.6V to 4.6V)
bool max77654_set_charge_voltage(uint16_t millivolts)
{
    bool success = false;
    uint8_t charge_bits = 0;

    if(max77654_initialized) {
        if(millivolts > MAX77654_CHG_CV_MAX) {
            NRF_LOG_ERROR("MAX77654 Charge Voltage cannot be set above %d mV to protect the battery.", MAX77654_CHG_CV_MAX);
            return false;
        }
        if(millivolts < 3600) millivolts = 3600;
        if(millivolts > 4600) millivolts = 4600;
        charge_bits = ((millivolts-3600)/25) & MAX77654_CHG_CV_MASK;
        success = update_register_bits(MAX77654_CNFG_CHG_G_REG, charge_bits, MAX77654_CHG_CV, MAX77654_CHG_CV_SHIFT);
        if(success) {
            MAX77654_LOG_INFO("MAX77654 Charge Voltage set to %d mV.", millivolts);
        } else {
            NRF_LOG_ERROR("MAX77654 Charge Voltage FAILED to set to %d mV.", millivolts);
        }
    }
    return success;
}

// Set input current limit (in mA)
// Range is 95 to 475 in increments of 95mA, see MAX77654_CNFG_CHG_B_REG definitions
bool max77654_set_current_limit(uint16_t current)
{
    bool success = false;
    uint8_t charge_bits = 0;

    if(max77654_initialized) {
        if(current <= 95) {
            charge_bits = MAX77654_ICHGIN_LIM_95MA;
        } else if(current <= 190) {
            charge_bits = MAX77654_ICHGIN_LIM_190MA;
        } else if(current <= 285) {
            charge_bits = MAX77654_ICHGIN_LIM_285MA;
        } else if(current <= 380) {
            charge_bits = MAX77654_ICHGIN_LIM_380MA;
        } else {
            charge_bits = MAX77654_ICHGIN_LIM_475MA;
        }
        charge_bits = charge_bits & MAX77654_ICHGIN_LIM_MASK;

        success = update_register_bits(MAX77654_CNFG_CHG_B_REG, charge_bits, MAX77654_ICHGIN_LIM, MAX77654_ICHGIN_LIM_SHIFT);
        if(success) {
            MAX77654_LOG_INFO("MAX77654 Input Current Limit set to %d mA.", (charge_bits*95)+95);
        } else {
            NRF_LOG_ERROR("MAX77654 Input Current Limit FAILED to set to %d mA.", (charge_bits*95)+95);
        }
    }
    return success;
}

bool max77564_factory_ship_mode(void) 
{
    bool success = false;
    success = max77654_write(MAX77654_CNFG_GLBL_REG, 0xA3); // Factory Ship mode is very low power mode
    return success;
}


