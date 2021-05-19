
//*****************************************************************************
//  MT9D111.c
//
// Micron MT9D111 camera sensor driver
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
//*****************************************************************************
//
//! \addtogroup mt9d111
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "mt9d111.h"
//#include "hw_types.h"
//#include "rom.h"
//#include "rom_map.h"
//#include "hw_memmap.h"
#include "i2c.h"
//#include "i2cconfig.h"
//#include "common.h"

#define RET_OK                  0
#define RET_ERROR               -1
#define SENSOR_PAGE_REG         0xF0
#define CAM_I2C_WRITE_ADDR      0x90
#define CAM_I2C_READ_ADDR       0x91
#define CAM_I2C_VAR_ADDR        0xC6
#define CAM_I2C_VAR_DATA 		0xC8
#define ENABLE_JPEG

#define JPEG_HEADER_WIDTH_INDEX
#define JPEG_HEADER_HEIGHT_INDEX


uint8_t jpegHeader[JPEG_HEADER_SIZE] =
{
  0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01,
  0x00, 0x01, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x84, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03,
  0x03, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04, 0x05, 0x09, 0x07,
  0x07, 0x05, 0x08, 0x0b, 0x0a, 0x0b, 0x0b, 0x0b, 0x0a, 0x0b, 0x0a, 0x0c, 0x0e, 0x11, 0x0f, 0x0c,
  0x0d, 0x10, 0x0d, 0x0a, 0x0b, 0x0f, 0x14, 0x0f, 0x10, 0x12, 0x12, 0x13, 0x14, 0x13, 0x0c, 0x0e,
  0x15, 0x17, 0x15, 0x13, 0x17, 0x11, 0x13, 0x13, 0x13, 0x01, 0x03, 0x03, 0x03, 0x05, 0x04, 0x05,
  0x09, 0x05, 0x05, 0x09, 0x13, 0x0c, 0x0b, 0x0c, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x01,
  0xE0, 0x02, 0x80, 0x03, 0x00, 0x21, 0x00, 0x01, 0x11, 0x01, 0x02, 0x11, 0x01, 0xff, 0xc4, 0x00,
  0x1f, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4,
  0x00, 0xb5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
  0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13,
  0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
  0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25,
  0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46,
  0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86,
  0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4,
  0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
  0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
  0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xc4, 0x00, 0x1f, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04,
  0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11,
  0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08,
  0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a,
  0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35,
  0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55,
  0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75,
  0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93,
  0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
  0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
  0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
  0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xdd, 0x00,
  0x04, 0x00, 0x20, 0xff, 0xda, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x01, 0x11, 0x02, 0x11, 0x00, 0x3f,
  0x00
};

const uint16_t patch_addr_0400[206]  = {
  0x0400,
  0x00cc,
  0xce06, 0x10ed, 0x0230, 0xec00, 0xce06, 0x10ed, 0x00ce, 0x0610,
  0xec04, 0xc300, 0x01ed, 0x04ce, 0x1070, 0xc603, 0xe70f, 0xc611,
  0xe740, 0xc680, 0xe742, 0xc603, 0xe701, 0xc6fe, 0xe74c, 0xc600,
  0xe71a, 0xe71b, 0xe710, 0xe712, 0xc680, 0xe711, 0xc680, 0xe713,
  0xc601, 0xe744, 0xc6fe, 0xe746, 0x39ce, 0x0620, 0xed02, 0x30ec,
  0x00ce, 0x0620, 0xed00, 0xce06, 0x20e6, 0x05cb, 0x01c4, 0x7fe7,
  0x05ce, 0x0720, 0x3a18, 0xce06, 0x2018, 0xe603, 0xe700, 0x3c34,
  0xce01, 0x0de6, 0x03e7, 0x04ce, 0x0620, 0xe603, 0xce01, 0x0de7,
  0x03ce, 0x1070, 0xc602, 0xe703, 0xc6ff, 0xe746, 0xc601, 0xe745,
  0x18ce, 0x010d, 0x18e6, 0x034f, 0xc300, 0x0105, 0xed10, 0x18e6,
  0x034f, 0x1830, 0x18ed, 0x01cc, 0x0101, 0x18a3, 0x0105, 0xed12,
  0xc600, 0xe745, 0xc6fe, 0xe746, 0xc602, 0xe703, 0x4fcc, 0x0050,   // 50 is time needed for lens setling
  0xbd9b, 0x1118, 0xce01, 0x0d18, 0xe605, 0xca02, 0x18e7, 0x0530,
  0xc603, 0x3a35, 0x39ce, 0x0630, 0xed02, 0x30ec, 0x00ce, 0x0630,
  0xed00, 0xce06, 0x30ec, 0x08c3, 0x0001, 0xed08, 0x39ce, 0x0640,
  0xed02, 0x30ec, 0x00ce, 0x0640, 0xed00, 0xcc06, 0x40ec, 0x08c3,
  0x0001, 0xed08, 0xbd9c, 0x43c1, 0x0126, 0x1218, 0xce01, 0x0d18,
  0xe605, 0xf400, 0xfd18, 0xe705, 0xce06, 0x40e7, 0x04ce, 0x0640,
  0xe704, 0x18ce, 0x010d, 0x18e6, 0x0539, 0x3c3c, 0x3c3c, 0x34cc,
  0x02c4, 0x30ed, 0x06fe, 0x1050, 0xec0c, 0xfd02, 0xc0fe, 0x02c0,
  0xec00, 0xfd02, 0xc230, 0x6f08, 0xe608, 0x4f05, 0xf302, 0xc28f,
  0xec00, 0x30ed, 0x00e6, 0x084f, 0x05e3, 0x0618, 0x8fec, 0x0018,
  0xed00, 0x6c08, 0xe608, 0xc104, 0x25de, 0x30ee, 0x06cc, 0x0400,
  0xed00, 0x30ee, 0x06cc, 0x0449, 0xed02, 0x30ee, 0x06cc, 0x04d5,
  0xed04, 0x30ee, 0x06cc, 0x04ed, 0xed06, 0xcc02, 0xc4fe, 0x010d,
  0xed00, 0x30c6, 0x093a, 0x3539
};


uint8_t jpegFooter[JPEG_FOOTER_SIZE] = {0xFF, 0xD9};

typedef struct MT9D111RegLst {
	unsigned char ucPageAddr;
	unsigned char ucRegAddr;
	unsigned short usValue;
} s_RegList;

#ifndef ENABLE_JPEG
static const s_RegList preview_on_cmd_list[]= {
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0001    },  // SEQ_CMD, Do Preview
    {1, 0xC6, 0xA104    },  // SEQ_CMD
    {111, 0xC8, 0x0003  },  // SEQ_CMD, Do Preview
    {1, 0xC6, 0xA103    },  // SEQ_CMD-refresh
    {1, 0xC8, 0x0005    },  // SEQ_CMD-refresh
    {1, 0xC6, 0xA103    },  // SEQ_CMD-refresh
    {1, 0xC8, 0x0006    },  // SEQ_CMD-refresh
    {1, 0xC6, 0xA104    },  // SEQ_CMD
    {111, 0xC8, 0x0003  },  // SEQ_CMD, Do Preview
    {100, 0x00, 0x01E0  },  // Delay = 500ms
};

static  const s_RegList freq_setup_cmd_List[]= {
    {1, 0xC6, 0x276D    },  // MODE_FIFO_CONF1_A
    {1, 0xC8, 0xE4E2    },  // MODE_FIFO_CONF1_A
    {1, 0xC6, 0xA76F    },  // MODE_FIFO_CONF2_A
    {1, 0xC8, 0x00E8    },  // MODE_FIFO_CONF2_A
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD (Refresh)
   // Set maximum integration time to get a minimum of 15 fps at 45MHz
    {1, 0xC6, 0xA20E    },  // AE_MAX_INDEX
    {1, 0xC8, 0x0004},      // AE_MAX_INDEX
    {1, 0xC6, 0xA102    },  // SEQ_MODE
    {1, 0xC8, 0x0001    },  // SEQ_MODE
    {1, 0xC6, 0xA102    },  // SEQ_MODE
    {1, 0xC8, 0x0005    },  // SEQ_MODE
   // Set minimum integration time to get a maximum of 15 fps at 45MHz
    {1, 0xC6, 0xA20D    },  // AE_MAX_INDEX
    {1, 0xC8, 0x0004    },  // AE_MAX_INDEX
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD (Refresh)
};

static  const s_RegList image_size_240_320_preview_cmds_list[]=
{
    {0, 0x07, 0x00FE    },  // HORZ_BLANK_A
    {0, 0x08, 0x02A0    },  // VERT_BLANK_A
    {0, 0x20, 0x0303    },  // READ_MODE_B (Image flip settings)
    {0, 0x21, 0x8400    },  // READ_MODE_A (1ADC)
    {1, 0xC6, 0x2703    },  // MODE_OUTPUT_WIDTH_A
    {1, 0xC8, 0x00F0    },  // MODE_OUTPUT_WIDTH_A
    {1, 0xC6, 0x2705    },  // MODE_OUTPUT_HEIGHT_A
    {1, 0xC8, 0x0140    },  // MODE_OUTPUT_HEIGHT_A
    {1, 0xC6, 0x2727    },  // MODE_CROP_X0_A
    {1, 0xC8, 0x0000    },  // MODE_CROP_X0_A
    {1, 0xC6, 0x2729    },  // MODE_CROP_X1_A
    {1, 0xC8, 0x00F0    },  // MODE_CROP_X1_A
    {1, 0xC6, 0x272B    },  // MODE_CROP_Y0_A
    {1, 0xC8, 0x0000    },  // MODE_CROP_Y0_A
    {1, 0xC6, 0x272D    },  // MODE_CROP_Y1_A
    {1, 0xC8, 0x0140    },  // MODE_CROP_Y1_A
    {1, 0xC6, 0x270F    },  // MODE_SENSOR_ROW_START_A
    {1, 0xC8, 0x001C    },  // MODE_SENSOR_ROW_START_A
    {1, 0xC6, 0x2711    },  // MODE_SENSOR_COL_START_A
    {1, 0xC8, 0x003C    },  // MODE_SENSOR_COL_START_A
    {1, 0xC6, 0x2713    },  // MODE_SENSOR_ROW_HEIGHT_A
    {1, 0xC8, 0x0280    },  // MODE_SENSOR_ROW_HEIGHT_A
    {1, 0xC6, 0x2715    },  // MODE_SENSOR_COL_WIDTH_A
    {1, 0xC8, 0x03C0    },  // MODE_SENSOR_COL_WIDTH_A
    {1, 0xC6, 0x2717    },  // MODE_SENSOR_X_DELAY_A
    {1, 0xC8, 0x0088    },  // MODE_SENSOR_X_DELAY_A
    {1, 0xC6, 0x2719    },  // MODE_SENSOR_ROW_SPEED_A
    {1, 0xC8, 0x0011    },  // MODE_SENSOR_ROW_SPEED_A
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0006    },  // SEQ_CMD
};

static  const s_RegList preview_cmds_list[]= {

    {1, 0xC6, 0xA77D    },  // MODE_OUTPUT_FORMAT_A
    {1, 0xC8, 0x0020    },  // MODE_OUTPUT_FORMAT_A; RGB565
    {1, 0xC6, 0x270B    },  // MODE_CONFIG
    {1, 0xC8, 0x0030    },  // MODE_CONFIG, JPEG disabled for A and B
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    }   // SEQ_CMD, refresh
};
#else
static const s_RegList capture_cmds_list[] = { { 0, 0x65, 0xA000 }, // Disable PLL
		{ 0, 0x65, 0xE000 },  // Power DOWN PLL
		{ 100, 0x00, 0x01F4 },  // Delay =500ms
		{ 0, 0x66, 0x500B }, { 0, 0x67, 0x0500 }, { 0, 0x65, 0xA000 }, // Disable PLL
		{ 0, 0x65, 0x2000 },  // Enable PLL
		{ 0, 0x20, 0x0000 },  // READ_MODE_B (Image flip settings)
		{ 100, 0x00, 0x01F4 },  // Delay =500ms
		{ 100, 0x00, 0x01F4 },  // Delay =500ms
		{ 100, 0x00, 0x01F4 },  // Delay =500ms
		{ 1, 0xC6, 0xA102 },  // SEQ_MODE
		{ 1, 0xC8, 0x0001 },  // SEQ_MODE
		{ 1, 0xC6, 0xA102 },  // SEQ_MODE
		{ 1, 0xC8, 0x0005 },  // SEQ_MODE
		{ 1, 0xC6, 0xA120 },  // Enable Capture video <<----now set to capture, not video)
		{ 1, 0xC8, 0x0001 },//001 for snapshot /////<<=======================================================originally 0x0002
		{ 1, 0xC6, 0x270B },  // Mode config, disable JPEG bypass
		{ 1, 0xC8, 0x0000 }, { 1, 0xC6, 0x2702 }, // FIFO_config0b, no spoof, adaptive clock
		{ 1, 0xC8, 0x001E },
		{ 1, 0xC6, 0x2772 }, // FIFO_config0b, no spoof, adaptive clock
		{ 1, 0xC8, 0x005F },
		{1,0xC6, 0x2779},
		{1,0xC8,1100}, //Spoof width
		{1,0xC6,0x277B},
		{1,0xC8, 47},//Spoof height

		{ 1, 0xC6, 0xA907 },  // JPEG mode config, video <<---Now single frame)
		{ 1, 0xC8, 0x0056 },  // Format YCbCr422
		{ 1, 0xC8, 0x0000 }, { 1, 0xC6, 0xA90A },  // Set the qscale1
		{ 1, 0xC8, 0x0089 }, { 1, 0xC6, 0x2908 },  // Set the restartInt
		{ 1, 0xC8, 0x0020 },

		{1,0x48,0x0303},
};

static s_RegList start_jpeg_capture_cmd_list[] = {
		//{ 1, 0xC6, 0xA103 }, // SEQ_CMD, Do capture
		//{ 2, 0x0D, 0x001F },
		{ 1, 0xC6, 0xA103 }, // SEQ_CMD, Do capture
		{ 1, 0xC8, 0x0002 },
		{ 2, 0x02,0x0101},
		{ 100, 0x00, 0x05F4 },  // Delay


		{ 100, 0x00, 0x01F4 },  // Delay =500ms
		};

static s_RegList get_jpeg_status[] = {

		{ 221, 0x02,0x0000}

};

static s_RegList set_adaptive_clock_divisors[]={

		{1, 0xC6, 0x2772},
		{1,0xC8,100},

		{ 100, 0x00, 0x05F4 },

		{1,0xC6,0x2774},
		{1,0xC8,100},

		{ 100, 0x00, 0x05F4 },

		{1,0xC6, 0x8776},
		{1,0xC8,100}


};

static s_RegList spoofSize[] = {

		{2, 0x10,400},
		{2,0x11,120},
};
static s_RegList handshake_cmd[] = {
		{ 1, 0xC6, 0xA907 },  // JPEG mode config, video <<---Now single frame)
		{ 1, 0xC8, 0x003C }
};

static s_RegList stop_jpeg_capture_cmd_list[] = { { 1, 0xC6, 0xA103 }, // SEQ_CMD, Do capture
		{ 1, 0xC8, 0x0001 }, { 100, 0x00, 0x01F4 },  // Delay =500ms
		};

static s_RegList check_jpeg_size_cmd_list[] = {
		{ 1, 0xC6, 0x2910 }, { 222, 0xC8, 0x0000},
		{ 100, 0x00, 0x05F4 },
		{ 1, 0xC6, 0xa90f }, { 224, 0xC8, 0x0000}
		};

static s_RegList check_jpeg_width_cmd_list[] = {
		{ 1, 0xC6, 0x2902 }, { 150, 0xC8, 0x0000}
		};


static s_RegList check_jpeg_height_cmd_list[] = {
		{ 1, 0xC6, 0x2904 }, { 150, 0xC8, 0x0000}
		};

#define INDEX_CROP_X0           1
#define INDEX_CROP_X1           3
#define INDEX_CROP_Y0           5
#define INDEX_CROP_Y1           7
#define INDEX_SIZE_WIDTH        12//9
#define INDEX_SIZE_HEIGHT       14//11
static s_RegList resolution_cmds_list[] = { { 100, 0x00, 0x01F4 }, // Delay =500ms
		{ 1, 0xC6, 0x2735 }, //MODE_CROP_X0_A   <<Actually B
		{ 1, 0xC8, 0x0000 }, //MODE_CROP_X0_A   <<Actually B
		{ 1, 0xC6, 0x2737 }, //MODE_CROP_X1_A   <<Actually B
		{ 1, 0xC8, 1600,  }, //MODE_CROP_X1_A   <<Actually B ////originally 1600
		{ 1, 0xC6, 0x2739 }, //MODE_CROP_Y0_A   <<Actually B
		{ 1, 0xC8, 0x0000 }, //MODE_CROP_Y0_A   <<Actually B
		{ 1, 0xC6, 0x273B }, //MODE_CROP_Y1_A   <<Actually B
		{ 1, 0xC8, 1200 }, //MODE_CROP_Y1_A   <<Actually B ////originally 1200
		{ 1, 0xC6, 0xA103 },  // SEQ_CMD, Do capture <<Actually Refresh
		{ 1, 0xC8, 0x0005 },

		{ 1, 0xC6, 0x2707 }, //MODE_OUTPUT_WIDTH_B <<correct
		{ 1, 0xC8, 1600 }, //MODE_OUTPUT_WIDTH_B
		{ 1, 0xC6, 0x2709 }, //MODE_OUTPUT_HEIGHT_B <<correct
		{ 1, 0xC8, 1200 }, //MODE_OUTPUT_HEIGHT_B
		};
#endif

static const s_RegList init_cmds_list[] = { { 100, 0x00, 0x01F4 }, { 0, 0x33,
		0x0343 }, // RESERVED_CORE_33
		{ 1, 0xC6, 0xA115 }, // SEQ_LLMODE  <<
		{ 1, 0xC8, 0x0020 }, // SEQ_LLMODE  << Flips bit 6?? Nothing in data sheet for that
		{ 0, 0x38, 0x0866 }, // RESERVED_CORE_38
		{ 2, 0x80, 0x0168 }, // LENS_CORRECTION_CONTROL
		{ 2, 0x81, 0x6432 }, // ZONE_BOUNDS_X1_X2
		{ 2, 0x82, 0x3296 }, // ZONE_BOUNDS_X0_X3
		{ 2, 0x83, 0x9664 }, // ZONE_BOUNDS_X4_X5
		{ 2, 0x84, 0x5028 }, // ZONE_BOUNDS_Y1_Y2
		{ 2, 0x85, 0x2878 }, // ZONE_BOUNDS_Y0_Y3
		{ 2, 0x86, 0x7850 }, // ZONE_BOUNDS_Y4_Y5
		{ 2, 0x87, 0x0000 }, // CENTER_OFFSET
		{ 2, 0x88, 0x0152 }, // FX_RED
		{ 2, 0x89, 0x015C }, // FX_GREEN
		{ 2, 0x8A, 0x00F4 }, // FX_BLUE
		{ 2, 0x8B, 0x0108 }, // FY_RED
		{ 2, 0x8C, 0x00FA }, // FY_GREEN
		{ 2, 0x8D, 0x00CF }, // FY_BLUE
		{ 2, 0x8E, 0x09AD }, // DF_DX_RED
		{ 2, 0x8F, 0x091E }, // DF_DX_GREEN
		{ 2, 0x90, 0x0B3F }, // DF_DX_BLUE
		{ 2, 0x91, 0x0C85 }, // DF_DY_RED
		{ 2, 0x92, 0x0CFF }, // DF_DY_GREEN
		{ 2, 0x93, 0x0D86 }, // DF_DY_BLUE
		{ 2, 0x94, 0x163A }, // SECOND_DERIV_ZONE_0_RED
		{ 2, 0x95, 0x0E47 }, // SECOND_DERIV_ZONE_0_GREEN
		{ 2, 0x96, 0x103C }, // SECOND_DERIV_ZONE_0_BLUE
		{ 2, 0x97, 0x1D35 }, // SECOND_DERIV_ZONE_1_RED
		{ 2, 0x98, 0x173E }, // SECOND_DERIV_ZONE_1_GREEN
		{ 2, 0x99, 0x1119 }, // SECOND_DERIV_ZONE_1_BLUE
		{ 2, 0x9A, 0x1663 }, // SECOND_DERIV_ZONE_2_RED
		{ 2, 0x9B, 0x1569 }, // SECOND_DERIV_ZONE_2_GREEN
		{ 2, 0x9C, 0x104C }, // SECOND_DERIV_ZONE_2_BLUE
		{ 2, 0x9D, 0x1015 }, // SECOND_DERIV_ZONE_3_RED
		{ 2, 0x9E, 0x1010 }, // SECOND_DERIV_ZONE_3_GREEN
		{ 2, 0x9F, 0x0B0A }, // SECOND_DERIV_ZONE_3_BLUE
		{ 2, 0xA0, 0x0D53 }, // SECOND_DERIV_ZONE_4_RED
		{ 2, 0xA1, 0x0D51 }, // SECOND_DERIV_ZONE_4_GREEN
		{ 2, 0xA2, 0x0A44 }, // SECOND_DERIV_ZONE_4_BLUE
		{ 2, 0xA3, 0x1545 }, // SECOND_DERIV_ZONE_5_RED
		{ 2, 0xA4, 0x1643 }, // SECOND_DERIV_ZONE_5_GREEN
		{ 2, 0xA5, 0x1231 }, // SECOND_DERIV_ZONE_5_BLUE
		{ 2, 0xA6, 0x0047 }, // SECOND_DERIV_ZONE_6_RED
		{ 2, 0xA7, 0x035C }, // SECOND_DERIV_ZONE_6_GREEN
		{ 2, 0xA8, 0xFE30 }, // SECOND_DERIV_ZONE_6_BLUE
		{ 2, 0xA9, 0x4625 }, // SECOND_DERIV_ZONE_7_RED
		{ 2, 0xAA, 0x47F3 }, // SECOND_DERIV_ZONE_7_GREEN
		{ 2, 0xAB, 0x5859 }, // SECOND_DERIV_ZONE_7_BLUE
		{ 2, 0xAC, 0x0000 }, // X2_FACTORS
		{ 2, 0xAD, 0x0000 }, // GLOBAL_OFFSET_FXY_FUNCTION
		{ 2, 0xAE, 0x0000 }, // K_FACTOR_IN_K_FX_FY
		{ 1, 0x08, 0x01FC }, // COLOR_PIPELINE_CONTROL
		{ 1, 0xC6, 0x2003 }, // MON_ARG1  (Call firmware function
		{ 1, 0xC8, 0x0748 }, // MON_ARG1 <<access register function 0x0748?
		{ 1, 0xC6, 0xA002 }, // MON_CMD
		{ 1, 0xC8, 0x0001 }, // MON_CMD
		{ 111, 0xC8, 0x0000 },  //waits for result
		{ 1, 0xC6, 0xA361 }, // AWB_TG_MIN0 <<correct
		{ 1, 0xC8, 0x00E2 }, // AWB_TG_MIN0 <<description of this	 process is "reserved"
		{ 1, 0x1F, 0x0018 }, // RESERVED_SOC1_1F
		{ 1, 0x51, 0x7F40 }, // RESERVED_SOC1_51
		{ 0, 0x33, 0x0343 }, // RESERVED_CORE_33
		{ 0, 0x38, 0x0868 }, // RESERVED_CORE_38
		{ 1, 0xC6, 0xA10F }, // SEQ_RESET_LEVEL_TH
		{ 1, 0xC8, 0x0042 }, // SEQ_RESET_LEVEL_TH
		{ 1, 0x1F, 0x0020 }, // RESERVED_SOC1_1F
		{ 1, 0xC6, 0xAB04 }, // HG_MAX_DLEVEL
		{ 1, 0xC8, 0x0008 }, // HG_MAX_DLEVEL
		{ 1, 0xC6, 0xA103 }, // SEQ_CMD
		{ 1, 0xC8, 0x0005 }, // SEQ_CMD
		{ 1, 0xC6, 0xA104 }, // SEQ_CMD
		{ 111, 0xC8, 0x0003 }, { 1, 0x08, 0x01FC }, // COLOR_PIPELINE_CONTROL
		{ 1, 0x08, 0x01EC }, // COLOR_PIPELINE_CONTROL
		{ 1, 0x08, 0x01FC }, // COLOR_PIPELINE_CONTROL
		{ 1, 0x36, 0x0F08 }, // APERTURE_PARAMETERS
		{ 1, 0xC6, 0xA103 }, // SEQ_CMD
		{ 1, 0xC8, 0x0005 }, // SEQ_CMD
		};
//*****************************************************************************
// Conversions of Previous Arduino Functions to Structures
//*****************************************************************************
static const s_RegList AF_refocus_cmds_list[] = { { 1, 0x08, 0xA102 }, { 1,
		0x36, 0x0031 }, { 1, 0xC6, 0xA504 }, { 1, 0xC8, 0x0001 }, };

static const s_RegList Ryans_resetCam_cmds_list[] = { { 0, 0x65, 0xA000 }, ///SOFT RESET PRCEEDURE
		{ 1, 0xC3, 0x0501 }, { 0, 0x0D, 0x0021 }, ///SOFT RESET ENABLED
		{ 100, 0x00, 0x0710 }, //delay 10 seconds
		{ 0, 0x0D, 0x0000 },
//	{100, 0x00, 0x2710  }, //delay 10 seconds

		};

static const s_RegList Ryans_Massive_cmds_list[] = {

{ 0, 0x0D, 0x0000 }, ///SOFT RESET DISABLED
		{ 1, 0xC6, 0xA103 },  // SEQ_CMD, Do capture
		{ 1, 0xC8, 0x0003 },

//	{1, 0xC6, 0xA120   	},
//	{1, 0xC8, 0x0001 	},
		{ 1, 0xC6, 0xA735 }, //MODE_CROP_X0_A   <<Actually B
		{ 1, 0xC8, 0x0100 }, //MODE_CROP_X0_A   <<Actually B
		{ 1, 0xC6, 0xA737 }, //MODE_CROP_X1_A   <<Actually B
		{ 1, 0xC8, 1600 }, //MODE_CROP_X1_A   <<Actually B
		{ 1, 0xC6, 0xA739 }, //MODE_CROP_Y0_A   <<Actually B
		{ 1, 0xC8, 0x0000 }, //MODE_CROP_Y0_A   <<Actually B
		{ 1, 0xC6, 0xA73B }, //MODE_CROP_Y1_A   <<Actually B
		{ 1, 0xC8, 1200 }, //MODE_CROP_Y1_A   <<Actually B
		{ 1, 0xC6, 0xA702 }, //MODE_CROP_Y1_A   <<Actually B
		{ 1, 0xC8, 0x0001 }, //MODE_CROP_Y1_A   <<Actually B
		{ 1, 0xC6, 0x2133 }, //Set_Num_Frames
		{ 1, 0xC8, 0x0001 }, //to 1
		{ 1, 0xC6, 0x2103 }, //Set_Num_Frames
		{ 1, 0xC8, 0x0002 }, //to 1
//	{0, 0x0D, 0x0021    },
//	{100, 0x00, 0x2710  }, //delay 10 seconds
//	{0, 0x0D, 0x0000    },

		};

//*****************************************************************************
// Static Function Declarations
//*****************************************************************************
static long RegLstWrite(s_RegList *pRegLst, unsigned long ulNofItems);
static long RegLstWriteLength(s_RegList *pRegLst, unsigned long ulNofItems,uint32_t* length);

void MX_I2C2_Init(void);

//*****************************************************************************
//
//! This function initilizes the camera sensor
//!
//! \param                      None
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************

void applyPatch(){
//
//	  uint16_t  start, size, data, offreg;
//	  uint16_t  pdata;
//	  int offset;
//
//	  //start = patch[0];   // start MCU_ADDRESS
//	  start = patch_addr_0400[0];
//	  //size = patch[1];    // data size
//	  size = patch_addr_0400[1];
//	  pdata = 2;     // data pointer
//	  offset = 0;         // MCU_DATA_0,7
//
//	  wrSensorReg8_16( 0xF0, 0x01);
//	  while (size > 0)
//	  {
//	    if (offset == 0)
//	    {
//	      //mi2010soc_reg_write(0x1C6, start);  // write MCU_ADDRESS
//	      myCAM.wrSensorReg8_16( 0xC6, start);
//	      //printk(KERN_INFO "REG=1, 0xC6, 0x%04X t// MCU_ADDRESSn", start);
//	    }
//	    //data = *pdata++;
//	    data = pgm_read_word(&patch_addr_0400[pdata++]);
//	    //offreg = 0x1C8 + offset;
//	    //mi2010soc_reg_write(offreg, data);  // write MCU_ADDRESS
//
//	    //addr = 0x2003;
//	    //myCAM.wrSensorReg8_16( 0xC6, addr);
//	    myCAM.wrSensorReg8_16( 0xC8 + offset, data);
//	    //printk(KERN_INFO "REG=1, 0x%02X, 0x%04X t// MCU_DATA_%dn", offreg&0xff, data, offset);
//	    start += 2;
//	    size--;
//	    offset++;
//	    if (offset == 8)
//	      offset = 0;
//	  }
//	  return;

}

long CameraSensorInit() {
	long lRetVal = -1;

	lRetVal = RegLstWrite((s_RegList*) init_cmds_list,
			sizeof(init_cmds_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

	RegLstWrite((s_RegList*) set_adaptive_clock_divisors,
				sizeof(set_adaptive_clock_divisors) / sizeof(s_RegList));

#ifndef ENABLE_JPEG
    //lRetVal = RegLstWrite((s_RegList *)preview_cmds_list,
    //                  sizeof(preview_cmds_list)/sizeof(s_RegList));
    //ASSERT_ON_ERROR(lRetVal);
    //lRetVal = RegLstWrite((s_RegList *)image_size_240_320_preview_cmds_list, \
    //                sizeof(image_size_240_320_preview_cmds_list)/ \
                    sizeof(s_RegList));
    //ASSERT_ON_ERROR(lRetVal);
    //lRetVal = RegLstWrite((s_RegList *)freq_setup_cmd_List,
    //                sizeof(freq_setup_cmd_List)/sizeof(s_RegList));
    //ASSERT_ON_ERROR(lRetVal);
    //lRetVal = RegLstWrite((s_RegList *)preview_on_cmd_list,
    // 	                sizeof(preview_on_cmd_list)/sizeof(s_RegList));
    //ASSERT_ON_ERROR(lRetVal);
#endif
	return 0;
}

//*****************************************************************************
//
//! This function checks the current jpeg size in bytes
//!
//! \param                      None
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************

uint32_t CheckJpegSize(){
	long lRetVal = -1;
	uint32_t len = 0;
	lRetVal = RegLstWriteLength((s_RegList *)check_jpeg_size_cmd_list,
	                    sizeof(check_jpeg_size_cmd_list)/sizeof(s_RegList),&len);
	return len;
}

uint16_t checkJpegStatus(){

	long lRetVal = -1;
	uint32_t len = 0;
	lRetVal = RegLstWriteLength((s_RegList *)get_jpeg_status,
	                    sizeof(get_jpeg_status)/sizeof(s_RegList),&len);
	return len;
}

uint32_t checkResolutionWidth(){

	long lRetVal = -1;
		uint32_t len = 0;
		lRetVal = RegLstWriteLength((s_RegList *)check_jpeg_width_cmd_list,
		                    sizeof(check_jpeg_width_cmd_list)/sizeof(s_RegList),&len);
		return len;
}
uint32_t checkResolutionHeight(){

	long lRetVal = -1;
		uint32_t len = 0;
		lRetVal = RegLstWriteLength((s_RegList *)check_jpeg_height_cmd_list,
		                    sizeof(check_jpeg_height_cmd_list)/sizeof(s_RegList),&len);
		return len;
}
//*****************************************************************************
//
//! This function initilizes the camera sensor
//!
//! \param                      None
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************

long CameraSoftReset() {
	long lRetVal = -1;

	lRetVal = RegLstWrite((s_RegList*) Ryans_resetCam_cmds_list,
			sizeof(Ryans_resetCam_cmds_list) / sizeof(s_RegList));
	// lRetVal = RegLstWrite((s_RegList *)Ryans_Massive_cmds_list, \
                                        sizeof(Ryans_Massive_cmds_list)/sizeof(s_RegList));

	//ASSERT_ON_ERROR(lRetVal);

	return 0;
}

//*****************************************************************************
//
//! This function configures the sensor in JPEG mode
//!
//! \param[in] width - X-Axis
//! \param[in] height - Y-Axis
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
long StartSensorInJpegMode(int width, int height) {
#ifdef ENABLE_JPEG
	long lRetVal = -1;

	lRetVal = RegLstWrite((s_RegList*) capture_cmds_list,
			sizeof(capture_cmds_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

	resolution_cmds_list[INDEX_SIZE_WIDTH].usValue = width;
	resolution_cmds_list[INDEX_SIZE_HEIGHT].usValue = height;
	lRetVal = RegLstWrite((s_RegList*) resolution_cmds_list,
			sizeof(resolution_cmds_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

//	lRetVal = RegLstWrite((s_RegList*) spoofSize,
//				sizeof(spoofSize) / sizeof(s_RegList));

	//lRetVal = RegLstWrite((s_RegList*) start_jpeg_capture_cmd_list,
	//		sizeof(start_jpeg_capture_cmd_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);
#endif
	return 0;
}

//*****************************************************************************
//
//! This function configures the sensor ouput resolution
//!
//! \param[in] width - X-Axis
//! \param[in] height - Y-Axis
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
long CameraSensorResolution(int width, int height) {
#ifdef ENABLE_JPEG
	long lRetVal = -1;

	lRetVal = RegLstWrite((s_RegList*) stop_jpeg_capture_cmd_list,
			sizeof(stop_jpeg_capture_cmd_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

	resolution_cmds_list[INDEX_SIZE_WIDTH].usValue = width;
	resolution_cmds_list[INDEX_SIZE_HEIGHT].usValue = height;
	lRetVal = RegLstWrite((s_RegList*) resolution_cmds_list,
			sizeof(resolution_cmds_list) / sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

	//lRetVal = RegLstWrite((s_RegList *)start_jpeg_capture_cmd_list,
	//                    sizeof(start_jpeg_capture_cmd_list)/sizeof(s_RegList));
	//ASSERT_ON_ERROR(lRetVal);

#else
    if(width != 240 || height != 256)
        return -1;
#endif
	return 0;
}

void doHandshake(){

	RegLstWrite((s_RegList*) handshake_cmd,
			sizeof(handshake_cmd) / sizeof(s_RegList));
}
void DoCapture() {
	long lRetVal = RegLstWrite((s_RegList*) start_jpeg_capture_cmd_list,
			sizeof(start_jpeg_capture_cmd_list) / sizeof(s_RegList));

}

void MT9D111Delay(unsigned long ucDelay);

//*****************************************************************************
//
//! This function implements delay in the camera sensor
//!
//! \param                      delay value
//!
//! \return                     None
//
//*****************************************************************************
#if defined(ewarm)
	void MT9D111Delay(unsigned long ucDelay)
	{
    __asm("    subs    r0, #1\n"
          "    bne.n   MT9D111Delay\n"
          "    bx      lr");
	}
#endif
#if defined(ccs)

    __asm("    .sect \".text:MT9D111Delay\"\n"
          "    .clink\n"
          "    .thumbfunc MT9D111Delay\n"
          "    .thumb\n"
          "    .global MT9D111Delay\n"
          "MT9D111Delay:\n"
          "    subs r0, #1\n"
          "    bne.n MT9D111Delay\n"
          "    bx lr\n");

#endif

//*****************************************************************************
//
//! This function implements the Register Write in MT9D111 sensor
//!
//! \param1                     Register List
//! \param2                     No. Of Items
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
static long RegLstWrite(s_RegList *pRegLst, unsigned long ulNofItems) {
	unsigned long ulNdx;
	unsigned short usTemp;
	uint32_t usTemp2;
	unsigned short usTemp3;
	unsigned char i;
	unsigned char ucBuffer[20], tempBuffer[20];
	unsigned long ulSize;
	long lRetVal = -1;

	if (pRegLst == NULL) {
		return RET_ERROR;
	}

	for (ulNdx = 0; ulNdx < ulNofItems; ulNdx++) {
		if (pRegLst->ucPageAddr == 100) {
			//PageAddr == 100, insret a delay equal to reg value
			HAL_Delay(pRegLst->usValue);
		} else if (pRegLst->ucPageAddr == 111) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);
				//ASSERT_ON_ERROR(lRetVal);
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
						2, 1)) {
					return RET_ERROR;
				}

				usTemp = ucBuffer[0] << 8;
				usTemp |= ucBuffer[1];
			} while (usTemp != pRegLst->usValue);
		} else if (pRegLst->ucPageAddr == 222) {
					// PageAddr == 111, wait for specified register value
					do {
						ucBuffer[0] = pRegLst->ucRegAddr;
						lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
								ucBuffer, 1, 1);
						//ASSERT_ON_ERROR(lRetVal);
						if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
								2, 1)) {
							return RET_ERROR;
						}
						usTemp2 = 0;

						usTemp2 = ucBuffer[0] << 8;
						usTemp2 |= ucBuffer[1];


					} while (usTemp2 == 0x00);
					int b = 0;
		}else if (pRegLst->ucPageAddr == 221) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = 0xF0;
			    ucBuffer[1] = 0x02;
			    ucBuffer[2] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 3, 1);
				//ASSERT_ON_ERROR(lRetVal);

				ucBuffer[0] = ucBuffer[2];
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);

				ucBuffer[0] = 0;
				ucBuffer[1] = 0;
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,

						2, 1)) {
					return RET_ERROR;
				}
				usTemp3 = 0;

				usTemp3 = ucBuffer[0] << 8;
				usTemp3 |= ucBuffer[1];
			}
			while (usTemp3 == 0x00);

			int d = 2+7;
			int e = d*2;
		}

		else if (pRegLst->ucPageAddr == 220) {

			//read in Register Value (expects C6 write with correct page in previous instruction)
			ucBuffer[0] = pRegLst->ucRegAddr;
			lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
					ucBuffer, 1, 1);
			tempBuffer[0] = 0x00;
			tempBuffer[1] = 0x00;
			lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR,
					tempBuffer, 2, 1);
			lRetVal = 0;
			lRetVal = tempBuffer[0];
			lRetVal << 8;
			lRetVal |= tempBuffer[1];
		} 		else {
			// Set the page
			ucBuffer[0] = SENSOR_PAGE_REG;
			ucBuffer[1] = 0x00;
			ucBuffer[2] = (unsigned char) (pRegLst->ucPageAddr);
			if (0
					!= HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, 3, 1)) {
				return RET_ERROR;
			}

			ucBuffer[0] = SENSOR_PAGE_REG;
			lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
					ucBuffer, 1, 1);
			//ASSERT_ON_ERROR(lRetVal);
			lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR,
					ucBuffer, 2, 1);
			//ASSERT_ON_ERROR(lRetVal);

			ucBuffer[0] = pRegLst->ucRegAddr;

			if (pRegLst->ucPageAddr == 0x1 && pRegLst->ucRegAddr == 0xC8) {
				usTemp = 0xC8;
				i = 1;
				while (pRegLst->ucRegAddr == usTemp) {
					ucBuffer[i] = (unsigned char) (pRegLst->usValue >> 8);
					ucBuffer[i + 1] = (unsigned char) (pRegLst->usValue & 0xFF);
					i += 2;
					usTemp++;
					pRegLst++;
					ulNdx++;
				}

				ulSize = (i - 2) * 2 + 1;
				ulNdx--;
				pRegLst--;
			} else {
				ulSize = 3;
				ucBuffer[1] = (unsigned char) (pRegLst->usValue >> 8);
				ucBuffer[2] = (unsigned char) (pRegLst->usValue & 0xFF);
			}

			if (0
					!= HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, ulSize, 1)) {
				return RET_ERROR;
			}
		}

		pRegLst++;
		HAL_Delay(10);
	}

	return RET_OK;
}

static long RegLstWriteLength(s_RegList *pRegLst, unsigned long ulNofItems, uint32_t* length ) {
	unsigned long ulNdx;
	unsigned short usTemp;
	uint32_t usTemp2;
	unsigned short usTemp3;
	unsigned char i;
	unsigned char ucBuffer[20], tempBuffer[20];
	unsigned long ulSize;
	long lRetVal = -1;
	uint8_t retries = 0;

	if (pRegLst == NULL) {
		return RET_ERROR;
	}

	for (ulNdx = 0; ulNdx < ulNofItems; ulNdx++) {
		if (pRegLst->ucPageAddr == 100) {
			//PageAddr == 100, insret a delay equal to reg value
			HAL_Delay(pRegLst->usValue);
		} else if (pRegLst->ucPageAddr == 111) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);
				//ASSERT_ON_ERROR(lRetVal);
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
						2, 1)) {
					return RET_ERROR;
				}

				usTemp = ucBuffer[0] << 8;
				usTemp |= ucBuffer[1];
			} while (usTemp != pRegLst->usValue);
		} else if (pRegLst->ucPageAddr == 222) {
					// PageAddr == 111, wait for specified register value
					do {
						ucBuffer[0] = pRegLst->ucRegAddr;
						lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
								ucBuffer, 1, 1);
						//ASSERT_ON_ERROR(lRetVal);
						if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
								2, 1)) {
							return RET_ERROR;
						}
						usTemp2 = 0;

						usTemp2 = ucBuffer[0] << 8;
						usTemp2 |= ucBuffer[1];
						*length = usTemp2;

					} while (usTemp2 == 0x00);
					int b = 0;
		}else if (pRegLst->ucPageAddr == 224) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);
				//ASSERT_ON_ERROR(lRetVal);
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
						2, 1)) {
					return RET_ERROR;
				}


				usTemp2 |= ucBuffer[1] << 16;
				//usTemp2 |= ucBuffer[1];
				*length = usTemp2;

				retries++;
			} while (usTemp2 == 0x00 && retries<20);
			int b = 0;
		} else if (pRegLst->ucPageAddr == 150) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);
				//ASSERT_ON_ERROR(lRetVal);
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,
						2, 1)) {
					return RET_ERROR;
				}
				usTemp2 = 0;

				usTemp2 = ucBuffer[0] << 8;
				usTemp2 |= ucBuffer[1];


			} while (usTemp2 == 0x00);
			int b = 0;
			}
		else if (pRegLst->ucPageAddr == 221) {
			// PageAddr == 111, wait for specified register value
			do {
				ucBuffer[0] = 0xF0;
			    ucBuffer[1] = 0x02;
			    ucBuffer[2] = pRegLst->ucRegAddr;
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 3, 1);
				//ASSERT_ON_ERROR(lRetVal);

				ucBuffer[0] = ucBuffer[2];
				lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
						ucBuffer, 1, 1);

				ucBuffer[0] = 0;
				ucBuffer[1] = 0;
				if (HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, ucBuffer,

						2, 1)) {
					return RET_ERROR;
				}
				usTemp3 = 0;

				usTemp3 = ucBuffer[0] << 8;
				usTemp3 |= ucBuffer[1];
				retries++;
			}
			while (usTemp3 == 0x00  && retries<20);

			int d = 2+7;
			int e = d*2;
		}

		else if (pRegLst->ucPageAddr == 220) {

			//read in Register Value (expects C6 write with correct page in previous instruction)
			ucBuffer[0] = pRegLst->ucRegAddr;
			lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
					ucBuffer, 1, 1);
			tempBuffer[0] = 0x00;
			tempBuffer[1] = 0x00;
			lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR,
					tempBuffer, 2, 1);
			lRetVal = 0;
			lRetVal = tempBuffer[0];
			lRetVal << 8;
			lRetVal |= tempBuffer[1];
		} 		else {
			// Set the page
			ucBuffer[0] = SENSOR_PAGE_REG;
			ucBuffer[1] = 0x00;
			ucBuffer[2] = (unsigned char) (pRegLst->ucPageAddr);
			if (0
					!= HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, 3, 1)) {
				return RET_ERROR;
			}

			ucBuffer[0] = SENSOR_PAGE_REG;
			lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
					ucBuffer, 1, 1);
			//ASSERT_ON_ERROR(lRetVal);
			lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR,
					ucBuffer, 2, 1);
			//ASSERT_ON_ERROR(lRetVal);

			ucBuffer[0] = pRegLst->ucRegAddr;

			if (pRegLst->ucPageAddr == 0x1 && pRegLst->ucRegAddr == 0xC8) {
				usTemp = 0xC8;
				i = 1;
				while (pRegLst->ucRegAddr == usTemp) {
					ucBuffer[i] = (unsigned char) (pRegLst->usValue >> 8);
					ucBuffer[i + 1] = (unsigned char) (pRegLst->usValue & 0xFF);
					i += 2;
					usTemp++;
					pRegLst++;
					ulNdx++;
				}

				ulSize = (i - 2) * 2 + 1;
				ulNdx--;
				pRegLst--;
			} else {
				ulSize = 3;
				ucBuffer[1] = (unsigned char) (pRegLst->usValue >> 8);
				ucBuffer[2] = (unsigned char) (pRegLst->usValue & 0xFF);
			}

			if (0
					!= HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, ulSize, 1)) {
				return RET_ERROR;
			}
		}

		pRegLst++;
		HAL_Delay(10);
	}

	return RET_OK;
}

/*static long I2CPage(unsigned char PageNumber){
	long lRetVal = -1;
	lRetVal = HAL_I2C_Master_Transmit(&hi2c2, SENSOR_PAGE_REG,
							PageNumber, 1, 1);
	return(lRetVal);
}*/

long I2CSelectVar(unsigned char PageNumber, unsigned short VarInt){
	long lRetVal = -1;
	unsigned char ucBuffer[5];
	ucBuffer[0] = SENSOR_PAGE_REG;
	ucBuffer[1] = PageNumber;
	ucBuffer[2] = CAM_I2C_VAR_ADDR;
	ucBuffer[3] = (VarInt >> 8) & 0xFF;
	ucBuffer[4] = VarInt & 0xFF;

	lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, 5, 1);
	return(lRetVal);
}

long I2CWriteVar(unsigned char PageNumber, unsigned short VarInt,unsigned short VarValue){
	long lRetVal = -1;
	unsigned char ucBuffer[7];
	ucBuffer[0] = SENSOR_PAGE_REG;
	ucBuffer[1] = PageNumber;
	ucBuffer[2] = CAM_I2C_VAR_ADDR;
	ucBuffer[3] = (VarInt >> 8) & 0xFF;
	ucBuffer[4] = VarInt & 0xFF;
	ucBuffer[5] = VarValue & 0xFF;
	ucBuffer[6] = (VarValue >>8) & 0xFF;


	lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
							ucBuffer, 7, 1);
	return(lRetVal);
}

long I2CReadVar(unsigned char PageNumber, unsigned short VarInt,unsigned short *VarValue){
	long lRetVal = -1;
	unsigned char ucBuffer1[6] ={0};
	unsigned char ucBuffer2[3] ={0};
	unsigned char readBuffer[5] = {0};
	ucBuffer1[0] = SENSOR_PAGE_REG;
	ucBuffer1[1] = PageNumber;
	ucBuffer1[2] = CAM_I2C_VAR_ADDR;
	ucBuffer1[3] = (VarInt >> 8) & 0xFF;
	ucBuffer1[4] = VarInt & 0xFF;
	ucBuffer1[5] = CAM_I2C_VAR_DATA;

	lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
			&ucBuffer1, 5, 1);

	lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
			&ucBuffer1[5], 1, 1);


	//ucBuffer2[0] = SENSOR_PAGE_REG;
	//ucBuffer2[1] = PageNumber;
	//ucBuffer2[2] = CAM_I2C_VAR_DATA;


	//lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR,
	//		&ucBuffer2, 3, 1);

	lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR,
			&readBuffer, 2, 1);
	*VarValue = readBuffer[0];
	return(lRetVal);
}


/*
 static long RegLstWriteUntilRead(s_RegList *pRegLst, unsigned long ulNofItems, unsigned short *buffReturn)
 {
 unsigned long       ulNdx;
 unsigned short      usTemp;
 unsigned char       i;
 unsigned char       ucBuffer[20], tempBuffer[20];
 unsigned long       ulSize;
 long lRetVal = -1;

 if(pRegLst == NULL)
 {
 return RET_ERROR;
 }

 for(ulNdx = 0; ulNdx < ulNofItems; ulNdx++)
 {
 if(pRegLst->ucPageAddr == 100)
 {
 //PageAddr == 100, insret a delay equal to reg value
 vTaskDelay(pRegLst->usValue);
 }
 else if(pRegLst->ucPageAddr == 111)
 {
 // PageAddr == 111, wait for specified register value
 do
 {
 ucBuffer[0] = pRegLst->ucRegAddr;
 lRetVal = HAL_I2C_Master_Transmit(&hi2c2,CAM_I2C_WRITE_ADDR,ucBuffer,1,1);
 //ASSERT_ON_ERROR(lRetVal);
 if(HAL_I2C_Master_Receive(&hi2c2,CAM_I2C_READ_ADDR,ucBuffer,2,1))
 {
 return RET_ERROR;
 }

 usTemp = ucBuffer[0] << 8;
 usTemp |= ucBuffer[1];
 }while(usTemp != pRegLst->usValue);
 }
 else if(pRegLst->ucPageAddr == 220){
 //read in Register Value (expects C6 write with correct page in previous instruction)
 ucBuffer[0] = pRegLst->ucRegAddr;
 lRetVal = HAL_I2C_Master_Transmit(&hi2c2, CAM_I2C_WRITE_ADDR, ucBuffer, 1, 1);
 tempBuffer[0] = 0x00;
 tempBuffer[1] = 0x00;
 lRetVal = HAL_I2C_Master_Receive(&hi2c2, CAM_I2C_READ_ADDR, tempBuffer, 2, 1);
 buffReturn = 0;
 buffReturn = tempBuffer[0];
 buffReturn << 8;
 buffReturn |= tempBuffer[1];
 }

 else
 {
 // Set the page
 ucBuffer[0] = SENSOR_PAGE_REG;
 ucBuffer[1] = 0x00;
 ucBuffer[2] = (unsigned char)(pRegLst->ucPageAddr);
 if(0 != HAL_I2C_Master_Transmit(&hi2c2,CAM_I2C_WRITE_ADDR,ucBuffer,3,1))
 {
 return RET_ERROR;
 }

 ucBuffer[0] = SENSOR_PAGE_REG;
 lRetVal = HAL_I2C_Master_Transmit(&hi2c2,CAM_I2C_WRITE_ADDR,ucBuffer,1,1);
 //ASSERT_ON_ERROR(lRetVal);
 lRetVal = HAL_I2C_Master_Receive(&hi2c2,CAM_I2C_READ_ADDR,ucBuffer,2,1);
 //ASSERT_ON_ERROR(lRetVal);

 ucBuffer[0] = pRegLst->ucRegAddr;

 if(pRegLst->ucPageAddr  == 0x1 && pRegLst->ucRegAddr == 0xC8)
 {
 usTemp = 0xC8;
 i=1;
 while(pRegLst->ucRegAddr == usTemp)
 {
 ucBuffer[i] = (unsigned char)(pRegLst->usValue >> 8);
 ucBuffer[i+1] = (unsigned char)(pRegLst->usValue & 0xFF);
 i += 2;
 usTemp++;
 pRegLst++;
 ulNdx++;
 }

 ulSize = (i-2)*2 + 1;
 ulNdx--;
 pRegLst--;
 }
 else
 {
 ulSize = 3;
 ucBuffer[1] = (unsigned char)(pRegLst->usValue >> 8);
 ucBuffer[2] = (unsigned char)(pRegLst->usValue & 0xFF);
 }

 if(0 != HAL_I2C_Master_Transmit(&hi2c2,CAM_I2C_WRITE_ADDR,ucBuffer,
 ulSize,1))
 {
 return RET_ERROR;
 }
 }

 pRegLst++;
 vTaskDelay(10);
 }

 return RET_OK;
 }



 void camReset(){
 unsigned short usTemp;
 usTemp = RegLstWrite((s_RegList *)stop_jpeg_capture_cmd_list,
 sizeof(stop_jpeg_capture_cmd_list)/sizeof(s_RegList),&usTemp);

 }
 */
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
