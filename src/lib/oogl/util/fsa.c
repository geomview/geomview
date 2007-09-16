/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/* $Id: fsa.c,v 1.5 2007/09/16 09:50:46 rotdrop Exp $
 *
 * fsa.c: finite state automaton for matching a finite set of strings
 *
 * The procedures in this file are documented in the manual page fsa.1,
 * a formatted copy of which is in ../../../doc/fsa.man.
 *
 * Mark Phillips
 * April 17, 1989: originally written
 * Nov 18 1991: modified to allow > 1 fsa
 */

#include <stdio.h>
#include "ooglutil.h"

/* size of blocks in which to allocate state nodes */
#define BLKSIZ 5

/* Special states and flags */
#define ACCEPT -1
#define REJECT -2
#define ERROR  -3

/* Operation flags */
#define PROGRAM 1
#define PARSE   2

typedef struct Trule_s {
  char			c;	/* char for this rule */
  int  			ns;	/* state to switch to */
  struct Trule_s 	*next;	/* ptr to next rule for this state */
} Trule;

typedef struct State_s {
  Trule *tlist;		/* list of transition rules for this state */
  void *return_value;	/* value to return if we end in this state */
} State;

struct Fsa_s {
  State **state;
  int state_count;
  void *reject_value;
  int initial_state;
  void *return_value;
};

/* This must be included *after* the typedef struct Fsa_s above: */
#include "fsa.h"

static Trule *new_trule_node(Fsa, int);
static void *fsa_execute(Fsa, const char *, void *, int);
static int next_state(Fsa, int, char, void *, int);
static int new_state(Fsa);
static void delete_trule_list(Trule *);

/*-----------------------------------------------------------------------
 * Function:	fsa_initialize
 * Description:	Initialize an FSA
 * Args  IN:	fsa: the fsa to initialize; if NULL, a new one is created
 *		reject: value to return when parsing unacceptable
 *		  string
 */
Fsa fsa_initialize(Fsa fsa, void *reject)
{
  if (fsa == NULL)
    fsa = OOGLNewE(struct Fsa_s, "struct Fsa");
  else {
    /* Clear out current program: */
    while (fsa->state_count--) {
      delete_trule_list( (fsa->state[fsa->state_count])->tlist );
      OOGLFree((char*)(fsa->state[fsa->state_count]));
    }
    OOGLFree((char*)(fsa->state));
  }
  fsa->state_count = 0;
  fsa->reject_value = reject;
  fsa->initial_state = new_state(fsa);

  return fsa;
}

/*-----------------------------------------------------------------------
 * Function:	fsa_delete
 * Description:	Delete an FSA
 * Args  IN:	fsa: the fsa to delete;
 */
void fsa_delete(Fsa fsa)
{
  if (fsa == NULL) {
    return;
  }
  /* Clear out current program: */
  while (fsa->state_count--) {
    delete_trule_list( (fsa->state[fsa->state_count])->tlist );
    OOGLFree((char*)(fsa->state[fsa->state_count]));
  }
  OOGLFree((char*)(fsa->state));
  OOGLFree(fsa);
}

/*-----------------------------------------------------------------------
 * Function:	fsa_install
 * Description:	Install a string in an FSA
 * Args  IN:	fsa: the fsa to install into
 *		s: the string to install
 *		v: the value to return when s is parsed
 * Returns:	v, unless there is not enough room in the FSA, in
 *		which case the fsa's reject value is returned
 */
void *
fsa_install(Fsa fsa, const char *s, void *v)
{
  return fsa_execute(fsa, s, v, PROGRAM);
}

/*-----------------------------------------------------------------------
 * Function:	fsa_parse
 * Description:	Parse a string
 * Args  IN:	fsa: the fsa to use
 *		s: the string to parse
 * Returns:	If s is acceptable, returns the value v that was
 *		specified when s was installed with fsa_install().  If
 *		v is not acceptable, returns the value of reject given
 *		in the most recent call to fsa_initialize().
 * Notes:	
 */
void *
fsa_parse(Fsa fsa, const char *s)
{
  return fsa_execute(fsa, s, 0, PARSE);
}

/*-----------------------------------------------------------------------
 * Function:	fsa_execute
 * Description:	parse or program (install) a string
 * Args  IN:	fsa: the fsa to use
 *		s: the string
 *		v: value to correspond to string
 *		op: PROGRAM or PARSE
 * Returns:	Result of parsing or installing.  If op==PROGRAM, this
 *		is always v, unless there is no more room in the FSA,
 *		in which case it is the fsa' reject value.  If op==PARSE,
 *		it is either v, or the reject value.
 *
 *		The philosophy of this procedure and procedure
 *		next_state() is that programming the FSA and parsing a
 *		string using the FSA are very similar processes; the
 *		only difference is in what is done when there is no
 *		rule for the current state corresponding to the
 *		current input char.  When parsing, we return a
 *		rejection.  When programming, we add a rule, and
 *		possibly a new state.
 *
 *		If op==PROGRAM, s is added to list of acceptable
 *		strings, and the FSA is programmed to return v for s.
 *		If op==PARSE, the FSA is executed and returns value
 *		corresponding to s, or reject value if s is not
 *		acceptable.
 */
static void *fsa_execute(Fsa fsa, const char *s, void *v, int op)
{
  int cs;			/* current state */

  if (s==NULL) return fsa->reject_value;
  cs = fsa->initial_state;
  fsa->return_value = fsa->reject_value;
  while ( (cs != ACCEPT) && (cs != REJECT) && (cs != ERROR) ) {
    cs = next_state(fsa, cs, *s, v, op);
    ++s;
  }
  return( (cs == ERROR) ? fsa->reject_value : fsa->return_value );
}

/*-----------------------------------------------------------------------
 * Function:	next_state
 * Description:	Get the next state based on current state and char
 * Args  IN:	fsa: the fsa to use
 *		cs: current state
 *		c: current char
 *		v: return value when programing
 *		op: PROGRAM or PARSE
 * Returns:	index of next state, or ERROR
 * Notes:	If op==PROGRAM and no transition rule for c exists for
 *		the current state, a new state is created and a
 *		transition rule to that state is added to the current
 *		state.
 *
 *		See also comments for fsa_execute().
 */
static int
next_state(Fsa fsa, int cs, char c, void *v, int op)
{
  Trule *t;

  /* Check tlist of current state for rule for c */
  t = (fsa->state[cs])->tlist;
  while (t != NULL) {
    if (t->c == c) {
      /* We found rule for c; if next state is ACCEPT, set return value. */
      /* Then return next state. */
      if (t->ns == ACCEPT) {
	if (op==PROGRAM)
	  fsa->return_value = (fsa->state[cs])->return_value = v;
	else
	  fsa->return_value = (fsa->state[cs])->return_value;
      }
      return(t->ns);
    }
    t = t->next;
  }
  /* No rule for c present; if just parsing, return rejection */
  if (op == PARSE) return(REJECT);

  /* Otherwise, add a rule for c to current state */

  /* Install a rule node for c in current state's tlist */
  t = new_trule_node(fsa, cs);
  if (t == NULL) return(ERROR);
  t->c = c;

  /* Now specify the next state for this rule */
  if (c == '\0') {
    /* '\0' means end of a word, so set return value for this state */
    /* to v and set the next state for this rule to ACCEPT */
    fsa->return_value = (fsa->state[cs])->return_value = v;
    t->ns = ACCEPT;
  }
  else {
    /* Not '\0' means another char in word, so create a new state */
    t->ns = new_state(fsa);
    if (t->ns == ERROR) return(ERROR);
  }

  return(t->ns);
}

/*-----------------------------------------------------------------------
 * Function:	new_trule_node
 * Description:	return a ptr to a new trule node in the tlist of a state
 * Args  IN:	fsa: the fsa to use
 *		n: index of state to add node to
 * Returns:	ptr to newly allowcated rule node
 */
static Trule *
new_trule_node(Fsa fsa, int n)
{
  Trule *t, *tnew;

  /* Allocate and initialize the node */
  tnew = OOGLNewE(Trule, "Trule *");
  if (tnew == NULL) return(NULL);
  tnew->c = '\1';
  tnew->ns = REJECT;
  tnew->next = NULL;

  /* Install it in tlist of state n */
  if (fsa->state[n]->tlist == NULL) {
    /* List is empty: */
    fsa->state[n]->tlist = tnew;
  }
  else {
    /* List is not empty; set t = last node in list, then add tnew to end */
    t = fsa->state[n]->tlist;
    while (t->next != NULL)  t = t->next;
    t->next = tnew;
  }

  /* Return ptr to the new trule node */
  return(tnew);
}

/*-----------------------------------------------------------------------
 * Function:	new_state
 * Description:	get a new state for an fsa
 * Args:	fsa: the fsa to use
 * Returns:	index (an int) of new state
 */
static int
new_state(Fsa fsa)
{
  if (fsa->state_count == 0) {
    fsa->state = OOGLNewNE(State *, BLKSIZ, "State *");
  }
  else if ((fsa->state_count % BLKSIZ == 0)) {
    fsa->state =
      OOGLRenewNE(State *,
		  fsa->state,
		  (fsa->state_count/BLKSIZ+1)*BLKSIZ,
		  "reallocating for State *");
  }

  fsa->state[fsa->state_count] = OOGLNewE(State, "State");
  if (fsa->state[fsa->state_count] == NULL) return(ERROR);
  fsa->state[fsa->state_count]->return_value = fsa->reject_value;
  fsa->state[fsa->state_count]->tlist = NULL;
  ++fsa->state_count;
  return(fsa->state_count-1);
}

/*-----------------------------------------------------------------------
 * Function:	delete_trule_list
 * Description:	Delete a trule list, freeing up its nodes
 * Args  IN:	tlist: ptr to first node in list
 */
static void
delete_trule_list(Trule *tlist)
{
  Trule *t;

  while (tlist != NULL) {
    t = tlist;
    tlist = tlist->next;
    OOGLFree((char*)t);
  }
}
