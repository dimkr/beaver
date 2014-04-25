/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** search.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May 31 03:24:37 2000
** Description:   Beaver search & replace functions source
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
#include <ctype.h>
#include <sys/stat.h>
#include "tools.h"
#include "editor.h"
#include "struct.h"
#include "msgbar.h"
#include "prefs.h"
#include "search.h" 

extern GArray *FileProperties;
extern GtkWidget *MainNotebook;
extern gint OpenedFilesCnt;
extern t_settings	Settings;
static gboolean SearchIsVisible = FALSE;
static gboolean GotoLineIsVisible = FALSE;
static t_search_prefs *SearchPrefs;
static GtkWidget *SearchEntry;
static GtkWidget *ReplaceEntry;
static GtkWidget *List;
static GtkWidget *SpinButton;

/* A sort of 'strcmp' for the characters: it returns '0' if the characteres
   are the same */

gint char_cmp (gboolean case_sen,
	       gchar char1,
	       gchar char2)
{
  if (!case_sen)
    {
      gint diff = 'A' - 'a';
      
      if (isupper(char1) && islower(char2))
	return (char2 + diff - char1);
      else if (isupper(char2) && islower(char1))
	return (char2 - diff - char1);
    }
  return (char2 - char1);
}


/* Return a list of found strings */

t_search_results *find (gint page,
			gboolean case_sen,
			gboolean reg_exp,
			gint start_pos,
			gchar *string)
{
  gchar *buffer;
  gint len, i, j, k = 1, line_nb = 1;
  t_search_results *res;

  (void)reg_exp; /* regular expressions are not yet implemented */
  res = g_malloc (sizeof(t_search_results));
  len = strlen (string);
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(page, Text)), 0, -1);
  for (i = 0; i < start_pos; i++)
    {
      if (buffer[i] == '\n')
	line_nb++;
    }
  for (i = start_pos; buffer[i+len-1] != '\0'; i++)
    {
      if (buffer[i] == '\n')
	line_nb++;
      for (j = 0; (string[j] != '\0') &&
	     !char_cmp(case_sen, buffer[i+j], string[j]); j++);
      if (j == len)
	{
	  res = g_realloc (res, (k + 1) * sizeof(t_search_results));
	  res[k].Line = line_nb;
	  res[k].Begin = i;
	      res[k++].End = i + len;
	}
    }
  res[0].Line = k - 1;
  g_free (buffer);
  return (res);
}


/* Replace all the 'string_in' by 'string_out' */

gint replace_all (gint page,
		  gboolean case_sen,
		  gboolean reg_exp,
		  gint start_pos,
		  gchar *string_in,
		  gchar *string_out)
{
  gchar *buffer_in, *buffer_out;
  gint len_in, len_out, string_in_len, string_out_len;
  gint i, j, k, rep_nb = 0;
  
  (void)reg_exp; /* regular expressions are not yet implemented */
  buffer_in = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(page, Text)), 0, -1);
  len_in = strlen (buffer_in);
  len_out = strlen (buffer_in) + 10;
  string_in_len = strlen (string_in);
  string_out_len = strlen (string_out);
  buffer_out = g_malloc (len_out);
  for (i = 0; i < start_pos; i++)
    buffer_out[i] = buffer_in[i];
  j = i;
  while (i <= len_in)
    {
      for (k = 0; (string_in[k] != '\0') &&
	     !char_cmp(case_sen, buffer_in[i+k], string_in[k]); k++);
      if (k == string_in_len)
	{
	  rep_nb++;
	  len_out = len_out + string_out_len - string_in_len;
	  buffer_out = g_realloc (buffer_out, len_out);
	  for (k = 0; k < string_out_len; j++, k++)
	    buffer_out[j] = string_out[k];
	  i += string_in_len;
	}
      else
	{
	  buffer_out[j] = buffer_in[i];
	  i++;
	  j++;
	}
    }
  buffer_out[j] = '\0';
  gtk_editable_delete_text
    (GTK_EDITABLE(FPROPS(page, Text)), 0, len_in);
  gtk_editable_insert_text (GTK_EDITABLE(FPROPS(page, Text)),
			    buffer_out, strlen(buffer_out), &i);
  g_free (buffer_in);
  g_free (buffer_out);
  return (rep_nb);
}


/* Callback function for the GtkList 'List' */

void goto_search (GtkList *MyList,
		  GtkWidget *Widget)
{
  gint CurrentPage;

  if (!OpenedFilesCnt)
    {
      print_msg ("Hey, you closed all the files!");
      return;
    }
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  if (!strcmp (SearchPrefs -> FileName, FPROPS(CurrentPage, Name)))
    {
      gchar *string, *msg;
      gint Line, Begin, End;

      gtk_label_get (GTK_LABEL(GTK_BIN(Widget) -> child), &string);
      sscanf (string, "%*d. Line %d (%d-%d)", &Line, &Begin, &End);
      if (End <= (gint)gtk_text_get_length(GTK_TEXT(FPROPS(CurrentPage, Text))))
	{
	  gtk_editable_set_position (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				     Begin);
	  gtk_editable_select_region (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				      Begin, End);
	  if (SearchPrefs -> CaseSen)
	    {
	      if (strcmp
		  (get_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text))),
		   SearchPrefs -> StringToSearch))
		{
		  print_msg ("You should refresh the Search...");
		  return;
		}
	    }
	  else
	    if (strcasecmp
		(get_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text))),
		 SearchPrefs -> StringToSearch))
	      {
		print_msg ("Oh oh, you should refresh the Search...");
		return;
	      }
	  msg = g_strdup_printf ("Line %d (%d-%d) selected...",
				 Line, Begin, End);
	  print_msg (msg);
	  g_free (msg);
	}
      else
	print_msg
	  ("Hey, seems like you deleted the part you try to reach now!");
      /* g_free (string);
	 ... causes seg fault, I don't know why... */
    }
  else
    print_msg ("The Current Search wasn't done on this file...");
  (void)MyList; /* avoid the "unused parameter" warning */
}


void search_window_not_visible (void)
{
  SearchIsVisible = FALSE;
}


/* Used to set up Search preferences */

void refresh_search (GtkWidget *Widget, gchar *Data)
{
  if (!strcmp (Data, "cursor"))
    SearchPrefs -> BeginCursorPos = TRUE;
  else if (!strcmp (Data, "begin"))
    SearchPrefs -> BeginCursorPos = FALSE;
  else if (!strcmp (Data, "case_sen"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	SearchPrefs -> CaseSen = TRUE;
      else
	SearchPrefs -> CaseSen = FALSE;
    }
/* TODO: implement the regexp feature */
/*  else if (!strcmp (Data, "reg_exp"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	SearchPrefs -> RegExp = TRUE;
      else
	SearchPrefs -> RegExp = FALSE;
    } */
  else if (!strcmp (Data, "search"))
    {
      gchar *msg;
      gint SearchBegin, i, CurrentPage;
      t_search_results *search_res;
      GtkWidget *ListItem;
  
      if (!OpenedFilesCnt)
	{
	  print_msg ("Hey, you closed all the files!");
	  return;
	}
      SearchPrefs -> StringToSearch = gtk_entry_get_text
	(GTK_ENTRY(SearchEntry));
      if (!strlen(SearchPrefs -> StringToSearch))
	return;
      CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
      SearchPrefs -> FileName = g_strdup (FPROPS(CurrentPage, Name));
      if (SearchPrefs -> BeginCursorPos)
	SearchBegin = gtk_editable_get_position
	  (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
      else
	SearchBegin = 0;
      gtk_list_clear_items (GTK_LIST(List), 0, -1);
      search_res = find (CurrentPage,
			 SearchPrefs -> CaseSen,
			 SearchPrefs -> RegExp,
			 SearchBegin,
			 SearchPrefs -> StringToSearch);
      for (i = 1; i <= search_res[0].Line; i++)
	{
	  gchar *str_temp;
	  
	  str_temp = g_strdup_printf ("%d. Line %d (%d-%d)", i,
				      search_res[i].Line,
				      search_res[i].Begin,
				      search_res[i].End);
	  ListItem = gtk_list_item_new_with_label (str_temp);
	  gtk_container_add (GTK_CONTAINER(List), ListItem);
	  g_free (str_temp);
	}
      gtk_list_select_item (GTK_LIST(List), 0);
      gtk_widget_show_all (List);
      gtk_entry_set_text (GTK_ENTRY(SearchEntry),
			  SearchPrefs -> StringToSearch);
      if (search_res[0].Line == 0)
	msg = g_strdup_printf
	  ("The string \"%s\" hasn't been found...",
	   SearchPrefs -> StringToSearch);
      else
	{
	  gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(Widget)),
			       GTK_WIDGET(GTK_LIST(List)->children->data));
	  if (search_res[0].Line == 1)
	    msg = g_strdup_printf
	      ("One string \"%s\" has been found...",
	       SearchPrefs -> StringToSearch);
	  else
	    msg = g_strdup_printf ("%d \"%s\" strings have been found...",
				   search_res[0].Line,
				   SearchPrefs -> StringToSearch);
	}
      print_msg (msg);
      g_free (msg);
      g_free (search_res);
    }
  else if (!strcmp (Data, "rep_all"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	{
	  SearchPrefs -> RepAll = TRUE;
	}
      else
	{
	  SearchPrefs -> RepAll = FALSE;
	}
    }
  else if (!strcmp (Data, "rep_all_buffers"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	{
	  SearchPrefs -> RepAllBuffers = TRUE;
	}
      else
	{
	  SearchPrefs -> RepAllBuffers = FALSE;
	}
    }
  else if (!strcmp (Data, "replace"))
    {
      gchar *ReplaceString;
      gint SearchBegin, CurrentPage;

      if (!OpenedFilesCnt)
	{
	  print_msg ("Hey, you closed all the files!");
	  return;
	}
      SearchPrefs -> StringToSearch = gtk_entry_get_text
	(GTK_ENTRY(SearchEntry));      
      if (!strlen(SearchPrefs -> StringToSearch))
	return;
      ReplaceString = gtk_entry_get_text (GTK_ENTRY(ReplaceEntry));
      CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
      if (SearchPrefs -> BeginCursorPos)
	SearchBegin = gtk_editable_get_position
	  (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
      else
	SearchBegin = 0;
      if (SearchPrefs -> RepAllBuffers)
	{
	  gchar *msg;
	  gint i;
	  gint rep_nb = 0;
	  for (i = 0; i < OpenedFilesCnt; i++)
	    {
	      if (!FPROPS(CurrentPage, ReadOnly))
		{
		  gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), i);
		  rep_nb += replace_all (i,
					 SearchPrefs -> CaseSen,
					 SearchPrefs -> RegExp,
					 (i == CurrentPage) ? SearchBegin : 0,
					 SearchPrefs -> StringToSearch,
					 ReplaceString);
		}
	    }
	  gtk_notebook_set_page (GTK_NOTEBOOK(MainNotebook), CurrentPage);
	  if (rep_nb == 0)
	    msg = g_strdup_printf
	      ("The string \"%s\" hasn't been found...",
	       SearchPrefs -> StringToSearch);
	  else if (rep_nb == 1)
	    msg = g_strdup_printf
	      ("The string \"%s\" has been replaced by \"%s\"...",
	       SearchPrefs -> StringToSearch, ReplaceString);
	  else
	    msg = g_strdup_printf
	      ("%d \"%s\" strings have been replaced by \"%s\"...",
	       rep_nb, SearchPrefs -> StringToSearch, ReplaceString);
	  print_msg (msg);
	  g_free (msg);
	}
      else if (SearchPrefs -> RepAll)
	{
	  gchar *msg;
	  gint rep_nb;

	  rep_nb = replace_all (CurrentPage,
				SearchPrefs -> CaseSen,
				SearchPrefs -> RegExp,
				SearchBegin,
				SearchPrefs -> StringToSearch,
				ReplaceString);
	  if (rep_nb == 0)
	    msg = g_strdup_printf
	      ("The string \"%s\" hasn't been found...",
	       SearchPrefs -> StringToSearch);
	  else if (rep_nb == 1)
	    msg = g_strdup_printf
	      ("The string \"%s\" has been replaced by \"%s\"...",
	       SearchPrefs -> StringToSearch, ReplaceString);
	  else
	    msg = g_strdup_printf
	      ("%d \"%s\" strings have been replaced by \"%s\"...",
	       rep_nb, SearchPrefs -> StringToSearch, ReplaceString);
	  print_msg (msg);
	  g_free (msg);
	}
      else 
	{
	  t_search_results *search_res;
	  
	  search_res = find (CurrentPage,
			     SearchPrefs -> CaseSen,
			     SearchPrefs -> RegExp,
			     SearchBegin,
			     SearchPrefs -> StringToSearch);
	  if (search_res[0].Line)
	    {
	      gint l_min, l_max;
	      gchar *buffer;
	      
	      l_min = min_from_selection
		(GTK_EDITABLE(FPROPS(CurrentPage, Text)));
	      l_max =
		max_from_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
	      buffer = gtk_editable_get_chars
		(GTK_EDITABLE(FPROPS(CurrentPage, Text)), l_min, l_max);
	      if (!g_strcasecmp (buffer, SearchPrefs -> StringToSearch))
		{
		  gtk_editable_delete_selection
		    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
		  gtk_editable_insert_text
		    (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
		     ReplaceString, strlen(ReplaceString), &SearchBegin);
		  print_msg ("Selection replaced...");
		}	
	      else
		{
		  gtk_editable_set_position
		    (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
		     search_res[1].Begin);
		  gtk_editable_select_region
		    (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
		     search_res[1].Begin, search_res[1].End);
		}
	      g_free (buffer);
	    }
	  else
	    {
	      if (BEEP) gdk_beep ();
	      print_msg ("End of Search...");
	    }
	  g_free (search_res); 
	}
      gtk_entry_set_text (GTK_ENTRY(SearchEntry),
			  SearchPrefs -> StringToSearch);
      gtk_entry_set_text (GTK_ENTRY(ReplaceEntry), ReplaceString);
    }
}


/* Display a Search Dialog window */

void search (gboolean Replace)
{
  GtkWidget *SearchWindow;
  GtkWidget *VBox1, *VBox2;
  GtkWidget *Frame;
  GtkWidget *ScrolledWindow;
  GtkWidget *Button;
  GSList *Group;
  gboolean homogenous = FALSE;

  if (SearchIsVisible) return;
  SearchPrefs = g_malloc (sizeof(t_search_prefs));
  SearchPrefs -> BeginCursorPos = TRUE;
  SearchPrefs -> CaseSen = FALSE;
  SearchPrefs -> RegExp = FALSE;
  SearchWindow = gtk_dialog_new ();
  gtk_window_set_policy (GTK_WINDOW(SearchWindow), FALSE, FALSE, FALSE);
  gtk_signal_connect (GTK_OBJECT(SearchWindow), "delete_event",
		      (GtkSignalFunc) search_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(SearchWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(SearchWindow));
  gtk_signal_connect (GTK_OBJECT(SearchWindow), "destroy",
		      (GtkSignalFunc) search_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT (SearchWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(SearchWindow));
  if (Replace)
    {
      gtk_window_set_title (GTK_WINDOW(SearchWindow),
			    "Search and Replace Text");
      SearchPrefs -> RepAll = FALSE;
      SearchPrefs -> RepAllBuffers = FALSE;
    }
  else
    {
      gtk_window_set_title (GTK_WINDOW(SearchWindow), "Search Text");
      homogenous = TRUE;
    }
  VBox1 = gtk_vbox_new (homogenous, 5);
  gtk_container_set_border_width (GTK_CONTAINER(VBox1), 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(SearchWindow) -> vbox),
		      VBox1, FALSE, FALSE, 0);
  Frame = gtk_frame_new ("Search");
  gtk_box_pack_start (GTK_BOX(VBox1), Frame, TRUE, TRUE, 0);
  VBox2 = gtk_vbox_new (FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER(VBox2), 5);
  gtk_container_add (GTK_CONTAINER(Frame), VBox2);
  SearchEntry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX(VBox2), SearchEntry, FALSE, FALSE, 0);
  Button = gtk_radio_button_new_with_label (NULL, "Start at cursor position");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_search, "cursor");
  gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0);
  Group = gtk_radio_button_group (GTK_RADIO_BUTTON(Button));
  Button = gtk_radio_button_new_with_label
    (Group, "Start at beginning of the document");
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_search, "begin");
  gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0);
  Button = gtk_check_button_new_with_label ("Case sensitive");
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_search, "case_sen"); 
  gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0);
/* TODO: implement the regexp feature */
/*  Button = gtk_check_button_new_with_label ("Regular expression search");
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_search, "reg_exp"); 
  gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0); */
  if (Replace)
    {
      Frame = gtk_frame_new ("Replace");
      gtk_box_pack_start (GTK_BOX(VBox1), Frame, TRUE, TRUE, 0);
      VBox2 = gtk_vbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER(VBox2), 5);
      gtk_container_add (GTK_CONTAINER(Frame), VBox2);
      ReplaceEntry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX(VBox2), ReplaceEntry, FALSE, FALSE, 0);
      Button = gtk_check_button_new_with_label ("Replace all");
      gtk_signal_connect (GTK_OBJECT(Button), "toggled",
			  (GtkSignalFunc)refresh_search, "rep_all");
      gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0);
      Button = gtk_check_button_new_with_label ("Replace all in every buffer");
      gtk_signal_connect (GTK_OBJECT(Button), "toggled",
			  (GtkSignalFunc)refresh_search, "rep_all_buffers");
      gtk_box_pack_start (GTK_BOX(VBox2), Button, FALSE, FALSE, 0);
      Button = gtk_button_new_with_label (" Replace ");
      gtk_signal_connect (GTK_OBJECT(Button), "clicked",
			  (GtkSignalFunc)refresh_search, "replace");
      print_msg ("Display Search and Replace window...");
    }
  else
    {
      Frame = gtk_frame_new ("Results");
      gtk_box_pack_start (GTK_BOX(VBox1), Frame, TRUE, TRUE, 0);
      VBox2 = gtk_vbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER(VBox2), 5);
      gtk_container_add (GTK_CONTAINER(Frame), VBox2);
      ScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(ScrolledWindow),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX(VBox2), ScrolledWindow, TRUE, TRUE, 0);
      List = gtk_list_new ();
      gtk_list_set_selection_mode (GTK_LIST(List), GTK_SELECTION_BROWSE);
      gtk_signal_connect (GTK_OBJECT(List), "select_child",
			  (GtkSignalFunc)goto_search, NULL);
      gtk_scrolled_window_add_with_viewport
	(GTK_SCROLLED_WINDOW(ScrolledWindow), List);
      gtk_container_set_focus_vadjustment
	(GTK_CONTAINER (List),
	 gtk_scrolled_window_get_vadjustment
	 (GTK_SCROLLED_WINDOW (ScrolledWindow)));
      gtk_container_set_focus_hadjustment
	(GTK_CONTAINER (List),
	 gtk_scrolled_window_get_hadjustment
	 (GTK_SCROLLED_WINDOW (ScrolledWindow)));
      Button = gtk_button_new_with_label (" Search ");
      gtk_signal_connect (GTK_OBJECT(Button), "clicked",
			  (GtkSignalFunc)refresh_search, "search");
      print_msg ("Display Search window...");
    }
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(SearchWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  Button = gtk_button_new_with_label (" Cancel ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) search_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(SearchWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(SearchWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_show_all (SearchWindow);
  SearchIsVisible = TRUE;
}


void set_line (void)
{
  gint CurrentPage, i, j = 0, len, line_to_go, line_nb = 1;
  gchar *buffer, *msg;

  if (!OpenedFilesCnt)
    {
      print_msg ("Hey, you closed all the files!");
      return;
    }
  line_to_go = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(SpinButton));
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  buffer = gtk_editable_get_chars
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
  len = strlen (buffer);
  for (i = 0; ((i <= len) && (line_nb < line_to_go)); i++)
    {
      if (buffer[i] == '\n')
	{
	  line_nb++;
	  j = i + 1;
	}
    }
  gtk_editable_set_position
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), j);
  for (i = j; ((buffer[i] != '\n') && (buffer[i] != '\0')); i++);
  gtk_editable_select_region (GTK_EDITABLE(FPROPS(CurrentPage, Text)), j, i);
  if (line_nb < line_to_go)
    {
      gchar *line_nb_str;

      line_nb_str = g_strdup_printf ("%d", line_nb);
      g_free (line_nb_str);
      msg = g_strdup_printf
	("Hey, this file has less lines than you think! Line %d selected...",
	 line_nb);
    }
  else
    msg = g_strdup_printf ("Line %d selected...", line_nb);
  print_msg (msg);
  g_free (msg);
  g_free (buffer);
}


void goto_line_window_not_visible (void)
{
  GotoLineIsVisible = FALSE;
}


void goto_line (void)
{
  GtkWidget *GotoLineWindow;
  GtkWidget *HBox;
  GtkWidget *Label;
  GtkWidget *Button;
  GtkObject *Size;

  if (GotoLineIsVisible) return;
  GotoLineWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(GotoLineWindow), "Goto Line..."); 
  gtk_window_set_policy (GTK_WINDOW(GotoLineWindow), FALSE, FALSE, FALSE);
  gtk_signal_connect (GTK_OBJECT(GotoLineWindow), "delete_event",
		      (GtkSignalFunc) goto_line_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(GotoLineWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(GotoLineWindow));
  gtk_signal_connect (GTK_OBJECT (GotoLineWindow), "destroy",
		      (GtkSignalFunc) goto_line_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT (GotoLineWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(GotoLineWindow));
  HBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(HBox), 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(GotoLineWindow) -> vbox),
		      HBox, FALSE, FALSE, 0);
  Label = gtk_label_new ("Line :");
  gtk_box_pack_start (GTK_BOX(HBox), Label, FALSE, FALSE, 0);
  Label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(HBox), Label, FALSE, FALSE, 3);
  Size = gtk_adjustment_new (1, 1, 0xffff, 1, 5, 0);
  SpinButton = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_entry_set_max_length (GTK_ENTRY(SpinButton), 5);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(SpinButton), TRUE);
  gtk_box_pack_start (GTK_BOX(HBox), SpinButton, TRUE, TRUE, 0);
  Button = gtk_button_new_with_label (" OK ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc)set_line, NULL);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) goto_line_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(GotoLineWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(GotoLineWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  Button = gtk_button_new_with_label (" Cancel ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) goto_line_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(GotoLineWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(GotoLineWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_window_set_focus (GTK_WINDOW(GotoLineWindow), SpinButton);
  gtk_widget_show_all (GotoLineWindow);
  print_msg ("Display Goto Line window...");
  GotoLineIsVisible = TRUE;
}
