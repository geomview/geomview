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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mgP.h"
#include <ctype.h>
#include <stdio.h>

#if defined(unix) || defined(__unix)
# include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
# include <io.h>
# define R_OK  4	/* sigh, no Windows include-file defines R_OK! */
# define pclose fclose
#endif

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>

struct xyc {
    int xsize, ysize, channels;
    enum _format {
	TF_BYTE,
	TF_BIT,
	TF_ASCII,
	TF_SGIRLE,		/* SGI RLE image */
	TF_SGIRAW		/* for(z) for(y=bottom to top) { data } */
    } format;
    int maxval;		/* For PNM TF_ASCII files */
    int *rleoff;	/* For TF_SGIRLE: offsets[z][y], then lengths[z][y] */
    char *rledata;	/* For TF_SGIRLE: all data (don't fseek so we can read from pipe)*/
};

static FILE *
gimme(char *fname, int *dopclose, struct xyc *size)
{
    char cmd[2048];
    char *suf, *p, *q;
    int i, c, len, slen, maxval;
    char *prefix = NULL;
    char *msg = NULL;
    FILE *f = NULL;
    int xy[2];
    static char *suffixes[] = {
	"\0zcat ", "Z",
	"\0gzip -dc ", "z", "gz",
	"\0tifftopnm ", "tiff", "tif",
	"\0giftoppm ", "gif",
	"\0",
	NULL
    };
    

    size->rleoff = NULL;  size->rledata = NULL;
    if(fname == NULL)
	goto nope;
    len = strlen(fname);
    for(i = 0; (suf = suffixes[i]) != NULL; i++) {
	if(suf[0] == '\0') {
	    prefix = suf+1;
	    continue;
	} else {
	    slen = strlen(suf);
	    if(slen < len && fname[len-slen-1] == '.' &&
			    strcmp(fname+len-slen, suf) == 0)
		break;
	}
    }

    if(access(fname, R_OK) < 0) {
	OOGLError(0, "%s: Can't read texture image: %s", fname, sperror());
	goto nope;
    }
    if(prefix == NULL || prefix[0] == '\0') {
	*dopclose = 0;
	f = fopen(fname, "rb");
#if defined(unix) || defined(__unix)
    } else {
	strcpy(cmd, prefix);
	for(p = cmd+strlen(cmd), q = fname; *q && p < &cmd[sizeof(cmd)-10]; ) {
	    if(!(isalnum(*q) || *q == '/' || *q=='.'))
		*p++ = '\\';	/* Quote anything remotely suspicious */
	    *p++ = *q++;
	}
	*p = '\0';
	*dopclose = 1;
	f = popen(cmd, "rb");
#endif
    }
    if(f == NULL) {
	OOGLError(1, "mg_inhaletexture: Can't %s %s: %s",
			*dopclose ? "invoke" : "open", cmd, sperror());
	goto nope;
    }
    c = fgetc(f);
    if(c == 0x01 && (c = fgetc(f)) == 0xDA) {
	/* SGI image file */
	short shorts[3];
	int storage = fgetc(f);
	int bpp = fgetc(f);
	if(bpp != 1) {
	    msg = "%s: must have 8-bit image values";
	    goto nope;
	}
	fgetc(f); fgetc(f);		/* Skip "dimension" */
	fgetns(f, 3, shorts, 1);	/* Read big-endian 16-bit values */
	size->xsize = shorts[0];
	size->ysize = shorts[1];
	size->channels = shorts[2];
	for(i = 0; i < 4+4+492; i++)	/* Skip min, max, pad data */
	    getc(f);
	size->format = (storage==0x01) ? TF_SGIRLE : TF_SGIRAW;
	if(size->format == TF_SGIRLE) {
	    /* Inhale offset&length table */
	    int n = size->ysize*size->channels;
	    int max = 0;
	    size->rleoff = OOGLNewNE(int, n*2, "TF_SGIRLE offsets");
	    msg = "%s: can't read RLE offsets";
	    if(fgetni(f, n*2, size->rleoff, 1) != n*2)
		goto nope;
	    if(ftell(f) < 0) {
		for(i = 0; i < n; i++)
		    if(max < size->rleoff[i])
			max = size->rleoff[i] + size->rleoff[i+n];
		size->rledata = OOGLNewNE(char, max+1, "TF_SGIRLE data");
		if(fread(size->rledata, max, 1, f) <= 0)
		    goto nope;
	    }
	}
    } else if(c == 'P' && (c = fgetc(f)) >= '1' && c <= '6') {
	msg = "%s: Bad header on PNM image";
	size->channels = (c == '3' || c == '6') ? 3 : 1;
	if(fgetni(f, 2, &size->xsize, 0) != 2)
	    goto nope;
	size->maxval = 1;
	if(c != '1' && c != '4')
	    if(fgetni(f, 1, &size->maxval, 0) <= 0)
		goto nope;
	switch(c) {
	case '1': case '2': case '3':	size->format = TF_ASCII; break;
	case '4':			size->format = TF_BIT; break;
	case '5': case '6':		size->format = TF_BYTE; break;
	}
	while((c = fgetc(f)) != '\n' && c != EOF)
	    ;
    } else {
	msg = "%s: Unknown texture image file format";
	goto nope;
    }
    return f;

  nope:
    size->xsize = size->ysize = size->channels = 0;
    if (f) {
#if defined(unix) || defined(__unix)
      if (*dopclose) {
	pclose(f);
      } else {
#endif
	fclose(f);
#if defined(unix) || defined(__unix)
      }
#endif
    }
    if(msg) {
	OOGLError(0, msg, fname);
    }

    return NULL;
}

int
readimage(Texture *tx, int offset, int rowsize, struct xyc *size, FILE *f, char *fname)
{
    int val, bit, i, j, k, c, row;
    int stride = tx->channels;
    char *pix;

    if(size->xsize <= 0 || size->ysize <= 0)
	return 0;

    if(size->format == TF_SGIRAW) {
	for(k = 0; k < size->channels; k++) {
	    for(i = 0; i < size->ysize; i++) {
		char *pix = tx->data + offset + k + rowsize * i;
		j = size->xsize;
		do {
		    *pix = getc(f);
		    pix += stride;
		} while(--j > 0);
	    }
	    if(feof(f))
		goto nope;
	}
    } else if(size->format == TF_SGIRLE) {
        int yup = size->rleoff[0] < size->rleoff[1];
	int rlebase = 512 + size->channels*size->ysize*4;
	for(k = 0; k < size->channels; k++) {
	    for(i = 0; i < size->ysize; i++) {
		char *rle = NULL;
		int row = (yup ? i : size->ysize-i-1);
		char *pix = tx->data + offset + k + rowsize * row;
		int foff = size->rleoff[k*size->ysize + row];
		int len = size->rleoff[(k+size->channels)*size->ysize + row];
		int count;
		j = size->xsize;
		if(size->rledata)
		    rle = size->rledata + foff - rlebase;
		else
		    fseek(f, foff, SEEK_SET);
		while((count = rle ? *rle++ : getc(f)) > 0) {
		    if(count & 0x80) {
			count &= 0x7F;
			do {
			    *pix = rle ? *rle++ : getc(f);
			    pix += stride;
			} while(--count > 0);
		    } else {
			int val = rle ? *rle++ : getc(f);
			do {
			    *pix = val;
			    pix += stride;
			} while(--count > 0);
		    }
		}
		if(feof(f))
		    goto nope;
	    }
	}
    } else {
	for(i = 0; i < size->ysize; i++) {
	    char *row = tx->data + rowsize * (size->ysize - i - 1) + offset;
	    if(tx->channels == size->channels && size->format == TF_BYTE) {
		j = fread(row, size->channels, size->xsize, f);
	    } else {
		register char *pix = row;
		int vals[3];
		j = size->xsize;
		switch(size->format) {
		case TF_BYTE:
		    switch(size->channels) {
		    case 1: do { *pix = getc(f); pix += stride; } while(--j); break;
		    case 3: do {
			      pix[0] = getc(f);
			      pix[1] = getc(f);
			      pix[2] = getc(f);
			      pix += stride;
			    } while(--j);
			    break;
		    }
		    break;
		case TF_BIT:
		    bit = 0;
		    do {
			if(--bit < 0) {
			    bit = 7;
			    k = getc(f);
			}
			*pix = (k >> bit) & 1;
			pix += stride;
		    } while(--j > 0);
		    break;
		case TF_ASCII:
		    do {
			for(k = 0; k < size->channels; k++) {
			    fgetni(f, 1, &val, 0);
			    pix[k] = val * 255 / size->maxval;
			}
			pix += stride;
		    } while(--j > 0);
		    break;
		default:
		  break;
		}
	    }
	    if(feof(f))
		break;
	}
    }

 nope:
    if(size->rleoff)
	OOGLFree(size->rleoff);
    if(size->rledata)
	OOGLFree(size->rledata);
    size->rleoff = NULL;
    size->rledata = NULL;

    if(i < size->ysize) {
	OOGLError(0, "%s: Error reading texture image row %d of %d",
		fname, i, size->ysize);
	return 0;
    }
    return 1;
}


/*
 * Give mg contexts a chance to purge any textures which aren't in use.
 * The wise user will call this routine once each redraw cycle;
 * any texture data which hasn't been used since the last call will be
 * purged.  So, if a texture has been superceded, we'll catch it here.
 * Several policies are possible; we just implement one.  Maybe there could
 * be a switch.  For example, we could:
 *  (a) purge a texture if its data hasn't actually been used in the last
 *	cycle.  (Then, if texturing is turned off and then on again, it'll
 *	have to be reloaded each time it's turned on.)
 *  (b) purge a texture if no geom object referring to it has been used
 *	in the last cycle, whether the texture was actually rendered or not.
 *	(This keeps textures loaded, but discards them if they're superceded.
 *	It could screw up if some objects just aren't rendered during some
 *	rendering cycles.)
 *  (c) purge a texture if all references to it (aside from texture caches)
 *	seem to be gone, i.e. by comparing reference counts with the number
 *	of users on the tx->users list.  This would preserve textures held
 *	in handles, or by objects which for some reason aren't being rendered.
 *	Stands a large chance of missing things that really deserve
 *	garbage-collection.
 * We actually implement (b).
 *
 * For each texture, we check the "used" bit, indicating that some rendered
 * geom object has had a reference to that texture since the last call to
 * mg_textureclock().  We purge textures which haven't thus been "used",
 * but which have at least one user whose mg context has been rendered to. 
 *
 */
int
mg_textureclock()
{
    Texture *tx;
    mgcontext *ctx;
    for(tx = AllLoadedTextures; tx != NULL; tx = tx->next) {
	if(!(tx->flags & TXF_USED)) {
	    TxUser *tu;
	    int anyused = 0, needed = 0;
	    for(tu = tx->users; tu != NULL; tu = tu->next) {
		/* We need this if:
		 *  this mg context has been used (but didn't use this texture)
		 * ...
		 */
		if(tu->ctx != NULL && ((mgcontext *)tu->ctx)->changed & MC_USED)
		   anyused = 1;
		if(tu->needed != NULL && (*tu->needed)(tu))
		   needed = 1;
	    }
	    if((anyused && !needed) || tx->users == NULL)
		TxPurge(tx);
	}
	tx->flags &= ~TXF_USED;
    }
    /* Reset all mg contexts' "used" bits for next cycle. */
    for(ctx = _mgclist; ctx != NULL; ctx = ctx->next) {
	ctx->changed &= ~MC_USED;
    }
    return 0;
}

/*
 * We assume the graphics system allows textures to be shared across
 * multiple windows of a type.  We're closing one of those windows.
 * If any texture-users claim to be using that window, switch them to
 * use another of the same type, if possible.  Otherwise, just purge them.
 */
void
mg_reassign_shared_textures(mgcontext *ctx, int mgdtype)
{
    mgcontext *another;
    Texture *tx;
    register TxUser *tu, **tup;
    for(another = _mgclist; another != NULL; another = another->next) {
	if(another != ctx && another->devno == mgdtype)
	    break;
    }
    for(tx = AllLoadedTextures; tx != NULL; tx = tx->next) {
	for(tup = &tx->users, tu=*tup; tu != NULL; tu=*tup) {
	    if(tu->ctx == ctx) {
		if(another != NULL) {
		    tu->ctx = another;
		    tup = &tu->next;
		} else {
		    *tup = tu->next;
		    if(tu->purge)
			(*tu->purge)(tu);
		    OOGLFree(tu);
		}
	    }
	}
	if(tx->users == NULL)
	    TxPurge(tx);
    }
}

void
mg_remove_txuser(TxUser *tu)
{
    TxUser **tup;
    Texture *tx;

    if(tu == NULL) return;
    tx = tu->tx;
    for(tup = &tx->users; *tup != NULL; tup = &(*tup)->next) {
	if(tu == *tup) {
	    *tup = tu->next;
	    if(tu->purge)
		(*tu->purge)(tu);
	    OOGLFree(tu);
	    break;
	}
    }
    if(tx->users == NULL)
	TxPurge(tx);
}

int
mg_same_file(char *fname1, char *fname2)
{
   struct stat st1, st2;
   char *tail1, *tail2;
   if((fname1 != NULL) != (fname2 != NULL))
	return 0;
   if(fname1 == NULL || strcmp(fname1, fname2) == 0)
	return 1;
   tail1 = strrchr(fname1, '/'); if(tail1 == NULL) tail1 = fname1;
   tail2 = strrchr(fname2, '/'); if(tail2 == NULL) tail2 = fname2;
   if(strcmp(fname1, fname2) != 0)
	return 0;
   if(stat(fname1, &st1) < 0 || stat(fname2, &st2) < 0 ||
	    st1.st_ino != st2.st_ino || st1.st_dev != st2.st_dev)
	return 0;
   return 1;
}
   
int
mg_same_texture(Texture *tx1, Texture *tx2)
{
    struct stat st;
    if(tx1 == tx2)
	return 1;
    if(tx1 == NULL || tx2 == NULL)
	return 0;
    if(!mg_same_file(tx1->filename, tx2->filename))
	return 0;
    if(!mg_same_file(tx1->alphafilename, tx2->alphafilename))
	return 0;
    /* Needn't compare application style, etc. */
    return 1;
}

TxUser *
mg_find_shared_texture(Texture *wanttx, int type)
{
    Texture *tx;
    TxUser *tu;
    for(tx = AllLoadedTextures; tx != NULL; tx = tx->next) {
	if(mg_same_texture(tx, wanttx)) {
	    for(tu = tx->users; tu != NULL; tu = tu->next) {
	        if(tu->ctx && ((mgcontext *)tu->ctx)->devno == type)
		    return tu;
	    }
	}
    }
    return NULL;
}

#ifdef _WIN32
# undef FD_ISSET	/* Use fake FD_ISSET from streampool.c for VC++ */
#endif

int
mg_find_free_shared_texture_id(int type)
{
    Texture *tx;
    TxUser *tu;
    fd_set ids;
    int id, max = 0;

    FD_ZERO(&ids);
    for(tx = AllLoadedTextures; tx != NULL; tx = tx->next) {
	for(tu = tx->users; tu != NULL; tu = tu->next) {
	    if(tu->ctx && ((mgcontext *)tu->ctx)->devno == type) {
		if(((unsigned)tu->id) < FD_SETSIZE) {
		    FD_SET(tu->id, &ids);
		    if(max < tu->id) max = tu->id;
		}
	    }
	}
    }
    for(id = 1; id < FD_SETSIZE && FD_ISSET(id, &ids) && id <= max; id++)
	    ;
    if(id >= FD_SETSIZE) {
	OOGLError(0, "Yikes: all %d texture slots in use?", FD_SETSIZE);
	id--;
    }
    return id;
}

int
mg_inhaletexture(Texture *tx, int rgba)
{
    FILE *f = NULL, *alphaf = NULL;
    char *failfile;
    struct xyc size, alphasize;
    int dopclose, alphapclose;
    int pixels, i, j, rowsize, ok, wantchans;
    void (*oldsigchld)();

    if(tx == NULL)
	return 0;
    if(tx->flags & TXF_LOADED)
	return (tx->data != NULL);

    tx->flags |= TXF_LOADED;		/* Even if we fail, don't retry */
    if(tx->data)
	OOGLFree(tx->data);
    tx->data = NULL;

    wantchans = tx->channels;

#if defined(unix) || defined(__unix)
    oldsigchld = (void ((*)()))signal(SIGCHLD, SIG_DFL);
#endif
    f = gimme(tx->filename, &dopclose, &size);
    alphasize.channels = 0;	/* Ensure valid for later, even if no alpha */
    alphaf = gimme(tx->alphafilename, &alphapclose, &alphasize);
    if(f == NULL) {
	if(alphaf) {
	    size = alphasize;
	    size.channels = 0;
	} else {
	    return 0;
	}
    }

    tx->xsize = size.xsize;
    tx->ysize = size.ysize;
    tx->channels = size.channels;

    if(alphaf) {
	if(size.xsize != alphasize.xsize || size.ysize != alphasize.ysize) {
	    OOGLError(1, "Texture data file (%s) is %dx%d, but alphafile (%s) is %dx%d: ignoring it",
	    tx->filename, size.xsize, size.ysize,
	    tx->alphafilename, alphasize.xsize, alphasize.ysize);
	    alphasize.channels = 0;
	} else {
	    tx->channels += alphasize.channels;
	}
    }
    
    /* Let's load the pixels in the form required by GL and Open GL. */

    pixels = tx->xsize * tx->ysize;

    if(tx->channels < wantchans)
	tx->channels = wantchans;

    rowsize = tx->xsize * tx->channels;
    rowsize = (rowsize + 3) & ~3;	/* Round up to 4-byte boundary */
    tx->data = OOGLNewNE(char, rowsize * tx->ysize, "Texture data");

    failfile = tx->filename;
    ok = readimage(tx, 0, rowsize, &size, f, tx->filename);
    if(alphaf)
	ok &= readimage(tx, tx->channels - alphasize.channels, rowsize, &alphasize,
				alphaf, tx->alphafilename);
    if(!ok) {
	OOGLFree(tx->data);
	tx->data = NULL;
    }

    if(size.channels + alphasize.channels != tx->channels) {
	if(tx->channels == 4) {
	    for(i = 0; i < size.ysize; i++) {
		register int t;
		int k = size.xsize;
		register char *p = tx->data + rowsize*i;
		switch(size.channels) {
		case 1:
		    do { p[1] = p[0]; p[2] = p[0];
			 if(alphasize.channels == 0) p[3] = 255;
			 p += 4;
		    } while(--k > 0);
		    break;
		case 3:
		    do { p[3] = 255; p += 4; } while(--k > 0);
		    break;
		}
	    }
	} else {
	    OOGLError(0,
	    "mg_inhaletexture: warning: dunno how to inhale %d+%d images into %d-chan image",
		size.channels, alphasize.channels, tx->channels);
	}
    }

    if(tx->data && (rgba & TXF_RGBA) == 0) {
	for(i = 0; i < size.ysize; i++) {
	    /* Arrange data in each pixel in the order GL prefers.
	     * Maybe the decision on whether this is needed
	     * should be an option to mg_inhaletexture().
	     */
	    register int t;
	    int k = size.xsize;
	    register char *p = tx->data + rowsize*i;
	    switch(tx->channels) {
	    case 2:
		do {
		    t = *p; *p = p[1]; p[1] = t;
		    p += 2;
		} while(--k > 0);
		break;
	    case 3:
		do {
		    t = *p; *p = p[2]; p[2] = t;
		    p += 3;
		} while(--k > 0);
		break;
	    case 4:
		do {
		    t = *p; *p = p[3]; p[3] = t;
		    t = p[1]; p[1] = p[2]; p[2] = t;
		    p += 4;
		} while(--k > 0);
	    }
	}
    } else {
	tx->flags |= TXF_RGBA;
    }
    if(f) {
	if(dopclose) pclose(f);
	else fclose(f);
    }
    if(alphaf) {
	if(alphapclose) pclose(alphaf);
	else fclose(alphaf);
    }
#ifdef SIGCHLD
    signal(SIGCHLD, oldsigchld);
#endif
    return ok;
}
