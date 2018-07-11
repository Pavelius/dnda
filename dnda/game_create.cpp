#include "main.h"

using namespace game;

struct roominfo {
	short unsigned		index;
	direction_s			dir;
};
enum corridor_content_s {
	DungeonTreasure, DungeonMonster, DungeonDoor, DungeonItem,
	DungeonTrap
};
const int chance_generate_room = 40;
const int chance_special_area = 5;
const int chance_corridor_content = 10;
const int chance_door_closed = 30;
const int dense_forest = 15;
const int max_building_size = 15;
extern tile_s location_type;
static slot_s slots_weapons_armor[] = {Melee, Ranged, OffHand, Head, Elbows, Legs, Torso};
static item_s item_treasure[] = {Coin, Coin, Coin, Coin, RingRed};
static item_s item_potion_scrolls[] = {ScrollRed, ScrollRed, ScrollRed,
ScrollGreen, ScrollGreen, ScrollBlue,
Book1, Book2, Book3, Book4, Book5,
Amulet1, Amulet2, Amulet3, Amulet4, Amulet5,
PotionRed, PotionGreen, PotionBlue,
WandRed, WandGreen, WandBlue,
RingRed, RingGreen, RingBlue};
static item_s item_food[] = {Ration, Ration, Ration, BreadEvlen, BreadHalflings, BreadDwarven, Sausage};
static roominfo	rooms[256]; // Кольцевой буфер генератора. Главное чтобы разница не была 256 значений.
static unsigned char stack_put, stack_get;

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

static int find(int x, int y, int w, int h, map_object_s value) {
	int x2 = x + w;
	int y2 = y + h;
	for(int y1 = y; y1 < y2; y1++) {
		for(int x1 = x; x1 < x2; x1++) {
			auto i = get(x1, y1);
			if(getobject(i) == value)
				return i;
		}
	}
	return 0xFFFF;
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

static short unsigned random(int x, int y, int w, int h) {
	return getfree(get(xrand(x, x + w - 1), xrand(y, y + h - 1)));
}

// Set horizontal wall in interior room
static short unsigned setiwh(int x, int y, int s, tile_s o, map_object_s r, bool locked_doors) {
	if(s <= 2)
		return 0xFFFF;
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
		return 0xFFFF;
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
	// Преобразуем координаты с учетом
	x = imin(imax(1, x), max_map_x - w - 1);
	y = imin(imax(1, y), max_map_y - h - 1);
	auto i = get(x, y);
	// Стены и пол
	set(i, Wall, w, h);
	set(to(i, RightDown), Floor, w - 2, h - 2);
	// Двери
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

static void create_location(int x, int y, int w, int h) {
	auto p = locations.add();
	p->type = House;
	p->set(x, y, x + w, y + h);
}

static void create_commoner(short unsigned index) {}

static creature* create_shopkeeper(site& e, short unsigned index) {
	e.owner = game::add(index, new creature(Shopkeeper));
	return e.owner;
}

static creature* create_adventurer(short unsigned index) {
	return game::add(index, new creature(Human,
		(gender_s)xrand(Male, Female),
		(class_s)xrand(Fighter, Mage)));
}

static creature* create_bartender(site& e, short unsigned index) {
	e.owner = game::add(index, new creature(Shopkeeper));
	return e.owner;
}

static void create_monster(short unsigned index) {
	static role_s monsters[] = {GoblinWarrior, OrcWarrior, GiantRat, Skeleton, Zombie};
	if(!creature::isbooming())
		game::add(index, new creature((role_s)(maprnd(monsters))));
}

static creature* create_priest(site& e, short unsigned index) {
	e.diety = (diety_s)xrand(GodBane, GodTyr);
	switch(e.diety) {
	case GodGruumsh:
		e.owner = game::add(index, new creature(Dwarf, Male, Cleric));
		break;
	default:
		e.owner = game::add(index, new creature(Human, Male, Cleric));
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

static void create_item(short unsigned index, item_s type, int level, bool forsale, identify_s identify = Unknown, char chance_curse = 20) {
	if(type == NoItem)
		return;
	auto chance_artifact = imax(0, level/4);
	auto chance_quality = imax(0, 40 + level);
	auto chance_magic = imax(0, 5 + level);
	auto it = item(type, chance_artifact, chance_magic, chance_curse, chance_quality);
	it.set(identify);
	if(it.gettype() == Coin) {
		auto count = xrand(1, 5) * level;
		if(count > 64)
			count = 64;
		it.setcount(count);
	}
	if(forsale)
		it.setforsale();
	drop(index, it);
}

static void create_door(short unsigned index) {
	set(index, Door);
	if(d100() < chance_door_closed)
		set(index, Sealed, true);
}

static void create_shop(int x0, int y0, int w, int h, int chance, unsigned char level, bool forsale, aref<item_s> source) {
	if(!source)
		return;
	for(auto y = y0; y < y0 + h; y++) {
		if(y < 0 || y >= max_map_y)
			continue;
		for(auto x = x0; x < x0 + w; x++) {
			if(x < 0 || x >= max_map_x)
				continue;
			if(d100() < chance)
				create_item(get(x, y), source.data[rand() % source.count], level, forsale, KnowEffect, 0);
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
		create_shop(x, y, w, h, 90, 30, true, item_potion_scrolls);
		break;
	case ShopFood:
		create_shopkeeper(e, pt);
		create_shop(x, y, w, h, 90, 5, true, item_food);
		break;
	case TreasureRoom:
		create_shop(x, y, w, h, 80, 10, false, item_treasure);
		break;
	}
}

static void create_interior(site& e, int x, int y, int w, int h, short unsigned i, site_s type, bool visualize) {
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
		// Дверь может попасть прямо на линию
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
#ifdef _DEBUG
	if(visualize) {
		logs::add("Создание внутренностей в (%1i, %2i)", e.x1, e.y1);
		logs::focusing(center(e.x1, e.y1, e.width(), e.height()));
	}
#endif
	switch(type) {
	case ShopPotionAndScrolls:
	case ShopWeaponAndArmor:
		create_interior(e, x1, y1, w1, h1, i, TreasureRoom, visualize);
		break;
	default:
		create_interior(e, x1, y1, w1, h1, i, EmpthyRoom, visualize);
		break;
	}
}

static void create_content(site& e, site_s type, bool visualize = false) {
	e.type = type;
	auto door = create_building(e.x1, e.y1, e.width(), e.height());
#ifdef _DEBUG
	if(visualize) {
		logs::add("Создание локации в (%1i, %2i)", e.x1, e.y1);
		logs::focusing(center(e.x1, e.y1, e.width(), e.height()));
	}
#endif
	create_interior(e, e.x1, e.y1, e.width(), e.height(), door, e.type, visualize);
}

static void create_city(int x, int y, int w, int h, int level) {
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
		create_location(x, y, w, h);
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
		create_city(x, y, w1, h, level + 1);
		create_city(x + w1, y, w - w1, h, level + 1);
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
		create_city(x, y, w, h1, level + 1);
		create_city(x, y + h1, w, h - h1, level + 1);
		if(level <= 2) {
			if(x == 1) {
				x--;
				w++;
			}
			create_road(x, y + h1 - 3, w, 3);
			for(int i = xrand(3, 6); i >= 0; i--)
				create_commoner(random(x, y + h1 - 3, w, 3));
		}
	}
}

static void put_block(short unsigned index, direction_s dir) {
	auto& e = rooms[stack_put++];
	e.index = index;
	e.dir = dir;
}

static roominfo& get_block() {
	return rooms[stack_get++];
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

static bool isvalidroom(short unsigned index, direction_s dir) {
	if(gettile(to(index, turn(dir, Left))) != NoTile)
		return false;
	if(gettile(to(index, turn(dir, Right))) != NoTile)
		return false;
	return true;
}

static void create_dungeon_item(short unsigned index) {
	create_item(index, random(slots_weapons_armor), statistic.level, false);
}

static void create_trap(short unsigned index) {
	set(index, Trap);
	set(index, Hidden, true);
}

static void create_treasure(short unsigned index) {
	create_item(index, Coin, 1, false);
}

static void create_corridor_content(short unsigned index) {
	typedef void(*proc)(short unsigned index);
	proc chances[] = {create_trap,
		create_treasure, create_treasure,
		create_door,
		create_monster, create_monster, create_monster, create_monster,
		create_dungeon_item, create_dungeon_item,
	};
	maprnd(chances)(index);
}

static void create_corridor(short unsigned index, direction_s dir) {
	if(!index)
		return;
	direction_s rnd[] = {Right, Left};
	if(d100() < 50)
		iswap(rnd[0], rnd[1]);
	short unsigned start = 0xFFFF;
	auto iterations = xrand(3, 10);
	while(true) {
		int new_index = to(index, dir);
		if(gettile(new_index) == Floor)
			return;
		if(new_index == 0xFFFF || !isvalidcorridor(index, dir))
			break;
		if(start == 0xFFFF)
			start = index;
		index = new_index;
		set(index, Floor);
		if(d100() < chance_corridor_content)
			create_corridor_content(index);
		if(--iterations == 0)
			break;
	}
	if(start != 0xFFFF) {
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

static void place_tiles(short unsigned index, direction_s dir, tile_s value, int count, direction_s test_dir) {
	while(count >= 0) {
		index = to(index, dir);
		if(!isvalidindex(index))
			return;
		if(gettile(index) != NoTile)
			return;
		auto i = to(index, dir);
		if(gettile(i) != NoTile)
			return;
		//if(gettile(to(i, test_dir)) != NoTile)
		//	return;
		set(index, value);
		count--;
	}
}

static void create_room(short unsigned index, direction_s dir) {
	auto w = xrand(1, 3);
	auto h = xrand(3, 6);
	auto h1 = h;
	auto w1 = w;
	auto start = Blocked;
	while(h >= 0) {
		index = to(index, dir);
		if(!isvalidindex(index))
			break;
		if(gettile(index) != NoTile)
			break;
		if(gettile(to(index, dir)) != NoTile)
			break;
		set(index, Floor);
		if(start == Blocked)
			start = index;
		place_tiles(index, turn(dir, Left), Floor, w, Right);
		place_tiles(index, turn(dir, Right), Floor, w, Left);
		h--;
	}
	statistic.rooms++;
	if(start != Blocked)
		put_block(index, dir);
}

static void create_dungeon(int x, int y, int w, int h, bool visualize) {
	if(statistic.level < 1)
		statistic.level = 1;
	put_block(get(10, 10), Right);
	put_block(get(70, 70), Left);
	while(stack_get != stack_put) {
		auto& e = get_block();
		if(isvalidroom(e.index, e.dir) && d100() < chance_generate_room)
			create_room(e.index, e.dir);
		else
			create_corridor(e.index, e.dir);
#ifdef _DEBUG
		if(visualize) {
			creature player;
			player.clear();
			player.position = e.index;
			logs::minimap(player);
		}
#endif
	}
}

static void update_rect(int offset, bool test_valid = true) {
	for(auto& e : locations) {
		if(!e)
			continue;
		e.x2 += offset;
		e.y2 += offset;
		if(e.width() <= 3 || e.height() <= 3)
			e.clear();
	}
}

static int compare_location(const void* p1, const void* p2) {
	auto e1 = (site*)p1;
	auto e2 = (site*)p2;
	return e2->width()*e2->height() - e1->width()*e1->height();
}

static void change_tile(tile_s t1, tile_s t2) {
	for(short unsigned i = 0; i <= max_map_x * max_map_y; i++) {
		if(gettile(i) == t1)
			set(i, t2);
	}
}

static void update_doors() {
	for(short unsigned i = 0; i <= max_map_x * max_map_y; i++) {
		if(getobject(i) == Door) {
			auto count = 0;
			if(!ispassable(to(i, Left)))
				count++;
			if(!ispassable(to(i, Right)))
				count++;
			if(!ispassable(to(i, Up)))
				count++;
			if(!ispassable(to(i, Down)))
				count++;
			if(count != 2)
				set(i, NoTileObject);
		}
	}
}

static void area_create(bool explored, bool visualize) {
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, (unsigned char)(rand() % 256));
	if(explored) {
		for(short unsigned i = 0; i < count; i++)
			set(i, Explored, true);
	}
	//
	if(location_type == Floor) {
		for(short unsigned i = 0; i < count; i++)
			set(i, NoTile);
	} else {
		create_objects(0, 0, max_map_x - 1, max_map_y - 1, xrand(10, 20), Hill);
		create_objects(0, 0, max_map_x - 1, max_map_y - 1, xrand(4, 12), Swamp);
		create_objects(0, 0, max_map_x - 1, max_map_y - 1, max_map_x*max_map_y*(dense_forest / 3) / 100, Tree);
	}
	if(location_type == City) {
		create_city(2, 2, max_map_x - 2, max_map_y - 2, 0);
		update_rect(-3);
		qsort(locations.data, locations.count, sizeof(locations.data[0]), compare_location);
		locations.count = zlen(locations.data);
		// Set start and finsh
		int current = 1;
		int max_possible_points = locations.count / 3;
		if(max_possible_points > 25)
			max_possible_points = 25;
		int biggest = locations.count - 1;
		iswap(locations.data[0], locations.data[biggest]);
		bool placed_stairs = false;
		for(auto& e : locations) {
			auto t = (site_s)xrand(House, ShopFood);
			if(current > max_possible_points)
				t = House;
			create_content(e, t, visualize);
			if(!placed_stairs && t == House) {
				if(d100() < 20 || current > max_possible_points) {
					placed_stairs = true;
					//t = StairsDown;
				}
			}
		}
	} else {
		create_dungeon(1, 1, max_map_x - 2, max_map_y - 2, visualize);
		change_tile(NoTile, Wall);
	}
	update_doors();
	for(auto& e : locations) {
		e.x2--;
		e.y2--;
	}
}

void game::create(tile_s type, short unsigned index, int level, bool explored, bool visualize) {
	initialize();
	if(!serialize(false)) {
		location_type = type;
		statistic.index = index;
		statistic.level = level;
		area_create(explored, visualize);
		serialize(true);
	}
}