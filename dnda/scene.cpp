#include "main.h"

bool cre_damage(sceneparam& e, creature& opponent, bool run) {
	if(run) {
		opponent.damage(e.damage.roll(), e.damage.type, e.interactive);
		if(e.duration)
			opponent.set(e.state, e.duration);
	}
	return true;
}

static unsigned select(aref<creature*> result, aref<creature*> source, sceneparam& sp, short unsigned index, int range) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	auto flags = sp.flags;
	for(auto p : source) {
		if((flags&Damaged) != 0 && p->gethits() >= p->getmaxhits())
			continue;
		if((flags&Hostile) != 0 && !p->isenemy(&sp.player))
			continue;
		if((flags&Friendly) != 0 && !p->isfriend(&sp.player))
			continue;
		if(range != -1 && game::distance(index, p->getposition()) > range)
			continue;
		if(!sp.proc.cre(sp, *p, false))
			continue;
		if(ps < pe)
			*ps++ = p;
	}
	return ps - result.data;
}

static unsigned exclude(aref<creature*> result, const creature* player) {
	auto ps = result.data;
	for(auto p : result) {
		if(p == player)
			continue;
		*ps++ = p;
	}
	return ps - result.data;
}

static unsigned select(aref<item*> result, sceneparam& sp) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	auto flags = sp.flags;
	for(auto& e : sp.player.wears) {
		if(!e)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - result.data;
}

bool scene::apply(const sceneeffect& eff, bool run) {
	sceneparam sp(eff, *player, true);
	auto los = 0;
	switch(eff.flags&RangeMask) {
	case You: los = 0; break;
	case Close: los = 1; break;
	case Reach: los = 2; break;
	default: los = player->getlos(); break;
	}
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = select(source, creatures, sp, sp.player.getposition(), los);
		if(run) {
			if((eff.flags&All) != 0) {
				for(auto& e : source)
					eff.proc.cre(sp, *e, true);
			} else {
				creature* opponent = 0;
				if(eff.flags&Nearest)
					opponent = source.random();
				else
					opponent = player->choose(source, sp.interactive);
				if(!opponent)
					return false;
				eff.proc.cre(sp, *opponent, true);
				if(eff.flags&Splash) {
					source.count = exclude(source, opponent);
					source.count = select(source, source, sp, opponent->getposition(), 1);
					for(auto& e : source)
						eff.proc.cre(sp, *e, true);
					return true;
				}
			}
		}
		return source.count != 0;
	} else if(eff.proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = select(source, sp);
	} else if(eff.proc.obj) {

	}
	return false;
}