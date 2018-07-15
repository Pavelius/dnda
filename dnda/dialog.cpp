#include "main.h"

aref<const speech*> dialog::select(aref<const speech*> source, const speech* p, speech_s type) const {
	auto pb = source.data;
	auto pe = pb + source.count;
	if(p) {
		for(; *p; p++) {
			if(p->type != type)
				continue;
			if(p->test) {
				if(p->type == Speech) {
					if(!p->test(*player, *this))
						continue;
				} else {
					if(!p->test(*opponent, *this))
						continue;
				}
			}
			// RULE: skills can show or hide answers
			if(p->skill.id) {
				if(player->get(p->skill.id) < p->skill.value)
					continue;
			}
			if(pb < pe)
				*pb++ = p;
			else
				break;
			if(p->stop_analize)
				break;
		}
	}
	return aref<const speech*>(source.data, pb - source.data);
}

const speech* dialog::say(const speech* pb, speech_s type) {
	const speech* source[32];
	auto result = select(source, pb, type);
	if(!result)
		return 0;
	if(type == Speech || type == Action) {
		zshuffle(result.data, result.count);
		if(type == Speech)
			opponent->sayvs(*player, result.data[0]->text);
		else
			opponent->actvs(*player, result.data[0]->text);
	} else {
		for(auto p : result)
			logs::add(p - pb, p->text);
	}
	logs::add("\n");
	return pb;
}

void dialog::apply(const speech& e) {
	result = e.success;
	if(e.fail && e.skill.id) {
		if(!player->roll(e.skill.id, -e.skill.value))
			result = e.fail;
	}
	// Procedure call after skill check.
	// This due to result is known.
	if(e.proc)
		e.proc(*this, e);
}

const speech* dialog::phase(const speech* p) {
	auto ask = say(p, Action);
	if(!ask)
		ask = say(p, Speech);
	if(ask)
		apply(*ask);
	if(say(p, Answer)) {
		auto index = logs::input();
		apply(p[index]);
	} else {
		if(result)
			logs::next();
	}
	return result;
}

void dialog::start(const speech* p) {
	while(p)
		p = phase(p);
	if(player)
		player->wait(3);
	if(opponent)
		opponent->wait(3);
}