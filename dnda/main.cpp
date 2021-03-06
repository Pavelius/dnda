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
	game::initialize(get(10, 10), 0, Plain);
	// Set new random values
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, (unsigned char)(rand() % 256));
	//create_worldmap();
	logs::worldedit();
}

static creature* create_character(short unsigned index, race_s race, class_s type, bool interactive) {
	creature* p;
	if(!interactive) {
		p = creature::addplayer();
		p->create(race,
			(gender_s)xrand(Male, Female),
			type);
	} else {
		logs::add("��� ��?");
		logs::add(Male, getstr(Male));
		logs::add(Female, getstr(Female));
		auto gender = (gender_s)logs::input();
		logs::add("�� ����� �� �����?");
		for(auto i = Human; i <= Halfling; i = (race_s)(i + 1))
			logs::add(i, getstr(i));
		auto race = (race_s)logs::input();
		logs::add("��� �� �����������?");
		for(auto i = Cleric; i <= Theif; i = (class_s)(i + 1))
			logs::add(i, getstr(i));
		auto type = (class_s)logs::input();
		p = new creature(race, gender, type);
	}
	index = getfree(index);
	if(index == Blocked)
		return 0;
	p->move(index);
	return p;
}

static void create_party(bool interactive) {
	auto start_index = game::statistic.positions[0];
	auto p1 = create_character(start_index, Human, Mage, interactive);
	auto p2 = create_character(start_index, Dwarf, Fighter, interactive);
	auto p3 = create_character(start_index, (race_s)xrand(Human, Halfling), (class_s)xrand(Cleric, Theif), interactive);
	p1->setplayer();
#ifdef _DEBUG
	p1->equip(item(Book1, 4, 20, 20, 40));
	p1->equip(item(Potion1, (enchantment_s)Experience));
#endif
}

static menu main_menu[] = {{"������� ����"},
{"������� ����� ����"},
{"���������� ����������� ����� ����"},
{"�����"},
{}};

int main(int argc, char* argv[]) {
	logs::initialize();
	//logs::choose(main_menu);
	create(AreaCity, get(10, 10), 0, false, false);
	create_party(false);
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