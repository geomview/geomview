/** Header file generated with fdesign on Sat Jul 29 20:50:57 2006.**/

#ifndef FD_MainForm_h_
#define FD_MainForm_h_

/** Callbacks, globals and object handlers **/
extern void QuitProc(FL_OBJECT *, long);
extern void UndoButtonProc(FL_OBJECT *, long);
extern void FileButtonProc(FL_OBJECT *, long);
extern void HelpButtonProc(FL_OBJECT *, long);
extern void InfoButtonProc(FL_OBJECT *, long);
extern void SpaceBrowserProc(FL_OBJECT *, long);
extern void ResetButtonProc(FL_OBJECT *, long);
extern void AngleInputProc(FL_OBJECT *, long);

extern void FileInputProc(FL_OBJECT *, long);
extern void FileOKButtonProc(FL_OBJECT *, long);
extern void FileCancelButtonProc(FL_OBJECT *, long);

extern void InfoOKButtonProc(FL_OBJECT *, long);

extern void HelpOKButtonProc(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *MainForm;

extern FL_OBJECT
        *QuitButton,
        *UndoButton,
        *FileButton,
        *HelpButton,
        *InfoButton,
        *SpaceBrowser,
        *ResetButton,
        *MainFormLabel,
        *AngleInput;

extern FL_FORM *FileForm;

extern FL_OBJECT
        *FileInput,
        *FileOKButton,
        *FileCancelButton;

extern FL_FORM *InfoForm;

extern FL_OBJECT
        *InfoFormLabel,
        *InfoOKButton;

extern FL_FORM *HelpForm;

extern FL_OBJECT
        *HelpBrowser,
        *HelpOKButton;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_MainForm_h_ */
