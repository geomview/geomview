/* Copyright (C) 2006-2007 Claus-Justus Heine 
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

/* simplistic help-system spawner (i.e. view the manual via external
 * browsers)
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lisp.h"

static char htmlbrowser[PATH_MAX] = { '\0', }, pdfviewer[PATH_MAX] = { '\0', };

void ui_manual_browser(const char *type)
{
  char helper[PATH_MAX];
  char *docdir;
  char *browser;
  char *lang;
  const char *file = NULL, *dfltfile, *langfile;
  
  if ((docdir = getenv("GEOMVIEW_DOC_DIR")) == NULL) {
    docdir = DOCDIR; /* compile-time default */
  }

  if (strncasecmp(type, "html", strlen("html")) == 0) {
    if (*htmlbrowser == '\0') {
      if ((browser = getenv("WEBBROWSER")) == NULL) {
	browser = DFLTHTMLBROWSER;
      }
    } else {
      browser = htmlbrowser;
    }
    dfltfile = "%s/html/index.html";
    langfile = "%s/html/%s/index.html";
  } else { /* if (strncasecmp(type, "pdf", strlen("pdf")) == 0) { */
    if (*pdfviewer == '\0') {
      if ((browser = getenv("PDFVIEWER")) == NULL) {
	browser = DFLTPDFVIEWER;
      }
    } else {
      browser = pdfviewer;
    }
    dfltfile = "%s/geomview.pdf";
    langfile = "%s/geomview-%s.pdf";
  }

  if ((lang = getenv("LANG")) != NULL) {
    snprintf(helper, PATH_MAX, langfile, docdir, lang);
    file = findfile(NULL, helper);
  }
  if (file == NULL) {
    snprintf(helper, PATH_MAX, dfltfile, docdir);
    file = findfile(NULL, helper);
  }
  if (file == NULL) {
    OOGLError(0, "Unable to locate the Geomview manual (\"%s\").", helper);
  } else {
    snprintf(helper, PATH_MAX, "%s %s &", browser, file);
    system(helper);
  }
}

LDEFINE(ui_html_browser, LVOID,
	"(ui-html-browser HTMLBROWSER)\n"
	"Use HTMLBROWSER when the `Manual (HTML)' menu item is selected "
	"in the help menu. If the `(ui-html-browser...)' command was never "
	"executed, then the default is to use the browser stored in the "
	"`WEBBROWSER' environment variable. If the environment variable is "
	"unset then `"DFLTHTMLBROWSER"' is used.")
{
  char *browser = NULL;
  
  LDECLARE(("ui-html-browser", LBEGIN,
	    LSTRING, &browser,
	    LEND));
  
  if (browser != NULL) {
    strncpy(htmlbrowser, browser, PATH_MAX);
  }
  
  return Lt;
}

LDEFINE(ui_pdf_viewer, LVOID,
	"(ui-pdf-browser PDFVIEWER\n"
	"Use PDFVIEWER when the `Manual (PDF)' menu item is selected "
	"in the help menu. If the `(ui-pdf-viewer...)' command was never "
	"executed, then the default is to use the viewer stored in the "
	"`PDFVIEWER' environment variable. If the environment variable is "
	"unset then `"DFLTPDFVIEWER"' is used.")
{
  char *browser = NULL;
  
  LDECLARE(("ui-pdf-viewer", LBEGIN,
	    LSTRING, &browser,
	    LEND));
  
  if (browser != NULL) {
    strncpy(pdfviewer, browser, PATH_MAX);
  }
  
  return Lt;
}




/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
