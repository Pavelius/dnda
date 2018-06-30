#include "main.h"

void effectparam::apply() {
	if(effect.number)
		count = effect.number.roll();
	if(effect.proc) {
		switch(effect.type.target) {
		case TargetInvertory:
			for(auto& e : player.wears) {
				itm = &e;
				effect.proc(*this);
			}
			for(auto& e : player.backpack) {
				itm = &e;
				effect.proc(*this);
			}
			break;
		default:
			effect.proc(*this);
			break;
		}
	}
	if(effect.text) {
		if(cre)
			cre->act(effect.text);
	}
}

bool effectparam::saving() const {
	if(cre && effect.save.type != NoSave) {
		auto chance_save = 0;
		switch(effect.save.type) {
		case SaveAbility:
			chance_save = cre->get(effect.save.ability) * 4;
			break;
		case SaveSkill:
			chance_save = cre->get(effect.save.skill);
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