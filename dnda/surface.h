#include "color.h"

#pragma once

namespace draw
{
	struct surface
	{
		struct plugin
		{
			const char*		name;
			const char*		filter;
			plugin*			next;
			static plugin*	first;
			//
			plugin(const char* name, const char* filter);
			//
			virtual int		decode(unsigned char* output, const unsigned char* input, unsigned size, int& output_scanline) = 0;
			virtual bool	inspect(int& w, int& h, int& bpp, const unsigned char* input, unsigned size) = 0;
		};
		int					width;
		int					height;
		int					scanline;
		int					bpp;
		unsigned char*		bits;
		surface();
		surface(int width, int height, int bpp);
		surface(const char* url, color* pallette = 0);
		~surface();
		operator bool() const { return bits != 0; }
		void				convert(int bpp, color* pallette);
		void				flipv();
		unsigned char*		ptr(int x, int y);
		int					read(const char* url, color* pallette = 0, int need_bpp = 0);
		void				resize(int width, int height, int bpp, bool alloc_memory);
		void				write(const char* url, color* pallette);
	};
	void					write(const char* url, unsigned char* bits, int width, int height, int bpp, int scanline, color* pallette);
}
