#include "color.h"
#include "pma.h"
#include "rect.h"

#pragma once

struct sprite : public pma
{
	enum flagse { NoIndex = 1 };
	enum encodes { Auto, RAW, RLE, ALC, RAW8, RLE8};
	struct frame
	{
		short int		sx, sy;
		short int		ox, oy;
		encodes			encode;
		unsigned		pallette;
		unsigned		offset;
		rect			getrect(int x, int y, unsigned flags) const;
	};
	struct cicle
	{
		short unsigned	start;
		short unsigned	count;
	};
	short int			width; // common width of all frames (if applicable)
	short int			height; // common height of all frames (if applicable)
	short int			ascend;
	short int			descend;
	short unsigned		flags; // must be zero
	unsigned			cicles; // count of anim structure
	unsigned			cicles_offset;
	frame				frames[1];
	//
	explicit operator bool() const;
	//
	frame&				add();
	frame&				addlast();
	void*				add(const void* data, int count);
	int					esize() const;
	const unsigned char* edata() const;
	int					ganim(int index, int tick);
	const frame&		get(int index) const;
	inline cicle*		gcicle(int index) { return (cicle*)offs(cicles_offset) + index; }
	inline int			gindex(int index) const { return *((short unsigned*)((cicle*)offs(cicles_offset) + cicles) + index); }
	int					glyph(unsigned sym) const;
	const unsigned char* offs(unsigned o) const { return (unsigned char*)this + o; }
	void				setup(int count, int pallette_count = 0, int cicles = 0, int cicles_indexes = 0);
	int					store(const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode, unsigned char shadow_index=1, color* pallette = 0, int frame_index=-1, unsigned char transparent_index = 0);
	void				write(const char* url);
};