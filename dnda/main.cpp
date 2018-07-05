#include "main.h"

using namespace game;

int main(int argc, char* argv[]) {
	static variant variants[] = {Bargaining, OfArmor, OfCharisma, Anger};
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

	p1->equip(item(Book1, 4, 20, 20, 40));
	p1->equip(item(PotionRed, Magical, (enchantment_s)ExperienceState));
	p1->equip(item(PotionRed, Magical, (enchantment_s)ExperienceState));
	p1->equip(Fear);
	p1->equip(Apple);
	p1->equip(Rock);
	p1->equip(Sling);
	p1->equip(Cloack1);

	p2->equip(Cloack5);

	p3->equip(BowLong);
	p3->equip(Arrow);
	p3->equip(item(SwordLong, Artifact, OfStrenght));
	p3->equip(item(Dagger, Cursed, OfDexterity));
	p3->equip(item(BracersLeather, Magical, OfDefence));

	//for(int i=0; i<6; i++)
	//	add(get(14, 14), new creature(GoblinWarrior));

	p1->play();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	srand((unsigned)time(0));
	//srand(1000);
	return main(0, 0);
}