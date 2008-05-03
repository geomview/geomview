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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "glob.h"
#include "animate.h"
#include "xanimate.h"

/*****************************************************************************/
/* static global vars: */

static char	      **frames = NULL;
static int		numframes = 0;
static int		curframe = 0;
static int		range = 0;
static int		bounce = 0;
static int		once = 0;
static int		direction = 1;

static int		hidden = 0;
static int		willplay = 0;
static int		mypid;

static char	       *command = NULL;

/*****************************************************************************/
/* static functions: */

static void clear_frames();
static void anim_parse_args(int, char **);

/*****************************************************************************/

void anim_close()
{
  clear_frames();
  exit(0);
}

/*****************************************************************************/

static void showusage()
{
  fprintf(stderr,"Incorrect command line arguments.\n");
  fprintf(stderr,"Usage: animate [-w posx posy] [-p] [-b] [-s speed] [-o] [-S script] [-hide]\n\t\t[-f file1 file2 ...]\n");

  anim_close();
}


/*****************************************************************************/

static char *concat_strings(char **str, int numstr)
{
  int length = 0, n, pos, ch;
  char *scrbuf;

  for (n = 0; n < numstr; n++)
    length += (strlen(str[n]) + 1);

  length += 10;				/* for good measure :-) */

  scrbuf = (char *) malloc(sizeof(char) * length);

  pos = 0;

  scrbuf[pos] = '{';
  pos++;
  for (n = 0; n < numstr; n++)
  {
    for (ch = 0; ch < (int)strlen(str[n]); ch++)
    {
      scrbuf[pos] = (str[n])[ch];
      pos++;
    }
    scrbuf[pos] = ',';
    pos++;
  }

  scrbuf[pos] = '}';
  scrbuf[pos + 1] = '\0';

  return scrbuf;
  
}

/*****************************************************************************/

static void anim_parse_args(int argc, char *argv[])
{
  int argnum = 1, dlen = 0, tmp = 0, scriptin = 0;
  char *darg = NULL, *str;

  if (argc == 1)
    return;

  while (argnum < argc)
  {
    if ((*argv[argnum] == '-')  &&
		((dlen = strlen(argv[argnum])) >= 2))
    {
      darg = argv[argnum];
      darg++;

      if (dlen == 2)
      {
	if (*darg == 'p')
	{
	  willplay = 1;
	  argnum++;
	}
	else
	if (*darg == 'b')
	{
	  UIsetBounce(1);
	  anim_bounce(1);
	  argnum++;
	}
	else
	if (*darg == 's')
	{
	  if ((argc-argnum) < 2)
	    showusage();
          if (!sscanf(argv[argnum+1], "%d", &tmp))
	    showusage();

	  UIsetSpeed(tmp);
	  argnum += 2;
	}
	else
	if (*darg == 'o')
	{
	  UIsetOnce(1);
	  anim_once(1);
	  argnum++;
	}
	else
	if (*darg == 'f')
	{
	  if ((argc-argnum) < 2)
	    showusage();
	  if (scriptin)
	  {
	    fprintf(stderr, "Can\'t use -S and -f simultaneously.\n");
	    fprintf(stderr, "Defaulted to -S.\n");
	  }
	  else
	  {
	    str = concat_strings( &(argv[argnum + 1]), argc - argnum - 1);
	    anim_load(str);
	    free(str);
	  }

	  argnum = argc;
	}
	else
	if (*darg == 'S')
	{
	  if ((argc-argnum) < 2)
	    showusage();

	  anim_loadscript( argv[argnum + 1]);
	  scriptin = 1;

	  argnum += 2;

	}
	else
	if (*darg == 'w')
	{
	  fprintf(stderr,"Window positioning not currently available.\n");
	  argnum += 3; 
	}
      }
      else
      if ((dlen == 5) && (!strcmp(darg, "hide")))
      {
	hidden = 1;
	argnum++;
      }
      else
	showusage();
    }
  }
}

/*****************************************************************************/

int
main(int argc, char *argv[])
{

  mypid = (int) getpid();

  UIloadinterface();

  anim_parse_args(argc, argv);

  if (!hidden)
    UIshowinterface();

  if (willplay)
    anim_play();

  UImainloop();
  return 0;
}

/*****************************************************************************/

char *get_info()
{

  static char info[] = "By Daeron Meyer\n\
Version 1.0X\n\
Copyright (c) 1994\n\
The Geometry Center\n\
www.geomview.org\n\
\n\
Geom-Animator is free software and is designed\n\
to be used as an animation tool for Geomview.\n\
It provides several simple animation controls such\n\
as playing, stopping, bouncing, single frame\n\
stepping, as well as going through frames once,\n\
then stopping. It also has the capability to\n\
confine frame playing to a subset of the total\n\
frames with the Range toggle. A brief description\n\
of functionality is included but for a more\n\
complete tutorial, please read the man pages.\n\
\n\
To read in a list of frames, enter their names\n\
in the input box at the bottom of the main panel\n\
and press click on Load. The input box should\n\
also accept any regular expressions. To read\n\
in a script containing a list of frames type\n\
the name of the script in the input box and click\n\
on the Script button.\n\
\n\
Once the frames have been read in (the animate\n\
object will appear in the geomview object\n\
browser) one can scroll through the frames by\n\
clicking on play or clicking on the forward or\n\
backward step buttons. To go directly to a frame,\n\
double click on the list entry for that frame.\n\
Clicking on stop will cause Animator to stop\n\
playing frames. The speed at which frames are\n\
playing can be adjusted using the speed slider.\n\
\n\
Clicking on the once button will set up Animator\n\
to go through a sequence of frames only once\n\
when you click on Play, then cause it to stop on\n\
the last or first frame (depending on the direction\n\
you are playing in).\n\
\n\
Toggling the Bounce button on will put Animator\n\
in bounce mode. This means that once the last\n\
or first frame is reached (while playing frames)\n\
the direction of play will automatically be\n\
reversed.\n\
\n\
Toggling the Range button on will put Animator\n\
in range mode. In this mode, Animator confines\n\
itself to looping through only the frames that\n\
have been outlined in the frame list.";

  return info;
}

/*****************************************************************************/

static void clear_frames()
{
  int n;

  if (numframes == 0)
    return;

  fprintf(stdout,"(progn\n");

  for (n = 0; n < numframes; n++)
  {
    fprintf(stdout, "(read geometry { define a%d-%d { LIST } } )\n", n, mypid);
    free(frames[n]);
  }

  fprintf(stdout,")\n");

  fflush(stdout);
  free(frames);
  frames = NULL;
  numframes = 0;

}

/*****************************************************************************/

static void load_frames(char **new_frames)
{
  char ret = '\0';

  frames = new_frames;
  numframes = 0;

  UIclearlist();
  while ((frames[numframes] != NULL) && (*frames[numframes] != '\0'))
  {
    fprintf(	stdout,
		"(read geometry { define a%d-%d < \"%s\" } )\n",
		numframes,
		mypid,
		frames[numframes]);

    fprintf(	stdout,
		"(read geometry { define anim%d { LIST : a%d-%d } } )\n",
		mypid,
		numframes,
		mypid);

    fprintf(	stdout, "(echo q)\n");

    fflush(stdout);

    ret = (char) fgetc(stdin);
    if (ret != 'q')
      anim_close();			/* something must be terribly wrong */

    UIaddlist(frames[numframes]);

    numframes++;
  }

  curframe = 0;
  anim_goframe(curframe);
}

/*****************************************************************************/

void anim_goframe(int num)
{
  char ret = '\0';

  if (num >= numframes)
    return;

  curframe = num;			/* in case UI is calling this */

  fprintf(	stdout,
		"(read geometry { define anim%d { LIST : a%d-%d } } )\n",
		mypid,
		num,
		mypid);

  if (command)
    fprintf(	stdout, command, num);

  fprintf(	stdout, "\n(echo q)\n");

  fflush(stdout);

  ret = (char) fgetc(stdin);
  if (ret != 'q')
    anim_close();			/* something must be terribly wrong */

  UIsetframe(frames[num]);

}

/*****************************************************************************/

void anim_sigframes(int cur, int *prev, int *next, int *first, int *last)
{
  int *pos_list;
  int pos_count;
  int will_free = 0;
  int n = 0, hit = 0;

  if (numframes == 0)
  {
    anim_close();			/* something must be terribly wrong */
  }

  if (!range)
  {
    *prev = cur - 1;
    if (*prev < 0)
      *prev = numframes - 1;

    *next = cur + 1;
    if (*next >= numframes)
      *next = 0;

    *first = 0;
    *last = numframes - 1;

    return;
  }

  will_free = UIgetselected(&pos_list, &pos_count);

  if (pos_count == 0)
  {
    *prev = cur;
    *next = cur;
    *first = 0;
    *last = numframes - 1;
  }
  else
  if (pos_count == 1)
  {
    if (!pos_list)
      *prev = cur;
    else
      *prev = pos_list[0] - 1;

    if ((*prev < 0) || (*prev >= numframes))
      *prev = 0;
    *next = *prev;
    *first = *prev;
    *last = *prev;
  }
  else
    {
      *first = pos_list[0] - 1;
      *last = pos_list[pos_count - 1] - 1;
      *prev = -1;
      *next = *first;
      for (n = 0; ((n < pos_count) && !hit) ; n++)
      {
        if (((pos_list[n] - 1) == cur) || ((pos_list[n] - 1) > cur))
	{
	  hit = 1;
	  if (n == (pos_count - 1))
	    *next = pos_list[0] - 1;
	  else
	    *next = pos_list[n+1] - 1;
	}
	else
	  *prev = pos_list[n] - 1;
      }
      if (*prev == -1)
	*prev = pos_list[pos_count - 1] - 1;
    }

  if (will_free)
    free(pos_list);
}

/*****************************************************************************/

void anim_stop()
{
  UIstopplay();
}

/*****************************************************************************/

void anim_play()
{
  if (numframes)
    UIstartplay();
}

/*****************************************************************************/

void anim_playing()
{
  if (direction)
    anim_stepf();
  else
    anim_stepb();
}

/*****************************************************************************/

void anim_stepf()
{
  int next, prev, first, last;

  if (!numframes)
    return;

  anim_sigframes(curframe, &prev, &next, &first, &last);

  curframe = next;
  
  direction = 1;
  if ((curframe == (numframes - 1)) || (range && (curframe == last)))
  {
    if (once)
      anim_stop();
    if (bounce)
      direction = 0;
  }

  anim_goframe(curframe);
}

/*****************************************************************************/

void anim_stepb()
{
  int next, prev, first, last;

  if (!numframes)
    return;

  anim_sigframes(curframe, &prev, &next, &first, &last);

  curframe = prev;

  direction = 0;
  if ((curframe == 0) || (range && (curframe == first)))
  {
    if (once)
      anim_stop();
    if (bounce)
      direction = 1;
  }

  anim_goframe(curframe);
}

/*****************************************************************************/

void anim_range(int num)
{
  range = num;
}

/*****************************************************************************/

void anim_bounce(int num)
{
  bounce = num;
}

/*****************************************************************************/

void anim_once(int num)
{
  once = num;
}

/*****************************************************************************/

void anim_setcommand(char *str)
{
  if (!strlen(str))
  {
    if (command == NULL)
      return;
    free(command);
    command = NULL;
  }

  if (command != NULL)
    free(command);

  command = (char *) malloc( sizeof(char) * (strlen(str) + 1) );
  sprintf(command,"%s",str);
}

/*****************************************************************************/

void anim_load(char *in_glob)
{
  char **new_frames = NULL;

  if (in_glob == NULL)
    return;

  if (in_glob[0] == '\0')
    return;

  new_frames = glob(in_glob);

  if (new_frames == NULL)
    return;

  if (new_frames[0] != NULL)
  {
    fprintf(stdout,"(read geometry { define anim%d { LIST } } )\n", mypid);
    fprintf(stdout,"(geometry animate%d : anim%d)\n", mypid, mypid);
    fflush(stdout);

    clear_frames();
    load_frames(new_frames);
  }

  return;

}

/*****************************************************************************/

static char **get_script(char *in_scr)
{
  FILE *scriptfile;
  char *scrbuf, **result, ch = '\0';
  int   bufsize = 1000;
  int   position = 0, wait = 0, inword = 0;

  scrbuf = (char *) malloc(sizeof(char) * (bufsize + 1));

  if (!(scriptfile = fopen(in_scr, "r")))
  {
    fprintf(stderr, "Unable to open script file.\n");
    return NULL;
  }

#define IS_NEWLINE '\n'
#define IS_COMMENT '#'
#define IS_SPACE ' '
#define IS_TAB '\t'
#define IS_LB '{'
#define IS_COMMA ','
#define IS_RB '}'

  scrbuf[position] = IS_LB;
  position++;

  while ( !feof(scriptfile) )
  {

    ch = (char) fgetc(scriptfile);

    if (ch == IS_COMMENT)
      wait = 1;

    if (wait)
    {
      if (ch == IS_NEWLINE)
	wait = 0;
    }

    if (!wait)
    {
      if ((ch != IS_TAB ) && (ch != IS_NEWLINE) && (ch != IS_SPACE) &&
		!feof(scriptfile))
      {
	scrbuf[position] = ch;
	position++;

	if (position == (bufsize - 2))
	{
          bufsize *= 2;
	  scrbuf = (char *)realloc(scrbuf, sizeof(char) * (bufsize + 1));
        }
	inword = 1;
      }
      else
	if (inword)
	{
	  inword = 0;
	  scrbuf[position] = IS_COMMA;
	  position++;

	  if (position == (bufsize - 2))
	  {
            bufsize *= 2;
	    scrbuf = (char *)realloc(scrbuf, sizeof(char) * (bufsize + 1));
          }
	}
    }
  }

  scrbuf[position-1] = IS_RB;

  fclose(scriptfile);

  scrbuf[position] = '\0';	/* terminate buffer */
  result = glob(scrbuf);	/* glob it */
  free(scrbuf);			/* remove the buffer */

  return result;		/* return globbed list of files */
}

/*****************************************************************************/

void anim_loadscript(char *in_scr)
{
  char **new_frames = NULL;

  if (in_scr == NULL)
    return;

  if (in_scr[0] == '\0')
    return;

  new_frames = get_script(in_scr);

  if (new_frames == NULL)
    return;

  if (new_frames[0] != NULL)
  {
    fprintf(stdout,"(read geometry { define anim%d { LIST } } )\n", mypid);
    fprintf(stdout,"(geometry animate%d : anim%d)\n", mypid, mypid);
    fflush(stdout);

    clear_frames();
    load_frames(new_frames);
  }

  return;

}

/*****************************************************************************/

void anim_speed(int percent)
{
  /* for now we let the user interface handle the the speed of update */
}

/*****************************************************************************/
