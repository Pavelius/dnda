#include "main.h"

short unsigned center(const rect& rc);

bool lowint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 7;
}

bool noint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 2;
}

static bool isguard(const creature& player, const dialog& e) {
	return player.getguard() != Blocked;
}

static bool isnoguard(const creature& player, const dialog& e) {
	return player.getguard() == Blocked;
}

static bool ishightpriest(const creature& player, const dialog& e) {
	auto p = player.getsite();
	return p && p->type == Temple && p->owner == &player;
}

static bool ishenchmen(const creature& player, const dialog& e) {
	return player.isfriend(e.player) && player.getrole() == Character;
}

static void setguard(dialog& e, const speech& sp) {
	e.opponent->setguard(e.opponent->getposition());
}

static void remguard(dialog& e, const speech& sp) {
	e.opponent->setguard(Blocked);
}

static site* get_non_explored_location() {
	site* source[128];
	auto pb = source;
	auto pe = source + sizeof(source) / sizeof(source[0]);
	for(auto& e : game::getsites()) {
		if(e.type == House)
			continue;
		if(game::is(center(e), Explored))
			continue;
		*pb++ = &e;
	}
	auto count = pb - source;
	if(!count)
		return 0;
	return source[rand() % count];
}

static void knownbuilding(dialog& e, const speech& sp) {
	auto p = get_non_explored_location();
	if(!p) {
		static const char* talk[] = {
			"� �� ���� ���������, ��� � ��� ��� � ���, �� ������ �� � ��� ��� ������.",
			"�� �� � ��� ��� ������ - ��� ������ ���� ����������.",
			"� ���������, ��� ������ ���� ���������.",
		};
		e.opponent->say(maprnd(talk));
		return;
	}
	static const char* talk_start[] = {
		"�� ��� ������ ������ ��������� %1.",
		"� ��� ������� ��������� %1.",
		"���� ������� � ��� ����������� ������� %1.",
	};
	// Remove FOW
	for(auto x = p->x1; x < p->x2; x++)
		for(auto y = p->y1; y < p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char name[260]; p->getname(name);
	e.opponent->say(maprnd(talk_start), name);
}

static speech hero_history[] = {{Speech, 0, "� �������� ��� ��� ��������. ������� ������� �����, ������� ����� � �����."},
{}};
static speech party_member[] = {{Action, noint, "%����� ������������ ������."},
{Action, noint, "%����� �������� ������ �� ������."},
{Action, noint, "%����� � ��������� ������� �� ����.", 0, 0, true},
{Speech, lowint, "���� ���� ������?"},
{Speech, lowint, "���?", 0, 0, true},
{Speech, 0, "����� �����?"},
{Speech, 0, "��� ����� ������?"},
{Speech, 0, "������."},
{Answer, isguard, "����� �� ����.", 0, 0, false, {}, remguard},
{Answer, isnoguard, "������� ��� �����.", 0, 0, false, {}, setguard},
{Answer, 0, "���� �� ��������� �������? ���� ������� �������. ����� ����� ���-��."},
{Answer, 0, "������� �� ������. �� �� ��������� �� ������� �� ����?", hero_history},
{Answer, 0, "������ ����������. ���������� ��������."},
{}};
static speech priest_talk[] = {{Speech, 0, "��� ��� ���� ����� ��� ���?"},
{Speech, 0, "���� ��� �������?"},
{Answer, 0, "�������� ��� ��� ��� �� �����?", priest_talk},
{Answer, 0, "����� �������� ������? � �� ����� �������?", priest_talk},
{Answer, 0, "������� ��� ���� ����.", priest_talk},
{}};
static speech smalltalk[] = {{Speech, ishenchmen, "��, ����, ������.", 0, party_member, true},
{Action, noint, "%����� ����������� �����."},
{Action, noint, "%����� ���������� ������� �� %�����.", 0, 0, true},
{Speech, lowint, "���� ���� ������?"},
{Speech, lowint, "���� ��� ���������?"},
{Speech, lowint, "��� �����%� �������."},
{Speech, lowint, "��� ������ �����.", 0, 0, true},
{Speech, ishightpriest, "��������� ����. ���� �������?", 0, priest_talk, true},
{Speech, 0, "������! ��� ����?"},
{Speech, 0, "������� ����, ��?"},
{Speech, 0, "��! �����%� �..."},
{Speech, 0, 0, 0, 0, false, {}, knownbuilding},
{}};

void creature::chat(creature* e) {
	dialog dg(this, e);
	dg.start(smalltalk);
}