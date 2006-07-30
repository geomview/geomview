/** Header file generated with fdesign on Sat Jul 29 21:37:44 2006.**/

#ifndef FD_MainForm_h_
#define FD_MainForm_h_

/** Callbacks, globals and object handlers **/
extern void ScaleProc(FL_OBJECT *, long);
extern void QuitProc(FL_OBJECT *, long);
extern void InfoProc(FL_OBJECT *, long);
extern void PathProc(FL_OBJECT *, long);
extern void GoProc(FL_OBJECT *, long);
extern void TilingProc(FL_OBJECT *, long);
extern void SpeedProc(FL_OBJECT *, long);

extern void DoneProc(FL_OBJECT *, long);
extern void DiagProc(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *MainForm;

extern FL_OBJECT
        *DodecScale,
        *Quit,
        *Info,
        *PathGroup,
        *Direct,
        *Equi,
        *Quarter,
        *Loop,
        *GoGroup,
        *Go,
        *Stop,
        *TileGroup,
        *Level3,
        *Level2,
        *Level1,
        *Level0,
        *SpeedGroup,
        *Speed2,
        *Speed3,
        *Speed4,
        *Speed1;

extern FL_FORM *HelpForm;

extern FL_OBJECT
        *HelpBrowser,
        *Done,
        *DiagramGroup,
        *EucDiag,
        *HypDiag;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_MainForm_h_ */
