#include "main.h"

int damageinfo::roll() const {
	if(max<=min)
		return min;
	return min + rand() % (max - min + 1);
}