#include "main.h"

using namespace game;

int main(int argc, char* argv[]) {
	//return 0;
	logs::initialize();
	create(City, get(10, 10), 1, false, false);
	auto p1 = add(get(8, 5), new creature(Human, Female, Mage));
	auto p2 = add(get(8, 5), new creature(Dwarf, Male, Fighter));
	auto p3 = add(get(2, 2), new creature(Elf, Male, Theif));
	p1->join(p1);
	p2->join(p1);
	p3->join(p1);
	p1->setplayer();

	p1->equip(Fear);
	p1->equip(Identify);

	p3->equip(BowLong);
	p3->equip({Arrow, 2, 10});
	p3->equip(item(SwordLong, Artifact, OfStrenght, 3, KnowEffect));
	p3->equip(item(Dagger, Cursed, OfDexterity, 2, KnowMagic));

	p2->equip(item(Shield, Mundane, NoEffect, 3));

	for(int i=0; i<6; i++)
		add(get(14, 14), new creature(GoblinWarrior));

	play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	unsigned c = clock();
	srand(c);
	return main(0, 0);
}