#include "archive.h"
#include "main.h"

// ABOUT TIME
// Each minute has 12 segments, and each segment if 6 seconds duration.
// Attack duration is 12 segments - speed of weapon - dexterity speed bonus but minimum of 3 segments

areainfo				game::statistic;
int						isqrt(int num);
static unsigned			start_year;
unsigned				segments = 7 * Hour;
static unsigned			exit_index;
static creature			creature_data[1024];
adat<location, 128>		locations;
adat<creature*, 512>	creatures;
adat<groundinfo, 2048>	grounditems;
tile_s					location_type;
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

void* creature::operator new(unsigned size) {
	for(auto& e : creature_data) {
		if(!e)
			return &e;
	}
	return creature_data;
}

void areainfo::clear() {
	memset(this, 0, sizeof(*this));
}

bool game::isbooming() {
	return creatures.count >= sizeof(creatures.data) / sizeof(creatures.data[0]);
}

bool game::isvisible(short unsigned i) {
	return (mpflg[i] & (1 << Visible)) != 0;
}

bool game::ishidden(short unsigned i) {
	return (mpflg[i] & (1 << Hidden)) != 0;
}

bool game::isopen(short unsigned i) {
	return (mpflg[i] & (1 << Opened)) != 0;
}

bool game::isseal(short unsigned i) {
	return (mpflg[i] & (1 << Sealed)) != 0;
}

bool game::isexplore(short unsigned i) {
	return (mpflg[i] & (1 << Explored)) != 0;
}

bool game::isexperience(short unsigned i) {
	return (mpflg[i] & (1 << Experience)) != 0;
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

unsigned getminute() {
	return (segments / Minute) % 60;
}

unsigned gethour() {
	return (segments / Hour) % 24;
}

unsigned getday() {
	return (segments / Day) % 30;
}

unsigned getmonth() {
	return (segments / (Day * 30)) % 12;
}

unsigned getyear() {
	return start_year + (segments / Year);
}

unsigned getturn() {
	return segments / Minute;
}

static char* zadd(char* result, const char* format, ...) {
	szprintv(zend(result), format, xva_start(format));
}

const char* getstrfdat(char* result, unsigned segments, bool show_time) {
	auto v = getday();
	result[0] = 0;
	if(v)
		szprint(result, "День %1i", v);
	if(result[0])
		zcat(result, ", ");
	szprint(zend(result), "%1i часов %2i минут", gethour(), getminute());
	return result;
}

void game::initialize() {
	memset(mpobj, 0, sizeof(mpobj));
	memset(mprnd, 0, sizeof(mprnd));
	for(auto& e : mptil)
		e = Plain;
	creatures.clear();
	locations.clear();
	grounditems.clear();
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
	return gettile() == Floor;
}

tile_s game::gettile() {
	return location_type;
}

tile_s game::gettile(short unsigned i) {
	if(i == 0xFFFF)
		return NoTile;
	return mptil[i];
}

int game::getrand(short unsigned i) {
	return mprnd[i];
}

void game::set(short unsigned i, tile_s value) {
	if(i == 0xFFFF)
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
	for(auto y = i; y != 0xFFFF && h > 0; y = to(y, Down), h--) {
		auto w1 = w;
		for(auto x = y; x != 0xFFFF && w1 > 0; x = to(x, Right), w1--)
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
				if(getcreature(i1))
					continue;
				return i1;
			}
		}
	}
	return 0xFFFF;
}

void game::set(short unsigned i, map_object_s value) {
	if(i == 0xFFFF)
		return;
	mpobj[i] = value;
}

void game::set(short unsigned i, unsigned char value) {
	if(i == 0xFFFF)
		return;
	mprnd[i] = value;
}

bool game::ispassable(short unsigned i) {
	if(i == 0xFFFF)
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
		return isopen(i);
	default:
		break;
	}
	return true;
}

bool game::ispassabledoor(short unsigned i) {
	if(i == 0xFFFF)
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
		return !isseal(i);
	default:
		break;
	}
	return true;
}

bool game::ispassablelight(short unsigned i) {
	if(i == 0xFFFF)
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
		return isopen(i);
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
	default: return Center;
	}
}

short unsigned game::to(short unsigned index, int id) {
	switch(id) {
	case Left:
		if((index%max_map_x) == 0)
			return 0xFFFF;
		return index - 1;
	case Right:
		if((index%max_map_x) >= max_map_x - 1)
			return 0xFFFF;
		return index + 1;
	case Up:
		if((index / max_map_x) == 0)
			return 0xFFFF;
		return index - max_map_x;
	case Down:
		if((index / max_map_x) >= max_map_y - 1)
			return 0xFFFF;
		return index + max_map_x;
	case LeftUp:
		if((index%max_map_x) == 0)
			return 0xFFFF;
		if((index / max_map_x) == 0)
			return 0xFFFF;
		return index - 1 - max_map_x;
	case LeftDown:
		if((index%max_map_x) == 0)
			return 0xFFFF;
		if((index / max_map_x) >= max_map_y - 1)
			return 0xFFFF;
		return index - 1 + max_map_x;
	case RightUp:
		if((index%max_map_x) >= max_map_x - 1)
			return -1;
		if((index / max_map_x) == 0)
			return 0xFFFF;
		return index + 1 - max_map_x;
	case RightDown:
		if((index%max_map_x) >= max_map_x - 1)
			return 0xFFFF;
		if((index / max_map_x) >= max_map_y - 1)
			return 0xFFFF;
		return index + 1 + max_map_x;
	case Center:
		return index;
	default:
		return 0xFFFF;
	}
}

static void update_players() {
	auto pb = players.data;
	for(auto p : players) {
		if(!p || !(*p))
			continue;
		*pb++ = p;
	}
	players.count = pb - players.data;
}

static void update_links() {
	for(auto p : creatures) {
		if(!p || !(*p))
			continue;
		if(p->enemy && !(*p->enemy))
			p->enemy = 0;
		if(p->charmer && !(*p->charmer))
			p->charmer = 0;
		if(p->horror && !(*p->horror))
			p->horror = 0;
		if(p->leader && !(*p->leader)) {
			if(p->leader->isplayer() && players.count)
				players.data[0]->setplayer();
			else
				p->leader = 0;
		}
	}
}

static void update_creatures() {
	auto pb = creatures.data;
	for(auto p : creatures) {
		if(!p || !(*p))
			continue;
		*pb++ = p;
	}
	creatures.count = pb - creatures.data;
}

static void update_visibllity() {
	auto max_count = max_map_x * max_map_y;
	for(auto i = 0; i < max_count; i++)
		game::set(i, Visible, false);
	for(auto e : players)
		e->setlos();
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
	if(index == 0xFFFF)
		return 0;
	creatures.add(element);
	element->move(index);
	return element;
}

creature* game::add(short unsigned index, class_s type, race_s race, gender_s gender, bool is_player) {
	auto p = new creature(race, gender, type);
	add(index, p);
	if(is_player)
		p->setplayer();
	return p;
}

static int getpartycount() {
	return players.count;
}

void game::play() {
	while(true) {
		exit_index = 0xFFFF;
		turn();
		segments++;
		if(!players.count) {
			// Все погибли
			logs::add("Все ваши персонажи мертвы.");
			logs::next();
			break;
		}
		if(exit_index != 0xFFFF) {
			serialize(true);
			// Кто-то активировал переход на другой уровень
			// Загрузим карту следующего уровня и сохраним состояние этого
		}
	}
}

void game::turn() {
	update_players();
	update_links();
	update_creatures();
	update_visibllity();
	for(auto e : creatures) {
		if(!(*e)) // Если уже убиты, но еще не удалены из списка
			continue;
		e->update();
		if(e->recoil > segments)
			continue;
		e->makemove();
		if(e->recoil <= segments)
			e->recoil = segments + 1;
	}
	update_players();
	update_links();
	update_creatures();
}

creature* game::getcreature(short unsigned index) {
	for(auto e : creatures) {
		if(e->position == index)
			return e;
	}
	return 0;
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
	auto current_index = 0xFFFF;
	auto current_value = 0xFFFF;
	for(auto d : all_aroud) {
		auto i = to(index, d);
		if(i == 0xFFFF)
			continue;
		if(movements[i] < current_value) {
			current_value = movements[i];
			current_index = i;
		}
	}
	return current_index;
}

short unsigned game::getstepfrom(short unsigned index) {
	auto current_index = 0xFFFF;
	auto current_value = 0;
	for(auto d : all_aroud) {
		auto i = to(index, d);
		if(i == 0xFFFF || movements[i] == 0xFFFF)
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
	if(index == 0xFFFF)
		return;
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
			if(!proc(i))
				continue;
			if(movements[i] == 0xFFFF || movements[i] > w) {
				movements[i] = w;
				stack[push++] = i;
			}
		}
	}
}

short unsigned game::getmovement(short unsigned index) {
	return movements[index];
}

bool logs::getcreature(const creature& e, creature** result, target_s target, int range) {
	creature* source[64];
	auto count = e.getcreatures(source, sizeof(source) / sizeof(source[0]), target, range);
	if(!count) {
		if(e.isplayer())
			logs::add("Вокруг никого нет.");
		return false;
	}
	short unsigned source_index[64];
	for(int i = 0; source[i] && i < sizeof(source) / sizeof(source[0]); i++)
		source_index[i] = source[i]->position;
	auto pn = logs::choose(e, source_index, count);
	auto pi = zfind(source_index, pn);
	if(pi == -1)
		return false;
	*result = source[pi];
	return true;
}

bool logs::getindex(const creature& e, short unsigned& result, target_s target, int range) {
	short unsigned source[7 * 7];
	auto count = e.getobjects(source, sizeof(source) / sizeof(source[0]), target, range);
	if(!count) {
		if(e.isplayer())
			logs::add("Вокруг нет подходящих объектов.");
		return false;
	}
	auto r = logs::choose(e, source, count);
	if(r == 0xFFFF)
		return false;
	result = r;
	return true;
}

bool logs::getitem(const creature& e, item** result, target_s target) {
	item* source[64];
	auto count = e.getitems(source, sizeof(source) / sizeof(source[0]), target);
	if(!count) {
		if(e.isplayer())
			logs::add("У вас нет предметов нужного вида.");
		return false;
	}
	auto pn = logs::choose(e, source, count, "Выбирайте предмет");
	if(!pn)
		return false;
	*result = pn;
	return true;
}

void game::looktarget(short unsigned index) {
	logs::add("Цель: ");
	auto p = game::getcreature(index);
	if(p) {
		logs::add(p->getname());
		return;
	}
	switch(getobject(index)) {
	case Tree: logs::add("Дерево"); break;
	case Door: logs::add("Дверь"); break;
	case Trap: logs::add("Ловушка"); break;
	}
}

void game::lookhere(short unsigned index) {
	char temp[260];
	item* source[32];
	if(!isexplore(index)) {
		logs::add("Это место надо исследовать.");
		return;
	}
	switch(gettile(index)) {
	case Wall: logs::add("Стена преграждает путь."); break;
	case Plain: logs::add("Равнина расположилась вокруг."); break;
	case Hill: logs::add("Невысокие бугорки затрудняют движение."); break;
	case Swamp: logs::add("Большая лужа находится на траве."); break;
	}
	switch(getobject(index)) {
	case Tree:
		logs::add("Здесь растет дерево.");
		break;
	case Door:
		logs::add("Здесь находится дверь и она %1.", isopen(index) ? "открыта" : "закрыта");
		if(isseal(index))
			logs::add("На двери висит замок.");
		break;
	case Trap:
		if(!ishidden(index))
			logs::add("Здесь находится %1.", getstr(gettrap(index)));
		break;
	}
	auto pc = getcreature(index);
	if(pc)
		logs::add("%1 стоит здесь.", pc->getname());
	auto item_count = getitems(source, sizeof(source) / sizeof(source[0]), index);
	if(item_count > 0) {
		logs::add("Внизу лежит");
		for(int i = 0; i < item_count; i++) {
			if(i) {
				if(i == item_count - 1)
					logs::add("и");
				else
					logs::add(",");
			}
			source[i]->getname(temp);
			szlower(temp, 1);
			logs::add(temp);
		}
		logs::add(".");
	}
}

static short unsigned compare_index;

int compare_creature_by_distance(const void* p1, const void* p2) {
	creature* e1 = *((creature**)p1);
	creature* e2 = *((creature**)p2);
	auto d1 = game::distance(compare_index, e1->position);
	auto d2 = game::distance(compare_index, e2->position);
	return d1 - d2;
}

creature* game::getnearest(creature** source, unsigned count, short unsigned position) {
	compare_index = position;
	qsort(source, count, sizeof(source[0]), compare_creature_by_distance);
	return source[0];
}

location* game::getlocation(short unsigned i) {
	point pt;
	pt.x = getx(i);
	pt.y = gety(i);
	for(auto& e : locations) {
		if(!e)
			continue;
		if(pt.in(e))
			return &e;
	}
	return 0;
}

void game::release(const creature* player, unsigned exeperience_cost) {
	// Начислим опыт, всем, кто считал его врагом
	for(auto e : creatures) {
		if(!*e)
			continue;
		if(e->enemy == player) {
			e->addexp(exeperience_cost);
			e->enemy = 0;
		}
		if(e->horror == player)
			e->horror = 0;
		if(e->charmer == player)
			e->charmer = 0;
		//if(e->leader == player)
		//	e->leader = 0;
	}
}

char* location::getname(char* result) const {
	zcpy(result, getstr(type));
	return result;
}

template<> void archive::set<location>(location& e) {
	set(e.type);
	set(e.x1); set(e.y1); set(e.x2); set(e.y2);
	set(e.diety);
	set(e.name);
	set(e.owner);
}

template<> void archive::set<creature>(creature& e) {
	set(e.race);
	set(e.type);
	set(e.gender);
	set(e.role);
	set(e.direction);
	set(e.abilities);
	set(e.skills);
	set(e.spells);
	set(e.name);
	set(e.level);
	set(e.hp);
	set(e.mhp);
	set(e.mp);
	set(e.mmp);
	set(e.recoil);
	set(e.restore_hits);
	set(e.restore_mana);
	set(e.experience);
	set(e.money);
	set(e.position);
	set(e.guard);
	set(e.states);
	set(e.wears);
	set(e.backpack);
	//set(e.charmer);
	//set(e.enemy);
	//set(e.horror);
	//set(e.leader);
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
	archive a(file, writemode);
	if(!a.signature("SAV"))
		return false;
	if(!a.version(0, 1))
		return false;
	a.set(locations);
	a.set(creatures);
	return true;
}