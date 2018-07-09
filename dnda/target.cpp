#include "main.h"

static bool you(const creature& player, const creature* opponent) {
	return opponent == &player;
}

static bool friendly(const creature& player, const creature* opponent) {
	return !player.isenemy(opponent);
}

static bool wounded_friendly(const creature& player, const creature* opponent) {
	return !player.isenemy(opponent) && player.gethits() < player.getmaxhits();
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

static bool undefined_item(const creature& player, const item& it) {
	return it.getidentify() < KnowEffect;
}

static bool damaged_item(const creature& player, const item& it) {
	return it.isdamaged();
}

static bool visible_trap(const creature& player, short unsigned index) {
	return !game::ishidden(index) && game::gettrap(index) != NoTrap;
}

static bool any_door(const creature& player, short unsigned index) {
	return game::getobject(index) == Door;
}

static bool door(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && !game::isseal(index);
}

static bool door_locked(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && game::isseal(index);
}

static bool hidden_object(const creature& player, short unsigned index) {
	return game::getobject(index) != 0 && game::ishidden(index);
}

static bool any_item(const creature& player, const item& it) {
	return true;
}

static bool any_object(const creature& player, short unsigned index) {
	if(game::ishidden(index))
		return false;
	auto value = game::getobject(index);
	return value == NoTileObject;
}

static struct target_info {
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
} target_data[] = {{"Нет"},
//
{"Вы", you, true},
{"Союзник", friendly},
{"Раненный союзник", wounded_friendly, false, true},
{"Враг", enemy},
//
{"Неопознанный предмет", undefined_item},
{"Съедобный предмет", damaged_item},
{"Поврежденный предмет", edible},
{"Зелье или другая жидкость", drinkable},
{"Книга или свиток", readable},
{"Оружие", readable},
{"Жезл или Посох", chargeable},
{"Все предметы из инвентаря", any_item, false, false, true},
//
{"Объект", any_object},
{"Дверь", any_door},
{"Закрытая дверь", door_locked},
{"Скрытый объект", hidden_object},
{"Ловушка", visible_trap},
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

bool targetdesc::isallow(const creature& player, aref<creature*> creatures) const {
	auto& e = target_data[target];
	if(e.proc.cre) {
		if(e.self_only)
			return true;
		creature* source[1];
		if(player.select(source, creatures, target, range, player.position, e.include_self ? 0 : &player))
			return true;
	} else if(e.proc.itm) {
		item* source[1];
		if(player.select(source, target))
			return true;
	} else if(e.proc.ind) {
		short unsigned source[1];
		if(player.select(source, target, range, player.position))
			return true;
	}
	return false;
}

aref<creature*> creature::select(aref<creature*> result, aref<creature*> creatures, target_s target, char range, short unsigned start, const creature* exclude) const {
	auto& ti = target_data[target];
	auto pb = result.data;
	auto pe = result.data + result.count;
	auto x = game::getx(start);
	auto y = game::gety(start);
	if(ti.proc.cre) {
		for(auto p : creatures) {
			if(p == exclude)
				continue;
			if(!ti.proc.cre(*this, p))
				continue;
			if(range && game::distance(start, p->position) > range)
				continue;
			if(pb < pe)
				*pb++ = p;
			else
				break;
		}
	}
	return aref<creature*>(result.data, pb - result.data);
}

aref<item*> creature::select(aref<item*> result, target_s target) const {
	auto& ti = target_data[target];
	auto pb = result.data;
	auto pe = result.data + result.count;
	if(ti.proc.itm) {
		for(auto& it : wears) {
			if(!it)
				continue;
			if(!ti.proc.itm(*this, it))
				continue;
			if(pb < pe)
				*pb++ = (item*)&it;
			else
				break;
		}
	}
	return aref<item*>(result.data, pb - result.data);
}

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

item* creature::choose(aref<item*> source, bool interactive) const {
	if(!source) {
		hint("У вас нет подходящео предмета.");
		return 0;
	}
	if(interactive)
		return logs::choose(*this, source.data, source.count, "Выбирайте предмет");
	return source.data[rand() % source.count];
}

short unsigned creature::choose(aref<short unsigned> source, bool interactive) const {
	if(!source) {
		hint("Вокруг нет подходящей цели.");
		return Blocked;
	}
	if(interactive)
		return logs::choose(*this, source.data, source.count);
	return source.data[rand() % source.count];
}

creature* creature::choose(aref<creature*> source, bool interactive) const {
	if(!source) {
		hint("Вокруг никого нет.");
		return 0;
	}
	if(interactive) {
		short unsigned index_data[256];
		for(unsigned i = 0; i < source.count; i++)
			index_data[i] = source.data[i]->position;
		auto pn = logs::choose(*this, index_data, source.count);
		for(unsigned i = 0; i < source.count; i++) {
			if(index_data[i] == pn) {
				return source.data[i];
			}
		}
	}
	return source.data[rand() % source.count];
}

bool effectparam::saving() const {
	if(cre && save != NoSkill) {
		auto chance_save = cre->get(save);
		if(chance_save > 0) {
			auto r = d100();
			if(r < chance_save) {
				if(interactive)
					cre->act("%герой перенес%ла эффект без последствий.");
				return true;
			}
		}
	}
	return false;
}

bool effectparam::applyfull() {
	if(proc.test) {
		if(!proc.test(*this))
			return false;
	}
	if(saving())
		return false;
	if(skill_roll && skill_roll >= skill_value - skill_bonus) {
		if(proc.fail)
			proc.fail(*this);
		return false;
	}
	if(text) {
		if(cre)
			cre->actvs(player, text);
		else if(itm) {
			auto p = creature::getplayer();
			if(p && p->canhear(player.position))
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
			type.target, type.range, player.position,
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
		auto source = player.select(source_data, type.target, type.range, player.position);
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