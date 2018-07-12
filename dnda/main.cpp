#include "main.h"

using namespace game;

static void test_overland() {
	game::initialize();
	// Set new random values
	auto count = max_map_x * max_map_y;
	for(short unsigned i = 0; i < count; i++)
		set(i, (unsigned char)(rand() % 256));
	game::set(get(10, 10), Sea);
	game::set(get(10, 11), Sea);
	game::set(get(11, 11), Sea);
	game::set(get(11, 12), Sea);
	game::set(get(5, 5), Mountains);
	game::set(get(5, 6), Mountains);
	game::set(get(6, 6), Mountains);
	game::set(get(8, 7), CloudPeaks);
	game::set(get(8, 8), CloudPeaks);
	game::set(get(9, 8), CloudPeaks);
	game::set(get(10, 8), Mountains);
	game::set(get(10, 9), Mountains);
	game::set(get(9, 9), Mountains);
	game::set(get(2, 2), Forest);
	game::set(get(2, 3), Forest);
	game::set(get(3, 3), Forest);
	game::set(get(2, 5), Swamp);
	game::set(get(2, 6), Swamp);
	game::set(get(3, 6), Swamp);
	game::set(get(8, 5), Foothills);
	game::set(get(8, 6), Foothills);
	game::set(get(9, 6), Foothills);
	logs::worldedit();
}

int main(int argc, char* argv[]) {
	logs::initialize();
	create("city", get(10, 10), 1, true, false);
	test_overland();
	return 0;
	auto p1 = add(get(8, 5), new creature(Human, Female, Mage));
	auto p2 = add(get(8, 5), new creature(Dwarf, Male, Fighter));
	auto p3 = add(get(2, 2), new creature(Elf, Male, Theif));
	//auto p4 = add(get(2, 2), new creature(Halfling, Male, Cleric));
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

	//for(int i=0; i<6; i++)
	//	add(get(14, 14), new creature(GoblinWarrior));

	p1->play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	srand((unsigned)time(0));
	//srand(1000);
	return main(0, 0);
}