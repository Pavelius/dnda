#include "main.h"

void effectparam::apply(void(*proc)(effectparam& e)) {
	if(!proc)
		return;
	switch(type.target) {
	case TargetInvertory:
		for(auto& e : player.wears) {
			itm = &e;
			proc(*this);
		}
		break;
	default:
		proc(*this);
		break;
	}
}

void effectparam::apply() {
	if(text) {
		if(cre)
			cre->actvs(player, text);
	}
	apply(success);
	if(experience)
		player.addexp(experience);
}

bool effectparam::saving() const {
	if(cre && save.type != NoSave) {
		auto chance_save = 0;
		switch(save.type) {
		case SaveAbility:
			chance_save = cre->get(save.ability) * 4;
			break;
		case SaveSkill:
			chance_save = cre->get(save.skill);
			break;
		}
		if(chance_save) {
			auto r = d100();
			if(r < chance_save) {
				if(interactive)
					cre->act("%герой перенес%ла эффект без последствий.");
				return true;
			}
		}
	}
	return false;
}