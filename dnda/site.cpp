#include "main.h"

static struct site_info {
	struct search_item { // This is for skill Alertness
		const char*		text;
		char			chance; // Bonus to skill Alertness
		char			quality; // Item quality
		char			magic; // Blessed or Cursed is fate decided chance to be artifact is magic/10.
		aref<item_s>	items;
		explicit operator bool() const { return text != 0; }
	};
	struct monster_info {
		role_s			owner;
		role_s			wandering;
	};
	const char*			name;
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
	monster_info		appear;
	aref<effectinfo>	skills;
} site_data[] = {{"�������"},
{"�������"},
{"�������� ����"},
{"�������� �����"},
{"���"},
{"����"},
{"�������"},
{"������"},
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