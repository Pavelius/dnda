#include "main.h"

using namespace game;

int main(int argc, char* argv[]) {
	//return 0;
	logs::initialize();
	create(City, get(10, 10), 1, false, false);
	auto p1 = add(get(8, 5), Human, Female, Mage, true);
	auto p2 = add(get(8, 5), Dwarf, Male, Fighter, true);
	auto p3 = add(get(2, 2), Elf, Male, Theif, true);
	p3->equip(BowLong);
	p3->equip(Identify);
	p3->equip(item(ScrollBlue, 10));
	p3->equip(item(WandBlue, 10));
	p3->equip(item(PotionBlue, 10));
	p3->equip({Arrow, 2, 10});
	p3->equip({SwordLong, Artifact, OfStrenght, 2});
	p2->equip({Shield, Mundane, NoEffect, 3});
	for(int i=0; i<6; i++)
		add(get(14, 14), new creature(GoblinWarrior));
	play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	unsigned c = clock();
	srand(c);
	return main(0, 0);
}