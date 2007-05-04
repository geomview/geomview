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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mgP.h"
#include <ctype.h>
#include <stdio.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(_WIN32) || defined(WIN32)
# include <io.h>
# define R_OK  4	/* sigh, no Windows include-file defines R_OK! */
# define iobpclose iobfclose
#endif

#include <fcntl.h>
#include <signal.h>
#include <string.h>

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
int mg_textureclock(void)
{
    Texture *tx, *txn;
    mgcontext *ctx;

    DblListIterate(&AllLoadedTextures, Texture, loadnode, tx, txn) {
	if (!(tx->flags & TXF_USED)) {
	    TxUser *tu;
	    int anyused = 0, needed = 0;

	    for (tu = tx->users; tu != NULL; tu = tu->next) {
		/* We need this if:
		 *  this mg context has been used (but didn't use this texture)
		 * ...
		 */
		if (tu->ctx != NULL &&
		    ((mgcontext *)tu->ctx)->changed & MC_USED) {
		    anyused = 1;
		}
		if (tu->needed != NULL && (*tu->needed)(tu)) {
		    needed = 1;
		}
	    }
	    if ((anyused && !needed) || tx->users == NULL) {
		TxPurge(tx);
	    }
	}
	tx->flags &= ~TXF_USED;
    }

    /* Reset all mg contexts' "used" bits for next cycle. */
    for (ctx = _mgclist; ctx != NULL; ctx = ctx->next) {
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
    Texture *tx, *txn;
    TxUser *tu, **tup;

    for (another = _mgclist; another != NULL; another = another->next) {
	if (another != ctx && another->devno == mgdtype)
	    break;
    }
    DblListIterate(&AllLoadedTextures, Texture, loadnode, tx, txn) {
	for (tup = &tx->users, tu=*tup; tu != NULL; tu=*tup) {
	    if (tu->ctx == ctx) {
		if (another != NULL) {
		    tu->ctx = another;
		    tup = &tu->next;
		} else {
		    *tup = tu->next;
		    if (tu->purge) {
			(*tu->purge)(tu);
		    }
		    OOGLFree(tu);
		}
	    }
	}
	if (tx->users == NULL) {
	    TxPurge(tx);
	}
    }
}

/* really == false means to only compare the images and the s/t clamp settings
 * really == true means to compare all components
 */
bool
mg_same_texture(Texture *tx1, Texture *tx2, bool really)
{
    if (tx1 == tx2) {
	return true;
    }
    if (tx1->image != tx2->image) {
	return false;
    }
    if ((tx1->flags ^ tx2->flags) & (TXF_SCLAMP|TXF_TCLAMP)) {
	return false;
    }
    if (really) {
	if (tx1->apply != tx2->apply) {
	    return false;
	}
	if (tx1->apply == TXF_BLEND &&
	    (tx1->background.r != tx2->background.r ||
	     tx1->background.g != tx2->background.g ||
	     tx1->background.b != tx2->background.b)) {
	    return false;
	}
    }
    return true;
}

/* Find a texture in the cache which shares the texture image and the
 * clamping, background and application styles may be different.
 */
TxUser *
mg_find_shared_texture(Texture *wanttx, int type)
{
    Texture *tx;
    TxUser *tu;

    DblListIterateNoDelete(&AllLoadedTextures, Texture, loadnode, tx) {
	if (mg_same_texture(tx, wanttx, false)) {
	    for (tu = tx->users; tu != NULL; tu = tu->next) {
	        if (tu->ctx && ((mgcontext *)tu->ctx)->devno == type)
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
    DblListIterateNoDelete(&AllLoadedTextures, Texture, loadnode, tx) {
	for (tu = tx->users; tu != NULL; tu = tu->next) {
	    if (tu->ctx && ((mgcontext *)tu->ctx)->devno == type) {
		if (((unsigned)tu->id) < FD_SETSIZE) {
		    FD_SET(tu->id, &ids);
		    if (max < tu->id) max = tu->id;
		}
	    }
	}
    }
    for (id = 1; id < FD_SETSIZE && FD_ISSET(id, &ids) && id <= max; id++)
	    ;
    if (id >= FD_SETSIZE) {
	OOGLError(0, "Yikes: all %d texture slots in use?", FD_SETSIZE);
	id--;
    }
    return id;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
