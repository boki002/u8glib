/*

  u8g_dev_ssd1325_nhd_2712864ucy3.c
  
  Driver for SSD1325 Controller (OLED Display)
  Tested with NHD-2.7-12864UCY3

  Universal 8bit Graphics Library
  
  Copyright (c) 2011, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
  SSD130x       Monochrom OLED Controller
  SSD131x       Character OLED Controller
  SSD132x       Graylevel OLED Controller
  SSD1331       Color OLED Controller       

*/

#include "u8g.h"

#define WIDTH 128
#define HEIGHT 64
#define PAGE_HEIGHT 8

/* http://www.newhavendisplay.com/app_notes/OLED_2_7_12864.txt */
u8g_pgm_uint8_t u8g_dev_ssd1325_nhd_27_12864ucy3_init_seq[] = {
  U8G_ESC_DLY(10),              /* delay 10 ms */
  U8G_ESC_CS(0),                 /* disable chip */
  U8G_ESC_ADR(0),               /* instruction mode */
  U8G_ESC_RST(1),               /* do reset low pulse with (1*16)+2 milliseconds */
  U8G_ESC_CS(1),                /* enable chip */
  0x0ae,                                /* display off, sleep mode */
  0x0b3, 0x091,                    /* set display clock divide ratio/oscillator frequency (set clock as 135 frames/sec) */
  0x0a8, 0x03f,                     /* multiplex ratio: 0x03f * 1/64 duty */
  0x0a2, 0x04c,                     /* display offset, shift mapping ram counter */
  0x0a1, 0x000,                     /* display start line */
  0x0ad, 0x002,                     /* master configuration: disable embedded DC-DC, enable internal VCOMH */
  0x0a0, 0x056,                     /* remap configuration, vertical address increment, enable nibble remap (upper nibble is left) */
  0x086,                                /* full current range (0x084, 0x085, 0x086) */
  0x0b8,                                /* set gray scale table */
      0x01, 0x011, 0x022, 0x032, 0x043, 0x054, 0x065, 0x076,
  0x081, 0x040,                    /* contrast, brightness, 0..128 */
  0x0b2, 0x051,                    /* frame frequency (row period) */
  0x0b1, 0x055,                    /* phase length */
  0x0bc, 0x010,                    /* pre-charge voltage level */
  0x0b4, 0x002,                    /* set pre-charge compensation level (not documented in the SDD1325 datasheet, but used in the NHD init seq.) */
  0x0b0, 0x028,                    /* enable pre-charge compensation (not documented in the SDD1325 datasheet, but used in the NHD init seq.) */
  0x0be, 0x01c,                     /* VCOMH voltage */
  0x0bf, 0x002|0x00d,           /* VSL voltage level (not documented in the SDD1325 datasheet, but used in the NHD init seq.) */
  0x0a5,                                 /* all pixel on */
  0x0af,                                  /* display on */
  U8G_ESC_DLY(100),             /* delay 100 ms */
  U8G_ESC_DLY(100),             /* delay 100 ms */
  0x0a4,                                 /* normal display mode */
  U8G_ESC_CS(0),             /* disable chip */
  
  U8G_ESC_DLY(100),             /* delay 100 ms */
  U8G_ESC_CS(1),             /* enable chip */
  0x015, 0x000, 0x004,          /* set column address */
  0x075, 0x000, 0x004,          /* set row address */
  U8G_ESC_ADR(1),               /* data mode */
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  0x0ff,
  U8G_ESC_ADR(0),               /* instruction mode */
  U8G_ESC_CS(0),             /* disable chip */
  
  U8G_ESC_END                /* end of sequence */
};

static void u8g_ssd1325_prepare_page(u8g_t *u8g, u8g_dev_t *dev, uint8_t page)
{
  u8g_SetAddress(u8g, dev, 0);          /* cmd mode */
  u8g_SetChipSelect(u8g, dev, 1);          /* activate device */
  
  u8g_WriteByte(u8g, dev, 0x015);       /* column address... */
  u8g_WriteByte(u8g, dev, 0x000);       /* start at column 0 */
  u8g_WriteByte(u8g, dev, 0x03f);       /* end at column 63 (which is y == 127), because there are two pixel in one column */
  
  u8g_WriteByte(u8g, dev, 0x075);       /* row address... */
  page <<= 3;
  u8g_WriteByte(u8g, dev, page);       /* start at the selected page */
  page += 7;
  u8g_WriteByte(u8g, dev, page);       /* end within the selected page */  
  
  u8g_SetAddress(u8g, dev, 1);          /* data mode */
}

/* assumes row autoincrement */
static  void u8g_ssd1325_write_16_pixel(u8g_t *u8g, u8g_dev_t *dev, uint8_t left, uint8_t right)
{
  uint8_t d, cnt;
  cnt = 8;
  do
  {
    d = 0;
    if ( left & 1 )
      d |= 0x0f0;
    if ( right & 1 )
      d |= 0x00f;
    u8g_WriteByte(u8g, dev, d);
    left >>= 1;
    right >>= 1;
    cnt--;
  }while ( cnt > 0 );
}


uint8_t u8g_dev_ssd1325_nhd_27_12864ucy3_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  switch(msg)
  {
    case U8G_DEV_MSG_INIT:
      u8g_InitCom(u8g, dev);
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1325_nhd_27_12864ucy3_init_seq);
      u8g_ssd1325_prepare_page(u8g, dev, 2);
      //u8g_ssd1325_write_16_pixel(u8g, dev, 0, 0);
      u8g_ssd1325_write_16_pixel(u8g, dev, 0x081, 0xff);
      u8g_SetChipSelect(u8g, dev, 0);          /* dectivate device */
      break;
    case U8G_DEV_MSG_STOP:
      break;
    case U8G_DEV_MSG_PAGE_NEXT:
      {
        u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
#ifdef XYZ        
        u8g_SetChipSelect(u8g, dev, 1);
        u8g_WriteByte(u8g, dev, 0x0b0 | pb->p.page); /* select current page (ST7565R) */
        u8g_SetAddress(u8g, dev, 1);           /* data mode */
        if ( u8g_pb_WriteBuffer(pb, u8g, dev) == 0 )
          return 0;
        u8g_SetChipSelect(u8g, dev, 0);
#endif
      }
      break;
  }
  return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
}

U8G_PB_DEV(u8g_dev_ssd1325_nhd_27_12864ucy3_sw_spi , WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1325_nhd_27_12864ucy3_fn, u8g_com_arduino_sw_spi_fn);
U8G_PB_DEV(u8g_dev_ssd1325_nhd_27_12864ucy3_hw_spi , WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1325_nhd_27_12864ucy3_fn, u8g_com_arduino_hw_spi_fn);

