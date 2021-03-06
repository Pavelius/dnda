#include "main.h"

struct range_info {
	const char*		id;
	const char*		name;
	unsigned char	distance;
};
range_info range_data[] = {{"You", "��"},
{"Close", "������"},
{"Reach", "�����"},
{"Near", "��������"},
};
assert_enum(range, Near);

static bool you(const creature& player, const creature* opponent) {
	return opponent == &player;
}

static bool friendly(const creature& player, const creature* opponent) {
	return !player.isenemy(opponent);
}

static bool wounded_friendly(const creature& player, const creature* opponent) {
	return !player.isenemy(opponent) && player.gethits() < player.getmaxhits();
}

static bool neutral(const creature& player, const creature* opponent) {
	return !player.isenemy(opponent) && !player.isparty(opponent);
}

static bool enemy(const creature& player, const creature* opponent) {
	return player.isenemy(opponent);
}

static bool edible(const creature& player, const item& it) {
	return it.isedible();
}

static bool drinkable(const creature& player, const item& it) {
	return it.isdrinkable();
}

static bool chargeable(const creature& player, const item& it) {
	return it.ischargeable();
}

static bool readable(const creature& player, const item& it) {
	return it.isreadable();
}

static bool mundane_item(const creature& player, const item& it) {
	return it.getmagic()==Mundane && it.isidentified();
}

static bool undefined_item(const creature& player, const item& it) {
	return !it.isidentified();
}

static bool damaged_item(const creature& player, const item& it) {
	return it.isdamaged();
}

static bool visible_trap(const creature& player, short unsigned index) {
	return !game::is(index, Hidden) && game::gettrap(index) != NoTrap;
}

static bool any_door(const creature& player, short unsigned index) {
	return game::getobject(index) == Door;
}

static bool door(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && !game::is(index, Sealed);
}

static bool door_locked(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && game::is(index, Sealed);
}

static bool hidden_object(const creature& player, short unsigned index) {
	return game::getobject(index) != 0 && game::is(index, Hidden);
}

static bool any_item(const creature& player, const item& it) {
	return true;
}

static bool any_object(const creature& player, short unsigned index) {
	if(game::is(index, Hidden))
		return false;
	auto value = game::getobject(index);
	return value != NoTileObject;
}

static struct targetinfo {
	struct callback {
		bool(*cre)(const creature& player, const creature* opponent);
		bool(*itm)(const creature& player, const item& it);
		bool(*ind)(const creature& player, short unsigned index);
		constexpr callback() : cre(), itm(), ind() {}
		constexpr callback(bool(*cre)(const creature& player, const creature* opponent)) : cre(cre), itm(), ind() {}
		constexpr callback(bool(*itm)(const creature& player, const item& it)) : cre(), itm(itm), ind() {}
		constexpr callback(bool(*ind)(const creature& player, short unsigned index)) : cre(), itm(), ind(ind) {}
	};
	const char*		name;
	callback		proc;
	bool			self_only;
	bool			include_self;
	bool			target_all_match;
} target_data[] = {{"���"},
//
{"��", you, true},
{"�������", friendly},
{"�������� �������", wounded_friendly, false, true},
{"�������", neutral},
{"����", enemy},
//
{"����� �������", any_item},
{"������� �������", mundane_item},
{"������������ �������", undefined_item},
{"��������� �������", damaged_item},
{"������������ �������", edible},
{"����� ��� ������ ��������", drinkable},
{"����� ��� ������", readable},
{"������", readable},
{"���� ��� �����", chargeable},
{"��� �������� �� ���������", any_item, false, false, true},
//
{"������", any_object},
{"�����", any_door},
{"�������� �����", door_locked},
{"������� ������", hidden_object},
{"�������", visible_trap},
};
assert_enum(target, TargetTrap);
getstr_enum(target);

static bool islos(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < max_map_x && y0 >= 0 && y0 < max_map_y) {
			auto i = game::get(x0, y0);
			if(!game::ispassablelight(i))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

bool targetdesc::iscreature() const {
	return target_data[target].proc.cre != 0;
}

bool targetdesc::isposition() const {
	return target_data[target].proc.ind != 0;
}

unsigned char targetdesc::getrange() const {
	return range_data[range].distance;
}

bool targetdesc::isallow(const creature& player, aref<creature*> creatures) const {
	auto& e = target_data[target];
	if(e.proc.cre) {
		if(e.self_only)
			return true;
		creature* source[1];
		if(player.select(source, creatures, target, range, player.getposition(), e.include_self ? 0 : &player))
			return true;
	} else if(e.proc.itm) {
		item* source[1];
		if(player.select(source, target))
			return true;
	} else if(e.proc.ind) {
		short unsigned source[1];
		if(player.select(source, target, range, player.getposition()))
			return true;
	}
	return false;
}

//aref<item*> creature::select(aref<item*> result, target_s target) const {
//	auto& ti = target_data[target];
//	auto pb = result.data;
//	auto pe = result.data + result.count;
//	if(ti.proc.itm) {
//		for(auto& it : wears) {
//			if(!it)
//				continue;
//			if(!ti.proc.itm(*this, it))
//				continue;
//			if(pb < pe)
//				*pb++ = (item*)&it;
//			else
//				break;
//		}
//	}
//	return aref<item*>(result.data, pb - result.data);
//}

static bool linelossv(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 == x1 && y0 == y1)
			return true;
		if(x0 >= 0 && x0 < max_map_x && y0 >= 0 && y0 < max_map_y) {
			if(!game::ispassablelight(game::get(x0, y0)))
				return false;
		}
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

aref<short unsigned> creature::select(aref<short unsigned> result, target_s target, char range, short unsigned start, bool los) const {
	auto& ti = target_data[target];
	auto pb = result.data;
	auto pe = result.data + result.count;
	if(ti.proc.ind) {
		if(!range)
			range = getlos();
		auto x = game::getx(start);
		auto y = game::gety(start);
		for(auto y1 = y - range; y1 <= y + range; y1++) {
			if(y1 < 0 || y1 >= max_map_y)
				continue;
			for(auto x1 = x - range; x1 <= x + range; x1++) {
				if(x1 < 0 || x1 >= max_map_x)
					continue;
				auto index = game::get(x1, y1);
				if(!ti.proc.ind(*this, index))
					continue;
				if(los && !linelossv(x, y, x1, y1))
					continue;
				if(pb < pe)
					*pb++ = index;
				else
					return aref<short unsigned>(result.data, pb - result.data);
			}
		}
	}
	return aref<short unsigned>(result.data, pb - result.data);
}

bool effectparam::applyfull() {
	if(proc.test) {
		if(!proc.test(*this))
			return false;
	}
	if(skill_roll) {
		auto total = skill_value + skill_bonus;
		if(skill_roll >= total) {
			if(proc.fail)
				proc.fail(*this);
			return false;
		}
	}
	if(text) {
		if(cre)
			cre->actvs(player, text);
		else if(itm) {
			auto p = creature::getplayer();
			if(p && p->canhear(player.getposition()))
				itm->act(text);
		} else
			player.act(text);
	}
	if(proc.success)
		proc.success(*this);
	if(experience)
		player.addexp(experience);
	return true;
}

int effectparam::apply(const char* format, const char* format_param) {
	auto& ti = target_data[type.target];
	if(format)
		player.actv(format, format_param);
	auto affected = 0;
	if(ti.proc.cre) {
		if(ti.self_only) {
			if(ti.proc.cre(player, &player)) {
				cre = (creature*)&player;
				applyfull();
				affected++;
			}
		}
		creature* source_data[256];
		auto source = player.select(source_data, creatures,
			type.target, type.getrange(), player.getposition(),
			ti.include_self ? 0 : &player);
		if(ti.target_all_match) {
			for(auto p : source) {
				cre = p;
				applyfull();
				affected++;
			}
		} else {
			cre = player.choose(source, interactive);
			if(cre) {
				applyfull();
				affected++;
			}
		}
	} else if(ti.proc.itm) {
		item* source_data[256];
		auto source = player.select(source_data, type.target);
		if(ti.target_all_match) {
			for(auto& p : player.wears) {
				itm = &p;
				applyfull();
				affected++;
			}
		} else {
			itm = player.choose(source, interactive);
			if(itm) {
				applyfull();
				affected++;
			}
		}
	} else if(ti.proc.ind) {
		short unsigned source_data[256];
		auto source = player.select(source_data, type.target, type.range, player.getposition());
		if(ti.target_all_match) {
			for(auto p : source) {
				if(!ti.proc.ind(player, p))
					continue;
				pos = p;
				applyfull();
				affected++;
			}
		} else {
			pos = player.choose(source, interactive);
			if(pos != Blocked) {
				applyfull();
				affected++;
			}
		}
	}
	return affected;
}