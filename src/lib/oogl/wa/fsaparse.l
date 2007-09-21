%{

/*
 * fsaparse.l (lex.yy.c)
 *
 * Lex definitions for lexical scanner for parsing word-acceptor
 * automata.  See fsaparse.y for more info.
 *
 * mbp Sat Mar 23 21:58:12 1991
 * mbp@thales.urich.edu
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h> /* for atof() */
#include <math.h>
#include <string.h>
#include "wa.yystype.h"
#include "wa.yacc.h"

YYSTYPE yylval;

/* disable output of unmatched input */
#undef output
#define output(c) 

%}

DIGIT	[0-9]
WS	[ \t\n,]
LETTER	[A-Za-z]
SIGN	[+-]

%% /* RULES SECTION */

{WS}+			;

"Format"	{ return(FORMAT);	}
"format"	{ return(FORMAT);	}
"fsa"		{ return(FSA);		}
"states"	{ return(STATES);	}
"symbols"	{ return(SYMBOLS);	}
"bfs"		{ return(BFS);		}
"min"		{ return(MIN);		}
"variables"	{ return(VARIABLES);	}
"alphabet"	{ return(ALPHABET);	}
"start"		{ return(START);	}
"atable"	{ return(ATABLE);	}
"inverses"	{ return(INVERSES);	}
"inv"		{ return(INV);		}

"{"		{ return(LEFT_BRACE);	}
"}"		{ return(RIGHT_BRACE);	}
"("		{ return(LEFT_PAREN);	}
")"		{ return(RIGHT_PAREN);	}
";"		{ return(SEMICOLON);	}
"%"		{ return(PERCENT);	}
"="		{ return(EQUAL);	}

{SIGN}?{DIGIT}+			{ yylval.i = atoi(yytext); return(INT);  }
{SIGN}?{DIGIT}*\.{DIGIT}?	{ yylval.d = atof(yytext); return(REAL); }
{SIGN}?{DIGIT}+\.{DIGIT}*	{ yylval.d = atof(yytext); return(REAL); }

[^ \t\n{};%()=]+		{ strcpy(yylval.s, yytext); return(STRING); }

%%  /* SUBPROGRAMS SECTION */

int yywrap()
{
  return(1);
}
