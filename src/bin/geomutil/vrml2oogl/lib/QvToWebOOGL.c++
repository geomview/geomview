#include <QvElement.h>
#include <QvNodes.h>
#include <QvState.h>
#include <QvToWebOOGL.h>
#include <QvMaterialBinding.h>
#include <QvNormalBinding.h>
#include <QvUnknownNode.h>
#include <QvInfo.h>

extern "C" {
#include <unistd.h>
}

//////////////////////////////////////////////////////////////////////////////
//
// ToWebOOGL will traverse and convert any vrml object to WebOOGL!
// Because ToWebOOGL() is defined in the header for ALL node
// classes, each one has an implementation here.
//
//////////////////////////////////////////////////////////////////////////////

// For debugging
static int indent = 0;
static int oogldepth = 0, unique = 0, mypid = 0;
static char out[5000];
static char colstr[100];
enum Binding {
        DEFAULT,
        OVERALL,
        PER_PART,
        PER_PART_INDEXED,
        PER_FACE,
        PER_FACE_INDEXED,
        PER_VERTEX,
        PER_VERTEX_INDEXED,
};


static void
OOGLannounce(const char *className)
{
/*
    for (int i = 0; i < indent; i++)
	fprintf(stderr,"\t");
    fprintf(stderr,"Converting a %s to WebOOGL - depth: %d\n", className, oogldepth);
*/
}
#define ANNOUNCE(className) OOGLannounce(QV__QUOTE(className))

static FILE *OOGLfout = (FILE *)NULL;
static char *OOGLhand = NULL;
static char *urls[100];

int OOGLnumuniq()
{
  return unique;
}

char *
OOGLgeturls(int i)
{
  return urls[i];
}

int
OOGLhandle(char *handle)
{
  OOGLhand = handle;
  return 1;
}

int
OOGLfile(FILE *fout)
{
  OOGLfout = fout;
  mypid = (int) getpid();
  return 1;
}

static void
OOGLout(const char *text)
{
    if (OOGLfout == NULL) return;

    for (int i = 1; i < indent; i++)
	fprintf(OOGLfout, " ");
    fprintf(OOGLfout, "%s\n", text);
}

static void
OOGLflush()
{
    if(OOGLfout != NULL)
	fflush(OOGLfout);
}

static void
OOGLoutindent()
{
    if (OOGLfout == NULL) return;

    for (int i = 1; i < indent; i++)
	fprintf(OOGLfout, " ");
}

static void
OOGLoutraw(const char *text)
{
    if (OOGLfout == NULL) return;

    fprintf(OOGLfout, "%s", text);
}


static void
OOGLgetcolor(QvMaterial *mt, QvMaterialBinding *mtb, int mtnum)
{
   // float *amb;
   float *em, *dif;
   float a;

   if (mt == NULL || mtnum < 0) { sprintf(colstr, "1 1 1 1"); return; }

   a = 1 - (mt->transparency).values[mtnum % (mt->transparency).num];
   em = &mt->emissiveColor.values[3 * (mtnum % mt->emissiveColor.num)];
   dif = &mt->diffuseColor.values[3 * (mtnum % mt->diffuseColor.num)];
   if (em[0] != 0.0 || em[1] != 0.0 || em[2] != 0.0) {
      // Funky calculations to simulate emissive color
      float r, g, b, ln;
      r = em[0];
      g = em[1];
      b = em[2];
      ln = (3.0 - sqrt(r*r + g*g + b*b)) / 6.0;
      r += (1.0 - r)*ln; g += (1.0 - g)*ln; b += (1.0 - b)*ln;
      sprintf(colstr, "%.3f %.3f %.3f %.3f", r, g, b, a);
    }
    else
      sprintf(colstr, "%.3f %.3f %.3f %.3f", dif[0], dif[1], dif[2], a);

}

static void
OOGLappearance(QvMaterial *mt)
{

    if (mt == NULL) return;

    OOGLout("appearance { material {");
    sprintf(out, "ambient %f %f %f", (mt->ambientColor).values[0],
                (mt->ambientColor).values[1], (mt->ambientColor).values[2]);
    OOGLout(out);

    float r, g, b, ln;

    if (!((mt->emissiveColor).values[0]==0.0 &&
	(mt->emissiveColor).values[1]==0.0 &&
                (mt->emissiveColor).values[2]==0.0))
    {
      // Funky calculations to simulate emissive color
      r = (mt->emissiveColor).values[0];
      g = (mt->emissiveColor).values[1];
      b = (mt->emissiveColor).values[2];
      ln = (3.0 - sqrt(r*r + g*g + b*b)) / 6.0;
      r += (1.0 - r)*ln; g += (1.0 - g)*ln; b += (1.0 - b)*ln;
    }
    else {
	r = mt->diffuseColor.values[0];
	g = mt->diffuseColor.values[1];
	b = mt->diffuseColor.values[2];
    }
    sprintf(out, "diffuse %.3f %.3f %.3f  edgecolor %.3f %.3f %.3f", r, g, b, r, g, b);
    OOGLout(out);

    sprintf(out, "specular %.3f %.3f %.3f", (mt->specularColor).values[0],
                (mt->specularColor).values[1], (mt->specularColor).values[2]);
    OOGLout(out);
    sprintf(out, "shininess %.3f", (mt->shininess).values[0]);
    OOGLout(out);
    sprintf(out, "alpha %.3f", 1.0 - (mt->transparency).values[0]);
    OOGLout(out);
//  Doesn't work with Geomview right now
//  sprintf(out, "emission %f %f %f", (mt->emissiveColor).values[0],
//              (mt->emissiveColor).values[1], (mt->emissiveColor).values[2]);

    OOGLout(out);
    OOGLout("}}");
}

#define DEFAULT_TRAVERSE(className)					      \
void									      \
className::ToWebOOGL(QvState *)						      \
{									      \
    ANNOUNCE(className);						      \
}

//////////////////////////////////////////////////////////////////////////////
//
// Groups.
//
//////////////////////////////////////////////////////////////////////////////

void
QvGroup::ToWebOOGL(QvState *state)
{
    int savedepth, i;

    ANNOUNCE(QvGroup);
    OOGLout("{ = LIST");
    OOGLout("# Group");

    savedepth = oogldepth;

    indent++;
    for (i = 0; i < getNumChildren(); i++)
	getChild(i)->ToWebOOGL(state);
    indent--;

    savedepth = oogldepth - savedepth; /* How deep did we get ? :-)*/

    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }

    out[savedepth*2] = '\0';
    OOGLout(out);
    OOGLout("}			# End Group");
    oogldepth = oogldepth - savedepth;
}

void
QvLOD::ToWebOOGL(QvState *state)
{
    int savedepth, i;

    ANNOUNCE(QvLOD);
    savedepth = oogldepth;

    indent++;

    // ??? In a real implementation, this would choose a child based
    // ??? on the projected screen areas.
    // WebOOGL punts on this one

    if (getNumChildren() > 0)
	getChild(0)->ToWebOOGL(state);

    indent--;

    savedepth = oogldepth - savedepth; /* How deep did we get ? :-) */
	
    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }

    out[savedepth*2] = '\0';
    OOGLout(out);
    oogldepth = oogldepth - savedepth;
}

void
QvSeparator::ToWebOOGL(QvState *state)
{
    int savedepth, i;

    ANNOUNCE(QvSeparator);

    OOGLout("{ = LIST		# Separator");
    savedepth = oogldepth;

    state->push();
    indent++;
    for (i = 0; i < getNumChildren(); i++)
	getChild(i)->ToWebOOGL(state);
    indent--;
    state->pop();

    savedepth = oogldepth - savedepth; /* How deep did we get ? :-)*/

    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }
    out[savedepth*2] = '\0';
    OOGLout(out);
    OOGLout("}			# End Separator");
    oogldepth = oogldepth - savedepth;
}

void
QvSwitch::ToWebOOGL(QvState *state)
{
  int savedepth, i;


    ANNOUNCE(QvSwitch);
    indent++;

    int which = (int) whichChild.value;

    savedepth = oogldepth;

    if (which == QV_SWITCH_NONE)
	;

    else if (which == QV_SWITCH_ALL)
	for (i = 0; i < getNumChildren(); i++)
	    getChild(i)->ToWebOOGL(state);

    else
	if (which < getNumChildren())
	    getChild(which)->ToWebOOGL(state);

    indent--;
    savedepth = oogldepth - savedepth; /* How deep did we get ? :-)*/

    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }
    out[savedepth*2] = '\0';
    OOGLout(out);
    oogldepth = oogldepth - savedepth;
}

void
QvTransformSeparator::ToWebOOGL(QvState *state)
{
    int savedepth, i;

    ANNOUNCE(QvTransformSeparator);

    // We need to "push" just the transformation stack. We'll
    // accomplish this by just pushing a no-op transformation onto
    // that stack. When we "pop", we'll restore that stack to its
    // previous state.

    QvElement *markerElt = new QvElement;
    markerElt->data = this;
    markerElt->type = QvElement::NoOpTransform;
    state->addElement(QvState::TransformationIndex, markerElt);

    savedepth = oogldepth;

    indent++;
    for (i = 0; i < getNumChildren(); i++)
	getChild(i)->ToWebOOGL(state);
    indent--;

    // Now do the "pop"
    while (state->getTopElement(QvState::TransformationIndex) != markerElt)
	state->popElement(QvState::TransformationIndex);

    savedepth = oogldepth - savedepth; /* How deep did we get ? :-)*/

    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }
    out[savedepth*2] = '\0';
    OOGLout(out);
    oogldepth = oogldepth - savedepth;
}

//////////////////////////////////////////////////////////////////////////////
//
// Properties.
//
//////////////////////////////////////////////////////////////////////////////

void
QvMaterial::ToWebOOGL(QvState *state)
{
    oogldepth++;
    ANNOUNCE(QvMaterial);
    QvElement *elt = new QvElement;
    elt->data = this;
    state->addElement(QvState::MaterialIndex, elt);
    OOGLout("{ = INST");
    OOGLout("geom {");
    OOGLappearance(this);
    OOGLout("= LIST");
}

#define DO_PROPERTY(className, stackIndex)				      \
void									      \
className::ToWebOOGL(QvState *state)					      \
{									      \
    ANNOUNCE(className);						      \
    QvElement *elt = new QvElement;					      \
    elt->data = this;							      \
    state->addElement(QvState::stackIndex, elt);			      \
}

DO_PROPERTY(QvCoordinate3,		Coordinate3Index)
DO_PROPERTY(QvMaterialBinding,		MaterialBindingIndex)
DO_PROPERTY(QvNormal,			NormalIndex)
DO_PROPERTY(QvNormalBinding,		NormalBindingIndex)
DO_PROPERTY(QvShapeHints,		ShapeHintsIndex)
DO_PROPERTY(QvFontStyle,		FontStyleIndex)

// WebOOGL punts on these ... (no support for texture mapping)
DO_PROPERTY(QvTextureCoordinate2,	TextureCoordinate2Index)
DO_PROPERTY(QvTexture2,			Texture2Index)
DO_PROPERTY(QvTexture2Transform,	Texture2TransformationIndex)


#define DO_TYPED_PROPERTY(className, stackIndex, eltType)		      \
void									      \
className::ToWebOOGL(QvState *state)					      \
{									      \
    ANNOUNCE(className);						      \
    QvElement *elt = new QvElement;					      \
    elt->data = this;							      \
    elt->type = QvElement::eltType;					      \
    state->addElement(QvState::stackIndex, elt);			      \
}

void
QvMatrixTransform::ToWebOOGL(QvState *state)
{
    oogldepth++;
    ANNOUNCE(QvMatrixTransform);
    QvElement *elt = new QvElement;
    elt->data = this;
    elt->type = QvElement::MatrixTransform;
    state->addElement(QvState::TransformationIndex, elt);
    OOGLout("{ = INST");
    OOGLout("transform {");
    sprintf(out, "%f %f %f %f", matrix.value[0][0], matrix.value[0][1],
				matrix.value[0][2], matrix.value[0][3]);
    OOGLout(out);
    sprintf(out, "%f %f %f %f", matrix.value[1][0], matrix.value[1][1],
				matrix.value[1][2], matrix.value[1][3]);
    OOGLout(out);
    sprintf(out, "%f %f %f %f", matrix.value[2][0], matrix.value[2][1],
				matrix.value[2][2], matrix.value[2][3]);
    OOGLout(out);
    sprintf(out, "%f %f %f %f", matrix.value[3][0], matrix.value[3][1],
				matrix.value[3][2], matrix.value[3][3]);
    OOGLout(out);
    OOGLout("} geom");
    OOGLout("{ = LIST");
}

static void OOGLoutmat(QvSFMatrix *mat1)
{
  int y;

  for (y = 0; y < 4; y++) {
    sprintf(out, "%f %f %f %f", mat1->value[0][y], mat1->value[1][y],
	    mat1->value[2][y], mat1->value[3][y]);
    OOGLout(out);
  }
}

static void OOGLmat_init(QvSFMatrix *mat1)
{
  int x, y;

  for (x = 0; x < 4; x++) 
    for (y = 0; y < 4; y++) {
      if (x == y) 
	mat1->value[x][y] = 1.0;
      else 
	mat1->value[x][y] = 0.0;
    }
}

static void OOGLmat_mult(QvSFMatrix *mat1, QvSFMatrix *mat2, 
				QvSFMatrix *result)
{
  int x,y;

  for (y = 0; y < 4; y++) {
    for (x = 0; x < 4; x++) {
      result->value[x][y] = mat1->value[0][y] * mat2->value[x][0]
      + mat1->value[1][y] * mat2->value[x][1]
      + mat1->value[2][y] * mat2->value[x][2]
      + mat1->value[3][y] * mat2->value[x][3];

    }
  }
}

static void OOGLmat_scale(QvSFMatrix *mat1, float xt, float yt, float zt)
{
  int x, y;

  for (x = 0; x < 4; x++) 
    for (y = 0; y < 4; y++) {
      if (x == y) 
	mat1->value[x][y] = 1.0;
      else 
	mat1->value[x][y] = 0.0;
    }
  mat1->value[0][0] = xt; mat1->value[1][1] = yt; mat1->value[2][2] = zt;

}

static void OOGLmat_trans(QvSFMatrix *mat1, float xt, float yt, float zt)
{
  int x, y;

  for (x = 0; x < 4; x++) 
    for (y = 0; y < 4; y++) {
      if (x == y) 
	mat1->value[x][y] = 1.0;
      else 
	mat1->value[x][y] = 0.0;
    }

   mat1->value[0][3] = xt;
   mat1->value[1][3] = yt;
   mat1->value[2][3] = zt;
}

static void OOGLmat_rot(QvSFMatrix *mat1, float angle, float *rotaxis)
{
    float csA, snA, vsA, s;
    float axis[3];

    OOGLmat_init(mat1);
    s = rotaxis[0]*rotaxis[0] + rotaxis[1]*rotaxis[1] + rotaxis[2]*rotaxis[2];
    if(s == 0. || angle == 0.)
	return;

    s = 1/sqrt(s);
    axis[0] = rotaxis[0]*s;
    axis[1] = rotaxis[1]*s;
    axis[2] = rotaxis[2]*s;
    csA = cos(angle); snA = -sin(angle); vsA = 1 - csA;

    mat1->value[0][0] = axis[0]*axis[0]*vsA + csA;
    mat1->value[1][0] = axis[0]*axis[1]*vsA - axis[2]*snA;
    mat1->value[2][0] = axis[0]*axis[2]*vsA + axis[1]*snA;

    mat1->value[0][1] = axis[1]*axis[0]*vsA + axis[2]*snA;
    mat1->value[1][1] = axis[1]*axis[1]*vsA + csA;
    mat1->value[2][1] = axis[1]*axis[2]*vsA - axis[0]*snA;

    mat1->value[0][2] = axis[2]*axis[0]*vsA - axis[1]*snA;
    mat1->value[1][2] = axis[2]*axis[1]*vsA + axis[0]*snA;
    mat1->value[2][2] = axis[2]*axis[2]*vsA + csA;  
}

void
QvTransform::ToWebOOGL(QvState *state) // NOT RIGHT YET
{
    QvSFMatrix mat1, mat2, mat3;
  
    OOGLmat_init(&mat1); OOGLmat_init(&mat2); OOGLmat_init(&mat3);

    OOGLmat_trans(&mat2, -center.value[0], -center.value[1], -center.value[2]);
    OOGLmat_mult(&mat1, &mat2, &mat3);
    OOGLmat_rot(&mat2, scaleOrientation.angle, scaleOrientation.axis);
    OOGLmat_mult(&mat3, &mat2, &mat1);
    OOGLmat_scale(&mat2, scaleFactor.value[0], scaleFactor.value[1],
		  scaleFactor.value[2]);
    OOGLmat_mult(&mat1, &mat2, &mat3);
    OOGLmat_rot(&mat2, -scaleOrientation.angle, scaleOrientation.axis);
    OOGLmat_mult(&mat3, &mat2, &mat1);
    OOGLmat_rot(&mat2, rotation.angle, rotation.axis);
    OOGLmat_mult(&mat1, &mat2, &mat3);
    OOGLmat_trans(&mat2, center.value[0], center.value[1], center.value[2]);
    OOGLmat_mult(&mat3, &mat2, &mat1);
    OOGLmat_trans(&mat2, translation.value[0], translation.value[1],
		  translation.value[2]);
    OOGLmat_mult(&mat1, &mat2, &mat3);

    oogldepth++;
    ANNOUNCE(QvTransform);
    QvElement *elt = new QvElement;
    elt->data = this;
    elt->type = QvElement::Transform;
    state->addElement(QvState::TransformationIndex, elt);
    OOGLout("{ = INST");
    OOGLout("transform {\t# VRML Transform");
    OOGLoutmat(&mat3);
    OOGLout("} geom");
    OOGLout("{ = LIST");
}

void
QvRotation::ToWebOOGL(QvState *state)
{
    QvElement *elt = new QvElement;
    QvSFMatrix mat1;

    oogldepth++;
    ANNOUNCE(QvRotation);

    OOGLmat_init(&mat1);
    OOGLmat_rot(&mat1, rotation.angle, rotation.axis);

    elt->data = this;
    elt->type = QvElement::Rotation;
    state->addElement(QvState::TransformationIndex, elt);

    OOGLout("{ = INST");
    OOGLout("transform {");
    OOGLoutmat(&mat1);
    OOGLout("} geom");
    OOGLout("{ = LIST");
}

void
QvTranslation::ToWebOOGL(QvState *state)
{
    QvSFMatrix mat1;

    oogldepth++;
    ANNOUNCE(QvTranslation);
    QvElement *elt = new QvElement;
    elt->data = this;
    elt->type = QvElement::Translation;
    state->addElement(QvState::TransformationIndex, elt);

    OOGLmat_trans(&mat1, translation.value[0], translation.value[1],
		  translation.value[2]);
    OOGLout("{ = INST");
    OOGLout("transform {");
    OOGLoutmat(&mat1);
    OOGLout("} geom");
    OOGLout("{ = LIST");
}

void
QvScale::ToWebOOGL(QvState *state)
{
    oogldepth++;
    ANNOUNCE(QvScale);
    QvElement *elt = new QvElement;
    elt->data = this;
    elt->type = QvElement::Scale;
    state->addElement(QvState::TransformationIndex, elt);
    OOGLout("{ = INST");
    OOGLout("transform {");
    sprintf(out, "%f 0 0 0", scaleFactor.value[0]);
    OOGLout(out);
    sprintf(out, "0 %f 0 0", scaleFactor.value[1]);
    OOGLout(out);
    sprintf(out, "0 0 %f 0", scaleFactor.value[2]);
    OOGLout(out);
    OOGLout("0 0 0 1");
    OOGLout("} geom");
    OOGLout("{ = LIST");
}

DO_TYPED_PROPERTY(QvDirectionalLight,	LightIndex, DirectionalLight)
DO_TYPED_PROPERTY(QvPointLight,		LightIndex, PointLight)
DO_TYPED_PROPERTY(QvSpotLight,		LightIndex, SpotLight)

DO_TYPED_PROPERTY(QvOrthographicCamera,	CameraIndex, OrthographicCamera)
DO_TYPED_PROPERTY(QvPerspectiveCamera,	CameraIndex, PerspectiveCamera)

//////////////////////////////////////////////////////////////////////////////
//
// Shapes.
//
//////////////////////////////////////////////////////////////////////////////

static void
OOGLprintProperties(QvState *state)
{
//  printf("--------------------------------------------------------------\n");
//  state->print();
//  printf("--------------------------------------------------------------\n");
}

void
QvCone::ToWebOOGL(QvState *state)
{
    ANNOUNCE(QvCone);
    OOGLout("# VRML Cone");
    OOGLout("{ = BEZ224");
    if (parts.value == ALL || parts.value == SIDES)
    {
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1", height.value/2.0,
	height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f 0 0 1\t0 0 %f 0\t%f 0 0 1", -bottomRadius.value/2.0,
        bottomRadius.value/2.0, bottomRadius.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1\n", -bottomRadius.value,
        -height.value/2.0, bottomRadius.value, bottomRadius.value,
	-height.value/2.0);
      OOGLout(out);

      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -bottomRadius.value,
        -height.value/2.0, -bottomRadius.value, bottomRadius.value,
	-height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f 0 0 1\t0 0 %f 0\t%f 0 0 1", -bottomRadius.value/2.0,
        -bottomRadius.value/2.0, bottomRadius.value/2.0);
      OOGLout(out);
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1\n", height.value/2.0,
	height.value/2.0);
      OOGLout(out);
    }
    if (parts.value == ALL || parts.value == BOTTOM)
    {
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1", -height.value/2.0,
        -height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -bottomRadius.value/2.0,
        -height.value/2.0, bottomRadius.value/2.0,
        bottomRadius.value/2.0, -height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1\n", -bottomRadius.value,
        -height.value/2.0, bottomRadius.value, bottomRadius.value,
	-height.value/2.0);
      OOGLout(out);

      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -bottomRadius.value,
        -height.value/2.0, -bottomRadius.value, bottomRadius.value,
	-height.value/2.0);
      OOGLout(out);   
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -bottomRadius.value/2.0,
        -height.value/2.0, -bottomRadius.value/2.0,
        bottomRadius.value/2.0, -height.value/2.0);
      OOGLout(out);
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1\n", -height.value/2.0,
        -height.value/2.0);
      OOGLout(out);
    }
    OOGLout("}");
}

void
QvCube::ToWebOOGL(QvState *state)
{
    float x = width.value / 2.0, y = height.value / 2.0, z = depth.value / 2.0;
    int i;

    ANNOUNCE(QvCube);
    OOGLout("# VRML Cube");
    OOGLout("{ = OFF");
    OOGLout("8 6 12");
    for(i = 0; i < 8; i++) {
	sprintf(out, "%g %g %g", i&1 ? -x : x, i&2 ? -y : y, i&4 ? -z : z);
	OOGLout(out);
    }
    OOGLout("4  3 2 1 0");
    OOGLout("4  4 5 6 7");
    OOGLout("4  2 3 7 6");
    OOGLout("4  0 1 5 4");
    OOGLout("4  0 4 7 3");
    OOGLout("4  1 2 6 5");
    OOGLout("}");
}

void
QvCylinder::ToWebOOGL(QvState *state)
{
    QvElement *elt = NULL;
    QvMaterial *qv_material = NULL;
    QvMaterialBinding *qv_materialb = NULL;
    Binding mb_val;

    elt = state->getTopElement(QvState::MaterialIndex);
    if (elt) { qv_material = (QvMaterial *)elt->data;}
    elt = state->getTopElement(QvState::MaterialBindingIndex);
    if (elt) { qv_materialb = (QvMaterialBinding *)elt->data; }

    if (qv_materialb) { mb_val = (Binding) qv_materialb->value.value; }
    else { mb_val = DEFAULT; }

    ANNOUNCE(QvCylinder);
    OOGLout("# VRML Cylinder");
    OOGLout("{");
    if (mb_val != PER_PART && mb_val != PER_PART_INDEXED)
    {
	OOGLappearance(qv_material);
	OOGLout("= BEZ224");
    } else {
	OOGLout("= CBEZ224");
    }

    colstr[0] = '\0';
    if (parts.value == ALL || parts.value == SIDES)
    {
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value,
	height.value/2.0, radius.value, radius.value, height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f 0 0 1\t0 0 %f 0\t%f 0 0 1", -radius.value,
	radius.value, radius.value);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1\n", -radius.value,
	-height.value/2.0, radius.value, radius.value, -height.value/2.0);
      OOGLout(out);

      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value,
	-height.value/2.0, -radius.value, radius.value, -height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f 0 0 1\t0 0 %f 0\t%f 0 0 1", -radius.value,
	-radius.value, radius.value);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value,
	height.value/2.0, -radius.value, radius.value, height.value/2.0);
      OOGLout(out);

      if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
        OOGLgetcolor(qv_material, qv_materialb, 0);
	sprintf(out, "%s %s %s %s\n", colstr, colstr, colstr, colstr);
	OOGLout(out);
      }

    }
    if (parts.value == TOP || parts.value == ALL)
    {
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1", height.value/2.0,
	height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value/2.0,
	height.value/2.0, radius.value/2.0,
	radius.value/2.0, height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1\n", -radius.value,
	height.value/2.0, radius.value, radius.value, height.value/2.0);
      OOGLout(out);

      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value,
	height.value/2.0, -radius.value, radius.value, height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value/2.0,
	height.value/2.0, -radius.value/2.0,
	radius.value/2.0, height.value/2.0);
      OOGLout(out);
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1", height.value/2.0,
	height.value/2.0);
      OOGLout(out);

      if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
        OOGLgetcolor(qv_material, qv_materialb, 1);
	sprintf(out, "%s %s %s %s\n", colstr, colstr, colstr, colstr);
	OOGLout(out);
      }
    }

    if (parts.value == BOTTOM || parts.value == ALL)
    {
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1", -height.value/2.0,
        -height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value/2.0,
        -height.value/2.0, radius.value/2.0,
        radius.value/2.0, -height.value/2.0);
      OOGLout(out);
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1\n", -radius.value,
        -height.value/2.0, radius.value, radius.value, -height.value/2.0);
      OOGLout(out);

      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value,
        -height.value/2.0, -radius.value, radius.value, -height.value/2.0);
      OOGLout(out);   
      sprintf(out, "%f %f 0 1\t0 0 %f 0\t%f %f 0 1", -radius.value/2.0,
        -height.value/2.0, -radius.value/2.0,
        radius.value/2.0, -height.value/2.0);
      OOGLout(out);
      sprintf(out, "0 %f 0 1\t0 0 0 0\t0 %f 0 1\n", -height.value/2.0,
        -height.value/2.0);
      OOGLout(out);
      if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
        OOGLgetcolor(qv_material, qv_materialb, 2);
        sprintf(out, "%s %s %s %s\n", colstr, colstr, colstr, colstr);
       OOGLout(out);
      }

    }

    OOGLout("}");
}

void
QvSphere::ToWebOOGL(QvState *state)
{
    QvElement *elt = NULL;
    QvMaterial *qv_material = NULL;
    // QvMaterialBinding *qv_materialb = NULL;

    elt = state->getTopElement(QvState::MaterialIndex);
    if (elt) { qv_material = (QvMaterial *)elt->data;}
    elt = state->getTopElement(QvState::MaterialBindingIndex);
    // if (elt) { qv_materialb = (QvMaterialBinding *)elt->data; }

    ANNOUNCE(QvSphere);
    OOGLout("# VRML Sphere");
    OOGLout("{");
    OOGLappearance(qv_material);
    OOGLout("= SPHERE");
    sprintf(out, "  %f", radius.value);
    OOGLout(out);
    OOGLout("  0 0 0");
    OOGLout("}");
}

void
QvIndexedFaceSet::ToWebOOGL(QvState *state)
{
    QvElement *elt = NULL;
    QvCoordinate3 *qv_coord = NULL;
    QvMaterial *qv_material = NULL;
    // QvMaterialBinding *qv_materialb = NULL;
    QvNormal *qv_normal = NULL;
    QvNormalBinding *qv_normalb = NULL;
    float *coords;
    // float *vects;
    long  *firstindex;
    // Binding mb_val;
    Binding nb_val;
    int   count, numfaces, bad, numverts, numcoords, numnormals;

    elt = state->getTopElement(QvState::Coordinate3Index);
    if (elt) { qv_coord = (QvCoordinate3 *)elt->data;}
    elt = state->getTopElement(QvState::MaterialIndex);
    if (elt) { qv_material = (QvMaterial *)elt->data;}
    elt = state->getTopElement(QvState::MaterialBindingIndex);
    // if (elt) { qv_materialb = (QvMaterialBinding *)elt->data; }
    elt = state->getTopElement(QvState::NormalIndex);
    if (elt) { qv_normal = (QvNormal *)elt->data; }
    elt = state->getTopElement(QvState::NormalBindingIndex);
    if (elt) { qv_normalb = (QvNormalBinding *)elt->data; }

    // if (qv_materialb) { mb_val = (Binding) qv_materialb->value.value; }
    // else { mb_val = DEFAULT; }
    if (qv_normalb) { nb_val = (Binding) qv_normalb->value.value; }
    else { nb_val = DEFAULT; }

    if (qv_coord) {
	numcoords = qv_coord->point.num;
	coords = qv_coord->point.values;
    } else { numcoords = 0; coords = NULL; }
    if (qv_normal) {
	numnormals = qv_normal->vector.num;
	// vects = qv_normal->vector.values;
    } else {
	numnormals = 0;
	// vects = NULL;
    }
    if (qv_material) {
    }

    firstindex = coordIndex.values;
    numverts = coordIndex.num,

    ANNOUNCE(QvIndexedFaceSet);

    numfaces = 0; bad = 0;
    for (count = 0; count < numverts; count++)
    {
      if (firstindex[count] < 0 && !bad) { numfaces++; bad = 1; }
	else bad = 0; /* bad handles case of two consecutive delimeters */
      if (firstindex[count] > (numcoords - 1)) return;
    }
    for (count = 0; count < numnormals; count++) {

    }

    OOGLout("# IndexedFaceSet");
    if (coords == NULL) { OOGLout("# Bad Values"); return; }
    if (nb_val) { }
    OOGLout("{ = OFF");
    sprintf(out, "%d %d 0\n", numcoords, numfaces);
    OOGLout(out);

    for (count = 0; count < numcoords; count++)
    {
      float x = coords[count*3], y = coords[count*3+1], z = coords[count*3+2];
      sprintf(out, "%f %f %f", x, y, z);
      OOGLout(out);
    }
    OOGLout("");

    count = 0;
    while (count < numverts) {
      int numvts, start;
      numvts = 0;
      start = count;
      while (firstindex[count] > -1 && count < numverts) {
	count++; numvts++;
      }
      if (count > start)
      {
	OOGLoutindent();
	sprintf(out, "%d", numvts);
	OOGLoutraw(out);
	for(int i=start;i<count;i++) {
	 sprintf(out, " %d", firstindex[i]);
	 OOGLoutraw(out);
	}
	OOGLoutraw("\n");
      }
      count++;
    }
    OOGLout("}");

}

void
QvIndexedLineSet::ToWebOOGL(QvState *state)
{
    QvElement *elt = NULL;
    QvCoordinate3 *qv_coord = NULL;
    QvMaterial *qv_material = NULL;
    QvMaterialBinding *qv_materialb = NULL;
    float *coords;
    long  *firstindex, *matindex;
    Binding mb_val;
    int   numvts, points, numsegs;
    int   count, numlines, numverts, numcoords, nummats;

    elt = state->getTopElement(QvState::Coordinate3Index);
    if (elt) { qv_coord = (QvCoordinate3 *)elt->data;}
    elt = state->getTopElement(QvState::MaterialIndex);
    if (elt) { qv_material = (QvMaterial *)elt->data;}
    elt = state->getTopElement(QvState::MaterialBindingIndex);
    if (elt) { qv_materialb = (QvMaterialBinding *)elt->data; }

    if (qv_materialb) { mb_val = (Binding) qv_materialb->value.value; }
    else { mb_val = DEFAULT; }

    if (qv_coord) {
      numcoords = qv_coord->point.num;
      coords = qv_coord->point.values;
    } else { numcoords = 0; coords = NULL; }

    nummats = materialIndex.num;
    matindex = materialIndex.values;
    firstindex = coordIndex.values;
    numverts = coordIndex.num,

    ANNOUNCE(QvIndexedLineSet);

    count = 0; numlines = 0; numvts = 0; points = 0; numsegs = 0;
    while (count < numverts) {
      int x;

      x = 0;
      while (firstindex[count] < 0 && count < numverts) count++;
      while (firstindex[count] > -1 && count < numverts) {
	count++; x++;
      }
      if (x == 1) {
	numvts++; numlines++; points++;
      } else if (x > 1) {
	numvts += x; numlines++; numsegs += x - 1;
      }
    }
        
    OOGLout("# IndexedLineSet");
    if (coords == NULL || numvts < 1 || numlines < 1) {
      OOGLout("# Bad Values");
      return;
    }

    OOGLout("{ = VECT");
    if (mb_val == DEFAULT || mb_val == OVERALL) {
      sprintf(out, "%d %d 1\n", numlines, numvts);
    } else if (mb_val == PER_FACE || mb_val == PER_FACE_INDEXED) {
      sprintf(out, "%d %d %d\n", numlines, numvts, numlines);
    } else if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
      sprintf(out, "%d %d %d\n", numsegs + points, numvts, numsegs + points);
    } else if (mb_val == PER_VERTEX || mb_val == PER_VERTEX_INDEXED) {
      sprintf(out, "%d %d %d\n", numlines, numvts, numvts);
    }
    OOGLout(out);

    /* number of vertices in each poly line */

    OOGLoutindent();
    count = 0;
    while (count < numverts) {
      int x;

      x = 0;
      while (firstindex[count] < 0 && count < numverts) count++;
      while (firstindex[count] > -1 && count < numverts) {
	count++; x++;
      }
      if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
	/* Here, we divide our poly lines into segments... */
	if (x == 1) {
	  sprintf(out,"1 ");
	  OOGLoutraw(out);
	} else if (x > 1) {
	  while (x > 1) {
	    sprintf(out,"2 ");
	    OOGLoutraw(out);
	    x--;
	  }
	  sprintf(out,"%d ", x - 1);
	  OOGLoutraw(out);
	}
      } else if (x > 0) {
	sprintf(out,"%d ", x);
	OOGLoutraw(out);
      }
    }
    OOGLoutraw("\n");
      
    /*  number of colors in each poly line */

    OOGLoutindent();
    if (mb_val == DEFAULT || mb_val == OVERALL) {
      count = 1;
      OOGLoutraw("1 ");
      while (count < numlines) {
	sprintf(out,"0 ");
	OOGLoutraw(out);
	count++;
      }
      OOGLoutraw("\n");
    } else if (mb_val == PER_FACE || mb_val == PER_FACE_INDEXED) {
      count = 0;
      while (count < numlines) {
	OOGLoutraw("1 "); count++;
      }
    } else if (mb_val == PER_PART || mb_val == PER_PART_INDEXED) {
      count = 0;
      while (count < (numsegs + points)) {
	OOGLoutraw("1 "); count++;
      }
    } else if (mb_val == PER_VERTEX || mb_val == PER_VERTEX_INDEXED) {
      count = 0;
      while (count < numverts) {
	int x;

	x = 0;
	while (firstindex[count] < 0 && count < numverts) count++;
	while (firstindex[count] > -1 && count < numverts) {
	  count++; x++;
	}
	if (x > 0) {
	  sprintf(out,"%d ", x);
	  OOGLoutraw(out);
	}
      }
    }
    OOGLout("");

    count = 0;
    while (count < numverts) {
      long num;

      while (firstindex[count] < 0 && count < numverts) count++;
      while (firstindex[count] > -1 && count < numverts) {
	float x, y, z;
	
	num = firstindex[count]; count++;
	if (num > -1 && num < numcoords) {
	  x = coords[num * 3];
	  y = coords[num * 3 + 1];
	  z = coords[num * 3 + 2];
	  sprintf(out,"%f %f %f", x, y, z);
	  OOGLout(out);
	} else sprintf(out,"%f %f %f", x, y, z);
      }
    }
    OOGLout("");

    OOGLoutindent();
    if (mb_val == DEFAULT || mb_val == OVERALL) {
      OOGLgetcolor(qv_material, qv_materialb, 0);
      OOGLout(colstr);
    } else if (mb_val == PER_FACE || mb_val == PER_PART ||
	       mb_val == PER_VERTEX) {
      int numpieces;
      
      if (mb_val == PER_FACE) numpieces = numlines;
      else if (mb_val == PER_PART) numpieces = numsegs + points;
      else if (mb_val == PER_VERTEX) numpieces = numvts;

      count = 0;
      while (count < numpieces) {
	OOGLgetcolor(qv_material, qv_materialb, count);
	OOGLout(colstr);
	count++;
      }
    } else if (mb_val == PER_FACE_INDEXED || mb_val == PER_PART_INDEXED || 
	       mb_val == PER_VERTEX_INDEXED) {
      int numpieces, x;
      
      if (mb_val == PER_FACE_INDEXED) numpieces = numlines;
      else if (mb_val == PER_PART_INDEXED) numpieces = numsegs + points;
      else if (mb_val == PER_VERTEX_INDEXED) numpieces = numvts;

      count = 0; x = 0;
      while (count < numpieces) {
	int num;
	
	if (nummats > 0) {
	  num = (int) matindex[x % nummats];
	  if (num >= 0) {
	    OOGLgetcolor(qv_material, qv_materialb, num);
	    OOGLout(colstr);
	    count++; x++;
	  } else x++;
	} else {
	  OOGLgetcolor(qv_material, qv_materialb, 0);
	  OOGLout(colstr); count++;
	}
      }
    }
    OOGLout("}");

}

void
QvPointSet::ToWebOOGL(QvState *state)
{
    QvElement *elt = NULL;
    QvCoordinate3 *qv_coord = NULL;
    QvMaterial *qv_material = NULL;
    QvMaterialBinding *qv_materialb = NULL;
    float *coords;
    // Binding mb_val;
    int   count, numcoords;
    long numverts;

    elt = state->getTopElement(QvState::Coordinate3Index);
    if (elt) { qv_coord = (QvCoordinate3 *)elt->data;}
    elt = state->getTopElement(QvState::MaterialIndex);
    if (elt) { qv_material = (QvMaterial *)elt->data;}
    elt = state->getTopElement(QvState::MaterialBindingIndex);
    if (elt) { qv_materialb = (QvMaterialBinding *)elt->data; }

    // if (qv_materialb) { mb_val = (Binding) qv_materialb->value.value; }
    // else { mb_val = DEFAULT; }

    if (qv_coord) {
      numcoords = qv_coord->point.num;
      coords = qv_coord->point.values;
    } else { numcoords = 0; coords = NULL; }

    if (qv_material) {
    }

    if (startIndex.value > numcoords) {
      OOGLout("# Bad Values");
      return;
    }

    if (numPoints.value == (-1)) {
      numverts = numcoords - startIndex.value;
    } else if ((numPoints.value - startIndex.value) > numcoords) {
      OOGLout("# Bad Values");
      return;
    } else {
      numverts = numPoints.value - startIndex.value;
    }

    ANNOUNCE(QvPointSet);
        
    OOGLout("# PointSet");
    if (coords == NULL) {
      OOGLout("# Bad Values");
      return;
    }
    OOGLout("{ = VECT");
    sprintf(out, "%d %d %d\n", numverts, numverts, numverts);
    OOGLout(out);

    /* number of vertices in each poly line */

    OOGLoutindent();
    for (count = 0; count < numverts; count++) {
      OOGLoutraw("1 ");
    }
    OOGLoutraw("\n");

    /*  number of colors in each poly line */

    OOGLoutindent();
    for (count = 0; count < numverts; count++) {
      OOGLoutraw("1 ");
    }
    OOGLoutraw("\n");

    for (count = 0; count < numverts; count++)
    {
      float x = coords[(count+startIndex.value)*3],
      y = coords[(count+startIndex.value)*3+1],
      z = coords[(count+startIndex.value)*3+2];

      sprintf(out, "%f %f %f", x, y, z);
      OOGLout(out);
    }
    OOGLout("");

    for (count = 0; count < numverts; count++) {
      OOGLgetcolor(qv_material, qv_materialb, count);
      OOGLout(colstr);
    }
    OOGLout("}");

}

static int knowngsfont(char *str)
{
  if(str == NULL)
    return 0;
  if(access(str, 0) >= 0)
    return 1;

  char buf[2048], *p, *tail;
  char *gsfontpath = getenv("GS_FONTPATH");

  if(gsfontpath == NULL)
    gsfontpath = "/usr/local/lib/ghostscript/fonts";
  for(p = tail = gsfontpath; tail && *p != '\0'; p = tail+1) {
    tail = strchr(p, ':');
    int len = tail ? tail - p : strlen(p);
    if(len > 1024) len = 1024;
    memcpy(buf, p, len);
    buf[len] = '/';
    strncpy(buf+len+1, str, 2046-len);
    if(access(buf, 0) >= 0)
	return 1;
  }
  return 0;
}

void QvAsciiText::ToWebOOGL(QvState *state)
{
  
  float fontsize = 1.0;
  // float linespace = 1.0;
  char *font = NULL;
  // int bold = 0, italic = 0;

  QvElement *elt = state->getTopElement(QvState::FontStyleIndex);
  if(elt && elt->data) {
    QvFontStyle *fs = (QvFontStyle *)elt->data;
    fontsize = fs->size.value;
    // if(fs->style.value & (1<<QvFontStyle::BOLD))
    //	bold = 1;
    switch(fs->family.value) {
    case QvFontStyle::SERIF:	  font = "hrpl_t.gsf";		 break;
    case QvFontStyle::TYPEWRITER: /* sorry, can't do that now */ break;
    }
  }

  int i, c;
  float y = 0;
  char buf[2048], *q;
  const char *p;
  char *just = "sw";
  FILE *f;

  switch(justification.value) {
    case QvAsciiText::LEFT: just = "sw"; break;
    case QvAsciiText::RIGHT: just = "se"; break;
    case QvAsciiText::CENTER: just = "s"; break;
  }
  OOGLout(string.num > 1 ? "{ = LIST # VRML AsciiText" : "# VRML AsciiText");
  for(i = 0; i < string.num; i++) {
    float wid = width.values[ (i<width.num) ? i : width.num-1 ];
    sprintf(buf, "hvectext -s %g -align %s -plane xy -at 0 %g 0 ",
		fontsize, just, y);
    if(wid > 0)
	sprintf(buf + strlen(buf), "-w %g ", wid);
    if(font && knowngsfont(font))
	sprintf(buf + strlen(buf), "-hershey %s ", font);

    q = buf + strlen(buf);
    *q++ = '\'';
    for(p = string.values[i].getString(); p && *p != '\0' && q < &buf[2048-3]; ) {
	if(*p == '\'' || *p == '\\') *q++ = '\\';
	*q++ = *p++;
    }
    *q++ = '\'';
    *q = '\0';
    f = popen(buf, "r");
    for(q = buf; (c = getc(f)) != EOF; ) {
	if(c == '\n') {
	   *q = '\0';
	   OOGLout(buf);
	   q = buf;
	} else {
	    *q++ = c;
	}
    }
    pclose(f);
    y -= spacing.value * fontsize;
  }
  if(string.num > 1)
    OOGLout("} # End AsciiText");
}

//////////////////////////////////////////////////////////////////////////////
//
// WWW-specific nodes.
//
//////////////////////////////////////////////////////////////////////////////

void
QvWWWInline::ToWebOOGL(QvState *)
{
  int notuniq = 0;
  int i;

    ANNOUNCE(QvWWWInline);

    if (OOGLhand) {
      sprintf(out, "{ : %s }", name.value.getString());
      for (i = 0; i < unique && !notuniq; i++) {
	if (!strcmp(urls[i], name.value.getString())) notuniq = 1;
      }
      if (!notuniq) { 
	urls[unique++] = (char *)name.value.getString(); 
      }
    } else {
      sprintf(out, "{ COMMENT url.%d WWWInline { \"%s\" }}", 
	      unique++, name.value.getString());
    }      
    OOGLout(out);
}

void
QvWWWAnchor::ToWebOOGL(QvState *state)
{
    int savedepth, i;

    ANNOUNCE(QvWWWAnchor);

    OOGLout("{ = LIST           # Anchor");
    OOGLoutindent();
    OOGLoutraw("{ COMMENT ");
    sprintf(out, "wwwanchor.%d HREF { \"%s\" } }\n", mypid,
	    name.value.getString());
    OOGLoutraw(out);

    savedepth = oogldepth;

    indent++;
    for (i = 0; i < getNumChildren(); i++)
	getChild(i)->ToWebOOGL(state);
    indent--;

    savedepth = oogldepth - savedepth; /* How deep did we get ? :-)*/

    for (i = 0; i < savedepth; i++)  { /* Do the equivalent OOGL pop */
      out[i*2] = '}';		   /* pop the LIST */
      out[i*2+1] = '}';		   /* pop the INST */
    }

    out[savedepth*2] = '\0';
    OOGLout(out);
    OOGLout("}			# End Anchor");
    oogldepth = oogldepth - savedepth;
   
}

//////////////////////////////////////////////////////////////////////////////
//
// Default traversal methods. These nodes have no effects during traversal.
//
//////////////////////////////////////////////////////////////////////////////

DEFAULT_TRAVERSE(QvInfo)
DEFAULT_TRAVERSE(QvUnknownNode)

//////////////////////////////////////////////////////////////////////////////

#undef ANNOUNCE
#undef DEFAULT_TRAVERSE
#undef DO_PROPERTY
#undef DO_SHAPE
#undef DO_TYPED_PROPERTY
