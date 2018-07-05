#include "main.h"

static void setstate(creature& player, item& it, const itemuse& e) {
	if(e.value.type == State)
		player.set(e.value.state, e.duration);
}

static void waitmore(creature& player, item& it, const itemuse& e) {
	if(e.value.type == State)
		player.set(e.value.state, e.duration);
}

static void dealdamage(creature& player, item& it, const itemuse& e) {
	player.damage(e.damage.roll(), e.damage.type, true);
}

static void drainmana(creature& player, item& it, const itemuse& e) {
	player.consume(e.damage.roll(), true);
}

static void drainexperience(creature& player, item& it, const itemuse& e) {
	player.addexp(e.damage.roll());
}

static const itemuse side_tome[] = {{2, "Манера изложения в книге была не лучшая и это просто вывело вас из себя.", setstate, {}, Anger, Hour},
{1, "Чтение заняло больше времени чем вы думали.", waitmore, {}, {}, Hour},
{1, "Чтение заняло намного больше времени чем вы думали.", waitmore, {}, {}, Hour * 2},
{}};
static const itemuse spell_tome_fail[] = {{1, "Книга истощила вас физически.", dealdamage, {1, 3, Magic}},
{1, "После утомленного вчитывания вы произнесли одно лишь слово из формулы заклинания вслух.", dealdamage, {1, 6, Fire}},
{2, "Книга истощила вас морально.", drainmana, {2, 5}},
{1, "Книга начала лишать вас памяти.", drainexperience, {5, 10}},
{}};

const itemuse* itemuse::random() const {
	if(!this)
		return 0;
	auto total = 0;
	for(auto p = this; *p; p++)
		total += p->chance;
	if(!total)
		return 0;
	auto result = rand() % total;
	total = 0;
	for(auto p = this; *p; p++) {
		total += p->chance;
		if(result < total)
			return p;
	}
	return 0;
}

void itemuse::apply(creature& player, item& it) const {
	if(text)
		player.hint(text);
	if(proc)
		proc(player, it, *this);
}

void itemuse::applyrnd(creature& player, item& it) const {
	auto p = random();
	if(p)
		apply(player, it);
}

void creature::readbook(item& it) {
	static char chance_learn_spell[] = {100, 70, 40, 30, 20, 10, 5, 2};
	char temp[260]; it.getname(temp, zendof(temp), false);
	auto& ri = it.getspecial();
	wait(Hour);
	act("%герой начал%а изучать %L1.", temp);
	if(d100() < ri.chance_side)
		side_tome->applyrnd(*this, it);
	if(!it)
		return;
	if(roll(Literacy, ri.bonus)) {
		auto spell = it.getspell();
		auto skill = it.getskill();
		if(spell) {
			auto level = get(spell);
			auto chance = maptbl(chance_learn_spell, level);
			if(d100() < chance) {
				level++;
				it.set(KnowQuality);
				if(level == 1)
					hint("Вы изучили заклинание '%1'.", getstr(spell));
				else
					hint("Ваш навык владения заклинанием '%1' улучшился до уровня %2i.", getstr(spell), level);
				set(spell, level);
			} else
				spell_tome_fail->applyrnd(*this, it);
		} else if(skill)
			raise(skill);
	} else
		hint("В конце-концов вам неудалось почерпнуть для себя ничего нового.");
	if(!it)
		return;
	if(d100() < ri.disapear) {
		act("Волшебным образом %L1 исчез.", temp);
		it.clear();
	}
}