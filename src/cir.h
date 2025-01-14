#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <termios.h>
#endif

struct CIR {
#if defined(_WIN32)
	DWORD attributes;
#else
	struct termios attributes;
#endif
	char tmp[16];
	int initialized;
};

enum CIKeyType {
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_ARROW_UP,
	KEY_ARROW_DOWN,
	KEY_ARROW_LEFT,
	KEY_ARROW_RIGHT,
	KEY_ENTER,
	KEY_HOME,
	KEY_END,
	KEY_DELETE,
	KEY_CTRL_C,
	KEY_CTRL_BACKSLASH,
	KEY_CTRL_D,
	KEY_ZERO,
	KEY_ONE,
	KEY_TWO,
	KEY_THREE,
	KEY_FOUR,
	KEY_FIVE,
	KEY_SIX,
	KEY_SEVEN,
	KEY_EIGHTH,
	KEY_NINE,
	KEY_HYPHEN,
	KEY_COMMA,
	KEY_EMPTY,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_EXCLAMATION_MARK,
	KEY_DOUBLE_QUOTATION_MARK,
	KEY_OCTOTHORPE,
	KEY_DOLLAR_SIGN,
	KEY_PERCENT_SIGN,
	KEY_AMPERSAND,
	KEY_APOSTROPHE,
	KEY_OPEN_ROUND_BRACKET,
	KEY_CLOSE_ROUND_BRACKET,
	KEY_ASTERISK,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_COLON,
	KEY_SEMICOLON,
	KEY_LESS_THAN,
	KEY_EQUAL_SIGN,
	KEY_GREATER_THAN,
	KEY_QUESTION_MARK,
	KEY_AT_SIGN,
	KEY_UNKNOWN
};

enum CIKeySubype {
	KEY_SUBTYPE_DIGIT,
	KEY_SUBTYPE_ASCII_LOWERCASE,
	KEY_SUBTYPE_ASCII_UPPERCASE,
	KEY_SUBTYPE_ARROW,
	KEY_SUBTYPE_PUNCTUATION,
	KEY_SUBTYPE_UNKNOWN
};

struct CIKey {
	enum CIKeyType type;
	enum CIKeySubype subtype;
	char* name;
#if defined(_WIN32)
	WORD code[5];
#else
	char code[5];
#endif
};

static const struct CIKey KEYBOARD_KEY_EMPTY = {
	KEY_EMPTY,
	KEY_SUBTYPE_UNKNOWN,
	"Empty",
	#if defined(_WIN32)
		{0x0}
	#else
		{0x0}
	#endif
};

static const struct CIKey KEYBOARD_KEY_UNKNOWN = {
	KEY_UNKNOWN,
	KEY_SUBTYPE_UNKNOWN,
	"Unknown",
	#if defined(_WIN32)
		{0x0}
	#else
		{0x0}
	#endif
};

static const struct CIKey keys[] = {
	{
		KEY_PAGE_UP,
		KEY_SUBTYPE_UNKNOWN,
		"Page Up",
	#if defined(_WIN32)
		{0x21}
	#else
		{0x1b, 0x5b, 0x35, 0x7e}
	#endif
	},
	{
		KEY_PAGE_DOWN,
		KEY_SUBTYPE_UNKNOWN,
		"Page Down",
	#if defined(_WIN32)
		{0x22}
	#else
		{0x1b, 0x5b, 0x36, 0x7e}
	#endif
	},
	{
		KEY_ARROW_UP,
		KEY_SUBTYPE_UNKNOWN,
		"Arrow Up",
	#if defined(_WIN32)
		{0x26}
	#else
		{0x1b, 0x5b, 0x41}
	#endif
	},
	{
		KEY_ARROW_DOWN,
		KEY_SUBTYPE_ARROW,
		"Arrow Down",
	#if defined(_WIN32)
		{0x28}
	#else
		{0x1b, 0x5b, 0x42}
	#endif
	},
	{
		KEY_ARROW_LEFT,
		KEY_SUBTYPE_ARROW,
		"Arrow Left",
	#if defined(_WIN32)
		{0x25}
	#else
		{0x1b, 0x5b, 0x44}
	#endif
	},
	{
		KEY_ARROW_RIGHT,
		KEY_SUBTYPE_ARROW,
		"Arrow Right",
	#if defined(_WIN32)
		{0x27}
	#else
		{0x1b, 0x5b, 0x43}
	#endif
	},
	{
		KEY_ENTER,
		KEY_SUBTYPE_UNKNOWN,
		"Enter",
	#if defined(_WIN32)
		{0xd}
	#else
		{0xd}
	#endif
	},
	{
		KEY_HOME,
		KEY_SUBTYPE_UNKNOWN,
		"Home",
	#if defined(_WIN32)
		{0x24}
	#else
		{0x1b, 0x5b, 0x48}
	#endif
	},
	{
		KEY_END,
		KEY_SUBTYPE_UNKNOWN,
		"End",
	#if defined(_WIN32)
		{0x23}
	#else
		{0x1b, 0x5b, 0x46}
	#endif
	},
	{
		KEY_DELETE,
		KEY_SUBTYPE_UNKNOWN,
		"Delete",
	#if defined(_WIN32)
		{0x8}
	#else
		{0x7f}
	#endif
	},
	{
		KEY_CTRL_C,
		KEY_SUBTYPE_UNKNOWN,
		"CTRL + C",
	#if defined(_WIN32)
		{0x999}
	#else
		{0x3}
	#endif
	},
	{
		KEY_CTRL_BACKSLASH,
		KEY_SUBTYPE_UNKNOWN,
		"CTRL + \\",
	#if defined(_WIN32)
		{0x999}
	#else
		{0x1c}
	#endif
	},
	{
		KEY_CTRL_D,
		KEY_SUBTYPE_UNKNOWN,
		"CTRL + D",
	#if defined(_WIN32)
		{0x999}
	#else
		{0x4}
	#endif
	},
	{
		KEY_ZERO,
		KEY_SUBTYPE_DIGIT,
		"0",
	#if defined(_WIN32)
		{0x30}
	#else
		{0x30}
	#endif
	},
	{
		KEY_ONE,
		KEY_SUBTYPE_DIGIT,
		"1",
	#if defined(_WIN32)
		{0x31}
	#else
		{0x31}
	#endif
	},
	{
		KEY_TWO,
		KEY_SUBTYPE_DIGIT,
		"2",
	#if defined(_WIN32)
		{0x32}
	#else
		{0x32}
	#endif
	},
	{
		KEY_THREE,
		KEY_SUBTYPE_DIGIT,
		"3",
	#if defined(_WIN32)
		{0x33}
	#else
		{0x33}
	#endif
	},
	{
		KEY_FOUR,
		KEY_SUBTYPE_DIGIT,
		"4",
	#if defined(_WIN32)
		{0x34}
	#else
		{0x34}
	#endif
	},
	{
		KEY_FIVE,
		KEY_SUBTYPE_DIGIT,
		"5",
	#if defined(_WIN32)
		{0x35}
	#else
		{0x35}
	#endif
	},
	{
		KEY_SIX,
		KEY_SUBTYPE_DIGIT,
		"6",
	#if defined(_WIN32)
		{0x36}
	#else
		{0x36}
	#endif
	},
	{
		KEY_SEVEN,
		KEY_SUBTYPE_DIGIT,
		"7",
	#if defined(_WIN32)
		{0x37}
	#else
		{0x37}
	#endif
	},
	{
		KEY_EIGHTH,
		KEY_SUBTYPE_DIGIT,
		"8",
	#if defined(_WIN32)
		{0x38}
	#else
		{0x38}
	#endif
	},
	{
		KEY_NINE,
		KEY_SUBTYPE_DIGIT,
		"9",
	#if defined(_WIN32)
		{0x39}
	#else
		{0x39}
	#endif
	},
	{
		KEY_HYPHEN,
		KEY_SUBTYPE_UNKNOWN,
		"-",
	#if defined(_WIN32)
		{0x2d}
	#else
		{0x2d}
	#endif
	},
	{
		KEY_COMMA,
		KEY_SUBTYPE_UNKNOWN,
		",",
	#if defined(_WIN32)
		{0x2c}
	#else
		{0x2c}
	#endif
	},
	{
		KEY_A,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"a",
	#if defined(_WIN32)
		{0x61}
	#else
		{0x61}
	#endif
	},
	{
		KEY_A,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"A",
	#if defined(_WIN32)
		{0x10, 0x41}
	#else
		{0x41}
	#endif
	},
	{
		KEY_B,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"b",
	#if defined(_WIN32)
		{0x62}
	#else
		{0x62}
	#endif
	},
	{
		KEY_B,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"B",
	#if defined(_WIN32)
		{0x10, 0x42}
	#else
		{0x42}
	#endif
	},
	{
		KEY_C,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"c",
	#if defined(_WIN32)
		{0x63}
	#else
		{0x63}
	#endif
	},
	{
		KEY_C,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"C",
	#if defined(_WIN32)
		{0x10, 0x43}
	#else
		{0x43}
	#endif
	},
	{
		KEY_D,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"d",
	#if defined(_WIN32)
		{0x64}
	#else
		{0x64}
	#endif
	},
	{
		KEY_D,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"D",
	#if defined(_WIN32)
		{0x10, 0x44}
	#else
		{0x44}
	#endif
	},
	{
		KEY_E,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"e",
	#if defined(_WIN32)
		{0x65}
	#else
		{0x65}
	#endif
	},
	{
		KEY_E,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"E",
	#if defined(_WIN32)
		{0x10, 0x45}
	#else
		{0x45}
	#endif
	},
	{
		KEY_F,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"f",
	#if defined(_WIN32)
		{0x66}
	#else
		{0x66}
	#endif
	},
	{
		KEY_F,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"F",
	#if defined(_WIN32)
		{0x10, 0x46}
	#else
		{0x46}
	#endif
	},
	{
		KEY_G,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"g",
	#if defined(_WIN32)
		{0x67}
	#else
		{0x67}
	#endif
	},
	{
		KEY_G,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"G",
	#if defined(_WIN32)
		{0x10, 0x47}
	#else
		{0x47}
	#endif
	},
	{
		KEY_H,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"h",
	#if defined(_WIN32)
		{0x68}
	#else
		{0x68}
	#endif
	},
	{
		KEY_H,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"H",
	#if defined(_WIN32)
		{0x10, 0x48}
	#else
		{0x48}
	#endif
	},
	{
		KEY_I,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"i",
	#if defined(_WIN32)
		{0x69}
	#else
		{0x69}
	#endif
	},
	{
		KEY_I,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"I",
	#if defined(_WIN32)
		{0x10, 0x49}
	#else
		{0x49}
	#endif
	},
	{
		KEY_J,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"j",
	#if defined(_WIN32)
		{0x6A}
	#else
		{0x6A}
	#endif
	},
	{
		KEY_J,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"J",
	#if defined(_WIN32)
		{0x10, 0x4A}
	#else
		{0x4A}
	#endif
	},
	{
		KEY_K,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"k",
	#if defined(_WIN32)
		{0x6B}
	#else
		{0x6B}
	#endif
	},
	{
		KEY_K,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"K",
	#if defined(_WIN32)
		{0x10, 0x4B}
	#else
		{0x4B}
	#endif
	},
	{
		KEY_L,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"l",
	#if defined(_WIN32)
		{0x6C}
	#else
		{0x6C}
	#endif
	},
	{
		KEY_L,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"L",
	#if defined(_WIN32)
		{0x10, 0x4C}
	#else
		{0x4C}
	#endif
	},
	{
		KEY_M,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"m",
	#if defined(_WIN32)
		{0x6D}
	#else
		{0x6D}
	#endif
	},
	{
		KEY_M,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"M",
	#if defined(_WIN32)
		{0x10, 0x4D}
	#else
		{0x4D}
	#endif
	},
	{
		KEY_N,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"n",
	#if defined(_WIN32)
		{0x6E}
	#else
		{0x6E}
	#endif
	},
	{
		KEY_N,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"N",
	#if defined(_WIN32)
		{0x10, 0x4E}
	#else
		{0x4E}
	#endif
	},
	{
		KEY_O,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"o",
	#if defined(_WIN32)
		{0x6F}
	#else
		{0x6F}
	#endif
	},
	{
		KEY_O,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"O",
	#if defined(_WIN32)
		{0x10, 0x4F}
	#else
		{0x4F}
	#endif
	},
	{
		KEY_P,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"p",
	#if defined(_WIN32)
		{0x70}
	#else
		{0x70}
	#endif
	},
	{
		KEY_P,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"P",
	#if defined(_WIN32)
		{0x10, 0x50}
	#else
		{0x50}
	#endif
	},
	{
		KEY_Q,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"q",
	#if defined(_WIN32)
		{0x71}
	#else
		{0x71}
	#endif
	},
	{
		KEY_Q,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"Q",
	#if defined(_WIN32)
		{0x10, 0x51}
	#else
		{0x51}
	#endif
	},
	{
		KEY_R,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"r",
	#if defined(_WIN32)
		{0x72}
	#else
		{0x72}
	#endif
	},
	{
		KEY_R,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"R",
	#if defined(_WIN32)
		{0x10, 0x52}
	#else
		{0x52}
	#endif
	},
	{
		KEY_S,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"s",
	#if defined(_WIN32)
		{0x73}
	#else
		{0x73}
	#endif
	},
	{
		KEY_S,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"S",
	#if defined(_WIN32)
		{0x10, 0x53}
	#else
		{0x53}
	#endif
	},
	{
		KEY_T,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"t",
	#if defined(_WIN32)
		{0x74}
	#else
		{0x74}
	#endif
	},
	{
		KEY_T,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"T",
	#if defined(_WIN32)
		{0x10, 0x54}
	#else
		{0x54}
	#endif
	},
	{
		KEY_U,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"u",
	#if defined(_WIN32)
		{0x75}
	#else
		{0x75}
	#endif
	},
	{
		KEY_U,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"U",
	#if defined(_WIN32)
		{0x10, 0x55}
	#else
		{0x55}
	#endif
	},
	{
		KEY_V,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"v",
	#if defined(_WIN32)
		{0x76}
	#else
		{0x76}
	#endif
	},
	{
		KEY_V,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"V",
	#if defined(_WIN32)
		{0x10, 0x56}
	#else
		{0x56}
	#endif
	},
	{
		KEY_W,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"w",
	#if defined(_WIN32)
		{0x77}
	#else
		{0x77}
	#endif
	},
	{
		KEY_W,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"W",
	#if defined(_WIN32)
		{0x10, 0x57}
	#else
		{0x57}
	#endif
	},
	{
		KEY_X,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"x",
	#if defined(_WIN32)
		{0x78}
	#else
		{0x78}
	#endif
	},
	{
		KEY_X,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"X",
	#if defined(_WIN32)
		{0x10, 0x58}
	#else
		{0x58}
	#endif
	},
	{
		KEY_Y,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"y",
	#if defined(_WIN32)
		{0x79}
	#else
		{0x79}
	#endif
	},
	{
		KEY_Y,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"Y",
	#if defined(_WIN32)
		{0x10, 0x59}
	#else
		{0x59}
	#endif
	},
	{
		KEY_Z,
		KEY_SUBTYPE_ASCII_LOWERCASE,
		"z",
	#if defined(_WIN32)
		{0x7A}
	#else
		{0x7A}
	#endif
	},
	{
		KEY_Z,
		KEY_SUBTYPE_ASCII_UPPERCASE,
		"Z",
	#if defined(_WIN32)
		{0x10, 0x5A}
	#else
		{0x5A}
	#endif
	},
	{
		KEY_EXCLAMATION_MARK,
		KEY_SUBTYPE_PUNCTUATION,
		"!",
	#if defined(_WIN32)
		{0x10, 0x21}
	#else
		{0x5A}
	#endif
	},
	{
		KEY_DOUBLE_QUOTATION_MARK,
		KEY_SUBTYPE_PUNCTUATION,
		"\"",
	#if defined(_WIN32)
		{0x10, 0x21}
	#else
		{0x5A}
	#endif
	},
	{
		KEY_OCTOTHORPE,
		KEY_SUBTYPE_PUNCTUATION,
		"#",
	#if defined(_WIN32)
		{0x10, 0x23}
	#else
		{0x23}
	#endif
	},
	{
		KEY_DOLLAR_SIGN,
		KEY_SUBTYPE_PUNCTUATION,
		"$",
	#if defined(_WIN32)
		{0x10, 0x24}
	#else
		{0x24}
	#endif
	},
	{
		KEY_PERCENT_SIGN,
		KEY_SUBTYPE_PUNCTUATION,
		"%",
	#if defined(_WIN32)
		{0x10, 0x25}
	#else
		{0x25}
	#endif
	},
	{
		KEY_AMPERSAND,
		KEY_SUBTYPE_PUNCTUATION,
		"&",
	#if defined(_WIN32)
		{0x10, 0x26}
	#else
		{0x26}
	#endif
	},
	{
		KEY_APOSTROPHE,
		KEY_SUBTYPE_PUNCTUATION,
		"'",
	#if defined(_WIN32)
		{0x27}
	#else
		{0x27}
	#endif
	},
	{
		KEY_OPEN_ROUND_BRACKET,
		KEY_SUBTYPE_PUNCTUATION,
		"(",
	#if defined(_WIN32)
		{0x10, 0x28}
	#else
		{0x28}
	#endif
	},
	{
		KEY_CLOSE_ROUND_BRACKET,
		KEY_SUBTYPE_PUNCTUATION,
		")",
	#if defined(_WIN32)
		{0x10, 0x29}
	#else
		{0x29}
	#endif
	},
	{
		KEY_ASTERISK,
		KEY_SUBTYPE_PUNCTUATION,
		"*",
	#if defined(_WIN32)
		{0x2A}
	#else
		{0x2A}
	#endif
	},
	{
		KEY_COMMA,
		KEY_SUBTYPE_PUNCTUATION,
		",",
	#if defined(_WIN32)
		{0x10, 0x2C}
	#else
		{0x2C}
	#endif
	},
	{
		KEY_HYPHEN,
		KEY_SUBTYPE_PUNCTUATION,
		"-",
	#if defined(_WIN32)
		{0x2D}
	#else
		{0x2D}
	#endif
	},
	{
		KEY_PERIOD,
		KEY_SUBTYPE_PUNCTUATION,
		".",
	#if defined(_WIN32)
		{0x2E}
	#else
		{0x2E}
	#endif
	},
	{
		KEY_SLASH,
		KEY_SUBTYPE_PUNCTUATION,
		"/",
	#if defined(_WIN32)
		{0x2F}
	#else
		{0x2F}
	#endif
	},
	{
		KEY_COLON,
		KEY_SUBTYPE_PUNCTUATION,
		":",
	#if defined(_WIN32)
		{0x10, 0x3A}
	#else
		{0x3A}
	#endif
	},
	{
		KEY_SEMICOLON,
		KEY_SUBTYPE_PUNCTUATION,
		";",
	#if defined(_WIN32)
		{0x3B}
	#else
		{0x3B}
	#endif
	},
	{
		KEY_LESS_THAN,
		KEY_SUBTYPE_PUNCTUATION,
		"<",
	#if defined(_WIN32)
		{0x10, 0x3C}
	#else
		{0x3C}
	#endif
	},
	{
		KEY_EQUAL_SIGN,
		KEY_SUBTYPE_PUNCTUATION,
		"=",
	#if defined(_WIN32)
		{0x3D}
	#else
		{0x3D}
	#endif
	},
	{
		KEY_GREATER_THAN,
		KEY_SUBTYPE_PUNCTUATION,
		">",
	#if defined(_WIN32)
		{0x10, 0x3E}
	#else
		{0x3E}
	#endif
	},
	{
		KEY_QUESTION_MARK,
		KEY_SUBTYPE_PUNCTUATION,
		"?",
	#if defined(_WIN32)
		{0x10, 0x3F}
	#else
		{0x3F}
	#endif
	},
	{
		KEY_AT_SIGN,
		KEY_SUBTYPE_PUNCTUATION,
		"@",
	#if defined(_WIN32)
		{0x10, 0x40}
	#else
		{0x40}
	#endif
	}
};

int cir_init(struct CIR* const reader);
const struct CIKey* cir_get(struct CIR* const reader);
int cir_free(struct CIR* const reader);

#pragma once
