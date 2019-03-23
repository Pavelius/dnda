#include "main.h"

template<class T>
static unsigned exclude(aref<T> result, const T player) {
	auto ps = result.data;
	for(auto p : result) {
		if(p == player)
			continue;
		*ps++ = p;
	}
	return ps - result.data;
}

static unsigned source_select(aref<creature*> result, aref<creature*> source, sceneparam& sp, short unsigned index, int range) {
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

static unsigned source_select(aref<item*> result, sceneparam& sp) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	auto flags = sp.flags;
	for(auto& e : sp.player.wears) {
		if(!e)
			continue;
		if((flags&Damaged) != 0 && !e.isdamaged())
			continue;
		if((flags&Conceal) != 0 && e.isidentified())
			continue;
		if((flags&Identified) != 0 && !e.isidentified())
			continue;
		if((flags&Hostile) != 0 && e.getmagic() != Cursed)
			continue;
		if((flags&Friendly) != 0 && e.getmagic() < BlessedItem)
			continue;
		if(!sp.proc.itm(sp, e, false))
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - result.data;
}

static unsigned source_select(aref<short unsigned> result, aref<short unsigned> source, sceneparam& sp, short unsigned start_index, int range) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	auto flags = sp.flags;
	for(auto index : source) {
		if((flags&Conceal) != 0 && !game::is(index, Hidden))
			continue;
		if(range > 0 && game::distance(start_index, index) > range)
			continue;
		if(!sp.proc.obj(sp, index, false))
			continue;
		if(ps < pe)
			*ps++ = index;
	}
	return ps - result.data;
}

static bool	cre_all(sceneparam& e, creature& subject, bool run) {
	return true;
}

creature* creature::getnearest(aref<creature*> source, unsigned flags) const {
	sceneeffect se = {cre_all, flags};
	sceneparam sp(se, const_cast<creature&>(*this), true);
	creature* creature_result[260];
	auto ps = creature_result;
	auto pe = creature_result + sizeof(creature_result)/sizeof(creature_result[0]);
	for(auto p : source) {
		if((flags&Damaged) != 0 && p->gethits() >= p->getmaxhits())
			continue;
		if((flags&Hostile) != 0 && !p->isenemy(this))
			continue;
		if((flags&Friendly) != 0 && !p->isfriend(this))
			continue;
		if(ps < pe)
			*ps++ = p;
	}
	aref<creature*> result;
	result.data = creature_result;
	result.count = ps - creature_result;
	return game::getnearest(result, position);
}

unsigned sceneparam::getduration() const {
	return state.duration * level;
}

void creature::apply(const sceneeffect& eff, scene& sc, const targetinfo& ti, int level, const char* format, const char* format_param) {
	sceneparam sp(eff, *this, true);
	sp.level = level;
	auto los = getlos(eff.flags);
	if(format)
		actv(format, format_param);
	if(eff.messages.action)
		act(eff.messages.action);
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = source_select(source, sc.creatures, sp, sp.player.getposition(), los);
		if((eff.flags&All) != 0) {
			for(auto& e : source) {
				if(eff.messages.success)
					e->act(eff.messages.success);
				eff.proc.cre(sp, *e, true);
			}
		} else {
			eff.proc.cre(sp, *ti.cre, true);
			if(eff.flags&Splash) {
				source.count = source_select(source_data, sc.creatures, sp, ti.cre->getposition(), 1);
				source.count = exclude(source, ti.cre);
				for(auto& e : source) {
					if(eff.messages.success)
						e->act(eff.messages.success);
					eff.proc.cre(sp, *e, true);
				}
			}
		}
	} else if(eff.proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = source_select(source, sp);
		if((eff.flags&All) != 0) {
			for(auto& e : source)
				eff.proc.itm(sp, *e, true);
		} else {
			eff.proc.itm(sp, *ti.itm, true);
			if(eff.flags&Splash) {
				source.count = exclude(source, ti.itm);
				if(source.count > 2)
					source.count = 2;
				for(auto& e : source)
					eff.proc.itm(sp, *e, true);
			}
		}
	} else if(eff.proc.obj) {
		short unsigned source_data[256]; aref<short unsigned> source(source_data);
		source.count = source_select(source, sc.indecies, sp, 0, 0);
		if((eff.flags&All) != 0) {
			for(auto index : source)
				eff.proc.obj(sp, index, true);
		} else {
			eff.proc.obj(sp, ti.obj, true);
			if(eff.flags&Splash) {
				source.count = source_select(source_data, sc.indecies, sp, ti.obj, 1);
				source.count = exclude(source, ti.obj);
				for(auto index : source)
					eff.proc.obj(sp, index, true);
			}
		}
	}
}

bool creature::choose(const sceneeffect& eff, scene& sc, targetinfo& ti) const {
	sceneparam sp(eff, const_cast<creature&>(*this), true);
	auto los = getlos(eff.flags);
	ti.clear();
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = source_select(source, sc.creatures, sp, sp.player.getposition(), los);
		if(source.count <= 0)
			return false;
		if((eff.flags&All) != 0) {
			return true;
			if(eff.flags&Nearest)
				ti = source.random();
			else
				ti = choose(source, sp.interactive);
			if(!ti.cre)
				return false;
		}
		return true;
	} else if(eff.proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = source_select(source, sp);
		if((eff.flags&All) != 0)
			return true;
		if(eff.flags&Nearest)
			ti = source.random();
		else
			ti = choose(source, sp.interactive);
		if(!ti.itm)
			return false;
		return true;
	} else if(eff.proc.obj) {
		short unsigned source_data[256]; aref<short unsigned> source(source_data);
		source.count = source_select(source, sc.indecies, sp, 0, 0);
		if((eff.flags&All) != 0)
			return true;
		ti = Blocked;
		if(eff.flags&Nearest)
			ti = source.random();
		else
			ti = choose(source, sp.interactive);
		if(ti.obj == Blocked)
			return false;
		return true;
	}
	return false;
}

bool scene::isenemy(const creature& player) const {
	for(auto pe : creatures) {
		if(player.isenemy(pe))
			return true;
	}
	return false;
}