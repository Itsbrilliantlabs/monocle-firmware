/******************************************************************************
  Copyright (2022), Brilliant Labs Limited (Hong Kong)
  Licensed under the MIT License
*******************************************************************************/

//TODO Rename tables from RGB to YUV; perhaps with a complier switch between them
//TODO consolidate ov5640_uxga_init_reg_tbl & ov5640_rgb565_reg_tbl & ov5640_rgb565_reg_tbl_1x into one for faster bootup

#ifndef _OV5640CFG_H
#define _OV5640CFG_H
#include "ov5640.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32������
//ov5640 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/12/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//JPEG����.7.5֡
//���֧��2592*1944��JPEGͼ�����
const uint16_t OV5640_jpeg_reg_tbl[][2]=
{
	0x4300, 0x30, // YUV 422, YUYV
	0x501f, 0x00, // YUV 422
	// Input clock = 24Mhz
	0x3035, 0x21, // PLL  
	0x3036, 0x69, // PLL 
	0x3c07, 0x07, // lightmeter 1 threshold[7:0] 
	0x3820, 0x46, // flip
	0x3821, 0x20, // mirror									 
	0x3814, 0x11, // timing X inc: no skipping
	0x3815, 0x11, // timing Y inc: no skipping
	0x3800, 0x00, // HS 
	0x3801, 0x00, // HS = 0
	0x3802, 0x00, // VS 
	0x3803, 0x00, // VS = 0
	0x3804, 0x0a, // HW (HE)		 
	0x3805, 0x3f, // HW (HE) = 2623
	0x3806, 0x07, // VH (VE)		 
	0x3807, 0x9f, // VH (VE) = 1951
	
	0x3808, 0x02, // DVPHO 	   
	0x3809, 0x80, // DVPHO = 640
	0x380a, 0x01, // DVPVO 		
	0x380b, 0xe0, // DVPVO = 480

	0x380c, 0x0b, // HTS
	0x380d, 0x1c, // HTS = 2844
	0x380e, 0x07, // VTS
	0x380f, 0xb0, // VTS = 1968
	0x3813, 0x04, // timing V offset   04
// these appear to be JPEG-specific, not set in other configs
	0x3618, 0x04,  // undocumented? // 0x00 in RGB/YUV
	0x3612, 0x2b,  // undocumented? // 0x29 in RGB/YUV
	0x3709, 0x12,  // undocumented? // 0x52 in RGB/YUV
	0x370c, 0x00,  // undocumented? // 0x03 in RGB/YUV
	
	0x4004, 0x06, // BLC line number // 0x02 in RGB/YUV
	0x3002, 0x00, // enable everytihng, including JFIFO, SFIFO, JPG // 0x1c in RGB/YUV = reset/disable JFIFO, SFIFO, JPG
	0x3006, 0xff, // enable clock of JPEG2x, JPEG // 0xc3 in RGB/YUV = disable clock of JPEG2x, JPEG
	0x4713, 0x03, // JPEG mode 3 // same in RGB/YUV
	0x4407, 0x01, // Quantization scale // 0x04 in RGB/YUV
	0x460b, 0x35, // VFIFO debug mode, not documented // 0x37 in RGB/YUV
// end JPEG-specific
	0x460c, 0x22, // bit[1]: manual DVP_PCLK // 0x20 in RGB/YUV
	0x4837, 0x16, // MIPI global timing 
	0x3824, 0x02, // PCLK manual divider 
	0x5001, 0xA3, // SDE on, scale on, UV average off, color matrix on, AWB on
	0x3503, 0x00, // AEC/AGC on (auto)
};

const uint16_t ov5640_jpeg_on_tbl[][2]=
{
	0x3618, 0x04,  // undocumented? // 0x00 in RGB/YUV
	0x3612, 0x2b,  // undocumented? // 0x29 in RGB/YUV
	0x3709, 0x12,  // undocumented? // 0x52 in RGB/YUV
	0x370c, 0x00,  // undocumented? // 0x03 in RGB/YUV
	
	0x4004, 0x06, // BLC line number // 0x02 in RGB/YUV
	0x3002, 0x00, // enable everytihng, including JFIFO, SFIFO, JPG // 0x1c in RGB/YUV = reset/disable JFIFO, SFIFO, JPG
	0x3006, 0xff, // enable all clocks, incl. JPEG2x, JPEG // 0xc3 in RGB/YUV = disable clock of JPEG2x, JPEG
	0x4713, 0x03, // JPEG mode 3 // same in RGB/YUV
	0x4407, 0x01, // JPEG Quantization scale // 0x04 in RGB/YUV
	0x460b, 0x35, // VFIFO debug mode, not documented // 0x37 in RGB/YUV
};

//RGB565����.15֡
//���֧��1280*800��RGB565ͼ�����
//NWA: 4.4fps, skipping off, max 2560x1600 ISP Input Size
const uint16_t ov5640_rgb565_reg_tbl[][2]=
{
        
#if (MK_HW_DEVICE == MK_HW_DEV_BRD) || (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0)
	// RGB mode
	0x4300, 0X6F, // FORMAT CONTROL 0x6 = RGB565, 
			// 0xF = output sequence {g[2:0],b[4:0]},{r[4:0],g[5:3]}
	0X501F, 0x01, // ISP RGB
#elif  (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	// YUV mode
	0x4300, 0x30, // YUV 422, YUYV
	0x501f, 0x00, // YUV 422
#else
        #error "Unspecified hardware target."
#endif

	// original code was: 1280x800, 15fps
	// input clock 24Mhz, PCLK 42Mhz
	0x3035, 0x41, // PLL DEFAULT PCLK = 41MHz
	//0x3035, 0x42,		// speed up 2x -- NATHAN -- FAIL: static on lower half of screen at max zoom (others OK)
        //0x3035, 0x81,		// slow down 2x -- NATHAN -- PCLK = 21MHz
	0x3036, 0x69, // PLL
	0x3c07, 0x07, // lightmeter 1 threshold[7:0]
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x3820, 0x46, // flip
	//TODO: put in mirror here...
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2)  || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x3820, 0x40, // [2-1] flip off
#else
        #error "Unspecified hardware target."
#endif
	0x3821, 0x00, // [2-1] mirror off | [0] binning off
	//0x3814, 0x31, // timing X inc: odd=3, even=1 ?
	//0x3815, 0x31, // timing Y inc: odd=3, even=1 ?
	0x3814, 0x11, // no skipping -- NATHAN TRY 1
	0x3815, 0x11, // no skipping -- NATHAN TRY 1
	// ISP input size (0,0) to (2623,1705)
	0x3800, 0x00, // HS
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x3801, 0x00, // HS --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x3801, 0x10, // HS -- NATHAN TRY 1
#else
        #error "Unspecified hardware target."
#endif
	0x3802, 0x00, // VS
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x3803, 0x00, // VS --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x3803, 0x0E, // VS -- NATHAN TRY 1
#else
        #error "Unspecified hardware target."
#endif
	0x3804, 0x0a, // HW (HE)
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x3805, 0x3f, // HW (HE) --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x3805, 0x2f, // HW (HE) -- NATHAN TRY 1
#else
        #error "Unspecified hardware target."
#endif
	0x3806, 0x06, // VH (VE)
	0x3807, 0xa9, // VH (VE)
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	/*// Output size, 1280x720 = 0x0500 x 0x02D0
	0x3808, 0x05, // DVPHO
	0x3809, 0x00, // DVPHO
	0x380a, 0x02, // DVPVO
	0x380b, 0xd0, // DVPVO */
	// Output size, 800x480 = 0x0320 x 0x01E0
	0x3808, 0x03, // DVPHO
	0x3809, 0x20, // DVPHO
	0x380a, 0x01, // DVPVO
	0x380b, 0xe0, // DVPVO
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	// Output size, 640x400 = 0x280 x 0x190
	0x3808, 0x02, // DVPHO
	0x3809, 0x80, // DVPHO
	0x380a, 0x01, // DVPVO
	0x380b, 0x90, // DVPVO
#else
        #error "Unspecified hardware target."
#endif
	// Total horizontal size, 1528 --> 2844 -> 2812 -> 2766 (no skipping) NATHAN TRY 1
	//0x380c, 0x05, // HTS
	//0x380d, 0xF8, // HTS
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x380c, 0x0B, // HTS - NATHAN TRY 2
	0x380d, 0x1C, // HTS - NATHAN TRY 2 --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x380c, 0x0a, // HTS
	0x380d, 0xfc, // HTS
#else
        #error "Unspecified hardware target."
#endif
	// Total vertical size, 900 --> 1968 -> 1708 (no skipping) NATHAN TRY 1
	//0x380e, 0x03, // VTS
	//0x380f, 0x84, // VTS
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x380e, 0x07, // VTS - NATHAN TRY 2
	0x380f, 0xB0, // VTS - NATHAN TRY 2 --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x380e, 0x06, // VTS
	0x380f, 0xac, // VTS
#else
        #error "Unspecified hardware target."
#endif
	// Windowing X_OFFSET, Y_OFFSET
	// 0x3810, 11, 12, 13 already set in ov5640_uxga_init_reg_tbl
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	0x3813, 0x04, // timing V offset --> re-set below in _1x[]
#elif (MK_HW_DEVICE == MK_HW_MK3B_V0_0) || (MK_HW_DEVICE == MK_HW_MK6_V0_0)|| (MK_HW_DEVICE == MK_HW_MK6_TEST_V0_0) || (MK_HW_DEVICE == MK_HW_MK9_V1_1) || (MK_HW_DEVICE == MK_HW_MK9B_V1_2) || (MK_HW_DEVICE == MK_HW_MK10_V2_2) || (MK_HW_DEVICE == MK_HW_MK11_V1_0)
	0x3813, 0x2e, // Timing Voffset[7:0] -- NATHAN TRY 1
#else
        #error "Unspecified hardware target."
#endif
	// Undocumented settings (not in Product Specification document)
	0x3618, 0x00, // undocumented? (in Linux driver 0x00 or 0x04)
	0x3612, 0x29, // undocumented? (in Linux driver 0x29 or once 0x2B)
	0x3709, 0x52, // undocumented? (in Linux driver 0x52 or 0x12)
	0x370c, 0x03, // undocumented? (in Linux driver 0x00 or 0x03)
	// AEC
	0x3a02, 0x02, // 60Hz max exposure
	0x3a03, 0xe0, // 60Hz max exposure 
	0x3a14, 0x02, // 50Hz max exposure
	0x3a15, 0xe0, // 50Hz max exposure
	// BLC
	// is it enabled or not? 0x4000 not set anywhere else; default is enabled
	// where are the other parameters?
	0x4004, 0x02, // BLC line number
	// seems JPEG-related, probably irrelevant for our purposes ?
	0x3002, 0x1c, // reset JFIFO, SFIFO, JPG
	0x3006, 0xc3, // disable clock of JPEG2x, JPEG
	0x4713, 0x03, // JPEG mode 3
	0x4407, 0x04, // Quantization scale
	0x460b, 0x37, // VFIFO debug mode, not documented
	
	0x460c, 0x20, // DVP PCLK divider control by auto mode
//	0x460c, 0x22, // manual DVP_PCLK -- NATHAN TEST 02-26
	0x4837, 0x16, // MIPI global timing
	0x3824, 0x04, // PCLK manual divider (not used if "auto mode" as above)
//	0x3824, 0x01, // PCLK manual divider DVP_PCLK=42MHz -- NATHAN TEST 02-26
//	0x3824, 0x02, // PCLK manual divider DVP_PCLK=21MHz -- NATHAN TEST 02-26
	0x5001, 0xA3, // SDE on, scale on, UV average off, color matrix on, AWB on
	0x3503, 0x00, // AEC/AGC on (auto)
}; 

/*NWA: changes 2020-04-19 for 1x zoom at 15fps:
 * skipping on
 * HS=16, VS=14
 * HW=2607, VH=1705
 * HTS=1528, VTS=900
 */
const uint16_t ov5640_rgb565_reg_tbl_1x[][2]=
{
	//skipping: on
	0x3814, 0x31, // timing X inc: odd=3, even=1 ?
	0x3815, 0x31, // timing Y inc: odd=3, even=1 ?
	//0x3814, 0x11, // no skipping -- NATHAN TRY 1
	//0x3815, 0x11, // no skipping -- NATHAN TRY 1
	// ISP input size 2592x1692
	0x3800, 0x00, // HS
	0x3801, 0x10, // HS -- 16
	0x3802, 0x00, // VS
	0x3803, 0x0E, // VS -- 14
	0x3804, 0x0a, // HW (HE)
	0x3805, 0x2f, // HW (HE) -- 0x0A2F=2607
	0x3806, 0x06, // VH (VE)
	0x3807, 0xa9, // VH (VE) -- 0x06A9=1705
	// Total horizontal size, 1528 (with skipping)
	0x380c, 0x05, // HTS
	0x380d, 0xF8, // HTS -- 0x05F8=1528
	//0x380c, 0x0a, // HTS
	//0x380d, 0xfc, // HTS
	// Total vertical size, 900 (with skipping)
	0x380e, 0x03, // VTS
	0x380f, 0x84, // VTS -- 0x0384=900
	//0x380e, 0x06, // VTS
	//0x380f, 0xac, // VTS -- 0x06AC=1708
	// HOFFSET = 16 = 0x10
	0x3810, 0x00,
	0x3811, 0x10,
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	// VOFFSET = 78 = 0x4E
	// pre-scaling size 2560x1536
	0x3812, 0x00,
	0x3813, 0x4e,
#else // all MK
	// VOFFSET = 46 = 0x2E
	// pre-scaling size 2560x1600
	0x3812, 0x00,
	0x3813, 0x2e,
#endif
}; 

/*NWA: changes 2020-04-19 for 2x (& 4x) zoom at 15fps:
 * skipping off
 * HS=0x0298=664, VS=0x01B5=437
 * HW=0x07A7=1959, VH=0x0502=1282
 * HTS, VTS no change
 */
const uint16_t ov5640_rgb565_reg_tbl_2x[][2]=
{
	//skipping: off
	0x3814, 0x11, // no skipping
	0x3815, 0x11, // no skipping
	// ISP input size 2592x1692
	0x3800, 0x02, // HS
	0x3801, 0x98, // HS -- 0x0298=664
	0x3802, 0x01, // VS
	0x3803, 0x90, // VS -- 0x0190=400
	0x3804, 0x07, // HW (HE)
	0x3805, 0xa7, // HW (HE) -- 0x07A7=1959
	0x3806, 0x04, // VH (VE)
	0x3807, 0xDF, // VH (VE) -- 0x04DF=1247
	// HOFFSET = 8 = 0x08
	0x3810, 0x00,
	0x3811, 0x08,
#if (MK_HW_DEVICE == MK_HW_DEV_BRD)
	// VOFFSET = 40 = 0x28
	// pre-scaling size 1280x768
	0x3812, 0x00,
	0x3813, 0x28,
#else // all MK
	// VOFFSET = 24 = 0x18
	// pre-scaling size 1280x800
	0x3812, 0x00,
	0x3813, 0x18,
#endif
}; 
 
//OV5640 UXGA��ʼ���Ĵ������б� 
const uint16_t ov5640_uxga_init_reg_tbl[][2]= 
{   
	// 24MHz input clock, 24MHz PCLK
	0x3008, 0x42, // software power down, bit[6]
	0x3103, 0x03, // system clock from PLL, bit[1]
	0x3017, 0xff, // FREX, Vsync, HREF, PCLK, D[9:6] output enable
	0x3018, 0xff, // D[5:0], GPIO[1:0] output enable
	0x3034, 0x1a, // MIPI 10-bit
	0x3037, 0x13, // PLL root divider, bit[4], PLL pre-divider, bit[3:0]
	0x3108, 0x01, // PCLK root divider, bit[5:4], SCLK2x root divider, bit[3:2]

	// SCLK root divider, bit[1:0]
	0x3630, 0x36,
	0x3631, 0x0e,
	0x3632, 0xe2,
	0x3633, 0x12,
	0x3621, 0xe0,
	0x3704, 0xa0,
	0x3703, 0x5a,
	0x3715, 0x78,
	0x3717, 0x01,
	0x370b, 0x60,
	0x3705, 0x1a,
	0x3905, 0x02,
	0x3906, 0x10,
	0x3901, 0x0a,
	0x3731, 0x12,
	0x3600, 0x08, // VCM control
	0x3601, 0x33, // VCM control
	0x302d, 0x60, // system control
	0x3620, 0x52,
	0x371b, 0x20,
	0x471c, 0x50,
	0x3a13, 0x43, // pre-gain = 1.047x
	0x3a18, 0x00, // gain ceiling
	0x3a19, 0xf8, // gain ceiling = 15.5x
	0x3635, 0x13,
	0x3636, 0x03,
	0x3634, 0x40,
	0x3622, 0x01,
	// 50/60Hz detection 50/60Hz �ƹ����ƹ���
	0x3c01, 0x34, // Band auto, bit[7]
	0x3c04, 0x28, // threshold low sum
	0x3c05, 0x98, // threshold high sum
	0x3c06, 0x00, // light meter 1 threshold[15:8]
	0x3c07, 0x08, // light meter 1 threshold[7:0]
	0x3c08, 0x00, // light meter 2 threshold[15:8]
	0x3c09, 0x1c, // light meter 2 threshold[7:0]
	0x3c0a, 0x9c, // sample number[15:8]
	0x3c0b, 0x40, // sample number[7:0]
	0x3810, 0x00, // Timing Hoffset[11:8]
	0x3811, 0x10, // Timing Hoffset[7:0]
	0x3812, 0x00, // Timing Voffset[10:8]
	0x3813, 0x04, // Timing Voffset[7:0]
	0x3708, 0x64,
	0x4001, 0x02, // BLC start from line 2
	0x4005, 0x1a, // BLC always update
	0x3000, 0x00, // enable blocks
	0x3004, 0xff, // enable clocks
	0x300e, 0x58, // MIPI power down, DVP enable
	0x302e, 0x00,
	0x4300, 0x30, // YUV 422, YUYV
	0x501f, 0x00, // YUV 422
	0x440e, 0x00,
	0x5000, 0xa7, // Lenc on, raw gamma on, BPC on, WPC on, CIP on
	// AEC target �Զ��ع����
	0x3a0f, 0x30, // stable range in high
	0x3a10, 0x28, // stable range in low
	0x3a1b, 0x30, // stable range out high
	0x3a1e, 0x26, // stable range out low
	0x3a11, 0x60, // fast zone high
	0x3a1f, 0x14, // fast zone low
	// Lens correction for ? ��ͷ����
	0x5800, 0x23,
	0x5801, 0x14,
	0x5802, 0x0f,
	0x5803, 0x0f,
	0x5804, 0x12,
	0x5805, 0x26,
	0x5806, 0x0c,
	0x5807, 0x08,
	0x5808, 0x05,
	0x5809, 0x05,
	0x580a, 0x08,

	0x580b, 0x0d,
	0x580c, 0x08,
	0x580d, 0x03,
	0x580e, 0x00,
	0x580f, 0x00,
	0x5810, 0x03,
	0x5811, 0x09,
	0x5812, 0x07,
	0x5813, 0x03,
	0x5814, 0x00,
	0x5815, 0x01,
	0x5816, 0x03,
	0x5817, 0x08,
	0x5818, 0x0d,
	0x5819, 0x08,
	0x581a, 0x05,
	0x581b, 0x06,
	0x581c, 0x08,
	0x581d, 0x0e,
	0x581e, 0x29,
	0x581f, 0x17,
	0x5820, 0x11,
	0x5821, 0x11,
	0x5822, 0x15,
	0x5823, 0x28,
	0x5824, 0x46,
	0x5825, 0x26,
	0x5826, 0x08,
	0x5827, 0x26,
	0x5828, 0x64,
	0x5829, 0x26,
	0x582a, 0x24,
	0x582b, 0x22,
	0x582c, 0x24,
	0x582d, 0x24,
	0x582e, 0x06,
	0x582f, 0x22,
	0x5830, 0x40,
	0x5831, 0x42,
	0x5832, 0x24,
	0x5833, 0x26,
	0x5834, 0x24,
	0x5835, 0x22,
	0x5836, 0x22,
	0x5837, 0x26,
	0x5838, 0x44,
	0x5839, 0x24,
	0x583a, 0x26,
	0x583b, 0x28,
	0x583c, 0x42,
	0x583d, 0xce, // lenc BR offset
	// AWB �Զ���ƽ��
	0x5180, 0xff, // AWB B block
	0x5181, 0xf2, // AWB control
	0x5182, 0x00, // [7:4] max local counter, [3:0] max fast counter
	0x5183, 0x14, // AWB advanced
	0x5184, 0x25,
	0x5185, 0x24,
	0x5186, 0x09,
	0x5187, 0x09,
	0x5188, 0x09,
	0x5189, 0x75,
	0x518a, 0x54,
	0x518b, 0xe0,
	0x518c, 0xb2,
	0x518d, 0x42,
	0x518e, 0x3d,
	0x518f, 0x56,
	0x5190, 0x46,
	0x5191, 0xf8, // AWB top limit
	0x5192, 0x04, // AWB bottom limit
	0x5193, 0x70, // red limit
	0x5194, 0xf0, // green limit
	0x5195, 0xf0, // blue limit
	0x5196, 0x03, // AWB control
	0x5197, 0x01, // local limit
	0x5198, 0x04,
	0x5199, 0x12,
	0x519a, 0x04,
	0x519b, 0x00,
	0x519c, 0x06,
	0x519d, 0x82,
	0x519e, 0x38, // AWB control
	// Gamma ٤������
	0x5480, 0x01, // Gamma bias plus on, bit[0]
	0x5481, 0x08,
	0x5482, 0x14,
	0x5483, 0x28,
	0x5484, 0x51,
	0x5485, 0x65,
	0x5486, 0x71,
	0x5487, 0x7d,
	0x5488, 0x87,
	0x5489, 0x91,
	0x548a, 0x9a,
	0x548b, 0xaa,
	0x548c, 0xb8,
	0x548d, 0xcd,
	0x548e, 0xdd,
	0x548f, 0xea,
	0x5490, 0x1d,
	// color matrix ɫ�ʾ���	
	0x5381, 0x1e, // CMX1 for Y
	0x5382, 0x5b, // CMX2 for Y
	0x5383, 0x08, // CMX3 for Y
	0x5384, 0x0a, // CMX4 for U
	0x5385, 0x7e, // CMX5 for U
	0x5386, 0x88, // CMX6 for U
	0x5387, 0x7c, // CMX7 for V
	0x5388, 0x6c, // CMX8 for V
	0x5389, 0x10, // CMX9 for V
	0x538a, 0x01, // sign[9]
	0x538b, 0x98, // sign[8:1]
	// UV adjust UV ɫ�ʱ��Ͷȵ���
	0x5580, 0x06, // saturation on, bit[1]
	0x5583, 0x40,
	0x5584, 0x10, 
	0x5589, 0x10,
	0x558a, 0x00,
	0x558b, 0xf8,
	0x501d, 0x40, // enable manual offset of contrast
	// CIP �񻯺ͽ���
	0x5300, 0x08, // CIP sharpen MT threshold 1
	0x5301, 0x30, // CIP sharpen MT threshold 2
	0x5302, 0x10, // CIP sharpen MT offset 1
	0x5303, 0x00, // CIP sharpen MT offset 2
	0x5304, 0x08, // CIP DNS threshold 1
	0x5305, 0x30, // CIP DNS threshold 2
	0x5306, 0x08, // CIP DNS offset 1
	0x5307, 0x16, // CIP DNS offset 2
	0x5309, 0x08, // CIP sharpen TH threshold 1
	0x530a, 0x30, // CIP sharpen TH threshold 2
	0x530b, 0x04, // CIP sharpen TH offset 1
	0x530c, 0x06, // CIP sharpen TH offset 2
	0x5025, 0x00, 
	0x3008, 0x02, // wake up from standby, bit[6]
	//�������ӵ�����
	0x4740, 0X21, //VSYNC ����Ч
};  

// Full initialization to target resolution, used by ov5640_YUV422_direct()
const uint16_t ov5640_YUV422_direct_reg_tbl[][2]= 
{   
	// 24MHz input clock, 42MHz PCLK

	0x3008, 0x42, // software power down, bit[6]

	0x3103, 0x03, // system clock from PLL, bit[1]
	0x3017, 0xff, // FREX, Vsync, HREF, PCLK, D[9:6] output enable
	0x3018, 0xff, // D[5:0], GPIO[1:0] output enable
	0x3034, 0x1a, // MIPI 10-bit
	0x3035, 0x41, // PLL
	0x3036, 0x69, // PLL
	0x3037, 0x13, // PLL root divider, bit[4], PLL pre-divider, bit[3:0]
	0x3108, 0x01, // PCLK root divider, bit[5:4], SCLK2x root divider, bit[3:2]

	// SCLK root divider, bit[1:0]
	0x3630, 0x36,
	0x3631, 0x0e,
	0x3632, 0xe2,
	0x3633, 0x12,
	0x3621, 0xe0,
	0x3704, 0xa0,
	0x3703, 0x5a,
	0x3715, 0x78,
	0x3717, 0x01,
	0x370b, 0x60,
	0x3705, 0x1a,
	0x3905, 0x02,
	0x3906, 0x10,
	0x3901, 0x0a,
	0x3731, 0x12,
	0x3600, 0x08, // VCM control
	0x3601, 0x33, // VCM control
	0x302d, 0x60, // system control
	0x3620, 0x52,
	0x371b, 0x20,
	0x471c, 0x50,
	0x3a13, 0x43, // pre-gain = 1.047x
	0x3a18, 0x00, // gain ceiling
	0x3a19, 0xf8, // gain ceiling = 15.5x
	0x3635, 0x13,
	0x3636, 0x03,
	0x3634, 0x40,
	0x3622, 0x01,
	// 50/60Hz detection 50/60Hz �ƹ����ƹ���
	0x3c01, 0x34, // Band auto, bit[7]
	0x3c04, 0x28, // threshold low sum
	0x3c05, 0x98, // threshold high sum
	0x3c06, 0x00, // light meter 1 threshold[15:8]
	0x3c07, 0x07, // light meter 1 threshold[7:0]
	0x3c08, 0x00, // light meter 2 threshold[15:8]
	0x3c09, 0x1c, // light meter 2 threshold[7:0]
	0x3c0a, 0x9c, // sample number[15:8]
	0x3c0b, 0x40, // sample number[7:0]
	// Windowing X_OFFSET, Y_OFFSET
	// pre-scaling size 2560x1600
	0x3810, 0x00, // Timing Hoffset[11:8]
	0x3811, 0x10, // Timing Hoffset[7:0] HOFFSET = 16 = 0x10
	0x3812, 0x00, // Timing Voffset[10:8]
	0x3813, 0x2e, // Timing Voffset[7:0] VOFFSET = 46 = 0x2E

	0x3708, 0x64,
	0x4001, 0x02, // BLC start from line 2
	0x4005, 0x1a, // BLC always update
	0x3000, 0x00, // enable blocks
	0x3004, 0xff, // enable clocks
	0x300e, 0x58, // MIPI power down, DVP enable
	0x302e, 0x00,
	0x4300, 0x30, // YUV 422, YUYV
	0x501f, 0x00, // YUV 422
	0x440e, 0x00,
	0x5000, 0xa7, // Lenc on, raw gamma on, BPC on, WPC on, CIP on
	// AEC target �Զ��ع����
	0x3a0f, 0x30, // stable range in high
	0x3a10, 0x28, // stable range in low
	0x3a1b, 0x30, // stable range out high
	0x3a1e, 0x26, // stable range out low
	0x3a11, 0x60, // fast zone high
	0x3a1f, 0x14, // fast zone low
	// Lens correction for ? ��ͷ����
	0x5800, 0x23,
	0x5801, 0x14,
	0x5802, 0x0f,
	0x5803, 0x0f,
	0x5804, 0x12,
	0x5805, 0x26,
	0x5806, 0x0c,
	0x5807, 0x08,
	0x5808, 0x05,
	0x5809, 0x05,
	0x580a, 0x08,

	0x580b, 0x0d,
	0x580c, 0x08,
	0x580d, 0x03,
	0x580e, 0x00,
	0x580f, 0x00,
	0x5810, 0x03,
	0x5811, 0x09,
	0x5812, 0x07,
	0x5813, 0x03,
	0x5814, 0x00,
	0x5815, 0x01,
	0x5816, 0x03,
	0x5817, 0x08,
	0x5818, 0x0d,
	0x5819, 0x08,
	0x581a, 0x05,
	0x581b, 0x06,
	0x581c, 0x08,
	0x581d, 0x0e,
	0x581e, 0x29,
	0x581f, 0x17,
	0x5820, 0x11,
	0x5821, 0x11,
	0x5822, 0x15,
	0x5823, 0x28,
	0x5824, 0x46,
	0x5825, 0x26,
	0x5826, 0x08,
	0x5827, 0x26,
	0x5828, 0x64,
	0x5829, 0x26,
	0x582a, 0x24,
	0x582b, 0x22,
	0x582c, 0x24,
	0x582d, 0x24,
	0x582e, 0x06,
	0x582f, 0x22,
	0x5830, 0x40,
	0x5831, 0x42,
	0x5832, 0x24,
	0x5833, 0x26,
	0x5834, 0x24,
	0x5835, 0x22,
	0x5836, 0x22,
	0x5837, 0x26,
	0x5838, 0x44,
	0x5839, 0x24,
	0x583a, 0x26,
	0x583b, 0x28,
	0x583c, 0x42,
	0x583d, 0xce, // lenc BR offset
	// AWB �Զ���ƽ��
	0x5180, 0xff, // AWB B block
	0x5181, 0xf2, // AWB control
	0x5182, 0x00, // [7:4] max local counter, [3:0] max fast counter
	0x5183, 0x14, // AWB advanced
	0x5184, 0x25,
	0x5185, 0x24,
	0x5186, 0x09,
	0x5187, 0x09,
	0x5188, 0x09,
	0x5189, 0x75,
	0x518a, 0x54,
	0x518b, 0xe0,
	0x518c, 0xb2,
	0x518d, 0x42,
	0x518e, 0x3d,
	0x518f, 0x56,
	0x5190, 0x46,
	0x5191, 0xf8, // AWB top limit
	0x5192, 0x04, // AWB bottom limit
	0x5193, 0x70, // red limit
	0x5194, 0xf0, // green limit
	0x5195, 0xf0, // blue limit
	0x5196, 0x03, // AWB control
	0x5197, 0x01, // local limit
	0x5198, 0x04,
	0x5199, 0x12,
	0x519a, 0x04,
	0x519b, 0x00,
	0x519c, 0x06,
	0x519d, 0x82,
	0x519e, 0x38, // AWB control
	// Gamma ٤������
	0x5480, 0x01, // Gamma bias plus on, bit[0]
	0x5481, 0x08,
	0x5482, 0x14,
	0x5483, 0x28,
	0x5484, 0x51,
	0x5485, 0x65,
	0x5486, 0x71,
	0x5487, 0x7d,
	0x5488, 0x87,
	0x5489, 0x91,
	0x548a, 0x9a,
	0x548b, 0xaa,
	0x548c, 0xb8,
	0x548d, 0xcd,
	0x548e, 0xdd,
	0x548f, 0xea,
	0x5490, 0x1d,
	// color matrix ɫ�ʾ���	
	0x5381, 0x1e, // CMX1 for Y
	0x5382, 0x5b, // CMX2 for Y
	0x5383, 0x08, // CMX3 for Y
	0x5384, 0x0a, // CMX4 for U
	0x5385, 0x7e, // CMX5 for U
	0x5386, 0x88, // CMX6 for U
	0x5387, 0x7c, // CMX7 for V
	0x5388, 0x6c, // CMX8 for V
	0x5389, 0x10, // CMX9 for V
	0x538a, 0x01, // sign[9]
	0x538b, 0x98, // sign[8:1]
	// UV adjust UV ɫ�ʱ��Ͷȵ���
	0x5580, 0x06, // saturation on, bit[1]
	0x5583, 0x40,
	0x5584, 0x10, 
	0x5589, 0x10,
	0x558a, 0x00,
	0x558b, 0xf8,
	0x501d, 0x40, // enable manual offset of contrast
	// CIP �񻯺ͽ���
	0x5300, 0x08, // CIP sharpen MT threshold 1
	0x5301, 0x30, // CIP sharpen MT threshold 2
	0x5302, 0x10, // CIP sharpen MT offset 1
	0x5303, 0x00, // CIP sharpen MT offset 2
	0x5304, 0x08, // CIP DNS threshold 1
	0x5305, 0x30, // CIP DNS threshold 2
	0x5306, 0x08, // CIP DNS offset 1
	0x5307, 0x16, // CIP DNS offset 2
	0x5309, 0x08, // CIP sharpen TH threshold 1
	0x530a, 0x30, // CIP sharpen TH threshold 2
	0x530b, 0x04, // CIP sharpen TH offset 1
	0x530c, 0x06, // CIP sharpen TH offset 2
	0x5025, 0x00, 

	0x4740, 0X21, //VSYNC ����Ч
	
	// NEW
	//skipping: on
	0x3814, 0x31, // timing X inc: odd=3, even=1 ?
	0x3815, 0x31, // timing Y inc: odd=3, even=1 ?
	// ISP input size 2592x1692
	0x3800, 0x00, // HS
	0x3801, 0x10, // HS -- 16
	0x3802, 0x00, // VS
	0x3803, 0x0E, // VS -- 14
	0x3804, 0x0a, // HW (HE)
	0x3805, 0x2f, // HW (HE) -- 0x0A2F=2607
	0x3806, 0x06, // VH (VE)
	0x3807, 0xa9, // VH (VE) -- 0x06A9=1705
	// Output size, 640x400 = 0x280 x 0x190
	0x3808, 0x02, // DVPHO
	0x3809, 0x80, // DVPHO
	0x380a, 0x01, // DVPVO
	0x380b, 0x90, // DVPVO
	// Total horizontal size, 1528 (with skipping)
	0x380c, 0x05, // HTS
	0x380d, 0xF8, // HTS -- 0x05F8=1528
	// Total vertical size, 900 (with skipping)
	0x380e, 0x03, // VTS
	0x380f, 0x84, // VTS -- 0x0384=900
	// Undocumented settings (not in Product Specification document)
	0x3618, 0x00, // undocumented? (in Linux driver 0x00 or 0x04)
	0x3612, 0x29, // undocumented? (in Linux driver 0x29 or once 0x2B)
	0x3709, 0x52, // undocumented? (in Linux driver 0x52 or 0x12)
	0x370c, 0x03, // undocumented? (in Linux driver 0x00 or 0x03)
	// AEC
	0x3a02, 0x02, // 60Hz max exposure
	0x3a03, 0xe0, // 60Hz max exposure 
	0x3a14, 0x02, // 50Hz max exposure
	0x3a15, 0xe0, // 50Hz max exposure
	// BLC
	// is it enabled or not? 0x4000 not set anywhere else; default is enabled
	// where are the other parameters?
	0x4004, 0x02, // BLC line number
	// seems JPEG-related, probably irrelevant for our purposes ?
	0x3002, 0x1c, // reset JFIFO, SFIFO, JPG
	0x3006, 0xc3, // disable clock of JPEG2x, JPEG
	0x4713, 0x03, // JPEG mode 3
	0x4407, 0x04, // Quantization scale
	0x460b, 0x37, // VFIFO debug mode, not documented

	0x460c, 0x20, // DVP PCLK divider control by auto mode
	0x4837, 0x16, // MIPI global timing
	0x3824, 0x04, // PCLK manual divider (not used if "auto mode" as above)
	0x5001, 0xA3, // SDE on, scale on, UV average off, color matrix on, AWB on
	0x3503, 0x00, // AEC/AGC on (auto)
	
	0x3008, 0x02, // wake up from standby, bit[6]
};

#endif //_OV5640CFG_H


















