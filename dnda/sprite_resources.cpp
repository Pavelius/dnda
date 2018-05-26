#include "resources.h"
#include "main.h"

res::element res::elements[] = {{""},
{"grass", "art"},
{"grass_w", "art"},
{"dungeon", "art"},
{"dungeon_w", "art"},
{"shadow", "art"},
{"road", "art"},
{"water", "art"},
{"monsters", "art"},
{"items", "art"},
{"doors", "art"},
{"fog", "art"},
{"features", "art"},
{"ui", "art"},
{"pcmar", "art"},
{"pcmbd", "art"},
{"pcmac", "art"},
};
static_assert(sizeof(res::elements) / sizeof(res::elements[0]) == ResPCmac + 1, "Invalid count of res::elements");