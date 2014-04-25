/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** tools.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Fri Jun  2 01:08:28 2000
** Description:   Beaver tools source
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
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "interface.h"
#include "filesops.h"
#include "editor.h"
#include "struct.h"
#include "msgbar.h"
#include "search.h"
#include "tools.h"
#include "conf.h"

extern GtkWidget *MainNotebook;
extern GArray *FileProperties;
extern gint OpenedFilesCnt;
static GtkWidget *Entry[4];
static gboolean ConverterIsVisible = FALSE;
static gboolean ColorIsVisible = FALSE;


/* Returns the beginning of the current selection */

gint min_from_selection (GtkEditable *Text)
{
  return((GTK_EDITABLE(Text)->selection_start_pos 
	  > GTK_EDITABLE(Text)->selection_end_pos 
	  ? GTK_EDITABLE(Text)->selection_end_pos 
	  : GTK_EDITABLE(Text)->selection_start_pos));
}


/* Returns the end of the current selection */

gint max_from_selection (GtkEditable *Text)
{
  return((GTK_EDITABLE(Text)->selection_start_pos 
	  < GTK_EDITABLE(Text)->selection_end_pos 
	  ? GTK_EDITABLE(Text)->selection_end_pos 
	  : GTK_EDITABLE(Text)->selection_start_pos));
}


/* Returns the string that is currently selected */

gchar *get_selection (GtkEditable *Text)
{
  gchar *selected_str;

  selected_str = gtk_editable_get_chars (Text,
					 min_from_selection (Text),
					 max_from_selection (Text));
  return selected_str;
}


/* A few Case manipulations functions */

void change_case (gint UpDown)
{	
  gint CurrentPage, l_min, l_max;
  gchar *buffer ;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  l_min = min_from_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  l_max = max_from_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  buffer = gtk_editable_get_chars (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				   l_min, l_max);
  if (UpDown == 1) g_strup (buffer);
  else if (UpDown == 2) g_strdown (buffer);
  else if (UpDown == 3)
    {
      gint l_len, i, j;
      gchar *Delimiters = " \t\b\n\f\'\"{}()[]<>%^&*~-+=_@#$\\|/:;,.?!";

      l_len = l_max - l_min;
      if ((buffer[0] >= 'a') && (buffer[0] <= 'z'))
	buffer[0] = buffer[0] - 'a' + 'A';
      for (i = 1; i < l_len; i++)
	{
	  for (j = 0; j <= (gint)strlen (Delimiters); j++)
	    {
	      if (buffer [i] == Delimiters[j])
		{
		  if ((buffer[i+1] >= 'a') && (buffer[i+1] <= 'z'))
		    buffer[i+1] = buffer[i+1] - 'a' + 'A';
		  break;
		}
	    }
	}
    }
  else if (UpDown == 4)
    {
      gint l_len, change_case, i;

      l_len = l_max - l_min;
      change_case = 'a' - 'A';
      for (i = 0; i <= l_len; i++)
	{
	  if ((buffer[i] >= 'A') && (buffer[i] <= 'Z'))
	    buffer[i] = buffer[i] + change_case;
	  else if ((buffer[i] >= 'a') && (buffer[i] <= 'z'))
	    buffer[i] = buffer[i] - change_case;
	}
    }
  gtk_editable_delete_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
			    buffer, strlen(buffer), &l_min);
  g_free (buffer);
}


/* Well, this function just... inserts time! How surprising! ;) */

void insert_time (gint CurrentPage)
{
  gint Position;
  time_t Time;

  if ((!OpenedFilesCnt) || FPROPS(CurrentPage, ReadOnly))
    return;
  Time = time (NULL);
  gtk_editable_delete_selection
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
		   ctime (&Time), 24, &Position);
  print_msg ("Insert Time...");
}


void converter_window_not_visible (void)
{
  ConverterIsVisible = FALSE;
}


/* Convert & refresh the display of the Converter */

void refresh_converter (GtkWidget *Widget, gint EntryID)
{
  gchar *NumberString;
  gint i = 0, j = 1;
  gulong Number = 0;

  NumberString = gtk_entry_get_text (GTK_ENTRY (Entry[EntryID-1]));
  if (EntryID == 1)
    {
      if ((strlen (NumberString) == 10) &&
	  (g_strcasecmp (NumberString, "4294967295") > 0))
	{
	  if (g_strcasecmp (NumberString, "9999999999") <= 0)
	    {
	      Number = ~0; /* <-- Max Unsigned Long Integer */
	    }
	  else
	    {
	      Number = 0; /* <-- Min Unsigned Long Integer */
	    }
	}
      else
	{
	  g_strreverse (NumberString);
	  while ((NumberString[i] >= '0') && (NumberString[i] <= '9'))
	    {
	      Number = Number + (NumberString[i] - '0')*j;
	      j *= 10;
	      i++;
	    }
	  if (NumberString[i] != '\0')
	    Number = 0; /* <-- Min Unsigned Long Integer */
	}
    }
  if (EntryID == 2)
    {
      g_strreverse (NumberString);

      while ((NumberString[i] == '0') || (NumberString[i] == '1'))
	{
	  Number = Number + (NumberString[i] - '0')*j;
	  j *= 2;
	  i++;
	}
      if (NumberString[i] != '\0')
	Number = 0; /* <-- Min Unsigned Long Integer */
    }
  if (EntryID == 3)
    {
      if ((strlen (NumberString) == 11) &&
	  (g_strcasecmp (NumberString, "37777777777") > 0))
	{
	  if (g_strcasecmp (NumberString, "77777777777") <= 0)
	    {
	      Number = ~0; /* <-- Max Unsigned Long Integer */
	    }
	  else
	    {
	      Number = 0; /* <-- Min Unsigned Long Integer */
	    }
	}
      else
	{
	  g_strreverse (NumberString);
	  while ((NumberString[i] >= '0') && (NumberString[i] <= '7'))
	    {
	      Number = Number + (NumberString[i] - '0')*j;
	      j *= 8;
	      i++;
	    }
	  if (NumberString[i] != '\0')
	    Number = 0; /* <-- Min Unsigned Long Integer */
	}
    }
  if (EntryID == 4)
    {
      gint Zero;

      g_strreverse (NumberString);
      while (((NumberString[i] >= '0') && (NumberString[i] <= '9')) ||
	     ((NumberString[i] >= 'a') && (NumberString[i] <= 'f')) ||
	     ((NumberString[i] >= 'A') && (NumberString[i] <= 'F')))
	{
	  if ((NumberString[i] >= '0') && (NumberString[i] <= '9'))
	    Zero = '0';
	  else if ((NumberString[i] >= 'a') && (NumberString[i] <= 'f'))
	    Zero = 'a' - 10;
	  else
	    Zero = 'A' - 10;
	  Number = Number + (NumberString[i] - Zero)*j;
	  j *= 16;
	  i++;
	}
      if (NumberString[i] != '\0')
	Number = 0; /* <-- Min Unsigned Long Integer */
    }
  gtk_entry_set_text (GTK_ENTRY (Entry[0]),
		      g_strdup_printf ("%lu", Number));
  gtk_entry_set_text (GTK_ENTRY (Entry[2]),
		      g_strdup_printf ("%lo", Number));
  gtk_entry_set_text (GTK_ENTRY (Entry[3]),
		      g_strdup_printf ("%lx", Number));
  if (Number)
    {
      gint Bit;

      gtk_entry_set_text (GTK_ENTRY (Entry[1]), "");
      while (Number > 0)
	{
	  Bit = Number % 2;
	  Number = Number / 2;
	  gtk_entry_prepend_text (GTK_ENTRY (Entry[1]),
				  g_strdup_printf ("%d", Bit));
	}
    }
  else
    gtk_entry_set_text (GTK_ENTRY (Entry[1]), "0");
  (void)Widget; /* avoid the "unused parameter" warning */
}


/* Display a Decimal, Binary, Octal, HexaDecimal Converter */

void converter (void)
{
  GtkWidget *ConverterWindow;
  GtkWidget *Table;
  GtkWidget *Label;
  GtkWidget *Button;
  gint i;

  if (ConverterIsVisible) return;
  ConverterWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(ConverterWindow), "Base Converter"); 
  gtk_window_set_policy (GTK_WINDOW(ConverterWindow), FALSE, FALSE, FALSE);
  gtk_signal_connect (GTK_OBJECT(ConverterWindow), "delete_event",
		      (GtkSignalFunc) converter_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(ConverterWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(ConverterWindow));
  gtk_signal_connect (GTK_OBJECT(ConverterWindow), "destroy",
		      (GtkSignalFunc) converter_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT (ConverterWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(ConverterWindow));
  Table = gtk_table_new (4, 3, FALSE);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(ConverterWindow) -> vbox),
		      Table, FALSE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 10);
  Label = gtk_label_new ("Decimal :");
  gtk_table_attach_defaults (GTK_TABLE(Table), Label, 0, 1, 0, 1);  
  Label = gtk_label_new ("Binary :");
  gtk_table_attach_defaults (GTK_TABLE(Table), Label, 0, 1, 1, 2);
  Label = gtk_label_new ("Octal :");
  gtk_table_attach_defaults (GTK_TABLE(Table), Label, 0, 1, 2, 3);
  Label = gtk_label_new ("Hexa :");
  gtk_table_attach_defaults (GTK_TABLE(Table), Label, 0, 1, 3, 4);
  for (i = 1; i <= 4; i++)
    {
      Entry[i-1] = gtk_entry_new ();
      gtk_table_attach_defaults (GTK_TABLE(Table), Entry[i-1], 1, 2, i-1, i);
      if (i == 1)
	gtk_entry_set_max_length (GTK_ENTRY(Entry[i-1]), 10);
      if (i == 2)
	gtk_entry_set_max_length (GTK_ENTRY(Entry[i-1]), 32);
      if (i == 3)
	gtk_entry_set_max_length (GTK_ENTRY(Entry[i-1]), 11);
      if (i == 4)
	gtk_entry_set_max_length (GTK_ENTRY(Entry[i-1]), 8);
      Button = gtk_button_new_with_label (" Convert ");
      gtk_table_attach_defaults (GTK_TABLE(Table), Button, 2, 3, i-1, i);
      gtk_signal_connect (GTK_OBJECT(Button), "clicked",
			  (GtkSignalFunc)refresh_converter,
			  (gpointer)(unsigned long)i);
    }
  Button = gtk_button_new_with_label (" Close ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) converter_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(ConverterWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(ConverterWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  gtk_widget_show_all (ConverterWindow);
  ConverterIsVisible = TRUE;
  print_msg ("Display Base Converter...");
}


void color_picker_not_visible (void)
{
  ColorIsVisible = FALSE;
}


/* Insert an hexadecimal color code */

void insert_color (GtkColorSelection *csd)
{
  gdouble Color[3];
  gint CurrentPage, Position;
  gchar *ColorString;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if ((!OpenedFilesCnt) || FPROPS(CurrentPage, ReadOnly))
    return;
  gtk_color_selection_get_color (csd, Color);
  ColorString = g_strdup_printf ("%02x%02x%02x",
				 (guint)(255 * Color[0]),
				 (guint)(255 * Color[1]),
				 (guint)(255 * Color[2]));
  gtk_editable_delete_selection
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
			    ColorString, 6, &Position);
  print_msg ("Insert Color...");
}


/* Display a Color Picker */

void color_picker (void)
{
  GtkColorSelectionDialog *ColorWindow;

  if (ColorIsVisible) return;
  ColorWindow = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new
    ("Color Picker");
  gtk_signal_connect (GTK_OBJECT(ColorWindow), "delete_event",
		      (GtkSignalFunc) color_picker_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(ColorWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(ColorWindow));
  gtk_signal_connect (GTK_OBJECT (ColorWindow), "destroy",
		      (GtkSignalFunc) color_picker_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(ColorWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(ColorWindow));
  gtk_label_set_text (GTK_LABEL(GTK_BIN(ColorWindow -> ok_button) -> child),
		      "Insert");
  gtk_signal_connect_object (GTK_OBJECT(ColorWindow -> ok_button), "clicked",
			     (GtkSignalFunc)insert_color,
			     GTK_OBJECT(ColorWindow -> colorsel));
  gtk_signal_connect (GTK_OBJECT(ColorWindow -> cancel_button), "clicked",
		      (GtkSignalFunc)color_picker_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(ColorWindow -> cancel_button),
			     "clicked",
			     (GtkSignalFunc)gtk_widget_destroy,
			     GTK_OBJECT(ColorWindow));
  gtk_widget_hide (ColorWindow -> help_button);
  ColorIsVisible = TRUE;
  gtk_widget_show (GTK_WIDGET(ColorWindow));
  ColorIsVisible = TRUE;
  print_msg ("Display Color Picker...");
}


/* These two functions are used by the conversion ones */

gint remove_char (char *string, char to_remove)
{
  gint i, j, res = 0;
  
  for (i = j = 0; string[i] != '\0'; i++)
    if (string[i] != to_remove)
      string[j++] = string[i];
  string[j] = '\0';
  if (i == j) res = 1;
  return (res);
}


gint replace_char (char *string, char to_replace, char new_char)
{
  gint i, res = 1;

  for (i = 0; string[i] != '\0'; i++)
    {
      if (string[i] == to_replace)
	{
	  if (res) res = 0;
	  string[i] = new_char;
	}
    }
  return (res);
}


/* File conversion functions */

void convert_all(void (*conv_func)(void))
{
  gint CurrentPage, i;

  if (!OpenedFilesCnt) return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  for (i = 0; i < OpenedFilesCnt; i++)
    if (!FPROPS(CurrentPage, ReadOnly))
      {
	gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), i);
	conv_func ();
      }
  gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), CurrentPage);
}

void conv_unix_to_dos (void)
{
  gint CurrentPage, res;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  res = replace_all (CurrentPage, TRUE, FALSE, 0, "\n", "\r\n");
  if (!res)
    print_msg ("This file is already DOS formated...");
  else
    print_msg ("File converted from UNIX to DOS Format...");
}


void conv_dos_to_unix (void)
{
  gchar *buffer;
  gint CurrentPage, len;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
  len = strlen (buffer);
  if  (remove_char (buffer, '\r'))
    print_msg ("This file is already UNIX formated...");
  else
    {
      gtk_editable_delete_text
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, len);
      gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				buffer, strlen (buffer), &len);
      print_msg ("File converted from DOS to UNIX Format...");
    }
  g_free(buffer);
}


void conv_dos_to_mac (void)
{
  gchar *buffer;
  gint CurrentPage, len;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
  len = strlen (buffer);
  if  (remove_char (buffer, '\n'))
    print_msg ("This file is already MAC formated...");
  else
    {
      gtk_editable_delete_text
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, len);
      gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				buffer, strlen (buffer), &len);
      print_msg ("File converted from DOS to MAC Format...");
    }
  g_free(buffer);
}


void conv_mac_to_unix (void)
{
  gchar *buffer;
  gint CurrentPage, len;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
  len = strlen (buffer); 
  if  (replace_char (buffer, '\r', '\n'))
    print_msg ("This file is already UNIX formated...");
  else
    {
      gtk_editable_delete_text
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, len);
      gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				buffer, len, &len);
      print_msg ("File converted from MAC to UNIX Format...");
    }
  g_free(buffer);
}


void conv_unix_to_mac (void)
{
  gchar *buffer;
  gint CurrentPage, len;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
  len = strlen (buffer);
  if  (replace_char (buffer, '\n', '\r'))
    print_msg ("This file is already MAC formated...");
  else
    {
      gtk_editable_delete_text
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, len);
      gtk_editable_insert_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				buffer, len, &len);
      print_msg ("File converted from UNIX to MAC Format...");
    }
  g_free(buffer);
}


void conv_mac_to_dos (void)
{
  gint CurrentPage, res;
  
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  res = replace_all (CurrentPage, TRUE, FALSE, 0, "\r", "\r\n");
  if (!res)
    print_msg ("This file is already DOS formated...");
  else
    print_msg ("File converted from MAC to DOS Format...");
}


/* Display a 'File Information' Dialog box */

void file_info (gint CurrentPage)
{
  GtkWidget *FileInfoWindow;
  GtkWidget *Button;
  GtkWidget *HBox;
  GtkWidget *FileInfoLabelLeft, *FileInfoLabelRight;
  gint BufferSize, FileSize, Difference;
  gboolean NewFile = FALSE;
  
  if (!OpenedFilesCnt) return;
  if (stat (FPROPS(CurrentPage, Name), &FPROPS(CurrentPage, Stats)) == -1)
    NewFile = TRUE;
  BufferSize = gtk_text_get_length (GTK_TEXT(FPROPS(CurrentPage, Text)));
  FileSize = NewFile ? 0 : (gint)FPROPS(CurrentPage, Stats).st_size;
  Difference = BufferSize - FileSize;
  FileInfoWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(FileInfoWindow), "File Information"); 
  gtk_window_set_policy (GTK_WINDOW(FileInfoWindow), FALSE, FALSE, FALSE);
  gtk_signal_connect_object (GTK_OBJECT(FileInfoWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileInfoWindow));
  gtk_signal_connect_object (GTK_OBJECT (FileInfoWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileInfoWindow));
  HBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(HBox), 5);
  FileInfoLabelLeft = gtk_label_new
    ("Base name :"
     "\nFull name :"
     "\nLanguage :"
     "\n"
     "\nModified :"
     "\nReadonly :"
     "\n"
     "\nBuffer Size :"
     "\nFile Size :"
     "\nDifference :"
#ifndef WIN32
     "\n"
     "\nPermissions :"
     "\nOwner UID :"
     "\nOwner GID :"
#endif
       );
  gtk_label_set_justify (GTK_LABEL(FileInfoLabelLeft), GTK_JUSTIFY_LEFT); 
  FileInfoLabelRight = gtk_label_new
    (g_strconcat
     (FPROPS(CurrentPage, BaseName),
      "\n", FPROPS(CurrentPage, Name),
      "\n", FPROPS(CurrentPage, WidgetInfo.Lg) == -1 ?
      UNKNOWN : Prefs.L[FPROPS(CurrentPage, WidgetInfo.Lg)].Description,
      "\n",
      "\n", FPROPS(CurrentPage, Changed[0]) ? "Yes" : "No",
      "\n", FPROPS(CurrentPage, ReadOnly) ? "Yes" : "No",
      "\n",
      "\n", g_strdup_printf ("%d", BufferSize),
      ((BufferSize == -1) || (BufferSize == 0) || (BufferSize == 1)) ?
      " Byte" : " Bytes",
      "\n", g_strdup_printf ("%d", FileSize),
      ((FileSize == -1) || (FileSize == 0) || (FileSize == 1)) ?
      " Byte" : " Bytes",
      "\n", g_strdup_printf ("%d", Difference),
      ((Difference == -1) || (Difference == 0) || (Difference == 1)) ?
      " Byte" : " Bytes",
#ifndef WIN32
      "\n",
      "\n", NewFile ?
      UNKNOWN : get_file_mode (FPROPS(CurrentPage, Stats)),
      "\n", NewFile ?
      UNKNOWN : g_strdup_printf ("%d", FPROPS(CurrentPage, Stats).st_uid),
      "\n", NewFile ?
      UNKNOWN : g_strdup_printf ("%d", FPROPS(CurrentPage, Stats).st_gid),
#endif
      NULL));
  gtk_label_set_justify (GTK_LABEL(FileInfoLabelRight), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(HBox), FileInfoLabelLeft, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(HBox), FileInfoLabelRight, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(FileInfoWindow) -> vbox),
		      HBox, FALSE, FALSE, 5);
  Button = gtk_button_new_with_label (" Close ");
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(FileInfoWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(FileInfoWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  gtk_widget_show_all (FileInfoWindow);
  print_msg ("Display File Information window...");
}


/* Print function */

void print_buffer (void)
{
  FILE *File;
  gint CurrentPage;
  char *TempName, *PrintCmd;

  if (!OpenedFilesCnt)
    return;
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if (!gtk_text_get_length (GTK_TEXT(FPROPS(CurrentPage, Text))))
    {
      print_msg ("Nothing to print !");
      return;
    }
  if (!FPROPS(CurrentPage, Changed[0]))
    {
      PrintCmd = g_strconcat (get_string_conf ("General/Misc/PrintCommand"),
			      " \"", FPROPS(CurrentPage, Name), "\"", NULL);
      print_msg (g_strconcat ("File \"", FPROPS(CurrentPage, Name),
			      "\" is being printed...", NULL));
    }
  else
    {
      gchar *Buffer;
      
      TempName = g_strconcat
	(TEMP_DIR, TEMP_PREFIX, FPROPS(CurrentPage, BaseName), NULL);
      if (!(File = fopen (TempName, "w")))
	{
	  print_msg (g_strconcat ("Buffer \"", FPROPS(CurrentPage, BaseName),
				  "\" cannot be printed", NULL));
	  g_free (TempName);
	  return;
	}
      Buffer = gtk_editable_get_chars
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
      fwrite (Buffer, gtk_text_get_length
	      (GTK_TEXT(FPROPS(CurrentPage, Text))), 1, File);
      g_free (Buffer);
      fclose (File);
      PrintCmd = g_strconcat(get_string_conf
			     ("General/Misc/PrintCommand"),
			     " \"", TempName,"\"", NULL);
      print_msg (g_strconcat ("Buffer \"", FPROPS(CurrentPage, BaseName),
			      "\" is being printed...", NULL));
      g_free (TempName);
    }
  if (system (PrintCmd))
    print_msg (g_strconcat ("Problem encountered while trying to print", NULL));
  g_free (PrintCmd);
}
