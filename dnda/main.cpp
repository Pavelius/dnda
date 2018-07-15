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

int main(int argc, char* argv[]) {
	//test_overland();
	//return 0;
	logs::initialize();
	create("city", get(10, 10), 0, false, false);
	//auto start_index = game::statistic.positions[0];
	auto start_index = get(10, 10);
	auto p1 = add(start_index, new creature(Human, Female, Mage));
	auto p2 = add(start_index, new creature(Dwarf, Male, Fighter));
	auto p3 = add(start_index, new creature(Elf, Male, Theif));
	for(int i = 0; i<3; i++)
		add(get(14, 14), new creature(GoblinRockthrower));
	p1->join(p1);
	p2->join(p1);
	p3->join(p1);
	//p4->join(p1);
	p1->setplayer();

	p1->equip(item(Book1, 4, 20, 20, 40));
	p1->equip(Repair);
	p1->equip(item(PotionRed, Mundane, (enchantment_s)Experience));
	p1->equip(DetectMagic);
	p1->equip(DoorKey);
	p1->equip(Apple);
	p1->equip(item(Cloack1, 4, 30, 20, 50));

	p2->equip(Cloack5);
	p2->equip(item(PotionRed, Mundane, (enchantment_s)Drunken));

	p3->equip(BowLong);
	p3->equip(Arrow);
	p3->equip(item(SwordLong, Artifact, OfStrenght));
	p3->equip(item(Dagger, Cursed, OfDexterity));
	p3->equip(item(BracersLeather, Mundane, OfDefence));

	p1->play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	srand((unsigned)time(0));
	//srand(1000);
	return main(0, 0);
}