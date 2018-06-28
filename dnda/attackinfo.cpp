#include "main.h"

int attackinfo::roll() const {
	if(damage[0] == damage[1])
		return damage[0];
	return damage[0] + rand() % (damage[1] - damage[0] + 1);
}