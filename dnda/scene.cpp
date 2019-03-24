#include "main.h"

struct action_info {
	variant			type;
	target_info		target;
	int				priority;
};

template<class T>
static T get_best(aref<T> source) {
	T p1 = 0;
	for(auto p : source) {
		if(!p1 || compare(p, p1) > 0)
			p1 = p;
	}
	return p1;
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

static unsigned source_select(aref<creature*> result, aref<creature*> source, sceneparam& sp, short unsigned index, int range, creature* player) {
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
		if(range > 0 && p==player)
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

unsigned sceneparam::getduration() const {
	return state.duration * level;
}

static unsigned short center_position;

static int compare(const creature* e1, const creature* e2) {
	auto p1 = e1->getposition();
	auto p2 = e2->getposition();
	auto d1 = game::distance(p1, center_position);
	auto d2 = game::distance(p2, center_position);
	if(d1 != d2)
		return d1 - d2;
	return 0;
}

static int compare(const item* e1, const item* e2) {
	return e1 - e2;
}

static int compare(const short unsigned e1, const short unsigned e2) {
	auto d1 = game::distance(e1, center_position);
	auto d2 = game::distance(e2, center_position);
	if(d1 != d2)
		return d1 - d2;
	return 0;
}

bool creature::choose(const effect_info& eff, scene& sc, target_info& ti, bool interactive) const {
	sceneparam sp(eff, const_cast<creature&>(*this), interactive);
	auto los = getlos(eff.flags);
	ti.clear();
	if(eff.proc.cre) {
		creature* source_data[32]; aref<creature*> source(source_data);
		source.count = source_select(source, sc.creatures, sp, sp.player.getposition(), los, &sp.player);
		if(source.count <= 0)
			return false;
		if((eff.flags&All) != 0)
			return true;
		if(eff.flags&Nearest) {
			center_position = getposition();
			ti = get_best(source);
		} else
			ti = choose(source, sp.interactive);
		if(!ti.cre)
			return false;
		return true;
	} else if(eff.proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = source_select(source, sp);
		if(source.count <= 0)
			return false;
		if((eff.flags&All) != 0)
			return true;
		if(eff.flags&Nearest)
			ti = get_best(source);
		else if(source.count > 0)
			ti = choose(source, sp.interactive);
		if(!ti.itm)
			return false;
		return true;
	} else if(eff.proc.obj) {
		short unsigned source_data[256]; aref<short unsigned> source(source_data);
		source.count = source_select(source, sc.indecies, sp, getposition(), los);
		if(source.count <= 0)
			return false;
		if((eff.flags&All) != 0)
			return true;
		ti = Blocked;
		if(eff.flags&Nearest) {
			center_position = getposition();
			ti = get_best(source);
		} else
			ti = choose(source, sp.interactive);
		if(ti.obj == Blocked)
			return false;
		return true;
	}
	return false;
}

void sceneparam::apply(scene& sc, const target_info& ti, const char* format, const char* format_param) {
	auto los = player.getlos(flags);
	if(format)
		player.actv(format, format_param);
	if(messages.action)
		player.act(messages.action);
	if(proc.cre) {
		creature* source_data[32];
		if((flags&All) != 0) {
			aref<creature*> source(source_data);
			source.count = source_select(source_data, sc.creatures, *this, player.getposition(), los, &player);
			for(auto& e : source) {
				if(messages.success)
					e->act(messages.success);
				proc.cre(*this, *e, true);
			}
		} else {
			proc.cre(*this, *ti.cre, true);
			if(flags&Splash) {
				aref<creature*> source(source_data);
				source.count = source_select(source_data, sc.creatures, *this, ti.cre->getposition(), 1, ti.cre);
				for(auto& e : source) {
					if(messages.success)
						e->act(messages.success);
					proc.cre(*this, *e, true);
				}
			}
		}
	} else if(proc.itm) {
		item* source_data[64]; aref<item*> source(source_data);
		source.count = source_select(source, *this);
		if((flags&All) != 0) {
			for(auto& e : source)
				proc.itm(*this, *e, true);
		} else {
			proc.itm(*this, *ti.itm, true);
			if(flags&Splash) {
				source.count = exclude(source, ti.itm);
				if(source.count > 2)
					source.count = 2;
				for(auto& e : source)
					proc.itm(*this, *e, true);
			}
		}
	} else if(proc.obj) {
		short unsigned source_data[256]; aref<short unsigned> source(source_data);
		source.count = source_select(source, sc.indecies, *this, 0, 0);
		if((flags&All) != 0) {
			for(auto index : source)
				proc.obj(*this, index, true);
		} else {
			proc.obj(*this, ti.obj, true);
			if(flags&Splash) {
				source.count = source_select(source_data, sc.indecies, *this, ti.obj, 1);
				source.count = exclude(source, ti.obj);
				for(auto index : source)
					proc.obj(*this, index, true);
			}
		}
	}
}

bool scene::isenemy(const creature& player) const {
	for(auto pe : creatures) {
		if(player.isenemy(pe))
			return true;
	}
	return false;
}