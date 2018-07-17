#include "main.h"

void manual_ability_skills(stringbuffer& sc, manual& e);
void manual_skill_ability(stringbuffer& sc, manual& e);
void manual_skill_race(stringbuffer& sc, manual& e);
void manual_skill_class(stringbuffer& sc, manual& e);
void manual_skill_focus_item(stringbuffer& sc, manual& e);

static manual::proc ability_procs[] = {manual_ability_skills};
static manual::proc skill_procs[] = {manual_skill_focus_item, manual_skill_ability, manual_skill_race, manual_skill_class};

static manual strenght[] = {{Element, "���� ���������", "��� **������ �������� ���** ��������� � ����� ������� �� ���������� (����-10)%%."},
{Element, "�������������� ����", "��� **������ �������� ���** ��������� � ������������ ������� ����� (����-10)/2."},
{Element, "����������������", "�������� ����� ������ ��� �������� **2.5 ��** �� ������ ������� ����. ���� ��� ��������� ����� �� **5 ��** �� ������� ����, �������� ����� �������� � ����� ��������� ���������. ���� �������� ����� ����� **5 ��** �� ������� ����, �� � ���������� � ����� ����� ����� ����� [--20%%] � ������ � [--20%%] � ����� ������� �� ����������. ����� ���� �������� �������� ��� ������ �������� ������������� �����, ������� �� ������ �� �����."},
{}};
static manual dexterity[] = {{Element, "���� ���������", "��� **������������� ������** ��������� � ����� ������� �� ���������� (��������-10)%%."},
{Element, "���� ���������", "��� **�����** �� �������� ��������� ���� ���� ������� �� 1%% �� ������ 2 ������� ��������."},
{Element, "�������� �����", "��� **�����** ����� ���������� �������� ��� �������� �������� [+16+] � ���������� �������� ��� �������� [-6-]."},
{}};
static manual constitution[] = {{Element, "�������������� ����", "��� ��������� �������� ���������� **�����** ������ ��� ������������ ���� ��������� ����� ����� �� ������."},
{Element, "�������������� �����", "����� �������� ��������������� 1 ��� �� ����� ������ (40 - ������������) ������� �����."},
{}};
static manual intellegence[] = {{Element, "�������������� ����", "��� ��������� �������� ���������� **����** ������ ��� ����������."},
{Element, "�������������� ����", "����� �������� ��������������� 1 ���� �� ����� ������ (50 - ��������� � 2) � 20 ������� ������."},
{Element, "��������� �������", "�� ���������� ������� ���������� ��������� ������� � ������ �������. ���, ��� ���������� 10 �������� �������� [4] ���������, ��� ���������� 12 �������� [5] ���������, ��� ���������� 14 �������� [6] ���������, � ��� ���������� 17 �������� [7] ���������."},
{}};
static manual wisdow[] = {{Element, "�������������� ����", "�������� **����** ������������� �� ��������/2. ����� ��������� ��� ������ ��������� ����� **������������**."},
{}};
static manual charisma[] = {
	{}};
static manual item_states[] = {{Element, "�������", "�������� ������� ����������� ���� ����� ����� � �������. ����� ������� **��������**, �������� � ����������� ������� �������."},
{Element, "���������", "��������� �������� �������� ������ ��� ����� ���������. �� **��������** �� ������� ����, �� �������� ��������� ��������������� ������. ��� ����� ��������, �� �� ������ ���������."},
{Element, "��������������", "��������, ������� ����� � ���� ������������� �����. �� **��������** �� ������� ����, ��� ����� ��������, �� �� ������ ���������."},
{Element, "�������", "�������� �������� ���� � ��������. �� **��������** �� ��� ������� ����, ��� �� �������� � �� ������ ���������, �� ���� �� ��������� ������ ������."},
{}};
static manual ability_manual[] = {{Header, Strenght, "���� �������� ����� ���������� ����, ���������� ��������� � ������������ ���������. ��� ����������� �������� ������� ��� ������, ��� ��� ��� ������ ���� ��������, ����� ������ ������� � ������� ������� �������.", strenght, ability_procs},
{Header, Dexterity, "�������� ������������� ��������� ���������� ����������� ���������, ������� ����������� ��������, ����������, �������� �������, �������� � ������������� �������. �������� ������ �� ������� ��������� �� ��������� ��� �������������, �� ��� ������ ���������� � ����������� ������� � ������, � ����� �� ��� ������������ �� ������. �������� - ������� ����������� ��� ������, ��� ����� ��� �� ���������������� �������", dexterity, ability_procs},
{Header, Constitution, "������������ ��������� ������������� ��� ���������� ������, ������������� ����������, �������� � ������������ � �����������, �������� � ������������. ��� ��� ��� ����������� ������ �� ��������� ������ ��������� � ��� ����� ��������� ����� �������� ����������, ��� ���������� ����������� ��� ����������� �� �������, ��� �������� �������� ������ ��� ���� �������.", constitution, ability_procs},
{Header, Intellegence, "��������� ������������� ������, ���������������� ��������� � ��� ����������� � ��������, ������� ������� ��� ����, ��� ����� �������� �������� ������. �� ���������� ������� ���������� �������, ������� �������� � ��������� �������. ��������� - ��� ������� ����������� ��� �����������, ������� ���������� �������� ����� ����, ����� �������� � ��������� ����������.", intellegence, ability_procs},
{Header, Wisdow, "�������� ������������� ������������ ��������������, ����������������, ���� ����, �������� ������ � �������� ���������.��� ����� ������ �� ������������ ��������� � ���������� ������������.", wisdow, ability_procs},
{Header, Charisma, "������� ������������� ����������� ��������� � ���������, ��� ������ ����������������� � ��������� ��������.��� �� �������� ���������� �����������������, ���� ���, ����������, ����� ������ ����.������� ����� ��� ���� ����������, � �������� ��� ���, ��� ���������� ����� ���� � ���������� �����������, �������� �������, ������� � ��������� ���������.", charisma, ability_procs},
{}};
static manual acrobatic[] = {{Element, "���� ���������", "��� **�����** �� �������� ��������� ���� ���� ������� �� 1%% �� ������ [+4%%] ������ � ������������ ������ ����������� ������� �� ��������, ���� � ��������� ���� ����� ���������� (������������� ������� �� �� �����)."},
{Element, "���� ������ �������", "���� �������� ��������� �� ������� �������, �������� ���� ����� ������. � ������ ������ ������� �� ����������� - �� ������ �� ������."},
{}};
static manual alertness[] = {{Element, "���� ���������� ������� ������", "��� ������ **��������** ������ �������� ����� ������. ���� � �������� ���� ������ ���� ��������� �����, ������� ������� ��� ������ ��������� ������, �� ���������� �������. ��� ���� �������� �������� ������� �����."},
{}};
static manual athletics[] = {{Element, "��������� �����������", "������ ������� ������� ���� �������� ��� ���������� ����������� **����**, **��������** ��� **������������** �� �������. ��� ��������� ����������� �������� ������ ������ � ������� ������ (8 - �������� �����������) x 7. ����� �������, ���� ������� �������� ����������� 6, ���� �������� �� ����� ����� ������ ��������+14%%. ��, ���� �������� ����������� ����� 10, ���� ����� ���� � ����� ������ ��������-14%%."},
{Element, "������ �����", "� ���������� ����� ����������� ����������� ��� �������� �����. ���� �������� �� � ����� ����� ������ �������� �� ������� � [--20%%]."},
{}};
static manual backstabing[] = {{Element, "���� �����", "������� ������� ����� �������� �������� +1%% � ����� �� ������ [++2%%]. �� ������ [++30%%] �� ������������ ������������ ���� � ��� ���� (������ �� x4 �� [+90%%])."},
{}};
static manual bargain[] = {{Element, "������", "������� ����� �������� �������� � ���������� ���� ������ �� [++40%%] �� [--40%%]."},
{}};
static manual concetration[] = {{Element, "�������������� ����", "��������, ������� ���� ����� ����������� �������� **����** �� 1 ������� �� ������ [+4%%] ������."},
{}};
static manual healing[] = {{Element, "�������������� �����", "��� ������� ������������� ������ �� ��������� ���������� ����� �� [1] � �� [3] + [1] �� ������ [+10%%] ������."},
{}};
static manual literacy[] = {{Element, "������ ������", "��� ��������� ������ �������� ���� ������ � � ������ ������ �� ��������� ������� �����. � ������ ������� ������ �� ����������, �� ����� �� �� ���������."},
{Element, "������ ������ ����", "��� ��������� �������� ���� ��� ����� ���������� �������� ���� ������ � � ������ ������ �� ��������� ����� ������, ����� �� ��� �� ���� � ������� �����. � ������ ������� ����� ��������� ����� ���������� � ����� �� �� ���������."},
{Element, "����������", "��� �������� ������ ���������, ���� ��� ��������� [+9] ��� ������ �� ������������� ��������� ���� �����."},
{}};
static manual weapon_focus[] = {{Element, "���� ���������", "���� �� ���������� � �������, �� ������� ���������������� ������ �����, �� ��������� +1%% � ����� ������� �� ����� �� ������ [+5%%] ������."},
{Element, "�������������� ����", "���� �� ���������� � �������, �� ������� ���������������� ������ �����, �� ��������� +1 � ������������� ����� �� ������ [+30%%] ������."},
{}};
static manual skills_manual[] = {{Header, Acrobatics, "�������� ������������ �� ���� ����� ���������� - �������, ��������, ������, �������, ����������� � �.�.", acrobatic, skill_procs},
{Header, Alertness, "������������ ��������� ���� ������ ������� � ������ ��, ��� ������ ������ �� ��������.", alertness, skill_procs},
{Header, Athletics, "���������� �������� ���������, ������ ������, ������� � �������� ������������ ������������.", athletics, skill_procs},
{Header, Backstabbing, "������� ������ ����� �������������� �������� ����� �� ������ ���� ����� �� �������, ��� ���� ����� ��� �����.", backstabing, skill_procs},
{Header, Bargaining, "����������� ��������� ����� ������. ������ �� ���� ������� ��� ������� ��������� � �� ����������� ���� ������.", bargain, skill_procs},
{Header, Concetration, "�����, ����������� �������������� �� ���������� ������������ ������.", concetration, skill_procs},
{Header, Healing, "�����, ��������� ������ ������, � ����� �������� ������� ��� ��������.", healing, skill_procs},
{Header, Literacy, "������ ������ � ������, � ����� �������� ����� �������� ������� ��������. ����� �������� ������ ������������ ��� ����� � ������, ��� ��� ��� ���� ������ ������ �� ������ �� �����.", literacy, skill_procs},
{Header, WeaponFocusAxes, "������� ����� ��������� ��������� �������� � �������� ���� �����.", weapon_focus, skill_procs},
{Header, WeaponFocusBlades, "������� ������� �������� ����� ����� �� ����� ������� ������.", weapon_focus, skill_procs},
{Header, WeaponFocusBows, "������� �������� �� ���� ����� ����� � ������.", weapon_focus, skill_procs},
{}};
static manual manual_refery[] = {{Header, "�����������", "������ �������� �������� ������ �������������: ����, ��������, ������������, ���������, �������� � �������. ��� ������ ����������� ������������� ���������� ����������� ���������, � ��� ������ - ��� ���������� ����������� � ������ ��������. ����������� ������������ ��������� ������� � ��� �������� ������ ����� �� �������� ����� ������������� �� **3** �� **18**.", ability_manual},
{Header, "������", "��� �������� �������� ������� ������� �������, ������� ���� �� �� ���� � �����. ����� ���������� � ���������� ����������� � ����� **����� �� �����** ������������� ��������. � ������ ���� �������� �������� ������ ���� ��������, ������� ����������� ��� [��������] � ������ � ����, � ����� ����� �������, ��������� ��������� ������� �� ������ [��������������]. ������� �������� ������� ����� **�����** ���� ������������ ������������ � �������� ������. �������� ������ ������� ���������� ������ ������� � ��� ��������� ��������. � ���������� � ����� **�����** ����� ����� ������������� � �������.", skills_manual},
{Header, "��������� ���������", "������ ������� ����� ����� ���� ���� ������������� ���������. ��������� ������ �� �������� � ����c���, � ����� �� ��, ����� �� ������� ����������.", item_states},
{}};
manual manual_main = {Header, "������", "��� �������, ������������ � ���� ������� � ���� ������������� �������.", manual_refery};