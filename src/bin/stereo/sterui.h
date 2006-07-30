/** Header file generated with fdesign on Sun Jul 30 02:32:58 2006.**/

#ifndef FD_stereo_h_
#define FD_stereo_h_

/** Callbacks, globals and object handlers **/
extern void StereoProc(FL_OBJECT *, long);
extern void SwapProc(FL_OBJECT *, long);
extern void QuitProc(FL_OBJECT *, long);
extern void FocalProc(FL_OBJECT *, long);
extern void HelpProc(FL_OBJECT *, long);
extern void ConvProc(FL_OBJECT *, long);
extern void MoreProc(FL_OBJECT *, long);

extern void DoneProc(FL_OBJECT *, long);

extern void OcularSepProc(FL_OBJECT *, long);
extern void ScreenWidthProc(FL_OBJECT *, long);
extern void DoneProc(FL_OBJECT *, long);
extern void CamNameProc(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *stereo;

extern FL_OBJECT
        *StereoBrowser,
        *SwapButton,
        *QuitButton,
        *BestViewText,
        *FocalButton,
        *HelpButton,
        *ConvSlider,
        *MoreButton;

extern FL_FORM *Help;

extern FL_OBJECT
        *HelpBrowser,
        *DoneButton;

extern FL_FORM *More;

extern FL_OBJECT
        *OcularInput,
        *ScreenWidthInput,
        *DoneButton,
        *FixedCamButton,
        *CamNameInput;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_stereo_h_ */
