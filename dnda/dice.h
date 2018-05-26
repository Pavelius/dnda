#pragma once

struct dice
{
	unsigned char		c;
	unsigned char		d;
	char				b;
	char				m;
	//
	static dice			create(int c, int d, int b = 0, int m = 0);
	void				clear() { c=0; d=0; m=0; b=0; }
	char*				print(char* result) const;
	char*				range(char* result) const;
	int					roll() const;
	static int			roll(int c, int d);
};