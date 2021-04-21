/*
 * (C) Jack Hay, Apr 2021
 *
 */

#include "display.h"
#include "../uart/debug.h"
#include "../kstdlib/kstdlib.h"
#include "font.h"

#define MMIO_BASE       0x3F000000
#define MBOX_REQUEST    0

//channels
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_SETPOWER       0x28001
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

volatile unsigned int  __attribute__((aligned(16))) MBOX[36];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

//display properties
uint32_t WIDTH;
uint32_t HEIGHT;
uint32_t PITCH;
uint32_t ISRGB;

//framebuffer address
uint8_t *FB_ADDR;

typedef struct pixel_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pixel_t;

/**
 * Make an mbox request
 * SOURCE: https://github.com/bztsrc/raspi3-tutorial/blob/master/09_framebuffer/mbox.c
 */
uint32_t mbox_call(uint8_t channel) {
  unsigned int r = (((unsigned int)((unsigned long)&MBOX)&~0xF) | (channel&0xF));
  /* wait until we can write to the mailbox */
  do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
  /* write the address of our message to the mailbox with channel identifier */
  *MBOX_WRITE = r;
  /* now wait for the response */
  while(1) {
      /* is there a response? */
      do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
      /* is it a response to our message? */
      if(r == *MBOX_READ)
          /* is it a valid successful response? */
          return MBOX[1]==MBOX_RESPONSE;
  }
  return 0;
}

/**
 * Initialize the display
 * SOURCE: https://github.com/bztsrc/raspi3-tutorial/blob/master/09_framebuffer/lfb.c
 */
void init_display() {
  MBOX[0] = 35*4;
  MBOX[1] = MBOX_REQUEST;

  MBOX[2] = 0x48003;  //set phy wh
  MBOX[3] = 8;
  MBOX[4] = 8;
  MBOX[5] = 1024;         //FrameBufferInfo.width
  MBOX[6] = 768;          //FrameBufferInfo.height

  MBOX[7] = 0x48004;  //set virt wh
  MBOX[8] = 8;
  MBOX[9] = 8;
  MBOX[10] = DISPLAY_WIDTH;        //FrameBufferInfo.virtual_width
  MBOX[11] = DISPLAY_HEIGHT;         //FrameBufferInfo.virtual_height

  MBOX[12] = 0x48009; //set virt offset
  MBOX[13] = 8;
  MBOX[14] = 8;
  MBOX[15] = 0;           //FrameBufferInfo.x_offset
  MBOX[16] = 0;           //FrameBufferInfo.y.offset

  MBOX[17] = 0x48005; //set depth
  MBOX[18] = 4;
  MBOX[19] = 4;
  MBOX[20] = 32;          //FrameBufferInfo.depth

  MBOX[21] = 0x48006; //set pixel order
  MBOX[22] = 4;
  MBOX[23] = 4;
  MBOX[24] = 1;           //RGB, not BGR preferably

  MBOX[25] = 0x40001; //get framebuffer, gets alignment on request
  MBOX[26] = 8;
  MBOX[27] = 8;
  MBOX[28] = 4096;        //FrameBufferInfo.pointer
  MBOX[29] = 0;           //FrameBufferInfo.size

  MBOX[30] = 0x40008; //get pitch
  MBOX[31] = 4;
  MBOX[32] = 4;
  MBOX[33] = 0;           //FrameBufferInfo.pitch

  MBOX[34] = MBOX_TAG_LAST;

  //this might not return exactly what we asked for, could be
  //the closest supported resolution instead
  if (mbox_call(MBOX_CH_PROP) && MBOX[20]==32 && MBOX[28]!=0) {
    MBOX[28]&=0x3FFFFFFF;   //convert GPU address to ARM address
    WIDTH=MBOX[5];          //get actual physical width
    HEIGHT=MBOX[6];         //get actual physical height
    PITCH=MBOX[33];         //get number of bytes per line
    ISRGB=MBOX[24];         //get the actual channel order
    FB_ADDR=(void*)((unsigned long)MBOX[28]);
    debug_log("set screen resolution");
    debug_val("width",WIDTH);
    debug_val("height",HEIGHT);
    debug_val("pitch",PITCH);
  } else {
    debug_err("unable to set screen resolution");
  }
}

/**
 * Draw a pixel to the fb
 */
void draw_pixel(uint32_t x, uint32_t y, uint8_t attr) {
  int offs = (y * PITCH) + (x * 4);
  *((unsigned int*)(FB_ADDR + offs)) = VGA_PAL[attr & 0x0f];
}

/**
 * Draw a character
 * Adapted from here: https://github.com/isometimes/rpi4-osdev/tree/master/part5-framebuffer
 */
void draw_char(unsigned char ch, int x, int y, unsigned char attr) {
  unsigned char *glyph = (unsigned char *)&FONT + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

  for (int i=0;i<FONT_HEIGHT;i++) {
    for (int j=0;j<FONT_WIDTH;j++) {
      unsigned char mask = 1 << j;
      unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

      draw_pixel(x+j, y+i, col);
    }
    glyph += FONT_BPL;
  }
}

/**
 * Clear the screen
 */
void clear_screen() {

}

/**
 * Write a message to the console
 * @param x position x
 * @param y position y
 * @param msg the message to write
 */
void draw_str(int x, int y, char *msg) {
  while ((msg != NULL) && *msg) {
    if (*msg == '\n') {
      y += FONT_HEIGHT + 2;
      x = 0;
    } else {
      draw_char(*msg,x,y,0x0f);
      x += FONT_WIDTH;
    }
    msg++;
  }
}
