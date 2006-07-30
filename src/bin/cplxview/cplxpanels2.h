/** Header file generated with fdesign on Sun Jul 30 01:38:43 2006.**/

#ifndef FD_cplxmainpanel_h_
#define FD_cplxmainpanel_h_

/** Callbacks, globals and object handlers **/
extern void functioncallback(FL_OBJECT *, long);
extern void helpcallback(FL_OBJECT *, long);
extern void quitcallback(FL_OBJECT *, long);
extern void sliderscallback(FL_OBJECT *, long);
extern void coordtypecallback(FL_OBJECT *, long);
extern void meshlabelcallback(FL_OBJECT *, long);
extern void rangelabelcallback(FL_OBJECT *, long);
extern void domaincallback(FL_OBJECT *, long);

extern void quitcplxhelpcallback(FL_OBJECT *, long);

extern void slidercallback(FL_OBJECT *, long);
extern void slidershidecallback(FL_OBJECT *, long);

extern void coordcallback(FL_OBJECT *, long);
extern void coordtypehidecallback(FL_OBJECT *, long);

extern void usercoordcallback(FL_OBJECT *, long);
extern void usercoordhidecallback(FL_OBJECT *, long);

extern void meshcallback(FL_OBJECT *, long);
extern void meshlabelhidecallback(FL_OBJECT *, long);

extern void rangecallback(FL_OBJECT *, long);
extern void rangelabelhidecallback(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *cplxmainpanel;

extern FL_OBJECT
        *functionbox,
        *messagebox,
        *helpbutton,
        *quitbutton,
        *funclabelbutton,
        *coordtypebutton,
        *meshlabelbutton,
        *rangelabelbutton,
        *x1box,
        *y1box,
        *x0box,
        *y0box;

extern FL_FORM *cplxhelppanel;

extern FL_OBJECT
        *cplxhelpbrowser,
        *quitcplxhelpbutton;

extern FL_FORM *sliderspanel;

extern FL_OBJECT
        *abox,
        *bbox,
        *slidershidebox;

extern FL_FORM *coordtypepanel;

extern FL_OBJECT
        *rectcoordbox,
        *polarcoordbox,
        *usercoordbox,
        *coordtypehidebox;

extern FL_FORM *usercoordpanel;

extern FL_OBJECT
        *suvbox,
        *tuvbox,
        *usercoordhidebox;

extern FL_FORM *meshlabelpanel;

extern FL_OBJECT
        *m1box,
        *m2box,
        *meshlabelhidebox;

extern FL_FORM *rangelabelpanel;

extern FL_OBJECT
        *realrangebutton,
        *imagrangebutton,
        *d4rangebutton,
        *hiderangelabelbutton;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_cplxmainpanel_h_ */
