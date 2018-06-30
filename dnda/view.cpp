#include "draw.h"
#include "resources.h"
#include "main.h"

struct hotkey {
	unsigned		key;
	const char*		command;
	void(*proc)(creature& e);
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
static char			state_message[4096];
static hotkey*		gethotkey(int id);
static int			answers[9];
static bool			show_gui_panel = true;

namespace colors {
color			fow = color::create(10, 10, 10);
}

static void clear_state() {
	state_message[0] = 0;
	current_key_index = 0;
}

void logs::add(const char* format, ...) {
	addv(format, xva_start(format));
}

void logs::add(int id, const char* format, ...) {
	if(current_key_index >= sizeof(answers) / sizeof(answers[0]))
		return;
	answers[current_key_index] = id;
	add("\n[%1i)] ", current_key_index + 1);
	addv(format, xva_start(format));
	current_key_index++;
}

void logs::addv(stringcreator& sc, const char* format, const char* vl) {
	char* p = zend(state_message);
	// First string may not be emphty or white spaced
	if(p == state_message)
		format = zskipspcr(format);
	if(format[0] == 0)
		return;
	if(p != state_message) {
		if(p[-1] != ' ' && p[-1] != '\n' && *format != '\n' && *format != '.' && *format != ',') {
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

void logs::addv(const char* format, const char* vl) {
	stringcreator sc;
	addv(sc, format, vl);
}

void __cdecl dlgerr(char const* title, char const* format, ...) {}

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

static int textl(int x, int y, int width, item& value) {
	char temp[260]; value.getname(temp, zendof(temp));
	draw::state push;
	switch(value.getidentify()) {
	case KnowEffect:
		switch(value.getmagic()) {
		case Artifact: draw::fore = colors::yellow; break;
		case Cursed: draw::fore = colors::red; break;
		case Magical: draw::fore = colors::text.mix(colors::blue, 128); break;
		default: draw::fore = colors::text; break;
		}
		break;
	case Unknown:
		draw::fore = colors::gray;
		break;
	default:
		draw::fore = colors::text;
		break;
	}
	draw::text(x, y, temp);
	return x + width;
}

static int shortcutnb(int x, int y, int w, int key) {
	draw::state push;
	char temp[32];
	draw::fore = colors::green;
	draw::text(x, y, key2str(temp, key));
	return x + w;
}

static int shortcut(int x, int y, int w, int key) {
	draw::state push;
	char temp[32];
	char temx[32];
	draw::fore = colors::green;
	szprints(temx, zendof(temx), "%1)", key2str(temp, key));
	draw::text(x, y, temx);
	return x + w;
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

static int input() {
	auto id = draw::input();
	if(!id)
		exit(0);
	return id;
}

inline bool wget(short unsigned i, direction_s direction, tile_s value) {
	auto i1 = game::to(i, direction);
	if(i1 == 0xFFFF)
		return true;
	return game::gettile(i1) == value;
}

inline bool wget(short unsigned i, direction_s direction, tile_s value, bool default_result) {
	auto i1 = game::to(i, direction);
	if(i1 == 0xFFFF)
		return default_result;
	return game::gettile(i1) == value;
}

inline int mget(int ox, int oy, int mx, int my) {
	int x = mx - ox;
	int y = my - oy;
	if(x < 0 || y < 0 || x >= scrx || y >= scry)
		return -1;
	return y * scrx + x;
}

inline bool xget(short unsigned i, direction_s direction) {
	auto i1 = game::to(i, direction);
	if(i1 == 0xFFFF)
		return false;
	return !game::isexplore(i1);
}

static int getaccindex(item_s type) {
	switch(type) {
	case BowLong: return 0;
	case BowShort: return 1;
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
	if(ranged) {
		auto index = getaccindex(ranged.gettype());
		if(index != -1)
			draw::image(x, y, gres(ResPCmac), index, flags, alpha);
	}
	draw::image(x, y, gres(ResPCmar), i1, flags, alpha);
	draw::image(x, y, gres(ResPCmbd), (race - Human) * 6 + (gender - Male) * 3 + at, flags, alpha);
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
	for(auto p : creatures) {
		auto i = mget(rc.x1, rc.y1, game::getx(p->position), game::gety(p->position));
		if(i == -1)
			continue;
		if(!game::isvisible(p->position))
			continue;
		units[i] = p;
	}
	// �������������� ������� ��������
	memset(stuff, 0, sizeof(stuff));
	for(auto& e : grounditems) {
		if(!e.object)
			continue;
		auto i = mget(rc.x1, rc.y1, game::getx(e.index), game::gety(e.index));
		if(i == -1)
			continue;
		if(stuff[i] == NoItem)
			stuff[i] = e.object.gettype();
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
				if(!game::ishidden(i))
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
					draw::image(x, y, gres(ResItems), 0, 0);
					break;
				default:
					draw::image(x, y, gres(ResItems), stuff[pi], 0);
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
				r = game::isopen(i) ? 0 : 1;
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
					switch(pc->direction) {
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
						view_avatar(x, y, pc->type, pc->race, pc->gender,
							pc->wears[Torso], pc->wears[Melee], pc->wears[OffHand],
							pc->wears[Ranged], pc->wears[TorsoBack], flags, alpha);
					} else
						draw::image(x, y, gres(ResMonsters), pc->role, flags, alpha);
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
				if(!game::isexplore(i))
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

static void view_info(const creature& e) {
	if(!show_gui_panel)
		return;
	char temp[512];
	const int tw = 26;
	const int dx = 52;
	const int width = 410;
	const int height = draw::texth() * 4;
	auto x = (draw::getwidth() - width) / 2;
	auto y = draw::getheight() - height - padding * 2;
	auto y2 = y;
	draw::state push;
	draw::fore = colors::white;
	view_dialog({x, y, x + width, y + height});
	draw::textf(x, y, width, e.getfullname(temp, zendof(temp), true, true));
	auto loc = game::getlocation(e.position);
	if(loc) {
		auto p = loc->getname(temp);
		draw::text(x + width - draw::textw(p), y, p);
	}
	y += draw::texth();
	int y1 = y;
	int x1 = x;
	y += field(x, y, tw, "��", e.get(Strenght));
	y += field(x, y, tw, "��", e.get(Intellegence));
	x += dx;
	y = y1;
	y += field(x, y, tw, "��", e.get(Dexterity));
	y += field(x, y, tw, "��", e.get(Wisdow));
	x += dx;
	y = y1;
	y += field(x, y, tw, "��", e.get(Constitution));
	y += field(x, y, tw, "��", e.get(Charisma));
	x += dx + 6;
	y = y1;
	y += field(x, y, tw, "��", e.getattackinfo(Melee).bonus, e.getattackinfo(Ranged).bonus);
	y += field(x, y, tw, "��", e.getdefence(), e.getarmor());
	x += dx + 6;
	y = y1;
	y += field(x, y, 40, "����", e.gethits(), e.getmaxhits());
	y += field(x, y, 40, "����", e.getmana(), e.getmaxmana());
	x += dx + 40 + 20 - tw;
	y = y1;
	y += field(x, y, 52, "����", e.experience);
	//y += field(x, y, 52, "�����", getstrfdat(temp, segments));
	y += field(x, y, 52, "������", e.money);
	x += dx + 58;
	x = x1;
	y = y1 + draw::texth() * 2;
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
		const char* pt = getstr(i);
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
			if(!game::isexplore(i))
				continue;
			switch(game::gettile(i)) {
			case Hill:
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
				pixel(x3, y3, water);
				break;
			case Wall:
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

static int view_total(int x, int y, int width, item** source, unsigned count) {
	char temx[64];
	char temp[1024];
	auto result = 0;
	auto rcount = 0;
	for(unsigned i = 0; i < count; i++) {
		result += source[i]->getweight();
		rcount++;
	}
	if(rcount)
		szprints(temp, zendof(temp), "����� %1.", szweight(temx, result));
	else
		szprints(temp, zendof(temp), "��� ���������� ���������.");
	return draw::textf(x, y, width, temp);
}

item* logs::choose(const creature& e, item** source, unsigned count, const char* title) {
	const int width = 500;
	const int height = 360;
	const int dy = 20;
	if(!title)
		title = "��������";
	while(true) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, title);
		for(unsigned i = 0; i < count; i++) {
			char temp[260];
			auto x1 = shortcut(x, y, 32, Alpha + 'A' + i);
			x1 = textl(x1, y, 408, *source[i]);
			x1 = textr(x1, y, 60, szweight(temp, source[i]->getweight()));
			y += dy;
		}
		y += padding;
		y += view_total(x, y, width, source, count);
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
			return 0;
		default:
			if(id >= Alpha + 'A' && id <= Alpha + 'Z') {
				auto count = zlen(source);
				auto pos = id - (Alpha + 'A');
				if(pos < count)
					return source[pos];
			}
			break;
		}
	}
}

static void character_pickup(creature& e) {
	char temp[260]; szprints(temp, zendof(temp), "������� ��������");
	item* source[48];
	auto index = e.position;
	auto p = source;
	for(auto& s : grounditems) {
		if(s.index != index)
			continue;
		if(!s.object)
			continue;
		*p++ = &s.object;
	}
	*p = 0;
	auto count = zlen(source);
	if(!count)
		return;
	auto result = logs::choose(e, source, count, temp);
	if(!result)
		return;
	if(e.pickup(*result))
		result->clear();
}

static void character_dropdown(creature& e) {
	char temp[260]; szprints(temp, zendof(temp), "�������� ��������");
	item* source[48];
	auto p = source;
	for(auto& s : e.backpack) {
		if(!s)
			continue;
		*p++ = &s;
	}
	item* result = logs::choose(e, source, p - source, temp);
	if(!result)
		return;
	if(e.dropdown(*result))
		result->clear();
}

static void character_stuff(creature& e) {
	item* source[48];
	auto p = source;
	for(auto& s : e.backpack) {
		if(!s)
			continue;
		*p++ = &s;
	}
	logs::choose(e, source, p - source, "������");
}

static item* choose_item(creature& e, point camera, slot_s slot) {
	char temp[260]; szprints(temp, zendof(temp), "��� ����� �� %1?", getstr(slot));
	item* source[sizeof(e.backpack) / sizeof(e.backpack[0])];
	auto p = source;
	for(auto& s : e.backpack) {
		if(!s || !s.is(slot))
			continue;
		*p++ = &s;
	}
	return logs::choose(e, source, p - source, temp);
}

static void character_invertory(creature& e) {
	const int width = 500;
	const int height = 360;
	const int dy = 20;
	while(true) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "����������");
		item* source[sizeof(e.wears) / sizeof(e.wears[0]) + 2];
		auto p = source;
		for(auto i = Head; i <= Amunitions; i = (slot_s)(i + 1)) {
			char temp[260];
			auto x1 = shortcut(x, y, 32, Alpha + 'A' + i);
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
		y += view_total(x, y, width, source, p - source);
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
			return;
		default:
			if(id >= Alpha + 'A' && id <= Alpha + 'A' + Amunitions) {
				auto slot = (slot_s)(id - (Alpha + 'A'));
				if(e.wears[slot]) {
					if(!e.pickup(e.wears[slot]))
						game::drop(e.position, e.wears[slot]);
					e.wears[slot].clear();
				} else {
					auto p = choose_item(e, camera, slot);
					if(p) {
						e.wears[slot] = *p;
						e.wears[slot].set(KnowQuality);
						p->clear();
					}
				}
			}
			break;
		}
	}
}

short unsigned logs::choose(const creature& e, short unsigned* source, int count) {
	if(!count)
		return 0xFFFF;
	else if(count == 1)
		return source[0];
	int current = 0;
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	while(true) {
		clear_state();
		short unsigned current_index = source[current];
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		game::looktarget(current_index);
		view_zone(&e, camera, ef);
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
			clear_state();
			return 0xFFFF;
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
}

static short unsigned choose_target(creature& e, short unsigned current_index) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	while(true) {
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		view_zone(&e, camera, ef);
		auto id = draw::input();
		auto phk = gethotkey(id);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(current_index, direction);
				if(ni != 0xFFFF)
					current_index = ni;
				continue;
			}
		}
		switch(id) {
		case KeyEscape:
			return 0xFFFF;
		case KeyEnter:
			return current_index;
		}
	}
}

static int compare_skills(const void* p1, const void* p2) {
	auto e1 = *((skill_s*)p1);
	auto e2 = *((skill_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

bool logs::choose(creature& e, skill_s& result, skill_s* source, unsigned count, bool can_escape) {
	char temp[260];
	const int width = 400;
	const int height = 360;
	const int dy = 20;
	while(true) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "������");
		auto index = 0;
		for(unsigned in = 0; in < count; in++) {
			auto i = source[in];
			if(e.getbasic(i) <= 0)
				continue;
			auto x1 = shortcut(x, y, 32, Alpha + 'A' + (index++));
			x1 = textl(x1, y, 308, getstr(i));
			x1 = textl(x1, y, 60, szpercent(temp, zendof(temp), e.get(i)));
			y += dy;
		}
		if(!index)
			textl(x, y, width, "� ��� ��� ���������� �������");
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
			if(can_escape)
				return false;
			break;
		default:
			if(id >= Alpha + 'A' && id <= Alpha + 'Z') {
				auto pos = (unsigned)(id - (Alpha + 'A'));
				if(pos < count) {
					result = source[pos];
					return true;
				}
			}
			break;
		}
	}
}

bool logs::choose(creature& e, skill_s& result, bool can_escape) {
	auto count = 0;
	skill_s source[TwoWeaponFighting + 1];
	for(auto i = Bargaining; i <= TwoWeaponFighting; i = (skill_s)(i + 1)) {
		if(e.getbasic(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare_skills);
	return choose(e, result, source, count, can_escape);
}

bool logs::choose(creature& e, spell_s& result, spell_s* source, unsigned count) {
	char temp[260];
	const int width = 400;
	const int height = 360;
	const int dy = 20;
	while(true) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
		view_zone(&e, camera);
		y += view_dialog({x, y, x + width, y + height}, "����������");
		auto index = 0;
		for(unsigned in = 0; in < count; in++) {
			auto i = source[in];
			if(e.get(i) <= 0)
				continue;
			auto x1 = shortcut(x, y, 32, Alpha + 'A' + (index++));
			x1 = textl(x1, y, 308, szprints(temp, zendof(temp), getstr(i)));
			x1 = textl(x1, y, 60, szprints(temp, zendof(temp), "%1i", e.getcost(i)));
			y += dy;
		}
		if(!index)
			textl(x, y, width, "� ��� ��� ����������");
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
			return false;
		default:
			if(id >= Alpha + 'A' && id <= Alpha + 'Z') {
				auto pos = (unsigned)(id - (Alpha + 'A'));
				if(pos < count) {
					result = source[pos];
					return true;
				}
			}
			break;
		}
	}
}

static void setplayer(creature& e, int index) {
	if(!players[index])
		return;
	players[index]->setplayer();
}

static void character_setplayer1(creature& e) {
	setplayer(e, 0);
}

static void character_setplayer2(creature& e) {
	setplayer(e, 1);
}

static void character_setplayer3(creature& e) {
	setplayer(e, 2);
}

static void character_setplayer4(creature& e) {
	setplayer(e, 3);
}

static void character_skill(creature& e) {
	skill_s result;
	if(!logs::choose(e, result))
		return;
	e.use(result);
}

static void character_spell(creature& e) {
	spell_s result;
	if(!logs::choose(e, result))
		return;
	e.use(result);
}

static void character_time(creature& e) {
	char temp[260];
	game::getdate(temp, zendof(temp), segments, true);
	logs::add("������ %1.", temp);
}

static void dungeon_lookaround(creature& e) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	ef[0].frame = 1;
	short unsigned current_index = e.position;
	while(true) {
		clear_state();
		game::lookhere(current_index);
		point camera = getcamera(game::getx(current_index), game::gety(current_index));
		ef[0].x = game::getx(current_index) * elx;
		ef[0].y = game::gety(current_index) * ely;
		view_zone(&e, camera, ef);
		auto id = draw::input();
		auto phk = gethotkey(id);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(current_index, direction);
				if(ni != 0xFFFF)
					current_index = ni;
				continue;
			}
		}
		switch(id) {
		case KeyEscape:
		case KeyEnter:
			return;
		}
	}
}

void logs::minimap(creature& e) {
	char temp[128]; temp[0] = 0;
	int w = max_map_x * mmaps + 280;
	int h = max_map_y * mmaps;
	point camera = getcamera(game::getx(e.position), game::gety(e.position));
	while(true) {
		draw::rectf({0, 0, draw::getwidth(), draw::getheight()}, colors::form);
		if(game::statistic.level)
			szprints(temp, zendof(temp), "������� %1i", game::statistic.level);
		//view_dialog(bsgets(Minimap, Name), temp, 1);
		view_mini((draw::getwidth() - w) / 2, (draw::getheight() - h) / 2, camera);
		int id = draw::input();
		switch(id) {
		case 0:
		case KeyEscape:
			return;
		}
	}
}

static void character_chat(creature& e) {
	creature* opponent = 0;
	if(!logs::getcreature(e, &opponent, TargetNotHostileCreature, 1))
		return;
	e.chat(opponent);
}

static void character_use(creature& e) {
	short unsigned source[3 * 3 + 1];
	auto count = e.getobjects(source, sizeof(source) / sizeof(source[0]), NoTarget, 1);
	if(!count) {
		logs::add("������ ��� ���� ������, ��� ����� ���� �� ������������.");
		return;
	}
	if(e.manipulate(logs::choose(e, source, count)))
		e.wait();
}

void logs::focusing(short unsigned index) {
	fxeffect ef[2];
	ef[0].res = gres(ResUI);
	ef[0].frame = 1;
	add("[...������]");
	while(true) {
		point camera = getcamera(game::getx(index), game::gety(index));
		ef[0].x = game::getx(index) * elx;
		ef[0].y = game::gety(index) * ely;
		view_zone(0, camera, ef);
		auto id = draw::input();
		auto phk = gethotkey(id);
		if(phk) {
			auto direction = getdirection(phk->key);
			if(direction != Center) {
				auto ni = game::to(index, direction);
				if(ni != 0xFFFF)
					index = ni;
				continue;
			}
		}
		switch(id) {
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
	add("[...������]");
	while(true) {
		point camera = getcamera(game::getx(p->position), game::gety(p->position));
		view_zone(p, camera);
		auto id = draw::input();
		switch(id) {
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
	while(true) {
		point camera = getcamera(game::getx(p->position), game::gety(p->position));
		view_zone(p, camera);
		auto id = draw::input();
		switch(id) {
		case Alpha + 'Y':
			clear_state();
			return true;
		case Alpha + 'N':
			clear_state();
			return false;
		}
	}
}

int logs::input() {
	auto p = creature::getplayer();
	if(!p)
		return 0;
	while(true) {
		point camera = getcamera(game::getx(p->position), game::gety(p->position));
		view_zone(p, camera);
		auto id = draw::input();
		if(id >= Alpha + '1' && id <= (Alpha + '9')) {
			id = id - (Alpha + '1');
			if(id < current_key_index) {
				clear_state();
				return answers[id];
			}
		}
	}
}

void testweapon(creature& e);
static void character_help(creature& e);

static void character_passturn(creature& e) {
	e.passturn(10);
}

static void ui_show_hide_panel(creature& e) {
	show_gui_panel = !show_gui_panel;
}

static void character_ranged_attack(creature& e) {
	e.rangeattack();
}

static void character_wand(creature& e) {
	item* result = 0;
	if(!logs::getitem(e, &result, TargetItemChargeable, "� ������ ��������?"))
		return;
	if(result)
		e.use(*result);
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
{Alpha + 'M', "����� ���������", logs::minimap},
{Alpha + 'V', "�������� � �������", character_stuff},
{Alpha + 'L', "����������� ������", dungeon_lookaround},
{Alpha + 'C', "���������� � ���-�� �����", character_chat},
{Alpha + 'A', "������������ �����", character_skill},
{Alpha + 'Q', "����� ������������� �������", character_ranged_attack},
{Alpha + 'U', "������������ ��������� ������", character_use},
{Alpha + 'S', "������������ ����������", character_spell},
{Ctrl + Alpha + 'T', "�������� �����", character_time},
{Ctrl + Alpha + 'W', "���������� ������", testweapon},
{KeyEscape, "������", character_help},
{F1, "������� 1-�� ������", character_setplayer1},
{F2, "������� 2-�� ������", character_setplayer2},
{F3, "������� 3-�� ������", character_setplayer3},
{F4, "������� 4-�� ������", character_setplayer4},
{KeySpace, "��������� 10 �����", character_passturn},
{Alpha + 'G', "��������/�������� ����������", ui_show_hide_panel},
{Alpha + 'Z', "������������ �������", character_wand},
};

int compare_hotkey(const void* p1, const void* p2) {
	auto e1 = (hotkey*)p1;
	auto e2 = (hotkey*)p2;
	return strcmp(e1->command, e2->command);
}

void logs::initialize() {
	viewport.x = 800;
	viewport.y = 600;
	qsort(hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0]), sizeof(hotkeys[0]), compare_hotkey);
	metrics::font = (sprite*)loadb("art/font.pma");
	metrics::h1 = (sprite*)loadb("art/h1.pma");
	metrics::h2 = (sprite*)loadb("art/h2.pma");
	metrics::h3 = (sprite*)loadb("art/h3.pma");
	static draw::window main_window(-1, -1, viewport.x, viewport.y, WFMinmax | WFResize, 32);
	colors::form = color::create(32, 32, 64);
	colors::text = colors::white;
	colors::special = colors::yellow;
	draw::fore = colors::text;
	draw::sysmouse(false);
}

static void character_help(creature& e) {
	const int width = 700;
	const int height = 360;
	const int dy = 20;
	while(true) {
		int x = (draw::getwidth() - width) / 2;
		int y = (draw::getheight() - height) / 2;
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
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
		auto id = draw::input();
		switch(id) {
		case KeyEscape:
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

void logs::turn(creature& e) {
	while(true) {
		point camera = getcamera(game::getx(e.position), game::gety(e.position));
		view_zone(&e, camera);
		auto phk = gethotkey(draw::input());
		if(!phk)
			continue;
		// ������� ����
		clear_state();
		// �������� �������, ��������� � ��������� �����
		auto direction = getdirection(phk->key);
		if(direction != Center) {
			if(!e.move(game::to(e.position, direction))) {
				logs::add("�� �� ������ ������ ����.");
				continue;
			}
			// ����� �� ��� ���� � ����������� �� ��������������
			break;
		}
		if(phk->proc)
			phk->proc(e);
		// �������� ����������� ����� 2 ��������.
		// �������� ����� ������������� �������������� ����� ��������
		e.wait(2);
		break;
	}
}

void draw::window::closing(void) {}

void draw::window::opening(void) {}

void draw::window::resizing(const rect& rc) {
	if(draw::canvas) {
		draw::canvas->resize(rc.x2, rc.y2, draw::canvas->bpp, true);
		draw::clipping.set(0, 0, rc.x2, rc.y2);
	}
}