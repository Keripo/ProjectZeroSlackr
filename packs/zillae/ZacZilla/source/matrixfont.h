/*
 * Copyright (C) 2004 Alastair S
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MATRIXFONT_H__
#define __MATRIXFONT_H__

#define COL_W 6
#define COL_H 10

#define MAXCHARS 63

static GR_BITMAP matrix_code_font[MAXCHARS+1][10] = {
	{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, // blank

	{0x0300, 0x3300, 0x4B00, 0x5F00, 0x5700, 
	 0x5700, 0x6700, 0x6B00, 0x3B00, 0x0300}, // 0
	{0x3300, 0x7300, 0x7300, 0x3300, 0x3300, 
 	 0x3300, 0x3300, 0x3300, 0x0000, 0x0000}, // 1
	{0x0000, 0x3000, 0x4800, 0x4000, 0x4000, 
 	 0x3000, 0x1800, 0x0800, 0x7800, 0x0000}, // 2
	{0x0000, 0x2000, 0x5800, 0x0800, 0x0800, 
 	 0x3800, 0x3000, 0x1800, 0x7800, 0x0000}, // 3
	{0x0000, 0x1000, 0x2000, 0x2000, 0x4000, 
 	 0x5000, 0x7000, 0x1000, 0x1000, 0x0000}, // 4
	{0x0000, 0x7800, 0x0800, 0x0800, 0x7800, 
 	 0x4000, 0x4000, 0x4000, 0x3800, 0x0000}, // 5
	{0x0000, 0x0800, 0x1000, 0x2000, 0x7000, 
 	 0x4800, 0x4400, 0x4800, 0x7800, 0x0000}, // 6
	{0x0300, 0x7F00, 0x0B00, 0x0B00, 0x1300, 
 	 0x2300, 0x2300, 0x2300, 0x2300, 0x0300}, // 7
	{0x0000, 0x1000, 0x6800, 0x4800, 0x3000, 
 	 0x7800, 0x4C00, 0x4C00, 0x7800, 0x0000}, // 8
	{0x0000, 0x3000, 0x6800, 0x4C00, 0x4C00, 
 	 0x7800, 0x2000, 0x3000, 0x1800, 0x0000}, // 9

 	{0x0000, 0x0000, 0x3000, 0x3000, 0x5800, 
 	 0x4800, 0x7800, 0x4800, 0x4C00, 0x0000}, // A (11)
	{0x0000, 0x0000, 0x7000, 0x4800, 0x4800, 
 	 0x7800, 0x4800, 0x4C00, 0x7800, 0x0000}, // B
	{0x0000, 0x3000, 0x4800, 0x4000, 0x4000, 
 	 0x4000, 0x4800, 0x3000, 0x0000, 0x0000}, // C
	{0x0000, 0x0000, 0x6000, 0x7000, 0x5800, 
 	 0x4800, 0x5800, 0x5000, 0x6000, 0x0000}, // D
	{0x0000, 0x7800, 0x4000, 0x4000, 0x7000, 
 	 0x4000, 0x4000, 0x7800, 0x0000, 0x0000}, // E
	{0x0000, 0x0000, 0x7000, 0x4000, 0x4000, 
 	 0x7000, 0x4000, 0x4000, 0x4000, 0x0000}, // F
	{0x0000, 0x3800, 0x4000, 0x4000, 0x5800, 
 	 0x4800, 0x4800, 0x3800, 0x0000, 0x0000}, // G
	{0x0000, 0x0000, 0x4800, 0x4800, 0x4800, 
 	 0x7800, 0x4800, 0x4800, 0x4800, 0x0000}, // H
 	{0x0300, 0x0300, 0x3B00, 0x1000, 0x1000,
 	 0x1200, 0x1300, 0x1300, 0x3B00, 0x0300}, // I
 	{0x0300, 0x0300, 0x1300, 0x1000, 0x1000, 
 	 0x1200, 0x1300, 0x5300, 0x7300, 0x0300}, // J
 	{0x0300, 0x4F00, 0x5300, 0x6000, 0x6000, 
 	 0x7200, 0x5B00, 0x4F00, 0x0300, 0x0300}, // K
 	{0x0300, 0x0300, 0x4300, 0x4000, 0x4000, 
 	 0x4200, 0x4300, 0x4300, 0x7B00, 0x0300}, // L
 	{0x0300, 0x4F00, 0x5700, 0x7400, 0x5400, 
 	 0x5600, 0x4700, 0x4700, 0x0300, 0x0300}, // M
 	{0x0300, 0x4F00, 0x5700, 0x7400, 0x5400, 
 	 0x5600, 0x4700, 0x4700, 0x0300, 0x0300}, // N (M for now)	
 	{0x0300, 0x0300, 0x3300, 0x4800, 0x4800, 
 	 0x4600, 0x4B00, 0x4B00, 0x3300, 0x0300}, // O
 	{0x0300, 0x0300, 0x7300, 0x4800, 0x4800, 
 	 0x7A00, 0x4300, 0x4300, 0x4300, 0x0300}, // P
 	{0x0300, 0x3300, 0x4B00, 0x4800, 0x4800, 
 	 0x5A00, 0x5B00, 0x2F00, 0x0300, 0x0300}, // Q
 	{0x0300, 0x0300, 0x7300, 0x4800, 0x4800, 
 	 0x7200, 0x5300, 0x5300, 0x4B00, 0x0300}, // R
 	{0x0300, 0x0300, 0x7300, 0x4800, 0x4000, 
 	 0x3200, 0x0B00, 0x4B00, 0x7300, 0x0300}, // S
 	{0x0300, 0x0300, 0x7B00, 0x2000, 0x2000, 
 	 0x2200, 0x2300, 0x2300, 0x2300, 0x0300}, // T
 	{0x0300, 0x4B00, 0x4B00, 0x4800, 0x4800, 
 	 0x4A00, 0x4B00, 0x7B00, 0x0300, 0x0300}, // U
 	{0x0300, 0x0300, 0x4700, 0x4800, 0x4800, 
 	 0x2A00, 0x2B00, 0x3300, 0x3300, 0x0300}, // V
 	{0x0300, 0x0300, 0x4700, 0x4400, 0x4400, 
 	 0x5A00, 0x7B00, 0x6B00, 0x6B00, 0x0300}, // W
 	{0x0300, 0x0300, 0x4F00, 0x6800, 0x3000, 
 	 0x1200, 0x3300, 0x6B00, 0x4F00, 0x0300}, // X
 	{0x0300, 0x0300, 0x4F00, 0x6800, 0x3000, 
 	 0x3200, 0x1300, 0x1300, 0x1300, 0x0300}, // Y
 	{0x0300, 0x0300, 0x7300, 0x1000, 0x1000, 
 	 0x2200, 0x6300, 0x4300, 0x7B00, 0x0300}, // Z

   	{0x0300, 0x0300, 0x7300, 0x1000, 0x1000,  // (37)
   	 0x7200, 0x1300, 0x1300, 0x7300, 0x0300}, // random matrix code
   	{0x0300, 0x2B00, 0x2B00, 0x4800, 0x4800,  // from here on
   	 0x1200, 0x1300, 0x2300, 0x2300, 0x0300}, //   |
   	{0x0300, 0x4300, 0x6300, 0x4400, 0x0800,  //  \|/
   	 0x0A00, 0x1300, 0x3300, 0x4300, 0x0300}, //   '
   	{0x0300, 0x7F00, 0x0B00, 0x1800, 0x1000, 
   	 0x1200, 0x2300, 0x2300, 0x2300, 0x4300},
   	{0x0B00, 0x0B00, 0x1300, 0x3000, 0x3000, 
   	 0x5200, 0x5300, 0x1300, 0x1300, 0x1300},
   	{0x2300, 0x2300, 0x7B00, 0x4800, 0x4800, 
   	 0x4A00, 0x0B00, 0x1300, 0x1300, 0x2300},
   	{0x0300, 0x7B00, 0x1300, 0x1000, 0x1000, 
   	 0x1200, 0x1300, 0x7B00, 0x0300, 0x0300},
   	{0x0300, 0x1300, 0x7B00, 0x1000, 0x1000, 
   	 0x3200, 0x5300, 0x5300, 0x5300, 0x1300},
   	{0x0300, 0x2300, 0x7B00, 0x2800, 0x2800, 
   	 0x2A00, 0x2B00, 0x4B00, 0x5B00, 0x4300},
   	{0x2300, 0x1B00, 0x7B00, 0x1000, 0x1400, 
   	 0x7E00, 0x9300, 0x1300, 0x1300, 0x1300},
   	{0x2300, 0x2B00, 0x3B00, 0x4800, 0x4800, 
   	 0x4A00, 0x1300, 0x1300, 0x2300, 0x2300},
   	{0x0300, 0x2300, 0x3F00, 0x4800, 0x4800, 
   	 0x5200, 0x1300, 0x1300, 0x1300, 0x2300},
   	{0x0300, 0x0300, 0x7B00, 0x0800, 0x0800, 
   	 0x0A00, 0x0B00, 0x7B00, 0x0B00, 0x0300},
   	{0x0300, 0x2B00, 0x2B00, 0x7C00, 0x2800, 
   	 0x2A00, 0x0B00, 0x1300, 0x1300, 0x2300},
   	{0x0300, 0x4300, 0x2300, 0x0000, 0x4400, 
   	 0x4A00, 0x0B00, 0x1300, 0x3300, 0x6300},
   	{0x0300, 0x7B00, 0x0B00, 0x1000, 0x1000, 
   	 0x1200, 0x2B00, 0x6B00, 0x4300, 0x0300},
   	{0x0300, 0x2300, 0x2300, 0x2C00, 0x7400, 
   	 0x6600, 0x2B00, 0x2B00, 0x2300, 0x3B00},
   	{0x0300, 0x4B00, 0x4B00, 0x4800, 0x0800, 
   	 0x1200, 0x1300, 0x1300, 0x6300, 0x2300},
	{0x0000, 0x7800, 0x0800, 0x7800, 0x0800, 
 	 0x0800, 0x1000, 0x1000, 0x6000, 0x0000},
	{0x0000, 0x7000, 0x0800, 0x3000, 0x3000, 
 	 0x2000, 0x2000, 0x2000, 0x4000, 0x0000},
	{0x0000, 0x1800, 0x1000, 0x2000, 0x2000, 
 	 0x5000, 0x5000, 0x1000, 0x1000, 0x0000},
	{0x0000, 0x2000, 0x7800, 0x4800, 0x4800, 
 	 0x4800, 0x1000, 0x1000, 0x2000, 0x0000},
	{0x0000, 0x0000, 0x7000, 0x2000, 0x2000, 
 	 0x2000, 0x2000, 0x7800, 0x0000, 0x0000},
	{0x0300, 0x1300, 0x1300, 0x1300, 0x7B00, 
 	 0x3300, 0x3300, 0x5300, 0x5300, 0x0300},
	{0x0300, 0x2300, 0x2B00, 0x3B00, 0x6B00, 
 	 0x2B00, 0x2300, 0x1300, 0x1300, 0x0300},
	{0x0300, 0x0300, 0x0300, 0x7300, 0x1300, 
 	 0x1300, 0x1300, 0x1300, 0x7B00, 0x0300}, // (63)
};


#endif
