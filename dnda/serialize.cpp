#include "archive.h"
#include "main.h"

static adat<creature, 1024> creature_data;

void* creature::operator new(unsigned size) {
	for(auto& e : creature_data) {
		if(!e)
			return &e;
	}
	return creature_data.add();
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

void game::savemap() {
	char temp[260];
	zcpy(temp, "maps/D");
	sznum(zend(temp), statistic.level, 2, "XX");
	sznum(zend(temp), statistic.index, 5, "00000");
	zcat(temp, ".dat");
	io::file file(temp, StreamWrite);
	if(!file)
		return;
	archive a(file, true);
	a.set(locations);
	a.set(creatures);
}