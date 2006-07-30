/** Header file generated with fdesign on Sun Jul 30 02:40:09 2006.**/

#ifndef FD_TransForm_h_
#define FD_TransForm_h_

/** Callbacks, globals and object handlers **/
extern void ApplyProc(FL_OBJECT *, long);
extern void SetProc(FL_OBJECT *, long);
extern void ReadProc(FL_OBJECT *, long);
extern void DoneProc(FL_OBJECT *, long);
extern void IdentityProc(FL_OBJECT *, long);
extern void TranslateButtonProc(FL_OBJECT *, long);
extern void RotateMode(FL_OBJECT *, long);
extern void TranslateMode(FL_OBJECT *, long);
extern void ScaleMode(FL_OBJECT *, long);
extern void RotateButtonProc(FL_OBJECT *, long);
extern void ScaleButtonProc(FL_OBJECT *, long);
extern void CoordSysProc(FL_OBJECT *, long);



/**** Forms and Objects ****/
extern FL_FORM *TransForm;

extern FL_OBJECT
        *t00,
        *t01,
        *t02,
        *t03,
        *t10,
        *t11,
        *t12,
        *t13,
        *t20,
        *t21,
        *t22,
        *t30,
        *t31,
        *t32,
        *t33,
        *ApplyButton,
        *SetButton,
        *ReadButton,
        *DoneButton,
        *IdentityButton,
        *targetGroup,
        *actGeom,
        *actCam,
        *TranslateGroup,
        *TranslationAxisX,
        *TranslationAxisY,
        *TranslationAxisZ,
        *TranslationMeasure,
        *TranslateButton,
        *ModeGroup,
        *RotateModeButton,
        *TranslateModeButton,
        *ScaleModeButton,
        *RotateGroup,
        *RotationAxisY,
        *RotationAxisZ,
        *RotateButton,
        *RotationMeasure,
        *RotationAxisX,
        *t23,
        *ScaleGroup,
        *ScaleMeasure,
        *ScaleButton,
        *ScaleX,
        *ScaleY,
        *ScaleZ,
        *CoordSysGroup,
        *CameraButton,
        *SelfButton,
        *WorldButton;

extern FL_FORM *fooform;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_TransForm_h_ */
