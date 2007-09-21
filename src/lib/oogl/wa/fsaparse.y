/*
 * fsaparse.y
 *
 *	Yacc grammar for parsing word-acceptor automata in the format
 *	described by DEAGL ("Data Exchange for Automata and Groups
 *	Language").
 *
 *	This grammar does NOT recognize the entire language described
 *	in DEAGL.  The main function is fsaparse(), defined below.  See
 *	the comments in that function for more info.
 *
 *	mbp Sat Mar 23 21:56:30 1991
 *	mbp@thales.urich.edu
 */

%{

#if 0
static char copyright[] =
  "Copyright (C) 1991 Mark B. Phillips, The Geometry Center";
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wa.h"
#include "wa.yystype.h"
#include "wa.yacc.h"

#define MAXSTATES 100
#define MAXGENS 128

static int adj[MAXSTATES];
static int adj_index = 0;
static void install_adjacency();
static void yyerror(char *s);
extern int yylex(void);

static wa *fsa;

%}

%start fsa_file

%token <i> FORMAT FSA STATES SYMBOLS BFS MIN VARIABLES ALPHABET START ATABLE INVERSES INV LEFT_BRACE RIGHT_BRACE LEFT_PAREN RIGHT_PAREN SEMICOLON PERCENT INT EQUAL

%token <s> STRING

%token <d> REAL

%type <i> start_states

%%
fsa_file	: FORMAT REAL fsa_format
		;

fsa_format	: FSA
		  LEFT_BRACE
		  states
		  symbols
		  fsa_characteristics
		  info_items
		  PERCENT
		  fsa_table
		  RIGHT_BRACE
		    { return(0); }
		; 

states		: STATES INT
		    {
		      fsa->nstates = $2;
		      fsa->action =
			(int**)malloc((fsa->nstates+1)*sizeof(int*));
		    }
		;

symbols		: SYMBOLS INT
		    {
		      int i,j;
		      fsa->ngens = $2;
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
		;

fsa_characteristics
		: fsa_characteristic
		| fsa_characteristic fsa_characteristics
		;


fsa_characteristic
		: BFS
  		| MIN
		| VARIABLES INT
		;


info_items	: info_item
		| info_item info_items
		;

info_item	: alphabet
  		| start_states
		    {
		      fsa->start= $1;
		    }
		;

alphabet	: ALPHABET LEFT_BRACE symbol_names RIGHT_BRACE
		;

symbol_names	: symbol_name
  		| symbol_names symbol_name
		;

symbol_name	: INT EQUAL STRING
		    {
		      fsa->genlist[$1-1] =
			(char*)malloc((strlen($3)+1)*sizeof(char));
		      strcpy(fsa->genlist[$1-1], $3);
		    }
		;

start_states	: START LEFT_BRACE INT RIGHT_BRACE
		    { $$ = $3; }
		| START LEFT_BRACE INT int_list RIGHT_BRACE
		    { $$ = $3; }
		
		;

int_list	: INT
		| INT int_list
		;


fsa_table	: ATABLE acstates
		| ATABLE
		;

acstates	: acstate
		| acstate acstates
		;

acstate		: INT STRING adjacency SEMICOLON
		  { install_adjacency($1); adj_index = 0; }
		;

adjacency	: INT
		  {
		    adj[adj_index++] = $1;
		  }
		| adjacency INT
		  {
		    adj[adj_index++] = $2;
		  }
		;


%%

static void yyerror(char *s)
{
  printf("yyerror: %s\n", s);
}

static void
  install_adjacency(n)
int n;
{
  int i;

  for (i=0; i<adj_index; ++i)
    fsa->action[n][i+1] = adj[i];
}

void
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
  extern FILE *yyin;
  int ret;

  parse_init(fsa);

  yyin = fp;
  ret = yyparse();

  return(ret ==0);
}
