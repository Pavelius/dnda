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
	if(strcmp(identifier, "герой") == 0)
		zcpy(result, name);
	else if(strcmp(identifier, "√≈–ќ…") == 0)
		zcpy(result, name_opponent);
	else if(strcmp(identifier, "геро€") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "√≈–ќя") == 0)
		grammar::of(result, name_opponent);
	else if(strcmp(identifier, "героем") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "√≈–ќ≈ћ") == 0)
		grammar::of(result, name_opponent);
	else if(strcmp(identifier, "ась") == 0)
		msg(*this, gender, result, result_max, "с€", identifier, "ос€");
	else if(strcmp(identifier, "ј—№") == 0)
		msg(*this, gender_opponent, result, result_max, "с€", "ась", "ос€");
	else if(strcmp(identifier, "а") == 0)
		msg(*this, gender, result, result_max, "", identifier, "о");
	else if(strcmp(identifier, "ла") == 0)
		msg(*this, gender, result, result_max, "", identifier, "ло");
	else if(strcmp(identifier, "она") == 0)
		msg(*this, gender, result, result_max, "он", identifier, "оно");
	else if(strcmp(identifier, "ее") == 0)
		msg(*this, gender, result, result_max, "его", identifier, "его");
	else if(strcmp(identifier, "нее") == 0)
		msg(*this, gender, result, result_max, "него", identifier, "него");
	else {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}