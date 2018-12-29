#include "main.h"

using namespace game;

static void create_worldmap() {
	static direction_s dir[] = {LeftDown, Left, LeftUp, Up, RightUp, Right, RightDown, Down};
	static tile_s tiles[] = {Sea, Swamp, Plain, Forest, Foothills, Mountains, CloudPeaks, NoTile};
	static tile_s chances[][8] = {{Sea, Sea, Sea, Sea, Sea, Forest, Foothills, Plain},
	{Swamp, Swamp, Swamp, Swamp, Swamp, Plain, Plain, Sea},
	{Foothills, Plain, Plain, Plain, Plain, Plain, Forest, Forest},
	{Foothills, Forest, Forest, Forest, Forest, Forest, Plain, Plain},
	{Mountains, Foothills, Foothills, Foothills, Foothills, Plain, Plain, Forest},
	{CloudPeaks, Mountains, Mountains, Mountains, Mountains, Mountains, Foothills, Forest},
	{CloudPeaks, CloudPeaks, CloudPeaks, CloudPeaks, CloudPeaks, Mountains, Mountains, Foothills},
	};
	unsigned short wolrdmaps[256 * 256];
	unsigned short push = 0;
	unsigned short pop = 0;
	auto i = game::get(10, 10);
	game::set(i, CloudPeaks);
	wolrdmaps[push++] = i;
	while(pop < push) {
		auto e = wolrdmaps[pop++];
		auto t = game::gettile(e);
		if(t == NoTile)
			continue;
		auto n = zfind(tiles, t);
		if(n == -1)
			continue;
		for(auto d : dir) {
			auto i = to(e, d);
			if(i == Blocked)
				continue;
			if(game::gettile(i) != NoTile)
				continue;
			if(n == -1)
				continue;
			auto t = maprnd(chances[n]);
			game::set(i, t);
			wolrdmaps[push++] = i;
		}
	}
}

static void test_overland() {
	logs::initialize();
	game::initialize(get(10,10), 0, Plain);
	game::serializew(false);
	// Set new random values
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, (unsigned char)(rand() % 256));
	//create_worldmap();
	logs::worldedit();
}

static creature* create_character(bool interactive) {
	if(!interactive)
		return new creature(
		(race_s)xrand(Human, Halfling),
		(gender_s)xrand(Male, Female),
		(class_s)xrand(Cleric, Theif));
	logs::add("Кто вы?");
	logs::add(Male, getstr(Male));
	logs::add(Female, getstr(Female));
	auto gender = (gender_s)logs::input();
	logs::add("Из каких вы краев?");
	for(auto i = Human; i <= Halfling; i = (race_s)(i + 1))
		logs::add(i, getstr(i));
	auto race = (race_s)logs::input();
	logs::add("Чем вы занимаетесь?");
	for(auto i = Cleric; i <= Theif; i = (class_s)(i + 1))
		logs::add(i, getstr(i));
	auto type = (class_s)logs::input();
	return new creature(race, gender, type);
}

static void create_party(bool interactive) {
	auto start_index = game::statistic.positions[0];
	//auto start_index = get(10, 10);
	if(!serializep(start_index, false)) {
		//auto p1 = creature::add(start_index, Human, Female, Mage);
		//auto p2 = creature::add(start_index, Dwarf, Male, Fighter);
		//auto p3 = creature::add(start_index, Elf, Male, Theif);
		auto p1 = create_character(interactive);
		auto p2 = create_character(interactive);
		auto p3 = create_character(interactive);
		p1->join(p1);
		p2->join(p1);
		p3->join(p1);
		p1->setplayer();
#ifdef _DEBUG
		p1->equip(item(Book1, 4, 20, 20, 40));
		p1->equip(item(Potion1, (enchantment_s)Experience));
		serializep(start_index, true);
		serializep(start_index, false);
#endif
	}
}

static menu main_menu[] = {{"Главное меню"},
{"Создать новую игру"},
{"Продолжить сохраненную ранее игру"},
{"Выйти"},
{}};

int main(int argc, char* argv[]) {
	//return 0;
	//test_overland();
	package::pack("art", "E:/applications/test/all.pkg");
	package::unpack("E:/applications/test/unpack", "E:/applications/test/all.pkg");
	logs::initialize();
	logs::choose(main_menu);
	create(AreaCity, get(10, 10), 0, false, false);
	create_party(true);
	auto p = creature::getplayer();
	if(!p)
		return 0;
	p->play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	srand((unsigned)time(0));
	//srand(1000);
	return main(0, 0);
}