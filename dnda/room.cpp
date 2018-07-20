#include "main.h"

static struct room_info {
	struct search_item { // This is for skill Alertness
		const char*		text;
		char			chance; // Bonus to skill Alertness
		char			quality; // Blessed of Cursed is fate decided
		char			magic; // Chance to be artifact is magic/10.
		aref<item_s>	items;
	};
	struct monster_info {
		char			chance;
		char			level;
		aref<role_s>	monsters;
	};
	const char*			name; // Message appear to player when enter to room
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
	monster_info		appear;
	aref<effectinfo>	skills;
} room_data[] = {{},
{" омната с оружием", "Ќа стелажах вокруг вас сто€ло было размещено разнообразное оружие.",
{"Ќа одном из стеллажей вы нашли более менее работоспособное оружие.", 0, 50},
{}},
};