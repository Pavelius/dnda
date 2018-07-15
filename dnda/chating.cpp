#include "main.h"

static bool lowint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 7;
}

static bool isanimal(const creature& player, const dialog& e) {
	return player.race == Animal;
}

static bool isguard(const creature& player, const dialog& e) {
	return player.guard != Blocked;
}

static bool isnoguard(const creature& player, const dialog& e) {
	return player.guard == Blocked;
}

static void setguard(dialog& e, const speech& sp) {
	e.opponent->guard = e.opponent->position;
}

static void remguard(dialog& e, const speech& sp) {
	e.opponent->guard = Blocked;
}

static speech old_house[] = {{Speech, 0, "� ��� ������������? �� ��� �������� ��� �� ������ ����� �����. ������ ����� �������� � ��� ����� �� �����."},
{Speech, 0, "����� ������ ������ ����� �� ���� ������ ����. ������ ��� �����-�� ��������� �����-�� ���������� ��������."},
{Speech, 0, "� ��� ����� �������, ��� �� ����� ���-�� ���������� � ���� ���� � ����������. �� ��� ���� ������� - ����� ������� ������ �� �����."},
{Speech, 0, "������� ����� ���. �� ������������, ������."},
{}};
static speech test_dialog[] = {{Speech, 0, "������ %�����. ��� ���� ���� �������?"},
{Answer, 0, "��� �� ������ ���������� ��� ������ ��� � �������?", old_house},
{Answer, 0, "��� �� ����� �� ����� ��������� ������ ��� �����?"},
{Answer, 0, "�������� ��� ����� �������� ������������ ����������� ��������������?", 0, 0, false, {Diplomacy, 20}},
{Answer, 0, "���� ������ �� ����."},
{}};
static speech party_member[] = {{Speech, isanimal, "������������ ������."},
{Speech, isanimal, "�������� ������ �� ������.", 0, 0, true},
{Speech, lowint, "���� ���� ������?"},
{Speech, lowint, "���?", 0, 0, true},
{Speech, 0, "����� �����?"},
{Speech, 0, "��� ����� ������?"},
{Speech, 0, "������."},
{Answer, isguard, "����� �� ����.", 0, 0, false, {}, remguard},
{Answer, isnoguard, "������� ��� �����.", 0, 0, false, {}, setguard},
{Answer, 0, "������ ����������. ���������� ��������."},
{}};
static speech smalltalk[] = {{Action, isanimal, "%����� ����������� �����."},
{Action, isanimal, "%����� ���������� ������� �� %�����.", 0, 0, true},
{Speech, lowint, "���� ���� ������?"},
{Speech, lowint, "���� ��� ���������?"},
{Speech, lowint, "��� �����%� �������."},
{Speech, lowint, "��� ������ �����.", 0, 0, true},
{Speech, 0, "������! ��� ����?"},
{Speech, 0, "������� ����, ��?"},
{Speech, 0, "��! �����%� �..."},
{}};

static site* get_non_explored_location() {
	site* source[128];
	auto pb = source;
	auto pe = source + sizeof(source) / sizeof(source[0]);
	for(auto& e : game::getsites()) {
		if(e.type == House)
			continue;
		//if(game::is(center(e), Explored))
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
	for(auto x = p->x1; x < p->x2; x++)
		for(auto y = p->y1; y < p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char temp[4096];
	char name[260]; p->getname(name);
	szprints(temp, zendof(temp), maprnd(talk_start), name);
	player->say(temp);
	return true;
}

void creature::chat(creature* e) {
	dialog dg(this, e);
	if(isfriend(e) && e->getleader() == this && e->role == Character)
		dg.start(party_member);
	else if(d100() < 30 && chat_location(e, this))
		e->wait(3);
	else
		dg.start(smalltalk);
}