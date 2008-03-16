/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#include <string.h>
#include <sys/stat.h>

#include "appearance.h"
#include "transobj.h"
#include "handleP.h"
#include "dbllist.h"

DBLLIST(AllLoadedTextures);

Texture *
TxFLoad(IOBFILE *inf, char *fname)
{
  Pool *p;
  Texture *tx = NULL;
  p = PoolStreamTemp(fname, inf, NULL, 0, &TextureOps);
  TxStreamIn(p, NULL, &tx);
  PoolDelete(p);
  return tx;
}

Texture *
TxFSave(Texture *tx, FILE *outf, char *fname)
{
  Pool *p;
  int ok;

  p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
  PoolSetOType(p, PO_DATA);
  PoolIncLevel(p, 1);
  ok = TxStreamOut(p, NULL, tx);
  PoolDelete(p);
  return ok ? tx : NULL;
}

void TxUpdateImage(Handle **hp, Ref *parentobj, Image **imgp)
{
  Texture *tx = (Texture *)parentobj;
    
  TxPurge(tx);
  HandleUpdRef(hp, parentobj, (Ref **)imgp);
}

static inline Texture *TxDefault(Texture *tx)
{
  memset((char *)tx, 0, sizeof(Texture));
  RefInit((Ref *)tx, TXMAGIC);
  TmIdentity(tx->tfm);
  DblListInit(&tx->loadnode);
  return tx;
}

Texture *
_TxSet(Texture *tx, int attr1, va_list *alist)
{
  int mask;
  Handle *h;
  Image  *img;
  TransformPtr f;
  int attr;
  char *str;
  bool do_purge = false;
  bool img_files = false;
  bool do_delete = false;

  if (tx == NULL) {
    tx = OOGLNewE(Texture, "TxCreate Texture");
    TxDefault(tx);
    do_delete = true;
  }

#define NEXT(type) va_arg(*alist, type)
	
  for ( attr = attr1; attr != TX_END; attr = NEXT(int)) {
    switch (attr) { /* parse argument list */
    case TX_DOCLAMP:
      mask = NEXT(int);
      tx->flags = (tx->flags & ~(TXF_SCLAMP|TXF_TCLAMP)) |
	(mask & (TXF_SCLAMP|TXF_TCLAMP));
      break;
    case TX_APPLY:
      mask = NEXT(int);
      if (mask < TXF_MODULATE || mask > TXF_REPLACE) {
	OOGLError(1, "TxSet: bad value for TX_APPLY: %d must be %d..%d",
		  mask, TXF_MODULATE, TXF_DECAL);
	goto nope;
      }
      tx->apply = (enum apply_enum)mask;
      break;
    case TX_FILE:
      str = NEXT(char *);
      if (str && tx->filename && strcmp(str, tx->filename) == 0) {
	break;
      }
      if (tx->filename) {
	OOGLFree(tx->filename);
      }
      tx->filename = str ? strdup(str) : NULL;
      do_purge = true;
      img_files = true;
      break;
    case TX_ALPHAFILE:
      str = NEXT(char *);
      if (str && tx->alphafilename &&
	  strcmp(str, tx->alphafilename) == 0) {
	break;
      }
      if (tx->alphafilename) {
	OOGLFree(tx->alphafilename);
      }
      tx->alphafilename = str ? strdup(str) : NULL;
      do_purge = true;
      img_files = true;
      break;
#if 0
    case TX_DATA:
      str = NEXT(char *);
      if (tx->data && str != tx->data)
	OOGLFree(tx->data);
      tx->data = str;
      if (str)
	tx->flags |= TXF_LOADED;
      break;
    case TX_XSIZE:
      mask = NEXT(int);
      if (mask != tx->xsize)
	tx->flags &= ~TXF_LOADED;
      tx->xsize = mask;
      break;
    case TX_YSIZE:
      mask = NEXT(int);
      if (mask != tx->ysize)
	tx->flags &= ~TXF_LOADED;
      tx->ysize = mask;
      break;
    case TX_CHANNELS:
      mask = NEXT(int);
      if (mask != tx->channels)
	tx->flags &= ~TXF_LOADED;
      tx->channels = mask;
      break;
#endif
#if 0 /* not implemented */
    case TX_COORDS:
      mask = NEXT(int);
      if (mask < TXF_COORD_GIVEN||mask > TXF_COORD_NORMAL) {
	OOGLError(1, "TxSet: bad value for TX_APPLY: %d must be %d..%d",
		  mask, TXF_MODULATE, TXF_DECAL);
	goto nope;
      }
      tx->coords = mask;
      break;
#endif
    case TX_BACKGROUND:
      tx->background = *NEXT(Color *);
      break;
    case TX_HANDLE_IMAGE:
      h   = NEXT(Handle *);
      img = NEXT(Image *);
      if (tx->imghandle) {
	HandlePDelete(&tx->imghandle);
      }
      tx->imghandle = REFGET(Handle, h);
      if (h) {
	HandleRegister(&tx->imghandle, (Ref *)tx, &tx->image,
		       TxUpdateImage);
	HandleSetObject(tx->imghandle, (Ref *)img);
      } else {
	tx->image = REFGET(Image, img);
      }
      do_purge = true;
      break;
    case TX_HANDLE_TRANSFORM:
      h = NEXT(Handle *);
      f = NEXT(TransformPtr);
      if (tx->tfmhandle) {
	HandlePDelete( &tx->tfmhandle );
      }
      tx->tfmhandle = REFGET(Handle, h);
      TmCopy(f, tx->tfm);
      if (h) {
	HandleRegister(&tx->tfmhandle,
		       (Ref *)tx, tx->tfm, TransUpdate);
      }
      break;
    default:
      OOGLError(1, "TxSet: unknown attribute %d", attr);
      goto nope;
    }
  }

  if (do_purge) {
    tx->flags &= ~TXF_LOADED;
    TxPurge(tx);
  }

  if (img_files) {
  }

  return tx;

 nope:
  if (do_delete) {
    TxDelete(tx);
  }
  return NULL;
}

Texture *
TxCreate(int a1, ...)
{
  va_list alist;
  Texture *tx;

  va_start(alist,a1);
  tx = _TxSet(NULL, a1, &alist);
  va_end(alist);

  return tx;
}

Texture *
TxSet(Texture *tx, int a1, ... )
{
  va_list alist;

  va_start(alist,a1);
  tx = _TxSet(tx, a1, &alist);
  va_end(alist);

  return tx;
}

TxUser *
TxAddUser(Texture *tx, int id, int (*needed)(TxUser *), void (*purge)(TxUser *))
{
  TxUser *tu = OOGLNew(TxUser);

  memset((void *)tu, 0, sizeof(TxUser));
  /* flags = 0, ctx = NULL, data = NULL */
  tu->tx = tx;
  tu->id = id;
  tu->needed = needed;
  tu->purge = purge;
  tu->next = tx->users;
  tx->users = tu;

  if (DblListEmpty(&tx->loadnode)) {
    /* Ensure this texture is on the AllLoadedTextures list. */
    DblListAdd(&AllLoadedTextures, &tx->loadnode);
  }
  return tu;
}

void TxRemoveUser(TxUser *tu)
{
  TxUser **tup;
  Texture *tx;

  if (tu == NULL) {
    return;
  }
  tx = tu->tx;
  for (tup = &tx->users; *tup != NULL; tup = &(*tup)->next) {
    if (tu == *tup) {
      *tup = tu->next;
      if (tu->purge) {
	(*tu->purge)(tu);
      }
      OOGLFree(tu);
      break;
    }
  }
  if (tx->users == NULL) {
    TxPurge(tx);
  }
}

/* Reclaim memory from texture, but don't discard essential stuff
 * Notify all users to purge their information. This will be their
 * only notice.
 */
void TxPurge(Texture *tx)
{
  /* Tell each user to get lost */
  TxUser *u, *nextu;

  DblListDelete(&tx->loadnode);

  for (u = tx->users; u != NULL; u = nextu) {
    nextu = u->next;
    if (u->purge) {
      (*u->purge)(u);
    }
    OOGLFree(u);
  }
  tx->users = NULL;
  tx->flags &= ~TXF_USED;
}

void
TxDelete(Texture *tx)
{
  if (tx == NULL) {
    return;
  }
  if (tx->magic != TXMAGIC) {
    OOGLWarn("Internal warning: TxDelete on non-Texture %x (%x != %x)",
	     tx, tx->magic, TXMAGIC);
    return;
  }
  if (RefDecr((Ref *)tx) > 0) {
    return;
  }

  TxPurge(tx);
  if (tx->filename) OOGLFree(tx->filename);
  if (tx->alphafilename) OOGLFree(tx->alphafilename);
  if (tx->tfmhandle) HandlePDelete(&tx->tfmhandle);
  if (tx->imghandle) HandlePDelete(&tx->imghandle);
  if (tx->image) ImgDelete(tx->image);

  OOGLFree(tx);
}

HandleOps TextureOps = {
  "texture",
  (int ((*)()))TxStreamIn,
  (int ((*)()))TxStreamOut,
  (void ((*)()))TxDelete,
  NULL,
  NULL,
};

static struct txkw {
  char *word;
  unsigned int aval; /* attribute or attribute value value for _TxSet() */
  int args;          /* > 0: number of following args,
		      * < 0: -number of following keywords determine
		      * attribute value.
		      */
} tx_kw[] = {
  { "texture",    0,                   1 },
  { "clamp",      TX_DOCLAMP,         -4 },
  {  "none",      0,                   0 },
  {  "s",         TXF_SCLAMP,          0 },
  {  "t",         TXF_TCLAMP,          0 },
  {  "st",        TXF_SCLAMP|TXF_TCLAMP, 0 },
  { "image",      TX_HANDLE_IMAGE,     0 },
  { "apply",      TX_APPLY,           -4 },
  {  "blend",     TXF_BLEND,           0 },
  {  "modulate",  TXF_MODULATE,        0 },
  {  "decal",     TXF_DECAL,           0 },
  {  "replace",   TXF_REPLACE,         0 },
  { "transform",  TX_HANDLE_TRANSFORM, 0 },  /*(s,t,r,q) . tfm = tx coords */
  { "background", TX_BACKGROUND,       3 },
  /* deprecated, maybe */
  { "file",       TX_FILE,             0 },
  { "alphafile",  TX_ALPHAFILE,        0 },
#if 0
  { "xsize",      TX_XSIZE,            1 },
  { "ysize",      TX_YSIZE,            1 },
  { "channels",   TX_CHANNELS,         1 },
#endif
  /********************/
#if 0 /* not implemented ... original comment: */
  /* Let's not implement this initially */
  { "coords",     TX_COORDS,          -4 },
  {  "given",     TXF_COORD_GIVEN,     0 },
  {  "local",     TXF_COORD_LOCAL,     0 },
  {  "camera",    TXF_COORD_CAMERA,    0 },
  {  "normal",    TXF_COORD_NORMAL,    0 },
#endif
};

#if SIZEOF_INO_T == 0
# undef SIZEOF_INO_T
# define SIZEOF_INO_T SIZEOF_LONG
#endif
#if SIZEOF_DEV_T == 0
# undef SIZEOF_DEV_T
# define SIZEOF_DEV_T SIZEOF_LONG
#endif
#if SIZEOF_TIME_T == 0
# undef SIZEOF_TIME_T
# define SIZEOF_TIME_T SIZEOF_LONG
#endif

#define INO_T_LSIZE (SIZEOF_INO_T+SIZEOF_INO_T-1)/SIZEOF_LONG
#define DEV_T_LSIZE (SIZEOF_DEV_T+SIZEOF_DEV_T-1)/SIZEOF_LONG
#define TIME_T_LSIZE (SIZEOF_TIME_T+SIZEOF_TIME_T-1)/SIZEOF_LONG
#define LONG_CHARS  (SIZEOF_LONG*2)

static inline int stat_to_handle(char *str, dev_t dev, ino_t ino, time_t time)
{
  union { ino_t ino; long  data[INO_T_LSIZE]; } ino_t_conv;
  union { dev_t dev; long  data[DEV_T_LSIZE]; } dev_t_conv;
  union { time_t time; long  data[TIME_T_LSIZE]; } time_t_conv;
  int i, cnt, len;

  strcpy(str, "::");
  str += (cnt = 2);
    
  ino_t_conv.ino = ino;
  for (i = 0; i < INO_T_LSIZE; i++) {
#if 0
    len = sprintf(str, "%0*lx", LONG_CHARS, ino_t_conv.data[i]);
#else
    len = sprintf(str, "%lx", ino_t_conv.data[i]);
#endif
    str += len;
    cnt += len;
  }

  *str++ = '@';
  cnt++;
  dev_t_conv.dev = dev;
  for (i = 0; i < DEV_T_LSIZE; i++) {
#if 0
    len = sprintf(str, "%0*lx", LONG_CHARS, dev_t_conv.data[i]);
#else
    len = sprintf(str, "%lx", dev_t_conv.data[i]);
#endif
    str += len;
    cnt += len;
  }

  *str++ = '@';
  cnt++;
  time_t_conv.time = time;
  for (i = 0; i < TIME_T_LSIZE; i++) {
#if 0
    len = sprintf(str, "%0*lx", LONG_CHARS, time_t_conv.data[i]);
#else
    len = sprintf(str, "%lx", time_t_conv.data[i]);
#endif
    str += len;
    cnt += len;
  }

  return cnt;
}

/* see the comments in src/lib/gprim/geom/geomstream.c */
int
TxStreamIn(Pool *p, Handle **hp, Texture **txp)
{
  IOBFILE *stream;
  char *fname;
  Handle *h = NULL;
  Texture *tx = NULL;
  float val[16];
  struct txkw *kw;
  char *w, *raww;
  int i, k = 0;
  int brack = 0;
  int empty = 1;
  bool braces = true;
  /*int plus = 0;*/
  bool more, mine = true; /* Questionable -- we'll report all errors */

  if ((stream = PoolInputFile(p)) == NULL) {
    return 0;
  }
  fname = PoolName(p);

  more = false;
  do {
    iobfnextc(stream, 0);

    switch(i = iobfgetc(stream)) {
    case ':':
    case '<':
      w = iobfdelimtok("{}()", stream, 0);
      /*
       * Consider doing a path search.
       * Do this before calling HandleReferringTo()
       * to prevent spurious error messages.
       */
      if (i == '<' && (h = HandleByName(w, &TextureOps)) == NULL && w[0] != '/') {
	w = findfile(fname, raww = w);
	if (w == NULL) {
	  OOGLSyntax(PoolInputFile(p),
		     "Error reading \"%s\": can't find file \"%s\"",
		     fname, raww);
	}
      } else if (h) {
	HandleDelete(h);
      }
      h = HandleReferringTo(i, w, &TextureOps, NULL);
      if (h != NULL) {
	tx = (Texture *)HandleObject(h);
	RefIncr((Ref*)tx);
      }
      break;

    case EOF: brack = 0; break;
    case '{': brack++; braces = true; break;
    case '}':
      if (brack-- <= 0) {
	iobfungetc(i, stream);
      }
      break;
    case '-':
    case '!':
      /*plus = -1;*/
      break;
    case '+':
      /*plus = 1;*/
      break;
    case '*':  break;

    default:
      more = false;
      iobfungetc(i, stream);
      w = iobfdelimtok("{}()", stream, 0);
      if (w == NULL) {
	break;
      }	    

      for (i = sizeof(tx_kw)/sizeof(tx_kw[0]), kw = tx_kw; --i >= 0; kw++)
	if (!strcmp(kw->word, w))
	  break;
      if (i < 0) {
	if (mine)
	  OOGLSyntax(stream, "%s: unknown texture keyword %s",
		     fname, w);
	return 0;
      }
      if (tx == NULL) {
	tx = TxCreate(TX_END);
      }

      if (kw->args < 0) {
	char allowed[256], *tail = allowed;
	w = iobfdelimtok("{}()", stream, 0);
	if (w == NULL) w = "";
	allowed[0] = '\0';
	for (k = 1; strcmp((kw+k)->word, w); k++) {
	  sprintf(tail, " %s", (kw+k)->word);
	  tail += strlen(tail);
	  if (k + kw->args >= 0) {
	    OOGLSyntax(stream, "%s: %s %s: expected one of: %s",
		       fname, kw->word, w, allowed);
	    TxDelete(tx);
	    return 0;
	  }
	}
      } else if (kw->args > 0) {
	int n = iobfgetnf(stream, kw->args, val, 0);
	if (n != kw->args) {
	  OOGLSyntax(stream, "%s: %s expected %d numeric values",
		     fname, w, kw->args);
	  TxDelete(tx);
	  return 0;
	}
      }

      empty++;

      switch((int)kw->aval) {
      case -1:
	mine = more = true;
	empty--;
	break;

      case TX_APPLY:
	tx->apply = (enum apply_enum)(kw+k)->aval;
	break;

      case TX_FILE:
      case TX_ALPHAFILE:
	raww = iobfdelimtok("{}()", stream, 0);
	w = findfile(fname, raww);
	if (w == NULL) {
	  OOGLSyntax(stream,
		     "Warning: reading \"%s\": can't find file \"%s\", ignoring texture",
		     fname, raww);
	} else {
	  TxSet(tx, kw->aval, w, TX_END);
	}
	break;

#if 0 /* does not belong here */
      case TX_XSIZE:
      case TX_YSIZE:
      case TX_CHANNELS:
	if (val[0] < 1 ||
	    val[0] > (tx_kw[i].aval==TX_CHANNELS) ? 4 : 100000) {
	  OOGLSyntax(stream, "%s: Bad value for %s: %s",
		     fname, kw->word, w);
	  TxDelete(tx);
	  return 0;
	}
	TxSet(tx, kw->aval, (int)val[0], TX_END);
	break;
#endif

#if 0 /* not implemented */
      case TX_COORDS:
	tx->coords = (kw+k)->aval;
	break;
#endif
      case TX_BACKGROUND: {
	/* We allow ColorA for compatibility, but the texture
	 * background color really is only RGB, not RGBA (see
	 * glTexEnvf(3)). So: if the next character is not a closing
	 * brace and not '\n', consume the next float which should be
	 * the alpha component
	 */
	float dummy;
	int c;

	if ((c = iobfnextc(stream, 1)) != '\n' && c != '}' && c != EOF) {
	  if (iobfgetnf(stream, 1, &dummy, 0) < 1) {
	    OOGLSyntax(stream, "%s: background color expected", fname);
	    TxDelete(tx);
	    return false;
	  }
	}
	TxSet(tx, kw->aval, val, TX_END);
	break;
      }
      case TX_HANDLE_IMAGE:
	if (!ImgStreamIn(p, &tx->imghandle, &tx->image)) {
	  OOGLSyntax(stream, "%s: texture image definition expected",
		     fname);
	  TxDelete(tx);
	  return false;
	}
	if (tx->filename) {
	  OOGLFree(tx->filename);
	  tx->filename = NULL;
	}
	if (tx->alphafilename) {
	  OOGLFree(tx->alphafilename);
	  tx->alphafilename = NULL;
	}
	if (tx->imghandle) {
	  HandleRegister(&tx->imghandle, (Ref *)tx, &tx->image,
			 TxUpdateImage);
	}
	break;
      case TX_HANDLE_TRANSFORM:
	if (!TransStreamIn(p, &tx->tfmhandle, tx->tfm)) {
	  OOGLSyntax(stream, "%s: 4x4 texture transform expected",
		     fname);
	  TxDelete(tx);
	  return false;
	}
	if (tx->tfmhandle) {
	  HandleRegister(&tx->tfmhandle, (Ref *)tx,
			 tx->tfm, TransUpdate);
	}
	break;
      case TX_DOCLAMP:
	tx->flags = (kw+k)->aval;
	break;


      default:
	break;
      }
      /*plus = 0;*/
    }
  } while (brack > 0 || more);

  /* handle file and alphafile constructs */
  if (h == NULL && tx->filename) {
    struct stat st;
    char hname[2*(4+(INO_T_LSIZE+DEV_T_LSIZE+TIME_T_LSIZE)*SIZEOF_LONG)+1];
    char *ptr;

    if (tx->imghandle) {
      HandlePDelete(&tx->imghandle);
      tx->imghandle = NULL;
    }
    if (tx->image) {
      ImgDelete(tx->image);
      tx->image = NULL;
    }
	
    if (stat(tx->filename, &st) < 0) {
      OOGLSyntax(stream, "%s: cannot stat file %s", fname, tx->filename);
      TxDelete(tx);
      return 0;
    }

    ptr = hname;
    ptr += stat_to_handle(ptr, st.st_dev, st.st_ino, st.st_mtime);

    if (tx->alphafilename) {
      if (stat(tx->alphafilename, &st) < 0) {
	OOGLSyntax(stream,
		   "%s: cannot stat file %s", fname, tx->filename);
	TxDelete(tx);
	return 0;
      }
      ptr += stat_to_handle(ptr, st.st_dev, st.st_ino, st.st_mtime);
    }
    /* we share texture images defined by the same files, as was
     * the previous behaviour. However, this is implemented using
     * references and handles to image objects.
     */
    tx->imghandle = HandleByName(hname, &ImageOps);
    if (tx->imghandle != NULL) {
      tx->image = REFGET(Image, HandleObject(tx->imghandle));
    } else {
      /* This means there is no image, create one */
      tx->image = tx->alphafilename
	? ImgCreate(IMG_DATA_CHAN_FILE,
		    IMGF_AUTO, NULL, tx->filename,
		    IMG_DATA_CHAN_FILE,
		    IMGF_ALPHA, NULL, tx->alphafilename,
		    IMG_END)
	: ImgCreate(IMG_DATA_CHAN_FILE,
		    IMGF_AUTO, NULL, tx->filename,
		    IMG_END);
      if (!tx->image) {
	OOGLSyntax(stream,
		   "%s: cannot create image from given file(s) "
		   "(\"%s\"/\"%s\"",
		   fname, tx->filename, tx->alphafilename);
	TxDelete(tx);
	return 0;
      }
      /* Generate a new reference */
      tx->imghandle = HandleAssign(hname, &ImageOps, (Ref *)tx->image);
      tx->imghandle->permanent = false;
    }
  }

  /* Pass the ownership of h and tx to the caller if requested */

  if (hp != NULL) {
    /* pass on ownership of the handle h to the caller of this function */
    if (*hp != NULL) {
      if (*hp != h) {
	HandlePDelete(hp);
      } else {
	HandleDelete(*hp);
      }
    }
    *hp = h;
  } else if (h) {
    /* Otherwise delete h because we are its owner. Note that
     * HandleReferringTo() has passed the ownership of h to us;
     * explicitly defined handles (hdefine and define constructs)
     * will not be deleted by this call.
     */
    HandleDelete(h);
  }

  /* same logic as for hp */
  if (txp != NULL) {
    if (*txp != NULL) {
      TxDelete(*txp);
    }
    *txp = tx;
  } else if(tx) {
    TxDelete(tx);
  }

  return (tx != NULL || h != NULL || (empty && braces));
}

int
TxStreamOut(Pool *p, Handle *h, Texture *tx)
{
  static const char *clamps[] = {"none", "s", "t", "st"};
  static const char *applies[] = {"modulate", "decal", "blend", "replace"};
  FILE *f = PoolOutputFile(p);

  (void)h;

  if (f == NULL) {
    return 0;
  }

  PoolFPrint(p, f, "texture {\n");
  PoolIncLevel(p, 1);
  PoolFPrint(p, f, "clamp %s\n", clamps[tx->apply & (TXF_SCLAMP|TXF_TCLAMP)]);
  PoolFPrint(p, f, "apply %s\n",
	     (unsigned int)tx->apply < COUNT(applies) ? applies[tx->apply]
	     : "???");
  PoolFPrint(p, f, "background %.8g %.8g %.8g\n",
	     tx->background.r, tx->background.g, tx->background.b);
  PoolFPrint(p, f, "");
  TransStreamOut(p, tx->tfmhandle, tx->tfm);
  if (tx->filename) {
    PoolFPrint(p, f, "file %s\n", tx->filename);
    if (tx->alphafilename) {
      PoolFPrint(p, f, "alphafile %s\n", tx->alphafilename);
    }
  } else {
    PoolFPrint(p, f, "");
    ImgStreamOut(p, tx->imghandle, tx->image);
  }
  PoolIncLevel(p, -1);
  PoolFPrint(p, f, "}\n");

  return !ferror(f);
}

Texture *
TxCopy(Texture *src, Texture *dst)
{
  Ref r;

  if (src == NULL) {
    if (dst) {
      TxDelete(dst);
    }
    return NULL;
  } else if (src == dst) {
    return dst;
  } else {
    if (dst == NULL) {
      dst = TxCreate(TX_END);
    } else {
      TxPurge(dst);
      if (dst->filename) OOGLFree(dst->filename);
      if (dst->alphafilename) OOGLFree(dst->alphafilename);
      if (dst->tfmhandle) HandlePDelete(&dst->tfmhandle);
      if (dst->imghandle) HandlePDelete(&dst->imghandle);
      if (dst->image) ImgDelete(dst->image);
    }
    r = *(Ref *)dst;
    *dst = *src;
    *(Ref *)dst = r;
    if (dst->filename) dst->filename = strdup(dst->filename);
    if (dst->alphafilename) dst->alphafilename = strdup(dst->alphafilename);
    if (dst->tfmhandle) dst->tfmhandle = REFGET(Handle, dst->tfmhandle);
    if (dst->imghandle) dst->imghandle = REFGET(Handle, dst->imghandle);
    if (dst->image) dst->image = REFGET(Image, dst->image);
    return dst;
  }
}

Texture *
TxMerge(Texture *src, Texture *dst, int mergeflags)
{
  (void)mergeflags;

  if (src == NULL)
    return REFGET(Texture, dst);
  if (dst == NULL) {
    return REFGET(Texture, src);
  }
  /* Oh, well.  XXX.  Leave real merging for later, if at all.
   * Meanwhile, any new texture completely replaces any old one.
   */
  return REFGET(Texture, src);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
