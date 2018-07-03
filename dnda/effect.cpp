#include "main.h"

static void apply_area(effectparam& e, void(*method)(effectparam& e)) {
	creature* result[64];
	effectparam e1 = e;
	e1.type.area = 0;
	for(auto p : e.cre->getcreatures(result, {e.type.target, e.type.area}, e.cre->position, &e.player, e.cre)) {
		e1.cre = p;
		method(e1);
	}
}

void effectparam::apply(void(*proc)(effectparam& e)) {
	if(!proc)
		return;
	if(type.area)
		apply_area(*this, proc);
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
	apply(proc.success);
	if(experience)
		player.addexp(experience);
}

bool effectparam::saving() const {
	if(cre && save != NoSkill) {
		auto chance_save = cre->get(save);
		if(chance_save>0) {
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