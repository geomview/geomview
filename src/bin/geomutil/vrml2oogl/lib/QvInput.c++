#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <QvInput.h>
#include <QvDebugError.h>
#include <QvReadError.h>
#include <QvNode.h>

#define CURVERSION 1.0 			// Current version of file format
#define COMMENT_CHAR '#'
#define	COMPRESS_CHAR 0x1F	/* First byte of compressed or gzipped file */

static const int numValidASCIIHeaders = 1;
struct headerStorage 
{
    const char *string;
    float version;
};

static const headerStorage ASCIIHeader[1] = {
    { "#VRML V1.0 ascii",	/* 20 chars */ 1.0 },
};

float
QvInput::isASCIIHeader(const char *string)
{
    for (int i = 0; i < numValidASCIIHeaders; i++) {
	if (strcmp(ASCIIHeader[i].string, string) == 0)
	    return ASCIIHeader[i].version;
    }
    return FALSE;
}

QvInput::QvInput()
{
    setFilePointer(stdin);
}

QvInput::~QvInput()
{
}

void
QvInput::setFilePointer(FILE *newFP)
{
    fp = newFP;
    lineNum = 1;
    version = CURVERSION;
    readHeader = FALSE;
    headerOk = TRUE;
    backBufIndex = -1;		
}

float
QvInput::getVersion()
{
    if (! readHeader)
	(void) checkHeader();

    return version;
}

QvBool
QvInput::get(char &c)
{
    QvBool ret;

    if (backBufIndex >= 0) {
	c = backBuf.getString()[backBufIndex++];

	if (c != '\0')
	    return TRUE;

	backBuf.makeEmpty();
	backBufIndex = -1;
    }

    if (! readHeader && ! checkHeader())
	return FALSE;

    if (eof()) {
	c = (char)EOF;
	ret = FALSE;
    }

    else {
	int i = getc(fp);

	if (i == EOF) {
	    c = (char)EOF;
	    ret = FALSE;
	}
	else {
	    c = (char) i;
	    ret = TRUE;
	}
    }

    return ret;
}

QvBool
QvInput::read(char &c)
{
    return (skipWhiteSpace() && get(c));
}

QvBool
QvInput::read(QvString &s)
{
    if (! skipWhiteSpace())
	return FALSE;

    QvBool      quoted;
    char        c;
    char        bufStore[256];
    char        *buf;
    int         bytesLeft;

    s.makeEmpty();

    if (! get(c))
	return FALSE;

    quoted = (c == '\"');
    if (! quoted)
	putBack(c);

    do {
	buf       = bufStore;
	bytesLeft = sizeof(bufStore) - 1;

	while (bytesLeft > 0) {

	    if (! get(*buf))
		break;

	    if (quoted) {
		if (*buf == '\"')
		    break;

		if (*buf == '\\') {
		    if ((get(c)) && c == '\"')
			*buf = '\"';
		    else
			putBack(c);
		}

		if (*buf == '\n')
		    lineNum++;
	    }

	    else if (isspace(*buf)) {
		putBack(*buf);
		break;
	    }

	    buf++;
	    bytesLeft--;
	}
	*buf = '\0';

	s += bufStore;

    } while (bytesLeft == 0);

    return TRUE;
}

QvBool
QvInput::read(QvName &n, QvBool validIdent)
{
    QvBool	gotChar;

    if (! skipWhiteSpace())
	return FALSE;

    if (! validIdent) {
	QvString s;

	if (! read(s))
	    return FALSE;

	n = s;
    }

    else {
	char	buf[256];
	char	*b = buf;
	char	c;

	if ((gotChar = get(c)) && QvName::isIdentStartChar(c)) {
	    *b++ = c;

	    while ((gotChar = get(c)) && QvName::isIdentChar(c)) {
		if (b - buf < 255)
		    *b++ = c;
	    }
	}
	*b = '\0';

	if (gotChar)
	    putBack(c);

	n = buf;
    }

    return TRUE;
}

#define READ_NUM(reader, readType, num, type)				      \
    QvBool ok;								      \
    if (! skipWhiteSpace())						      \
    ok = FALSE;								      \
    else {								      \
	readType _tmp;							      \
	ok = reader(_tmp);						      \
	if (ok)								      \
	    num = (type) _tmp;						      \
    }									      \
    return ok

#define READ_INTEGER(num, type)						      \
    READ_NUM(readInteger, long, num, type)

#define READ_UNSIGNED_INTEGER(num, type)				      \
    READ_NUM(readUnsignedInteger, unsigned long, num, type)

#define READ_REAL(num, type)						      \
    READ_NUM(readReal, double, num, type)

QvBool
QvInput::read(int &i)
{
    READ_INTEGER(i, int);
}

QvBool
QvInput::read(unsigned int &i)
{
    READ_UNSIGNED_INTEGER(i, unsigned int);
}

QvBool
QvInput::read(short &s)
{
    READ_INTEGER(s, short);
}

QvBool
QvInput::read(unsigned short &s)
{
    READ_UNSIGNED_INTEGER(s, unsigned short);
}

QvBool
QvInput::read(long &l)
{
    READ_INTEGER(l, long);
}

QvBool
QvInput::read(unsigned long &l)
{
    READ_UNSIGNED_INTEGER(l, unsigned long);
}

QvBool
QvInput::read(float &f)
{
    READ_REAL(f, float);
}

QvBool
QvInput::read(double &d)
{
    READ_REAL(d, double);
}

QvBool
QvInput::eof() const
{
    return feof(fp);
}

void
QvInput::getLocationString(QvString &string) const
{
    char buf[128];
    sprintf(buf, "\tOccurred at line %3d", lineNum);
    string = buf;
}

void
QvInput::putBack(char c)
{
    if (c == (char) EOF)
	return;

    if (backBufIndex >= 0)
	--backBufIndex;
    else
	ungetc(c, fp);
}

void
QvInput::putBack(const char *string)
{
    backBuf = string;
    backBufIndex = 0;
}

#if defined(unix) || defined(__unix)
# include <unistd.h>

static FILE *
spliceDecompress(QvInput *in, FILE *fp, char c)
{
    int pfd[2], zpfd[2];
    int k = 0;
    FILE *togz = (FILE *)NULL;

    /* Build a pipeline:
     * 0x1F+"in"data->zpfd[1] => zpfd[0]->gzip->pfd[1] => pfd[0]->parser
     */
    if(pipe(pfd) < 0 || pipe(zpfd) < 0) {
	QvReadError::post(in, "Can't make pipe to read compressed VRML");
	return NULL;
    }
    switch(fork()) {
    case -1:
	QvReadError::post(in, "Can't fork subprocess to decompress VRML");
	return NULL;

    case 0:
	close(pfd[0]);

	switch(fork()) {
	case -1:
	    QvReadError::post(in, "Can't fork subprocess to decompress VRML");
	    _exit(1);

	case 0:
	    int ch;
	    togz = fdopen(zpfd[1], "w");
	    close(pfd[1]);
	    close(zpfd[0]);
	    putc(c, togz);
	    while((ch = getc(fp)) != EOF) {
		putc(ch, togz);
		k++;
	    }
	    fflush(togz);
	    _exit(0);

	default:
	    close(zpfd[1]);
	    if(zpfd[0] != 0) {
		dup2(zpfd[0], 0);
		close(zpfd[0]);
	    }
	    if(pfd[1] != 1) {
		dup2(pfd[1], 1);
		close(pfd[1]);
	    }
	    execlp("gzip", "gzip", "-d", NULL);
	    execlp("compress", "compress", "-d", NULL);
	    QvReadError::post(in, "Can't exec gzip nor compress to read compressed VRML");
	    _exit(1);
	}

    default:
	close(pfd[1]);
	close(zpfd[0]);
	close(zpfd[1]);
	return fdopen(pfd[0], "r");
    }
}
    
#endif /* unix */
    

QvBool
QvInput::checkHeader()
{
    char	c;

    readHeader = TRUE;

    /* Read first character cautiously, in case we need to reroute input.
     * Ugh.  Is there no clean way to do this?
     */

    int got;

#if defined(unix) || defined(__unix)
    got = ::read(fileno(fp), &c, 1) > 0;
    if(got && c == COMPRESS_CHAR) {
	if((fp = spliceDecompress(this, fp, c)) == NULL) {
	    headerOk = FALSE;
	    return FALSE;
	}
	got = get(c);
    }
#else
    got = get(c);
#endif

    if(got) {
	if (c == COMMENT_CHAR) {
	    char	buf[256];
	    int		i = 0;

	    buf[i++] = c;
	    while (get(c) && c != '\n')
		buf[i++] = c;
	    if(i>0 && buf[i-1] == '\r')
		i--;
	    buf[i] = '\0';
	    if (c == '\n')
		lineNum++;

	    if ((version = isASCIIHeader(buf)))
		return TRUE;
	}
	else
	    putBack(c);
    }

    QvReadError::post(this, "File does not have a valid header string");
    headerOk = FALSE;
    return FALSE;
}

QvBool
QvInput::skipWhiteSpace()
{
    char	c;
    QvBool	gotChar;

    if (! readHeader && ! checkHeader())
	return FALSE;

    while (TRUE) {

	while ((gotChar = get(c)) && isspace(c))
	    if (c == '\n')
		lineNum++;

	if (! gotChar)
	    break;

	if (c == COMMENT_CHAR) {
	    while (get(c) && c != '\n')
		;

	    if (eof())
		QvReadError::post(this, "EOF reached before end of comment");
	    else
		lineNum++;
	}
	else {
	    putBack(c);
	    break;				    }
    }

    return TRUE;
}

QvBool
QvInput::readInteger(long &l)
{
    char str[32];
    char *s = str;

    if (readChar(s, '-') || readChar(s, '+'))
	s++;

    if (! readUnsignedIntegerString(s))
	return FALSE;

    l = strtol(str, NULL, 0);

    return TRUE;
}

QvBool
QvInput::readUnsignedInteger(unsigned long &l)
{
    char	str[32];		
    if (! readUnsignedIntegerString(str))
	return FALSE;
#ifdef sun
    l = atol(str);
#else
    l = strtoul(str, NULL, 0);
#endif

    return TRUE;
}

QvBool
QvInput::readUnsignedIntegerString(char *str)
{
    int  minSize = 1;
    char *s = str;

    if (readChar(s, '0')) {

	if (readChar(s + 1, 'x')) {
	    s += 2 + readHexDigits(s + 2);
	    minSize = 3;
	}

	else
	    s += 1 + readDigits(s + 1);
    }

    else
	s += readDigits(s);

    if (s - str < minSize)
	return FALSE;

    *s = '\0';

    return TRUE;
}

QvBool
QvInput::readReal(double &d)
{
    char	str[32];
    int		n;
    char	*s = str;
    QvBool	gotNum = FALSE;

    n = readChar(s, '-');
    if (n == 0)
	n = readChar(s, '+');
    s += n;

    if ((n = readDigits(s)) > 0) {
	gotNum = TRUE;
	s += n;
    }

    if (readChar(s, '.') > 0) {
	s++;

	if ((n = readDigits(s)) > 0) {
	    gotNum = TRUE;
	    s += n;
	}
    }

    if (! gotNum)
	return FALSE;

    n = readChar(s, 'e');
    if (n == 0)
	n = readChar(s, 'E');

    if (n > 0) {
	s += n;

	n = readChar(s, '-');
	if (n == 0)
	    n = readChar(s, '+');
	s += n;

	if ((n = readDigits(s)) > 0)
	    s += n;

	else
	    return FALSE;		    }

    *s = '\0';

    d = atof(str);

    return TRUE;
}

int
QvInput::readDigits(char *string)
{
    char c, *s = string;

    while (get(c)) {

	if (isdigit(c))
	    *s++ = c;

	else {
	    putBack(c);
	    break;
	}
    }

    return s - string;
}

int
QvInput::readHexDigits(char *string)
{
    char c, *s = string;

    while (get(c)) {

	if (isxdigit(c))
	    *s++ = c;

	else {
	    putBack(c);
	    break;
	}
    }

    return s - string;
}

int
QvInput::readChar(char *string, char charToRead)
{
    char	c;
    int		ret;

    if (! get(c))
	ret = 0;

    else if (c == charToRead) {
	*string = c;
	ret = 1;
    }

    else {
	putBack(c);
	ret = 0;
    }

    return ret;
}

void
QvInput::addReference(const QvName &name, QvNode *node)
{
    refDict.enter((u_long) name.getString(), (void *) node);

    node->setName(name);
}

QvNode *
QvInput::findReference(const QvName &name) const
{
    void	*node;

    if (refDict.find((u_long) name.getString(), node))
	return (QvNode *) node;

    return NULL;
}
