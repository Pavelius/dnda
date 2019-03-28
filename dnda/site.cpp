#include "main.h"

static struct site_info {
	struct search_item { // This is for skill Alertness
		const char*		text;
		char			chance; // Bonus to skill Alertness
		char			quality; // Item quality
		char			magic; // There is a percent random chance to be Blessed or Cursed item. Chance to be Artifact is magic/10.
		explicit operator bool() const { return text != 0; }
	};
	const char*			name;
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
	aref<item_s>		items;
	aref<skill_use_info> skills;
} site_data[] = {{"�������"},
{"�������"},
{"�������� ����"},
{"�������� �����"},
{"���"},
{"������", "� ��������� �������� ���� ������ � ����� ������� �����."},
{"����"},
{"�������", "� ������� ������ ����� ������ � ������ ����� ����. ����, ������� ����� ���� �������� � ����."},
{"������", "��������� �������� ������ ����� ����. ������ ����� ������ ������."},
{"��������� �����"},
{"������� ������ � �����", "��������� ������� � ������� �� ������ � �� ������� ������ ��������� ������ �� �������."},
{"������� ������� � ������", "��� ��� ��������� ������� � ������� ������ ������ � ����. ����� ������ ����� � �������� �� ��� ���������� ����������� �������������."},
{"������� ���"},
};
assert_enum(site, ShopFood);
getstr_enum(site);

int	site::getfoundchance() const {
	if(found > 5)
		return 0;
	return 100 - (found * 20);
}

void site::wait(unsigned value) {
	if(!recoil)
		recoil = game::getseconds();
	recoil += value;
}

short unsigned site::getposition() const {
	return game::get(x1 + width() / 2, y1 + height() / 2);
}

void site::entering(creature& player) {
	if(site_data[type].entering)
		player.hint(site_data[type].entering);
}

void site::update() {
	if(recoil > game::getseconds())
		return;
	if(game::isdungeon())
		game::spawn(game::getfree(getposition()));
	wait(xrand(5 * Minute, 15 * Minute));
}

char* site::getname(char* result) const {
	zcpy(result, getstr(type));
	return result;
}

aref<skill_use_info> site::getskills() const {
	return site_data[type].skills;
}