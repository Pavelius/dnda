#include "archive.h"
#include "main.h"

// ABOUT TIME
// Each minute has 12 segments, and each segment if 6 seconds duration.
// Attack duration is 12 segments - speed of weapon - dexterity speed bonus but minimum of 3 segments

areainfo				game::statistic;
int						isqrt(int num);
static unsigned			start_year;
unsigned				segments = 7 * Hour;
adat<site, 128>			sites;
adat<groundinfo, 2048>	grounditems;
static tile_s			mptil[max_map_x*max_map_y];
static map_object_s		mpobj[max_map_x*max_map_y];
static unsigned char	mprnd[max_map_x*max_map_y];
static unsigned char	mpflg[max_map_x*max_map_y];
static short unsigned	movements[max_map_x*max_map_y];
static short unsigned	stack[256 * 256];
static direction_s		all_aroud[] = {Left, Right, Up, Down, LeftDown, LeftUp, RightDown, RightUp};
static const direction_s orientations_5b5[25] = {
	LeftUp, LeftUp, Up, Up, RightUp,
	Left, LeftUp, Up, RightUp, RightUp,
	Left, Left, Center, Right, Right,
	LeftDown, LeftDown, Down, RightDown, Right,
	LeftDown, Down, Down, RightDown, RightDown
};

site* game::add(site_s type, rect rc) {
	auto p = sites.add();
	p->type = type;
	*((rect*)p) = rc;
	return p;
}

bool game::is(short unsigned i, map_flag_s v) {
	return (mpflg[i] & (1 << v)) != 0;
}

aref<site> game::getsites() {
	return sites;
}

void game::set(short unsigned i, map_flag_s type, bool value) {
	if(value)
		mpflg[i] |= (1 << type);
	else
		mpflg[i] &= ~(1 << type);
}

direction_s game::getdirection(point s, point d) {
	const int osize = 5;
	int deltaX = d.x - s.x;
	int deltaY = d.y - s.y;
	int st = (2 * imax(iabs(deltaX), iabs(deltaY)) + osize - 1) / osize;
	if(!st)
		return Center;
	int ax = deltaX / st;
	int ay = deltaY / st;
	return orientations_5b5[(ay + (osize / 2))*osize + ax + (osize / 2)];
}

static unsigned getminute() {
	return (segments / Minute) % 60;
}

static unsigned gethour() {
	return (segments / Hour) % 24;
}

static unsigned getday() {
	return (segments / Day) % 30;
}

static char* zadd(char* result, const char* result_maximum, const char* format, ...) {
	szprintvs(zend(result), result_maximum, format, xva_start(format));
}

const char* game::getdate(char* result, const char* result_maximum, unsigned segments, bool show_time) {
	auto v = getday();
	result[0] = 0;
	if(v)
		szprints(result, result_maximum, "День %1i", v);
	if(result[0])
		szprints(zend(result), result_maximum, ", ");
	szprints(zend(result), result_maximum, "%1i часов %2i минут", gethour(), getminute());
	return result;
}

int	game::getnight() {
	const unsigned dawn_hour = 5 * Hour;
	const unsigned dusk_hour = 20 * Hour;
	const unsigned lenght = 2 * Hour;
	// Dawn begin with 5 to 7
	// Evening was in 20 to 22
	unsigned k = segments % Day;
	if(k >= dawn_hour + lenght && k < dusk_hour)
		return 0;
	if(k >= dawn_hour && k < (dawn_hour + lenght))
		return 100 - ((k - dawn_hour) * 100) / lenght;
	if(k >= dusk_hour && k < (dusk_hour + lenght))
		return (k - dusk_hour) * 100 / lenght;
	return 100;
}

map_object_s game::getobject(short unsigned i) {
	return mpobj[i];
}

bool game::isdungeon() {
	return statistic.isdungeon;
}

tile_s game::gettile(short unsigned i) {
	if(i == Blocked)
		return NoTile;
	return mptil[i];
}

int game::getrand(short unsigned i) {
	return mprnd[i];
}

void game::set(short unsigned i, tile_s value) {
	if(i == Blocked)
		return;
	mptil[i] = value;
	switch(value) {
	case Wall:
	case Water:
	case Road:
		set(i, NoTileObject);
		break;
	}
}

void game::set(short unsigned i, tile_s value, int w, int h) {
	for(auto y = i; y != Blocked && h > 0; y = to(y, Down), h--) {
		auto w1 = w;
		for(auto x = y; x != Blocked && w1 > 0; x = to(x, Right), w1--)
			set(x, value);
	}
}

int game::getindex(short unsigned i, tile_s e) {
	auto m = 0;
	auto n = gettile(to(i, Up));
	if(n != e && n)
		m |= 1;
	n = gettile(to(i, Down));
	if(n != e && n)
		m |= 2;
	n = gettile(to(i, Left));
	if(n != e && n)
		m |= 4;
	n = gettile(to(i, Right));
	if(n != e && n)
		m |= 8;
	return m;
}

short unsigned game::getfree(short unsigned i) {
	auto xc = getx(i);
	auto yc = gety(i);
	for(int r = 0; r < 10; r++) {
		for(int y = yc - r; y <= yc + r; y++) {
			if(y < 0 || y >= max_map_y)
				continue;
			for(int x = xc - r; x <= xc + r; x++) {
				if(x < 0 || x >= max_map_x)
					continue;
				auto i1 = get(x, y);
				if(!ispassable(i1))
					continue;
				if(creature::getcreature(i1))
					continue;
				return i1;
			}
		}
	}
	return Blocked;
}

void game::set(short unsigned i, map_object_s value) {
	if(i == Blocked)
		return;
	mpobj[i] = value;
}

void game::set(short unsigned i, unsigned char value) {
	if(i == Blocked)
		return;
	mprnd[i] = value;
}

bool game::ispassable(short unsigned i) {
	if(i == Blocked)
		return false;
	switch(mptil[i]) {
	case Wall:
		return false;
	default:
		break;
	}
	switch(mpobj[i]) {
	case Tree:
		return false;
	case Door:
		return is(i, Opened);
	default:
		break;
	}
	return true;
}

bool game::ispassabledoor(short unsigned i) {
	if(i == Blocked)
		return false;
	switch(mptil[i]) {
	case Wall:
		return false;
	default:
		break;
	}
	switch(mpobj[i]) {
	case Tree:
		return false;
	case Door:
		return !is(i, Sealed);
	default:
		break;
	}
	return true;
}

bool game::ispassablelight(short unsigned i) {
	if(i == Blocked)
		return false;
	switch(mptil[i]) {
	case Wall:
	case NoTile:
		return false;
	default:
		break;
	}
	switch(mpobj[i]) {
	case Door:
		return is(i, Opened);
	case Tree:
		return false;
	default:
		break;
	}
	return true;
}

trap_s game::gettrap(short unsigned i) {
	if(mpobj[i] != Trap)
		return NoTrap;
	return (trap_s)(1 + mprnd[i] % (TrapWater - TrapAnimal + 1));
}

direction_s game::turn(direction_s from, direction_s side) {
	switch(side) {
	case Up:
		return from;
	case RightUp:
		switch(from) {
		case Left: return LeftUp;
		case LeftUp: return Up;
		case Up: return RightUp;
		case RightUp: return Right;
		case Right: return RightDown;
		case RightDown: return Down;
		case Down: return LeftDown;
		case LeftDown: return Left;
		default: return Center;
		}
	case LeftUp:
		switch(from) {
		case Left: return LeftDown;
		case LeftDown: return Down;
		case Down: return RightDown;
		case RightDown: return Right;
		case Right: return RightUp;
		case RightUp: return Up;
		case Up: return LeftUp;
		case LeftUp: return Left;
		default: return Center;
		}
	case Down:
		switch(from) {
		case Left: return Right;
		case Right: return Left;
		case Up: return Down;
		case Down: return Up;
		default: return Center;
		}
	case Left:
		switch(from) {
		case Left: return Down;
		case Right: return Up;
		case Up: return Left;
		case Down: return Right;
		default: return Center;
		}
	case Right:
		switch(from) {
		case Left: return Up;
		case Right: return Down;
		case Up: return Right;
		case Down: return Left;
		default: return Center;
		}
	default:
		return Center;
	}
}

short unsigned game::to(short unsigned index, direction_s id) {
	switch(id) {
	case Left:
		if((index%max_map_x) == 0)
			return Blocked;
		return index - 1;
	case Right:
		if((index%max_map_x) >= max_map_x - 1)
			return Blocked;
		return index + 1;
	case Up:
		if((index / max_map_x) == 0)
			return Blocked;
		return index - max_map_x;
	case Down:
		if((index / max_map_x) >= max_map_y - 1)
			return Blocked;
		return index + max_map_x;
	case LeftUp:
		if((index%max_map_x) == 0)
			return Blocked;
		if((index / max_map_x) == 0)
			return Blocked;
		return index - 1 - max_map_x;
	case LeftDown:
		if((index%max_map_x) == 0)
			return Blocked;
		if((index / max_map_x) >= max_map_y - 1)
			return Blocked;
		return index - 1 + max_map_x;
	case RightUp:
		if((index%max_map_x) >= max_map_x - 1)
			return -1;
		if((index / max_map_x) == 0)
			return Blocked;
		return index + 1 - max_map_x;
	case RightDown:
		if((index%max_map_x) >= max_map_x - 1)
			return Blocked;
		if((index / max_map_x) >= max_map_y - 1)
			return Blocked;
		return index + 1 + max_map_x;
	case Center:
		return index;
	default:
		return Blocked;
	}
}

void game::drop(short unsigned i, item object) {
	if(!object)
		return;
	auto p = grounditems.add();
	p->object = object;
	p->index = i;
}

creature* game::add(short unsigned index, creature* element) {
	if(!element)
		return 0;
	index = getfree(index);
	if(index == Blocked)
		return 0;
	element->move(index);
	return element;
}

int game::distance(short unsigned i1, short unsigned i2) {
	auto dx = (int)getx(i1) - (int)getx(i2);
	auto dy = (int)gety(i1) - (int)gety(i2);
	return isqrt(dx*dx + dy * dy);
}

int game::getitems(item** result, unsigned maximum_count, short unsigned index) {
	auto pb = result;
	auto pe = pb + maximum_count - 1;
	for(auto& li : grounditems) {
		if(!li.object || li.index != index)
			continue;
		if(pb < pe)
			*pb++ = (item*)&li.object;
	}
	*pb = 0;
	return pb - result;
}

short unsigned game::getstepto(short unsigned index) {
	auto current_index = Blocked;
	auto current_value = Blocked;
	for(auto d : all_aroud) {
		auto i = to(index, d);
		if(i >= BlockedCreature)
			continue;
		if(movements[i] < current_value) {
			current_value = movements[i];
			current_index = i;
		}
	}
	return current_index;
}

short unsigned game::getstepfrom(short unsigned index) {
	auto current_index = Blocked;
	auto current_value = 0;
	for(auto d : all_aroud) {
		auto i = to(index, d);
		if(i == Blocked || movements[i] >= BlockedCreature)
			continue;
		if(movements[i] > current_value) {
			current_value = movements[i];
			current_index = i;
		}
	}
	return current_index;
}

void game::makewave(short unsigned index, bool(*proc)(short unsigned)) {
	memset(movements, 0xFFFFFFFF, sizeof(movements));
	if(index == Blocked)
		return;
	if(true)
		creature::setblocks(movements, BlockedCreature);
	auto start = index;
	short unsigned push = 0;
	short unsigned pop = 0;
	stack[push++] = start;
	movements[start] = 0;
	while(push != pop) {
		auto n = stack[pop++];
		auto w = movements[n] + 1;
		for(auto d : all_aroud) {
			auto i = to(n, d);
			if(movements[i] == BlockedCreature || !proc(i))
				continue;
			if(movements[i] == Blocked || movements[i] > w) {
				movements[i] = w;
				stack[push++] = i;
			}
		}
	}
}

short unsigned game::getmovement(short unsigned index) {
	return movements[index];
}

void game::looktarget(short unsigned index) {
	logs::add("Цель: ");
	auto p = creature::getcreature(index);
	if(p) {
		logs::add(p->getname());
		return;
	}
	logs::add("%L1", getstr(getobject(index)));
}

void game::lookhere(short unsigned index) {
	char temp[260];
	item* source[32];
	if(!is(index, Explored)) {
		logs::add("Это место надо исследовать.");
		return;
	}
	logs::add(creature::getname(gettile(index)));
	switch(getobject(index)) {
	case NoTile: break;
	case Tree:
		logs::add("Выское дерево выросло прямо посредине.");
		break;
	case Door:
		if(is(index, Opened))
			logs::add("Здесь находится дверь.");
		else {
			logs::add("Здесь находится закрытая дверь.");
			if(is(index, Sealed))
				logs::add("На двери висит замок.");
		}
		break;
	case Trap:
		if(!is(index, Hidden))
			logs::add("Здесь находится %1.", getstr(gettrap(index)));
		break;
	}
	auto pc = creature::getcreature(index);
	if(pc)
		logs::add("%1 стоит здесь.", pc->getname());
	auto item_count = getitems(source, sizeof(source) / sizeof(source[0]), index);
	if(item_count > 0) {
		logs::addnc("Внизу лежит");
		for(int i = 0; i < item_count; i++) {
			if(i) {
				if(i == item_count - 1)
					logs::addnc("и");
				else
					logs::addnc(",");
			}
			source[i]->getname(temp, zendof(temp));
			szlower(temp, 1);
			logs::addnc(temp);
		}
		logs::add(".");
	}
}

static short unsigned compare_index;

int compare_creature_by_distance(const void* p1, const void* p2) {
	creature* e1 = *((creature**)p1);
	creature* e2 = *((creature**)p2);
	auto d1 = game::distance(compare_index, e1->getposition());
	auto d2 = game::distance(compare_index, e2->getposition());
	return d1 - d2;
}

creature* game::getnearest(aref<creature*> source, short unsigned position) {
	if(!source)
		return 0;
	creature* result = 0;
	auto result_distance = max_map_x * 2;
	for(auto p : source) {
		auto d = distance(p->getposition(), position);
		if(d < result_distance) {
			result = p;
			result_distance = d;
		}
	}
	return result;
}

site* game::getlocation(short unsigned i) {
	if(i == Blocked)
		return 0;
	point pt; pt.x = getx(i); pt.y = gety(i);
	for(auto& e : sites) {
		if(!e)
			continue;
		if(pt.x >= e.x1 && pt.x < e.x2 && pt.y>=e.y1 && pt.y<e.y2)
			return &e;
	}
	return 0;
}

char* site::getname(char* result) const {
	zcpy(result, getstr(type));
	return result;
}

template<> void archive::set<site>(site& e) {
	set(e.type);
	set(e.x1); set(e.y1); set(e.x2); set(e.y2);
	set(e.diety);
	set(e.name);
	set(e.owner);
}

archive::dataset creature_dataset();
void creature_serialize(archive& e);

void game::initialize(short unsigned index, int level, tile_s tile) {
	memset(mpobj, 0, sizeof(mpobj));
	memset(mprnd, 0, sizeof(mprnd));
	memset(mpflg, 0, sizeof(mpflg));
	for(auto& e : mptil)
		e = tile;
	sites.clear();
	grounditems.clear();
	creature::initialize();
	statistic = areainfo();
	statistic.index = index;
	statistic.level = level;
}

bool game::serialize(bool writemode) {
	char temp[260];
	zcpy(temp, "maps/D");
	sznum(zend(temp), statistic.level, 2, "XX");
	sznum(zend(temp), statistic.index, 5, "00000");
	zcat(temp, ".dat");
	io::file file(temp, writemode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive::dataset pointers[] = {creature_dataset(), sites};
	archive a(file, writemode, pointers);
	if(!a.signature("SAV"))
		return false;
	if(!a.version(1, 0))
		return false;
	a.set(game::statistic);
	a.setr(mpflg);
	a.setr(mptil);
	a.setr(mpobj);
	a.setr(mprnd);
	a.set(grounditems);
	a.set(sites);
	creature_serialize(a);
	return true;
}

bool game::serializew(bool writemode) {
	io::file file("maps/worldmap.dat", writemode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, writemode);
	if(!a.signature("WMP"))
		return false;
	if(!a.version(0, 1))
		return false;
	a.setr(mpflg);
	a.setr(mptil);
	a.setr(mpobj);
	a.setr(mprnd);
	return true;
}