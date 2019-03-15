#include "main.h"

bool cre_damage(sceneparam& e, creature& opponent, bool run) {
	if(run) {
		opponent.damage(e.damage.roll(), e.damage.type, e.interactive);
		if(e.duration)
			opponent.set(e.state, e.duration);
	}
	return true;
}

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

void creature::apply(const sceneeffect& eff, scene& sc, const targetinfo& ti) {
	sceneparam sp(eff, *this, true);
	auto los = getlos(eff.flags);
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = source_select(source, sc.creatures, sp, sp.player.getposition(), los);
		if((eff.flags&All) != 0) {
			for(auto& e : source)
				eff.proc.cre(sp, *e, true);
		} else {
			eff.proc.cre(sp, *ti.cre, true);
			if(eff.flags&Splash) {
				source.count = source_select(source_data, sc.creatures, sp, ti.cre->getposition(), 1);
				source.count = exclude(source, ti.cre);
				for(auto& e : source)
					eff.proc.cre(sp, *e, true);
			}
		}
	} else if(eff.proc.itm) {
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
				ti.cre = source.random();
			else
				ti.cre = choose(source, sp.interactive);
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
			ti.itm = source.random();
		else
			ti.itm = choose(source, sp.interactive);
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

bool creature::apply(const sceneeffect& eff, scene& sc, bool run) {
	sceneparam sp(eff, *this, true);
	auto los = getlos(eff.flags);
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = source_select(source, sc.creatures, sp, sp.player.getposition(), los);
		if(run) {
			if((eff.flags&All) != 0) {
				for(auto& e : source)
					eff.proc.cre(sp, *e, true);
			} else {
				creature* opponent = 0;
				if(eff.flags&Nearest)
					opponent = source.random();
				else
					opponent = choose(source, sp.interactive);
				if(!opponent)
					return false;
				eff.proc.cre(sp, *opponent, true);
				if(eff.flags&Splash) {
					source.count = source_select(source_data, sc.creatures, sp, opponent->getposition(), 1);
					source.count = exclude(source, opponent);
					for(auto& e : source)
						eff.proc.cre(sp, *e, true);
					return true;
				}
			}
		}
		return source.count != 0;
	} else if(eff.proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = source_select(source, sp);
		if(run) {
			if((eff.flags&All) != 0) {
				for(auto& e : source)
					eff.proc.itm(sp, *e, true);
			} else {
				item* value = 0;
				if(eff.flags&Nearest)
					value = source.random();
				else
					value = choose(source, sp.interactive);
				if(!value)
					return false;
				eff.proc.itm(sp, *value, true);
				if(eff.flags&Splash) {
					source.count = exclude(source, value);
					if(source.count > 2)
						source.count = 2;
					for(auto& e : source)
						eff.proc.itm(sp, *e, true);
					return true;
				}
			}
		}
		return source.count != 0;
	} else if(eff.proc.obj) {
		short unsigned source_data[256]; aref<short unsigned> source(source_data);
		source.count = source_select(source, sc.indecies, sp, 0, 0);
		if(run) {
			if((eff.flags&All) != 0) {
				for(auto index : source)
					eff.proc.obj(sp, index, true);
			} else {
				short unsigned value = Blocked;
				if(eff.flags&Nearest)
					value = source.random();
				else
					value = choose(source, sp.interactive);
				if(value == Blocked)
					return false;
				eff.proc.obj(sp, value, true);
				if(eff.flags&Splash) {
					source.count = source_select(source_data, sc.indecies, sp, value, 1);
					source.count = exclude(source, value);
					for(auto index : source)
						eff.proc.obj(sp, index, true);
					return true;
				}
			}
		}
		return source.count != 0;
	}
	return false;
}