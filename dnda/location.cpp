#include "archive.h"
#include "main.h"

static short unsigned	movements[max_map_x*max_map_y];
static short unsigned	stack[256 * 256];
static direction_s		all_aroud[] = {Left, Right, Up, Down, LeftDown, LeftUp, RightDown, RightUp};

tile_s location::gettile(short unsigned i) const {
	if(i == Blocked)
		return NoTile;
	return mptil[i];
}

int location::getavatar(short unsigned i, tile_s e) const {
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

void location::drop(short unsigned i, item object) {
	if(!object)
		return;
	auto p = items.add();
	*((item*)p) = object;
	p->index = i;
}

trap_s location::gettrap(short unsigned i) const {
	if(mpobj[i] != Trap)
		return NoTrap;
	return (trap_s)(1 + mprnd[i] % (TrapWater - TrapAnimal + 1));
}

aref<item*> location::getitems(aref<item*> result, short unsigned index) {
	auto pb = result.begin();
	auto pe = result.end();
	for(auto& li : items) {
		if(!li || li.index != index)
			continue;
		if(pb < pe)
			*pb++ = &li;
	}
	return aref<item*>(result.data, pb - result.data);
}

int	location::getnight() const {
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

short unsigned location::getfree(short unsigned i) const {
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

bool location::ispassable(short unsigned i) const {
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

bool location::ispassabledoor(short unsigned i) const {
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

bool location::ispassablelight(short unsigned i) const {
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

void location::set(short unsigned i, tile_s value) {
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

void location::set(short unsigned i, map_object_s value) {
	if(i == Blocked)
		return;
	mpobj[i] = value;
}

void location::set(short unsigned i, unsigned char value) {
	if(i == Blocked)
		return;
	mprnd[i] = value;
}

void location::set(short unsigned i, map_flag_s type, bool value) {
	if(value)
		mpflg[i] |= (1 << type);
	else
		mpflg[i] &= ~(1 << type);
}

direction_s location::turn(direction_s from, direction_s side) {
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

short unsigned location::to(short unsigned index, direction_s id) {
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

short unsigned location::getstepto(short unsigned index) const {
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

short unsigned location::getstepfrom(short unsigned index) const {
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

void location::makewave(short unsigned index, bool(location::*proc)(short unsigned) const) {
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
			if(movements[i] == BlockedCreature || !(this->*proc)(i))
				continue;
			if(movements[i] == Blocked || movements[i] > w) {
				movements[i] = w;
				stack[push++] = i;
			}
		}
	}
}

location::site* location::getsite(short unsigned i) const {
	if(i == Blocked)
		return 0;
	point pt; pt.x = getx(i); pt.y = gety(i);
	for(auto& e : sites) {
		if(!e)
			continue;
		if(pt.x >= e.x1 && pt.x < e.x2 && pt.y >= e.y1 && pt.y<e.y2)
			return (site*)&e;
	}
	return 0;
}

void location::create(short unsigned index, int level, tile_s tile) {
	this->world_index = index;
	this->level = level;
	memset(this, 0, sizeof(location));
	memset(mptil, tile, sizeof(mptil));
}

template<> void archive::set<site>(site& e);

bool location::serialize(bool writemode) {
	char temp[128];
	zcpy(temp, "maps/D");
	sznum(zend(temp), level, 2, "XX");
	sznum(zend(temp), world_index, 5, "00000");
	zcat(temp, ".dat");
	io::file file(temp, writemode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, writemode);
	if(!a.signature("SAV"))
		return false;
	if(!a.version(1, 0))
		return false;
	a.set(type);
	a.set(level);
	a.set(world_index);
	a.set(artifacts);
	a.setr(habbitants);
	a.setr(mpflg);
	a.setr(mptil);
	a.setr(mpobj);
	a.setr(mprnd);
	a.setr(items);
	a.set(sites);
	return true;
}