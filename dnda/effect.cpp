#include "main.h"

bool effectc::saving(const targets& ti, bool interactive) const {
	if(ti.cre && save.type != NoSave) {
		auto chance_save = 0;
		switch(save.type) {
		case SaveAbility:
			chance_save = ti.cre->get(save.ability) * 4;
			break;
		case SaveSkill:
			chance_save = ti.cre->get(save.skill);
			break;
		}
		if(chance_save) {
			auto r = d100();
			if(r < chance_save) {
				if(interactive)
					ti.cre->act("%герой перенес%ла эффект без последствий.");
				return true;
			}
		}
	}
	return false;
}