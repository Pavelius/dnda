#include "main.h"

const char* variant::getname() const {
	switch(type) {
	case Ability: return getstr(ability);
	case Skill: return getstr(skill);
	case State: return getstr(state);
	case Enchantment: return getstr(enchantment);
	case String: return text;
	case Item: return getstr(itemtype);
	case ItemObject: return getstr(itemobject.gettype());
	default: return "";
	}
}