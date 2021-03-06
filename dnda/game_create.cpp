#include "main.h"

using namespace game;

const int chance_generate_room = 40;
const int chance_special_area = 5;
const int chance_corridor_content = 10;
const int chance_door_closed = 30;
const int dense_forest = 15;
const int max_building_size = 15;
typedef adat<rect, 64> rooma;

struct vector {
	short unsigned		index;
	direction_s			dir;
	char				chance;
};
extern adat<site, 128>	sites;
static vector			rooms[256];
static unsigned char	stack_put, stack_get;
static direction_s		connectors_side[] = {Up, Left, Right, Down};
static slot_s slots_weapons_armor[] = {Melee, Ranged, OffHand, Head, Elbows, Legs, Torso};
static item_s item_treasure[] = {Coin, Coin, Coin, Coin, RingRed};
static item_s item_food[] = {Ration, Ration, Ration, BreadEvlen, BreadHalflings, BreadDwarven, Sausage};
static item_s item_potion_scrolls[] = {Scroll1, Scroll2, Scroll3,
Scroll1, Scroll2, Scroll3,
Book1, Book2, Book3, Book4, Book5,
Amulet1, Amulet2, Amulet3, Amulet4, Amulet5,
Boot1, Boot2, IronBoot1, IronBoot2, IronBoot3,
Potion1, Potion2, Potion3, Potion4, Potion5,
Wand1, Wand2, Wand3, Wand4, Wand5,
RingRed, RingGreen, RingBlue};

template<class T> static const T* findid(aref<T> source, const char* id) {
	for(auto& e : source) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

static void show_minimap_step(short unsigned index, bool visualize) {
	if(visualize)
		logs::minimap(index);
}

static void create_objects(int x, int y, int w, int h, int count, map_object_s object) {
	for(int i = 0; i < count; i++) {
		int x1 = xrand(x, x + w);
		int y1 = xrand(y, y + h);
		set(get(x1, y1), object);
	}
}

static void create_objects(int x, int y, int w, int h, int count, tile_s object) {
	for(int i = 0; i < count; i++) {
		int x1 = xrand(x, x + w);
		int y1 = xrand(y, y + h);
		set(get(x1, y1), object);
	}
}

static short unsigned find(int x, int y, int w, int h, map_object_s value) {
	int x2 = x + w;
	int y2 = y + h;
	for(int y1 = y; y1 < y2; y1++) {
		for(int x1 = x; x1 < x2; x1++) {
			auto i = get(x1, y1);
			if(getobject(i) == value)
				return i;
		}
	}
	return Blocked;
}

static void ellipse(int x0, int y0, int x1, int y1, tile_s object) {
	int a = iabs(x1 - x0), b = iabs(y1 - y0), b1 = b & 1;
	long dx = 4 * (1 - a)*b*b, dy = 4 * (b1 + 1)*a*a;
	long err = dx + dy + b1 * a*a, e2; /* error of 1.step */
	if(x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
	if(y0 > y1) y0 = y1; /* .. exchange them */
	y0 += (b + 1) / 2; y1 = y0 - b1;   /* starting pixel */
	a *= 8 * a; b1 = 8 * b*b;
	do {
		set(get(x0, y0), object, x1 - x0, 1);
		set(get(x0, y1), object, x1 - x0, 1);
		e2 = 2 * err;
		if(e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
		if(e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; } /* x step */
	} while(x0 <= x1);
	while(y0 - y1 < b) {  /* too early stop of flat ellipses a=1 */
		set(get(x0 - 1, y0), object); /* -> finish tip of ellipse */
		set(get(x1 + 1, y0++), object);
		set(get(x0 - 1, y1), object);
		set(get(x1 + 1, y1--), object);
	}
}

static int bpoint(short unsigned index, int width, int height, direction_s dir) {
	int x = game::getx(index);
	int y = game::gety(index);
	switch(dir) {
	case Left: return game::get(x, y + xrand(1, height - 2));
	case Right: return game::get(x + width - 1, y + xrand(1, height - 2));
	case Up: return game::get(x + xrand(1, width - 2), y);
	default: return game::get(x + xrand(1, width - 2), y + height - 1);
	}
}

static short unsigned center(int x, int y, int w, int h) {
	return get(x + w / 2, y + (h / 2));
}

short unsigned center(const rect& rc) {
	return get(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2);
}

static short unsigned random(int x, int y, int w, int h) {
	return getfree(get(xrand(x, x + w - 1), xrand(y, y + h - 1)));
}

static direction_s direction(short unsigned i1, short unsigned i2) {
	auto x1 = game::getx(i1);
	auto y1 = game::gety(i1);
	auto x2 = game::getx(i2);
	auto y2 = game::gety(i2);
	auto w = iabs(x1 - x2);
	auto h = iabs(y1 - y2);
	if(h < w) {
		if(y1 < y2)
			return Down;
		return Up;
	} else {
		if(x1 < x2)
			return Right;
		return Left;
	}
}

// Set horizontal wall in interior room
static short unsigned setiwh(int x, int y, int s, tile_s o, map_object_s r, bool locked_doors) {
	if(s <= 2)
		return Blocked;
	set(get(x, y), o, s, 1);
	auto i = get(x + xrand(1, s - 2), y);
	set(i, Floor);
	set(i, Floor);
	set(i, r);
	set(i, Opened, false);
	if(locked_doors)
		set(i, Sealed, true);
	return i;
}

// Set vertical wall in interior room
static short unsigned setiwv(int x, int y, int s, tile_s o, map_object_s r, bool locked_doors) {
	if(s <= 2)
		return Blocked;
	set(get(x, y), o, 1, s);
	auto i = get(x, y + xrand(1, s - 2));
	set(i, Floor);
	set(i, r);
	set(i, Opened, false);
	if(locked_doors)
		set(i, Sealed, true);
	return i;
}

static short unsigned create_building(int x, int y, int w, int h) {
	static direction_s rdir[] = {Right, Left, Up, Down};
	// ����������� ���������� � ������
	x = imin(imax(1, x), max_map_x - w - 1);
	y = imin(imax(1, y), max_map_y - h - 1);
	auto i = get(x, y);
	// ����� � ���
	set(i, Wall, w, h);
	set(to(i, RightDown), Floor, w - 2, h - 2);
	// �����
	auto dir = maprnd(rdir);;
	auto door = bpoint(i, w, h, dir);
	set(door, Floor);
	set(door, Door);
	auto m1 = to(door, dir);
	auto t1 = gettile(m1);
	if(t1 != Plain && t1 != Road)
		set(m1, Plain);
	set(m1, NoTileObject);
	return door;
}

static void create_lake(int x, int y, int w, int h) {
	int w2 = w / 2;
	int h2 = h / 2;
	for(int i = 0; i < 5; i++) {
		int x1 = x + xrand(0, w2);
		int y1 = y + xrand(0, h2);
		int w1 = w2 - xrand(0, 2);
		int h1 = h2 - xrand(0, 2);
		ellipse(x1, y1, x1 + w1, y1 + h1, Water);
	}
}

static void create_forest(int x, int y, int w, int h) {
	create_objects(x, y, w, h, (w*h*dense_forest) / 100, Tree);
}

static void create_road(int x, int y, int w, int h) {
	if(h <= 4) {
		if(x <= 3) {
			w += x;
			x = 0;
		} else if(x + w >= max_map_x - 3)
			w = max_map_x - x;
	} else {
		if(y <= 3) {
			h += y;
			y = 0;
		} else if(y + h >= max_map_y - 3)
			h = max_map_y - y;
	}
	set(get(x, y), Road, w, h);
}

static void create_commoner(short unsigned index) {
	static role_s commoners[] = {HumanMale, HumanMale, HumanFemale, HumanChild};
	creature::add(index, maprnd(commoners));
}

static creature* create_shopkeeper(site& e, short unsigned index) {
	e.owner = creature::add(index, Shopkeeper);
	return e.owner;
}

static creature* create_adventurer(short unsigned index) {
	return creature::add(index, Human, (gender_s)xrand(Male, Female), (class_s)xrand(Fighter, Mage));
}

static creature* create_bartender(site& e, short unsigned index) {
	e.owner = creature::add(index, Shopkeeper);
	return e.owner;
}

static void create_monster(short unsigned index) {
	game::spawn(index);
}

static creature* create_priest(site& e, short unsigned index) {
	e.diety = (diety_s)xrand(GodBane, GodTyr);
	switch(e.diety) {
	case GodGruumsh:
		e.owner = creature::add(index, Dwarf, Male, Cleric);
		break;
	default:
		e.owner = creature::add(index, Human, Male, Cleric);
		break;
	}
	return e.owner;
}

static item_s random(aref<item_s> source) {
	if(!source)
		return NoItem;
	return source.data[rand() % source.count];
}

static item_s random(aref<slot_s> source) {
	item_s result[128];
	return random(item::getitems(result, source));
}

static void create_item(short unsigned index, item_s type, int level, bool forsale, bool identify = false, char chance_curse = 20) {
	if(type == NoItem)
		return;
	auto chance_artifact = imax(0, level / 4);
	auto chance_quality = imax(0, 40 + level);
	auto chance_magic = imax(0, 5 + level);
	auto it = item(type, chance_artifact, chance_magic, chance_curse, chance_quality);
	it.setidentify(identify);
	if(type == Coin) {
		auto count = xrand(1, 5) * level;
		if(count > 64)
			count = 64;
		it.setcount(count);
	}
	if(forsale)
		it.setforsale();
	if(it.isartifact())
		statistic.artifacts++;
	drop(index, it);
}

static void create_door(short unsigned index) {
	set(index, Door);
	if(d100() < chance_door_closed)
		set(index, Sealed, true);
}

static void create_shop(int x0, int y0, int w, int h, int chance, unsigned char level, bool forsale, aref<item_s> source, int chance_curse = 0, identify_s identify = KnowEffect) {
	if(!source)
		return;
	for(auto y = y0; y < y0 + h; y++) {
		if(y < 0 || y >= max_map_y)
			continue;
		for(auto x = x0; x < x0 + w; x++) {
			if(x < 0 || x >= max_map_x)
				continue;
			if(d100() < chance)
				create_item(get(x, y), source.data[rand() % source.count], level, forsale, identify, chance_curse);
		}
	}
}

static void create_objects(site& e, int x, int y, int w, int h, site_s type) {
	if(w < 1 || h < 1)
		return;
	auto pt = center(x, y, w, h);
	item_s source[128];
	switch(type) {
	case Temple:
		create_priest(e, pt);
		break;
	case Tavern:
		create_bartender(e, pt);
		for(auto i = xrand(1, 3); i > 0; i--)
			create_adventurer(pt);
		break;
	case ShopWeaponAndArmor:
		create_shopkeeper(e, pt);
		create_shop(x, y, w, h, 90, 10, true, item::getitems(source, slots_weapons_armor));
		break;
	case ShopPotionAndScrolls:
		create_shopkeeper(e, pt);
		create_shop(x, y, w, h, 90, 10, true, item_potion_scrolls);
		break;
	case ShopFood:
		create_shopkeeper(e, pt);
		create_shop(x, y, w, h, 90, 10, true, item_food, 25, Unknown);
		break;
	case TreasureRoom:
		create_shop(x, y, w, h, 80, 10, false, item_treasure);
		break;
	case Barracs:
		for(auto i = xrand(2, 4); i > 0; i--)
			creature::add(pt, HumanGuard);
		break;
	case CityHall:
		break;
	}
}

static void create_interior(site& e, int x, int y, int w, int h, short unsigned i, site_s type) {
	if(w < 5 && h < 5) {
		create_objects(e, x + 1, y + 1, w - 2, h - 2, type);
		return;
	}
	if(i == 0xFFFF)
		return;
	int dx = getx(i);
	int dy = gety(i);
	int x1 = x, y1 = y, w1 = w, h1 = h;
	int x2 = x, y2 = y, w2 = w, h2 = h;
	if(w > h && w >= 5) {
		auto wp = x + w / 2 + 1 - (rand() % 3);
		// ����� ����� ������� ����� �� �����
		if(wp == dx) {
			if(wp <= (x + w / 2))
				wp++;
			else
				wp--;
		}
		if((wp - x) < 2 || (x + w - wp) <= 2)
			return;
		i = setiwv(wp, y, h, Wall, Door, true);
		if(dx < wp) {
			x1 = wp;
			w1 = (x + w) - wp;
			w2 = wp - x + 1;
		} else {
			x2 = wp;
			w1 = wp - x;
			w2 = (x + w) - wp;
		}
	} else if(h >= 5) {
		auto wp = y + h / 2 + 1 - (rand() % 3);
		if(wp == dy) {
			if(wp <= (y + h / 2))
				wp++;
			else
				wp--;
		}
		if((wp - y) < 2 || (y + h - wp) <= 2)
			return;
		i = setiwh(x, wp, w, Wall, Door, true);
		if(dy < wp) {
			y1 = wp;
			h1 = (y + h) - wp;
			h2 = wp - y + 1;
		} else {
			y2 = wp;
			h1 = wp - y;
			h2 = (y + h) - wp;
		}
	}
	create_objects(e, x2 + 1, y2 + 1, w2 - 2, h2 - 2, type);
	switch(type) {
	case ShopPotionAndScrolls:
	case ShopWeaponAndArmor:
		create_interior(e, x1, y1, w1, h1, i, TreasureRoom);
		break;
	default:
		create_interior(e, x1, y1, w1, h1, i, EmpthyRoom);
		break;
	}
}

static void create_content(site& e, site_s type) {
	e.type = type;
	auto door = create_building(e.x1, e.y1, e.width(), e.height());
	create_interior(e, e.x1, e.y1, e.width(), e.height(), door, e.type);
}

static void create_dungeon_item(short unsigned index) {
	create_item(index, random(slots_weapons_armor), statistic.level, false);
}

static void create_trap(short unsigned index) {
	set(index, Trap);
	set(index, Hidden, true);
}

static void create_treasure(short unsigned index) {
	create_item(index, Coin, statistic.level, false);
}

static void create_corridor_content(short unsigned index) {
	typedef void(*proc)(short unsigned index);
	proc chances[] = {create_trap,
		create_treasure, create_treasure,
		//create_door,
		create_monster, create_monster,
		create_dungeon_item, create_dungeon_item,
	};
	maprnd(chances)(index);
}

static void create_city_level(int x, int y, int w, int h, int level, rooma& rooms) {
	if(w > max_building_size && w<max_building_size * 3
		&& h>max_building_size && h < max_building_size * 3
		&& d100() < chance_special_area) {
		switch(rand() % 4) {
		case 1:
			create_forest(x, y, w - 3, h - 3);
			break;
		default:
			create_lake(x + rand() % 3, y + rand() % 3, w - 3 * 2, h - 3 * 2);
			break;
		}
		return;
	}
	if(w <= max_building_size && h <= max_building_size) {
		if(w > 6 && h > 6)
			rooms.add({x, y, x + w - 3, y + h - 3});
		return;
	}
	// Calculate direction
	int m = xrand(30, 60);
	int r = -1;
	if(w / 3 >= h / 2)
		r = 0;
	else if(h / 3 >= w / 2)
		r = 1;
	if(r == -1)
		r = (d100() < 50) ? 0 : 1;
	if(r == 0) {
		int w1 = (w*m) / 100; // horizontal
		create_city_level(x, y, w1, h, level + 1, rooms);
		create_city_level(x + w1, y, w - w1, h, level + 1, rooms);
		if(level <= 2) {
			if(y == 1) {
				y--;
				h++;
			}
			create_road(x + w1 - 3, y, 3, h);
			for(int i = xrand(3, 6); i >= 0; i--)
				create_commoner(random(x + w1 - 3, y, 3, h));
		}
	} else {
		int h1 = (h*m) / 100; // vertial
		create_city_level(x, y, w, h1, level + 1, rooms);
		create_city_level(x, y + h1, w, h - h1, level + 1, rooms);
		if(level <= 2) {
			if(x == 1) {
				x--;
				w++;
			}
			create_road(x, y + h1 - 3, w, 3);
			for(int i = xrand(2, 4); i >= 0; i--)
				create_commoner(random(x, y + h1 - 3, w, 3));
		}
	}
}

static void change_tile(tile_s t1, tile_s t2) {
	for(short unsigned i = 0; i <= max_map_x * max_map_y; i++) {
		if(gettile(i) == t1)
			set(i, t2);
	}
}

static void put_block(short unsigned index, direction_s dir) {
	auto& e = rooms[stack_put++];
	e.index = index;
	e.dir = dir;
}

static vector& get_block() {
	return rooms[stack_get++];
}

static bool ispassable(short unsigned index, const rect& correct) {
	if(index == Blocked)
		return false;
	auto x = game::getx(index);
	auto y = game::gety(index);
	if(x < correct.x1 || x >= correct.x2 || y < correct.y1 || y >= correct.y2)
		return false;
	return true;
}

static bool isvalidindex(short unsigned i) {
	auto x = getx(i);
	auto y = gety(i);
	if(x == 0 || x == max_map_x - 1 || y == 0 || y == max_map_y - 1)
		return false;
	return true;
}

static bool isvalidsides(short unsigned index, direction_s dir) {
	return gettile(to(index, turn(dir, Left))) == NoTile
		&& gettile(to(index, turn(dir, Right))) == NoTile;
}

static bool ispassable(tile_s tile) {
	return tile == Floor || tile == Road;
}

static bool isvalidcorridor(short unsigned index, direction_s dir) {
	auto i = to(index, dir);
	if(!isvalidindex(i))
		return false;
	auto tile = gettile(i);
	if(ispassable(tile))
		return true;
	if(gettile(i) != NoTile)
		return false;
	if(!isvalidsides(i, dir))
		return false;
	if(!isvalidsides(to(i, dir), dir))
		return false;
	return true;
}

static void create_corridor(short unsigned index, direction_s dir) {
	short unsigned start = Blocked;
	auto iterations = xrand(3, 8);
	auto tile = Floor;
	while(true) {
		auto i0 = to(index, dir); // Forward
		auto i1 = to(i0, dir); // Forward - Forward
		auto i2 = to(i0, turn(dir, Left)); // Forward - Left
		auto i3 = to(i0, turn(dir, Right)); // Forward - Right
		auto i4 = to(i1, turn(dir, Left)); // Forward - Forward - Left
		auto i5 = to(i1, turn(dir, Right)); // Forward - Forward - Right
		if(!ispassable(i0) || gettile(i2) == tile || gettile(i3) == tile)
			break;
		if(gettile(i1) == tile || gettile(i4) == tile || gettile(i5) == tile)
			break;
		if(start == Blocked)
			start = index;
		index = i0;
		set(index, Floor);
		if(d100() < chance_corridor_content)
			create_corridor_content(index);
		if(--iterations == 0)
			break;
	}
	if(start != Blocked) {
		direction_s rnd[] = {Right, Left, Up};
		zshuffle(rnd, 3);
		int count = 1;
		if(d100() < 50)
			count++;
		if(d100() < 20)
			count++;
		for(auto e : rnd) {
			auto e1 = turn(dir, e);
			if(isvalidcorridor(index, e1)) {
				put_block(index, e1);
				if(--count == 0)
					break;
			}
		}
	}
}

static void create_corridor(int x, int y, int w, int h, direction_s dir) {
	switch(dir) {
	case Up: put_block(game::get(x + rand() % w, y), Up); break;
	case Left: put_block(game::get(x, y + rand() % h), Left); break;
	case Down: put_block(game::get(x + rand() % w, y + h - 1), Down); break;
	default: put_block(game::get(x + w - 1, y + rand() % h), Right); break;
	}
}

static void create_room(int x, int y, int w, int h) {
	for(auto x1 = x; x1 < x + w; x1++) {
		for(auto y1 = y; y1 < y + h; y1++) {
			game::set(game::get(x1, y1), Floor);
		}
	}
	auto p = sites.add();
	p->x1 = x;
	p->y1 = y;
	p->x2 = p->x1 + w - 1;
	p->y2 = p->y1 + h - 1;
	p->type = Lair;
}

static int compare_rect(const void* p1, const void* p2) {
	auto e1 = (rect*)p1;
	auto e2 = (rect*)p2;
	return e2->width()*e2->height() - e1->width()*e1->height();
}

static void create_big_rooms(int x, int y, int w, int h, adat<rect, 64>& rooms, bool visualize) {
	if(h < max_building_size * 2 && w < max_building_size * 2) {
		auto dw = xrand(max_building_size - 4 - 6, max_building_size - 4);
		auto dh = xrand(max_building_size - 4 - 6, max_building_size - 4);
		auto x1 = x + 1 + rand() % (w - dw - 2);
		auto y1 = y + 1 + rand() % (h - dh - 2);
		rooms.add({x1, y1, x1 + dw, y1 + dh});
	} else if(w > h) {
		auto w1 = w / 2 + (rand() % 8) - 4;
		create_big_rooms(x, y, w1, h, rooms, visualize);
		create_big_rooms(x + w1, y, w - w1, h, rooms, visualize);
	} else {
		auto h1 = h / 2 + (rand() % 6) - 3;
		create_big_rooms(x, y, w, h1, rooms, visualize);
		create_big_rooms(x, y + h1, w, h - h1, rooms, visualize);
	}
}

static void create_connector(short unsigned index, direction_s dir, const rect& correct) {
	auto iterations = xrand(4, 10);
	auto start = Blocked;
	auto tile = Floor;
	for(; iterations > 0; iterations--) {
		auto i0 = to(index, dir); // Forward
		auto i1 = to(i0, dir); // Forward - Forward
		auto i2 = to(i0, turn(dir, Left)); // Forward - Left
		auto i3 = to(i0, turn(dir, Right)); // Forward - Right
		auto i4 = to(i1, turn(dir, Left)); // Forward - Forward - Left
		auto i5 = to(i1, turn(dir, Right)); // Forward - Forward - Right
		if(!ispassable(i0, correct) || game::gettile(i0) == tile)
			return;
		if(iterations == 1
			&& (game::gettile(i4) == tile || game::gettile(i5) == tile))
			break;
		index = i0;
		set(index, Floor);
		if(start == Blocked) {
			start = index;
			if(d100() < 60)
				game::set(start, Door);
		} else {
			if(d100() < chance_corridor_content)
				create_corridor_content(index);
		}
		if(game::gettile(i2) == tile || game::gettile(i3) == tile)
			return; // Maybe need `break`?
	}
	direction_s rnd[] = {Right, Left, Up};
	zshuffle(rnd, 3);
	int count = 1;
	if(d100() < 50)
		count++;
	if(d100() < 20)
		count++;
	for(auto e : rnd) {
		put_block(index, turn(dir, e));
		if(--count == 0)
			break;
	}
}

static bool iswall(short unsigned i, direction_s d1, direction_s d2) {
	auto t1 = game::gettile(to(i, d1));
	auto t2 = game::gettile(to(i, d2));
	return (t1 == Wall || t1 == NoTile) && (t2 == Wall || t2 == NoTile);
}

static void update_doors() {
	for(short unsigned i = 0; i <= max_map_x * max_map_y; i++) {
		if(getobject(i) == Door) {
			if(iswall(i, Left, Right))
				continue;
			if(iswall(i, Up, Down))
				continue;
			set(i, NoTileObject);
		}
	}
}

static void indoor_floor() {
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, NoTile);
}

static void outdoor_floor() {
	create_objects(0, 0, max_map_x - 1, max_map_y - 1, xrand(10, 20), Hill);
	create_objects(0, 0, max_map_x - 1, max_map_y - 1, xrand(4, 12), Swamp);
	create_objects(0, 0, max_map_x - 1, max_map_y - 1, max_map_x*max_map_y*(dense_forest / 3) / 100, Tree);
}

static void create_dungeon_content(rooma& rooms, bool visualize) {
	rooms.count -= 2; // Two room of lesser size would cutted off
	zshuffle(rooms.data, rooms.count);
	statistic.positions[0] = center(rooms[0]);
	game::set(statistic.positions[0], StairsDown);
	statistic.positions[1] = center(rooms[1]);
	game::set(statistic.positions[1], StairsUp);
	for(auto& e : rooms)
		create_room(e.x1, e.y1, e.width(), e.height());
	for(auto& e : rooms)
		create_corridor(e.x1, e.y1, e.width(), e.height(), maprnd(connectors_side));
	for(auto& e : rooms)
		create_corridor(e.x1, e.y1, e.width(), e.height(), maprnd(connectors_side));
	rect rc = {2, 2, max_map_x - 2, max_map_y - 2};
	while(stack_get != stack_put) {
		auto& e = get_block();
		create_connector(e.index, e.dir, rc);
		show_minimap_step(e.index, visualize);
	}
}

static void create_city(int x, int y, int w, int h, rooma& rooms, bool visualize) {
	create_city_level(x, y, w + x, h + y, 0, rooms);
}

static void create_city_buildings(rooma& rooms, bool visualize) {
	auto current = 1;
	auto max_possible_points = rooms.getcount() / 3;
	if(max_possible_points > 25)
		max_possible_points = 25;
	auto placed_stairs = false;
	for(auto& e : rooms) {
		auto t = (site_s)xrand(Temple, ShopFood);
		if(current > max_possible_points)
			t = House;
		auto p = sites.add();
		*((rect*)p) = e;
		create_content(*p, t);
		if(!placed_stairs && t == House) {
			placed_stairs = true;
			statistic.positions[0] = center(e);
			statistic.positions[0] = getfree(statistic.positions[0]);
			game::set(statistic.positions[0], StairsDown);
		}
		current++;
	}
}

static void create_maze(int x, int y, int w, int h, rooma& rooms, bool visualize) {
	put_block(get(xrand(x + w / 8, x + w / 4), xrand(x + h / 8, x + h / 4)), Right);
	put_block(get(xrand(x + w - w / 4, x + w - 2), xrand(x + h - h / 4, x + h - 2)), Left);
	while(stack_get != stack_put) {
		auto& e = get_block();
		create_corridor(e.index, e.dir);
		show_minimap_step(e.index, visualize);
	}
	change_tile(NoTile, Wall);
}

static void create_dungeon_lairs(rooma& rooms, bool visualize) {
	for(auto& e : rooms) {
		auto t = Lair;
		auto p = sites.add();
		*((rect*)p) = e;
	}
}

static void worldmap_outdoor() {
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, Plain);
}

bool game::create(dungeon_area_s type, short unsigned index, int level, bool explored, bool visualize) {
	static struct dungeon_info {
		const char*	name;
		bool		isdungeon;
		point		offset;
		void(*floor)();
		void(*rooms)(int x, int y, int w, int h, rooma& rooms, bool visualize);
		void(*content)(rooma& rooms, bool visualize);
	} dungeon_area_data[] = {{"��������", true, {1, 1}, indoor_floor, create_maze},
	{"����������", true, {1, 1}, indoor_floor, create_big_rooms, create_dungeon_content},
	{"�������", false, {1, 1}, outdoor_floor},
	{"���", false, {1, 1}, outdoor_floor},
	{"�����", false, {1, 1}, outdoor_floor},
	{"�����", false, {2, 2}, outdoor_floor, create_city, create_city_buildings},
	};
	assert_enum(dungeon_area, AreaCity);
	initialize(index, level, Plain);
	if(!serialize(false)) {
		auto& a = dungeon_area_data[type];
		stack_get = stack_put = 0;
		statistic.index = index;
		statistic.level = level;
		statistic.isdungeon = a.isdungeon;
		if(a.isdungeon) {
			if(statistic.level < 1)
				statistic.level = 1;
		}
		// Set new random values
		auto count = max_map_x * max_map_y;
		for(short unsigned i = 0; i < count; i++)
			set(i, (unsigned char)(rand() % 256));
		// Explore all map
		if(explored) {
			for(short unsigned i = 0; i < count; i++)
				set(i, Explored, true);
		}
		rooma rooms;
		if(a.floor)
			a.floor();
		if(a.rooms) {
			a.rooms(a.offset.x, a.offset.y, max_map_x - 1 - a.offset.x, max_map_y - 1 - a.offset.y, rooms, visualize);
			qsort(rooms.data, rooms.count, sizeof(rooms.data[0]), compare_rect);
		}
		if(a.content)
			a.content(rooms, visualize);
		change_tile(NoTile, Wall);
		update_doors();
		serialize(true);
	}
	return true;
}