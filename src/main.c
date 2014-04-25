/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** main.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed Jan 10 20:32:23 2001
** Description:   Beaver main source file
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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "interface.h"
#include "wordfile.h"
#include "main.h"


/* The incredibly complex 'main' function :) */

int main (int argc, char *argv[])
{
  gchar *GenPath, *ConfDir, *WordPath;
  struct stat Stats;	   

  if (argc >= 2)
    {
      gint i;
      gchar *AppName;
      
      AppName = g_strdup_printf(APP_NAME);
      g_strdown (AppName);
      for (i = 1; i < argc; i++)
	{
	  if (!strcmp (argv[i], "--help") || !strcmp (argv[i], "-?"))
	    {
	      g_print
		(g_strconcat
		 ("Usage: ", AppName, " [OPTION...] [FILE...]"
		  "\n\n"
		  "Help options\n"
		  "  -?, --help\t\t\tShow this help message\n"
		  "  --usage\t\t\tDisplay brief usage message\n\n"
		  "GTK options\n"
		  "  --gdk-debug=FLAGS\t\tGdk debugging flags to set\n"
		  "  --gdk-no-debug=FLAGS\t\tGdk debugging flags to "
		  "unset\n"
		  "  --display=DISPLAY\t\tX display to use\n"
		  "  --sync\t\t\tMake X calls synchronous\n"
		  "  --no-xshm\t\t\tDon't use X shared memory extension\n"
		  "  --name=NAME\t\t\tProgram name as used by the window"
		  "manager\n"
		  "  --class=CLASS\t\t\tProgram class as used by the \n"
		  "window manager\n"
		  "  --gtk-debug=FLAGS\t\tGtk+ debugging flags to set\n"
		  "  --gtk-no-debug=FLAGS\t\tGtk+ debugging flags to "
		  "unset\n"
		  "  --g-fatal-warnings\t\tMake all warnings fatal\n"
		  "  --gtk-module=MODULE\t\tLoad an additional Gtk "
		  "module\n\n", AppName, " options\n",
		  "  --version\t\t\tDispay version number\n", NULL));
	      return (1);
	    }
	  else if (!strcmp (argv[i], "--usage"))
	    {
	      g_print
		(g_strconcat
		 ("Usage: ", AppName, " [-?] [--version] [--usage] "
		  "[--gdk-debug=FLAGS]\n\t[--gdk-no-debug=FLAGS] "
		  "[--display=DISPLAY] [--sync] [--no-xshm]\n\t"
		  "[--name=NAME] [--class=CLASS] [--gtk-debug=FLAGS] "
		  "[--gtk-no-debug=FLAGS]\n\t[--g-fatal-warnings] "
		  "[--gtk-module=MODULE]\n", NULL));
	      return (1);
	    }
	  else if (!strcmp (argv[i], "--version"))
	    {
	      g_print (g_strconcat (AppName, " " VERSION_NUMBER "\n", NULL));
	      return (1);
	    }
	  else if (!strcmp (argv[i], "--"))
	    {
	      break;
	    }
	}
      g_free (AppName);
    }
  ConfDir = g_strconcat (g_get_home_dir (), PATH_SEP_STRING, CONF_DIR, NULL);
  mkdir (ConfDir, 0755);
  GenPath = g_strconcat (ConfDir, PATH_SEP_STRING, "General", NULL);
  if (stat (GenPath, &Stats) == -1)
    {
        FILE *General;
	gchar *DefaultConfig;
	
	DefaultConfig = g_strconcat (
	  "## This is the main Beaver preferences file.\n"
	  "## Do not edit by hand (of course do not delete the file) !!!"
	  "\n"
	  "## Else, crashes could occur... :(\n"
	  "\n"
	  "[RecentFiles]\n"
	  "MaxNb = 6\n"
	  "\n"
	  "[Window]\n"
	  "Autosave = TRUE\n"
	  "Width = 540\n"
	  "Height = 480\n"
	  "\n"
	  "[Tabs]\n"
	  "Position = 1\n"
	  "LabelLength = 12\n"
	  "\n"
	  "[Editor]\n"
	  "Wordfile = ", ConfDir,  PATH_SEP_STRING, "wordfile.txt"
	  "\n"
	  "Font = fixed"
	  "\n"
	  "BGRed = 65535\n"
	  "BGBlue = 65535\n"
	  "BGGreen = 65535\n"  
	  "FGRed = 0\n"
	  "FGBlue = 0\n"
	  "FGGreen = 0\n"
	  "SelectedBGRed = 40000\n"
	  "SelectedBGBlue = 40000\n"
	  "SelectedBGGreen = 40000\n"
	  "SelectedFGRed = 65535\n"
	  "SelectedFGBlue = 65535\n"
	  "SelectedFGGreen = 65535\n"
	  "Wordwrap = FALSE\n"
	  "\n"
	  "[MsgBar]\n"
	  "Display = TRUE\n"
	  "Interval = 2000\n"
	  "\n"
	  "[ToolBar]\n"
	  "Display = TRUE\n"
	  "\n"
	  "[ScrollBar]\n"
	  "Position = 2\n"
	  "\n"
	  "[AutoSave]\n"
	  "Delay = 0\n"
	  "Backup = FALSE\n"
#ifdef WIN32
	  "BackupExt = .bak\n"
#else
	  "BackupExt = ~\n"
#endif  
	  "\n"
	  "[CompletionPopUp]\n"
	  "Width = 130\n"
	  "Height = 200\n"
	  "\n"
	  "[Adv]\n"
	  "SynHigh = TRUE\n"
	  "SynHighDepth = 12000\n"
	  "AutoIndent = FALSE\n"
	  "AutoCorrect = TRUE\n"
	  "\n"
	  "[Misc]\n"
#ifdef WIN32
	  "PrintCommand = %s > lpt1\n"
#else
	  "PrintCommand = lpr\n"
#endif
	  "Beep = TRUE\n"
	  "\n"
	  "[Languages]\n", NULL);
	General = fopen (GenPath, "w");
	
	fprintf (General, "%s", DefaultConfig);
	fclose (General);
	g_free (DefaultConfig);
    }
  g_free (GenPath);
  WordPath = g_strconcat (ConfDir, PATH_SEP_STRING,
			  "wordfile.txt", NULL);
  if (stat (WordPath, &Stats) == -1)
    {
      FILE *Wordfile;
      
      Wordfile = fopen (WordPath, "wt");
      fprintf (Wordfile, "%s", default_wordfile);
      fclose (Wordfile);
    }
  g_free (WordPath);
  g_free (ConfDir);
  interface (argc, argv);
  return (0);
}
