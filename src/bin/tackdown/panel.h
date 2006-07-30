/** Header file generated with fdesign on Sun Jul 30 00:32:36 2006.**/

#ifndef FD_MainForm_h_
#define FD_MainForm_h_

/** Callbacks, globals and object handlers **/
extern void TransButtonProc(FL_OBJECT *, long);
extern void QuitButtonProc(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *MainForm;

extern FL_OBJECT
        *TransButton,
        *worldButton;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_MainForm_h_ */
