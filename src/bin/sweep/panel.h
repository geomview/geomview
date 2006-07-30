/** Header file generated with fdesign on Sat Jul 29 21:15:47 2006.**/

#ifndef FD_MainForm_h_
#define FD_MainForm_h_

/** Callbacks, globals and object handlers **/
extern void GoButtonProc(FL_OBJECT *, long);
extern void QuitButtonProc(FL_OBJECT *, long);
extern void RotSweepProc(FL_OBJECT *, long);
extern void TransSweepProc(FL_OBJECT *, long);



/**** Forms and Objects ****/
extern FL_FORM *MainForm;

extern FL_OBJECT
        *TransGroup,
        *Translength,
        *Transxdir,
        *Transydir,
        *Transzdir,
        *TypeGroup,
        *RotSweep,
        *TransSweep,
        *RotGroup,
        *Rotlength,
        *DegButton,
        *RadButton,
        *Rotendz,
        *Rotendy,
        *Rotendx,
        *Rotdirz,
        *Rotdiry,
        *Rotdirx,
        *Rotdivisions;

extern FL_FORM *FooForm;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_MainForm_h_ */
