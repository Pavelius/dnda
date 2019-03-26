#pragma once

struct stringcreator {
	const char*			parseformat(char* result, const char* result_max, const char* format, const char* format_param);
	virtual void		parseidentifier(char* result, const char* result_max, const char* identifier);
	static char*		parseint(char* dst, const char* result_max, int value, int precision, const int radix);
	static char*		parsenumber(char* dst, const char* result_max, unsigned value, int precision, const int radix);
	virtual void		parsevariable(char* result, const char* result_max, const char** format);
	void				printv(char* result, const char* result_max, const char* format, const char* format_param);
	void				prints(char* result, const char* result_maximum, const char* format, ...);
};
struct stringbuffer : stringcreator {
	char*				result;
	const char*			result_maximum;
	constexpr stringbuffer(char* result, const char* result_maximum) : result(result), result_maximum(result_maximum) {}
	template<unsigned N> constexpr stringbuffer(char(&data)[N]) : result(data), result_maximum(data+N-1) {}
	explicit operator bool() const { return result != 0 && result[0]; }
	void				add(const char* format, ...);
	void				header(const char* header, const char* separator = ", ");
	void				clear() { result[0] = 0; }
	void				trail(const char* header);
};