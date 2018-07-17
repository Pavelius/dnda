#include "main.h"

void setstate(effectparam& e) {
	// Set state and increase duration by level
	auto duration = e.state.duration * e.level;
	e.cre->set(e.state.type, duration);
	if(e.state.type == Scared) {
		if(!e.cre->saving(e.interactive, ResistCharm, 0)) {
			e.cre->set(e.state.type, duration);
			e.cre->sethorror(&e.player);
		}
	} else if(e.state.type == Charmed) {
		if(!e.cre->saving(e.interactive, ResistCharm, 0)) {
			e.cre->set(e.state.type, duration);
			e.cre->setcharmer(&e.player);
		}
	} else if(e.state.type == Paralized) {
		if(!e.cre->saving(e.interactive, ResistParalize, 0))
			e.cre->set(e.state.type, duration);
	} else
		e.cre->set(e.state.type, duration);
}

static void setdamage(effectparam& e) {
	// Apply damage to level
	auto damage = e.damage;
	if(e.level > 1) {
		auto level = e.level - 1;
		damage.min += level;
		if(damage.max > 10)
			damage.max += level * 2;
		else
			damage.max += level;
	}
	e.cre->damage(damage.roll(), e.damage.type, true);
}

static void healdamage(effectparam& e) {
	// Heal damage according level
	e.cre->heal(e.damage.roll() + e.level - 1, true);
}

static void repair(effectparam& e) {
	e.itm->repair(e.level);
}

static void bless_item(effectparam& e) {
	// Bless item has chance to make item cursed and dependson level.
	auto chance = 60 + e.level * 5;
	if(d100() < chance)
		e.itm->set(BlessedItem);
	else
		e.itm->set(Cursed);
}

static void detect_evil(effectparam& e) {
	// Detect only first level items
	char temp[260];
	if(e.param >= e.level)
		return;
	if(e.itm->getidentify() < KnowEffect && e.itm->iscursed()) {
		e.player.act("%1 осветился красным светом.", e.itm->getname(temp, zendof(temp)));
		e.itm->set(KnowEffect);
		e.param++;
	}
}

static void detect_magic(effectparam& e) {
	// Detect only first level items
	char temp[260];
	if(e.param >= e.level)
		return;
	if(e.itm->getidentify() < KnowEffect && e.itm->ismagical()) {
		e.player.act("%1 осветился голубым светом.", e.itm->getname(temp, zendof(temp)));
		e.param++;
	}
}

static void identify(effectparam& e) {
	char temp[260];
	e.player.act("%1 на мгновение осветился белым светом.", e.itm->getname(temp, zendof(temp)));
	// Identify has little chance to curse item
	auto chance = 30 - e.level * 2;
	if(d100() < chance && e.itm->getmagic() != Artifact)
		e.itm->set(Cursed);
	e.itm->set(KnowEffect);
}

static struct spell_info {
	const char*			name;
	const char*			nameof;
	short unsigned		cost;
	char				levels[2];
	effectinfo			effect;
} spell_data[] = {{"", ""},
{"Броня", "брони", 10, {}, {{TargetSelf}, {setstate}, {Armored, 4 * Hour}, "%герой озарил%ась синим светом."}},
{"Благословение", "благословения", 8, {}, {{TargetFriendly, 2}, {setstate}, {Blessed, Turn}, "%герой озарил%ась желтым светом."}},
{"Благословить предмет", "благословения", 10, {0, 10}, {{TargetItemMundane}, {bless_item}, {}}},
{"Очаровать персону", "шарма", 13, {}, {{TargetFriendly, 4}, {setstate}, {Charmed, Day}, "Внезапно %герой стал%а вести себя дружелюбно."}},
{"Определить зло", "определения зла", 12, {}, {{TargetInvertory}, {detect_evil}, {}}},
{"Определить магию", "определения магии", 8, {}, {{TargetInvertory}, {detect_magic}, {}}},
{"Страх", "страха", 5, {}, {{TargetHostile, 5, 2}, {setstate}, {Scared, 5 * Minute}, "%герой запаниковал%а и начал%а бежать.", {}}},
{"Лечение", "лечения", 7, {}, {{TargetFriendly, 1}, {healdamage}, {}, "%герой озарился белым светом.", {1, 8, Magic}}},
{"Опознать предмет", "опознания", 20, {3}, {{TargetItemUnidentified}, {identify}, {}}},
{"Невидимость", "невидимости", 8, {}, {{TargetFriendly, 1}, {setstate}, {Hiding, Hour}, "%герой исчез%ла из виду."}},
{"Свет", "света", 1, {}, {{TargetFriendly, 1}, {setstate}, {Lighted, Hour}, "Вокруг %героя появилось несколько светящихся шариков."}},
{"Волшебный снаряд", "колдовства", 3, {}, {{TargetHostile, 6}, {setdamage}, {}, "Из пальцев %ГЕРОЯ вылетело несколько светящихся шариков.", {2, 8, Magic}}},
{"Починка", "ремонта", 10, {}, {{TargetItemDamaged}, {repair}, {}, "%герой на мгновение зажгл%ась синим светом и теперь выглядит не таким сломанным."}},
{"Исцелить яд", "лечения яда", 15, {}, {{TargetFriendly}, {setstate}, {RemovePoison}, "%герой на мгновение окутался желтым свечением."}},
{"Исцелить болезнь", "лечения болезней", 15, {}, {{TargetFriendly}, {setstate}, {RemoveSick}, "%герой на мгновение окутался зеленым свечением."}},
{"Щит", "щита", 6, {}, {{TargetSelf}, {setstate}, {Shielded, Hour / 2}, "Перед %героем появился полупрозрачный барьер."}},
{"Шокирующая хватка", "электричества", 4, {}, {{TargetHostile, 1}, {setdamage}, {}, "Электрический разряд поразил %героя.", {3, 12, Electricity}}},
{"Усыпление", "усыпления", 5, {}, {{TargetHostile, 4}, {setstate}, {Sleeped, Minute}, "Внезапно %герой заснул%а.", {}}},
};
assert_enum(spell, LastSpell);
getstr_enum(spell);

static int get_index(class_s id) {
	switch(id) {
	case Paladin:
	case Cleric:
		return 1;
	case Mage:
		return 0;
	default:
		return -1;
	}
}

const char* item::getname(spell_s value) {
	return spell_data[value].nameof;
}

bool creature::isallow(spell_s value) const {
	auto index = get_index(type);
	if(index == -1)
		return spell_data[value].levels[0] == 0 && spell_data[value].levels[1] == 0;
	return level >= spell_data[value].levels[index];
}

int creature::getcost(spell_s value) const {
	return spell_data[value].cost;
}

bool creature::use(spell_s value) {
	auto cost = getcost(value);
	if(getmana() < cost) {
		hint("Не хватает маны.");
		return false;
	}
	auto result = use(value, 1, "%герой прокричал%а мистическую формулу.");
	if(result)
		mp -= cost;
	wait(Minute);
	return result;
}

bool creature::use(spell_s value, int level, const char* format, ...) {
	if(level < 1)
		level = 1;
	return apply(spell_data[value].effect, level, isplayer(), format, xva_start(format), 0, 0);
}

static int compare(const void* p1, const void* p2) {
	auto e1 = *((spell_s*)p1);
	auto e2 = *((spell_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

bool logs::choose(creature& e, spell_s& result) {
	unsigned count = 0;
	spell_s source[sizeof(spell_data) / sizeof(spell_data[0])];
	for(auto i = FirstSpell; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(e.get(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare);
	return logs::choose(e, result, {source, count});
}

spell_s creature::aispell(aref<creature*> creatures, target_s target) {
	adat<spell_s, LastSpell + 1> recomended;
	auto mana = getmana();
	for(auto i = (spell_s)1; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(!spells[i])
			continue;
		if(spell_data[i].effect.type.iscreature())
			continue;
		if(getcost(i) > mana)
			continue;
		if(target && spell_data[i].effect.type.target != target)
			continue;
		if(!spell_data[i].effect.type.isallow(*this, creatures))
			continue;
		recomended.add(i);
	}
	if(recomended.count > 0)
		return recomended.data[rand() % recomended.count];
	return NoSpell;
}

bool creature::aiusewand(aref<creature*> creatures, target_s target) {
	for(auto& it : wears) {
		if(!it)
			continue;
		auto spell = it.getspell();
		if(spell) {
			if(it.istome())
				continue;
			if(it.getcharges() <= 0)
				continue;
			if(!spell_data[spell].effect.type.iscreature())
				continue;
			if(target && spell_data[spell].effect.type.target != target)
				continue;
			if(!spell_data[spell].effect.type.isallow(*this, creatures))
				continue;
			use(it);
			return true;
		}
	}
	return false;
}