/* Copyright (C) 2007 Claus-Justus Heine
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

#include <limits.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/*#undef HAVE_LIBZ*/

#ifndef HAVE_LIBZ
# define HAVE_LIBZ 0
#endif

#if HAVE_LIBZ
# include <zlib.h>
#endif

#include "appearance.h"
#include "handleP.h"

#ifndef max
# define max(a, b) ((a) >= (b) ? (a) : (b))
#endif
#ifndef min
# define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

HandleOps ImageOps = {
  "img",
  (int ((*)(Pool *p, Handle **hp, Ref **rp)))ImgStreamIn,
  (int ((*)(Pool *p, Handle *h, Ref *r)))ImgStreamOut,
  (void ((*)(Ref *rp)))ImgDelete,
  NULL,
  NULL
};

struct imgheader {
  int xsize, ysize, channels, maxval;
  enum _format {
    IMGF_BYTE,
    IMGF_BIT,
    IMGF_ASCII,
    IMGF_SGIRLE,  /* SGI RLE image */
    IMGF_SGIRAW   /* for (z) for (y=bottom to top) { data } */
  } format;
  int *rleoff;   /* For IMGF_SGIRLE: offsets[z][y], then lengths[z][y] */
  char *rledata; /* For IMGF_SGIRLE: all data
		  * (don't fseek so we can read from pipe)
		  */
};

static bool readimage(Image *img, unsigned *chmask, char *filter,
		      char *imgfname, char *imgdata, int datalen);
static bool parseheader(Image *img, IOBFILE *imgf, unsigned *chmask,
			const char *type, struct imgheader *header);
static bool readdata(Image *img, IOBFILE *imgf, unsigned chmask,
		     struct imgheader *header);
static int run_filter(const char *filter, int fdin, bool wronly, int *cpidp);
static int data_pipe(const char *data, int datalen, int *cpidp);
#if HAVE_LIBZ
static int gv_compress2(Bytef *dest, uLongf *destLen,
			const Bytef *source, uLong sourceLen,
			int level);
#endif

Image *ImgDefault(Image *img)
{
  memset((char *)img, 0, sizeof(*img));
  RefInit((Ref *)img, IMGMAGIC);
  img->maxval = 255;
  return img;
}

Image *_ImgSet(Image *img, int attr1, va_list *alist)
{
  int attr, val;
  bool newimg;
  char *chanfile, *filter;
  unsigned chmask;
  void *chandata;
  int chansize = 0;
#undef NEXT
#define NEXT(type) va_arg(*alist, type)
	
  if (img == NULL) {
    /*
     * New Image created here.
     */
    img = OOGLNewE(Image, "ImgCreate Image");
    ImgDefault(img);
    newimg = true;
  }

  for (attr = attr1; attr != IMG_END; attr = NEXT(int)) {
    switch (attr) { /* parse argument list */
    case IMG_WIDTH:
      val = NEXT(int);
      if (val != img->width && img->data != NULL) {
	OOGLFree(img->data);
	img->data = NULL;
      }
      img->width = val;
      break;
    case IMG_HEIGHT:
      val = NEXT(int);
      if (val != img->height && img->data != NULL) {
	OOGLFree(img->data);
	img->data = NULL;
      }
      img->height = val;
      break;
    case IMG_CHANNELS:
      val = NEXT(int);
      if (val != img->channels && img->data != NULL) {
	OOGLFree(img->data);
	img->data = NULL;
      }
      img->channels = val;
      break;
    case IMG_MAXVAL:
      val = NEXT(int);
      if (val != img->maxval && img->data != NULL) {
	OOGLFree(img->data);
	img->data = NULL;
      }
      img->maxval = val;
      if (img->maxval != 255) {
	OOGLError(1, "ImgSet: "
		  "maxval is tied to 255, nothing else is implemented");	
	goto nope;
      }
      break;
    case IMG_DATA:
      if (img->data) {
	OOGLFree(img->data);
      }
      img->data = NEXT(char *);
      break;
    case IMG_DATA_CHAN_FILE:
    case IMG_DATA_CHAN_DATA:
      chmask   = NEXT(unsigned);
      filter   = NEXT(char *); /* can be NULL */
      if (attr == IMG_DATA_CHAN_FILE) {
	chanfile = NEXT(char *);
	chandata = NULL;
      } else {
	chandata = NEXT(void *);
	chansize = NEXT(int);
	chanfile = NULL;
      }
      if (chmask == IMGF_ALPHA) {
	if (img->channels == 0) {
	  OOGLError(1,
		    "ImgSet(): data: "
		    "don't know which channel is the alpha mask");
	  goto nope;
	}
	switch (img->channels) {
	case 1: case 2: chmask = 0x2; break;
	case 3: case 4: chmask = 0x8; break;
	}
      } else if (chmask == IMGF_AUTO) {
	chmask = 0;
      }
      /* try to suck it in, readimage() will examine filter
       * and the suffix of imgfname and the magic bytes at
       * the beginning of the image data. An explicitly
       * specified filter will take precedence.
       */
      if (!readimage(img, &chmask, filter, chanfile, chandata, chansize)) {
	goto nope;
      }
      break;
    default:
      OOGLError(1, "ImgSet: unknown attribute %d", attr);
      break;
    }
  }
  return img;
  
 nope:
  if (newimg) {
    ImgDelete(img);
  }
  return NULL;
  }

Image *ImgCreate(int a1, ...)
{
  va_list alist;
  Image *img;
    
  va_start(alist, a1);
  img = _ImgSet(NULL, a1, &alist);
  va_end(alist);

  return img;
}

void ImgDelete(Image *img)
{
  if (img == NULL) {
    return;
  }

  if (img->magic != IMGMAGIC) {
    OOGLWarn("Internal warning: ImgDelete on non-Imag %x (%x != %x)",
	     img, img->magic, IMGMAGIC);
    return;
  }
  if (RefDecr((Ref *)img) > 0) {
    return;
  }
  if (img->data) {
    OOGLFree(img->data);
  }
  OOGLFree(img);
}

Image *ImgSet(Image *img, int a1, ...)
{
  va_list alist;
    
  va_start(alist, a1);
  img = _ImgSet(img, a1, &alist);
  va_end(alist);

  return img;
}

/* see the comments in src/lib/gprim/geom/geomstream.c */
int ImgStreamIn(Pool *p, Handle **hp, Image **imgp)
{
  Handle *h = NULL;
  Handle *hname = NULL;
  Image *img = NULL;
  char *w, *raww = NULL;
  int c, i, val;
  int moredata;
  int brack = 0;
  IOBFILE *inf;
  char *fname, *imgfname = NULL, *filter, *imgdata = NULL;
  long datasize = 0;
  unsigned long chmask;
  bool have_chdata;
  static const struct imgkw {
    const char *word;
    int aval;
    int nargs;
    int min;
    int max;
  } imgkw[] = { 
    { "width", IMG_WIDTH, 1, 1, 0 },
    { "height", IMG_HEIGHT, 1, 1, 0 },
    { "channels", IMG_CHANNELS, 1, 1, 4 },
    { "maxval", IMG_MAXVAL, 1, 255, 255 },
    { "data", IMG_DATA, -1, 1, 0 } /* data chmask filter DATA */
  };
  const struct imgkw *kw;
  static const struct chflkw {
    const char *word;
    int value;
  } chflkw[] = {
    { "LUMINANCE", IMGF_LUMINANCE },
    { "LUMINANCE_ALPHA", IMGF_LUMINANCE_ALPHA },
    { "RGB", IMGF_RGB },
    { "RGBA", IMGF_RGBA },
    { "ALPHA", IMGF_ALPHA },
    { "AUTO", IMGF_AUTO }
  };
  const struct chflkw *chfl;
#undef N_ITEMS
#define N_ITEMS(array) (sizeof(array)/sizeof(*(array)))

  if (p == NULL || (inf = PoolInputFile(p)) == NULL) {
    return 0;
  }
  fname = PoolName(p);

  have_chdata = false;

  do {
    switch(c = iobfnextc(inf, 0)) {
    case '{': brack++; iobfgetc(inf); break;
    case '}': if (brack--) iobfgetc(inf); break;
    case '<':
    case ':':
    case '@':
      iobfgetc(inf);
      w = iobfdelimtok("{}()", inf, 0);
      if (c == '<' && (h = HandleByName(w, &ImageOps)) == NULL && w[0] != '/') {
	w = findfile(PoolName(p), raww = w);
	if (w == NULL) {
	  OOGLSyntax(inf,
		     "Reading image from \"%s\": "
		     "can't find file \"%s\"",
		     PoolName(p), raww);
	  return false;
	}
      } else if (h) {
	/* HandleByName() increases the ref. count s.t. the
	 * caller of HandleByName() owns the returned handle.
	 */
	HandleDelete(h);
      }
      h = HandleReferringTo(c, w, &ImageOps, NULL);
      if (h) {
	img = (Image *)HandleObject(h);
	/* Increment the ref. count. This way we can call
	 * HandleDelete() and ImgDelete() independently.
	 */
	REFGET(Image, img);
      }
      break;

    default:
      w = iobfdelimtok("{}()", inf, 0);
      if (w == NULL) {
	break;
      }
      if (strcmp(w, "define") == 0) {
	hname = HandleCreateGlobal(iobftoken(inf, 0), &ImageOps);	
	break;
      }
      for (i = N_ITEMS(imgkw), kw = imgkw; --i >= 0; kw++) {
	if (!strcmp(kw->word, w)) {
	  break;
	}
      }
      if (i < 0) {
	OOGLSyntax(inf, "%s: unknown image keyword %s", fname, w);
	ImgDelete(img);
	return false;
      }
      if (img == NULL) {
	img = ImgCreate(IMG_END);
      }

      if (kw->nargs == 1) {
	int val, n;

	if (have_chdata) {
	  OOGLSyntax(inf,
		     "%s: %s must be specified before defining the image data",
		     fname, w);
	  ImgDelete(img);
	  return false;
	}

	n = iobfgetni(inf, kw->nargs, &val, 0);
	if (n != kw->nargs) {
	  OOGLSyntax(inf, "%s: %s expected %d numeric values",
		     fname, w, kw->nargs);
	  ImgDelete(img);
	  return false;
	}
	if (kw->min <= kw->max && (val < kw->min || val > kw->max)) {
	  OOGLSyntax(inf, "%s: %s value %d out of range (min %d, max %d",
		     fname, w, val, kw->min, kw->max);
	}
	ImgSet(img, kw->aval, val, IMG_END);
      } else {
	/* data CHANNELMASK [FILTER] [{] < FILENAME [}]
	 *
	 * or
	 *
	 * data CHANNELMASK [FILTER] DATASIZE [{]\nDATA ... [}]
	 *
	 * FILTER may be a program or one of the know types, i.e. one
	 * of raw, png, tif[f], jpeg, gif, gzip, bzip2, compress.  If
	 * FILTER is a program, then it must convert FILENAME to pnm
	 * and the output must be presented at stdout. If no type is
	 * specified, then the data must bin SGIRAW or SGIRLE or PNM
	 * format. These formats are autodetected. If no SGIRAW,
	 * SGIRLE or PNM format could be detected, then "raw" is
	 * assumed.
	 *
	 * If the FILENAME has a known suffix (Z, z, gz, bz2,
	 * raw, tiff, tif, png, gif) then the input filter is
	 * deduced from the suffix.
	 *
	 * DATASIZE _must_ be specified for embedded images and gives
	 * the size of the embedded data.
	 */
	w = iobfdelimtok("{}()", inf, 0);
	if (w == NULL) {
	  OOGLSyntax(inf, "%s: data: expected the channel mask", fname);
	  ImgDelete(img);
	  return false;
	}
	for (i = N_ITEMS(chflkw), chfl = chflkw; --i >= 0; chfl++) {
	  if (!strcmp(chfl->word, w)) {
	    chmask = chfl->value;
	    break;
	  }
	}
	if (i < 0) {
	  /* should be a numerical value */
	  chmask = strtol(w, NULL, 0);
	  if (datasize == LONG_MIN || datasize == LONG_MAX) {
	    OOGLSyntax(inf, "%s: data: bogus channel mask definition: \"%s\"",
		       fname, w);
	    ImgDelete(img);
	    return false;
	  }
	  chmask &= 0xf;
	}

	imgfname = NULL;
	filter   = NULL;
	moredata = 0;
	datasize = -1;
	do {
	  switch(c = iobfnextc(inf, 1)) {
	  case '{':
	    /* allow brackets around data and file-name, i.e.
	     * data MASK [FILTER] { < FILENAME }
	     * or
	     * data MASK [FILTER] DATASIZE {\n
	     * data bytes ...
	     * }
	     */
	    brack++;
	    iobfgetc(inf);
	    moredata = 1;
	    break;
	  case '<': /* read from a file, find and open it */
	    iobfgetc(inf); /* consume '<' */
	    w = iobfdelimtok("{}()", inf, 0);
	    imgfname = findfile(fname, w);
	    if (imgfname == NULL) {
	      OOGLSyntax(inf,
			 "Warning: reading \"%s\": can't find file \"%s\"",
			 fname, raww);
	      ImgDelete(img);
	      return false;
	    }
	    moredata = 0;
	    break;
	  case '\n': /* read embedded data from _this_ file */
	    if (datasize < 0) {
	      /* this means that the "filter" token is actually the
	       * size of the data section
	       */
	      if (filter == NULL) {
		OOGLSyntax(inf,
			   "%s: the size of the embedded image data is missing",
			   fname);
		ImgDelete(img);
		return false;
	      }
	      datasize = strtol(filter, NULL, 0);
	      if (datasize == LONG_MIN || datasize == LONG_MAX) {
		OOGLSyntax(inf, "%s: bogus size definition: \"%s\"",
			   fname, filter);
		ImgDelete(img);
		return false;
	      }
	      filter = NULL;
	    }
	    iobfgetc(inf); /* consume '\n', image data follows */

	    /* read all the stuff in, we do it here to maintain the
	     * file position
	     */
	    imgdata = OOGLNewNE(char, datasize, "embedded image data");
	    if (iobfread(imgdata, datasize, 1, inf) <= 0) {
	      OOGLError(0, "%s: can't read embedded image data", fname);
	      OOGLFree(imgdata);
	      ImgDelete(img);
	      return false;
	    }
	    moredata = 0;
	    break;
	  default:
	    /* assume that it is a filter definition _or_ the data-size. */
	    if (filter) {
	      /* must be the data-size */
	      if (iobfgetni(inf, 1, &val, 0) != 1) {
		OOGLSyntax(inf, "%s: expected the data size", fname);
		ImgDelete(img);
		return false;
	      }
	      datasize = val;
	    } else {
	      /* maybe a filter of the data size, decide later */
	      filter = iobfdelimtok("{}()", inf, 0);
	      if (filter == NULL) {
		OOGLSyntax(inf, "%s: expected an input filter or "
			   "the size of the data-section",
			   fname);
		ImgDelete(img);
		return false;
	      }
	      /* we leave it to readimage() to determine
	       * whether this is a valid filter program, or
	       * prefix, file type spec.
	       */
	      moredata = 1;
	    }
	    break;
	  }
	} while (moredata);

	if ((imgfname
	     ? ImgSet(img, IMG_DATA_CHAN_FILE,
		      chmask, filter, imgfname, IMG_END)
	     : ImgSet(img, IMG_DATA_CHAN_DATA,
		      chmask, filter, imgdata, (int)datasize, IMG_END))
	    == NULL) {
#if 1
	  OOGLSyntax(inf, "%s: unable to read data for channelmask 0x%x, "
		     "continuing anyway", fname, (int)chmask);
#else
	  ImgDelete(img);
	  if (imgdata) { /* free buffer for embedded data */
	    OOGLFree(imgdata);
	  }
	  return false;
#endif
	}
		
	if (imgdata) { /* free buffer for embedded data */
	  OOGLFree(imgdata);
	  imgdata = NULL;
	} else {
	  imgfname = NULL;
	}

	have_chdata = true;
      }
      break;
    }
  } while (brack);
  
  if (hname != NULL) {
    if (img) {
      HandleSetObject(hname, (Ref *)img);
    }
    if (h) {
      /* HandleReferringTo() has passed the ownership to us, so delete
       * h because we do not need it anymore.
       */
      HandleDelete(h);
    }
    h = hname;
  }

  /* Pass the ownership of h and img to the caller if requested */

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
  if (imgp != NULL) {
    if (*imgp != NULL) {
      ImgDelete(*imgp);
    }
    *imgp = img;
  } else if(img) {
    ImgDelete(img);
  }

  return (h != NULL || img != NULL);
}

/* We do not record the file-name, so we can only output raw image
 * data. What format to choose? Highly compressed would be best; we
 * choose "gzip --best" or "bzip --best" if available; the
 * alpha-channel is output as pgm and the image data as RGB pnm
 * (channels == 4) or also as pgm (channels == 2).
 */
int ImgStreamOut(Pool *p, Handle *h, Image *img)
{
  FILE *f = PoolOutputFile(p);
  char *obuf;
  int olen;

  if (f == NULL ||
      (img == NULL &&
       (h == NULL || (img = (Image *)HandleObject(h)) == NULL))) {
    return false;
  }

  fprintf(f, "image {\n");
  PoolIncLevel(p, 1);
  if (PoolStreamOutHandle(p, h, img != NULL)) {
    PoolFPrint(p, f, "width %d\n", img->width);
    PoolFPrint(p, f, "height %d\n", img->height);
    PoolFPrint(p, f, "channels %d\n", img->channels);
    PoolFPrint(p, f, "maxval %d\n", img->maxval);
    /* For convenience we dump the stuff in pnm/pgm format with separate
     * alpha channel.
     */
    switch (img->channels) {
    case 2: /* two greymaps, luminance and alpha */
#if 1
      olen = ImgWritePAM(img, 0x3, HAVE_LIBZ, &obuf);
      PoolFPrint(p, f, "data LUMINANCE_ALPHA %s%d {\n",
		 HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      break;
#else
      olen = ImgWritePGM(img, 1, HAVE_LIBZ, obuf);
      PoolFPrint(p, f, "data ALPHA %s%d {\n", HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      /* fall through to luminance only */
#endif
    case 1: /* greymap, luminance */
      olen = ImgWritePGM(img, 0, HAVE_LIBZ, &obuf);
      PoolFPrint(p, f,
		 "data LUMINANCE %s%d {\n", HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      break;
    case 4: /* RGBA */
#if 1
      olen = ImgWritePAM(img, 0xf, HAVE_LIBZ, &obuf);
      PoolFPrint(p, f, "data RGBA %s%d {\n", HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      break;
#else
      olen = ImgWritePGM(img, 3, HAVE_LIBZ, &obuf);
      PoolFPrint(p, f, "data ALPHA %s%d {\n", HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      /* fall through to RGB */
#endif
    case 3: /* RGB only */
      olen = ImgWritePNM(img, 0x07, HAVE_LIBZ, &obuf);
      PoolFPrint(p, f, "data RGB %s%d {\n", HAVE_LIBZ ? "gzip " : "", olen);
      fwrite(obuf, olen, 1, f);
      fprintf(f, "\n");
      PoolFPrint(p, f, "}\n");
      OOGLFree(obuf);
      break;
    }
  }
  PoolIncLevel(p, -1);
  PoolFPrint(p, f, "}\n");
  return !ferror(f);
}
	
Image *ImgFSave(Image *img, FILE *outf, char *fname)
{
  Pool *p;
  int ok;

  p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
  PoolSetOType(p, PO_DATA);
  PoolIncLevel(p, 1);
  ok = ImgStreamOut(p, NULL, img);
  PoolDelete(p);
  return ok ? img : NULL;
}

Image *ImgFLoad(IOBFILE *inf, char *fname)
{
  Pool *p;
  Image *img = NULL;

  p = PoolStreamTemp(fname, inf, NULL, 0, &ImageOps);
  ImgStreamIn(p, NULL, &img);
  PoolDelete(p);
  return img;
}

/* Compress the contents of *buffer, free *buffer and return the
 * compressed buffer in a newly allocated chunk pointed to by
 * *buffer. Return the amount of compressed data.
 */
static inline int maybe_compress_buffer(char **buffer, int n_bytes)
{
#if HAVE_LIBZ
# if !HAVE_COMPRESSBOUND
#  define compressBound(slen) ((slen) + ((slen) >> 12) + ((slen) >> 14) + 11)
# endif
  char *bufptr;
  unsigned long c_n_bytes;

  /* we do it inefficient and quick and dirty: we simply call the
   * compress() function and compress the entire buffer
   */
  bufptr = *buffer;
  c_n_bytes = compressBound(n_bytes);
  *buffer = OOGLNewNE(char, c_n_bytes, "compressed buffer");
  if (gv_compress2((Bytef *)*buffer, &c_n_bytes,
		   (Bytef *)bufptr, n_bytes, 9) != Z_OK) {
    OOGLFree(*buffer);
    *buffer = bufptr;
  } else {
    OOGLFree(bufptr);
    n_bytes = c_n_bytes;
  }
#endif
  return n_bytes;
}

#define PNM_HEADER_LEN (2 + 1 + 10 + 1 + 10 + 1 + 5 + 1)
/* P5 1000000000 1000000000 65535\n */

/* Pack one channel of the image data into an PGM image and write that
 * data to *buffer. *buffer is allocated in this function, its length
 * is returned. If channel >= img->channel, then the result will be an
 * all-black PGM image. Optionally compress the image.
 */
int ImgWritePGM(Image *img, int channel, bool compressed, char **buffer)
{
  int row, col, stride, rowlen, depth;
  unsigned long n_bytes, h_len;
  char *imgptr, *bufptr;

  depth   = (img->maxval > 255) + 1;
  rowlen  = depth * img->width;
  n_bytes = rowlen * img->height + PNM_HEADER_LEN;
  bufptr  = *buffer = OOGLNewNE(char, n_bytes, "PGM buffer");

  bufptr += h_len =
    sprintf(*buffer, "P5 %d %d %d\n", img->width, img->height, img->maxval);
  n_bytes -= PNM_HEADER_LEN - h_len;
  
  if (channel >= img->channels) {
    memset(*buffer, 0, n_bytes);
  } else {
    stride = img->channels * depth;
    for (row = img->height-1; row >= 0; row--) {
      imgptr = img->data + channel + rowlen * img->channels * row;
      for (col = 0; col < img->width; col++) {
	*bufptr++ = *imgptr;
	if (depth == 2) {
	  *bufptr++ = *(imgptr+1);
	}
	imgptr += stride;
      }
    }
  }

  return compressed ? maybe_compress_buffer(buffer, n_bytes) : n_bytes;
}

/* Pack up to 3 channels of the image data into an PNM image and write
 * that data to *buffer. *buffer is allocated in this functions, its
 * length is returned. Missing channels are filled with 0's.
 *
 * Optionally compress the image.
 */
int ImgWritePNM(Image *img, unsigned chmask, bool compressed, char **buffer)
{
  int row, col, stride, depth, rowlen;
  unsigned long n_bytes, h_len;
  int channels[3], i, j;
  char *imgptr, *bufptr;

  depth   = (img->maxval > 255) + 1;
  rowlen  = 3 * depth * img->width;
  n_bytes = rowlen * img->height + PNM_HEADER_LEN;

  bufptr = *buffer = OOGLNewNE(char, n_bytes, "PNM buffer");

  channels[0] = channels[1] = channels[2] = -1;
  for (i = j = 0; i < img->channels && j < 3 && chmask; i++, chmask >>= 1) {
    if (chmask & 1) {
      channels[j++] = i;
    }
  }

  bufptr += h_len =
    sprintf(*buffer, "P6 %d %d %d\n", img->width, img->height, img->maxval);
  n_bytes -= PNM_HEADER_LEN - h_len;
  
  imgptr = img->data;
  stride = img->channels * depth;
  for (row = img->height-1; row >= 0; row--) {
    imgptr = img->data + stride * img->width * row;
    for (col = 0; col < img->width; col++) {
      for (j = 0; j < 3; j++) {
	if (channels[j] >= 0) {
	  for (i = 0; i < depth; i++) {
	    *bufptr++ = *(imgptr + channels[j] + i);
	  }
	} else {
	  for (i = 0; i < depth; i++) {
	    *bufptr++ = '\0';
	  }
	}
      }
      imgptr += stride;
    }
  }

  return compressed ? maybe_compress_buffer(buffer, n_bytes) : n_bytes;
}

/* Pack any number of channels into a PAM image. Optionally compress
 * the image. Excess bits set in chmask are ignored. The destination
 * image will have min(img->channels, #chmask bits) channels (i.e. at
 * most 4).
 *
 * The destination image is written into memory, allocated in
 * *buffer. The amount of data in *buffer is returned (size after
 * compression).
 */

/* worst case header length for our case */
#define PAM_HEADER_LEN				\
  sizeof("P7\n"					\
	 "WIDTH 1000000000\n"			\
	 "HEIGHT 1000000000\n"			\
	 "DEPTH 4\n"				\
	 "MAXVAL 65535\n"			\
	 "ENDHDR\n")

int ImgWritePAM(Image *img, unsigned chmask, bool compressed, char **buffer)
{
  int row, col, stride, depth, rowlen;
  unsigned long n_bytes, h_len;
  int channels[4], n_chan, i, j;
  char *imgptr, *bufptr;

  for (i = n_chan = 0; i < img->channels && chmask; i++, chmask >>= 1) {
    if (chmask & 1) {
      channels[n_chan++] = i;
    }
  }

  depth   = (img->maxval > 255) + 1; /* #bytes for a single channel */
  rowlen  = n_chan * depth * img->width;
  n_bytes = rowlen * img->height + PAM_HEADER_LEN;

  bufptr = *buffer = OOGLNewNE(char, n_bytes, "PAM buffer");

  bufptr += h_len =
    sprintf(*buffer,
	    "P7\nWIDTH %d\nHEIGHT %d\nDEPTH %d\nMAXVAL %d\nENDHDR\n",
	    img->width, img->height, n_chan, img->maxval);
  n_bytes -= PAM_HEADER_LEN - h_len;
  
  imgptr = img->data;
  stride = img->channels * depth;
  for (row = img->height-1; row >= 0; row--) {
    imgptr = img->data + stride * img->width * row;
    for (col = 0; col < img->width; col++) {
      for (j = 0; j < n_chan; j++) {
	for (i = 0; i < depth; i++) {
	  *bufptr++ = *(imgptr + channels[j] + i);
	}
      }
      imgptr += stride;
    }
  }

  return compressed ? maybe_compress_buffer(buffer, n_bytes) : n_bytes;
}

/* Dump an image to disk through the given filter. The filter must
 * read from stdin. The output of the filter is discarded, it is
 * assumed that "filter" writes the data to disk or to whatever
 * destination itself.
 * 
 * This is mainly to support textures with the RenderMan MG backend.
 *
 * The image filter must understand PAM image data if chmask has 2 or
 * 4 bits set, otherwise it must understand PNM data.
 *
 * This routine is ugly: we simply convert the image data into a PGM,
 * PNM or PAM image in memory using ImgWritePGM/PNM/PAM() and then
 * pipe the resulting data with data_pipe() to the stdin of the
 * filter.
 *
 * E.g.:
 *
 * ImgWriteFilter(img, 0xf, "pamtotiff -lzw -truecolor > img.tiff");
 *
 */
bool ImgWriteFilter(Image *img, unsigned chmask, const char *filter)
{
  int n_chan, buflen;
  unsigned mask;
  char *buf = NULL;
  int data_fd = -1;
  int data_pid = -1, filter_pid = -1;
  int result = false;
  void (*old_sigchld)(int);

  for (n_chan = 0, mask = chmask; mask; n_chan += mask & 1, mask >>= 1);
  n_chan = min(img->channels, n_chan);

  switch (n_chan) {
  case 1:
    buflen = ImgWritePGM(img, 0, false, &buf);
    break;
  case 3:
    buflen = ImgWritePNM(img, chmask, false, &buf);
    break;
  case 2:
  case 4:
    buflen = ImgWritePAM(img, chmask, false, &buf);
    break;
  }
  if ((data_fd = data_pipe(buf, buflen, &data_pid)) <= 0) {
    OOGLError(1, "ImgWriteFilter(): unable to generate data pipe");
    goto out;
  }
  if (run_filter(filter, data_fd, true, &filter_pid) < 0) {
    OOGLError(1, "ImgWriteFilter(): unable to run image filter");
    goto out;
  }
  result = true;
 out:
  if (buf) {
    OOGLFree(buf);
  }
  if (data_fd) {
    close(data_fd);
  }
  /* This could cause problems when an emodule exits during the short
   * period of SIGDFLT being in action ...
   */
  signal(SIGCHLD, old_sigchld = signal(SIGCHLD, SIG_DFL));
  if (old_sigchld != SIG_DFL &&
      old_sigchld != SIG_IGN &&
      old_sigchld != SIG_ERR) {
    /* ... send ourselves a SIGCHLD in this case. */
    kill(getpid(), SIGCHLD);
  } else {
    /* possibly wait for the filter processes. */
    while (filter_pid != -1 || data_pid != -1) {
      int cpid, status;

      cpid = wait(&status);
      if (cpid == filter_pid) {
	filter_pid = -1;
      } else if (cpid == data_pid) {
	data_pid = -1;
      }
      if (cpid == -1) {
	break;
      }
    }
  }
  return result;
}

/* reading of images */

struct filter 
{
  struct filter *next;
  const char *program;
  const char *suffixes[10];
};

static struct filter decompressors[] = {
  { decompressors+1, "gzip -dc", { "z", "gz", "gzip", NULL } },
  { NULL, "bzip2 -dc", { "bz2", "bzip2", NULL } },
};

static struct filter converters[] = {
  { converters+1, "tifftopnm", { "tiff", "tif", NULL } },
  { converters+2, "pngtopnm", { "png", NULL } },
  { converters+3, "giftoppm", { "gif", NULL } },
  { NULL, "jpegtopnm", { "jpeg", "jpg", NULL } },
};

static bool readimage(Image *img, unsigned *chmask, char *filtertype,
		      char *imgfname, char *imgdata, int datalen)
{
  struct filter *filters[2] = { decompressors, converters }, *filter;
  int filterpids[2] = { -1, -1 }, datapid = -1;
  int filterfds[2] = { -1, -1 };
  IOBFILE *imgf = NULL;
  int imgfd = -1;
  char *suffix, *explicit_filter = NULL;
  struct imgheader header;
  int i, j;
  bool result = true;
  void (*old_sigchld)(int);

  if (imgfname) {
    if ((imgfd = open(imgfname, O_RDONLY)) == -1) {
      OOGLError(0, "can't open file \"%s\"", imgfname);
      return false;
    }
  } else {
    /* What to do with imgdata? If we do not need a filter, then we
     * could just use the data-block we have already read in. If we
     * need a filter, then we fork a child that pipes imgdata to the
     * filter (in order to prevent deadlocks).
     */
  }
  
  if (filtertype == NULL) {
    filtertype = strdup(imgfname);
  } else {
    filtertype = strdup(filtertype);
    explicit_filter = filtertype;
  }

  for (j = 0; j < 2; j++) {
    if (filtertype == NULL || (suffix = strrchr(filtertype, '.')) == NULL) {
      suffix = filtertype;
    } else {
      suffix++; /* advance past '.' */
    }
    if (suffix) {
      for (filter = filters[j]; filter != NULL; filter = filter->next) {
	for (i = 0; filter->suffixes[i] != NULL; i++) {
	  if (strcasecmp(suffix, filter->suffixes[i]) == 0) {
	    if (imgfd == -1) {
	      imgfd = data_pipe(imgdata, datalen, &datapid);
	      if (imgfd < 0) {
		result = false;
		goto out;
	      }
	    }
	    filterfds[j] =
	      run_filter(filter->program, imgfd, false, &filterpids[j]);
	    if (filterfds[j] < 0) {
	      result = false;
	      goto out;
	    }
	    if (suffix != filtertype) {
	      suffix--;
	    }
	    *suffix = '\0'; /* mark this part as handled */
	    break;
	  }
	}
	if (filterfds[j] >= 0) {
	  close(imgfd);
	  imgfd = filterfds[j];
	  break;
	}
      }
    }
  }
  /* try to run any explicitly specified filter, but omit the dummy
   * filter "raw", which just means to interprete the stuff literally.
   */
  if (filterfds[1] < 0 && explicit_filter &&
      *explicit_filter != '\0' && strcasecmp(suffix, "raw") != 0) {
    /* explicitly specified filter */
    filterfds[1] = run_filter(explicit_filter, imgfd, false, &filterpids[1]);
    if (filterfds[1] < 0) {
      result = false;
      goto out;
    } else {
      close(imgfd);
      imgfd = filterfds[1];
    }
  }

  /* at this point we assume that reading from imgfd will give us raw,
   * SGI or PNM data. We generate an IOBFILE, simply because we want
   * to be able to use those iobfgetni() etc. functions, and because
   * parseheader() might want to set a file-marker.
   */
  if (imgfd == -1) {
    if ((imgf = iobfileopen(fmemopen(imgdata, datalen, "r"))) == NULL) {
      result = false;
      goto out;
    }
  } else {
    if ((imgf = iobfileopen(fdopen(imgfd, "r"))) == NULL) {
      result = false;
      goto out;
    }
  }

  if (filtertype == NULL || (suffix = strrchr(filtertype, '.')) == NULL) {
    suffix = filtertype;
  } else {
    suffix++; /* advance past '.' */
  }
  if (!parseheader(img, imgf, chmask, suffix, &header)
      ||
      !readdata(img, imgf, *chmask, &header)) {
    result = false;
    goto out;
  }

 out:
  free(filtertype); /* undo strdup() */

  /* This could cause problems when an emodule exits during the short
   * period of SIGDFLT being in action ...
   */
  signal(SIGCHLD, old_sigchld = signal(SIGCHLD, SIG_DFL));
  if (old_sigchld != SIG_DFL &&
      old_sigchld != SIG_IGN &&
      old_sigchld != SIG_ERR) {
    /* ... send us a SIGCHLD in this case. */
    kill(getpid(), SIGCHLD);
  } else {
    /* possibly wait for the filter processes. */
    while (filterpids[0] != -1 || filterpids[1] != -1 || datapid != -1) {
      int cpid, status;

      cpid = wait(&status);
      for (j = 0; j < 2; j++) {
	if (cpid == filterpids[j]) {
	  filterpids[j] = -1;
	}
      }
      if (cpid == datapid) {
	datapid = -1;
      }
      if (cpid == -1) {
	break;
      }
    }
  }

  if (imgf) {
    iobfclose(imgf);
  }

  return result;
}

static bool
parseheader(Image *img, IOBFILE *imgf, unsigned *chmask,
	    const char *type, struct imgheader *header)
{
  char scratch[1024];
  char *w;
  int chmask_channels;
  unsigned mask;
  int i, c;
  char *msg = NULL;
  IOBFILE *f = NULL;

  header->rleoff = NULL;
  header->rledata = NULL;

  chmask_channels = 0;
  for (i = 0, mask = *chmask; i < 4 && mask; i++, mask >>= 1) {
    if (mask & 1) {
      chmask_channels++;
    }
  }

  if (type && strcasecmp(type, "raw") == 0) {
    if (*chmask == IMGF_AUTO) {
      OOGLError(1, "parseheader(): "
		"chmask == AUTO cannot work with raw image data");
      return false;
    }
    header->format   = IMGF_BYTE;
    header->xsize    = img->width;
    header->ysize    = img->height;
    header->maxval   = img->maxval ? img->maxval : 255;
    header->channels = chmask_channels;
  } else {
    if (iobfsetmark(imgf) < 0) {
      OOGLError(1, "parseheader(): cannot set file mark");
      return false;
    }
    c = iobfgetc(imgf);
    if (c == 0x01 && (c = iobfgetc(imgf)) == 0xDA) {
      /* SGI image file */
      short shorts[3];
      int storage = iobfgetc(imgf);
      int bpp = iobfgetc(imgf);
      if (bpp != 1) {
	msg = "%s: must have 8-bit image values";
	goto nope;
      }
      iobfgetc(imgf); iobfgetc(imgf);		/* Skip "dimension" */
      iobfgetns(imgf, 3, shorts, 1);	/* Read big-endian 16-bit values */
      header->xsize    = shorts[0];
      header->ysize    = shorts[1];
      header->channels = shorts[2];
      header->maxval   = 255;
      if (iobfread(scratch, 4+4+492, 1, imgf) <= 0) {
	/* Skip min, max, pad data */
	msg = "can't consume pad data";
	goto nope;
      }
      header->format = (storage == 0x01) ? IMGF_SGIRLE : IMGF_SGIRAW;
      if (header->format == IMGF_SGIRLE) {
	/* Inhale offset&length table */
	int n = header->ysize * header->channels;
	int max = 0;

	header->rleoff = OOGLNewNE(int, n*2, "IMGF_SGIRLE offsets");
	msg = "%s: can't read RLE offsets";
	if (iobfgetni(imgf, n*2, header->rleoff, 1) != n*2) {
	  goto nope;
	}
	if (iobftell(imgf) < 0) {
	  for (i = 0; i < n; i++) {
	    if (max < header->rleoff[i]) {
	      max = header->rleoff[i] + header->rleoff[i+n];
	    }
	  }
	  header->rledata = OOGLNewNE(char, max+1, "IMGF_SGIRLE data");
	  if (iobfread(header->rledata, max, 1, f) <= 0) {
	    goto nope;
	  }
	}
      }
    } else if (c == 'P' && (c = iobfgetc(imgf)) >= '1' && c <= '7') {
      if (c <= '6') {	
	msg = "%s: Bad header on PNM image";
	header->channels = (c == '3' || c == '6') ? 3 : 1;
	if (iobfgetni(imgf, 1, &header->xsize, 0) != 1) {
	  goto nope;
	}
	if (iobfgetni(imgf, 1, &header->ysize, 0) != 1) {
	  goto nope;
	}
	if (c != '1' && c != '4') {
	  if (iobfgetni(imgf, 1, &header->maxval, 0) <= 0) {
	    goto nope;
	  }
	} else {
	  header->maxval = 255;
	}
	switch(c) {
	case '1': case '2': case '3': header->format = IMGF_ASCII; break;
	case '4':                     header->format = IMGF_BIT;   break;
	case '5': case '6':           header->format = IMGF_BYTE;  break;
	}
	while((c = iobfgetc(imgf)) != '\n' && c != EOF);
      } else {
	int need = 4;
	/* newer PAM format, possibly including an alpha channel */
	msg = "%s: Bad header on PAM image";

	while ((w = iobftoken(imgf, 0)) != NULL) {
	  if (strncmp(w, "ENDHDR", 6) == 0) {
	    break;
	  } else if (strncmp(w, "HEIGHT", 6) == 0) {
	    if (iobfgetni(imgf, 1, &header->ysize, 0) != 1) {
	      goto nope;
	    }
	    --need;
	  } else if (strncmp(w, "WIDTH", 5) == 0) {
	    if (iobfgetni(imgf, 1, &header->xsize, 0) != 1) {
	      goto nope;
	    }
	    --need;
	  } else if (strncmp(w, "DEPTH", 5) == 0) {
	    if (iobfgetni(imgf, 1, &header->channels, 0) != 1) {
	      goto nope;
	    }
	    --need;
	  } else if (strncmp(w, "MAXVAL", 5) == 0) {
	    if (iobfgetni(imgf, 1, &header->maxval, 0) != 1) {
	      goto nope;
	    }
	    if (header->maxval > 255) {
	      msg =
		"%s: PAM herader: sorry, 16 bits per channel is unsupported";
	      goto nope;
	    }
	    --need;
	  } else {
	    /* just skip everything else, i.e. the tuple type. We
	     * interprete 1 channel as luminance, 2 channels as
	     * luminance & alpha, 3 channels as RGB and 4 channels as
	     * RGBA, no matter what the header says.
	     */
	  }
	}
	if (need != 0) {
	  goto nope; /* did not find all required fields */
	}
	header->format = IMGF_BYTE; /* will work */
      }
    } else {
      /* assume raw pixel data */
      iobfseekmark(imgf);
      iobfclearmark(imgf);
      if (*chmask == 0xf) {
	OOGLError(1, "parseheader(): "
		  "chmask == AUTO cannot work with raw image data");
	return false;
      }
      header->format   = IMGF_BYTE;
      header->xsize    = img->width;
      header->ysize    = img->height;
      header->maxval   = img->maxval ? img->maxval : 255;
      header->channels = chmask_channels;
    }
  }

  /* set image dimensions from the data if the user did not specify
   * them explicitly.
   */
  if (img->width <= 0) {
    img->width = header->xsize;
  }
  if (img->height <= 0) {
    img->height = header->ysize;
  }

  /* Check for consistency. The number of channels can be different,
   * but the x- and y-size must match the width and height of the
   * image.
   */
  if (img->width != header->xsize || img->height != header->ysize) {
    msg = "%s: specified image dimensions do not match the image data";
    goto nope;
  }

  if (*chmask == 0) { /* determine channels from image header */
    *chmask = (1 << header->channels) - 1;
  } else if (chmask_channels > header->channels && header->channels != 1) {
    /* We allow to fill several destination channels with a single
     * grey-level image, but otherwise the user has to provide enough
     * channels such that the number destination and source channels
     * match.
     */
    OOGLError(1, "parseheader(): "
	      "source image has only %d channels, needed are %d channels",
	      header->channels, chmask_channels);
    msg = NULL;
    goto nope;
  }

  return true;

 nope:
  header->xsize = header->ysize = header->channels = 0;
  if (msg) {
    OOGLError(0, msg, "parseheader()");
  }
  return false;
}

static bool
readdata(Image *img, IOBFILE *imgf, unsigned chmask, struct imgheader *header)
{
  int val, bit, i, j, k, depth;
  int chan_map[4] = { -1, -1, -1, -1 };
  int n_chan, maxval, mask_channels;
  unsigned mask;
  int rowsize, stride, yup, rlebase;

  if (header->xsize <= 0 || header->ysize <= 0) {
    return false;
  }

  /* readdata() only works with maxval == 255; and nothing else is
   * used elsewhere in Geomview, so what.
   */
  if (img->maxval != 255) {
    OOGLError(0, "readdata(): maxval != 255 is not supported yet");
    return false;
  }
  maxval = 255;

  /* chan_map[j] == i: stuff the j-th source channel into the i-th
   * destination channel
   */
  for (i = mask_channels = 0, mask = chmask;
       i < 4 && mask != 0; i++, mask >>= 1) {
    if (mask & 1) {
      chan_map[mask_channels++] = i;
    }
  }
  n_chan = max(img->channels, i);

  depth   = ((maxval > 255) + 1);
  stride  = n_chan * depth;
  rowsize = img->width * stride;

  if (img->data == NULL || img->channels < n_chan) {
    char *old_data = img->data;
    char *new_data;
    int old_stride;
    char *old_buf = old_data;
    
    img->data =
      new_data = OOGLNewNE(char, img->height * rowsize, "New image data");

    old_stride = old_data ? img->channels : 0;
    for (i = 0; i < img->height * img->width; i++) {
      for (k = 0; k < old_stride; k++) {
	new_data[k] = old_data[k];
      }
      /* Also initialize any new fields. This would only be needed for
       * channels not set in chmask, but the conditional would
       * probably eat up more time than the assignment. Reverting the
       * loop-order would also do no good because this is a pixmap:
       * iterating over the channels in the outer loop would be very
       * unfriendly for the cache.
       */
      for (; k < stride-1; k++) {
	new_data[k] = '\0';
      }
      new_data[stride-1] = (char)0xff; /* alpha channel defaults to 0xff */
      old_data += old_stride;
      new_data += stride;
    }
    OOGLFree(old_buf);
    img->channels = n_chan;
  }

  switch (header->format) {
  case IMGF_SGIRAW:
    for (k = 0; k < min(mask_channels, header->channels); k++) {
      int dst_chan = chan_map[k];
      for (i = 0; i < header->ysize; i++) {
	char *pix = img->data + dst_chan + rowsize * i;
	j = header->xsize;
	do {
	  *pix = iobfgetc(imgf);
	  pix += stride;
	} while(--j > 0);
      }
      if (iobfeof(imgf)) {
	goto nope;
      }
    }
    break;
  case IMGF_SGIRLE:
    yup = header->rleoff[0] < header->rleoff[1];
    rlebase = 512 + header->channels*header->ysize*4;
    for (k = 0; k < min(mask_channels, header->channels); k++) {
      int dst_chan = chan_map[k];
      for (i = 0; i < header->ysize; i++) {
	char *rle = NULL;
	int row = (yup ? i : header->ysize-i-1);
	char *pix = img->data + dst_chan + rowsize * row;
	int foff = header->rleoff[k*header->ysize + row];
	/*int len = header->rleoff[(k+header->channels)*header->ysize + row];*/
	int count;
	j = header->xsize;
	if (header->rledata) {
	  rle = header->rledata + foff - rlebase;
	} else {
	  iobfseek(imgf, foff, SEEK_SET);
	}
	while ((count = rle ? *rle++ : iobfgetc(imgf)) > 0) {
	  if (count & 0x80) {
	    count &= 0x7F;
	    do {
	      *pix = rle ? *rle++ : iobfgetc(imgf);
	      pix += stride;
	    } while(--count > 0);
	  } else {
	    int val = rle ? *rle++ : iobfgetc(imgf);
	    do {
	      *pix = val;
	      pix += stride;
	    } while(--count > 0);
	  }
	}
	if (iobfeof(imgf)) {
	  goto nope;
	}
      }
    }
  case IMGF_BYTE:
  case IMGF_BIT:
  case IMGF_ASCII:
    for (i = 0; i < header->ysize; i++) {
      char *row = img->data + rowsize * (header->ysize - i - 1);
      if (img->channels == header->channels &&
	  header->format == IMGF_BYTE &&
	  (chmask ^ ((1 << n_chan) - 1)) == 0 && header->maxval == 255){
	/* all channels specified, so just copy the row in one run */
	j = iobfread(row, header->channels, header->xsize, imgf);
      } else {
	char *pix = row;
	j = header->xsize;
	switch(header->format) {
	case IMGF_BYTE:
	  do {
	    for (k = 0; k < min(mask_channels, header->channels); k++) {
	      pix[chan_map[k]] = iobfgetc(imgf) * 255 / header->maxval;
	    }
	    for (; k < header->channels; k++) {
	      iobfgetc(imgf); /* consume any remaining channels */
	    }
	    pix += stride;
	  } while(--j);
	  break;
	case IMGF_BIT:
	  bit = 0;
	  do {
	    if (--bit < 0) {
	      bit = 7;
	      k = iobfgetc(imgf);
	    }
	    pix[chan_map[0]] = (k >> bit) & 1;
	    pix += stride;
	  } while(--j > 0);
	  break;
	case IMGF_ASCII:
	  do {
	    for (k = 0; k < min(mask_channels, header->channels); k++) {
	      iobfgetni(imgf, 1, &val, 0);
	      pix[chan_map[k]] = val * 255 / header->maxval;
	    }
	    for (; k < header->channels; k++) {
	      iobfgetni(imgf, 1, &val, 0); /* consume any remaining channels */
	    }
	    pix += stride;
	  } while(--j > 0);
	  break;
	default:
	  break;
	}
      }
      if (iobfeof(imgf))
	break;
    }
  }

  if (header->channels == 1 && mask_channels > 1) {
    /* clone the data from chan_map[0] into all other channels; this
     * allows to generate an RGB(A) image from a single grey-map
     */
    char *pix = img->data;

    for (i = 0; i < img->height * rowsize; i++) {
      for (j = 1; j < mask_channels; j++) {
	for (k = 0; k < depth; k++) {
	  pix[chan_map[j]+k] = pix[chan_map[0]+k];
	}
      }
      pix += stride;
    }
  }

 nope:
  if (header->rleoff) {
    OOGLFree(header->rleoff);
  }
  if (header->rledata) {
    OOGLFree(header->rledata);
  }
  header->rleoff = NULL;
  header->rledata = NULL;

  if (i < header->ysize) {
    OOGLError(0, "readdata(): Error reading image row %d of %d",
	      i, header->ysize);
    return false;
  }
  return true;
}

/* Take a file-descriptor and connect it to the stdin of a filter
 * program, return a file descriptor which is connected to stdout of
 * the filter.
 *
 * If wronly == true, then also close the read-part of the pipe and
 * return 0 on success.
 */
static int run_filter(const char *filter, int fdin, bool wronly, int *cpidp)
{
  int pfd[2];
  int cpid;

  if (!wronly) {
    if (pipe(pfd) == -1) {
      OOGLError(1, "%s: pipe() failed", filter);
      return -1;
    }
  }
  
  if ((cpid = fork()) == -1) {
    OOGLError(1, "%s: fork() failed", filter);
    return -1;
  }

  if (cpid == 0) {
    /* child (filter process), close pdf[0] (the reader), connect fdin
     * to stdin, and pdf[1] to stdout.
     */
    close(STDIN_FILENO);
    if (dup2(fdin, STDIN_FILENO) != STDIN_FILENO) {
      OOGLError(1, "%s: cannot reassign STDIN_FILENO");
      _exit(EXIT_FAILURE);
    }
    close(fdin);

    if (wronly) {
      /* close stdout and duplicate it on stderr, otherwise a process
       * listeningon our stdout might get confused.
       */
      close(STDOUT_FILENO);
      if (dup2(STDERR_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
	OOGLError(1, "%s: cannot reassign STDOUT_FILENO");
	_exit(EXIT_FAILURE);
      }
    } else {
      /* if !wronly close the read-end of the pipe and dup the write
       * end on STDOUT.
       */
      close(pfd[0]);
      close(STDOUT_FILENO);
      if (dup2(pfd[1], STDOUT_FILENO) != STDOUT_FILENO) {
	OOGLError(1, "%s: cannot reassign STDOUT_FILENO");
	_exit(EXIT_FAILURE);
      }
      close(pfd[1]);
    }
    
    /* now run the filter process, run it through sh 'cause `filter'
     * can be a command with arguments (and possibly file descriptor
     * redirections).
     */
    execl("/bin/sh", "sh", "-c", filter, NULL);

    OOGLError(1, "execl(%s) failed.", filter);
    _exit(EXIT_FAILURE);
  } else {
    /* parent */
    if (cpidp) {
      *cpidp = cpid;
    }
    if (wronly) {
      close(pfd[1]); /* close the write end */
    }
  }

  return wronly ? 0 : pfd[0]; /* return the read end */
}

/* Fork a child which pipes data through a pipe to the parent.
 *
 * The return value is a pipe descriptor connected to the child's
 * output.
 */
static int data_pipe(const char *data, int datalen, int *cpidp)
{

  int pfd[2];
  int cpid;

  if (pipe(pfd) == -1) {
    OOGLError(1, "data_pipe(): pipe() failed");
    return -1;
  }
  
  if ((cpid = fork()) == -1) {
    OOGLError(1, "data_pipe(): fork() failed");
    return -1;
  }

  if (cpid == 0) { /* child */
    /* NOTE: we use _exit() and not exit() to avoid calling atexit()
     * functions inherited from the parent.
     */
    close(pfd[0]); /* close the reader */
    if (write(pfd[1], data, datalen) != datalen) {
      OOGLError(1, "data_pipe(): write() failed");
      _exit(EXIT_FAILURE);
    }
    if (close(pfd[1]) < 0) {
      OOGLError(1, "data_pipe(): close() failed");
      _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
  } else {         /* parent */
    if (cpidp) {
      *cpidp = cpid;
    }
    close(pfd[1]); /* close the write end */
  }

  return pfd[0]; /* return the read end */
}

#if HAVE_LIBZ
static int gv_compress2(Bytef *dest, uLongf *destLen,
			const Bytef *source, uLong sourceLen,
			int level)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS+16, MAX_MEM_LEVEL,
		       Z_DEFAULT_STRATEGY);
    if (err != Z_OK) return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}
#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */

