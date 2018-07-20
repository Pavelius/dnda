#include "main.h"

struct roomeffect {
	struct search_item { // This is for skill Alertness
		const char*		text;
		char			chance;
		char			quality; // Blessed of Cursed is fate decided
		char			magic; // Chance to be artifact is magic/10.
		aref<item_s>	items;
	};
	struct monster_info {
		char			chance;
		aref<role_s>	monsters;
		char			level;
	};
	struct skill_info {
		skill_s			skill;
		char			bonus;
		void(*success)(roomeffect& e);
		void(*fail)(roomeffect& e);
		explicit operator bool() const { return skill != NoSkill; }
	};
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
	monster_info		appear;
	skill_info			skills[4];
};