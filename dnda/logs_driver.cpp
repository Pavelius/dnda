#include "main.h"

static void msg(gender_s gender, char* result, const char* text_male, const char* text_female, const char* text_pluar) {
	if(gender == Female) {
		if(text_female)
			zcpy(result, text_female);
	} else {
		if(text_male)
			zcpy(result, text_male);
	}
}

void logs::driver::parseidentifier(char* result, const char* result_max, const char* identifier) {
	if(strcmp(identifier, "герой") == 0)
		zcpy(result, name);
	else if(strcmp(identifier, "героя") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "героем") == 0)
		grammar::of(result, name);
	else if(strcmp(identifier, "оружием") == 0) {
		if(weapon)
			grammar::by(result, weapon);
		else
			zcpy(result, "руками");
	} else if(strcmp(identifier, "ась") == 0)
		msg(gender, result, "ся", identifier, "ись");
	else if(strcmp(identifier, "а") == 0)
		msg(gender, result, "", identifier, "и");
	else if(strcmp(identifier, "ла") == 0)
		msg(gender, result, "", identifier, "ли");
	else if(strcmp(identifier, "она") == 0)
		msg(gender, result, "он", identifier, "они");
	else if(strcmp(identifier, "ее") == 0)
		msg(gender, result, "его", identifier, "их");
	else if(strcmp(identifier, "нее") == 0)
		msg(gender, result, "него", identifier, "них");
	else {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}

logs::driver::driver(const char* name, gender_s gender, const char* weapon) : name(name), gender(gender), weapon(weapon) {
}