/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** filesops.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Sat Jun  3 20:48:56 2000
** Description:   Files operations source
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
#include "main.h"
#include "editor.h"
#include "struct.h"
#include "interface.h"
#include "msgbar.h"
#include "conf.h"
#include "prefs.h"
#include "filesops.h"


#define OPEN_FILE       0
#define SAVE_FILE_AS    1
#define QUIT_FILE       0
#define CLOSE_FILE      1
#define CLOSE_ALL_FILE  2


extern GtkWidget *MainNotebook;
extern GArray *FileProperties;
extern t_settings Settings;
extern gint OpenedFilesCnt;
extern gint NewFilesCnt;
static gboolean FileSelectorIsVisible = FALSE;
static gboolean QuestionIsVisible = FALSE;
static GPtrArray *CurrentRecentFiles;
static gint AutosaveSig = 0;

/* This function is used by 'init_file_properties' to set the base filename
   and the type of the file. */

gchar *str_get_last_part (gchar *String, gchar Separator,
			  gboolean ReturnIfSeparatorNotFound)
{
  gchar *LastPart;
  gint i = 0, j = 0;
  gboolean Switch;

  LastPart = g_malloc (256);
  Switch = FALSE;
  while (String[i] != '\0')
    {  
      if (String[i] == Separator)
	{
	  if (!Switch) Switch = TRUE;
	  i++;
	  j = 0;
	}
      else LastPart[j++] = String[i++];
    }   
  LastPart[j] = '\0';
  if (!ReturnIfSeparatorNotFound && !Switch) LastPart[0] = '\0';
  return (LastPart);
}


/* Return the absolute path of the file 'FileName' */

gchar *get_absolute_path (gchar *FileName)
{
  gchar *TempFileName, *AbsolutePath;
  gchar **Tab;
  gint i = 0;

  if (g_path_is_absolute (FileName))
    return g_strdup(FileName);
  TempFileName = g_strconcat (g_get_current_dir (),
			      PATH_SEP_STRING, FileName, NULL);
  Tab = g_strsplit (TempFileName, PATH_SEP_STRING, 0);
  g_free (TempFileName);
  while (Tab[i] != NULL)
    {
      if (!strcmp (Tab[i], ".."))
	{
	  gint j;
	  
	  for (j = i; Tab[j] != NULL; j++)
	    {
	      Tab[j-1] = Tab[j+1];
	    }
	  i--;
	}
      else i++;
    }
  AbsolutePath = g_strjoinv (PATH_SEP_STRING, Tab);
  g_strfreev (Tab);
  return (AbsolutePath);
}


/* This function is called when a text has changed (since last save) */

void set_changed (void)
{
  gint CurrentPage;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  FPROPS(CurrentPage, Changed[0]) = 1;
  set_label (GTK_NOTEBOOK(MainNotebook), CurrentPage);
  print_msg("File has been modified...");
  gtk_signal_disconnect (GTK_OBJECT(FPROPS(CurrentPage, Text)),
			 FPROPS(CurrentPage, Changed[1]));
}


/* This function is called when you want a text to be readonly (or not) */

void toggle_readonly (void)
{
  gint CurrentPage;

  if (!OpenedFilesCnt) return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if (FPROPS(CurrentPage, ReadOnly) == 0)
    {
      gtk_text_set_editable (GTK_TEXT(FPROPS(CurrentPage, Text)), FALSE);
      FPROPS(CurrentPage, ReadOnly) = 1;
      set_label (GTK_NOTEBOOK(MainNotebook), CurrentPage);
      print_msg ("Readonly mode activated");
    }
  else if (FPROPS(CurrentPage, ReadOnly) == 1)
    {
      gtk_text_set_editable (GTK_TEXT(FPROPS(CurrentPage, Text)), TRUE);
      FPROPS(CurrentPage, ReadOnly) = 0;
      set_label (GTK_NOTEBOOK(MainNotebook), CurrentPage);
      print_msg ("Readonly mode deactivated");      
    }
  else print_msg (g_strconcat ("Readonly mode cannot be toggled: \"",
			       FPROPS(CurrentPage, BaseName),
			       "\" is write protected", NULL));
}


/* Init the selected file properties */

void init_file_properties (gchar *FileName, gint CurrentPage)
{
  struct stat Stats;
  
  FPROPS(CurrentPage, Name) = g_strdup (FileName);
  FPROPS(CurrentPage, BaseName) =
    g_strdup (str_get_last_part (FileName, PATH_SEP, TRUE));
  FPROPS(CurrentPage, Type) =
    g_strdup (str_get_last_part (FPROPS(CurrentPage, BaseName), '.', FALSE));
  if (stat (FileName, &Stats) != -1)
    {
      FILE *File;

      if (((File = fopen (FPROPS(CurrentPage, Name), "a")) != NULL))
	{ 
	  FPROPS(CurrentPage, ReadOnly) = 0;
	  fclose (File);
	}
      else
	{
	  FPROPS(CurrentPage, ReadOnly) = -1;
	}
    }
  else FPROPS(CurrentPage, ReadOnly) = 0;
  FPROPS(CurrentPage, Changed[0]) = 0;
  stat (FileName, &FPROPS(CurrentPage, Stats));
}


/* Return the rwx permissions of a file in a string */

gchar *get_file_mode (struct stat Stats)
{
  static gchar Mode[10];
  
  g_snprintf (Mode, 10, "%c%c%c%c%c%c%c%c%c",
	      (Stats.st_mode & S_IRUSR) ? 'r' : '-',
	      (Stats.st_mode & S_IWUSR) ? 'w' : '-',
	      (Stats.st_mode & S_IXUSR) ? 'x' : '-',
	      (Stats.st_mode & S_IRGRP) ? 'r' : '-',
	      (Stats.st_mode & S_IWGRP) ? 'w' : '-',
	      (Stats.st_mode & S_IXGRP) ? 'x' : '-',
	      (Stats.st_mode & S_IROTH) ? 'r' : '-',
	      (Stats.st_mode & S_IWOTH) ? 'w' : '-',
	      (Stats.st_mode & S_IXOTH) ? 'x' : '-');
  return (Mode);
}


/* Update the recent files by putting a new file in the list */


void init_recent_files (void)
{
  gint i = 0;

  CurrentRecentFiles = g_ptr_array_new ();
  for (i = 1; i <= RECENT_FILES_MAX_NB; i++)
    {
      gchar *Temp;

      Temp = g_strconcat
	("/File/Recent Files/", g_strdup_printf ("%d. ", i),
	 str_get_last_part
	 (g_strdelimit
	  (get_string_conf
	   (g_strdup_printf ("General/RecentFiles/File%d", i)), "_", '-'),
	  PATH_SEP, TRUE), NULL);
      g_ptr_array_add (CurrentRecentFiles, (gpointer) Temp);
      if (strcmp (get_string_conf
		  (g_strdup_printf ("General/RecentFiles/File%d", i)), ""))
	{
	  GtkItemFactoryEntry NewEntry = {
	    g_ptr_array_index (CurrentRecentFiles, i-1),
	    NULL, (GtkSignalFunc)open_recent_file,
	    (glong)get_string_conf
	    (g_strdup_printf
	     ("General/RecentFiles/File%d", i)),"<Item>"};
	  gtk_item_factory_create_items
	    (gtk_item_factory_from_path  ("<main>"),
	     1, &NewEntry, NULL);
	}
    }
}


void put_recent_file (gchar *FileName)
{
  gint i;
  
  DIRECTORY = g_strconcat (g_dirname (FileName), PATH_SEP_STRING, NULL);
  for (i = 1; i <= RECENT_FILES_MAX_NB; i++)
    gtk_item_factories_path_delete
      ("<main>", g_ptr_array_index (CurrentRecentFiles, i-1));
  for (i = 1; i < RECENT_FILES_MAX_NB; i++)
    {
      if (!strcmp (FileName, get_string_conf
		    (g_strdup_printf ("General/RecentFiles/File%d", i))))
	{
	  gint j;
	  
	  for (j = i; j > 1; j--)
	    {
	      set_string_conf
		(g_strdup_printf ("General/RecentFiles/File%d", j),
		 get_string_conf (g_strdup_printf
				  ("General/RecentFiles/File%d", j-1)));
	    }
	  set_string_conf ("General/RecentFiles/File1", FileName);
	  display_recent_files ();
	  return;
	}
    }
  for (i = RECENT_FILES_MAX_NB; i > 1; i--)
    {
      set_string_conf
	(g_strdup_printf ("General/RecentFiles/File%d", i),
	 get_string_conf (g_strdup_printf
			  ("General/RecentFiles/File%d", i-1)));
    }
  set_string_conf ("General/RecentFiles/File1", FileName);
  display_recent_files ();
}


/* Used to put the most recently opened files in a menu */

void display_recent_files (void)
{
  gint i = 0;

  for (i = RECENT_FILES_MAX_NB; i > 0; i--)
    g_ptr_array_remove_index_fast (CurrentRecentFiles, i-1);
  for (i = 1; i <= RECENT_FILES_MAX_NB; i++)
    {
      gchar *Temp;

      Temp = g_strconcat
	("/File/Recent Files/", g_strdup_printf ("%d. ", i),
	 str_get_last_part
	 (g_strdelimit
	  (get_string_conf
	   (g_strdup_printf ("General/RecentFiles/File%d", i)), "_", '-'),
	  PATH_SEP, TRUE), NULL);
      g_ptr_array_add (CurrentRecentFiles, (gpointer) Temp);
      if (strcmp (get_string_conf
		  (g_strdup_printf ("General/RecentFiles/File%d", i)), ""))
	{
	  GtkItemFactoryEntry NewEntry = {
	    g_ptr_array_index (CurrentRecentFiles, i-1),
	    NULL, (GtkSignalFunc)open_recent_file,
	    (glong)get_string_conf
	    (g_strdup_printf
	     ("General/RecentFiles/File%d", i)),"<Item>"};
	  gtk_item_factory_create_items
	    (gtk_item_factory_from_path  ("<main>"),
	     1, &NewEntry, NULL);
	}
    }
}


/* Callback function for the Recent files menu */

void open_recent_file (GtkWidget *DummyWidget, gchar *FileName)
{
  struct stat Stats;
  gint CurrentPage;

  if (stat (FileName, &Stats) == -1)
    {      
      print_msg ("This file doesn't exist anymore...");      
      return;
    }
  if ((Stats.st_mode & S_IFMT) == S_IFDIR) return;
  put_recent_file (FileName);
  add_page_in_notebook (GTK_NOTEBOOK(MainNotebook), FileName);
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  open_file_in_editor(GTK_WIDGET(FPROPS(CurrentPage, Text)), FileName);
  if (FPROPS(CurrentPage, ReadOnly))
    print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, BaseName),
			    "\" opened in Readonly mode...", NULL));
  else
    print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, BaseName),
			    "\" opened...", NULL));
  FPROPS(CurrentPage, Changed[1]) =
    gtk_signal_connect (GTK_OBJECT(FPROPS(CurrentPage, Text)), "changed",
			(GtkSignalFunc) set_changed, NULL);
  (void)DummyWidget; /* avoid the "unused parameter" warning */
}


/* Enable/Disable Autosave */

void autosave (gint Delay)
{
  if (Delay)
    {
      if (AutosaveSig)
	{
	  gtk_timeout_remove (AutosaveSig);
	}
      AutosaveSig = gtk_timeout_add (1000 * Delay,
				     (GtkFunction)save_all,
				     NULL);
    }
  else
    if (AutosaveSig)
      {
	gtk_timeout_remove (AutosaveSig);
	AutosaveSig = 0;
      }
}


void file_selection_window_not_visible (void)
{
  FileSelectorIsVisible = FALSE;
}


/* Return the appropriate File Selection window, using one of the two
   functions below (i.e 'save_file_as_func' or 'open_file_func') */

GtkWidget *file_selection_window_new (guint Op)
{
  GtkWidget *FileSelector;
  GtkWidget *Button;
  gchar *Title, *Directory;
  GtkSignalFunc func;

  Title = Op? g_strconcat("Save \"", FPROPS(gtk_notebook_get_current_page
					    (GTK_NOTEBOOK(MainNotebook)),
					    BaseName),
			  "\"  As...", NULL) : "Open File...";
  func = Op? save_file_as_func : open_file_func;
  FileSelector = gtk_file_selection_new (Title);
  Directory = DIRECTORY;
  gtk_file_selection_set_filename (GTK_FILE_SELECTION(FileSelector),
                                   Directory);
  gtk_signal_connect (GTK_OBJECT(FileSelector), "delete_event",
		      (GtkSignalFunc) file_selection_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(FileSelector), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileSelector));
  gtk_signal_connect (GTK_OBJECT(FileSelector), "destroy_event",
		      (GtkSignalFunc) file_selection_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(FileSelector), "destroy_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileSelector));
  Button = GTK_FILE_SELECTION(FileSelector) -> ok_button;
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) func,
			     GTK_OBJECT(FileSelector));
  Button = GTK_FILE_SELECTION (FileSelector) -> cancel_button;
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) file_selection_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileSelector));
  gtk_window_set_modal (GTK_WINDOW(FileSelector), TRUE);
  FileSelectorIsVisible = TRUE;
  //g_free (Directory); ... it causes a segfault !!! Why ?!?
  return (FileSelector);
}


/* Used to save a file as... */

void save_file_as_func (GtkFileSelection *FileSelector)
{
  gchar *FileName;
  FILE *File;
  struct stat Stats;
  gint CurrentPage;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  FileName = gtk_file_selection_get_filename (FileSelector);
  if (stat (FileName, &Stats) != -1)
    if ((Stats.st_mode & S_IFMT) == S_IFDIR) return;
  if ((File = fopen (FileName, "w")))
    {
      gchar *Buffer;  
      GtkWidget *CurrentText;
 
      put_recent_file (FileName);
      CurrentText = FPROPS(CurrentPage, Text);
      Buffer = gtk_editable_get_chars (GTK_EDITABLE(CurrentText), 0, -1);
      fwrite (Buffer, gtk_text_get_length (GTK_TEXT(CurrentText)), 1, File);
      g_free (Buffer);
      fclose (File);
      if (FPROPS(CurrentPage, Changed[0]) == 0)
	gtk_signal_disconnect (GTK_OBJECT(FPROPS(CurrentPage, Text)),
			       FPROPS(CurrentPage, Changed[1]));
      init_file_properties (FileName, CurrentPage);
      refresh_editor(FPROPS(CurrentPage, Text), SYHI_AUTODETECT);
      FPROPS(CurrentPage, Changed[1]) = gtk_signal_connect
	(GTK_OBJECT(CurrentText), "changed", (GtkSignalFunc)set_changed, NULL);
      set_label (GTK_NOTEBOOK(MainNotebook), CurrentPage);
      set_title (GTK_NOTEBOOK(MainNotebook), NULL, CurrentPage);
      print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, BaseName),
			      "\" saved...", NULL));
      FileSelectorIsVisible = FALSE;
      gtk_widget_destroy(GTK_WIDGET(FileSelector));
    }
  else
    {
      print_msg (g_strconcat ("\"", FPROPS(CurrentPage, BaseName),
			      "\" cannot be saved: \"",
			      str_get_last_part (FileName, PATH_SEP, TRUE),
			      "\" is write protected", NULL));
      FileSelectorIsVisible = FALSE;
      gtk_widget_destroy(GTK_WIDGET(FileSelector));
    }
}


/* Used to open a file */

void open_file_func (GtkFileSelection *FileSelector)
{
  gchar *FileName;
  struct stat Stats;
  gint CurrentPage;

  FileName = gtk_file_selection_get_filename (FileSelector);
  if (stat (FileName, &Stats) == -1) return;
  if ((Stats.st_mode & S_IFMT) == S_IFDIR) return;
  put_recent_file (FileName);
  add_page_in_notebook (GTK_NOTEBOOK(MainNotebook), FileName);
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  open_file_in_editor(GTK_WIDGET(FPROPS(CurrentPage, Text)), FileName);
  if (FPROPS(CurrentPage, ReadOnly))
    print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, BaseName),
			    "\" opened in Readonly mode...", NULL));
  else
    print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, BaseName),
			    "\" opened...", NULL));
  FPROPS(CurrentPage, Changed[1]) =
    gtk_signal_connect (GTK_OBJECT(FPROPS(CurrentPage, Text)), "changed",
			(GtkSignalFunc) set_changed, NULL);
  FileSelectorIsVisible = FALSE;
  gtk_widget_destroy(GTK_WIDGET(FileSelector));
}


void new_file (void)
{
  gint CurrentPage;

  print_msg("New file...");
  add_page_in_notebook (GTK_NOTEBOOK(MainNotebook), NULL);
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  open_file_in_editor(GTK_WIDGET(FPROPS(CurrentPage, Text)), NULL);
  FPROPS(CurrentPage, Changed[1]) =
    gtk_signal_connect (GTK_OBJECT (FPROPS(CurrentPage, Text)),
			"changed", (GtkSignalFunc) set_changed, NULL);
}


void open_file (void)
{ 
  GtkWidget *FileSelector;
  
  FileSelector = file_selection_window_new (OPEN_FILE);
  gtk_widget_show (FileSelector);
}


void save_file (void)
{
  FILE *File;
  gint CurrentPage;

  if (!OpenedFilesCnt) return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if (!FPROPS(CurrentPage, Changed[0]))
    {
      print_msg ("File needn't be saved");
      return;  
    }
  if (FPROPS(CurrentPage, ReadOnly))
    {
      print_msg ("File cannot be saved: Readonly mode is activated");
      return;
    }
  if ((stat (FPROPS(CurrentPage, Name), &FPROPS(CurrentPage, Stats)) == -1)
      && (!strncmp (FPROPS(CurrentPage, Name), "Untitled ", 9)))
    save_file_as();
  else
    {
      if ((BACKUP) && (stat (FPROPS(CurrentPage, Name),
			     &FPROPS(CurrentPage, Stats)) != -1))
	{
	  gchar *BackupName;
	  
	  BackupName = g_strconcat (FPROPS(CurrentPage, Name),
				    BACKUP_EXT, NULL);
	  rename (FPROPS(CurrentPage, Name), BackupName);
	  g_free (BackupName);
	}
      if ((File = fopen (FPROPS(CurrentPage, Name), "w")))
	{
	  gchar *Buffer;

	  Buffer = gtk_editable_get_chars
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
	  fwrite (Buffer, gtk_text_get_length
		  (GTK_TEXT(FPROPS(CurrentPage, Text))), 1, File);
	  g_free (Buffer);
	  fclose (File);
	  put_recent_file (FPROPS(CurrentPage, Name));
	  FPROPS(CurrentPage, Changed[0]) = 0;
	  FPROPS(CurrentPage, Changed[1]) = gtk_signal_connect
	    (GTK_OBJECT(FPROPS(CurrentPage, Text)), "changed",
	     (GtkSignalFunc)set_changed, NULL);
	  set_label (GTK_NOTEBOOK(MainNotebook), CurrentPage);
	  print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, Name),
				  "\" saved...", NULL));
	}
      else
	{
	  print_msg (g_strconcat ("\"", FPROPS(CurrentPage, BaseName),
				  "\" cannot be saved: it is write protected",
				  NULL));
	  save_file_as();
	}
    }
}

void save_file_as (void)
{ 
  GtkWidget *FileSelector;

  if (!OpenedFilesCnt) return;
  FileSelector = file_selection_window_new (SAVE_FILE_AS);
  gtk_widget_show (FileSelector);
}


void save_all (void)
{
  gint CurrentPage, i;

  if (!OpenedFilesCnt) return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  for (i = 0; i < OpenedFilesCnt; i++)
    {
      gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), i);
      save_file ();
      while (FileSelectorIsVisible) gtk_main_iteration_do (FALSE);
    }
  gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), CurrentPage);
}


void question_window_not_visible (void)
{
  QuestionIsVisible = FALSE;
}


/* Display a Dialog box which ask you wether you wanna save or not the
   modified files when you quit or when you close these files */

GtkWidget *question_window_new (guint Op, gint CurrentPage)
{
  GtkWidget *QuestionWindow;
  GtkWidget *Button;
  GtkWidget *QuestionLabel;
  GtkWidget *Box;
  GtkSignalFunc func, save_func;
  gchar *Question;

  switch(Op)
    {
    case QUIT_FILE      :
      func = gtk_main_quit;
      save_func = close_all;
      QuestionLabel = gtk_label_new
	("Some files have been modified.\n"
	 "Do you wish to save them?");
      break;
    case CLOSE_FILE     :
    case CLOSE_ALL_FILE :
    default             :
      func = close_file_func;
      save_func = save_file_before_close_func;
      Question = g_strconcat ("\"", FPROPS(CurrentPage, BaseName),
			      "\" has been modified.\n",
			      "Do you wish to save it?", NULL);
      QuestionLabel = gtk_label_new (Question);
      g_free (Question);
    }
  QuestionWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(QuestionWindow), "Question"); 
  gtk_window_set_policy (GTK_WINDOW(QuestionWindow), FALSE, FALSE, FALSE);
  gtk_window_set_position (GTK_WINDOW(QuestionWindow), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW(QuestionWindow), TRUE);
  gtk_signal_connect (GTK_OBJECT(QuestionWindow), "delete_event",
		      (GtkSignalFunc) question_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(QuestionWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(QuestionWindow));
  gtk_signal_connect (GTK_OBJECT(QuestionWindow), "destroy",
		      (GtkSignalFunc) question_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT (QuestionWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(QuestionWindow));
  Box = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(Box), 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(QuestionWindow) -> vbox),
		      Box, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(Box), QuestionLabel, TRUE, TRUE, 0);
  Button = gtk_button_new_with_label (" Yes ");
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_hide_all,
			     GTK_OBJECT(QuestionWindow)); 
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) save_func, NULL);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) func, (gpointer)(glong)CurrentPage);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) question_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(QuestionWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(QuestionWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  Button = gtk_button_new_with_label ( " No ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) func, (gpointer)(glong)CurrentPage);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
	              (GtkSignalFunc) question_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(QuestionWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(QuestionWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  Button = gtk_button_new_with_label (" Cancel ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
	              (GtkSignalFunc) question_window_not_visible,
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(QuestionWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(QuestionWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  QuestionIsVisible = TRUE;
  return (QuestionWindow);
}


/*  Used to close a file */

void close_file_func (GtkWidget *Widget, gint CurrentPage)
{
  gtk_notebook_remove_page (GTK_NOTEBOOK(MainNotebook), CurrentPage);
  g_free (FPROPS(CurrentPage, Name));
  g_free (FPROPS(CurrentPage, BaseName));
  g_free (FPROPS(CurrentPage, Type));
  gtk_widget_destroyed (FPROPS(CurrentPage, Text),
			&FPROPS(CurrentPage, Text));
  g_array_remove_index (FileProperties, CurrentPage);
  OpenedFilesCnt--;
  print_msg ("File closed...");
  if (!OpenedFilesCnt)
    {
      set_title (GTK_NOTEBOOK(MainNotebook), NULL, -1);
      NewFilesCnt = 0;
    }
  (void)Widget; /* avoid the "unused parameter" warning */
}


/*  Used to save a file just before closing it */
 
void save_file_before_close_func (void)
{
  gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook),
			 gtk_notebook_get_current_page
			 (GTK_NOTEBOOK(MainNotebook)));
  save_file ();
  while (FileSelectorIsVisible) gtk_main_iteration_do (FALSE);
}


void close_file (void)
{
  gint CurrentPage;

  if (!OpenedFilesCnt) return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if (FPROPS(CurrentPage, Changed[0]) && !FPROPS(CurrentPage, ReadOnly))
  {
    GtkWidget *Question;

    Question = question_window_new (CLOSE_FILE, CurrentPage);
    gtk_widget_show_all (Question);
  }
  else
    {
      gtk_notebook_remove_page (GTK_NOTEBOOK(MainNotebook), CurrentPage);
      g_free (FPROPS(CurrentPage, Name));
      g_free (FPROPS(CurrentPage, BaseName));
      g_free (FPROPS(CurrentPage, Type));
      gtk_widget_destroyed (FPROPS(CurrentPage, Text),
			    &FPROPS(CurrentPage, Text));
      g_array_remove_index (FileProperties, CurrentPage);
      OpenedFilesCnt--;
      print_msg ("File closed...");
      if (!OpenedFilesCnt)
	{
	  set_title (GTK_NOTEBOOK(MainNotebook), NULL, -1);
	  NewFilesCnt = 0;
	}
    } 
}


void close_all (void)
{
  gint i;
  
  if (!OpenedFilesCnt) return; 
  print_msg ("Closing all files...");
  for (i = OpenedFilesCnt-1; i >= 0; i--)
    {
      if (!FPROPS(i, Changed[0]) || FPROPS(i, ReadOnly))   
	{
	  gtk_notebook_remove_page (GTK_NOTEBOOK(MainNotebook), i);
	  g_free (FPROPS(i, Name));
	  g_free (FPROPS(i, BaseName));
	  g_free (FPROPS(i, Type));
	  gtk_widget_destroyed (FPROPS(i, Text), &FPROPS(i, Text));
	  g_array_remove_index (FileProperties, i);
	  OpenedFilesCnt--;
	}
    }
  for (i = OpenedFilesCnt-1; i >= 0; i--)
    {
      GtkWidget *Question;

      gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), i);
      Question = question_window_new (CLOSE_ALL_FILE, i);
      gtk_widget_show_all (Question);     
      while (QuestionIsVisible) gtk_main_iteration_do (FALSE); 
    }
  if (!OpenedFilesCnt)
    {
      print_msg ("All files closed...");
      set_title (GTK_NOTEBOOK(MainNotebook), NULL, -1);
      NewFilesCnt = 0;
    }
}


void quit (void)
{
  GtkWidget *Question;

  if (!OpenedFilesCnt) gtk_main_quit ();
  else
    {
      gint i;

      for (i = 0; i < OpenedFilesCnt; i++)
	{  
	  if ((FPROPS(i, Changed[0]) && !FPROPS(i, ReadOnly)) &&
	      (!QuestionIsVisible))
	    {
	      Question = question_window_new (QUIT_FILE, i);
	      gtk_widget_show_all (Question);
	      return;
	    }
	}
      if (!QuestionIsVisible) gtk_main_quit ();
    }
}
