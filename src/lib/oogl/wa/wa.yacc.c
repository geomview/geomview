#ifndef lint
static char wafsasccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define wafsaclearin (wafsachar=(-1))
#define wafsaerrok (wafsaerrflag=0)
#define YYRECOVERING (wafsaerrflag!=0)
#define YYPREFIX "wafsa"
#line 17 "fsaparse.y"

#include <stdio.h>
#include "wa.h"
#include "yystype.h"
#include "y.tab.h"

#define MAXSTATES 100
#define MAXGENS 128

static char copyright[] =
  "Copyright (C) 1991 Mark B. Phillips, The Geometry Center";

static int adj[MAXSTATES];
static int adj_index = 0;
static int install_adjacency();

static wa *fsa;

#line 31 "y.tab.c"
#define FORMAT 257
#define FSA 258
#define STATES 259
#define SYMBOLS 260
#define BFS 261
#define MIN 262
#define VARIABLES 263
#define ALPHABET 264
#define START 265
#define ATABLE 266
#define INVERSES 267
#define INV 268
#define LEFT_BRACE 269
#define RIGHT_BRACE 270
#define LEFT_PAREN 271
#define RIGHT_PAREN 272
#define SEMICOLON 273
#define PERCENT 274
#define INT 275
#define EQUAL 276
#define STRING 277
#define REAL 278
#define YYERRCODE 256
short wafsalhs[] = {                                        -1,
    0,    2,    3,    4,    5,    5,    8,    8,    8,    6,
    6,    9,    9,   10,   11,   11,   12,    1,    1,   13,
   13,    7,    7,   14,   14,   15,   16,   16,
};
short wafsalen[] = {                                         2,
    3,    9,    2,    2,    1,    2,    1,    1,    2,    1,
    2,    1,    1,    4,    1,    2,    3,    4,    5,    1,
    2,    2,    1,    1,    2,    4,    1,    2,
};
short wafsadefred[] = {                                      0,
    0,    0,    0,    0,    1,    0,    0,    0,    3,    0,
    0,    4,    7,    8,    0,    0,    0,    9,    0,    0,
   13,    0,    0,   12,    6,    0,    0,    0,   11,    0,
    0,   15,    0,    0,    0,    0,   14,   16,   18,    0,
    0,    0,   22,    0,    2,   17,   21,   19,    0,   25,
   27,    0,   26,   28,
};
short wafsadgoto[] = {                                       2,
   21,    5,    8,   11,   16,   22,   35,   17,   23,   24,
   31,   32,   41,   43,   44,   52,
};
short wafsasindex[] = {                                   -249,
 -264,    0, -243, -253,    0, -242, -257, -241,    0, -255,
 -259,    0,    0,    0, -251, -254, -259,    0, -248, -247,
    0, -246, -254,    0,    0, -250, -245, -240,    0, -244,
 -270,    0, -269, -239, -237, -238,    0,    0,    0, -235,
 -236, -234,    0, -239,    0,    0,    0,    0, -233,    0,
    0, -266,    0,    0,
};
short wafsarindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -252,    0,    0,    0,
    0,    0, -230,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -232,    0,    0,    0,    0,    0, -229,
    0,    0,    0, -225,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short wafsagindex[] = {                                      0,
    0,    0,    0,    0,    6,    4,    0,    0,    0,    0,
    0,   -2,   -9,   -7,    0,    0,
};
#define YYTABLESIZE 45
short wafsatable[] = {                                      37,
   39,   13,   14,   15,   30,   40,   53,    1,   54,   19,
   20,    5,    5,    3,    4,    6,    7,    9,   10,   12,
   26,   27,   25,   18,   30,   34,   29,   28,   38,   33,
   47,   36,   45,   48,    0,   42,   50,   23,   46,   40,
   20,   51,   49,   10,   24,
};
short wafsacheck[] = {                                     270,
  270,  261,  262,  263,  275,  275,  273,  257,  275,  264,
  265,  264,  265,  278,  258,  269,  259,  275,  260,  275,
  269,  269,   17,  275,  275,  266,   23,  274,   31,  275,
   40,  276,  270,  270,   -1,  275,   44,  270,  277,  275,
  270,  275,  277,  274,  270,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 278
#if YYDEBUG
char *wafsaname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"FORMAT","FSA","STATES","SYMBOLS",
"BFS","MIN","VARIABLES","ALPHABET","START","ATABLE","INVERSES","INV",
"LEFT_BRACE","RIGHT_BRACE","LEFT_PAREN","RIGHT_PAREN","SEMICOLON","PERCENT",
"INT","EQUAL","STRING","REAL",
};
char *wafsarule[] = {
"$accept : fsa_file",
"fsa_file : FORMAT REAL fsa_format",
"fsa_format : FSA LEFT_BRACE states symbols fsa_characteristics info_items PERCENT fsa_table RIGHT_BRACE",
"states : STATES INT",
"symbols : SYMBOLS INT",
"fsa_characteristics : fsa_characteristic",
"fsa_characteristics : fsa_characteristic fsa_characteristics",
"fsa_characteristic : BFS",
"fsa_characteristic : MIN",
"fsa_characteristic : VARIABLES INT",
"info_items : info_item",
"info_items : info_item info_items",
"info_item : alphabet",
"info_item : start_states",
"alphabet : ALPHABET LEFT_BRACE symbol_names RIGHT_BRACE",
"symbol_names : symbol_name",
"symbol_names : symbol_names symbol_name",
"symbol_name : INT EQUAL STRING",
"start_states : START LEFT_BRACE INT RIGHT_BRACE",
"start_states : START LEFT_BRACE INT int_list RIGHT_BRACE",
"int_list : INT",
"int_list : INT int_list",
"fsa_table : ATABLE acstates",
"fsa_table : ATABLE",
"acstates : acstate",
"acstates : acstate acstates",
"acstate : INT STRING adjacency SEMICOLON",
"adjacency : INT",
"adjacency : adjacency INT",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int wafsadebug;
int wafsanerrs;
int wafsaerrflag;
int wafsachar;
short *wafsassp;
YYSTYPE *wafsavsp;
YYSTYPE wafsaval;
YYSTYPE wafsalval;
short wafsass[YYSTACKSIZE];
YYSTYPE wafsavs[YYSTACKSIZE];
#define wafsastacksize YYSTACKSIZE
#line 162 "fsaparse.y"

wafsaerror(s)
char *s;
{
  printf("wafsaerror: %s\n", s);
}

static int
  install_adjacency(n)
int n;
{
  int i;

  for (i=0; i<adj_index; ++i)
    fsa->action[n][i+1] = adj[i];
}

parse_init(f)
     wa *f;
{
  fsa = f;

  fsa->fail = fsa->state = 0;
}

/*-----------------------------------------------------------------------
 * Function:	fsaparse
 * Description:	parse a .wa file into an wa struct
 * Args  IN:	fp: .wa file pointer
 *      OUT:	*fsa: the resulting fsa struct
 * Returns:	1 for success, 0 for error
 * Author:	mbp
 * Date:	Sat Mar 23 21:47:04 1991
 * Notes:	Does not recognize the full grammar defined by
 *		"Data Exchange for Automatic Groups".  Only recognizes
 *		files of the form used by several examples I was
 *		using at the time I wrote this.  I think, but am not
 *		at all sure, that this includes all word-acceptor
 *		automata.
 */
int
  fsaparse(fp, fsa)
FILE *fp;
wa *fsa;
{
  extern FILE *wafsain;
  int ret;

  parse_init(fsa);

  wafsain = fp;
  ret = wafsaparse();

  return(ret ==0);
}
#line 240 "y.tab.c"
#define YYABORT goto wafsaabort
#define YYREJECT goto wafsaabort
#define YYACCEPT goto wafsaaccept
#define YYERROR goto wafsaerrlab
int
wafsaparse()
{
    register int wafsam, wafsan, wafsastate;
#if YYDEBUG
    register char *wafsas;
    extern char *getenv();

    if (wafsas = getenv("YYDEBUG"))
    {
        wafsan = *wafsas;
        if (wafsan >= '0' && wafsan <= '9')
            wafsadebug = wafsan - '0';
    }
#endif

    wafsanerrs = 0;
    wafsaerrflag = 0;
    wafsachar = (-1);

    wafsassp = wafsass;
    wafsavsp = wafsavs;
    *wafsassp = wafsastate = 0;

wafsaloop:
    if (wafsan = wafsadefred[wafsastate]) goto wafsareduce;
    if (wafsachar < 0)
    {
        if ((wafsachar = wafsalex()) < 0) wafsachar = 0;
#if YYDEBUG
        if (wafsadebug)
        {
            wafsas = 0;
            if (wafsachar <= YYMAXTOKEN) wafsas = wafsaname[wafsachar];
            if (!wafsas) wafsas = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, wafsastate, wafsachar, wafsas);
        }
#endif
    }
    if ((wafsan = wafsasindex[wafsastate]) && (wafsan += wafsachar) >= 0 &&
            wafsan <= YYTABLESIZE && wafsacheck[wafsan] == wafsachar)
    {
#if YYDEBUG
        if (wafsadebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, wafsastate, wafsatable[wafsan]);
#endif
        if (wafsassp >= wafsass + wafsastacksize - 1)
        {
            goto wafsaoverflow;
        }
        *++wafsassp = wafsastate = wafsatable[wafsan];
        *++wafsavsp = wafsalval;
        wafsachar = (-1);
        if (wafsaerrflag > 0)  --wafsaerrflag;
        goto wafsaloop;
    }
    if ((wafsan = wafsarindex[wafsastate]) && (wafsan += wafsachar) >= 0 &&
            wafsan <= YYTABLESIZE && wafsacheck[wafsan] == wafsachar)
    {
        wafsan = wafsatable[wafsan];
        goto wafsareduce;
    }
    if (wafsaerrflag) goto wafsainrecovery;
#ifdef lint
    goto wafsanewerror;
#endif
wafsanewerror:
    wafsaerror("syntax error");
#ifdef lint
    goto wafsaerrlab;
#endif
wafsaerrlab:
    ++wafsanerrs;
wafsainrecovery:
    if (wafsaerrflag < 3)
    {
        wafsaerrflag = 3;
        for (;;)
        {
            if ((wafsan = wafsasindex[*wafsassp]) && (wafsan += YYERRCODE) >= 0 &&
                    wafsan <= YYTABLESIZE && wafsacheck[wafsan] == YYERRCODE)
            {
#if YYDEBUG
                if (wafsadebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *wafsassp, wafsatable[wafsan]);
#endif
                if (wafsassp >= wafsass + wafsastacksize - 1)
                {
                    goto wafsaoverflow;
                }
                *++wafsassp = wafsastate = wafsatable[wafsan];
                *++wafsavsp = wafsalval;
                goto wafsaloop;
            }
            else
            {
#if YYDEBUG
                if (wafsadebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *wafsassp);
#endif
                if (wafsassp <= wafsass) goto wafsaabort;
                --wafsassp;
                --wafsavsp;
            }
        }
    }
    else
    {
        if (wafsachar == 0) goto wafsaabort;
#if YYDEBUG
        if (wafsadebug)
        {
            wafsas = 0;
            if (wafsachar <= YYMAXTOKEN) wafsas = wafsaname[wafsachar];
            if (!wafsas) wafsas = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, wafsastate, wafsachar, wafsas);
        }
#endif
        wafsachar = (-1);
        goto wafsaloop;
    }
wafsareduce:
#if YYDEBUG
    if (wafsadebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, wafsastate, wafsan, wafsarule[wafsan]);
#endif
    wafsam = wafsalen[wafsan];
    wafsaval = wafsavsp[1-wafsam];
    switch (wafsan)
    {
case 2:
#line 60 "fsaparse.y"
{ return(0); }
break;
case 3:
#line 64 "fsaparse.y"
{
		      fsa->nstates = wafsavsp[0].i;
		      fsa->action =
			(int**)malloc((fsa->nstates+1)*sizeof(int*));
		    }
break;
case 4:
#line 72 "fsaparse.y"
{
		      int i,j;
		      fsa->ngens = wafsavsp[0].i;
		      for (i=0; i<=fsa->nstates; ++i) {
			fsa->action[i] =
			  (int*)malloc((fsa->ngens+1)*sizeof(int));
			for (j=0; j<=fsa->ngens; ++j)
			  fsa->action[i][j] = 0;
		      }
		      fsa->genlist=(char**)malloc(fsa->ngens*sizeof(char*));
		      for (i=0; i<fsa->ngens; ++i)
			fsa->genlist[i] = "";
		    }
break;
case 13:
#line 106 "fsaparse.y"
{
		      fsa->start= wafsavsp[0].i;
		    }
break;
case 17:
#line 119 "fsaparse.y"
{
		      fsa->genlist[wafsavsp[-2].i-1] =
			(char*)malloc((strlen(wafsavsp[0].s)+1)*sizeof(char));
		      strcpy(fsa->genlist[wafsavsp[-2].i-1], wafsavsp[0].s);
		    }
break;
case 18:
#line 127 "fsaparse.y"
{ wafsaval.i = wafsavsp[-1].i; }
break;
case 19:
#line 129 "fsaparse.y"
{ wafsaval.i = wafsavsp[-2].i; }
break;
case 26:
#line 147 "fsaparse.y"
{ install_adjacency(wafsavsp[-3].i); adj_index = 0; }
break;
case 27:
#line 151 "fsaparse.y"
{
		    adj[adj_index++] = wafsavsp[0].i;
		  }
break;
case 28:
#line 155 "fsaparse.y"
{
		    adj[adj_index++] = wafsavsp[0].i;
		  }
break;
#line 447 "y.tab.c"
    }
    wafsassp -= wafsam;
    wafsastate = *wafsassp;
    wafsavsp -= wafsam;
    wafsam = wafsalhs[wafsan];
    if (wafsastate == 0 && wafsam == 0)
    {
#if YYDEBUG
        if (wafsadebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        wafsastate = YYFINAL;
        *++wafsassp = YYFINAL;
        *++wafsavsp = wafsaval;
        if (wafsachar < 0)
        {
            if ((wafsachar = wafsalex()) < 0) wafsachar = 0;
#if YYDEBUG
            if (wafsadebug)
            {
                wafsas = 0;
                if (wafsachar <= YYMAXTOKEN) wafsas = wafsaname[wafsachar];
                if (!wafsas) wafsas = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, wafsachar, wafsas);
            }
#endif
        }
        if (wafsachar == 0) goto wafsaaccept;
        goto wafsaloop;
    }
    if ((wafsan = wafsagindex[wafsam]) && (wafsan += wafsastate) >= 0 &&
            wafsan <= YYTABLESIZE && wafsacheck[wafsan] == wafsastate)
        wafsastate = wafsatable[wafsan];
    else
        wafsastate = wafsadgoto[wafsam];
#if YYDEBUG
    if (wafsadebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *wafsassp, wafsastate);
#endif
    if (wafsassp >= wafsass + wafsastacksize - 1)
    {
        goto wafsaoverflow;
    }
    *++wafsassp = wafsastate;
    *++wafsavsp = wafsaval;
    goto wafsaloop;
wafsaoverflow:
    wafsaerror("yacc stack overflow");
wafsaabort:
    return (1);
wafsaaccept:
    return (0);
}
