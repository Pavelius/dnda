#include "main.h"

static void msg(logs::driver& sc, gender_s gender, char* result, const char* result_maximum, const char* text_male, const char* text_female, const char* text_thing) {
	if(gender == Female)
		sc.prints(result, result_maximum, text_female);
	else if(gender==Male)
		sc.prints(result, result_maximum, text_male);
	else
		sc.prints(result, result_maximum, text_thing);
}

void logs::driver::parseidentifier(char* result, const char* result_max, const char* identifier) {
	if(strcmp(identifier, "�����") == 0)
		zcpy(result, name);
	else if(strcmp(identifier, "�����") == 0)
		zcpy(result, name_opponent);
	else if(strcmp(identifier, "�����") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "�����") == 0)
		grammar::of(result, name_opponent);
	else if(strcmp(identifier, "������") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "������") == 0)
		grammar::of(result, name_opponent);
	else if(strcmp(identifier, "���") == 0)
		msg(*this, gender, result, result_max, "��", identifier, "���");
	else if(strcmp(identifier, "���") == 0)
		msg(*this, gender_opponent, result, result_max, "��", "���", "���");
	else if(strcmp(identifier, "�") == 0)
		msg(*this, gender, result, result_max, "", identifier, "�");
	else if(strcmp(identifier, "��") == 0)
		msg(*this, gender, result, result_max, "", identifier, "��");
	else if(strcmp(identifier, "���") == 0)
		msg(*this, gender, result, result_max, "��", identifier, "���");
	else if(strcmp(identifier, "��") == 0)
		msg(*this, gender, result, result_max, "���", identifier, "���");
	else if(strcmp(identifier, "���") == 0)
		msg(*this, gender, result, result_max, "����", identifier, "����");
	else {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}