/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

/*
 * NOTES:
 * 1. mrti() assumes unsigned short is 2 bytes, MSB first
 *    this may have to change for other architecture
 * 2. will it be possible to increase the encoded float array length
 *    by "adding l" to create 256 aggregates of 3 or 4 floats each, as
 *    opposed to 256 straight floats? (see appendix C, pg 163 of the
 *    'big' renderman manual)
 * 3. the name mrti should be changed to mgri (it sounds better)
 */

#include "mgribtoken.h"
#include <stdio.h>

#include "mg.h"
#include "mgribP.h"
#include "mgrib.h"

#ifdef test
#include <stdio.h>
#endif

#define BUFFERSIZE 1024*128    /* 128k buffer */
#define TMPSIZE 8192
#define SECTIONWIDTH 70        /* width of a section header */

#define STRINGDEF	(char)0315
#define STRINGREF	(char)0317
#define LNGSTRINGENCODE	(char)0241   /* length follows, is unsigned short */
#define STRINGENCODE	(char)0220
#define RIREQDEF	(char)0314
#define RIREQREF	(char)0246
#define FLOATARRAYDEF	(char)0310
#define INTEGER		(char)0201   /* unsigned short */
#define FLOAT		(char)0244

void binary_token(int a1, va_list *alist);
void ascii_token(int a1, va_list *alist);

TokenBuffer *_tokenbuffer;
#define tokenbuffer _tokenbuffer->tkb_buffer
#define ptr         _tokenbuffer->tkb_ptr
#define limit       _tokenbuffer->tkb_limit
#define worldptr    _tokenbuffer->tkb_worldptr

#define TABLEROW(cname, reqn) {cname, sizeof(cname)-1, reqn, 0 }

/* following is the table: see mgribtoken.h (struct) for desc of fields */
static struct _table table[] = {
    TABLEROW("", 0),		/* mr_NULL */

    /* Ri Requests */
    TABLEROW("AttributeBegin", 0),
    TABLEROW("AttributeEnd", 1),
    TABLEROW("TransformBegin", 2),
    TABLEROW("TransformEnd", 3),
    TABLEROW("Identity", 4),
    TABLEROW("ConcatTransform", 5),
    TABLEROW("Surface", 6),
    TABLEROW("ShadingInterpolation", 7),
    TABLEROW("Color", 8),
    TABLEROW("Opacity", 9),
    TABLEROW("Polygon", 10),
    TABLEROW("PatchMesh", 11),
    TABLEROW("Format", 12),
    TABLEROW("Projection", 13),
    TABLEROW("Clipping", 14),
    TABLEROW("WorldBegin", 15),
    TABLEROW("WorldEnd", 16),
    TABLEROW("Display", 17),
    TABLEROW("ScreenWindow", 19),
    TABLEROW("LightSource", 20),
    TABLEROW("Sphere", 21),
    TABLEROW("Translate", 22),
    TABLEROW("Rotate", 23),
    TABLEROW("Cylinder", 24),
    TABLEROW("NuPatch", 25),
    TABLEROW("ShadingRate", 26),
    TABLEROW("Option", 27),
    TABLEROW("Illuminate", 28),
    TABLEROW("FrameBegin", 29),
    TABLEROW("FrameEnd", 30),
    TABLEROW("ReverseOrientation", 31),
    TABLEROW("Curves", 32),
    TABLEROW("Points", 33),
    TABLEROW("MakeTexture", 34),
    TABLEROW("Declare", 35),

    /* following are reserved - do not add */
    /* or remove fields, just change them! */
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),
    TABLEROW("", 255),

    /* Strings - we start these out at position 50 */
    TABLEROW("P", 0),
    TABLEROW("N", 1),
    TABLEROW("Cs", 2),
    TABLEROW("Pw", 3),
    TABLEROW("Os", 4),
    TABLEROW("st", 5),
    TABLEROW("plastic", 6),
    TABLEROW("paintedplastic", 7),
    TABLEROW("hplastic", 8),
    TABLEROW("eplastic", 9),
    TABLEROW("heplastic", 10),
    TABLEROW("constant", 11),
    TABLEROW("ambientlight", 12),
    TABLEROW("lightcolor", 13),
    TABLEROW("distantlight", 14),
    TABLEROW("intensity", 15),
    TABLEROW("file", 16),
    TABLEROW("rgb", 17),
    TABLEROW("rgba", 18),
    TABLEROW("Ka", 19),
    TABLEROW("Kd", 20),
    TABLEROW("Ks", 21),
    TABLEROW("specularcolor", 22), 
    TABLEROW("roughness", 23),
    TABLEROW("fov", 24),
    TABLEROW("perspective", 25),
    TABLEROW("to", 26),
    TABLEROW("framebuffer", 27),
    TABLEROW("texturename", 28),
    TABLEROW("width", 29),
    TABLEROW("constantwidth", 30),
    /* strings for extended texture aware shaders with support for alpha */
    TABLEROW("GVmodulateplastic", 31),
    TABLEROW("GVmodulateconstant", 32),
    TABLEROW("GVdecalplastic", 33),
    TABLEROW("GVdecalconstant", 34),
    TABLEROW("GVblendplastic", 35),
    TABLEROW("GVblendconstant", 36),
    TABLEROW("GVreplaceplastic", 37),
    TABLEROW("GVreplaceconstant", 38),
    TABLEROW("periodic", 39),
    TABLEROW("gaussian", 40)
};

void mrti_makecurrent(TokenBuffer *tkbuf)
{
    _tokenbuffer = tkbuf;
}

void mrti_delete(TokenBuffer *tkbuf)
{
    if (tkbuf->tkb_buffer) {
	free(tkbuf->tkb_buffer);
	if (_tokenbuffer == tkbuf) {
	    _tokenbuffer = NULL;
	}
	memset(tkbuf, 0, sizeof(tkbuf));
    }
}

/* initialize tokenbuffer */
void mrti_init(TokenBuffer *tkbuf)
{
    tkbuf->tkb_buffer = (char *)malloc(BUFFERSIZE);
    tkbuf->tkb_limit = (char *)(tkbuf->tkb_buffer + BUFFERSIZE);
    tkbuf->tkb_worldptr = tkbuf->tkb_ptr = tkbuf->tkb_buffer;
    tkbuf->tkb_buffer[0] = '\0';
}

/* reset the ptr & tokenbuffer */
void mrti_reset(void)
{
    worldptr = ptr = tokenbuffer;
    tokenbuffer[0] = '\0';
}

/* quick copy routine w/ ptr update */
void cat(char *s, char *a)
{
    while((*(s++)=(*(a++)))) ptr++;
}


/* routine will check buffer for possible overun, will realloc if needed */
/* generally, "worst case" values should be passed to check_buffer       */
void check_buffer(int length)
{
    length += 8;
    if((char *)(ptr+length)>(char *)limit) {
        /* optinally check for maximum buffer allocation here */
	size_t world = worldptr - tokenbuffer;
	size_t used = ptr - tokenbuffer;
	size_t avail = limit - tokenbuffer;
	/* Buffer grows exponentially, by factor of 1.5 each time */
	do { avail += avail>>1; } while(used+length >= avail);
	tokenbuffer = (char *)realloc(tokenbuffer, avail);
	ptr = tokenbuffer + used;
	worldptr = tokenbuffer + world;
	limit = tokenbuffer + avail;
    }
}

/* process variable size token list */
void mrti(int a1, ... )
{
    va_list alist;

    va_start(alist, a1);
 
    switch(_mgribc->render_device) {
    case RMD_BINARY: binary_token(a1, &alist); break;
    case RMD_ASCII:  ascii_token(a1, &alist);  break;
    }
    va_end(alist);
}

/* return 1 when supplied token requires its own (ascii) line, 0 otherwise */
bool line_initializer(int token)
{
    /* THIS IS HACKISH - IT WILL CREATE A LINE FEED BEFORE */
    /* IT CREATES THE NEW LINE, WHICH ISN'T ALWAYS GOOD!!  */
    if(token<STRINGBASE||token==mr_comment||token==mr_section||
       (token>=mr_P && token<=mr_Os)) return true;
    else return false;
}

/* ASCII SUBPROCESSING */
void ascii_token(int token, va_list *alist)
{
    int i;
    int count, number;
    static int arraysize;
    static int expectSubArray=0;
    static char astring[128];
    double nextfloat; /* va_arg converts floats to doubles */
    float *floatptr;
    char *s;
    int len;

    do {
    
	if(expectSubArray && (token!=mr_subarray3) && (token!=mr_subarray2)) {
	    /* ERROR */
	}
    
	/* check to see if we need to start a new line */
	if(line_initializer(token) && *(ptr-1)!='\n') *(ptr++)='\n';
    
	switch(token) {
	
	case mr_section:
	    check_buffer(SECTIONWIDTH); /* should not exceed this */
	    s = va_arg(*alist, char*);
	    len = strlen(s);
	    if(len+3>SECTIONWIDTH) len = SECTIONWIDTH-3;/* 3 added characters */
	    *(ptr++)='\n';
	    cat(ptr,"# ");
	    cat(ptr,s);
	    cat(ptr," ");
	
	    len = SECTIONWIDTH - 3 - len;
	    while((len--)>0) *(ptr++)='*';
	    break;

	case mr_comment:
	    s = va_arg(*alist, char*);
	    len = strlen(s);
	    check_buffer(len+2);
	    cat(ptr,"# ");
	    cat(ptr,s);
	    break;

	case mr_header:
	    s = va_arg(*alist, char*);
	    len = strlen(s);
	    check_buffer(len+2);
	    cat(ptr,"##");
	    cat(ptr,s);
	    break;
	
	case mr_nl:
	    /* check for space */
	    check_buffer(2);
	    if(*(ptr-1)==' ') ptr--;
	    *(ptr++)='\n';
	    break;
	
	case mr_array:
	    arraysize = va_arg(*alist, int);
	    check_buffer(arraysize*16+4); /* how else can we do this? */
	    *(ptr++)='[';
	    for(i=0;i<arraysize;i++) {
		nextfloat = va_arg(*alist, double);
		sprintf(astring,"%.8g ",nextfloat);
		cat(ptr, astring);
	    }
	    *(--ptr)=0; /* get rid of space */
	    cat(ptr,"] ");
	    break;
	
	case mr_buildarray:
	    arraysize = va_arg(*alist, int);
	    check_buffer(1);
	    expectSubArray = 1;
	    *(ptr++)='[';
	    break;

	case mr_parray: {
	    int size;
	    size = va_arg(*alist, int);
	    check_buffer(2+16*size);
	    *(ptr++)='[';
	    /* if arraysize<0 then ERROR */
	    floatptr = va_arg(*alist, float*);
	    for(i=0;i<size;i++) {
		sprintf(astring,"%.8g ",*(floatptr++));
		cat(ptr,astring);
	    }
	    *(--ptr)=0; /* get rid of unwanted space */
	    cat(ptr,"] ");
	    break;
	}
    
	case mr_subarray3:
	    check_buffer(16*3+7);
	    arraysize-=3;
	    /* if arraysize<0 then ERROR */
	    floatptr = va_arg(*alist, float*);
	    sprintf(astring,"%.8g %.8g %.8g   ",
		    *(floatptr),*(floatptr+1),*(floatptr+2));
	    cat(ptr,astring);
	    if(arraysize<=0) {
		expectSubArray = 0;
		*(ptr-=3)=0; /* get rid of unwanted spaces */
		cat(ptr,"] ");
	    }
	    break;

	case mr_subarray2:
	    check_buffer(16*3+7);
	    arraysize-=2;
	    /* if arraysize<0 then ERROR */
	    floatptr = va_arg(*alist, float*);
	    sprintf(astring,"%.8g %.8g   ",
		    *(floatptr),*(floatptr+1));
	    cat(ptr,astring);
	    if(arraysize<=0) {
		expectSubArray = 0;
		*(ptr-=2)=0; /* get rid of unwanted spaces */
		cat(ptr,"] ");
	    }
	    break;

	case mr_int:
	    number = va_arg(*alist, int);
	    check_buffer(16);
	    sprintf(astring,"%d ",number);
	    cat(ptr,astring);
	    break;

	case mr_float:
	    nextfloat = va_arg(*alist, double);
	    check_buffer(16);
	    sprintf(astring,"%.8g ",nextfloat);
	    cat(ptr,astring);
	    break;
	
	case mr_intseq:
	    /* OBSOLETE */
	    count = va_arg(*alist, int);
	    check_buffer(count*16);
	    for(i=0;i<count;i++) {
		number = va_arg(*alist, int);
		sprintf(astring,"%d ",number);
		cat(ptr,astring);
	    }
	    *(--ptr)=0;
	    break;

	case mr_string: {
	    char *string;
	    string = va_arg(*alist, char *);
	    len = strlen(string);
	    check_buffer(len+3);
	    *(ptr++)='"';
	    cat(ptr,string);
	    *(ptr++)='"';
	    *(ptr++)=' ';
	    break;
	}

	case mr_embed: {
	    /* directly embed a string */
	    char *string;
	    string = va_arg(*alist, char *);
	    len = strlen(string);
	    check_buffer(len);
	    cat(ptr,string);
	    break;
	}
	    /* these requests require a trailing line feed */
	case mr_attributebegin:
	case mr_attributeend:
	case mr_transformbegin:
	case mr_transformend:
	case mr_identity:
	case mr_polygon:
	case mr_worldend:
	case mr_frameend:
	case mr_reverseorientation:
	    check_buffer(table[token].len);
	    cat(ptr,table[token].name);
	    /*
	     *(ptr++) = '\n';
	     */
	    break;
	case mr_worldbegin:
	    check_buffer(table[token].len);
	    worldptr = ptr;
	    cat(ptr,table[token].name);
	    break;
	    
	    /* these requests require a trailing space */
	case mr_color:
	case mr_opacity:
	case mr_format:
	case mr_display:
	case mr_screenwindow:
	case mr_clipping:
	case mr_concattransform:
	case mr_projection:
	case mr_lightsource:
	case mr_illuminate:
	case mr_surface:
	case mr_sphere:
	case mr_patchmesh:
	case mr_rotate:
	case mr_translate:
	case mr_cylinder:
	case mr_nupatch:
	case mr_option:
	case mr_shadinginterpolation:
	case mr_shadingrate:
	case mr_framebegin:
	case mr_curves:
	case mr_points:
	case mr_maketexture:
	case mr_declare:
	    check_buffer(table[token].len+1);
	    cat(ptr,table[token].name);
	    *(ptr++)=' ';
	    break;
	
	    /* anything left over should be a string  */
	    /* which requires quotes & trailing space */
	    /* (THIS SHOULD BE MOVED OUT OF SWITCH)  */
	default:
	    check_buffer(table[token].len+3);
	    *(ptr++)='"';
	    cat(ptr,table[token].name);
	    *(ptr++)='"';
	    *(ptr++)=' ';
	    break;

	}
    		
    } while ((token=va_arg(*alist, int))!=mr_NULL);
}


/* BINARY SUBPROCESSING */
/* NOTE: some hardware will not support copying data types > 1 byte into an
 * arbitrary (unaligned) location. e.g.
 *    *(unsigned short *)ptr = anUnsignedVariable
 * may be an illegal instruction. Therefore, macros which will copy these
 * values one byte at a time follow here and are used where needed. These
 * macros will work only with float sizes of 4 bytes (IEEE) & unsigned short
 * int sizes of 2 bytes
 */

/* cH: hey, folks, there are sane machines around, I mean: with the
 * "correct" LITTLE-ENDIAN byte order ...
 *
 * RIB requires BIG-ENDIAN, so handle the case.
 */
#if WORDS_BIGENDIAN
# define COPYUSHORT( value )			\
    *ptr++ = ((char *)&value)[0];		\
    *ptr++ = ((char *)&value)[1]
# define COPYFLOAT( value )			\
    *ptr++ = ((char *)&value)[0];		\
    *ptr++ = ((char *)&value)[1];		\
    *ptr++ = ((char *)&value)[2];		\
    *ptr++ = ((char *)&value)[3]
#else
# define COPYUSHORT( value )			\
    *ptr++ = ((char *)&value)[1];		\
    *ptr++ = ((char *)&value)[0]
# define COPYFLOAT( value )			\
    *ptr++ = ((char *)&value)[3];		\
    *ptr++ = ((char *)&value)[2];		\
    *ptr++ = ((char *)&value)[1];		\
    *ptr++ = ((char *)&value)[0]
#endif

void binary_token(int token, va_list *alist)
{
    int i;
    static int expectSubArray=0;
    static int arraysize;
    float *floatptr;

    if(expectSubArray && (token!=mr_subarray3) && (token!=mr_subarray2)) {
    	/* ERROR */
    }

    do {

	/* single '&' on next line changed to && by mbp Wed May 17 22:33:00 2000 */    
	if((token>=STRINGBASE) && (token<mr_array)) {
	    /* The token is a string defintion/request */
	    if(!table[token].defined) {
		check_buffer(7 + table[token].len);
		*(ptr++)=STRINGDEF;
		*(ptr++)=table[token].reqn;
		if(table[token].len<16) {
		    *(ptr++)=STRINGENCODE+table[token].len;
		} else {
		    unsigned short length;
		    length = (unsigned short) table[token].len;
		    *(ptr++)=LNGSTRINGENCODE;
		    COPYUSHORT(length);
		}
		cat(ptr,table[token].name);
		table[token].defined = 1;
	    }
	    *(ptr++)=STRINGREF;
	    *(ptr++)=table[token].reqn;
	}
    
	else switch(token) {
    
	    /* THESE SHOULD BE MOVE BEFORE SWITCH */
	    /* AND TURNED INTO A SPECIAL CASE    */
	case mr_attributebegin:
	case mr_attributeend:
	case mr_transformbegin:
	case mr_transformend:
	case mr_identity:
	case mr_polygon:
	case mr_worldbegin:
	case mr_worldend:
	case mr_color:
	case mr_opacity:
	case mr_format:
	case mr_display:
	case mr_screenwindow:
	case mr_clipping:
	case mr_concattransform:
	case mr_projection:
	case mr_lightsource:
	case mr_illuminate:
	case mr_surface:
	case mr_sphere:
	case mr_patchmesh:
	case mr_translate:
	case mr_rotate:
	case mr_cylinder:
	case mr_nupatch:
	case mr_option:
	case mr_shadinginterpolation:
	case mr_shadingrate:
	case mr_framebegin:
	case mr_frameend:
	case mr_curves:
	case mr_points:
	case mr_maketexture:
	case mr_declare:
	    if (token == mr_worldbegin) {
		worldptr = ptr;
	    }
	    check_buffer(7+table[token].len);
	    if(!table[token].defined) {
		/* we must define it */
		*(ptr++)=RIREQDEF;
		*(ptr++)=table[token].reqn;
		if(table[token].len<16) {
		    *(ptr++)=STRINGENCODE+table[token].len;
		} else {
		    unsigned short length;
		    length = (unsigned short) table[token].len;
		    *(ptr++)=LNGSTRINGENCODE;
		    COPYUSHORT(length);
		}
		cat(ptr,table[token].name);
		table[token].defined = 1;
	    }
	    *(ptr++)=RIREQREF;
	    *(ptr++)=table[token].reqn;
	    break;
    
	case mr_array: {
	    char arraycount;
	    float f;
	    arraycount = (char)va_arg(*alist, int);
	    check_buffer(2+4*arraycount);
	    *(ptr++)=FLOATARRAYDEF;
	    *(ptr++)=arraycount;
	    for(i=0;i<arraycount;i++) {
		/* will float be the same on each platform? (ieee?)*/
		/* IS THIS NEXT STEP LEGAL?!? */
		f = (float) va_arg(*alist, double);
		COPYFLOAT(f);
	    }
	    break;
	}
    
	case mr_buildarray: {
	    char arraycount;
	    arraysize = va_arg(*alist, int);
	    arraycount = (char)arraysize;
	    check_buffer(2);
	    *(ptr++)=FLOATARRAYDEF;
	    *(ptr++)=arraycount;
	    expectSubArray = 1;
	    break;
	}
    
	case mr_subarray3: {
	    arraysize-=3;
	    if(arraysize<0) { /* ERROR */ };
	    check_buffer(12); /* 3*sizeof(float) */
	    floatptr = va_arg(*alist, float*);
	    COPYFLOAT(floatptr[0]);
	    COPYFLOAT(floatptr[1]);
	    COPYFLOAT(floatptr[2]);
	    if(arraysize<=0) {
		expectSubArray = 0;
	    }
	    break;
	}
  
	case mr_subarray2: {
	    arraysize-=2;
	    if(arraysize<0) { /* ERROR */ };
	    check_buffer(8); /* 3*sizeof(float) */
	    floatptr = va_arg(*alist, float*);
	    COPYFLOAT(floatptr[0]);
	    COPYFLOAT(floatptr[1]);
	    if(arraysize<=0) {
		expectSubArray = 0;
	    }
	    break;
	}

	case mr_parray: {
	    char arraycount;
	    arraycount = (char)va_arg(*alist, int);
	    check_buffer(2+arraycount*4);
	    *(ptr++)=FLOATARRAYDEF;
	    *(ptr++)=arraycount;
	    floatptr = va_arg(*alist, float*);
	    for (i = 0; i < arraycount; i++) {
		COPYFLOAT(floatptr[i]);
	    }
	    if(arraysize<=0) {
		expectSubArray = 0;
	    }
	    break;
	}

	case mr_int: {
	    unsigned short number;
	    check_buffer(3);
	    number = (unsigned short)va_arg(*alist, int);
	    *(ptr++) = INTEGER;
	    COPYUSHORT(number);
	    break;
	}

	case mr_float: {
	    float afloat;
	    check_buffer(5);
	    afloat = (float)va_arg(*alist, double);
	    *(ptr++) = FLOAT;
	    COPYFLOAT(afloat);
	    break;
	}
	
	case mr_intseq: {
	    /* OBSOLETE */
	    unsigned short number;
	    int  count;
	    count = va_arg(*alist, int);
	    check_buffer(count*2);
	    for(i=0;i<count;i++) {
		number = (unsigned short)va_arg(*alist, int);
		*(ptr++) = INTEGER;
		COPYUSHORT(number);
	    }
	    break;
	}

	case mr_string: {
	    unsigned short length;
	    char *string;
	    string = va_arg(*alist, char *);
	    length = (unsigned short) strlen(string);
	    check_buffer(3+length);
	    if(length<16) {
		*(ptr++)=STRINGENCODE+(char)length;
	    } else {
		*(ptr++)=LNGSTRINGENCODE;
		COPYUSHORT(length);
	    }
	    cat(ptr,string);
	    break;
	}
    
	case mr_embed: {
	    /* no attempt is made to encode this becuase it's so general */
	    char *string;
	    int length;
	    string = va_arg(*alist, char *);
	    length = strlen(string);
	    check_buffer(length);
	    cat(ptr,string);
	    break;
	}
    
	case mr_nl:
	    /* temporary for debugging (puts tokens on spereate lines) */
	    /* printf("\n"); */
	    break;
    
	} /* switch */    
    } while ((token=va_arg(*alist, int))!=mr_NULL);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
