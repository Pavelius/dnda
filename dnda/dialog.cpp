#include "main.h"

aref<const speech*> dialog::select(aref<const speech*> source, const speech* p, speech_s type) {
	auto pb = source.data;
	auto pe = pb + source.count;
	if(p) {
		for(; *p; p++) {
			if(p->type != type)
				continue;
			if(p->proc) {
				if(!p->proc(*this, *p, false))
					continue;
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
		}
	}
	return aref<const speech*>(source.data, pb - source.data);
}

const speech* dialog::say(const speech* pb, speech_s type) {
	const speech* source[32];
	auto result = select(source, pb, type);
	if(!result)
		return 0;
	if(type == Speech) {
		zshuffle(result.data, result.count);
		opponent->sayvs(*player, result.data[0]->text);
	} else {
		for(auto p : result)
			logs::add(p - pb, p->text);
	}
	logs::add("\n");
	return pb;
}

const speech* dialog::phase(const speech* p) {
	say(p, Speech);
	if(say(p, Answer)) {
		auto index = logs::input();
		auto ap = p + index;
		result = ap->success;
		if(ap->fail && ap->skill.id) {
			if(!player->roll(ap->skill.id, -ap->skill.value))
				result = ap->fail;
		}
		// Procedure call after skill check.
		// This due to result is known.
		if(ap->proc)
			ap->proc(*this, *ap, true);
	} else {
		result = p->success;
		if(result)
			logs::next();
	}
	return result;
}

void dialog::start(const speech* p) {
	while(p)
		p = phase(p);
}

static speech old_house[] = {{Speech, 0, "� ��� ������������? �� ��� �������� ��� �� ������ ����� �����. ������ ����� �������� � ��� ����� �� �����."},
{Speech, 0, "����� ������ ������ ����� �� ���� ������ ����. ������ ��� �����-�� ��������� �����-�� ���������� ��������."},
{Speech, 0, "� ��� ����� �������, ��� �� ����� ���-�� ���������� � ���� ���� � ����������. �� ��� ���� ������� - ����� ������� ������ �� �����."},
{Speech, 0, "������� ����� ���. �� ������������, ������."},
{}};
static speech test_dialog[] = {{Speech, 0, "������ %�����. ��� ���� ���� �������?"},
{Answer, 0, "��� �� ������ ���������� ��� ������ ��� � �������?", old_house},
{Answer, 0, "��� �� ����� �� ����� ��������� ������ ��� �����?"},
{Answer, 0, "�������� ��� ����� �������� ������������ ����������� ��������������?", 0, 0, {Diplomacy, 20}},
{Answer, 0, "���� ������ �� ����."},
{}};
static speech party_member[] = {{Speech, 0, "����� �����?"},
{Speech, 0, "��� ����� ������?"},
{Speech, 0, "������."},
{Answer, 0, "����� �� ����.", old_house},
{Answer, 0, "������� ��� �����."},
{Answer, 0, "������ ����������. ���������� ��������."},
{}};
static speech smalltalk[] = {{Speech, 0, "������! ��� ����?"},
{Speech, 0, "������� ����, ��?"},
{Speech, 0, "��! �����%� �..."},
{}};

static void chat_smalltalk(creature* player, creature* opponent) {
	static const char* talk[] = {
		"������! ��� ����?",
		"������� ����, ��?",
		"��! �����%� �...",
	};
	player->say(maprnd(talk), opponent->getname());
}

static void chat_boss(creature* player, creature* opponent) {
	static const char* talk[] = {
		"����� �����?",
		"��� ����� ������?",
		"������.",
	};
	player->say(maprnd(talk), opponent->getname());
}

static site* get_non_explored_location() {
	site* source[128];
	auto pb = source;
	auto pe = source + sizeof(source) / sizeof(source[0]);
	for(auto& e : locations) {
		if(e.type == House)
			continue;
		rect rc = e;
		//if(game::isexplore(center(rc)))
		//	continue;
		*pb++ = &e;
	}
	auto count = pb - source;
	if(!count)
		return 0;
	return source[rand() % count];
}

static bool chat_location(creature* player, creature* opponent) {
	auto p = get_non_explored_location();
	if(!p)
		return false;
	static const char* talk_start[] = {
		"�� ��� ������ ������ ��������� %1.",
		"� ��� ������� ��������� %1.",
		"���� ������� � ��� ����������� ������� %1.",
	};
	// ��������� �������
	for(auto x = p->x1; x <= p->x2; x++)
		for(auto y = p->y1; y <= p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char temp[4096];
	char name[260]; p->getname(name);
	szprints(temp, zendof(temp), maprnd(talk_start), name);
	player->say(temp);
	return true;
}

void creature::chat(creature* e) {
	enum boss_commands {
		NoBossCommand,
		GuardPlace, FollowMe,
	};
	if(isfriend(e) && e->getleader() == this && e->role == Character) {
		dialog dg(this, e);
		dg.start(test_dialog);
		return;
	}
	if(isfriend(e) && e->getleader() == this && e->role == Character) {
		chat_boss(e, this);
		if(e->isguard())
			logs::add(FollowMe, "����� �� ����.");
		else
			logs::add(GuardPlace, "������� ��� �����.");
		logs::add(NoBossCommand, "������. ���������� ��������.");
		switch(logs::input()) {
		case GuardPlace:
			e->guard = e->position;
			break;
		case FollowMe:
			e->guard = Blocked;
			e->party = this;
			break;
		}
		return;
	}
	if(d100() < 30 && chat_location(e, this)) {
		e->wait(3);
		return;
	}
	chat_smalltalk(e, this);
	e->wait(2);
}

int creature::getreaction(creature* opponent) const {
	if(!opponent)
		return 0;
	auto result = opponent->get(Charisma) * 2;
	if(opponent->gender != gender)
		result += opponent->get(Charisma) - 8;
	if(is(Anger))
		result -= 40;
	if(is(Goodwill))
		result += 30;
	if(is(Drunken))
		result += 10;
	return result;
}