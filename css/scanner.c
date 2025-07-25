/* Generated by re2c 3.0 on Sat Jul 12 15:06:08 2025 */
#line 1 "scanner.l"
/*
 * This file is part of NetSurf, http://netsurf-browser.org/
 * Licensed under the GNU General Public License,
 *                http://www.opensource.org/licenses/gpl-license
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 */

/** \file
 * CSS tokeniser using re2c.
 *
 * see CSS2 Specification, chapter 4
 * http://www.w3.org/TR/REC-CSS2/syndata.html,
 * and errata
 * http://www.w3.org/Style/css2-updates/REC-CSS2-19980512-errata
 */

#include <stdbool.h>
#define CSS_INTERNALS
#include "css/css.h"
#include "css/parser.h"

#define YYCTYPE unsigned char
#define YYCURSOR (*buffer)
#define YYLIMIT end
#define YYMARKER marker
#define YYFILL(n) { return 0; }


/**
 * Identify a CSS source token.
 *
 * \param  buffer      source to tokenise, updated to new position
 * \param  end         end of source
 * \param  token_text  updated to start of recognized token
 * \return  token number
 */

int css_tokenise(unsigned char **buffer, unsigned char *end,
		unsigned char **token_text)
{
	unsigned char *marker;

start:
	*token_text = YYCURSOR;


#line 50 "<stdout>"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 5) YYFILL(5);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case '\v':
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
		case '!':
		case '%':
		case '&':
		case '?':
		case '`':
		case 0x7F: goto yy1;
		case '\t':
		case '\n':
		case '\f':
		case '\r':
		case ' ': goto yy3;
		case '"': goto yy5;
		case '#': goto yy6;
		case '$': goto yy7;
		case '\'': goto yy8;
		case '(': goto yy9;
		case ')': goto yy10;
		case '*': goto yy11;
		case '+': goto yy12;
		case ',': goto yy14;
		case '-': goto yy15;
		case '.': goto yy16;
		case '/': goto yy18;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy19;
		case ':': goto yy21;
		case ';': goto yy22;
		case '<': goto yy23;
		case '=': goto yy24;
		case '>': goto yy25;
		case '@': goto yy26;
		case 'U': goto yy30;
		case '[': goto yy31;
		case '\\': goto yy32;
		case ']': goto yy33;
		case '^': goto yy34;
		case 'u': goto yy35;
		case '{': goto yy36;
		case '|': goto yy37;
		case '}': goto yy38;
		case '~': goto yy39;
		default: goto yy27;
	}
yy1:
	++YYCURSOR;
yy2:
#line 109 "scanner.l"
	{ return DELIM; }
#line 141 "<stdout>"
yy3:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\v') {
		if (yych <= 0x08) goto yy4;
		if (yych <= '\n') goto yy3;
	} else {
		if (yych <= '\r') goto yy3;
		if (yych == ' ') goto yy3;
	}
yy4:
#line 88 "scanner.l"
	{ return S; }
#line 156 "<stdout>"
yy5:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x1F) {
		if (yych == '\t') goto yy41;
		goto yy2;
	} else {
		if (yych == 0x7F) goto yy2;
		goto yy41;
	}
yy6:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= '[') {
		if (yych <= '/') {
			if (yych == '-') goto yy46;
			goto yy2;
		} else {
			if (yych <= '9') goto yy46;
			if (yych <= '@') goto yy2;
			if (yych <= 'Z') goto yy46;
			goto yy2;
		}
	} else {
		if (yych <= '_') {
			if (yych <= '\\') goto yy48;
			if (yych <= '^') goto yy2;
			goto yy46;
		} else {
			if (yych <= '`') goto yy2;
			if (yych <= 'z') goto yy46;
			if (yych <= 0x7F) goto yy2;
			goto yy46;
		}
	}
yy7:
	yych = *++YYCURSOR;
	if (yych == '=') goto yy49;
	goto yy2;
yy8:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x1F) {
		if (yych == '\t') goto yy51;
		goto yy2;
	} else {
		if (yych == 0x7F) goto yy2;
		goto yy51;
	}
yy9:
	++YYCURSOR;
#line 83 "scanner.l"
	{ return LPAREN; }
#line 210 "<stdout>"
yy10:
	++YYCURSOR;
#line 84 "scanner.l"
	{ return RPAREN; }
#line 215 "<stdout>"
yy11:
	yych = *++YYCURSOR;
	if (yych == '=') goto yy53;
#line 107 "scanner.l"
	{ return ASTERISK; }
#line 221 "<stdout>"
yy12:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '.') goto yy54;
	if (yych <= '/') goto yy13;
	if (yych <= '9') goto yy19;
yy13:
#line 104 "scanner.l"
	{ return PLUS; }
#line 231 "<stdout>"
yy14:
	++YYCURSOR;
#line 103 "scanner.l"
	{ return COMMA; }
#line 236 "<stdout>"
yy15:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'Z') {
		if (yych <= '.') {
			if (yych <= ',') goto yy2;
			if (yych <= '-') goto yy55;
			goto yy54;
		} else {
			if (yych <= '/') goto yy2;
			if (yych <= '9') goto yy19;
			if (yych <= '@') goto yy2;
			goto yy27;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy56;
			if (yych <= '^') goto yy2;
			goto yy27;
		} else {
			if (yych <= '`') goto yy2;
			if (yych <= 'z') goto yy27;
			if (yych <= 0x7F) goto yy2;
			goto yy27;
		}
	}
yy16:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy17;
	if (yych <= '9') goto yy57;
yy17:
#line 106 "scanner.l"
	{ return DOT; }
#line 270 "<stdout>"
yy18:
	yych = *++YYCURSOR;
	if (yych == '*') goto yy58;
	goto yy2;
yy19:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '-') {
			if (yych == '%') goto yy59;
			if (yych >= '-') goto yy60;
		} else {
			if (yych <= '.') goto yy54;
			if (yych <= '/') goto yy20;
			if (yych <= '9') goto yy19;
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') goto yy61;
			if (yych == '\\') goto yy63;
		} else {
			if (yych <= '`') {
				if (yych <= '_') goto yy61;
			} else {
				if (yych <= 'z') goto yy61;
				if (yych >= 0x80) goto yy61;
			}
		}
	}
yy20:
#line 68 "scanner.l"
	{ return NUMBER; }
#line 305 "<stdout>"
yy21:
	++YYCURSOR;
#line 102 "scanner.l"
	{ return COLON; }
#line 310 "<stdout>"
yy22:
	++YYCURSOR;
#line 80 "scanner.l"
	{ return SEMI; }
#line 315 "<stdout>"
yy23:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '!') goto yy64;
	goto yy2;
yy24:
	++YYCURSOR;
#line 101 "scanner.l"
	{ return EQUALS; }
#line 325 "<stdout>"
yy25:
	++YYCURSOR;
#line 105 "scanner.l"
	{ return GT; }
#line 330 "<stdout>"
yy26:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= '\\') {
		if (yych <= '@') {
			if (yych == '-') goto yy65;
			goto yy2;
		} else {
			if (yych <= 'Z') goto yy66;
			if (yych <= '[') goto yy2;
			goto yy68;
		}
	} else {
		if (yych <= '`') {
			if (yych == '_') goto yy66;
			goto yy2;
		} else {
			if (yych <= 'z') goto yy66;
			if (yych <= 0x7F) goto yy2;
			goto yy66;
		}
	}
yy27:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy28:
	if (yych <= 'Z') {
		if (yych <= '-') {
			if (yych == '(') goto yy69;
			if (yych >= '-') goto yy27;
		} else {
			if (yych <= '/') goto yy29;
			if (yych <= '9') goto yy27;
			if (yych >= 'A') goto yy27;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy56;
			if (yych >= '_') goto yy27;
		} else {
			if (yych <= '`') goto yy29;
			if (yych <= 'z') goto yy27;
			if (yych >= 0x80) goto yy27;
		}
	}
yy29:
#line 63 "scanner.l"
	{ return IDENT; }
#line 381 "<stdout>"
yy30:
	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '+') goto yy71;
	goto yy28;
yy31:
	++YYCURSOR;
#line 85 "scanner.l"
	{ return LBRAC; }
#line 391 "<stdout>"
yy32:
	yych = *++YYCURSOR;
	if (yych <= '\r') {
		if (yych == '\n') goto yy2;
		if (yych <= '\v') goto yy27;
		goto yy2;
	} else {
		if (yych <= '9') {
			if (yych <= '/') goto yy27;
			goto yy72;
		} else {
			if (yych <= '`') goto yy27;
			if (yych <= 'f') goto yy72;
			goto yy27;
		}
	}
yy33:
	++YYCURSOR;
#line 86 "scanner.l"
	{ return RBRAC; }
#line 412 "<stdout>"
yy34:
	yych = *++YYCURSOR;
	if (yych == '=') goto yy73;
	goto yy2;
yy35:
	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == 'r') goto yy74;
	goto yy28;
yy36:
	++YYCURSOR;
#line 81 "scanner.l"
	{ return LBRACE; }
#line 426 "<stdout>"
yy37:
	yych = *++YYCURSOR;
	if (yych == '=') goto yy75;
	goto yy2;
yy38:
	++YYCURSOR;
#line 82 "scanner.l"
	{ return RBRACE; }
#line 435 "<stdout>"
yy39:
	yych = *++YYCURSOR;
	if (yych == '=') goto yy76;
	goto yy2;
yy40:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy41:
	if (yych <= '"') {
		if (yych <= '\t') {
			if (yych >= '\t') goto yy40;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy40;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy40;
			goto yy45;
		} else {
			if (yych != 0x7F) goto yy40;
		}
	}
yy42:
	YYCURSOR = YYMARKER;
	if (yyaccept <= 5) {
		if (yyaccept <= 2) {
			if (yyaccept <= 1) {
				if (yyaccept == 0) {
					goto yy2;
				} else {
					goto yy13;
				}
			} else {
				goto yy20;
			}
		} else {
			if (yyaccept <= 4) {
				if (yyaccept == 3) {
					goto yy29;
				} else {
					goto yy47;
				}
			} else {
				goto yy62;
			}
		}
	} else {
		if (yyaccept <= 8) {
			if (yyaccept <= 7) {
				if (yyaccept == 6) {
					goto yy67;
				} else {
					goto yy44;
				}
			} else {
				goto yy90;
			}
		} else {
			if (yyaccept == 9) {
				goto yy70;
			} else {
				goto yy107;
			}
		}
	}
yy43:
	++YYCURSOR;
yy44:
#line 65 "scanner.l"
	{ return STRING; }
#line 509 "<stdout>"
yy45:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\r') {
			if (yych <= '\f') goto yy40;
			goto yy77;
		} else {
			if (yych == '"') goto yy78;
			goto yy40;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '9') goto yy79;
			if (yych <= '[') goto yy40;
			goto yy45;
		} else {
			if (yych <= '`') goto yy40;
			if (yych <= 'f') goto yy79;
			goto yy40;
		}
	}
yy46:
	yyaccept = 4;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '[') {
		if (yych <= '/') {
			if (yych == '-') goto yy46;
		} else {
			if (yych <= '9') goto yy46;
			if (yych <= '@') goto yy47;
			if (yych <= 'Z') goto yy46;
		}
	} else {
		if (yych <= '_') {
			if (yych <= '\\') goto yy48;
			if (yych >= '_') goto yy46;
		} else {
			if (yych <= '`') goto yy47;
			if (yych <= 'z') goto yy46;
			if (yych >= 0x80) goto yy46;
		}
	}
yy47:
#line 66 "scanner.l"
	{ return HASH; }
#line 559 "<stdout>"
yy48:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\r') {
		if (yych == '\n') goto yy42;
		if (yych <= '\v') goto yy46;
		goto yy42;
	} else {
		if (yych <= '9') {
			if (yych <= '/') goto yy46;
			goto yy80;
		} else {
			if (yych <= '`') goto yy46;
			if (yych <= 'f') goto yy80;
			goto yy46;
		}
	}
yy49:
	++YYCURSOR;
#line 98 "scanner.l"
	{ return SUFFIX; }
#line 582 "<stdout>"
yy50:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy51:
	if (yych <= '\'') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy42;
			goto yy50;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy50;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy50;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy50;
		}
	}
yy52:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\r') {
			if (yych <= '\f') goto yy50;
			goto yy81;
		} else {
			if (yych == '\'') goto yy82;
			goto yy50;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '9') goto yy83;
			if (yych <= '[') goto yy50;
			goto yy52;
		} else {
			if (yych <= '`') goto yy50;
			if (yych <= 'f') goto yy83;
			goto yy50;
		}
	}
yy53:
	++YYCURSOR;
#line 99 "scanner.l"
	{ return SUBSTR; }
#line 632 "<stdout>"
yy54:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy42;
	if (yych <= '9') goto yy57;
	goto yy42;
yy55:
	yych = *++YYCURSOR;
	if (yych == '>') goto yy84;
	goto yy42;
yy56:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\r') {
		if (yych == '\n') goto yy42;
		if (yych <= '\v') goto yy27;
		goto yy42;
	} else {
		if (yych <= '9') {
			if (yych <= '/') goto yy27;
			goto yy72;
		} else {
			if (yych <= '`') goto yy27;
			if (yych <= 'f') goto yy72;
			goto yy27;
		}
	}
yy57:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= 'Z') {
		if (yych <= '-') {
			if (yych == '%') goto yy59;
			if (yych <= ',') goto yy20;
			goto yy60;
		} else {
			if (yych <= '/') goto yy20;
			if (yych <= '9') goto yy57;
			if (yych <= '@') goto yy20;
			goto yy61;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy63;
			if (yych <= '^') goto yy20;
			goto yy61;
		} else {
			if (yych <= '`') goto yy20;
			if (yych <= 'z') goto yy61;
			if (yych <= 0x7F) goto yy20;
			goto yy61;
		}
	}
yy58:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych == '*') goto yy85;
	goto yy58;
yy59:
	++YYCURSOR;
#line 69 "scanner.l"
	{ return PERCENTAGE; }
#line 698 "<stdout>"
yy60:
	yych = *++YYCURSOR;
	if (yych <= '^') {
		if (yych <= 'Z') {
			if (yych <= '@') goto yy42;
		} else {
			if (yych == '\\') goto yy63;
			goto yy42;
		}
	} else {
		if (yych <= '`') {
			if (yych >= '`') goto yy42;
		} else {
			if (yych <= 'z') goto yy61;
			if (yych <= 0x7F) goto yy42;
		}
	}
yy61:
	yyaccept = 5;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '[') {
		if (yych <= '/') {
			if (yych == '-') goto yy61;
		} else {
			if (yych <= '9') goto yy61;
			if (yych <= '@') goto yy62;
			if (yych <= 'Z') goto yy61;
		}
	} else {
		if (yych <= '_') {
			if (yych <= '\\') goto yy63;
			if (yych >= '_') goto yy61;
		} else {
			if (yych <= '`') goto yy62;
			if (yych <= 'z') goto yy61;
			if (yych >= 0x80) goto yy61;
		}
	}
yy62:
#line 70 "scanner.l"
	{ return DIMENSION; }
#line 742 "<stdout>"
yy63:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\r') {
		if (yych == '\n') goto yy42;
		if (yych <= '\v') goto yy61;
		goto yy42;
	} else {
		if (yych <= '9') {
			if (yych <= '/') goto yy61;
			goto yy86;
		} else {
			if (yych <= '`') goto yy61;
			if (yych <= 'f') goto yy86;
			goto yy61;
		}
	}
yy64:
	yych = *++YYCURSOR;
	if (yych == '-') goto yy87;
	goto yy42;
yy65:
	yych = *++YYCURSOR;
	if (yych <= '^') {
		if (yych <= 'Z') {
			if (yych <= '@') goto yy42;
		} else {
			if (yych == '\\') goto yy68;
			goto yy42;
		}
	} else {
		if (yych <= '`') {
			if (yych >= '`') goto yy42;
		} else {
			if (yych <= 'z') goto yy66;
			if (yych <= 0x7F) goto yy42;
		}
	}
yy66:
	yyaccept = 6;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '[') {
		if (yych <= '/') {
			if (yych == '-') goto yy66;
		} else {
			if (yych <= '9') goto yy66;
			if (yych <= '@') goto yy67;
			if (yych <= 'Z') goto yy66;
		}
	} else {
		if (yych <= '_') {
			if (yych <= '\\') goto yy68;
			if (yych >= '_') goto yy66;
		} else {
			if (yych <= '`') goto yy67;
			if (yych <= 'z') goto yy66;
			if (yych >= 0x80) goto yy66;
		}
	}
yy67:
#line 64 "scanner.l"
	{ return ATKEYWORD; }
#line 808 "<stdout>"
yy68:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\r') {
		if (yych == '\n') goto yy42;
		if (yych <= '\v') goto yy66;
		goto yy42;
	} else {
		if (yych <= '9') {
			if (yych <= '/') goto yy66;
			goto yy88;
		} else {
			if (yych <= '`') goto yy66;
			if (yych <= 'f') goto yy88;
			goto yy66;
		}
	}
yy69:
	++YYCURSOR;
yy70:
#line 93 "scanner.l"
	{ return FUNCTION; }
#line 832 "<stdout>"
yy71:
	yych = *++YYCURSOR;
	if (yych <= '>') {
		if (yych <= '/') goto yy42;
		if (yych <= '9') goto yy89;
		goto yy42;
	} else {
		if (yych == '@') goto yy42;
		if (yych <= 'F') goto yy89;
		goto yy42;
	}
yy72:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= 0x1F) {
			if (yych <= '\v') {
				if (yych <= 0x08) goto yy29;
				if (yych <= '\n') goto yy27;
				goto yy29;
			} else {
				if (yych <= '\f') goto yy27;
				if (yych <= '\r') goto yy91;
				goto yy29;
			}
		} else {
			if (yych <= '(') {
				if (yych <= ' ') goto yy27;
				if (yych <= '\'') goto yy29;
				goto yy69;
			} else {
				if (yych == '-') goto yy27;
				goto yy29;
			}
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') {
				if (yych <= '9') goto yy72;
				if (yych <= '@') goto yy29;
				goto yy27;
			} else {
				if (yych == '\\') goto yy56;
				goto yy29;
			}
		} else {
			if (yych <= 'f') {
				if (yych <= '_') goto yy27;
				if (yych <= '`') goto yy29;
				goto yy72;
			} else {
				if (yych <= 'z') goto yy27;
				if (yych <= 0x7F) goto yy29;
				goto yy27;
			}
		}
	}
yy73:
	++YYCURSOR;
#line 97 "scanner.l"
	{ return PREFIX; }
#line 896 "<stdout>"
yy74:
	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == 'l') goto yy92;
	goto yy28;
yy75:
	++YYCURSOR;
#line 96 "scanner.l"
	{ return DASHMATCH; }
#line 906 "<stdout>"
yy76:
	++YYCURSOR;
#line 95 "scanner.l"
	{ return INCLUDES; }
#line 911 "<stdout>"
yy77:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\n') {
			if (yych <= 0x08) goto yy42;
			goto yy40;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy40;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy40;
			goto yy45;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy40;
		}
	}
yy78:
	yyaccept = 7;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy44;
			goto yy40;
		} else {
			if (yych <= 0x1F) goto yy44;
			if (yych <= '!') goto yy40;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy40;
			goto yy45;
		} else {
			if (yych == 0x7F) goto yy44;
			goto yy40;
		}
	}
yy79:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\f') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy40;
		} else {
			if (yych <= '\r') goto yy77;
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy40;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '/') goto yy40;
			if (yych <= '9') goto yy79;
			if (yych <= '[') goto yy40;
			goto yy45;
		} else {
			if (yych <= 'f') {
				if (yych <= '`') goto yy40;
				goto yy79;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy40;
			}
		}
	}
yy80:
	yyaccept = 4;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych <= '\r') {
			if (yych <= '\n') {
				if (yych <= 0x08) goto yy47;
				goto yy46;
			} else {
				if (yych <= '\v') goto yy47;
				if (yych <= '\f') goto yy46;
				goto yy93;
			}
		} else {
			if (yych <= ',') {
				if (yych == ' ') goto yy46;
				goto yy47;
			} else {
				if (yych <= '-') goto yy46;
				if (yych <= '/') goto yy47;
				goto yy80;
			}
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') {
				if (yych <= '@') goto yy47;
				goto yy46;
			} else {
				if (yych == '\\') goto yy48;
				goto yy47;
			}
		} else {
			if (yych <= 'f') {
				if (yych <= '_') goto yy46;
				if (yych <= '`') goto yy47;
				goto yy80;
			} else {
				if (yych <= 'z') goto yy46;
				if (yych <= 0x7F) goto yy47;
				goto yy46;
			}
		}
	}
yy81:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\n') {
			if (yych <= 0x08) goto yy42;
			goto yy50;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy50;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy50;
			goto yy52;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy50;
		}
	}
yy82:
	yyaccept = 7;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy44;
			goto yy50;
		} else {
			if (yych <= 0x1F) goto yy44;
			if (yych <= '&') goto yy50;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy50;
			goto yy52;
		} else {
			if (yych == 0x7F) goto yy44;
			goto yy50;
		}
	}
yy83:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\f') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy50;
		} else {
			if (yych <= '\r') goto yy81;
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy50;
			goto yy43;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '/') goto yy50;
			if (yych <= '9') goto yy83;
			if (yych <= '[') goto yy50;
			goto yy52;
		} else {
			if (yych <= 'f') {
				if (yych <= '`') goto yy50;
				goto yy83;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy50;
			}
		}
	}
yy84:
	++YYCURSOR;
#line 78 "scanner.l"
	{ goto start; /* ignore CDC */ }
#line 1114 "<stdout>"
yy85:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych == '*') goto yy85;
	if (yych == '/') goto yy94;
	goto yy58;
yy86:
	yyaccept = 5;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych <= '\r') {
			if (yych <= '\n') {
				if (yych <= 0x08) goto yy62;
				goto yy61;
			} else {
				if (yych <= '\v') goto yy62;
				if (yych <= '\f') goto yy61;
				goto yy95;
			}
		} else {
			if (yych <= ',') {
				if (yych == ' ') goto yy61;
				goto yy62;
			} else {
				if (yych <= '-') goto yy61;
				if (yych <= '/') goto yy62;
				goto yy86;
			}
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') {
				if (yych <= '@') goto yy62;
				goto yy61;
			} else {
				if (yych == '\\') goto yy63;
				goto yy62;
			}
		} else {
			if (yych <= 'f') {
				if (yych <= '_') goto yy61;
				if (yych <= '`') goto yy62;
				goto yy86;
			} else {
				if (yych <= 'z') goto yy61;
				if (yych <= 0x7F) goto yy62;
				goto yy61;
			}
		}
	}
yy87:
	yych = *++YYCURSOR;
	if (yych == '-') goto yy96;
	goto yy42;
yy88:
	yyaccept = 6;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych <= '\r') {
			if (yych <= '\n') {
				if (yych <= 0x08) goto yy67;
				goto yy66;
			} else {
				if (yych <= '\v') goto yy67;
				if (yych <= '\f') goto yy66;
				goto yy97;
			}
		} else {
			if (yych <= ',') {
				if (yych == ' ') goto yy66;
				goto yy67;
			} else {
				if (yych <= '-') goto yy66;
				if (yych <= '/') goto yy67;
				goto yy88;
			}
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') {
				if (yych <= '@') goto yy67;
				goto yy66;
			} else {
				if (yych == '\\') goto yy68;
				goto yy67;
			}
		} else {
			if (yych <= 'f') {
				if (yych <= '_') goto yy66;
				if (yych <= '`') goto yy67;
				goto yy88;
			} else {
				if (yych <= 'z') goto yy66;
				if (yych <= 0x7F) goto yy67;
				goto yy66;
			}
		}
	}
yy89:
	yyaccept = 8;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych == '-') goto yy98;
		if (yych >= '0') goto yy89;
	} else {
		if (yych <= '?') {
			if (yych >= '?') goto yy89;
		} else {
			if (yych <= '@') goto yy90;
			if (yych <= 'F') goto yy89;
		}
	}
yy90:
#line 75 "scanner.l"
	{ return UNICODE_RANGE; }
#line 1237 "<stdout>"
yy91:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '(') {
			if (yych == '\n') goto yy27;
			if (yych <= '\'') goto yy29;
			goto yy69;
		} else {
			if (yych <= '-') {
				if (yych <= ',') goto yy29;
				goto yy27;
			} else {
				if (yych <= '/') goto yy29;
				if (yych <= '9') goto yy27;
				goto yy29;
			}
		}
	} else {
		if (yych <= '^') {
			if (yych <= 'Z') goto yy27;
			if (yych == '\\') goto yy56;
			goto yy29;
		} else {
			if (yych <= '`') {
				if (yych <= '_') goto yy27;
				goto yy29;
			} else {
				if (yych <= 'z') goto yy27;
				if (yych <= 0x7F) goto yy29;
				goto yy27;
			}
		}
	}
yy92:
	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '(') goto yy99;
	goto yy28;
yy93:
	yyaccept = 4;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'Z') {
		if (yych <= '-') {
			if (yych == '\n') goto yy46;
			if (yych <= ',') goto yy47;
			goto yy46;
		} else {
			if (yych <= '/') goto yy47;
			if (yych <= '9') goto yy46;
			if (yych <= '@') goto yy47;
			goto yy46;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy48;
			if (yych <= '^') goto yy47;
			goto yy46;
		} else {
			if (yych <= '`') goto yy47;
			if (yych <= 'z') goto yy46;
			if (yych <= 0x7F) goto yy47;
			goto yy46;
		}
	}
yy94:
	++YYCURSOR;
#line 91 "scanner.l"
	{ goto start; /* ignore comments */ }
#line 1311 "<stdout>"
yy95:
	yyaccept = 5;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'Z') {
		if (yych <= '-') {
			if (yych == '\n') goto yy61;
			if (yych <= ',') goto yy62;
			goto yy61;
		} else {
			if (yych <= '/') goto yy62;
			if (yych <= '9') goto yy61;
			if (yych <= '@') goto yy62;
			goto yy61;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy63;
			if (yych <= '^') goto yy62;
			goto yy61;
		} else {
			if (yych <= '`') goto yy62;
			if (yych <= 'z') goto yy61;
			if (yych <= 0x7F) goto yy62;
			goto yy61;
		}
	}
yy96:
	++YYCURSOR;
#line 77 "scanner.l"
	{ goto start; /* ignore CDO */ }
#line 1344 "<stdout>"
yy97:
	yyaccept = 6;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'Z') {
		if (yych <= '-') {
			if (yych == '\n') goto yy66;
			if (yych <= ',') goto yy67;
			goto yy66;
		} else {
			if (yych <= '/') goto yy67;
			if (yych <= '9') goto yy66;
			if (yych <= '@') goto yy67;
			goto yy66;
		}
	} else {
		if (yych <= '_') {
			if (yych == '\\') goto yy68;
			if (yych <= '^') goto yy67;
			goto yy66;
		} else {
			if (yych <= '`') goto yy67;
			if (yych <= 'z') goto yy66;
			if (yych <= 0x7F) goto yy67;
			goto yy66;
		}
	}
yy98:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy42;
	if (yych <= '9') goto yy100;
	if (yych <= '@') goto yy42;
	if (yych <= 'F') goto yy100;
	goto yy42;
yy99:
	yyaccept = 9;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x1F) {
		if (yych <= '\n') {
			if (yych <= 0x08) goto yy70;
			goto yy102;
		} else {
			if (yych <= '\v') goto yy70;
			if (yych <= '\r') goto yy102;
			goto yy70;
		}
	} else {
		if (yych <= '(') {
			if (yych <= '\'') goto yy102;
			goto yy70;
		} else {
			if (yych == 0x7F) goto yy70;
			goto yy102;
		}
	}
yy100:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') goto yy90;
	if (yych <= '9') goto yy100;
	if (yych <= '@') goto yy90;
	if (yych <= 'F') goto yy100;
	goto yy90;
yy101:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy102:
	if (yych <= '"') {
		if (yych <= '\r') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy101;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= ' ') goto yy101;
			if (yych >= '"') goto yy104;
		}
	} else {
		if (yych <= ')') {
			if (yych <= '&') goto yy103;
			if (yych <= '\'') goto yy105;
			if (yych <= '(') goto yy42;
			goto yy106;
		} else {
			if (yych <= '\\') {
				if (yych >= '\\') goto yy108;
			} else {
				if (yych == 0x7F) goto yy42;
			}
		}
	}
yy103:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\r') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy109;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= ' ') goto yy109;
			if (yych <= '!') goto yy103;
			goto yy42;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '&') goto yy103;
			if (yych <= '(') goto yy42;
			if (yych <= ')') goto yy106;
			goto yy103;
		} else {
			if (yych <= '\\') goto yy108;
			if (yych == 0x7F) goto yy42;
			goto yy103;
		}
	}
yy104:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy42;
			goto yy104;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy104;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy104;
			goto yy110;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy104;
		}
	}
yy105:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy42;
			goto yy105;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy105;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy105;
			goto yy111;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy105;
		}
	}
yy106:
	++YYCURSOR;
yy107:
#line 73 "scanner.l"
	{ return URI; }
#line 1515 "<stdout>"
yy108:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= ')') {
		if (yych <= '\v') {
			if (yych != '\n') goto yy103;
		} else {
			if (yych <= '\r') goto yy109;
			if (yych <= '(') goto yy103;
			goto yy112;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '/') goto yy103;
			if (yych <= '9') goto yy113;
			goto yy103;
		} else {
			if (yych <= '\\') goto yy108;
			if (yych <= '`') goto yy103;
			if (yych <= 'f') goto yy113;
			goto yy103;
		}
	}
yy109:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\r') {
		if (yych <= 0x08) goto yy42;
		if (yych == '\v') goto yy42;
		goto yy109;
	} else {
		if (yych <= ' ') {
			if (yych <= 0x1F) goto yy42;
			goto yy109;
		} else {
			if (yych == ')') goto yy106;
			goto yy42;
		}
	}
yy110:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\r') {
			if (yych <= '\f') goto yy104;
			goto yy114;
		} else {
			if (yych == '"') goto yy115;
			goto yy104;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '9') goto yy116;
			if (yych <= '[') goto yy104;
			goto yy110;
		} else {
			if (yych <= '`') goto yy104;
			if (yych <= 'f') goto yy116;
			goto yy104;
		}
	}
yy111:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\r') {
			if (yych <= '\f') goto yy105;
			goto yy117;
		} else {
			if (yych == '\'') goto yy118;
			goto yy105;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '9') goto yy119;
			if (yych <= '[') goto yy105;
			goto yy111;
		} else {
			if (yych <= '`') goto yy105;
			if (yych <= 'f') goto yy119;
			goto yy105;
		}
	}
yy112:
	yyaccept = 10;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\r') {
			if (yych <= 0x08) goto yy107;
			if (yych == '\v') goto yy107;
			goto yy109;
		} else {
			if (yych <= 0x1F) goto yy107;
			if (yych <= ' ') goto yy109;
			if (yych <= '!') goto yy103;
			goto yy107;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '&') goto yy103;
			if (yych <= '(') goto yy107;
			if (yych <= ')') goto yy106;
			goto yy103;
		} else {
			if (yych <= '\\') goto yy108;
			if (yych == 0x7F) goto yy107;
			goto yy103;
		}
	}
yy113:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '(') {
		if (yych <= '\r') {
			if (yych <= '\n') {
				if (yych <= 0x08) goto yy42;
				goto yy103;
			} else {
				if (yych <= '\v') goto yy42;
				if (yych <= '\f') goto yy103;
				goto yy120;
			}
		} else {
			if (yych <= '!') {
				if (yych <= 0x1F) goto yy42;
				goto yy103;
			} else {
				if (yych <= '"') goto yy42;
				if (yych <= '&') goto yy103;
				goto yy42;
			}
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '/') {
				if (yych <= ')') goto yy106;
				goto yy103;
			} else {
				if (yych <= '9') goto yy113;
				if (yych <= '[') goto yy103;
				goto yy108;
			}
		} else {
			if (yych <= 'f') {
				if (yych <= '`') goto yy103;
				goto yy113;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy103;
			}
		}
	}
yy114:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\n') {
			if (yych <= 0x08) goto yy42;
			goto yy104;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy104;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy104;
			goto yy110;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy104;
		}
	}
yy115:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '!') {
		if (yych <= '\v') {
			if (yych <= 0x08) goto yy42;
			if (yych <= '\t') goto yy115;
			if (yych <= '\n') goto yy109;
			goto yy42;
		} else {
			if (yych <= '\r') goto yy109;
			if (yych <= 0x1F) goto yy42;
			if (yych <= ' ') goto yy115;
			goto yy104;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '"') goto yy109;
			if (yych == ')') goto yy121;
			goto yy104;
		} else {
			if (yych <= '\\') goto yy110;
			if (yych == 0x7F) goto yy42;
			goto yy104;
		}
	}
yy116:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\f') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy104;
		} else {
			if (yych <= '\r') goto yy114;
			if (yych <= 0x1F) goto yy42;
			if (yych <= '!') goto yy104;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '/') goto yy104;
			if (yych <= '9') goto yy116;
			if (yych <= '[') goto yy104;
			goto yy110;
		} else {
			if (yych <= 'f') {
				if (yych <= '`') goto yy104;
				goto yy116;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy104;
			}
		}
	}
yy117:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\n') {
			if (yych <= 0x08) goto yy42;
			goto yy105;
		} else {
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy105;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy105;
			goto yy111;
		} else {
			if (yych == 0x7F) goto yy42;
			goto yy105;
		}
	}
yy118:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '&') {
		if (yych <= '\v') {
			if (yych <= 0x08) goto yy42;
			if (yych <= '\t') goto yy118;
			if (yych <= '\n') goto yy109;
			goto yy42;
		} else {
			if (yych <= '\r') goto yy109;
			if (yych <= 0x1F) goto yy42;
			if (yych <= ' ') goto yy118;
			goto yy105;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '\'') goto yy109;
			if (yych == ')') goto yy122;
			goto yy105;
		} else {
			if (yych <= '\\') goto yy111;
			if (yych == 0x7F) goto yy42;
			goto yy105;
		}
	}
yy119:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\f') {
			if (yych <= 0x08) goto yy42;
			if (yych == '\v') goto yy42;
			goto yy105;
		} else {
			if (yych <= '\r') goto yy117;
			if (yych <= 0x1F) goto yy42;
			if (yych <= '&') goto yy105;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '/') goto yy105;
			if (yych <= '9') goto yy119;
			if (yych <= '[') goto yy105;
			goto yy111;
		} else {
			if (yych <= 'f') {
				if (yych <= '`') goto yy105;
				goto yy119;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy105;
			}
		}
	}
yy120:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '!') {
		if (yych <= '\v') {
			if (yych <= 0x08) goto yy42;
			if (yych <= '\t') goto yy109;
			if (yych <= '\n') goto yy103;
			goto yy42;
		} else {
			if (yych <= '\r') goto yy109;
			if (yych <= 0x1F) goto yy42;
			if (yych <= ' ') goto yy109;
			goto yy103;
		}
	} else {
		if (yych <= ')') {
			if (yych <= '"') goto yy42;
			if (yych <= '&') goto yy103;
			if (yych <= '(') goto yy42;
			goto yy106;
		} else {
			if (yych <= '\\') {
				if (yych <= '[') goto yy103;
				goto yy108;
			} else {
				if (yych == 0x7F) goto yy42;
				goto yy103;
			}
		}
	}
yy121:
	yyaccept = 10;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '"') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy107;
			goto yy104;
		} else {
			if (yych <= 0x1F) goto yy107;
			if (yych <= '!') goto yy104;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy104;
			goto yy110;
		} else {
			if (yych == 0x7F) goto yy107;
			goto yy104;
		}
	}
yy122:
	yyaccept = 10;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '\'') {
		if (yych <= '\t') {
			if (yych <= 0x08) goto yy107;
			goto yy105;
		} else {
			if (yych <= 0x1F) goto yy107;
			if (yych <= '&') goto yy105;
			goto yy109;
		}
	} else {
		if (yych <= '\\') {
			if (yych <= '[') goto yy105;
			goto yy111;
		} else {
			if (yych == 0x7F) goto yy107;
			goto yy105;
		}
	}
}
#line 110 "scanner.l"


}
