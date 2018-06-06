#include "main.h"

int main(int argc, char* argv[]) {
	//return 0;
	logs::initialize();
	game::create(City, game::get(10, 10), 1, false, false);
	auto p4 = game::add(game::get(8, 5), Human, Female, Mage, true);
	auto p1 = game::add(game::get(8, 5), Dwarf, Male, Fighter, true);
	auto p2 = game::add(game::get(2, 2), Elf, Male, Theif, true);
	p2->equip(BowLong);
	p1->equip({Shield, Mundane, NoEffect, 3});
	p2->equip({Arrow, 2, 10});
	p2->equip({SwordLong, Artifact, OfStrenght, 2});
	//for(int i=0; i<6; i++)
	//	game::add(game::get(14, 14), new creature(GoblinWarrior));
	game::serialize(true);
	game::play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	unsigned c = clock();
	srand(c);
	return main(0, 0);
}