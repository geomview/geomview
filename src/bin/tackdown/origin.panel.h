/** Header file generated with fdesign on Sun Jul 30 00:37:57 2006.**/

#ifndef FD_MainForm_h_
#define FD_MainForm_h_

/** Callbacks, globals and object handlers **/
extern void ShowButtonProc(FL_OBJECT *, long);
extern void DoneButtonProc(FL_OBJECT *, long);
extern void CancelButtonProc(FL_OBJECT *, long);
extern void QuitButtonProc(FL_OBJECT *, long);



/**** Forms and Objects ****/
extern FL_FORM *MainForm;

extern FL_OBJECT
        *ShowButton,
        *DoneButton,
        *CancelButton,
        *Instruc1,
        *Instruc2,
        *Instruc3,
        *TargetInput,
        *Instruc4;

extern FL_FORM *ErrorForm;

extern FL_OBJECT
        *Error1,
        *Error2,
        *Error3,
        *OkButton;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_MainForm_h_ */
