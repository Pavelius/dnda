#include "bitops.h"
#include "color.h"
#include "command.h"
#include "crt.h"
#include "draw.h"
#include "stringid.h"

// Default theme colors
color					colors::active;
color					colors::button;
color					colors::form;
color					colors::window;
color					colors::text;
color					colors::border;
color					colors::edit;
color					colors::h1;
color					colors::h2;
color					colors::h3;
color					colors::special;
color					colors::tips::text;
color					colors::tips::back;
color					colors::tabs::text;
color					colors::tabs::back;
// Drag elements
const char*				draw::drag::id;
int						draw::drag::part;
int						draw::drag::value;
point					draw::drag::mouse;
// Color context and font context
color					draw::fore;
const sprite*			draw::font;
float					draw::linw = 1.0;
static unsigned char	forc;
bool					draw::mouseinput = true;
color*					draw::palt;
rect					draw::clipping;
rect					draw::lastarea;
char					draw::link[2048];
// Hot keys and menus
int						hot::animate; // Каждый такт таймера это значение увеличивается на единицу.
cursors					hot::cursor; // Текущая форма курсора
int						hot::key; // Событие, которое происходит в данный момент
point					hot::mouse; // current mouse coordinates
bool					hot::pressed; // flag if any of mouse keys is pressed
int						hot::param;
rect					hot::element;
bool					sys_optimize_mouse_move = true;
rect					sys_static_area;
// Locale draw variables
draw::surface*			draw::canvas;
static bool				line_antialiasing = true;
// Metrics
rect					metrics::edit = {4, 4, -4, -4};
sprite*					metrics::font = (sprite*)loadb("font.pma");
sprite*					metrics::h1 = (sprite*)loadb("h1.pma");
sprite*					metrics::h2 = (sprite*)loadb("h2.pma");
sprite*					metrics::h3 = (sprite*)loadb("h3.pma");
sprite*					metrics::icons = (sprite*)loadb("icons.pma");
int						metrics::h3s = 2;
int						metrics::padding = 4;
int						metrics::scroll = 16;
bool					metrics::show::left;
bool					metrics::show::right;
bool					metrics::show::bottom;
bool					metrics::show::statusbar;
stringid*				textf_icons_id;

float sqrt(const float x)
{
	const float xhalf = 0.5f*x;
	union // get bits for floating value
	{
		float x;
		int i;
	} u;
	u.x = x;
	u.i = 0x5f3759df - (u.i >> 1);  // gives initial guess y0
	return x*u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy 
}

int isqrt(int num)
{
	int res = 0;
	int bit = 1 << 30;
	// "bit" starts at the highest power of four <= the argument.
	while(bit > num)
		bit >>= 2;
	while(bit != 0)
	{
		if(num >= res + bit)
		{
			num -= res + bit;
			res = (res >> 1) + bit;
		}
		else
			res >>= 1;
		bit >>= 2;
	}
	return res;
}

int distance(point p1, point p2)
{
	auto dx = p1.x - p2.x;
	auto dy = p1.y - p2.y;
	return isqrt(dx*dx + dy*dy);
}

static inline void correct(int& x1, int& y1, int& x2, int& y2)
{
	if(x1 > x2)
		iswap(x1, x2);
	if(y1 > y2)
		iswap(y1, y2);
}

static bool correct(int& x1, int& y1, int& x2, int& y2, const rect& clip, bool include_edge = true)
{
	correct(x1, y1, x2, y2);
	if(x2 < clip.x1 || x1 >= clip.x2 || y2 < clip.y1 || y1 >= clip.y2)
		return false;
	if(x1 < clip.x1)
		x1 = clip.x1;
	if(y1 < clip.y1)
		y1 = clip.y1;
	if(include_edge)
	{
		if(x2 > clip.x2)
			x2 = clip.x2;
		if(y2 > clip.y2)
			y2 = clip.y2;
	}
	else
	{
		if(x2 >= clip.x2)
			x2 = clip.x2 - 1;
		if(y2 >= clip.y2)
			y2 = clip.y2 - 1;
	}
	return true;
}

static const char* glink(char* temp, const char** source)
{
	temp[0] = 0;
	const char* p = *source;
	if(*p != '(')
		return 0;
	p++;
	if(*p == '\"')
	{
		p = psstr(p+1, temp, *p);
	}
	else
	{
		char* s = temp;
		while(*p && *p != ')')
			*s++ = *p++;
		*s++ = 0;
	}
	if(*p == ')')
		p++;
	*source = p;
	return temp;
}

static const char* word(const char* text)
{
	while(((unsigned char)*text) > 0x20 && *text != '*' && *text != '[' && *text != ']')
		text++;
	return text;
}

static bool match(const char** string, const char* name)
{
	int n = zlen(name);
	if(memcmp(*string, name, n) != 0)
		return false;
	(*string) += n;
	return true;
}

char* key2str(char* result, int key)
{
	result[0] = 0;
	if(key&Ctrl)
		zcat(result, "Ctrl+");
	if(key&Alt)
		zcat(result, "Alt+");
	if(key&Shift)
		zcat(result, "Shift+");
	key = key & 0xFFFF;
	switch(key)
	{
	case KeyDown: zcat(result, "Down"); break;
	case KeyDelete: zcat(result, "Del"); break;
	case KeyEnd: zcat(result, "End"); break;
	case KeyEnter: zcat(result, "Enter"); break;
	case KeyHome: zcat(result, "Home"); break;
	case KeyLeft: zcat(result, "Left"); break;
	case KeyPageDown: zcat(result, "Page Down"); break;
	case KeyPageUp: zcat(result, "Page Up"); break;
	case KeyRight: zcat(result, "Right"); break;
	case KeyUp: zcat(result, "Up"); break;
	case F1: zcat(result, "F1"); break;
	case F2: zcat(result, "F2"); break;
	case F3: zcat(result, "F3"); break;
	case F4: zcat(result, "F4"); break;
	case F5: zcat(result, "F5"); break;
	case F6: zcat(result, "F6"); break;
	case F7: zcat(result, "F7"); break;
	case F8: zcat(result, "F8"); break;
	case F9: zcat(result, "F9"); break;
	case F10: zcat(result, "F10"); break;
	case F11: zcat(result, "F11"); break;
	case F12: zcat(result, "F12"); break;
	case KeySpace: zcat(result, "Space"); break;
	case KeyEscape: zcat(result, "ESC"); break;
	default:
		zcat(result, char(szupper(key - Alpha)));
		break;
	}
	return result;
}

command* command_theme_dark;

void set_dark_theme()
{
	colors::active = color::create(172, 128, 0);
	colors::border = color::create(73, 73, 80);
	colors::button = color::create(0, 122, 204);
	colors::form = color::create(51, 51, 55);
	colors::window = color::create(30, 30, 30);
	colors::text = color::create(255, 255, 255);
	colors::edit = color::create(38, 79, 120);
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	colors::special = color::create(255, 244, 32);
	colors::border = colors::window.mix(colors::text, 128);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(100, 100, 120);
	colors::tabs::back = color::create(255, 204, 0);
	colors::tabs::text = colors::black;
	command_theme_dark->execute();
}

command* command_theme_light;

void set_light_theme()
{
	colors::active = color::create(0, 128, 172);
	colors::button = color::create(223, 223, 223);
	colors::form = color::create(240, 240, 240);
	colors::window = color::create(255, 255, 255);
	colors::text = color::create(0, 0, 0);
	colors::edit = color::create(173, 214, 255);
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	colors::special = color::create(0, 0, 255);
	colors::border = color::create(172, 172, 172);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(80, 80, 120);
	colors::tabs::back = color::create(0, 122, 204);
	colors::tabs::text = color::create(255, 255, 255);
	command_theme_light->execute();
}

void draw::drag::begin(const char* id, int part)
{
	drag::id = id;
	drag::part = part;
	drag::mouse = hot::mouse;
}

bool draw::drag::active(const char* id, int part)
{
	if(drag::id == id && drag::part == part)
	{
		if(!hot::pressed || hot::key == KeyEscape)
		{
			drag::id = 0;
			drag::part = 0;
			hot::key = 0;
			hot::cursor = CursorArrow;
			return false;
		}
		return true;
	}
	return false;
}

draw::state::state() :
	forc(forc),
	mouseinput(draw::mouseinput),
	fore(draw::fore),
	font(draw::font),
	linw(draw::linw),
	palt(draw::palt),
	canvas(draw::canvas),
	clip(clipping)
{
}

draw::state::~state()
{
	draw::font = this->font;
	::forc = this->forc;
	draw::mouseinput = this->mouseinput;
	draw::fore = this->fore;
	draw::linw = this->linw;
	draw::palt = this->palt;
	draw::clipping = this->clip;
	draw::canvas = this->canvas;
}

int draw::getbpp()
{
	return canvas ? canvas->bpp : 1;
}

int draw::getwidth()
{
	return canvas ? canvas->width : 0;
}

int draw::getheight()
{
	return canvas ? canvas->height : 0;
}

unsigned char* draw::ptr(int x, int y)
{
	return canvas ? (canvas->bits + y*canvas->scanline + x*canvas->bpp / 8) : 0;
}

void draw::pixel(int x, int y)
{
	if(x >= clipping.x1 && x < clipping.x2 && y >= clipping.y1 && y < clipping.y2)
	{
		if(!canvas)
			return;
		switch(canvas->bpp)
		{
		case 8:
			*((unsigned char*)canvas->bits + y*canvas->scanline + x) = forc;
			break;
		case 32:
			*((color*)((char*)canvas->bits + y*canvas->scanline + x * 4)) = fore;
			break;
		}
	}
}

void draw::pixel(int x, int y, unsigned char a)
{
	if(x < clipping.x1 || x >= clipping.x2 || y < clipping.y1 || y >= clipping.y2 || a == 0xFF)
		return;
	color* p = (color*)ptr(x, y);
	if(a == 0)
		*p = fore;
	else
	{
		p->b = (((unsigned)p->b*(a)) + (fore.b*(255 - a))) >> 8;
		p->g = (((unsigned)p->g*(a)) + (fore.g*(255 - a))) >> 8;
		p->r = (((unsigned)p->r*(a)) + (fore.r*(255 - a))) >> 8;
		p->a = 0;
	}
}

static void linew(int x0, int y0, int x1, int y1, float wd)
{
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx - dy, e2, x2, y2; /* error value e_xy */
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);

	for(wd = (wd + 1) / 2; ; ) {                                    /* pixel loop */
		draw::pixel(x0, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(err - dx + dy) / ed - wd + 1))));
		e2 = err; x2 = x0;
		if(2 * e2 >= -dx) {                                            /* x step */
			for(e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				draw::pixel(x0, y2 += sy, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(x0 == x1) break;
			e2 = err; err -= dy; x0 += sx;
		}
		if(2 * e2 <= dy) {                                             /* y step */
			for(e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				draw::pixel(x2 += sx, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(y0 == y1) break;
			err += dx; y0 += sy;
		}
	}
}

void draw::line(int x0, int y0, int x1, int y1)
{
	if(!canvas)
		return;
	if(y0 == y1)
	{
		if(!correct(x0, y0, x1, y1, clipping, false))
			return;
		switch(canvas->bpp)
		{
		case 8:
			bop::set8(canvas->ptr(x0, y0), canvas->scanline, x1 - x0 + 1, 1, forc);
			break;
		case 32:
			bop::set32(canvas->ptr(x0, y0), canvas->scanline, x1 - x0 + 1, 1, fore);
			break;
		}
	}
	else if(x0 == x1)
	{
		if(!correct(x0, y0, x1, y1, clipping, false))
			return;
		switch(canvas->bpp)
		{
		case 8:
			bop::set8(canvas->ptr(x0, y0), canvas->scanline, 1, y1 - y0 + 1, forc);
			break;
		case 32:
			bop::set32(canvas->ptr(x0, y0), canvas->scanline, 1, y1 - y0 + 1, fore);
			break;
		}
	}
	else if(line_antialiasing)
	{
		if(linw != 1.0)
			linew(x0, y0, x1, y1, linw);
		else
		{
			int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
			int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
			int err = dx - dy, e2, x2; // error value e_xy
			int ed = dx + dy == 0 ? 1 : isqrt(dx*dx + dy*dy);
			for(;;)
			{
				pixel(x0, y0, 255 * iabs(err - dx + dy) / ed);
				e2 = err; x2 = x0;
				if(2 * e2 >= -dx) // x step
				{
					if(x0 == x1) break;
					if(e2 + dy < ed)
						pixel(x0, y0 + sy, 255 * (e2 + dy) / ed);
					err -= dy; x0 += sx;
				}
				if(2 * e2 <= dy) // y step
				{
					if(y0 == y1)
						break;
					if(dx - e2 < ed)
						pixel(x2 + sx, y0, 255 * (dx - e2) / ed);
					err += dx; y0 += sy;
				}
			}
		}
	}
	else
	{
		int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2;
		for(;;)
		{
			pixel(x0, y0);
			e2 = 2 * err;
			if(e2 >= dy)
			{
				if(x0 == x1) break;
				err += dy; x0 += sx;
			}
			if(e2 <= dx)
			{
				if(y0 == y1) break;
				err += dx; y0 += sy;
			}
		}
	}
}

void draw::bezierseg(int x0, int y0, int x1, int y1, int x2, int y2)
{
	int sx = x2 - x1, sy = y2 - y1;
	long xx = x0 - x1, yy = y0 - y1, xy;             /* relative values for checks */
	double dx, dy, err, ed, cur = xx*sy - yy*sx;                    /* curvature */
	assert(xx*sx <= 0 && yy*sy <= 0);      /* sign of gradient must not change */
	if(sx*(long)sx + sy*(long)sy > xx*xx + yy*yy) {     /* begin with longer part */
		x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;     /* swap P0 P2 */
	}
	if(cur != 0)
	{                                                      /* no straight line */
		xx += sx; xx *= sx = x0 < x2 ? 1 : -1;              /* x step direction */
		yy += sy; yy *= sy = y0 < y2 ? 1 : -1;              /* y step direction */
		xy = 2 * xx*yy; xx *= xx; yy *= yy;             /* differences 2nd degree */
		if(cur*sx*sy < 0) {                              /* negated curvature? */
			xx = -xx; yy = -yy; xy = -xy; cur = -cur;
		}
		dx = 4.0*sy*(x1 - x0)*cur + xx - xy;                /* differences 1st degree */
		dy = 4.0*sx*(y0 - y1)*cur + yy - xy;
		xx += xx; yy += yy; err = dx + dy + xy;                   /* error 1st step */
		do {
			cur = imin(dx + xy, -xy - dy);
			ed = imax(dx + xy, -xy - dy);               /* approximate error distance */
			ed += 2 * ed*cur*cur / (4 * ed*ed + cur*cur);
			pixel(x0, y0, (unsigned char)(255 * iabs(err - dx - dy - xy) / ed));          /* plot curve */
			if(x0 == x2 || y0 == y2) break;     /* last pixel -> curve finished */
			x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
			if(2 * err + dx > 0) {                                        /* x step */
				if(err - dy < ed) pixel(x0, y0 + sy, (unsigned char)(255 * iabs(err - dy) / ed));
				x0 += sx; dx -= xy; err += dy += yy;
			}
			if(y1) {                                                  /* y step */
				if(cur < ed) pixel(x1 + sx, y0, (unsigned char)(255 * iabs(cur) / ed));
				y0 += sy; dy -= xy; err += dx += xx;
			}
		} while(dy < dx);                  /* gradient negates -> close curves */
	}
	line(x0, y0, x2, y2);                  /* plot remaining needle to end */
}

void draw::bezier(int x0, int y0, int x1, int y1, int x2, int y2)
{
	int x = x0 - x1, y = y0 - y1;
	double t = x0 - 2 * x1 + x2, r;
	if((long)x*(x2 - x1) > 0) {                        /* horizontal cut at P4? */
		if((long)y*(y2 - y1) > 0)                     /* vertical cut at P6 too? */
			if(iabs((y0 - 2 * y1 + y2) / t*x) > iabs(y)) {               /* which first? */
				x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;            /* swap points */
			}                            /* now horizontal cut at P4 comes first */
		t = (x0 - x1) / t;
		r = (1 - t)*((1 - t)*y0 + 2.0*t*y1) + t*t*y2;                       /* By(t=P4) */
		t = (x0*x2 - x1*x1)*t / (x0 - x1);                       /* gradient dP4/dx=0 */
		x = ifloor(t + 0.5); y = ifloor(r + 0.5);
		r = (y1 - y0)*(t - x0) / (x1 - x0) + y0;                  /* intersect P3 | P0 P1 */
		bezierseg(x0, y0, x, ifloor(r + 0.5), x, y);
		r = (y1 - y2)*(t - x2) / (x1 - x2) + y2;                  /* intersect P4 | P1 P2 */
		x0 = x1 = x; y0 = y; y1 = ifloor(r + 0.5);             /* P0 = P4, P1 = P8 */
	}
	if((long)(y0 - y1)*(y2 - y1) > 0) {                    /* vertical cut at P6? */
		t = y0 - 2 * y1 + y2; t = (y0 - y1) / t;
		r = (1 - t)*((1 - t)*x0 + 2.0*t*x1) + t*t*x2;                       /* Bx(t=P6) */
		t = (y0*y2 - y1*y1)*t / (y0 - y1);                       /* gradient dP6/dy=0 */
		x = ifloor(r + 0.5); y = ifloor(t + 0.5);
		r = (x1 - x0)*(t - y0) / (y1 - y0) + x0;                  /* intersect P6 | P0 P1 */
		bezierseg(x0, y0, ifloor(r + 0.5), y, x, y);
		r = (x1 - x2)*(t - y2) / (y1 - y2) + x2;                  /* intersect P7 | P1 P2 */
		x0 = x; x1 = ifloor(r + 0.5); y0 = y1 = y;             /* P0 = P6, P1 = P7 */
	}
	bezierseg(x0, y0, x1, y1, x2, y2);                  /* remaining part */
}

void draw::spline(point* original_points, int n)
{
	point points[256];
	n = imin(sizeof(points) / sizeof(points[0]), (unsigned)n) - 1;
	memcpy(points, original_points, sizeof(points[0])*(n + 1));
	const int M_MAX = 6;
	float mi = 1, m[M_MAX];                    /* diagonal constants of matrix */
	int i, x0, y0, x1, y1, x2 = points[n].x, y2 = points[n].y;
	assert(n > 1);                        /* need at least 3 points P[0]..P[n] */
	points[1].x = x0 = 8 * points[1].x - 2 * points[0].x;                          /* first row of matrix */
	points[1].y = y0 = 8 * points[1].y - 2 * points[0].y;
	for(i = 2; i < n; i++) {                                 /* forward sweep */
		if(i - 2 < M_MAX) m[i - 2] = mi = (float)(1.0 / (6.0 - mi));
		points[i].x = x0 = ifloor(8 * points[i].x - x0*mi + 0.5);                        /* store yi */
		points[i].y = y0 = ifloor(8 * points[i].y - y0*mi + 0.5);
	}
	x1 = ifloor((x0 - 2 * x2) / (5.0 - mi) + 0.5);                 /* correction last row */
	y1 = ifloor((y0 - 2 * y2) / (5.0 - mi) + 0.5);
	for(i = n - 2; i > 0; i--) {                           /* back substitution */
		if(i <= M_MAX) mi = m[i - 1];
		x0 = ifloor((points[i].x - x1)*mi + 0.5);                            /* next corner */
		y0 = ifloor((points[i].y - y1)*mi + 0.5);
		bezier((x0 + x1) / 2, (y0 + y1) / 2, x1, y1, x2, y2);
		x2 = (x0 + x1) / 2; x1 = x0;
		y2 = (y0 + y1) / 2; y1 = y0;
	}
	bezier(points[0].x, points[0].y, x1, y1, x2, y2);
}

void draw::line(int x0, int y0, int x1, int y1, unsigned char c1)
{
	draw::state push;
	setcolor(c1);
	line(x0, y0, x1, y1);
}

void draw::line(int x0, int y0, int x1, int y1, color c1)
{
	draw::state push;
	fore = c1;
	line(x0, y0, x1, y1);
}

void draw::linet(int x0, int y0, int x1, int y1)
{
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; // error value e_xy
	for(;;)
	{
		if((x0 + y0) & 1)
			pixel(x0, y0);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if(e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
}

void draw::rectb(rect rc)
{
	line(rc.x1, rc.y1, rc.x2, rc.y1);
	line(rc.x2, rc.y1 + 1, rc.x2, rc.y2);
	line(rc.x2 - 1, rc.y2, rc.x1, rc.y2);
	line(rc.x1, rc.y2 - 1, rc.x1, rc.y1);
}

void draw::rectb(rect rc, unsigned char c1)
{
	draw::state push;
	setcolor(c1);
	rectb(rc);
}

void draw::rectb(rect rc, color c1)
{
	draw::state push;
	fore = c1;
	rectb(rc);
}

void draw::rectf(rect rc)
{
	if(!canvas)
		return;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	if(rc.x1 == rc.x2)
		return;
	switch(canvas->bpp)
	{
	case 8:
		bop::set8(ptr(rc.x1, rc.y1), canvas->scanline,
			rc.x2 - rc.x1, rc.y2 - rc.y1, forc);
		break;
	case 32:
		bop::set32(ptr(rc.x1, rc.y1), canvas->scanline,
			rc.x2 - rc.x1, rc.y2 - rc.y1, fore);
		break;
	}
}

void draw::rectf(rect rc, unsigned char c1)
{
	state push;
	setcolor(c1);
	rectf(rc);
}

void draw::rectf(rect rc, color c1)
{
	state push;
	fore = c1;
	rectf(rc);
}

void draw::rectf(rect rc, color c1, unsigned char alpha)
{
	if(!canvas)
		return;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	if(rc.x1 == rc.x2)
		return;
	switch(canvas->bpp)
	{
	case 8:
		break;
	case 32:
		bop::set32(ptr(rc.x1, rc.y1), canvas->scanline,
			rc.x2 - rc.x1, rc.y2 - rc.y1, c1, alpha);
		break;
	}
}

void draw::rectf(rect rc, unsigned char c1, unsigned char alpha)
{
	state push;
	setcolor(c1);
	rectf(rc, fore, alpha);
}

void draw::rectx(rect rc, color c1)
{
	state push; fore = c1;
	linet(rc.x1, rc.y1, rc.x2, rc.y1);
	linet(rc.x2, rc.y1 + 1, rc.x2, rc.y2);
	linet(rc.x2 - 1, rc.y2, rc.x1, rc.y2);
	linet(rc.x1, rc.y2 - 1, rc.x1, rc.y1);
}

void draw::gradv(rect rc, const color c1, const color c2, int skip)
{
	if(!canvas)
		return;
	int m = iabs(rc.height());
	if(skip > m)
		skip = m;
	double k3 = m;
	if(!k3)
		return;
	int y0 = rc.y1;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	int w1 = rc.width();
	skip += rc.y1 - y0;
	for(int y = rc.y1 + skip; y < rc.y2; y++)
	{
		double k2 = (double)(y - rc.y1) / k3;
		double k1 = 1.00f - k2;
		color c;
		c.r = (unsigned char)(c1.r*k1 + c2.r*k2);
		c.g = (unsigned char)(c1.g*k1 + c2.g*k2);
		c.b = (unsigned char)(c1.b*k1 + c2.b*k2);
		bop::set32(canvas->ptr(rc.x1, y), canvas->scanline, w1, 1, c);
	}
}

void draw::gradh(rect rc, const color c1, const color c2, int skip)
{
	if(!canvas)
		return;
	double k3 = iabs(rc.width());
	if(!k3)
		return;
	int x0 = rc.x1;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	int h1 = rc.height();
	skip += rc.x1 - x0;
	for(int x = rc.x1 + skip; x < rc.x2; x++)
	{
		double k2 = (double)(x - rc.x1) / k3;
		double k1 = 1.00f - k2;
		color c;
		c.r = (unsigned char)(c1.r*k1 + c2.r*k2);
		c.g = (unsigned char)(c1.g*k1 + c2.g*k2);
		c.b = (unsigned char)(c1.b*k1 + c2.b*k2);
		bop::set32(canvas->ptr(x, rc.y1), canvas->scanline, 1, h1, c);
	}
}

void draw::circlef(int xm, int ym, int r, const color c1, unsigned char alpha)
{
	if(xm - r >= clipping.x2 || xm + r < clipping.x1 || ym - r >= clipping.y2 || ym + r < clipping.y1)
		return;
	int x = -r, y = 0, err = 2 - 2 * r, y1;
	do
	{
		y1 = ym + y;
		rectf({xm + x, y1, xm - x, y1 + 1}, c1, alpha);
		if(y != 0)
		{
			y1 = ym - y;
			rectf({xm + x, y1, xm - x, y1 + 1}, c1, alpha);
		}
		r = err;
		if(r <= y)
			err += ++y * 2 + 1;
		if(r > x || err > y)
			err += ++x * 2 + 1;
	} while(x < 0);
}

void draw::circle(int xm, int ym, int r)
{
	int x = r, y = 0; // II. quadrant from bottom left to top right
	int x2, e2, err = 2 - 2 * r; // error of 1.step
	r = 1 - err;
	for(;;)
	{
		int i = 255 * iabs(err + 2 * (x + y) - 2) / r; // get blend value of pixel
		pixel(xm + x, ym - y, i); // I. Quadrant
		pixel(xm + y, ym + x, i); // II. Quadrant
		pixel(xm - x, ym + y, i); // III. Quadrant
		pixel(xm - y, ym - x, i); // IV. Quadrant
		if(x == 0)
			break;
		e2 = err; x2 = x; // remember values
		if(err > y)
		{
			/* x step */
			int i = 255 * (err + 2 * x - 1) / r; // outward pixel
			if(i < 255)
			{
				pixel(xm + x, ym - y + 1, i);
				pixel(xm + y - 1, ym + x, i);
				pixel(xm - x, ym + y - 1, i);
				pixel(xm - y + 1, ym - x, i);
			}
			err -= --x * 2 - 1;
		}
		if(e2 <= x2--)
		{
			/* y step */
			int i = 255 * (1 - 2 * y - e2) / r;
			if(i < 255)
			{
				pixel(xm + x2, ym - y, i);
				pixel(xm + y, ym + x2, i);
				pixel(xm - x2, ym + y, i);
				pixel(xm - y, ym - x2, i);
			}
			err -= --y * 2 - 1;
		}
	}
}

void draw::circle(int x, int y, int r, const color c1)
{
	state push;
	fore = c1;
	circle(x, y, r);
}

void draw::setcolor(unsigned char index)
{
	forc = index;
	if(palt)
		fore = palt[index];
}

void draw::setclip(rect rcn)
{
	rect rc = draw::clipping;
	if(rc.x1 < rcn.x1)
		rc.x1 = rcn.x1;
	if(rc.y1 < rcn.y1)
		rc.y1 = rcn.y1;
	if(rc.x2 > rcn.x2)
		rc.x2 = rcn.x2;
	if(rc.y2 > rcn.y2)
		rc.y2 = rcn.y2;
	draw::clipping = rc;
}

static void intersect_rect(rect& r1, const rect& r2)
{
	if(hot::mouse.in(r2))
		r1 = r2;
	else if(!r1.intersect(r2))
		return;
	else
	{
		if(hot::mouse.x > r2.x2 && r2.x2 > r1.x1)
			r1.x1 = r2.x2;
		if(hot::mouse.x < r2.x1 && r2.x1 < r1.x2)
			r1.x2 = r2.x1;
		if(hot::mouse.y > r2.y2 && r2.y2 > r1.y1)
			r1.y1 = r2.y2;
		if(hot::mouse.y < r2.y1 && r2.y1 < r1.y2)
			r1.y2 = r2.y1;
	}
}

areas draw::area(rect rc)
{
	if(!hot::mouse.in(clipping))
		return AreaNormal;
	if(sys_optimize_mouse_move)
		intersect_rect(sys_static_area, rc);
	if(drag::id)
		return AreaNormal;
	if(!mouseinput)
		return AreaNormal;
	if(hot::mouse.in(rc))
	{
		lastarea = rc;
		return hot::pressed ? AreaHilitedPressed : AreaHilited;
	}
	return AreaNormal;
}

bool draw::areb(rect rc)
{
	areas a = area(rc);
	return a == AreaHilited
		|| a == AreaHilitedPressed;
}

int	draw::aligned(int x, int width, unsigned flags, int dx)
{
	switch(flags&AlignMask)
	{
	case AlignRightBottom:
	case AlignRightCenter:
	case AlignRight: return x + width - dx;
	case AlignCenterBottom:
	case AlignCenterCenter:
	case AlignCenter: return x + (width - dx) / 2;
	default: return x;
	}
}

int draw::alignedh(const rect& rc, const char* string, unsigned state)
{
	int ty;
	switch(state&AlignMask)
	{
	case AlignCenterCenter:
	case AlignRightCenter:
	case AlignLeftCenter:
		if(state&TextSingleLine)
			ty = draw::texth();
		else
			ty = draw::texth(string, rc.width());
		return (rc.height() - ty) / 2;
	case AlignCenterBottom:
	case AlignRightBottom:
	case AlignLeftBottom:
		if(state&TextSingleLine)
			ty = draw::texth();
		else
			ty = draw::texth(string, rc.width());
		return rc.y2 - ty;
	default:
		return 0;
	}
}

int draw::textw(const char* string, int count)
{
	if(!font)
		return 0;
	int x1 = 0;
	if(count == -1)
	{
		const char *s1 = string;
		while(*s1)
			x1 += textw(szget(&s1));
	}
	else
	{
		const char *s1 = string;
		const char *s2 = string + count;
		while(s1 < s2)
			x1 += textw(szget(&s1));
	}
	return x1;
}

const char* draw::skiptr(const char* string)
{
	// skiping trail symbols
	for(; *string && *string == 0x20; string++);
	if(*string == 13)
	{
		string++;
		if(*string == 10)
			string++;
	}
	else if(*string == 10)
	{
		string++;
		if(*string == 13)
			string++;
	}
	return string;
}

int draw::textw(rect& rc, const char* string)
{
	int w1 = rc.width();
	rc.y2 = rc.y1;
	rc.x2 = rc.x1;
	while(string[0])
	{
		int c = textbc(string, w1);
		if(!c)
			break;
		int m = textw(string, c);
		if(rc.width() < m)
			rc.x2 = rc.x1 + m;
		rc.y2 += texth();
		string = skiptr(string + c);
	}
	return rc.height();
}

int draw::texth(const char* string, int width)
{
	int dy = texth();
	int y1 = 0;
	while(*string)
	{
		int c = textbc(string, width);
		if(!c)
			break;
		y1 += dy;
		string = skiptr(string + c);
	}
	return y1;
}

void draw::text(int x, int y, const char* string, int count, unsigned flags)
{
	if(!font)
		return;
	int dy = texth();
	if(y >= clipping.y2 || y + dy < clipping.y1)
		return;
	if(count == -1)
		count = zlen(string);
	const char *s1 = string;
	const char *s2 = string + count;
	while(s1 < s2)
	{
		int sm = szget(&s1);
		if(sm >= 0x21)
			glyph(x, y, sm, flags);
		x += textw(sm);
	}
}

int draw::textc(int x, int y, int width, const char* string, int count, unsigned flags)
{
	state push;
	setclip({x, y, x + width, y + texth()});
	text(x, y, string, count, flags);
	return texth();
}

int draw::textbc(const char* string, int width)
{
	if(!font)
		return 0;
	int p = -1;
	int w = 0;
	const char* s1 = string;
	while(true)
	{
		unsigned s = szget(&s1);
		if(s == 0x20 || s == 9)
			p = s1 - string;
		else if(s == 0)
		{
			p = s1 - string - 1;
			break;
		}
		else if(s == 10 || s == 13)
		{
			p = s1 - string;
			break;
		}
		w += textw(s);
		if(w > width)
			break;
	}
	if(p == -1)
		p = s1 - string;
	return p;
}

int	draw::text(rect rc, const char* string, unsigned state, int* max_width)
{
	if(!string || string[0] == 0)
		return 0;
	int x1 = rc.x1;
	int y1 = rc.y1 + alignedh(rc, string, state);
	int dy = texth();
	if(max_width)
		*max_width = 0;
	if(state&TextSingleLine)
	{
		text(aligned(x1, rc.width(), state, draw::textw(string)), y1,
			string, -1, state);
		return dy;
	}
	else
	{
		int w1 = rc.width();
		while(y1 < rc.y2)
		{
			int c = textbc(string, w1);
			if(!c)
				break;
			int w = textw(string, c);
			if(max_width && *max_width < w)
				*max_width = w;
			text(aligned(x1, w1, state, w), y1, string, c, state);
			y1 += dy;
			string = skiptr(string + c);
		}
		return y1 - rc.y1;
	}
}

static const char* textspc(const char* p, int x0, int& x, int tab_width)
{
	int tb;
	while(true)
	{
		switch(p[0])
		{
		case ' ':
			p++;
			x += draw::textw(' ');
			continue;
		case '\t':
			p++;
			if(!tab_width)
				tb = draw::textw(' ') * 4;
			else
				tb = tab_width;
			x = x0 + ((x - x0 + tb) / tb)*tb;
			continue;
		}
		break;
	}
	return p;
}

static int textfln(int x0, int y0, int width, const char** string, color c1, int* max_width, int tab_width)
{
	char temp[4096];
	int y = y0;
	int x = x0;
	int x2 = x0 + width;
	const char* p = *string;
	unsigned flags = 0;
	draw::fore = c1;
	if(max_width)
		*max_width = 0;
	temp[0] = 0;
	while(true)
	{
		if(p[0] == '*' && p[1] == '*')
		{
			p += 2;
			if(flags&TextBold)
				flags &= ~TextBold;
			else
				flags |= TextBold;
			continue;
		}
		else if(p[0] == '*')
		{
			p++;
			if(flags&TextItalic)
				flags &= ~TextItalic;
			else
			{
				if((flags&TextItalic) == 0)
					x += draw::texth() / 3;
				flags |= TextItalic;
			}
			continue;
		}
		else if(p[0] == '[' && p[1] == '[')
			p++;
		else if(p[0] == ']' && p[1] == ']')
			p++;
		else if(p[0] == '[')
		{
			p++;
			switch(*p)
			{
			case '~':
				p++;
				draw::fore = colors::text.mix(colors::window, 64);
				break;
			case '+':
				p++;
				draw::fore = colors::green;
				break;
			case '-':
				p++;
				draw::fore = colors::red;
				break;
			case '!':
				p++;
				draw::fore = colors::yellow;
				break;
			case '#':
				p++;
				flags |= TextUscope;
				draw::fore = colors::special;
				break;
			default:
				draw::fore = colors::special;
				break;
			}
			glink(temp, &p);
		}
		else if(p[0] == ']')
		{
			p++;
			draw::fore = c1;
			temp[0] = 0;
			flags &= ~TextUscope;
		}
		// Обработаем пробелы и табуляцию
		p = textspc(p, x0, x, tab_width);
		int w;
		if(p[0] == ':' && p[1] >= 'a' && p[1] <= 'z')
		{
			p++;
			char temp[128];
			p = psidn(p, temp, temp + sizeof(temp) - 1);
			if(*p == ':')
				p++;
			w = 0;
			if(metrics::icons)
			{
				int index = 0;
				if(textf_icons_id)
				{
					for(auto p = textf_icons_id; p->id; p++)
					{
						if(strcmp(p->id, temp) == 0)
						{
							index = p->value;
							break;
						}
					}
				}
				auto fr = metrics::icons->get(index);
				w = fr.sx;
				if(x + w > x2)
				{
					if(max_width)
						*max_width = imax(*max_width, x - x0);
					x = x0;
					y += draw::texth();
				}
				draw::image(x + fr.sx / 2, y, metrics::icons, index, 0);
			}
		}
		else
		{
			const char* p2 = word(p);
			w = draw::textw(p, p2 - p);
			if(x + w > x2)
			{
				if(max_width)
					*max_width = imax(*max_width, x - x0);
				x = x0;
				y += draw::texth();
			}
			draw::text(x, y, p, p2 - p, flags);
			p = p2;
		}
		int x4 = x;
		x += w;
		p = textspc(p, x0, x, tab_width);
		if(temp[0] || (flags&TextUscope)!=0)
		{
			int x3 = imin(x2, x);
			int y2 = y + draw::texth();
			if(flags&TextUscope)
				draw::line(x4, y2, x3, y2, draw::fore);
			rect rc = {x4, y, x3, y2};
			if(draw::areb(rc))
			{
				if(flags&TextUscope)
				{
					hot::cursor = CursorHand;
					if(temp[0] && hot::key == MouseLeft && !hot::pressed)
					{
						zcpy(draw::link, temp, sizeof(draw::link) - 1);
						//draw::execute(HtmlLink);
					}
				}
				else
					zcpy(draw::link, temp, sizeof(draw::link) - 1);
			}
		}
		// Отметим перевод строки и окончание строки
		if(p[0] == 0 || p[0] == 10 || p[0] == 13)
		{
			y += draw::texth();
			p = szskipcr(p);
			break;
		}
	}
	if(max_width)
		*max_width = imax(*max_width, x - x0);
	*string = p;
	return y - y0;
}

static int render_control(const char** result, int x, int y, int width)
{
	char type[64];
	char name[64];
	char buffer[4096];
	auto p = *result;
	auto pb = buffer;
	auto pe = buffer + sizeof(buffer) - 1;
	p = psidn(p, type, type + sizeof(type) - 1);
	p = zskipsp(p);
	while(*p && *p != ')')
	{
		p = psidn(p, name, name + sizeof(name) - 1);
		p = zskipsp(p);
		int value_number = 1;
		const char* value_text = 0;
		if(*p == '=')
		{
			p = zskipsp(p+1);
			if(isnum(*p) || *p == '-')
				value_number = sz2num(p, &p);
			else if(*p == '\"' || *p == '\'')
			{
				value_text = pb;
				p = psstr(p + 1, pb, p[0]);
				pb = zend(pb);
				if(pb < pe)
					pb = pb + 1;
			}
			else
			{
				// Error
				while(*p != ')' && *p)
					p++;
			}
			p = zskipsp(p);
		}
		//auto fd = draw::element_type[0].find(name);
		//if(fd)
		//{
		//	if(fd->type==text_type)
		//		fd->set(fd->ptr(&e), (int)value_text);
		//	else if(fd->type == number_type)
		//		fd->set(fd->ptr(&e), value_number);
		//}
	}
	if(*p == ')')
		p++;
	p = szskipcr(p);
	*result = p;
	//auto pm = draw::widget::plugin::find(type);
	//if(pm)
	//	return pm->render(x, y, width, e);
	return 0;
}

int draw::textf(int x, int y, int width, const char* string, int* max_width,
	int min_height, int* cashe_height, const char** cashe_string, int tab_width)
{
	state push;
	color color_text = fore;
	const char* p = string;
	int y0 = y;
	if(cashe_height)
	{
		*cashe_string = p;
		*cashe_height = 0;
	}
	if(max_width)
		*max_width = 0;
	while(p[0])
	{
		int mw2 = 0;
		if(cashe_height && (y - y0) <= min_height)
		{
			*cashe_string = p;
			*cashe_height = y - y0;
		}
		if(match(&p, "###")) // Header 3
		{
			p = zskipsp(p);
			font = metrics::h3;
			y += textfln(x, y, width, &p, colors::h3, &mw2, tab_width);
		}
		else if(match(&p, "##")) // Header 2
		{
			p = zskipsp(p);
			font = metrics::h2;
			y += textfln(x, y, width, &p, colors::h2, &mw2, tab_width);
		}
		else if(match(&p, "#")) // Header 1
		{
			p = zskipsp(p);
			font = metrics::h1;
			y += textfln(x, y, width, &p, colors::h1, &mw2, tab_width);
		}
		else if(match(&p, "...")) // Без форматирования
		{
			p = szskipcr(p);
			font = metrics::font;
			color c1 = colors::window.mix(colors::edit, 256 - 32);
			y += texth() / 2;
			while(p[0])
			{
				int c = textbc(p, width);
				if(!c)
					break;
				rectf({x, y, x + width, y + texth()}, c1);
				text(x, y, p, c);
				y += texth();
				p += c;
				if(match(&p, "..."))
				{
					p = szskipcr(p);
					y += texth() / 2;
					break;
				}
			}
		}
		else if(match(&p, "* ")) // Список
		{
			int dx = texth() / 2;
			int rd = texth() / 6;
			circlef(x + dx + 2, y + dx, rd, color_text);
			circle(x + dx + 2, y + dx, rd, color_text);
			y += textfln(x + texth(), y, width - texth(), &p, color_text, &mw2, tab_width);
		}
		else if(p[0] == '$' && p[1] == '(')
		{
			p = zskipsp(p + 2);
			y += render_control(&p, x, y, width);
		}
		else
		{
			y += textfln(x, y, width, &p, color_text, &mw2, tab_width);
			if(p[0])
			{
				if(p[0] == ':' && p[1] == ':' && p[2] == ':')
					p += 3;
				else
					y += metrics::padding;
			}
		}
		// Возвратим стандартные настройки блока
		font = metrics::font;
		fore = color_text;
		if(max_width)
		{
			if(*max_width < mw2)
				*max_width = mw2;
		}
	}
	return y - y0;
}

int draw::textf(rect& rc, const char* string, int tab_width)
{
	state push;
	clipping.clear();
	rc.y2 = rc.y1 + draw::textf(0, 0, rc.width(), string, &rc.x2, 0, 0, 0, tab_width);
	rc.x2 += rc.x1;
	return rc.height();
}

int draw::textlb(const char* string, int index, int width, int* line_index, int* line_count)
{
	auto dy = texth();
	auto p = string;
	if(line_index)
		*line_index = 0;
	if(line_count)
		*line_count = 0;
	while(true)
	{
		int c = textbc(p, width);
		if(!c)
			break;
		if(index < c || p[c] == 0)
		{
			if(line_count)
				*line_count = c;
			break;
		}
		index -= c;
		if(line_index)
			*line_index = *line_index + 1;
		p = skiptr(p + c);
	}
	return p - string;
}

static void hilite_text_line(int x, int y, int width, int height, const char* string, int count, unsigned state, int i1, int i2)
{
	int w = draw::textw(string, count);
	auto x0 = draw::aligned(x, width, state, w);
	// Выделение
	if(i1 != i2 && ((i1 >= 0 && i1 < count) || (i2 >= 0 && i2 < count) || (i1 < 0 && i2 >= count)))
	{
		int rx1 = x0 + 1;
		int rx2 = x0 + w + 1;
		if(i1 >= 0 && i1 <= count)
			rx1 = x0 + 1 + draw::textw(string, i1);
		if(i2 >= 0 && i2 <= count)
			rx2 = x0 + 1 + draw::textw(string, i2);
		draw::rectf({rx1, y, rx2, y + height}, colors::edit);
	}
	// Вывод текста
	draw::text(x0, y, string, count);
	// Вывод курсора
	if(i1 >= 0 && i1 == i2
		&& (i1 < count || (i1 == count && string[count] == 0)))
	{
		int rx1 = x0 + draw::textw(string, i1);
		draw::line(rx1, y, rx1, y + height, colors::text);
	}
}

int	draw::texte(rect rc, const char* string, unsigned state, int p1, int p2)
{
	auto i1 = p1;
	auto i2 = p2;
	if(i2 != -1 && i1 > i2)
		iswap(i1, i2);
	if(i2 == -1)
		i2 = i1;
	auto dy = texth();
	auto w1 = rc.width();
	auto y1 = rc.y1 + alignedh(rc, string, state);
	auto p = string;
	if(!p)
		return 0;
	if(state&TextSingleLine)
	{
		hilite_text_line(rc.x1, y1, w1, dy, p, zlen(p), state, i1, i2);
		return dy;
	}
	else if(string[0])
	{
		while(y1 < rc.y2)
		{
			int c = textbc(p, w1);
			if(!c)
				break;
			hilite_text_line(rc.x1, y1, w1, dy, p, c, state, i1, i2);
			i1 -= c;
			i2 -= c;
			y1 += dy;
			p = skiptr(p + c);
		}
	}
	else
		hilite_text_line(rc.x1, y1, w1, dy, p, 0, state, i1, i2);
	return y1 - rc.y1;
}

int draw::hittest(int x, int hit_x, const char* p, int lenght)
{
	if(hit_x < x)
		return -2;
	int index = 0;
	int syw = 0;
	while(index < lenght)
	{
		syw = draw::textw(szget(&p));
		if(hit_x <= x + 1 + syw / 2)
			break;
		x += syw;
		index++;
	}
	if(index == lenght)
	{
		if(hit_x > x + syw)
			index = -3;
	}
	return index;
}

int draw::hittest(rect rc, const char* string, unsigned state, point pt)
{
	int w1 = rc.width();
	int dy = texth();
	int x1 = rc.x1;
	int y1 = rc.y1 + draw::alignedh(rc, string, state);
	auto p = (const char*)string;
	if(state&TextSingleLine)
	{
		auto c = zlen(string);
		auto w1 = textw(string, c);
		auto x0 = aligned(x1, w1, state, w1);
		return hittest(x0, pt.x, string, c);
	}
	else
	{
		while(y1 < rc.y2)
		{
			int c = textbc(p, w1);
			if(!c)
				break;
			int w = textw(p, c);
			auto x0 = aligned(x1, w1, state, w);
			if(pt.y >= y1 && pt.y < y1 + dy)
			{
				if(pt.x >= x0 && pt.x < x0 + w)
					return p - string + draw::hittest(x0, pt.x, p, c);
				if(pt.x < x0)
					return -2;
				return -3;
			}
			y1 += dy;
			p = draw::skiptr(p + c);
		}
	}
	return -1;
}

static unsigned char* skip_v3(unsigned char* s, int h)
{
	const int		cbs = 1;
	const int		cbd = 1;
	if(!s || !h)
		return s;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			if(--h == 0)
				return s;
		}
		else if(c <= 0x9F)
		{
			if(c <= 0x7F)
				s += c*cbs;
			else
			{
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c*cbs;
			}
		}
		else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_rle32(unsigned char* s, int h)
{
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			if(--h == 0)
				return s;
		}
		else if(c <= 0x9F)
		{
			if(c <= 0x7F)
				s += c*cbs;
			else
			{
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c*cbs;
			}
		}
		else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_alc(unsigned char* s, int h)
{
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			if(--h == 0)
				return s;
		}
		else if(c <= 0x7F)
			s += c*cbs;
		else if(c == 0x80)
			s++;
	}
}

bool draw::hittest(int x, int y, const sprite* e, int id, int flags, point mouse)
{
	int x2, y2;
	if(!mouse.in(clipping) || !e)
		return false;
	const sprite::frame& f = e->get(id);
	if(!f.offset)
		return false;
	if(flags&ImageMirrorH)
	{
		x2 = x;
		if((flags&ImageNoOffset) == 0)
			x2 += f.ox;
		x = x2 - f.sx;
	}
	else
	{
		if((flags&ImageNoOffset) == 0)
			x -= f.ox;
		x2 = x + f.sx;
	}
	if(flags&ImageMirrorV)
	{
		y2 = y;
		if((flags&ImageNoOffset) == 0)
			y2 += f.oy;
		y = y2 - f.sy;
	}
	else
	{
		if((flags&ImageNoOffset) == 0)
			y -= f.oy;
		y2 = y + f.sy;
	}
	unsigned char* s = (unsigned char*)e + f.offset;
	if(y2<clipping.y1 || y>clipping.y2 || x2<clipping.x1 || x>clipping.x2)
		return false;
	int wd = (flags&ImageMirrorV) ? -1 : 1;
	int sy = (flags&ImageMirrorV) ? y2 - 1 : y;
	rect rc = {x, y, x2, y2};
	if(!mouse.in(rc))
		return false;
	switch(f.encode)
	{
	case sprite::RLE8:
		if(flags&ImageMirrorH)
			return bop::rle8h(x2 - 1, sy, -1, wd, s, y2 - y, mouse);
		else
			return bop::rle8h(x, sy, 1, wd, s, y2 - y, mouse);
	case sprite::RLE:
		if(flags&ImageMirrorH)
		{
			//	bop::rle32m(ptr(x2 - 1, sy), wd, s, y2 - y,
			//		ptr(clipping.x1, sy),
			//		ptr(clipping.x2, sy),
			//		alpha);
		}
		else
		{
			//	bop::rle32(ptr(x, sy), wd, s, y2 - y,
			//		ptr(clipping.x1, sy),
			//		ptr(clipping.x2, sy),
			//		alpha);
		}
		return false;
	default:
		return true;
	}
}

void draw::image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha)
{
	const int cbd = 1;
	int x2, y2;
	color* pal;
	if(!e)
		return;
	const sprite::frame& f = e->get(id);
	if(!f.offset)
		return;
	if(!canvas)
		return;
	if(flags&ImageMirrorH)
	{
		x2 = x;
		if((flags&ImageNoOffset) == 0)
			x2 += f.ox;
		x = x2 - f.sx;
	}
	else
	{
		if((flags&ImageNoOffset) == 0)
			x -= f.ox;
		x2 = x + f.sx;
	}
	if(flags&ImageMirrorV)
	{
		y2 = y;
		if((flags&ImageNoOffset) == 0)
			y2 += f.oy;
		y = y2 - f.sy;
	}
	else
	{
		if((flags&ImageNoOffset) == 0)
			y -= f.oy;
		y2 = y + f.sy;
	}
	unsigned char* s = (unsigned char*)e + f.offset;
	if(y2<clipping.y1 || y>clipping.y2 || x2<clipping.x1 || x>clipping.x2)
		return;
	int bypp = canvas->bpp / 8;
	if(y < clipping.y1)
	{
		if((flags&ImageMirrorV) == 0)
		{
			switch(f.encode)
			{
			case sprite::ALC: s = skip_alc(s, clipping.y1 - y); break;
			case sprite::RAW: s += (clipping.y1 - y)*f.sx * 3; break;
			case sprite::RAW8: s += (clipping.y1 - y)*f.sx; break;
			case sprite::RLE8: s = skip_v3(s, clipping.y1 - y); break;
			case sprite::RLE: s = skip_rle32(s, clipping.y1 - y); break;
			default: break;
			}
		}
		y = clipping.y1;
	}
	if(y2 > clipping.y2)
	{
		if(flags&ImageMirrorV)
		{
			switch(f.encode)
			{
			case sprite::ALC: s = skip_alc(s, y2 - clipping.y2); break;
			case sprite::RAW: s += (y2 - clipping.y2)*f.sx * 3; break;
			case sprite::RAW8: s += (y2 - clipping.y2)*f.sx; break;
			case sprite::RLE8: s = skip_v3(s, y2 - clipping.y2); break;
			case sprite::RLE: s = skip_rle32(s, y2 - clipping.y2); break;
			default: break;
			}
		}
		y2 = clipping.y2;
	}
	if(y >= y2)
		return;
	int wd = (flags&ImageMirrorV) ? -canvas->scanline : canvas->scanline;
	int sy = (flags&ImageMirrorV) ? y2 - 1 : y;
	switch(f.encode)
	{
	case sprite::RAW:
		if(x < clipping.x1)
		{
			s += (clipping.x1 - x) * 3;
			x = clipping.x1;
		}
		if(x2 > clipping.x2)
			x2 = clipping.x2;
		if(x >= x2)
			return;
		if(bypp == 4)
		{
			if(flags&ImageMirrorH)
				bop::raw32m(ptr(x2 - 1, y), wd, s, f.sx * 3,
					x2 - x,
					y2 - y);
			else
				bop::raw32(ptr(x, y), wd, s, f.sx * 3,
					x2 - x,
					y2 - y);
		}
		break;
	case sprite::RAW8:
		if(x < clipping.x1)
		{
			s += clipping.x1 - x;
			x = clipping.x1;
		}
		if(x2 > clipping.x2)
			x2 = clipping.x2;
		if(x >= x2)
			return;
		if(bypp == 1)
		{
			if(flags&ImageMirrorH)
			{
				if(flags&ImageTransparent)
					bop::raw8tm(ptr(x2 - 1, y), wd, s, f.sx,
						x2 - x, y2 - y);
				else
					bop::raw8m(ptr(x2 - 1, y), wd, s, f.sx,
						x2 - x, y2 - y);
			}
			else
			{
				if(flags&ImageTransparent)
					bop::raw8t(ptr(x, y), wd, s, f.sx,
						x2 - x,
						y2 - y);
				else
					bop::cpy(ptr(x, y), wd,
						s, f.sx,
						x2 - x, y2 - y, 1);
			}
		}
		else if(bypp == 4)
		{
			if(!f.pallette || (flags&ImagePallette))
				pal = draw::palt;
			else
				pal = (color*)e->offs(f.pallette);
			if(!pal)
				return;
			if(flags&ImageMirrorH)
				bop::raw832m(ptr(x2 - 1, y), wd, s, f.sx,
					x2 - x,
					y2 - y,
					pal);
			else
				bop::raw832(ptr(x, y), wd, s, f.sx, x2 - x, y2 - y, pal);
		}
		break;
	case sprite::RLE8:
		if(bypp == 1)
		{
			if(flags&ImageMirrorH)
				bop::rle8m(ptr(x2 - 1, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy));
			else
				bop::rle8(ptr(x, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy));
		}
		else if(bypp == 4)
		{
			if(!f.pallette || (flags&ImagePallette))
				pal = draw::palt;
			else
				pal = (color*)e->offs(f.pallette);
			if(!pal)
				return;
			if(flags&ImageMirrorH)
				bop::rle832m(ptr(x2 - 1, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy),
					alpha, pal);
			else
				bop::rle832(ptr(x, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy),
					alpha, pal);
		}
		break;
	case sprite::RLE:
		if(bypp == 4)
		{
			if(flags&ImageMirrorH)
				bop::rle32m(ptr(x2 - 1, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy),
					alpha);
			else
				bop::rle32(ptr(x, sy), wd, s, y2 - y,
					ptr(clipping.x1, sy),
					ptr(clipping.x2, sy),
					alpha);
		}
		break;
	case sprite::ALC:
		if(bypp != 4)
			break;
		if(flags&TextBold)
			bop::alc32(ptr(x, sy - 1), wd, s, y2 - y,
				ptr(clipping.x1, sy - 1),
				ptr(clipping.x2, sy - 1),
				fore, (flags&TextItalic) != 0);
		if(flags&TextBold)
			bop::alc32(ptr(x, sy - 1), wd, s, y2 - y,
				ptr(clipping.x1, sy - 1),
				ptr(clipping.x2, sy - 1),
				fore, (flags&TextItalic) != 0);
		bop::alc32(ptr(x, sy), wd, s, y2 - y,
			ptr(clipping.x1, sy), ptr(clipping.x2, sy),
			fore, (flags&TextItalic) != 0);
		break;
	default:
		break;
	}
}

void draw::image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha, color* pal)
{
	draw::state push;
	draw::palt = pal;
	image(x, y, e, id, flags | ImagePallette, alpha);
}

void draw::stroke(int x, int y, const sprite* e, int id, int flags, unsigned char thin, unsigned char* koeff)
{
	color tr;
	tr.a = 0;
	tr.r = 255;
	tr.g = 255;
	tr.b = 255;
	auto fr = e->get(id);
	rect rc = fr.getrect(x, y, flags);
	draw::surface canvas(rc.width()+2, rc.height()+2, 32);
	x--; y--;
	if(true)
	{
		draw::state push;
		draw::canvas = &canvas;
		draw::setclip();
		draw::rectf({0, 0, canvas.width, canvas.height}, tr);
		draw::image(1, 1, e, id, ImageNoOffset);
	}
	for(int y1 = 0; y1 < canvas.height; y1++)
	{
		bool inside = false;
		for(int x1 = 0; x1 < canvas.width; x1++)
		{
			auto m = (color*)canvas.ptr(x1, y1);
			if(!inside)
			{
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, px--)
				{
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = true;
			}
			else
			{
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 2;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n<thin; n++, px++)
				{
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = false;
			}
		}
	}
	for(int x1 = 0; x1 < canvas.width; x1++)
	{
		bool inside = false;
		for(int y1 = 0; y1 < canvas.height; y1++)
		{
			auto m = (color*)canvas.ptr(x1, y1);
			if(!inside)
			{
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n<thin; n++, py--)
				{
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = true;
			}
			else
			{
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 2;
				for(auto n = 0; n<thin; n++, py++)
				{
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = false;
			}
		}
	}
}

static bool corrects(const draw::surface& dc, int& x, int& y, int& width, int& height)
{
	if(x + width > dc.width)
		width = dc.width - x;
	if(y + height > dc.height)
		height = dc.height - y;
	if(width <= 0 || height <= 0)
		return false;
	return true;
}

static bool correctb(int& x1, int& y1, int& w, int& h, int& ox)
{
	int x11 = x1;
	int x2 = x1 + w;
	int y2 = y1 + h;
	if(!correct(x1, y1, x2, y2, draw::clipping))
		return false;
	ox = x1 - x11;
	w = x2 - x1;
	h = y2 - y1;
	return true;
}

void draw::blit(surface& ds, int x1, int y1, int w, int h, unsigned flags, draw::surface& ss, int xs, int ys)
{
	if(ss.bpp != ds.bpp)
		return;
	int ox;
	if(!correctb(x1, y1, w, h, ox))
		return;
	if(ds.bpp == 8)
	{
		if(flags&ImageMirrorH)
		{
			if(flags&ImageTransparent)
				bop::raw8tm(
					ds.ptr(x1 + w - 1, y1), ds.scanline,
					ss.ptr(xs, ys) + ox, ss.scanline,
					w, h);
			else
				bop::raw8m(
					ds.ptr(x1 + w - 1, y1), ds.scanline,
					ss.ptr(xs, ys) + ox, ss.scanline,
					w, h);
		}
		else
		{
			if(flags&ImageTransparent)
				bop::raw8t(
					ds.ptr(x1, y1), ds.scanline,
					ss.ptr(xs, ys) + ox, ss.scanline,
					w, h);
			else
				bop::cpy(
					ds.ptr(x1, y1), ds.scanline,
					ss.ptr(xs, ys) + ox, ss.scanline,
					w, h, 1);
		}
	}
	else if(ds.bpp == 32)
	{
		if(flags&ImageTransparent)
			bop::cpy32t(
				ds.ptr(x1, y1), ds.scanline,
				ss.ptr(xs, ys) + ox * 4, ss.scanline,
				w, h);
		else
			bop::cpy(
				ds.ptr(x1, y1), ds.scanline,
				ss.ptr(xs, ys) + ox * 4, ss.scanline,
				w, h, 4);
	}
}

void draw::blit(surface& dest, int x, int y, int width, int height, unsigned flags, surface& source, int x_source, int y_source, int width_source, int height_source)
{
	if(width == width_source && height == height_source)
	{
		blit(dest, x, y, width, height, flags, source, x_source, y_source);
		return;
	}
	if(source.bpp != dest.bpp)
		return;
	if(!corrects(dest, x, y, width, height))
		return;
	if(!corrects(source, x_source, y_source, width_source, height_source))
		return;
	int ox;
	if(!correctb(x_source, y_source, width, height, ox))
		return;
	if(dest.bpp == 32)
	{
		bop::scale32(
			dest.ptr(x, y), dest.scanline, width, height,
			source.ptr(x_source, y_source) + ox * 4, source.scanline, width_source, height_source);
	}
}

void draw::initialize()
{
	set_light_theme();
	command_app_initialize->execute();
}