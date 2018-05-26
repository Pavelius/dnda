#include "crt.h"
#include "surface.h"

draw::surface::surface() : width(0), height(0), scanline(0), bpp(32), bits(0)
{
}

draw::surface::surface(int width, int height, int bpp) : surface()
{
	resize(width, height, bpp, true);
}

draw::surface::plugin* draw::surface::plugin::first;

draw::surface::plugin::plugin(const char* name, const char* filter) : name(name), filter(filter), next(0)
{
	seqlink(this);
}

unsigned char* draw::surface::ptr(int x, int y)
{
	return bits + y*scanline + x*(bpp / 8);
}

draw::surface::~surface()
{
	resize(0, 0, 0, true);
}

void draw::surface::resize(int width, int height, int bpp, bool alloc_memory)
{
	if(this->width == width && this->height == height && this->bpp == bpp)
		return;
	this->bpp = bpp;
	this->width = width;
	this->height = height;
	this->scanline = color::scanline(width, bpp);
	if(width)
	{
		unsigned size = (height + 1)*scanline;
		if(alloc_memory)
			bits = (unsigned char*)rmreserve(bits, size);
	}
	else
	{
		delete bits;
		bits = 0;
	}
}

void draw::surface::flipv()
{
	color::flipv(bits, scanline, height);
}

void draw::surface::convert(int new_bpp, color* pallette)
{
	if(bpp == new_bpp)
	{
		bpp = iabs(new_bpp);
		return;
	}
	auto old_scanline = scanline;
	scanline = color::scanline(width, new_bpp);
	if(iabs(new_bpp) <= bpp)
		color::convert(bits, width, height, new_bpp, 0, bits, bpp, pallette, old_scanline);
	else
	{
		unsigned char* new_bits = new unsigned char[(height + 1)*scanline];
		color::convert(
			new_bits, width, height, new_bpp, pallette,
			bits, bpp, pallette, old_scanline);
		delete bits;
		bits = new_bits;
	}
	bpp = iabs(new_bpp);
}