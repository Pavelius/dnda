#include "main.h"

static bool you(const creature& player, const creature* opponent) {
	return opponent == &player;
}

static bool ally(const creature& player, const creature* opponent) {
	return opponent != &player && player.getparty() == opponent->getparty();
}

static bool you_ally(const creature& player, const creature* opponent) {
	return opponent == &player || player.getparty() == opponent->getparty();
}

static bool enemy(const creature& player, const creature* opponent) {
	return player.isenemy(opponent);
}

static bool any_creature(const creature& player, const creature* opponent) {
	return true;
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

static bool any_item(const creature& player, const item& it) {
	return true;
}

static bool visible_trap(const creature& player, short unsigned index) {
	return !game::ishidden(index) && game::gettrap(index) != NoTrap;
}

static bool door(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && !game::isseal(index);
}

static bool door_locked(const creature& player, short unsigned index) {
	return game::getobject(index) == Door && game::isseal(index);
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
	bool			self;
	bool			target_all_match;
} target_data[] = {{"���"},
//
{"��", you, true},
{"�������", ally},
{"�� ��� �������", you_ally},
{"��������", any_creature},
{"����", enemy},
//
{"�������", any_item},
{"������������ �������", undefined_item},
{"��������� �������", edible},
{"����� ��� ������ ��������", drinkable},
{"����� ��� ������", readable},
{"������", any_item},
{"���� ��� �����", chargeable},
{"��� �������� �� ���������", any_item, false, true},
//
{"�����", door},
{"�������� �����", door_locked},
{"�������", visible_trap},
};
assert_enum(target, TargetTrap);
getstr_enum(target);

bool targetdesc::isallow(const creature& player, aref<creature*> creatures) const {
	auto& e = target_data[target];
	if(e.proc.cre) {
		if(e.self)
			return true;
		for(auto p : creatures) {
			if(e.proc.cre(player, p))
				return true;
		}
	} else if(e.proc.itm) {
		for(auto& it : player.wears) {
			if(it && e.proc.itm(player, it))
				return true;
		}
	}
	return false;
}