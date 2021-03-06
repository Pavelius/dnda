#include "draw.h"
#include "resources.h"
#include "main.h"

using namespace draw;

template<class T> static const T* findkey(const T* source, int key) {
	for(auto p = source; *p; p++) {
		if(p->key == key)
			return p;
	}
	return 0;
}

struct hotkey {
	unsigned		key;
	const char*		command;
	void(*proc)(creature& e, scene& sc);
	explicit operator bool() const { return key != 0; }
};

struct fxeffect : point {
	sprite*			res;
	short unsigned	frame;
	short unsigned	flags;
	unsigned char	alpha;
	point			end;
	fxeffect() : res(0), frame(0), flags(0), alpha(0xFF) {
		x = 0;
		y = 0;
		end = *this;
	}
	void direction() {
		int dx = iabs(x - end.x);
		int dy = iabs(y - end.y);
		if(dy < dx)
			frame++;
		if(dy < dx / 2)
			frame++;
		if(x > end.x)
			flags |= ImageMirrorH;
		if(end.y > y)
			flags |= ImageMirrorV;
	}
	void paint(point camera) {
		int x1 = x - camera.x;
		int y1 = y - camera.y;
		draw::image(x1, y1, res, frame, flags);
	}
};

static int			current_key_index;
const int			elx = 64;
const int			ely = 48;
const int			scrx = 32;
const int			scry = 32;
const int			mmaps = 5;
const unsigned char	dialog_alpha = 200;
const int			padding = 8;
static point		viewport;
static char			state_message[1024];
static const char*	p_message = state_message;
static hotkey*		gethotkey(int id);
static int			answers[9];
static bool			show_gui_panel = true;
static avec<const char*> messages;

namespace colors {
color				fow = color::create(10, 10, 10);
}

static bool itm_chargeable(scene& sc, sceneparam& e, item& v, bool run) {
	return v.ischargeable();
}

static bool itm_drinkable(scene& sc, sceneparam& e, item& v, bool run) {
	return v.isdrinkable();
}

static bool itm_edible(scene& sc, sceneparam& e, item& v, bool run) {
	return v.isedible();
}

static void clear_state() {
	state_message[0] = 0;
	p_message = state_message;
	current_key_index = 0;
}

void logs::add(const char* format, ...) {
	addv(format, xva_start(format));
}

void logs::addnc(const char* format, ...) {
	stringcreator sc;
	addvnc(sc, format, xva_start(format));
}

void logs::add(int id, const char* format, ...) {
	if(current_key_index >= sizeof(answers) / sizeof(answers[0]))
		return;
	stringcreator sc;
	answers[current_key_index] = id;
	addnc("\n[%1i)] ", current_key_index + 1);
	addvnc(sc, format, xva_start(format));
	current_key_index++;
	p_message = zend(state_message);
}

void logs::addvnc(stringcreator& sc, const char* format, const char* vl) {
	auto p = zend(state_message);
	if(p == state_message)
		format = zskipspcr(format);
	if(format[0] == 0)
		return;
	if(p != state_message) {
		if(p[-1] != ' ' && p[-1] != '\n' && p[-1] != '\"' && *format != '\n' && *format != '.' && *format != ',' && *format != '\"') {
			*p++ = ' ';
			*p = 0;
		}
		if(p[-1] == '\n' && format[0] == '\n')
			format++;
		if(p[-1] == ' ' && format[0] == ' ')
			format++;
	}
	sc.printv(p, state_message + sizeof(state_message) - 1, format, vl);
}

void logs::addv(stringcreator& sc, const char* format, const char* vl) {
	addvnc(sc, format, vl);
	messages.add(szdup(zskipspcr(p_message)));
	auto p = zend(state_message);
	if(p > (state_message + sizeof(state_message) - 260))
		logs::next();
	p_message = p;
}

void logs::addv(const char* format, const char* vl) {
	stringcreator sc;
	addv(sc, format, vl);
}

static direction_s getdirection(unsigned id) {
	switch(id) {
	case KeyDown: return Down;
	case KeyUp: return Up;
	case KeyLeft: return Left;
	case KeyRight: return Right;
	case KeyPageDown: return RightDown;
	case KeyPageUp: return RightUp;
	case KeyHome: return LeftUp;
	case KeyEnd: return LeftDown;
	default: return Center;
	}
}

static char* szweight(char* result, int value) {
	auto p1 = value / 100;
	auto p2 = value % 100;
	sznum(result, p1, 0, "0", 10);
	zcat(result, ".");
	sznum(zend(result), p2, 2, "00", 10);
	zcat(result, " ��");
	return result;
}

static char* szpercent(char* result, const char* result_maximum, int value) {
	return szprints(result, result_maximum, "%1i%%", value);
}

static int textr(int x, int y, int width, const char* value) {
	draw::text(x + width - draw::textw(value), y, value);
	return x + width;
}

static int textl(int x, int y, int width, const char* value) {
	draw::text(x, y, value);
	return x + width;
}

static int textf(int x, int y, int width, const char* value) {
	draw::textf(x, y, width, value);
	return x + width;
}

static int textl(int x, int y, int width, item& value) {
	char temp[260]; value.getname(temp, zendof(temp));
	draw::textf(x, y, width, temp);
	return x + width;
}

static int shortcutnb(int x, int y, int w, int key) {
	draw::state push;
	char temp[32];
	draw::fore = colors::green;
	draw::text(x, y, key2str(temp, key));
	return x + w;
}

static int shortcut(int x, int y, int w, int index) {
	draw::state push;
	char temp[32];
	char temx[32];
	draw::fore = colors::green;
	temp[0] = 'A' + index;
	temp[1] = 0;
	szprints(temx, zendof(temx), "%1)", temp);
	draw::text(x, y, temx);
	return x + w;
}

static int headel(int x, int y, int width, const char* value) {
	draw::state push;
	draw::fore = colors::yellow;
	return textl(x, y, width, value);
}

static int header(int x, int y, int width, const char* value) {
	draw::state push;
	draw::fore = colors::yellow;
	return textr(x, y, width, value);
}

static bool getkey(int& id, unsigned count) {
	unsigned result = 0;
	if(id >= (Alpha + 'A') && id <= (Alpha + 'Z'))
		result = id - (Alpha + 'A');
	else
		return false;
	if(result >= count)
		return false;
	id = result;
	return true;
}

static int fiela(int x, int y, int w, const char* name, int value, int basic_value) {
	char temp[128];
	draw::state push;
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	if(value < basic_value)
		draw::fore = colors::text.mix(colors::red, 96);
	else if(value > basic_value)
		draw::fore = colors::text.mix(colors::green, 64);
	draw::text(x + w, y, szprints(temp, zendof(temp), "%1i", value));
	return draw::texth();
}

static int field(int x, int y, int w, const char* name, int value) {
	char temp[128];
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	draw::text(x + w, y, szprints(temp, zendof(temp), "%1i", value));
	return draw::texth();
}

static int field(int x, int y, const char* value) {
	draw::text(x, y, value);
	return draw::texth();
}

static int field(int x, int y, int w, const char* name, const char* value) {
	char temp[128];
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	draw::text(x + w, y, value);
	return draw::texth();
}

static int field(int x, int y, int w, const char* name, int value, int max_value) {
	char temp[128];
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	draw::text(x + w, y, szprints(temp, zendof(temp), "%1i/%2i", value, max_value));
	return draw::texth();
}

static int fielp(int x, int y, int w, const char* name, int p1, int p2) {
	char temp[128];
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	draw::text(x + w, y, szprints(temp, zendof(temp), "%1i%% � %2i%%", chance_to_hit + p1, chance_to_hit + p2));
	return draw::texth();
}

static int fielr(int x, int y, int w, const char* name, int p1, int p2) {
	char temp[128];
	draw::text(x, y, szprints(temp, zendof(temp), "%1:", name));
	szprints(temp, zendof(temp), "%1i%%", p1);
	if(p2)
		szprints(zend(temp), zendof(temp), " � %1i", p2);
	draw::text(x + w, y, temp);
	return draw::texth();
}

static point getcamera(int mx, int my) {
	point pt;
	pt.x = mx * elx - viewport.x / 2;
	pt.y = my * ely - viewport.y / 2;
	if(pt.x < 0)
		pt.x = 0;
	if(pt.y < 0)
		pt.y = 0;
	return pt;
}

inline bool wget(short unsigned i, direction_s direction, tile_s value) {
	auto i1 = game::to(i, direction);
	if(i1 == Blocked)
		return true;
	return game::gettile(i1) == value;
}

inline bool wget(short unsigned i, direction_s direction, tile_s value, bool default_result) {
	auto i1 = game::to(i, direction);
	if(i1 == Blocked)
		return default_result;
	return game::gettile(i1) == value;
}

int mget(int ox, int oy, int mx, int my) {
	int x = mx - ox;
	int y = my - oy;
	if(x < 0 || y < 0 || x >= scrx || y >= scry)
		return -1;
	return y * scrx + x;
}

inline bool xget(short unsigned i, direction_s direction) {
	auto i1 = game::to(i, direction);
	if(i1 == Blocked)
		return false;
	return !game::is(i1, Explored);
}

static int getorder(item_s type) {
	switch(type) {
	case Sling: return 2;
	default: return 0;
	}
}

static int getaccindex(item_s type) {
	switch(type) {
	case BowLong: return 0;
	case BowShort: return 1;
	case Sling: return 2;
	case CrossbowLight: case CrossbowHeavy: return 3;
	default: return -1;
	}
}

static void view_avatar(int x, int y, class_s type, race_s race, gender_s gender, item& body, item& melee, item& offhand, item& ranged, item& back, short unsigned flags, unsigned char alpha) {
	const int FirstWeapon = AxeBattle;
	int i1 = 0;
	int i2 = 1;
	if(melee.istwohanded()) {
		i1 = 2;
		i2 = 3 + melee.gettype() - FirstWeapon;
	} else {
		if(melee.gettype())
			i1 = 3 + melee.gettype() - FirstWeapon;
		if(offhand.gettype() == Shield)
			i2 = 3 + (SwordTwoHanded - FirstWeapon + 1) + (SwordTwoHanded - FirstWeapon + 1);
		else if(offhand.gettype())
			i2 = 3 + (SwordTwoHanded - FirstWeapon + 1) + (offhand.gettype() - FirstWeapon);
	}
	auto at = 2;
	switch(body.gettype()) {
	case NoItem:
		if(type == Fighter || type == Theif)
			at = 1;
		else
			at = 0;
		break;
	case LeatherArmour:
	case StuddedLeatherArmour:
		at = 1;
		break;
	}
	// Cloack
	if(back) {
		switch(race) {
		case Dwarf:
		case Halfling:
			draw::image(x, y, gres(ResPCmac), 5, flags, alpha);
			break;
		default:
			draw::image(x, y, gres(ResPCmac), 4, flags, alpha);
			break;
		}
	}
	// Weapon on back
	if(ranged && getorder(ranged.gettype()) == 0) {
		auto index = getaccindex(ranged.gettype());
		if(index != -1)
			draw::image(x, y, gres(ResPCmac), index, flags, alpha);
	}
	// Character body
	draw::image(x, y, gres(ResPCmar), i1, flags, alpha);
	draw::image(x, y, gres(ResPCmbd), (race - Human) * 6 + (gender - Male) * 3 + at, flags, alpha);
	// Weapon on belt
	if(ranged && getorder(ranged.gettype()) == 2) {
		auto index = getaccindex(ranged.gettype());
		if(index != -1)
			draw::image(x, y, gres(ResPCmac), index, flags, alpha);
	}
	// Hand
	draw::image(x, y, gres(ResPCmar), i2, flags, alpha);
}

static void view_dialog(rect rc) {
	rc.offset(-padding);
	draw::rectf(rc, colors::black, dialog_alpha);
	draw::rectb(rc, colors::border);
}

static int view_dialog(rect rc, const char* title) {
	draw::state push;
	auto x = rc.x1;
	auto y = rc.y1;
	auto w = rc.width();
	rc.offset(-padding);
	view_dialog(rc);
	draw::font = metrics::h2;
	draw::fore = colors::yellow;
	draw::text(x + (w - draw::textw(title)) / 2, y, title);
	return draw::texth() + padding;
}

static void view_world(point camera, bool show_fow = true, fxeffect* effects = 0) {
	sprite* plain = gres(ResPlains);
	sprite* hills = gres(ResFoothills);
	sprite* mount = gres(ResMountains);
	sprite* tmount = gres(ResCloudPeaks);
	sprite* sea = gres(ResSea);
	sprite* decals = gres(ResDecals);
	viewport.x = draw::getwidth();
	viewport.y = draw::getheight();
	auto mmx = max_map_x * elx;
	auto mmy = max_map_y * ely;
	if(camera.x < 0)
		camera.x = 0;
	if(camera.y < 0)
		camera.y = 0;
	if(camera.x > mmx - viewport.x)
		camera.x = mmx - viewport.x;
	if(camera.y > mmy - viewport.y)
		camera.y = mmy - viewport.y;
	camera.x -= elx / 2;
	camera.y -= ely / 2;
	rect rc;
	rc.x1 = camera.x / elx;
	rc.y1 = camera.y / ely;
	rc.x2 = rc.x1 + viewport.x / elx + 2;
	rc.y2 = rc.y1 + viewport.y / ely + 3;
	int x0 = 0;
	int y0 = 0;
	// ������ �������
	for(auto my = rc.y1; my <= rc.y2; my++) {
		if(my >= max_map_y)
			break;
		for(auto mx = rc.x1; mx <= rc.x2; mx++) {
			if(mx >= max_map_x)
				continue;
			auto x = x0 + mx * elx - camera.x;
			auto y = y0 + my * ely - camera.y;
			auto i = game::get(mx, my);
			auto t = game::gettile(i);
			auto r = game::getrand(i) % 4;
			switch(t) {
			case Plain:
			case Mountains:
			case Forest:
			case Swamp:
				draw::image(x, y, plain, r, 0);
				break;
			case Sea:
				draw::image(x, y, sea, game::getindex(i, Sea), 0);
				break;
			case Foothills:
				draw::image(x, y, plain, r, 0);
				draw::image(x, y, hills, game::getindex(i, Foothills), 0);
				break;
			}
		}
	}
	// ������� �������
	for(auto my = rc.y1; my <= rc.y2; my++) {
		if(my >= max_map_y)
			break;
		for(auto mx = rc.x1; mx <= rc.x2; mx++) {
			if(mx >= max_map_x)
				continue;
			auto x = x0 + mx * elx - camera.x;
			auto y = y0 + my * ely - camera.y;
			auto i = game::get(mx, my);
			auto t = game::gettile(i);
			auto r = game::getrand(i) % 4;
			switch(t) {
			case Mountains:
				draw::image(x, y, mount, game::getindex(i, Mountains), 0);
				break;
			case CloudPeaks:
				draw::image(x, y, tmount, game::getindex(i, CloudPeaks), 0);
				break;
			case Forest:
				draw::image(x, y, decals, 0 + (game::getrand(i) % 3), 0);
				break;
			case Swamp:
				draw::image(x, y, decals, 3 + (game::getrand(i) % 3), 0);
				break;
			}
		}
	}
	// ������ ������� ��������
	if(effects) {
		for(auto p = effects; p->res; p++)
			p->paint(camera);
	}
}

static void view_board(point camera, bool show_fow = true, fxeffect* effects = 0) {
	creature* units[scrx*scry];
	item_s stuff[scrx*scry];
	auto night_percent = 0;
	auto is_dungeon = game::isdungeon();
	sprite *floor, *walls;
	if(is_dungeon) {
		floor = gres(ResDungeon);
		walls = gres(ResDungeonW);
	} else {
		night_percent = (128 * game::getnight()) / 100;
		floor = gres(ResGrass);
		walls = gres(ResGrassW);
	}
	auto shadow = gres(ResShadow);
	viewport.x = draw::getwidth();
	viewport.y = draw::getheight();
	auto mmx = max_map_x * elx;
	auto mmy = max_map_y * ely;
	if(camera.x < 0)
		camera.x = 0;
	if(camera.y < 0)
		camera.y = 0;
	if(camera.x > mmx - viewport.x)
		camera.x = mmx - viewport.x;
	if(camera.y > mmy - viewport.y)
		camera.y = mmy - viewport.y;
	camera.x -= elx / 2;
	camera.y -= ely / 2;
	rect rc;
	rc.x1 = camera.x / elx;
	rc.y1 = camera.y / ely;
	rc.x2 = rc.x1 + viewport.x / elx + 2;
	rc.y2 = rc.y1 + viewport.y / ely + 3;
	int x0 = 0;
	int y0 = 0;
	// �������������� ������� �����
	memset(units, 0, sizeof(units));
	creature::select(units, rc);
	// �������������� ������� ��������
	memset(stuff, 0, sizeof(stuff));
	for(auto& e : grounditems) {
		if(!e)
			continue;
		auto i = mget(rc.x1, rc.y1, game::getx(e.index), game::gety(e.index));
		if(i == -1)
			continue;
		if(stuff[i] == NoItem)
			stuff[i] = e.gettype();
		else
			stuff[i] = ManyItems;
	}
	// ������ �������
	for(auto my = rc.y1; my <= rc.y2; my++) {
		if(my >= max_map_y)
			break;
		for(auto mx = rc.x1; mx <= rc.x2; mx++) {
			if(mx >= max_map_x)
				continue;
			auto x = x0 + mx * elx - camera.x;
			auto y = y0 + my * ely - camera.y;
			auto i = game::get(mx, my);
			auto t = game::gettile(i);
			auto o = game::getobject(i);
			auto r = game::getrand(i) % 4;
			auto can_be_shadow = false;
			// �����
			switch(t) {
			case Hill:
				draw::image(x, y, floor, r, 0);
				draw::image(x, y, gres(ResFeature), 2, 0);
				can_be_shadow = true;
				break;
			case Swamp:
				draw::image(x, y, floor, r, 0);
				draw::image(x, y, gres(ResFeature), 3, 0);
				can_be_shadow = true;
				break;
			case Floor:
				if(is_dungeon)
					draw::image(x, y, floor, r, 0);
				else
					draw::image(x, y, floor, 4, 0);
				can_be_shadow = true;
				break;
			case Plain:
				draw::image(x, y, floor, r, 0);
				can_be_shadow = true;
				break;
			case Wall:
				draw::image(x, y, walls, 0, 0);
				if(wget(i, Down, Wall)) {
					if(wget(i, Right, Wall)) {
						if(!wget(i, RightDown, Wall))
							draw::image(x, y, walls, 10, 0);
					} else if(wget(i, RightDown, Wall))
						draw::image(x, y, walls, 12, 0);
					else
						draw::image(x, y, walls, 2, 0);
					if(wget(i, Left, Wall)) {
						if(!wget(i, LeftDown, Wall))
							draw::image(x, y, walls, 9, 0);
					} else if(wget(i, LeftDown, Wall))
						draw::image(x, y, walls, 11, 0);
					else
						draw::image(x, y, walls, 4, 0);
				} else {
					if(is_dungeon) {
						switch(i % 3) {
						case 0:
							draw::image(x, y, walls, 13, 0);
							break;
						case 1:
							draw::image(x, y, walls, 14, 0);
							break;
						default:
							draw::image(x, y, walls, 1, 0);
							break;
						}
					} else
						draw::image(x, y, walls, 1, 0);
					if(!wget(i, Left, Wall))
						draw::image(x, y, walls, 8, 0);
					if(!wget(i, Right, Wall))
						draw::image(x, y, walls, 7, 0);
				}
				break;
			case Water:
				draw::image(x, y, gres(ResWater), game::getindex(i, Water), 0);
				can_be_shadow = true;
				break;
			case Road:
				draw::image(x, y, gres(ResRoad), game::getindex(i, Road), 0);
				break;
			case NoTile:
				draw::rectf({x - elx / 2, y - ely / 2, x + elx / 2, y + ely / 2}, colors::black);
				//draw::rectb({x-elx/2, y-ely/2, x + elx/2, y + ely/2}, colors::black);
				break;
			}
			// ������� �� �����
			switch(o) {
			case Trap:
				if(!game::is(i, Hidden))
					draw::image(x, y, gres(ResFeature), 59 + game::gettrap(i) - TrapAnimal, 0);
				break;
			}
			// �������� �� �����
			auto pi = mget(rc.x1, rc.y1, mx, my);
			if(pi != -1) {
				switch(stuff[pi]) {
				case NoItem:
					break;
				case ManyItems:
					draw::image(x, y - 8, gres(ResItems), 0, 0);
					break;
				default:
					draw::image(x, y - 8, gres(ResItems), stuff[pi], 0);
					break;
				}
			}
			// ����
			if(can_be_shadow) {
				bool uw = game::gettile(game::to(i, Up)) == Wall;
				bool rw = game::gettile(game::to(i, Right)) == Wall;
				bool dw = game::gettile(game::to(i, Down)) == Wall;
				bool lw = game::gettile(game::to(i, Left)) == Wall;
				if(uw)
					draw::image(x, y, shadow, 0, 0);
				if(lw)
					draw::image(x, y, shadow, 1, 0);
				if(dw)
					draw::image(x, y, shadow, 2, 0);
				if(rw)
					draw::image(x, y, shadow, 3, 0);
				if(!uw && !rw && game::gettile(game::to(i, RightUp)) == Wall)
					draw::image(x, y, shadow, 4, 0);
				if(!uw && !lw && game::gettile(game::to(i, LeftUp)) == Wall)
					draw::image(x, y, shadow, 5, 0);
				if(!dw && !rw && game::gettile(game::to(i, RightDown)) == Wall)
					draw::image(x, y, shadow, 6, 0);
				if(!dw && !lw && game::gettile(game::to(i, LeftDown)) == Wall)
					draw::image(x, y, shadow, 7, 0);
			}
		}
	}
	// ������ ������� ��������
	if(effects) {
		for(auto p = effects; p->res; p++)
			p->paint(camera);
	}
	// ������� �������
	for(auto my = rc.y1; my <= rc.y2; my++) {
		if(my >= max_map_y)
			break;
		for(auto mx = rc.x1; mx <= rc.x2; mx++) {
			if(mx >= max_map_x)
				continue;
			auto x = x0 + mx * elx - camera.x;
			auto y = y0 + my * ely - camera.y;
			auto i = game::get(mx, my);
			auto t = game::getobject(i);
			auto r = game::getrand(i);
			switch(t) {
			case Tree:
				draw::image(x, y, gres(ResFeature), 4 + r % 3, 0);
				break;
			case StairsDown:
				draw::image(x, y, gres(ResFeature), 56, 0);
				break;
			case StairsUp:
				draw::image(x, y, gres(ResFeature), 57, 0);
				break;
			case Door:
				r = game::is(i, Opened) ? 0 : 1;
				if(game::gettile(game::to(i, Left)) == Wall && game::gettile(game::to(i, Right)) == Wall)
					draw::image(x, y + 16, gres(ResDoors), r, 0);
				else if(game::gettile(game::to(i, Up)) == Wall && game::gettile(game::to(i, Down)) == Wall)
					draw::image(x, y, gres(ResDoors), 2 + r, 0);
				break;
			}
			// ����� ������
			int i1 = mget(rc.x1, rc.y1, mx, my);
			if(i1 != -1) {
				auto pc = units[i1];
				if(pc) {
					unsigned flags;
					switch(pc->getdirection()) {
					case Left:
					case LeftUp:
					case LeftDown:
						flags = ImageMirrorH;
						break;
					default:
						flags = 0;
						break;
					}
					// ���������� ����� ������ ����� ��� ������ ������
					unsigned char alpha = 0xFF;
					if(pc->is(Hiding))
						alpha = 0x80;
					if(pc->ischaracter()) {
						view_avatar(x, y, pc->getclass(), pc->getrace(), pc->getgender(),
							pc->wears[Torso], pc->wears[Melee], pc->wears[OffHand],
							pc->wears[Ranged], pc->wears[TorsoBack], flags, alpha);
					} else
						draw::image(x, y, gres(ResMonsters), pc->getrole(), flags, alpha);
				}
			}
			// ���������� �����
			if(game::gettile(i) != Wall) {
				// � ������� ����� ������ ���������� ������ � ����������
				if(wget(i, Down, Wall, is_dungeon)) {
					draw::image(x, y, walls, 3, 0);
					if(!wget(i, Left, Wall) && !wget(i, LeftDown, Wall))
						draw::image(x, y, walls, 6, 0);
					if(!wget(i, Right, Wall) && !wget(i, RightDown, Wall))
						draw::image(x, y, walls, 5, 0);
				}
			}
		}
	}
	// ������ �����
	if(night_percent)
		draw::rectf({0, 0, draw::getwidth(), draw::getheight()}, color::create(0, 0, 64), night_percent);
	// Show fog of war
	if(show_fow) {
		for(auto my = rc.y1; my <= rc.y2; my++) {
			if(my >= max_map_y)
				break;
			for(auto mx = rc.x1; mx <= rc.x2; mx++) {
				if(mx >= max_map_x)
					continue;
				auto x = x0 + mx * elx - camera.x;
				auto y = y0 + my * ely - camera.y - ely / 2;
				auto i = game::get(mx, my);
				if(!game::is(i, Explored))
					draw::rectf({x - elx / 2, y - ely / 2, x + elx / 2, y + ely / 2}, colors::fow);
				else {
					bool dw = xget(i, Down);
					bool up = xget(i, Up);
					bool rg = xget(i, Right);
					bool lf = xget(i, Left);
					if(dw)
						draw::image(x, y, gres(ResFog), 2, 0);
					if(up)
						draw::image(x, y, gres(ResFog), 2, ImageMirrorV);
					if(lf)
						draw::image(x, y, gres(ResFog), 1, 0);
					if(rg)
						draw::image(x, y, gres(ResFog), 1, ImageMirrorH);
					if(!lf && !up && xget(i, LeftUp))
						draw::image(x, y, gres(ResFog), 0, ImageMirrorV);
					if(!lf && !dw && xget(i, LeftDown))
						draw::image(x, y, gres(ResFog), 0, 0);
					if(!rg && !up && xget(i, RightUp))
						draw::image(x, y, gres(ResFog), 0, ImageMirrorV | ImageMirrorH);
					if(!rg && !dw && xget(i, RightDown))
						draw::image(x, y, gres(ResFog), 0, ImageMirrorH);
				}
			}
		}
	}
}

static void view_message() {
	if(!state_message[0])
		return;
	rect rc = {0, 0, 400, 800};
	draw::textf(rc, state_message);
	auto width = rc.width();
	auto height = rc.height();
	rc.x1 = (draw::getwidth() - width) / 2;
	rc.y1 = padding * 2;
	rc.x2 = rc.x1 + width;
	rc.y2 = rc.y1 + height;
	view_dialog(rc);
	draw::textf(rc.x1, rc.y1, width, state_message);
}

static int texth(int x, int y, const char* text, char level) {
	static color color_level[] = {colors::text, colors::yellow};
	draw::state push;
	if(level > 0)
		draw::fore = color_level[level];
	draw::text(x, y, text);
	return draw::textw(text) + 4;
}

static void view_info(const creature& e) {
	if(!show_gui_panel)
		return;
	char temp[512];
	const int tw = 26;
	const int dx = 52;
	const int width = 500;
	const int height = draw::texth() * 4;
	auto x = (draw::getwidth() - width) / 2;
	auto y = draw::getheight() - height - padding * 2;
	auto y2 = y;
	draw::state push;
	draw::fore = colors::white;
	view_dialog({x, y, x + width, y + height});
	draw::textf(x, y, width, e.getfullname(temp, zendof(temp), true, true));
	auto loc = e.getsite();
	if(loc) {
		auto p = loc->getname(temp);
		draw::text(x + width - draw::textw(p), y, p);
	}
	y += draw::texth();
	int y1 = y;
	int x1 = x;
	y += fiela(x, y, tw, "��", e.get(Strenght), e.getbasic(Strenght));
	y += fiela(x, y, tw, "��", e.get(Intellegence), e.getbasic(Intellegence));
	x += dx;
	y = y1;
	y += fiela(x, y, tw, "��", e.get(Dexterity), e.getbasic(Dexterity));
	y += fiela(x, y, tw, "��", e.get(Wisdow), e.getbasic(Wisdow));
	x += dx;
	y = y1;
	y += fiela(x, y, tw, "��", e.get(Constitution), e.getbasic(Constitution));
	y += fiela(x, y, tw, "��", e.get(Charisma), e.getbasic(Charisma));
	x += dx + 6;
	y = y1;
	y += fielp(x, y, tw, "��", e.getattackinfo(Melee).bonus, e.getattackinfo(Ranged).bonus);
	y += fielr(x, y, tw, "��", e.getdefence(), e.getarmor());
	x += dx + 50;
	y = y1;
	y += field(x, y, 40, "����", e.gethits(), e.getmaxhits());
	y += field(x, y, 40, "����", e.getmana(), e.getmaxmana());
	x += dx + 40 + 20 - tw;
	y = y1;
	y += field(x, y, 52, "����", e.getexperience());
	//y += field(x, y, 52, "�����", getstrfdat(temp, segments));
	y += field(x, y, 52, "������", e.getmoney());
	x += dx + 58;
	x = x1;
	y = y1 + draw::texth() * 2;
	// Draw encumbrance
	switch(e.getencumbrance()) {
	case Encumbered: x += texth(x, y, "��������", 0); break;
	case HeavilyEncumbered: x += texth(x, y, "��������", 1); break;
	}
	// Draw status
	for(auto i = Anger; i <= LastState; i = (state_s)(i + 1)) {
		switch(i) {
		case Poisoned:
		case PoisonedStrong:
			continue;
		case PoisonedWeak:
			if(!e.is(PoisonedWeak) && !e.is(Poisoned) && !e.is(PoisonedStrong))
				continue;
			break;
		default:
			if(!e.is(i))
				continue;
			break;
		}
		auto pt = getstr(i);
		draw::text(x, y, pt);
		x += draw::textw(pt) + 4;
	}
}

static void view_zone(const creature* p, point camera, fxeffect* effects = 0) {
	view_board(camera, true, effects);
	view_message();
	if(p)
		view_info(*p);
}

static void pixel(int x, int y, color c1) {
	color* p = (color*)draw::ptr(x, y);
	for(int j = 0; j < mmaps; j++) {
		for(int i = 0; i < mmaps; i++)
			p[i] = c1;
		p += draw::canvas->scanline / sizeof(color);
	}
}

static void view_mini(int x, int y, point camera) {
	if(x < 8)
		x = 8;
	if(y < 64)
		y = 64;
	int y3 = y;
	auto is_dungeon = game::isdungeon();
	color floor = colors::gray;
	if(!is_dungeon)
		floor = color::create(51, 140, 29);
	color floor1 = floor.darken();
	color object = colors::gray;
	if(!is_dungeon)
		object = floor1.darken();
	color wall = colors::black.lighten();
	color door = colors::red;
	color stairs = colors::red.darken().darken();
	color water = colors::blue;
	color road = color::create(94, 70, 51).mix(floor, 192);
	color border = color::create(128, 128, 128);
	for(int y1 = 0; y1 < max_map_y; y1++) {
		int x3 = x;
		for(int x1 = 0; x1 < max_map_x; x1++, x3 += mmaps) {
			auto i = game::get(x1, y1);
			if(!game::is(i, Explored))
				continue;
			switch(game::gettile(i)) {
			case Hill:
			case Foothills:
			case Swamp:
				pixel(x3, y3, object);
				break;
			case Plain:
				pixel(x3, y3, floor);
				break;
			case Floor:
				pixel(x3, y3, floor1);
				break;
			case Water:
			case Sea:
				pixel(x3, y3, water);
				break;
			case Wall:
			case CloudPeaks:
				pixel(x3, y3, wall);
				break;
			case Road:
				pixel(x3, y3, road);
				break;
			}
			switch(game::getobject(i)) {
			case Door:
				pixel(x3, y3, door);
				break;
			case Tree:
				pixel(x3, y3, object);
				break;
			case StairsDown:
			case StairsUp:
				pixel(x3, y3, stairs);
				break;
			}
		}
		y3 += mmaps;
	}
	int mmx = max_map_x * mmaps;
	int mmy = max_map_y * mmaps;
	int scx = max_map_x * elx;
	int scy = max_map_y * ely;
	int xs1 = camera.x;
	int ys1 = camera.y;
	if(xs1 < 0)
		xs1 = 0;
	if(ys1 < 0)
		ys1 = 0;
	if(xs1 + viewport.x > scx)
		xs1 = scx - viewport.x;
	if(ys1 + viewport.y > scy)
		ys1 = scy - viewport.y;
	draw::rectb(
		{x + xs1 * mmx / scx, y + ys1 * mmy / scy,
		x + (xs1 + viewport.x)*mmx / scx, y + (ys1 + viewport.y)*mmy / scy},
		border);
	//view_legends(x, y, map::size.x*mmaps + metrics::padding);
}

static int view_total(int x, int y, int width, item** source, unsigned count, const creature* player) {
	char temx[64];
	char temp[1024];
	auto result = 0;
	auto rcount = 0;
	for(unsigned i = 0; i < count; i++) {
		result += source[i]->getweight();
		rcount++;
	}
	if(rcount) {
		szprints(temp, zendof(temp), "����� %1.", szweight(temx, result));
		if(player) {
			szprints(zend(temp), zendof(temp), " ��� ����� ��� ������ [%1].", szweight(temx, player->getweight()));
			szprints(zend(temp), zendof(temp), " ������ ��������� ��� %1", szweight(temx, player->getweight(Encumbered)));
			szprints(zend(temp), zendof(temp), " � ������ ��������� ��� %1.", szweight(temx, player->getweight(HeavilyEncumbered)));
		}
	} else
		szprints(temp, zendof(temp), "��� ���������� ���������.");
	return draw::textf(x, y, width, temp);
}

item* logs::choose(const creature& e, item** source, unsigned count, const char* title) {
	const int width = 500;
	const int height = 360;
	const int dy = 20;
	if(!title)
		title = "��������� �������";
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, title);
		for(unsigned i = 0; i < count; i++) {
			char temp[260];
			auto x1 = shortcut(x, y, 32, i);
			x1 = textl(x1, y, 408, *source[i]);
			x1 = textr(x1, y, 60, szweight(temp, source[i]->getweight()));
			y += dy;
		}
		y += padding;
		y += view_total(x, y, width, source, count, &e);
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return 0;
		default:
			if(getkey(hot::key, count))
				return source[hot::key];
			break;
		}
	}
	return 0;
}

static void character_pickup(creature& e, scene& sc) {
	char temp[260]; szprints(temp, zendof(temp), "������� ��������");
	item* source[48];
	auto index = e.getposition();
	auto p = source;
	for(auto& s : grounditems) {
		if(s.index != index)
			continue;
		if(!s)
			continue;
		*p++ = &s;
	}
	*p = 0;
	auto count = zlen(source);
	if(!count)
		return;
	auto result = logs::choose(e, source, count, temp);
	if(!result)
		return;
	e.pickup(*result);
}

static void character_dropdown(creature& e, scene& sc) {
	char temp[260]; szprints(temp, zendof(temp), "�������� ��������");
	item* source[LastBackpack + 1];
	auto p = source;
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(!e.wears[slot])
			continue;
		*p++ = &e.wears[slot];
	}
	item* result = logs::choose(e, source, p - source, temp);
	if(!result)
		return;
	e.dropdown(*result);
}

static void character_stuff(creature& e, scene& sc) {
	item* source[48];
	auto p = source;
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(!e.wears[slot])
			continue;
		*p++ = &e.wears[slot];
	}
	logs::choose(e, source, p - source, "������");
}

static item* choose_item(creature& e, point camera, slot_s slot) {
	char temp[260]; szprints(temp, zendof(temp), "��� ����� �� %1?", getstr(slot));
	item* source[LastBackpack + 1];
	auto p = source;
	for(auto i = FirstBackpack; i <= LastBackpack; i = (slot_s)(i + 1)) {
		if(!e.wears[i] || !e.wears[i].is(slot))
			continue;
		*p++ = &e.wears[i];
	}
	return logs::choose(e, source, p - source, temp);
}

static void character_invertory(creature& e, scene& sc) {
	const int width = 500;
	const int height = 360;
	const int dy = 20;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "����������");
		item* source[sizeof(e.wears) / sizeof(e.wears[0]) + 2];
		auto p = source;
		for(auto i = Head; i <= Amunitions; i = (slot_s)(i + 1)) {
			char temp[260];
			auto x1 = shortcut(x, y, 32, i);
			x1 = textl(x1, y, 108, szprints(temp, zendof(temp), "%1:", getstr(i)));
			if(e.wears[i]) {
				x1 = textl(x1, y, 300, e.wears[i]);
				x1 = textr(x1, y, 60, szweight(temp, e.wears[i].getweight()));
				*p++ = &e.wears[i];
			} else {
				x1 = textl(x1, y, 300, "-");
			}
			y += dy;
		}
		y += padding;
		y += view_total(x, y, width, source, p - source, &e);
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return;
		default:
			if(getkey(hot::key, Amunitions + 1)) {
				clear_state();
				auto slot = (slot_s)hot::key;
				if(e.wears[slot]) {
					e.unequip(e.wears[slot]);
				} else {
					auto p = choose_item(e, camera, slot);
					if(p) {
						auto it = *p;
						p->clear();
						e.equip(slot, it);
					}
				}
			}
			break;
		}
	}
}

short unsigned logs::choose(const creature& e, short unsigned* source, int count) {
	if(!count)
		return Blocked;
	else if(count == 1)
		return source[0];
	int current = 0;
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	while(ismodal()) {
		clear_state();
		short unsigned current_index = source[current];
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		game::looktarget(current_index);
		view_zone(&e, camera, ef);
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			clear_state();
			return Blocked;
		case KeyEnter:
			clear_state();
			return current_index;
		case KeyLeft:
			current--;
			break;
		case KeyRight:
			current++;
			break;
		}
		if(current >= count)
			current = 0;
		if(current < 0)
			current = count - 1;
	}
	return Blocked;
}

static short unsigned choose_target(creature& e, short unsigned current_index) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	while(ismodal()) {
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		view_zone(&e, camera, ef);
		draw::domodal();
		auto phk = gethotkey(hot::key);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(current_index, direction);
				if(ni != Blocked)
					current_index = ni;
				continue;
			}
		}
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return Blocked;
		case KeyEnter:
			return current_index;
		}
	}
	return Blocked;
}

int compare_skills(const void* p1, const void* p2) {
	auto e1 = *((skill_s*)p1);
	auto e2 = *((skill_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

void logs::raise(creature& e, int left) {
	skill_s source_data[LastSkill + 1];
	char source_checks[LastSkill + 1]; memset(source_checks, 2, sizeof(source_checks));
	aref<skill_s> source;
	auto pb = source_data;
	for(auto i = Bargaining; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(e.getbasic(i))
			*pb++ = i;
	}
	source.data = source_data;
	source.count = pb - source.data;
	if(!source)
		return;
	qsort(source.data, source.count, sizeof(source.data[0]), compare_skills);
	char temp[260];
	const int width = 400;
	const int height = 360;
	const int dy = 20;
	unsigned real_count = 0;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, szprints(temp, zendof(temp), "��������� ������� (�������� %1i)", left));
		auto index = 0;
		auto x1 = x + 32;
		x1 = headel(x1, y, 250, "��������");
		x1 = headel(x + 300, y, 30, "����");
		y += dy;
		auto old_fore = draw::fore;
		for(auto i : source) {
			x1 = shortcut(x, y, 32, index++);
			auto dice = e.getraise(i);
			x1 = textl(x1, y, 250, getstr(i));
			x1 = textr(x + 300, y, 30, szpercent(temp, zendof(temp), e.get(i)));
			if(source_checks[i] > 0)
				x1 = textl(x + 335, y, 40, szprints(temp, zendof(temp), "+ %1i-%2i%%", dice.min, dice.max));
			y += dy;
			real_count++;
		}
		draw::fore = old_fore;
		if(!index)
			textl(x, y, width, "� ��� ��� ���������� �������");
		draw::domodal();
		if(getkey(hot::key, real_count)) {
			if(source_checks[source[hot::key]]) {
				source_checks[source[hot::key]]--;
				e.raise(source[hot::key]);
				if(--left <= 0)
					return;
			}
		}
	}
}

bool logs::choose(creature& e, skill_s& result, aref<skill_s> source, bool can_escape) {
	char temp[260];
	const int width = 400;
	const int height = 360;
	const int dy = 20;
	unsigned real_count = 0;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "������");
		auto index = 0;
		for(auto i : source) {
			auto x1 = shortcut(x, y, 32, index++);
			x1 = textl(x1, y, 308, getstr(i));
			x1 = textr(x + 340, y, 60, szpercent(temp, zendof(temp), e.get(i)));
			y += dy;
			real_count++;
		}
		if(!index)
			textl(x, y, width, "� ��� ��� ���������� �������");
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			if(can_escape)
				return false;
			break;
		default:
			if(getkey(hot::key, real_count)) {
				result = source[hot::key];
				return true;
			}
			break;
		}
	}
	return false;
}

bool logs::choose(creature& e, skill_s& result, bool can_escape) {
	unsigned count = 0;
	skill_s source[TwoWeaponFighting + 1];
	for(auto i = Bargaining; i <= TwoWeaponFighting; i = (skill_s)(i + 1)) {
		if(e.getbasic(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare_skills);
	return choose(e, result, {source, count}, can_escape);
}

bool logs::choose(creature& e, spell_s& result, aref<spell_s> source) {
	char temp[260];
	const int width = 400;
	const int height = 360;
	const int dy = 20;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "����������");
		auto x1 = x + 32;
		x1 = headel(x1, y, 240, "��������");
		x1 = header(x1, y, 60, "�������");
		x1 = header(x1, y, 60, "����");
		y += dy;
		unsigned index = 0;
		for(auto i : source) {
			if(e.get(i) <= 0)
				continue;
			auto x1 = shortcut(x, y, 32, index++);
			x1 = textl(x1, y, 240, szprints(temp, zendof(temp), getstr(i)));
			x1 = textr(x1, y, 60, szprints(temp, zendof(temp), "%1i", e.get(i)));
			x1 = textr(x1, y, 60, szprints(temp, zendof(temp), "%1i", e.getcost(i)));
			y += dy;
		}
		if(!index)
			textl(x, y, width, "� ��� ��� ����������");
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return false;
		default:
			if(getkey(hot::key, index)) {
				result = source[hot::key];
				return true;
			}
			break;
		}
	}
	return false;
}

static void setplayer(creature& e, int index) {
	auto p = creature::getplayer(index);
	if(p)
		p->setplayer();
}

static void character_setplayer1(creature& e, scene& sc) {
	setplayer(e, 0);
}

static void character_setplayer2(creature& e, scene& sc) {
	setplayer(e, 1);
}

static void character_setplayer3(creature& e, scene& sc) {
	setplayer(e, 2);
}

static void character_setplayer4(creature& e, scene& sc) {
	setplayer(e, 3);
}

static void character_skill(creature& e, scene& sc) {
	skill_s result;
	if(!logs::choose(e, result))
		return;
	e.use(sc, result);
}

static void character_spell(creature& e, scene& sc) {
	spell_s result;
	if(!logs::choose(e, result))
		return;
	e.use(sc, result);
}

static void character_time(creature& e, scene& sc) {
	char temp[260];
	game::getdate(temp, zendof(temp), game::getseconds(), true);
	logs::add("������ %1.", temp);
}

static void dungeon_lookaround(creature& e, scene& sc) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	ef[0].frame = 1;
	short unsigned current_index = e.getposition();
	while(ismodal()) {
		clear_state();
		game::lookhere(current_index);
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		view_zone(&e, camera, ef);
		draw::domodal();
		auto phk = gethotkey(hot::key);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(current_index, direction);
				if(ni != Blocked)
					current_index = ni;
				continue;
			}
		}
		switch(hot::key) {
		case KeyEscape:
		case KeyEnter:
			return;
		}
	}
}

void logs::minimap(short unsigned position) {
	char temp[128]; temp[0] = 0;
	int w = max_map_x * mmaps + 280;
	int h = max_map_y * mmaps;
	point camera = getcamera(game::getx(position), game::gety(position));
	while(ismodal()) {
		draw::rectf({0, 0, draw::getwidth(), draw::getheight()}, colors::form);
		if(game::statistic.level)
			szprints(temp, zendof(temp), "������� %1i", game::statistic.level);
		//view_dialog(bsgets(Minimap, Name), temp, 1);
		view_mini((draw::getwidth() - w) / 2, (draw::getheight() - h) / 2, camera);
		draw::domodal();
		switch(hot::key) {
		case 0:
		case KeyEscape:
		case KeySpace:
			return;
		}
	}
}

static void character_chat(creature& e, scene& sc) {
	creature* creature_data[256];
	auto creatures = e.getcreatures(creature_data, e.getposition(), 1);
	auto source = e.getcreatures(creature_data, e.getposition(), 1);
	auto opponent = e.choose(source, true);
	if(opponent)
		e.chat(opponent);
}

static void character_use(creature& e, scene& sc) {
	//short unsigned source_data[3 * 3 + 1];
	//auto source = e.select(source_data, TargetObject, 1, e.getposition());
	//if(!source) {
	//	logs::add("������ ��� ���� ������, ��� ����� ���� �� ������������.");
	//	return;
	//}
	//e.manipulate(logs::choose(e, source.data, source.count));
}

void logs::focusing(short unsigned index) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	ef[0].frame = 1;
	add("[...������]");
	while(ismodal()) {
		point camera = getcamera(game::getx(index), game::gety(index));
		ef[0].x = game::getx(index) * elx;
		ef[0].y = game::gety(index) * ely;
		view_zone(0, camera, ef);
		draw::domodal();
		auto phk = gethotkey(hot::key);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(index, direction);
				if(ni != Blocked)
					index = ni;
				continue;
			}
		}
		switch(hot::key) {
		case KeySpace:
			clear_state();
			return;
		}
	}
}

void logs::next() {
	auto p = creature::getplayer();
	if(!p)
		return;
	szprints(zend(state_message), zendof(state_message), "[...������]");
	while(ismodal()) {
		point camera = getcamera(game::getx(p->getposition()), game::gety(p->getposition()));
		view_zone(p, camera);
		draw::domodal();
		switch(hot::key) {
		case KeySpace:
			clear_state();
			return;
		}
	}
}

bool logs::chooseyn() {
	auto p = creature::getplayer();
	if(!p)
		return true;
	add("([+Y/N])?");
	while(ismodal()) {
		point camera = getcamera(game::getx(p->getposition()), game::gety(p->getposition()));
		view_zone(p, camera);
		draw::domodal();
		switch(hot::key) {
		case Alpha + 'Y':
			clear_state();
			return true;
		case Alpha + 'N':
			clear_state();
			return false;
		}
	}
	return false;
}

int logs::input() {
	auto p = creature::getplayer();
	while(ismodal()) {
		point camera = {0, 0};
		if(p)
			camera = getcamera(game::getx(p->getposition()), game::gety(p->getposition()));
		view_zone(p, camera);
		draw::domodal();
		if(hot::key >= Alpha + '1' && hot::key <= (Alpha + '9')) {
			hot::key = hot::key - (Alpha + '1');
			if(hot::key < current_key_index) {
				clear_state();
				return answers[hot::key];
			}
		}
	}
	return 0;
}

void testweapon(creature& e, scene& sc);

static void character_help(creature& e, scene& sc);

static void character_passturn(creature& e, scene& sc) {
	e.wait(Turn * 3);
}

static void ui_show_hide_panel(creature& e, scene& sc) {
	show_gui_panel = !show_gui_panel;
}

static void character_ranged_attack(creature& e, scene& sc) {
	auto enemy = e.getenemy(sc.creatures);
	if(!enemy)
		e.hint("������ ��� ���������� ����");
	else
		e.rangeattack(enemy);
}

static void character_use(creature& e, scene& sc, itm_proc pr, const char* title) {
	anyptr ti;
	sceneparam sp = {{pr}, e, true};
	if(!e.choose(sp, sc, ti, true, title)) {
		e.hint("� ��� ��� ���������� ��������.");
		return;
	}
	item* pi = ti;
	if(pi)
		e.use(sc, *pi);
}

static void character_wand(creature& e, scene& sc) {
	character_use(e, sc, itm_chargeable, "� ������ ��������?");
}

static void character_drink(creature& e, scene& sc) {
	character_use(e, sc, itm_drinkable, "��� ������ ������?");
}

static void character_eat(creature& e, scene& sc) {
	character_use(e, sc, itm_edible, "��� ������ ������?");
}

static void character_read(creature& e, scene& sc) {
	e.use(sc, Literacy);
}

int compare_manual(const void* p1, const void* p2) {
	auto e1 = *((manual**)p1);
	auto e2 = *((manual**)p2);
	auto n1 = e1->value.getname();
	auto n2 = e2->value.getname();
	return strcmp(n1, n2);
}

static void view_manual(creature& e, stringbuffer& sc, manual& element) {
	const int width = 520;
	const int height = 464;
	const int dy = 20;
	const int padding = 4;
	auto index = e.getposition();
	unsigned current_index;
	adat<manual*, 64> source;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = padding * 3;
		point camera = getcamera(game::getx(index), game::gety(index));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, element.value.getname());
		if(element.text)
			y += draw::textf(x, y, width, element.text) + padding;
		if(element.child) {
			for(auto p = element.child; *p; p++) {
				if(p->type != Element)
					continue;
				sc.clear();
				sc.add("[%1]: ", p->value.getname());
				sc.add(p->text);
				y += draw::textf(x, y, width, sc.result) + padding;
			}
		}
		for(auto& proc : element.procs) {
			sc.clear();
			proc(sc, element);
			if(sc)
				y += draw::textf(x, y, width, sc.result) + padding;
		}
		current_index = 0;
		source.clear();
		if(element.child) {
			for(auto p = element.child; *p; p++) {
				if(p->type != Header)
					continue;
				source.add(p);
			}
			qsort(source.data, source.getcount(), sizeof(source.data[0]), compare_manual);
			for(auto p : source) {
				auto x1 = shortcut(x, y, 32, current_index++);
				y += draw::textf(x1, y, width - 32, p->value.getname());
			}
		}
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			clear_state();
			return;
		default:
			if(getkey(hot::key, current_index))
				view_manual(e, sc, *source.data[hot::key]);
			break;
		}
	}
}

static void character_give(creature& e, scene& sc) {
	//creature* source[128];
	//auto result = e.getcreatures(source, e.getposition(), 1, &e);
	//auto pc = e.choose(result, true);
	//if(!pc)
	//	return;
	//item* source_item[128];
	//auto result_item = e.select(source_item, TargetItemWeapon);
	//auto pi = e.choose(result_item, true, "�������� �������");
	//if(!pi)
	//	return;
	//e.give(*pc, *pi, true);
}

extern manual manual_main;

static void character_manual(creature& e, scene& sc) {
	char temp[2048];
	stringbuffer sb(temp);
	view_manual(e, sb, manual_main);
}

static void character_logs(creature& e, scene& sc) {
	const int width = 520;
	const int height = 464;
	const int dy = 20;
	const int pixel_per_line = draw::texth();
	const int lines_per_screen = height / pixel_per_line;
	int index = messages.count - lines_per_screen + 2;
	while(ismodal()) {
		if(index >= (int)messages.count)
			index = messages.count - 1;
		if(index < 0)
			index = 0;
		auto x = (draw::getwidth() - width) / 2;
		auto y = padding * 3;
		auto y0 = y;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "���������");
		for(unsigned i = index; i < messages.count && y < y0 + height; i++)
			y += draw::textf(x, y, width, messages[i]);
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return;
		case KeyUp: index--; break;
		case KeyDown: index++; break;
		case KeyPageDown: index += lines_per_screen - 2; break;
		case KeyPageUp: index -= lines_per_screen - 2; break;
		}
	}
}

static void character_minimap(creature& e, scene& sc) {
	logs::minimap(e.getposition());
}

static void character_debug(creature& e, scene& sc) {
	creature* creature_data[256];
	auto creatures = e.getcreatures(creature_data, e.getposition(), e.getlos());
	auto opponent = e.choose(creatures, true);
	opponent->setplayer();
}

static hotkey hotkeys[] = {{KeyLeft, "��������� �����"},
{KeyHome, "��������� ����� � �����"},
{KeyEnd, "��������� ���� � �����"},
{KeyRight, "��������� ������"},
{KeyPageUp, "��������� ������ � �����"},
{KeyPageDown, "��������� ������ � ���� "},
{KeyUp, "��������� �����"},
{KeyDown, "��������� ����"},
{Alpha + 'I', "������ ��������", character_invertory},
{Alpha + 'P', "������� �������", character_pickup},
{Alpha + 'D', "�������� �������", character_dropdown},
{Alpha + 'M', "����� ���������", character_minimap},
{Alpha + 'V', "�������� � �������", character_stuff},
{Alpha + 'L', "����������� ������", dungeon_lookaround},
{Alpha + 'C', "���������� � ���-�� �����", character_chat},
{Alpha + 'A', "������������ �����", character_skill},
{Alpha + 'Q', "����� ������������� �������", character_ranged_attack},
{Alpha + 'U', "������������ ��������� ������", character_use},
{Alpha + 'S', "������������ ����������", character_spell},
{Ctrl + KeySpace, "������", character_help},
{F1, "������� 1-�� ������", character_setplayer1},
{F2, "������� 2-�� ������", character_setplayer2},
{F3, "������� 3-�� ������", character_setplayer3},
{F4, "������� 4-�� ������", character_setplayer4},
{KeySpace, "��������� 10 �����", character_passturn},
{Alpha + 'G', "��������/�������� ����������", ui_show_hide_panel},
{Alpha + 'Z', "������������ �������", character_wand},
{Alpha + 'E', "������ ���-��", character_eat},
{Ctrl + Alpha + 'T', "�������� �����", character_time},
{Ctrl + Alpha + 'W', "���������� ������", testweapon},
{Ctrl + Alpha + 'D', "������ ���-��", character_drink},
{Ctrl + Alpha + 'R', "��������� ���-��", character_read},
{Ctrl + Alpha + 'M', "������� ������", character_manual},
{Ctrl + Alpha + 'L', "�������� ���������", character_logs},
{Ctrl + Alpha + 'G', "������ �������", character_give},
#ifdef _DEBUG
{Ctrl + Alpha + 'X', "��������� (�������)", character_debug},
#endif
};

void logs::worldedit() {
	struct editor {
		struct shortcut {
			int			key;
			const char*	name;
			void		(editor::*proc)(const shortcut& e);
			direction_s	direction;
			tile_s		tile;
			explicit operator bool() const { return key != 0; }
		};
		short unsigned	position;
		tile_s			tile;
		bool			stop;
		void place(const shortcut& e) {
			game::set(position, tile);
		}
		void move(const shortcut& e) {
			auto new_position = game::to(position, e.direction);
			if(new_position != Blocked)
				position = new_position;
		}
		void cancel(const shortcut& e) {
			stop = true;
		}
		void settile(const shortcut& e) {
			tile = e.tile;
		}
		void savemap(const shortcut& e) {
		}
		void readmap(const shortcut& e) {
		}
		void minimap(const shortcut& e) {
			logs::minimap(position);
		}
		void info() {
			if(!show_gui_panel)
				return;
			char temp[512];
			const int width = 300;
			const int height = draw::texth() * 2;
			auto x = (draw::getwidth() - width) / 2;
			auto y = draw::getheight() - height - padding * 2;
			auto y2 = y;
			auto px = game::getx(position);
			auto py = game::gety(position);
			draw::state push;
			draw::fore = colors::white;
			view_dialog({x, y, x + width, y + height});
			y += field(x, y, 80, "��������", getstr(tile));
			y += field(x, y, 80, "����������", szprint(temp, "%1i, %2i", px, py));
		}
		const shortcut* getshortcuts() const {
			static editor::shortcut hotkeys[] = {{KeyLeft, "��������� �����", &editor::move, Left},
			{KeyHome, "��������� ����� � �����", &editor::move, LeftUp},
			{KeyEnd, "��������� ���� � �����", &editor::move, LeftDown},
			{KeyRight, "��������� ������", &editor::move, Right},
			{KeyPageUp, "��������� ������ � �����", &editor::move, RightUp},
			{KeyPageDown, "��������� ������ � ���� ", &editor::move, RightDown},
			{KeyUp, "��������� �����", &editor::move, Up},
			{KeyDown, "��������� ����", &editor::move, Down},
			{Alpha + 'Q', "����������� ��������� ��������", &editor::place},
			{Alpha + '1', "������� ����", &editor::settile, Center, Sea},
			{Alpha + '2', "������� ������", &editor::settile, Center, Swamp},
			{Alpha + '3', "������� �������", &editor::settile, Center, Plain},
			{Alpha + '4', "������� ���", &editor::settile, Center, Forest},
			{Alpha + '5', "������� �����", &editor::settile, Center, Foothills},
			{Alpha + '6', "������� ����", &editor::settile, Center, Mountains},
			{Alpha + '7', "������� �������� ����", &editor::settile, Center, CloudPeaks},
			{Ctrl + Alpha + 'S', "��������� �����", &editor::savemap},
			{Ctrl + Alpha + 'R', "������������ �����", &editor::readmap},
			{Alpha + 'M', "����� ����", &editor::minimap},
			{KeyEscape, "��������� ����� � ������� ����", &editor::cancel},
			};
			return hotkeys;
		}
		editor() : position(game::get(10, 10)), tile(Sea), stop(false) {}
	};
	editor context;
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	while(!context.stop) {
		point camera = getcamera(game::getx(context.position), game::gety(context.position));
		ef[0].x = game::getx(context.position) * elx;
		ef[0].y = game::gety(context.position) * ely;
		view_world(camera, true, ef);
		view_message();
		context.info();
		draw::domodal();
		auto pid = findkey(context.getshortcuts(), hot::key);
		if(pid && pid->proc) {
			(context.*pid->proc)(*pid);
			clear_state();
		}
	}
}

int compare_hotkey(const void* p1, const void* p2) {
	auto e1 = (hotkey*)p1;
	auto e2 = (hotkey*)p2;
	return strcmp(e1->command, e2->command);
}

void logs::initialize() {
	viewport.x = 800; viewport.y = 600;
	qsort(hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0]), sizeof(hotkeys[0]), compare_hotkey);
	metrics::font = (sprite*)loadb("art/font.pma");
	metrics::h1 = (sprite*)loadb("art/h1.pma");
	metrics::h2 = (sprite*)loadb("art/h2.pma");
	metrics::h3 = (sprite*)loadb("art/h3.pma");
	draw::create(-1, -1, viewport.x, viewport.y, WFMinmax | WFResize, 32);
	colors::active = color::create(172, 128, 0);
	colors::border = color::create(32, 32, 32);
	colors::button = color::create(0, 122, 204);
	colors::form = color::create(48, 48, 48);
	colors::window = color::create(30, 30, 30);
	colors::text = color::create(255, 255, 255);
	colors::edit = color::create(38, 79, 120);
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	colors::special = color::create(255, 244, 32);
	colors::border = colors::window.mix(colors::text, 192);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(100, 100, 120);
	colors::tabs::back = color::create(255, 204, 0);
	colors::tabs::text = colors::black;
	draw::fore = colors::text;
}

static void character_help(creature& e, scene& sc) {
	const int width = 700;
	const int height = 460;
	const int dy = 20;
	while(ismodal()) {
		int x = (draw::getwidth() - width) / 2;
		int y = padding * 3;
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "������� �������");
		auto w = width / 2;
		auto y1 = y;
		for(auto& m : hotkeys) {
			auto x1 = x;
			x1 = shortcutnb(x1, y1, 80, m.key);
			x1 = textl(x1, y1, 218, m.command);
			y1 += dy;
			if(y1 >= y + height - dy) {
				x = x + width / 2;
				y1 = y;
			}
		}
		draw::domodal();
		switch(hot::key) {
		case KeyEscape:
		case KeySpace:
			return;
		}
	}
}

static hotkey* gethotkey(int id) {
	for(auto& e : hotkeys) {
		if(e.key == id)
			return &e;
	}
	return 0;
}

void logs::choose(const menu* p) {
	auto width = 400;
	auto height = (zlen(p)-1)*draw::texth() + metrics::h1->get(0).sy;
	while(p) {
		logs::add(p->text);
		for(auto index = 1; p[index]; index++)
			logs::add(index, p[index].text);
		auto id = logs::input();
		if(p[id].proc)
			p[id].proc();
		if(p[id].child)
			choose(p[id].child);
		p = p[id].next;
	}
}

void logs::turn(creature& e, scene& sc) {
	while(ismodal()) {
		point camera = getcamera(game::getx(e.getposition()), game::gety(e.getposition()));
		view_zone(&e, camera);
		draw::domodal();
		auto phk = gethotkey(hot::key);
		if(!phk)
			continue;
		// ������� ����
		clear_state();
		// �������� �������, ��������� � ��������� �����
		auto direction = getdirection(phk->key);
		if(direction != Center) {
			if(!e.move(game::to(e.getposition(), direction))) {
				logs::add("�� �� ������ ������ ����.");
				continue;
			}
			// ����� �� ��� ���� � ����������� �� ��������������
			break;
		}
		if(phk->proc)
			phk->proc(e, sc);
		e.wait(2);
		break;
	}
}