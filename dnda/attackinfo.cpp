#include "main.h"

void attackinfo::clear() {
	bonus = 0;
	speed = 0;
	critical = 20;
	multiplier = 2;
	damage[0] = 0;
	damage[1] = 2;
}

int attackinfo::roll() const {
	if(damage[0] == damage[1])
		return damage[0];
	return damage[0] + rand() % (damage[1] - damage[0] + 1);
}