#include "main.h"

static void setstate(creature& player, item& it, const itemuse& e) {
	if(e.value.type == State)
		player.set(e.value.state, e.duration);
}

static void waitmore(creature& player, item& it, const itemuse& e) {
	if(e.value.type == State)
		player.set(e.value.state, e.duration);
}

static void nothing(creature& player, item& it, const itemuse& e) {
}

static const itemuse side_tome[] = {{2, "������ ��������� � ����� ���� �� ������ � ��� ������ ������ ��� �� ����.", setstate, {}, Anger, Hour},
{1, "������ ������ ������ ������� ��� �� ������.", waitmore, {}, {}, Hour},
{1, "������ ������ ������� ������ ������� ��� �� ������.", waitmore, {}, {}, Hour * 2},
{}};
static const itemuse spell_tome_fail[] = {{2, "������ ��������� � ����� ���� �� ������ � ��� ������ ������ ��� �� ����.", setstate, {}, Anger, Hour},
{1, "������ ������ ������ ������� ��� �� ������.", waitmore, {}, {}, Hour},
{1, "������ ������ ������� ������ ������� ��� �� ������.", waitmore, {}, {}, Hour * 2},
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
		if(result <= total)
			return p;
		total += p->chance;
	}
	return 0;
}

void itemuse::apply(creature& player, item& it) const {
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
	act("%����� �����%� ������� %L1.", temp);
	if(d100() < ri.chance_side) {
		side_tome->applyrnd(*this, it);
		if(!it)
			return;
	}
	if(!roll(Literacy, ri.bonus)) {
		hint("� �����-������ ��� ��������� ���������� ��� ���� ������ ������.");
		return;
	}
	auto spell = it.getspell();
	auto skill = it.getskill();
	if(spell) {
		auto level = get(spell);
		auto chance = maptbl(chance_learn_spell, level);
		if(d100() < chance) {
			level++;
			it.set(KnowQuality);
			if(level == 1)
				hint("�� ������� ���������� '%1'.", getstr(spell));
			else
				hint("��� ����� �������� ����������� '%1' ��������� �� ������ %2i.", getstr(spell), level);
			set(spell, level);
		} else
			spell_tome_fail->applyrnd(*this, it);
	} else if(skill)
		raise(skill);
	if(!it)
		return;
	if(d100() < ri.disapear) {
		act("��������� ������� %1 �������.");
		it.clear();
	}
}