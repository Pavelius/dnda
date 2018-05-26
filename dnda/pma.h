#pragma once

struct pma
{
	char				name[4]; // Identifier of current block
	int					size; // Size of all block
	int					count; // Count of records in this block
	int					find(const char* name) const;
	const pma*			getheader(const char* name) const;
	const char*			getstring(int id) const;
};