/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** languages.c
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:27:13 2000
** Description:   Beaver languages menus generator source
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


#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "filesops.h"
#include "editor.h"
#include "struct.h"
#include "conf.h"
#include "main.h"
#include "msgbar.h"
#include "languages.h"

/* Max number of entries per language */

#define ENTRIES_MAX_NB 1000


extern GtkWidget *MainNotebook;
extern GArray *FileProperties;
extern gint OpenedFilesCnt;


/* A little private function */

char *slash_to_backslash (char *s)
{
  char *it = s;

  while (*it)
    {
      if (*it == '/')
	*it ='\\';
      it++;
    }
  return s;
}


/* Load the Beaver Language files that are declared in the 'General' config
   file */

void init_languages_menu (void)
{
  gchar *BeaverLanguageFile;
  gint i = 1;
  gint lang;
  gchar *tmp_str;
  GtkItemFactoryEntry Sep = {"/Languages/sep", NULL, NULL, 0, "<Separator>"};
  GtkItemFactoryEntry NoSyhiEntry = {"/Languages/" NO_SYHI, NULL,
				     languages_treatment,
				     SYHI_DISABLE, "<Item>"};
  gtk_item_factory_create_items
    (gtk_item_factory_from_path  ("<main>"), 1, &NoSyhiEntry, NULL);
  gtk_item_factory_create_items
    (gtk_item_factory_from_path  ("<main>"), 1, &Sep, NULL);
  for (lang = 0; lang < MAX_LANG; lang++)
    {
      if (Prefs.L[lang].IsDefined)
	{
	  gchar *s = g_strconcat("/Languages/",
				 slash_to_backslash(Prefs.L[lang].Description),
				 NULL);
	  GtkItemFactoryEntry NewEntry = {s, NULL, languages_treatment,
					  lang, "<Item>"};

	  gtk_item_factory_create_items
	    (gtk_item_factory_from_path  ("<main>"), 1, &NewEntry, NULL);
	  g_free(s);
	}
      
    }
  if (strlen(tmp_str = get_string_conf ("General/Languages/BLFile1")))
    {
      gtk_item_factory_create_items
	(gtk_item_factory_from_path  ("<main>"), 1, &Sep, NULL);
    }
  g_free(tmp_str);

  while (g_strcasecmp ((BeaverLanguageFile = get_string_conf
			(g_strdup_printf
			 ("General/Languages/BLFile%d", i))), ""))
    {  
      FILE *File;
      gchar *Temp;

      if ((File = fopen ((Temp = g_strconcat
			  (g_get_home_dir (), PATH_SEP_STRING,
			   CONF_DIR, PATH_SEP_STRING, BeaverLanguageFile,
			   NULL)), "r")) == NULL)
	{
	  g_free (Temp);
	  i++;
	  continue;
	}
      else
	{
	  gchar *BaseDirectory, *BaseEntryPath;
	  gint j = 1;
	  
	  fclose (File);
	  g_free (Temp);
	  while (g_strcasecmp ((BaseDirectory = get_string_conf
				(g_strdup_printf
				 (g_strconcat
				  (BeaverLanguageFile,
				   "/Directories/Directory%d", NULL), j++))),
			       ""))
	    {
	      GtkWidget *Menu;
	      gchar *Directory;
	      gchar *FullDirectory;
	      GtkItemFactoryEntry NewEntry = {
		(Directory = g_strconcat ("/Languages/", BaseDirectory, NULL)),
		NULL, NULL, 0, "<Branch>"};
	      
	      gtk_item_factory_create_items
		(gtk_item_factory_from_path  ("<main>"),
		 1, &NewEntry, NULL);
	      Menu = gtk_tearoff_menu_item_new ();
	      FullDirectory = g_strconcat ("<main>", Directory, NULL);
	      gtk_menu_prepend (GTK_MENU
				(gtk_item_factory_get_widget
				 (gtk_item_factory_from_path  ("<main>"),
				  FullDirectory)), Menu);
	      g_free (Directory);
	      g_free (FullDirectory);
	    }
	  j = 1;
	  while (g_strcasecmp ((BaseEntryPath = get_string_conf
				(g_strdup_printf
				 (g_strconcat
				  (BeaverLanguageFile,
				   "/Entry%d/Path", NULL), j))),
			       ""))
	    {

	      if (!strcmp (str_get_last_part
			   (BaseEntryPath, '/', TRUE), "SEP"))
		{
		  gchar *EntryPath;
		  GtkItemFactoryEntry NewEntry = {
		    (EntryPath = g_strconcat
		     ("/Languages/", BaseEntryPath, NULL)),
		    NULL, NULL, 0, "<Separator>"};
		  
		  gtk_item_factory_create_items
		    (gtk_item_factory_from_path  ("<main>"),
		     1, &NewEntry, NULL);
		  g_free (EntryPath);
		  j++;
		}
	      else
		{
		  gchar *EntryPath;
		  GtkItemFactoryEntry NewEntry = {
		    (EntryPath = g_strconcat
		     ("/Languages/", BaseEntryPath, NULL)),
		    get_string_conf (g_strdup_printf
				     (g_strconcat
				      (BeaverLanguageFile,
				       "/Entry%d/KeyBinding", NULL), j)),
		    entries_treatment, ENTRIES_MAX_NB*i + j++, "<Item>"};
		  
		  gtk_item_factory_create_items
		    (gtk_item_factory_from_path  ("<main>"),
		     1, &NewEntry, NULL);
		  g_free (EntryPath);
		}
	    }
	}
      i++;
    }
}


/* This is the function called by the items of the Languages menu generated
   above */

void languages_treatment (GtkWidget *Widget, gint Op)
{
  gint CurrentPage;
  gchar *msg;
  GtkWidget *CurrentText;

  if (!OpenedFilesCnt)
    return;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  CurrentText = FPROPS(CurrentPage, Text);
  if (FPROPS(CurrentPage, Changed[0]) == 0)
    gtk_signal_disconnect (GTK_OBJECT(FPROPS(CurrentPage, Text)),
			   FPROPS(CurrentPage, Changed[1]));
  refresh_editor(FPROPS(CurrentPage, Text), Op);
  if (FPROPS(CurrentPage, Changed[0]) == 0) 
    FPROPS(CurrentPage, Changed[1]) = gtk_signal_connect
      (GTK_OBJECT(CurrentText), "changed", (GtkSignalFunc)set_changed, NULL);
  if (Op >= 0)
      msg = g_strconcat("\"", Prefs.L[Op].Description,
			"\" mode activated...", NULL);
  else if (Op == SYHI_DISABLE)
      msg = g_strdup("Language support disabled...");
  else
      msg = g_strdup("*Bug!* Unknown Op");
  print_msg(msg);
  g_free(msg);
  (void)Widget; /* avoid the "unused parameter" warning */
}


/* This is the function called by the items of the menu generated above */

void entries_treatment (GtkWidget *Widget, gint Op)
{
  gint CurrentPage;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
    {
      gchar *TempString, *Text1, *Text2;
      gint TempInt, Position, Len;
      
      TempInt = Op/ENTRIES_MAX_NB;
      Op = Op - TempInt*ENTRIES_MAX_NB;
      TempString = g_strconcat (get_string_conf
				(g_strdup_printf
				 ("General/Languages/BLFile%d", TempInt)),
				g_strdup_printf ("/Entry%d/InsertText1", Op),
				NULL);
      Text1 = get_string_conf (TempString);
      g_free (TempString);
      TempString = g_strconcat (get_string_conf
				(g_strdup_printf
				 ("General/Languages/BLFile%d", TempInt)),
				g_strdup_printf ("/Entry%d/InsertText2", Op),
				NULL);
      Text2 = get_string_conf (TempString);
      g_free (TempString);
      gtk_editable_delete_selection
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)));
      TempInt = gtk_editable_get_position
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)));
      if ((Len = strlen (Text1)))
	{
	  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				    Text1, Len, &TempInt);
	  g_free (Text1);
	}
      Position = TempInt;
      if ((Len = strlen (Text2)))
	{
	  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				    Text2, Len, &TempInt);
	  g_free (Text2);
	}
      gtk_editable_set_position
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), Position);
    }
  (void)Widget; /* avoid the "unused parameter" warning */
}
