#ifndef __SGBAR_CONFIG_H__
#define __SGBAR_CONFIG_H__

#include <gtk/gtk.h>
#define NUMTAGS 9
#define NUM_CSS_PATHS 2
static const char *taglabel = "";
static const char *volumeicon = "";
static const char *brightnessicon = "";
static char *cssfilepaths[NUM_CSS_PATHS] = {"./style.css", "/usr/local/share/sgbar/style.css"};
static int separator_spacing = 8;
#endif
