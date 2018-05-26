#include "crt.h"
#include "dice.h"

dice dice::create(int c, int d, int b, int m)
{
	return{(unsigned char)c, (unsigned char)d, (char)b, (char)m};
}

int dice::roll(int c, int d)
{
	if(!d)
		return c;
	int result = 0;
	for(int i = 0; i < c; i++)
		result += 1 + (rand() % d);
	return result;
}

int dice::roll() const
{
	if(!c)
		return b;
	else if(!m)
		return roll(c, d) + b;
	return roll(c, d)*m + b;
}

char* dice::print(char* result) const
{
	szprint(result, "%1id%2i", c, d);
	if(b!=0)
		szprint(zend(result), "%+1i", b);
	return result;
}

char* dice::range(char* result) const
{
	szprint(result, "%1i-%2i", c+b, c*d+b);
	return result;
}