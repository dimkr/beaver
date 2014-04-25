/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** main.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed Jan 10 20:32:13 2001
** Description:   Beaver main source file header
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef WIN32
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#define CONF_DIR "config"
#define TEMP_DIR "c:\\windows\\temp\\"
#define TEMP_PREFIX "buffer_"
#else
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#define CONF_DIR ".beaver"
#define TEMP_DIR "/tmp/"
#define TEMP_PREFIX "buffer_"
#endif

#define APP_NAME        "Beaver"
#define APP_MOTTO       "Beaver's an Early\nAdVanced EditoR"
#define APP_URL	        "www.beaver-project.org"
#define VERSION_NUMBER  "0.2.7"
#define UNTITLED        "Untitled"
#define WELCOME_MSG     "Welcome to"
#define UNKNOWN         "???"
#define NO_SYHI		"None"


int main (int argc, char *argv[]);

#endif
