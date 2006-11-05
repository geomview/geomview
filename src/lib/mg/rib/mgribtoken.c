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

#define STRINGDEF	0315
#define STRINGREF	0317
#define LNGSTRINGENCODE	0241   /* length follows, is unsigned short */
#define STRINGENCODE	0220
#define RIREQDEF	0314
#define RIREQREF	0246
#define FLOATARRAYDEF	0310
#define INTEGER		0201   /* unsigned short */
#define FLOAT		0244

void binary_token(int a1, va_list *alist);
void ascii_token(int a1, va_list *alist);

unsigned char *tokenbuffer=NULL;
unsigned char *limit;
unsigned char *ptr;

/* following is the table: see mgribtoken.h (struct) for desc of fields */
static struct _table table[] = {
    {"", 0, 0, 0},		/* mr_NULL */

    /* Ri Requests */
    {"AttributeBegin",		14,  0, 0},
    {"AttributeEnd",		12,  1, 0},
    {"TransformBegin",		14,  2, 0},
    {"TransformEnd",		12,  3, 0},
    {"Identity",		 8,  4, 0},
    {"ConcatTransform",		15,  5, 0},
    {"Surface",			 7,  6, 0},
    {"ShadingInterpolation",	20,  7, 0},
    {"Color",		 	 5,  8, 0},
    {"Opacity",			 7,  9, 0},
    {"Polygon",			 7, 10, 0},
    {"PatchMesh",		 9, 11, 0},
    {"Format",			 6, 12, 0},
    {"Projection",		10, 13, 0},
    {"Clipping",		 8, 14, 0},
    {"WorldBegin",		10, 15, 0},
    {"WorldEnd",		 8, 16, 0},
    {"Display",			 7, 17, 0},
    {"ScreenWindow",		12, 19, 0},
    {"LightSource",		11, 20, 0},
    {"Sphere",			 6, 21, 0},
    {"Translate",		 9, 22, 0},
    {"Rotate",			 6, 23, 0},
    {"Cylinder",		 8, 24, 0},
    {"NuPatch",			 7, 25, 0},
    {"ShadingRate",		11, 26, 0},
    {"Option",			 6, 27, 0},
    {"Illuminate",		10, 28, 0},
    {"FrameBegin",		10, 29, 0},
    {"FrameEnd",		 8, 30, 0},
    {"ReverseOrientation",	18, 31, 0},
    {"Curves",			 6, 32, 0},
    {"Points",			 6, 33, 0},

    /* following are reserved - do not add */
    /* or remove fields, just change them! */
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},
    {"", 0, 255, 0},

    /* Strings - we start these out at position 50 */
    {"P",			 1,  0, 0},
    {"N",			 1,  1, 0},
    {"Cs",            	 	 2,  2, 0},
    {"Pw",			 2,  3, 0},
    {"Os",			 2,  4, 0},
    {"st",			 2,  5, 0},
    {"plastic",			 7,  6, 0},
    {"paintedplastic",		14,  7, 0},
    {"hplastic",		 8,  8, 0},
    {"eplastic",		 8,  9, 0},
    {"heplastic",		 9, 10, 0},
    {"constant",		 8, 11, 0},
    {"ambientlight",		12, 12, 0},
    {"lightcolor",		10, 13, 0},
    {"distantlight",		12, 14, 0},
    {"intensity",		 9, 15, 0},
    {"file",			 4, 16, 0},
    {"rgb",			 3, 17, 0},
    {"rgba",			 4, 18, 0},
    {"Ka",			 2, 19, 0},
    {"Kd",			 2, 20, 0},
    {"Ks",			 2, 21, 0},
    {"specularcolor",		13, 22, 0}, 
    {"roughness",		 9, 23, 0},
    {"fov",			 3, 24, 0},
    {"perspective",		11, 25, 0},
    {"to",			 2, 26, 0},
    {"framebuffer",		11, 27, 0},
    {"texturename",		11, 28, 0},
    {"width",			 5, 29, 0},
    {"constantwidth",		13, 30, 0},
};

/* initialize tokenbuffer */
void mrti_init()
{
    if(tokenbuffer) free(tokenbuffer);
    tokenbuffer = (unsigned char *)malloc(BUFFERSIZE);
    tokenbuffer[0] = (char)0;
    limit = (unsigned char *)(tokenbuffer + BUFFERSIZE);
    ptr = tokenbuffer;
}

/* reset the ptr & tokenbuffer */
void mrti_reset()
{
	ptr = tokenbuffer;
	tokenbuffer[0] = (char)0;
}

/* quick copy routine w/ ptr update */
void cat(unsigned char *s, char *a)
{
    while((*(s++)=(*(a++)))) ptr++;
}


/* routine will check buffer for possible overun, will realloc if needed */
/* generally, "worst case" values should be passed to check_buffer       */
void check_buffer(int length)
{
    length += 8;
    if((unsigned char *)(ptr+length)>(unsigned char *)limit) {
        /* optinally check for maximum buffer allocation here */
	long used = ptr - tokenbuffer;
	long avail = limit - tokenbuffer;
	    /* Buffer grows exponentially, by factor of 1.5 each time */
	do { avail += avail>>1; } while(used+length >= avail);
	tokenbuffer = (unsigned char *)realloc(tokenbuffer, avail);
	ptr = tokenbuffer + used;
	limit = tokenbuffer + avail;
    }
}

/* process variable size token list */
void mrti(int a1, ... )
{
    va_list alist;

    va_start(alist, a1);
 
    switch((int)(_mgribc->render_device & (RMD_BINARY|RMD_ASCII)))
    {
	case (int)RMD_BINARY: binary_token(a1, &alist); break;
	case (int)RMD_ASCII:  ascii_token(a1, &alist); break;
    }
    va_end(alist);
}

/* return 1 when supplied token requires its own (ascii) line, 0 otherwise */
int line_initializer(int token)
{
    /* THIS IS HACKISH - IT WILL CREATE A LINE FEED BEFORE */
    /* IT CREATES THE NEW LINE, WHICH ISN'T ALWAYS GOOD!!  */
    if(token<STRINGBASE||token==mr_comment||token==mr_section||
       (token>=mr_P && token<=mr_Os)) return 1;
    else return 0;
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
    
    if(expectSubArray && (token!=mr_subarray3)) {
    	/* ERROR */
    }
    
    /* check to see if we need to start a new line */
    if(line_initializer(token) && *(ptr-1)!='\n') *(ptr++)='\n';
    
    switch(token) {
	
    case mr_section:
	check_buffer(SECTIONWIDTH); /* should not exceed this */
        s = va_arg(*alist, char*);
        len = strlen(s);
	if(len+3>SECTIONWIDTH) len = SECTIONWIDTH-3; /* 3 added characters */
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
	    sprintf(astring,"%g ",nextfloat);
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

    case mr_parray:
    {
    	int size;
    	size = va_arg(*alist, int);
	check_buffer(2+16*size);
	*(ptr++)='[';
	/* if arraysize<0 then ERROR */
	floatptr = va_arg(*alist, float*);
	for(i=0;i<size;i++) {
	    sprintf(astring,"%g ",*(floatptr++));
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
    	sprintf(astring,"%g %g %g   ",
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
    	sprintf(astring,"%g %g   ",
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
	sprintf(astring,"%g ",nextfloat);
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

    case mr_string:
    {
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

    case mr_embed:
    {
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
    case mr_worldbegin:
    case mr_worldend:
    case mr_frameend:
    case mr_reverseorientation:
	check_buffer(table[token].len);
	cat(ptr,table[token].name);
	/*
	*(ptr++) = '\n';
	*/
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

#define COPYUSHORT( value )	*ptr++ = ((char *)&value)[0];\
				*ptr++ = ((char *)&value)[1]
#define COPYFLOAT( value )	*ptr++ = ((char *)&value)[0];\
				*ptr++ = ((char *)&value)[1];\
				*ptr++ = ((char *)&value)[2];\
				*ptr++ = ((char *)&value)[3]

void binary_token(int token, va_list *alist)
{
int i;
static int expectSubArray=0;
static int arraysize;
float *floatptr;

    if(expectSubArray && (token!=mr_subarray3)) {
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
    
    case mr_array:
    {
        unsigned char arraycount;
	float f;
    	arraycount = (unsigned char)va_arg(*alist, int);
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
    
    case mr_buildarray:
    {
    	unsigned char arraycount;
	arraysize = va_arg(*alist, int);
	arraycount = (unsigned char)arraysize;
	check_buffer(2);
	*(ptr++)=FLOATARRAYDEF;
	*(ptr++)=arraycount;
	expectSubArray = 1;
	break;
    }
    
    case mr_subarray3:
    {
    	arraysize-=3;
	if(arraysize<0) /* ERROR */;
	check_buffer(12); /* 3*sizeof(float) */
    	floatptr = va_arg(*alist, float*);
	memcpy((char *)ptr, (char *)floatptr, 3*sizeof(float));
	ptr+=3*sizeof(float);
	if(arraysize<=0) {
	    expectSubArray = 0;
	}
	break;
    }
  
    case mr_parray:
    {
    	unsigned char arraycount;
	arraycount = (unsigned char)va_arg(*alist, int);
	check_buffer(2+arraycount*4);
	*(ptr++)=FLOATARRAYDEF;
	*(ptr++)=arraycount;
	floatptr = va_arg(*alist, float*);
	memcpy((char *)ptr, (char *)floatptr, arraycount*sizeof(float));
	ptr+=arraycount*sizeof(float);
	if(arraysize<=0) {
	    expectSubArray = 0;
	}
	break;
    }

    case mr_int:
    {
    	unsigned short number;
	check_buffer(3);
        number = (unsigned short)va_arg(*alist, int);
	*(ptr++) = INTEGER;
	COPYUSHORT(number);
	break;
    }

    case mr_float:
    {
    	float afloat;
	check_buffer(5);
        afloat = (float)va_arg(*alist, double);
	*(ptr++) = FLOAT;
	COPYFLOAT(afloat);
	break;
    }
	
    case mr_intseq:
    {
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

    case mr_string:
    {
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
    
    case mr_embed:
    {
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
