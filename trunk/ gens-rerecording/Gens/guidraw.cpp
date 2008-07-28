//A series of functions to be used for drawing things on the MD Screen
//TODO: triangles, circles, ellipses.

//PutText places text, centered, on point x,y but within the bounds of point xl,yl (low) and point xh,yh (high)

//All following functions take:
//color32: an RGB color code in the form of rrrrrrrrggggggggbbbbbbbb
//color16: an RGB color code in the form of rrrrrggggggbbbbb (xrrrrrgggggbbbbb if low bit of mode_555 is set)
//wrap: an integer from 0 to 8 which specifies how far off the screen (in multiples of the screensize) an object 
//		(or portion thereof) can be and still be drawn -- this is accomplished by halving the opacity, and drawing a 
//		corresponding distance from the opposing edge, each when coordinates are found to be outside the display surface
//		wrap defaults to 1 if not specified
//Opac: an integer from 0 to 255 which specifies the opacity of the object to be drawn: 255 means full opacity, and 0 
//		means it can barely be seen.
//		Opac defaults to 255 if not specified

//Pixel sets a pixel x,y
//DrawLine draws a line from point x1,y1 to point x2,y2, with no anti-aliasing

//All following functions additionally take:
//fill: an integer from 0 to 1 which specifies whether or not the object should be filled
//		fill defaults to 0 if unspecified.
//fillOpac: an integer from 0 to 255 which specifies opacity of the fill, using the same convention as "Opac" above
//		fillOpac defaults to 127 if unspecified.

//DrawBoxPP draws a rectangle defined by point x1,y1 (upper-left corner) and point x2,y2 (lower right corner)
//DrawBoxMWH draws a rectangle defined by point x,y (midpoint) width-radius w, and height-radius h
//DrawBoxCWH draws a rectangle defined by point x,y (upper-left corner), width w, and height h
//DrawEccoOct draws an octogon defined by point x,y (midpoint), and radius r, according to an algorithm used in Ecco 2

#include <windows.h>
#include "guidraw.h"
#include "drawutil.h"
#include "math.h"
#include "misc.h"

void PutText (char *string, short x, short y, short xl, short yl, short xh, short yh, int outstyle, int style)
{
	xl = max(xl,0);
	yl = max(xl,0);
	xh = min(xh + 319,318);
	yh = min(yh + 217,217);
	xh -= 4 * strlen(string);
	x = x - ((5 * (strlen(string) - 1))/2);
	x = min(max(x,max(xl,1)),min(xh,318 - 4 * (int)strlen(string)));
	y = min(max(y - 3,max(yl,1)),yh);
	const static int xOffset [] = {-1,-1,-1,0,1,1,1,0};
	const static int yOffset [] = {-1,0,1,1,1,0,-1,-1};
	for(int i = 0 ; i < 8 ; i++)
		Print_Text(string,strlen(string),x + xOffset[i],y + yOffset[i],outstyle);
	Print_Text(string,strlen(string),x,y,style);
}

void Pixel (short x, short y, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac)
{
//	unsigned char Opac;
	while (wrap && Opac)
	{
		bool off = false;
		if (x < 0) x += 320, off = true;
		if (x > 319) x -= 320, off = true;
		if (y < 0) y += 224, off = true;
		if (y > 223) y -= 224, off = true;
		if (off)
		{
			Opac >>=1;
			wrap--;
		}
		else wrap = 0;	// if it's not offscreen, end this loop early
	}
	x = max(0,min(319,x));
	y = max(0,min(223,y));
	x+=8;
	unsigned int off = y;
	off *= 336;
	off += x;

	color32 = DrawUtil::Blend(color32,MD_Screen32[off], (int)Opac+1);
	color16 = DrawUtil::Blend(color16,MD_Screen[off], (int)Opac+1);
	if (Bits32) 
		MD_Screen32[off] = color32;
	else
		MD_Screen[off] = color16;
}

void DrawLineBress(short x1,short y1,short x2,short y2,short dx,short dy,unsigned int color32,unsigned short color16,char wrap, unsigned char Opac)
{
	bool steep = abs(dy) > abs(dx);
	if (steep)
	{
		x1 ^= y1, y1 ^= x1, x1 ^= y1;
		x2 ^= y2, y2 ^= x2, x2 ^= y2;
		dx ^= dy, dy ^= dx, dx ^= dy;
	}
	if (x1 > x2)
	{
		x1 ^= x2, x2 ^= x1, x1 ^= x2;
		y1 ^= y2, y2 ^= y1, y1 ^= y2;
		dx = 0-dx, dy = 0-dy;
	}
	short sy = ((dy < 0)? -1 : 1);
	short thresh = dx;
	dy = abs(dy);
	short err = 0;
	for (short x = x1, y = y1; x <= x2; x++)
	{
		if (steep)	Pixel(y,x,color32,color16,wrap, Opac);
		else		Pixel(x,y,color32,color16,wrap, Opac);
		err += dy << 1;
		if (err >= thresh)
		{
			y += sy;
			err -= dx << 1;
		}
	}	
}

void DrawLine(short x1, short y1, short x2, short y2, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac)
{
	short dx = x2 - x1;
	short dy = y2 - y1;

	if (!dy)
	{
		if (x1 > x2) x1 ^= x2, x2 ^= x1, x1 ^= x2;
		for (short JXQ = x1; JXQ <= x2; JXQ++)
			Pixel(JXQ,y1,color32,color16,wrap, Opac);
	}
	else if (!dx)
	{
		if (y1 > y2) y1 ^= y2, y2 ^= y1, y1 ^= y2;
		for (short JXQ = y1; JXQ <= y2; JXQ++)
			Pixel(x1,JXQ,color32,color16,wrap, Opac);
	}
	else DrawLineBress(x1,y1,x2,y2,dx,dy,color32,color16,wrap, Opac);
}
void DrawBoxPP (short x1, short y1, short x2, short y2, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac, char fill, unsigned char fillOpac)
{
	if (x1 > x2) x1^=x2, x2^=x1,x1^=x2;
	if (y1 > y2) y1^=y2, y2^=y1,y1^=y2;
	for (short JXQ = x1; JXQ <= x2; JXQ++)
	{
		Pixel(JXQ,y1,color32,color16,wrap, Opac);
		if ((y2 - y1) > 0) Pixel(JXQ,y2,color32,color16,wrap, Opac);
	}
	for (short JXQ = y1 + 1; JXQ < y2; JXQ++)
	{
		Pixel(x1,JXQ,color32,color16,wrap, Opac);
		if ((x2 - x1) > 0) Pixel(x2,JXQ,color32,color16,wrap, Opac);
	}
	if (fill)
	{
		if ((x2 - x1) > 0) x1++, x2--;
		if ((y2 - y1) > 0) y1++, y2--;
		while (((x2 - x1) > 0) && ((y2 - y1) > 0))
		{
			DrawBoxPP(x1,y1,x2,y2,color32,color16,wrap,fillOpac);
			if ((x2 - x1) > 0) x1++, x2--;
			if ((y2 - y1) > 0) y1++, y2--;
		}
		if (!(x1 - x2) || !(y1 - y2))
			DrawLine(x1,y1,x2,y2,color32,color16,wrap,fillOpac);
	}
}
void DrawBoxCWH (short x, short y, short w, short h, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac, char fill, unsigned char fillOpac)
{
	for (short JXQ = 0; JXQ <= w; JXQ++)
	{
		Pixel(x + JXQ,y, color32, color16, wrap, Opac);
		if (h) Pixel(x + JXQ,y + h, color32, color16, wrap, Opac);
	}
	for (short JXQ = 1; JXQ < h; JXQ++)
	{
		Pixel(x,y + JXQ, color32, color16, wrap, Opac);
		if (w) Pixel(x + w,y + JXQ, color32, color16, wrap, Opac);
	}
	if (fill)
	{
		Opac = fillOpac;
		x++,y++,w-=2,h-=2;
		while ((w > 0) && (h > 0))
		{
			for (short JXQ = 0; JXQ <= w; JXQ++)
			{
				Pixel(x + JXQ,y, color32, color16, wrap, Opac);
				if (h) Pixel(x + JXQ,y + h, color32, color16, wrap, Opac);
			}
			for (short JXQ = 1; JXQ < h; JXQ++)
			{
				Pixel(x,y + JXQ, color32, color16, wrap, Opac);
				if (w) Pixel(x + w,y + JXQ, color32, color16, wrap, Opac);
			}
			x++,y++,w-=2,h-=2;
		}
//		if (w) DrawLine(x, y, x + w, y, color32, color16, wrap, fillOpac);
//		else   DrawLine(x, y, x, y + max(0,h-1), color32, color16, wrap, fillOpac);
	}
}
void DrawBoxMWH (short x, short y, short w, short h, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac, char fill, unsigned char fillOpac)
{
	for (short JXQ = 0; JXQ <= w; JXQ++)
	{
		Pixel(x - JXQ,y - h, color32, color16, wrap, Opac);
		if (h) Pixel(x - JXQ,y + h, color32, color16, wrap, Opac);
		if (JXQ) Pixel(x + JXQ,y - h, color32, color16, wrap, Opac);
		if (h && JXQ) Pixel(x + JXQ,y + h, color32, color16, wrap, Opac);
	}
	for (short JXQ = 0; JXQ < h; JXQ++)
	{
		Pixel(x - w,y - JXQ, color32, color16, wrap, Opac);
		if (JXQ) Pixel(x - w,y + JXQ, color32, color16, wrap, Opac);
		if (w) Pixel(x + w,y - JXQ, color32, color16, wrap, Opac);
		if (w && JXQ) Pixel(x + w,y + JXQ, color32, color16, wrap, Opac);
	}
	if (fill)
	{
		Opac = fillOpac;
		while ((--w > 0) && (--h > 0))
		{
			for (short JXQ = 0; JXQ <= w; JXQ++)
			{
				Pixel(x - JXQ,y - h, color32, color16, wrap, Opac);
				if (h) Pixel(x - JXQ,y + h, color32, color16, wrap, Opac);
				if (JXQ) Pixel(x + JXQ,y - h, color32, color16, wrap, Opac);
				if (h && JXQ) Pixel(x + JXQ,y + h, color32, color16, wrap, Opac);
			}
			for (short JXQ = 0; JXQ < h; JXQ++)
			{
				Pixel(x - w,y - JXQ, color32, color16, wrap, Opac);
				if (JXQ) Pixel(x - w,y + JXQ, color32, color16, wrap, Opac);
				if (w) Pixel(x + w,y - JXQ, color32, color16, wrap, Opac);
				if (w && JXQ) Pixel(x + w,y + JXQ, color32, color16, wrap, Opac);
			}
		}
		if (w) DrawLine(x - w, y, x + w, y, color32, color16, wrap, Opac);
		else if (--h > 0) DrawLine(x, y - h, x, y + h, color32, color16, wrap, Opac);
		else Pixel(x,y,color32,color16,wrap,Opac);
	}
}
void DrawEccoOct (short x, short y, short r, unsigned int color32, unsigned short color16, char wrap, unsigned char Opac, char fill, unsigned char fillOpac)
{
	if (r < 0)
		return;
	short off = r;
	for (short JXQ = 0; (JXQ <= off); JXQ++)
	{
		int d1 = JXQ << 16;
		int d2 = off << 16;
		if (d1 < d2) d1 ^= d2, d2 ^= d1, d1 ^= d2;
		d1 += d2 >> 1;
		d2 -= d2 >> 3;
		d1 >>= 16;
		if (d1 > r) off--;
		if (JXQ > off) continue;
		Pixel(x - off, y - JXQ, color32, color16, wrap, Opac);
		if (off != JXQ) Pixel(x - JXQ, y - off, color32, color16, wrap, Opac);
		if (off)
		{
			Pixel(x + off, y - JXQ, color32, color16, wrap, Opac);
			Pixel(x - JXQ, y + off, color32, color16, wrap, Opac);
		}
		if (JXQ && (off != JXQ))
		{
			Pixel(x - off, y + JXQ, color32, color16, wrap, Opac);
			Pixel(x + JXQ, y - off, color32, color16, wrap, Opac);
		}
		if (off && JXQ)
		{
			Pixel(x + off, y + JXQ, color32, color16, wrap, Opac);
			if (off != JXQ) Pixel(x + JXQ, y + off, color32, color16, wrap, Opac);
		}
	}
	if (fill)
	{
		Opac = fillOpac;
		while (--r > 0)
		{
			off = r;
			for (short JXQ = 0; (JXQ <= off); JXQ++)
			{
				int d1 = JXQ << 16;
				int d2 = off << 16;
				if (d1 < d2) d1 ^= d2, d2 ^= d1, d1 ^= d2;
				d1 += d2 >> 1;
				d2 -= d2 >> 3;
				d1 >>= 16;
				if (d1 > r) off--;
				if (JXQ > off) continue;
				Pixel(x - off, y - JXQ, color32, color16, wrap, Opac);
				if (off != JXQ) Pixel(x - JXQ, y - off, color32, color16, wrap, Opac);
				if (off)
				{
					Pixel(x + off, y - JXQ, color32, color16, wrap, Opac);
					Pixel(x - JXQ, y + off, color32, color16, wrap, Opac);
				}
				if (JXQ && (off != JXQ))
				{
					Pixel(x - off, y + JXQ, color32, color16, wrap, Opac);
					Pixel(x + JXQ, y - off, color32, color16, wrap, Opac);
				}
				if (off && JXQ)
				{
					Pixel(x + off, y + JXQ, color32, color16, wrap, Opac);
					if (off != JXQ) Pixel(x + JXQ, y + off, color32, color16, wrap, Opac);
				}
			}
		}
		Pixel(x,y,color32,color16,wrap,Opac);
	}
}