/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** prefs.c
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed Jan 10 20:30:44 2001
** Description:   Beaver preferences tool source
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
#include <string.h>
#include <sys/stat.h>
#include "interface.h"
#include "msgbar.h"
#include "conf.h"
#include "editor.h"
#include "struct.h"
#include "main.h"
#include "filesops.h"
#include "prefs.h"


extern GtkWidget *MainWindow;
extern GtkWidget *MainNotebook;
extern GArray *FileProperties;
extern gint OpenedFilesCnt;
static gboolean PreferencesAreVisible = FALSE;
static gboolean FontIsVisible = FALSE;
static gboolean ColorIsVisible = FALSE;
static gboolean WordFileIsVisible = FALSE;
static t_settings set_temp;


t_settings init_settings (void)
{
  t_settings set;

  set.recent_files = get_int_conf ("General/RecentFiles/MaxNb");
  set.main_window_size_autosave =
    get_bool_conf ("General/Window/Autosave");
  set.main_window_width = get_int_conf ("General/Window/Width");
  set.main_window_height = get_int_conf ("General/Window/Height");
  set.msgbar_display = get_bool_conf ("General/MsgBar/Display");
  set.msgbar_interval = get_int_conf ("General/MsgBar/Interval");
  set.toggle_wordwrap = get_bool_conf ("General/Editor/Wordwrap");
  set.toolbar_display = get_bool_conf ("General/ToolBar/Display");
  set.max_tab_label_length = get_int_conf ("General/Tabs/LabelLength");
  set.tab_position = get_int_conf ("General/Tabs/Position");
  set.scrollbar_position = get_int_conf ("General/ScrollBar/Position");
  set.complete_window_width =
    get_int_conf ("General/CompletionPopUp/Width");
  set.complete_window_height =
    get_int_conf ("General/CompletionPopUp/Height");
  set.backup = get_bool_conf ("General/AutoSave/Backup");
  set.backup_ext = get_string_conf ("General/AutoSave/BackupExt");
  set.autosave_delay = get_int_conf ("General/AutoSave/Delay");
  set.directory = get_string_conf ("General/RecentFiles/Directory");
  set.font = get_string_conf ("General/Editor/Font");
  set.print_cmd = get_string_conf ("General/Misc/PrintCommand");
  set.wordfile = get_string_conf ("General/Editor/Wordfile");
  set.bg[0] = get_int_conf ("General/Editor/BGRed");
  set.bg[1] = get_int_conf ("General/Editor/BGGreen");
  set.bg[2] = get_int_conf ("General/Editor/BGBlue");
  set.fg[0] = get_int_conf ("General/Editor/FGRed");
  set.fg[1] = get_int_conf ("General/Editor/FGGreen");
  set.fg[2] = get_int_conf ("General/Editor/FGBlue");
  set.selected_bg[0] = get_int_conf ("General/Editor/SelectedBGRed");
  set.selected_bg[1] = get_int_conf ("General/Editor/SelectedBGGreen");
  set.selected_bg[2] = get_int_conf ("General/Editor/SelectedBGBlue");
  set.selected_fg[0] = get_int_conf ("General/Editor/SelectedFGRed");
  set.selected_fg[1] = get_int_conf ("General/Editor/SelectedFGGreen");
  set.selected_fg[2] = get_int_conf ("General/Editor/SelectedFGBlue");
  set.beep = get_bool_conf ("General/Misc/Beep");
  set.syn_high = get_bool_conf ("General/Adv/SynHigh");
  set.syn_high_depth = get_int_conf ("General/Adv/SynHighDepth");
  set.auto_indent = get_bool_conf ("General/Adv/AutoIndent");
  set.auto_correct = get_bool_conf ("General/Adv/AutoCorrect");
  return (set);
}


void set_preferences_to_disk (t_settings *set, t_colors *col)
{
  if (set)
    {
      set_int_conf ("General/RecentFiles/MaxNb", set -> recent_files);
      set_bool_conf ("General/Window/Autosave",
		     set -> main_window_size_autosave);
      if (set -> main_window_size_autosave)
	{
	  set_int_conf ("General/Window/Width",
			MainWindow -> allocation.width);
	  set_int_conf ("General/Window/Height",
			MainWindow -> allocation.height);
	}
      else
	{
	  set_int_conf ("General/Window/Width", set -> main_window_width);
	  set_int_conf ("General/Window/Height", set -> main_window_height);
	}
      set_bool_conf ("General/MsgBar/Display", set -> msgbar_display);
      set_int_conf ("General/MsgBar/Interval", set -> msgbar_interval);
      set_bool_conf ("General/Editor/Wordwrap", set -> toggle_wordwrap);
      set_bool_conf ("General/ToolBar/Display", set -> toolbar_display);
      set_int_conf ("General/Tabs/LabelLength", set -> max_tab_label_length);
      set_int_conf ("General/Tabs/Position", set -> tab_position);
      set_int_conf ("General/ScrollBar/Position", set -> scrollbar_position);
      set_int_conf ("General/CompletionPopUp/Width",
		    set -> complete_window_width);
      set_int_conf ("General/CompletionPopUp/Height",
		    set -> complete_window_height);
      set_int_conf ("General/Editor/SelectedBGRed", set -> selected_bg[0]);
      set_int_conf ("General/Editor/SelectedBGGreen", set -> selected_bg[1]);
      set_int_conf ("General/Editor/SelectedBGBlue", set -> selected_bg[2]);
      set_int_conf ("General/Editor/SelectedFGRed", set -> selected_fg[0]);
      set_int_conf ("General/Editor/SelectedFGGreen", set -> selected_fg[1]);
      set_int_conf ("General/Editor/SelectedFGBlue", set -> selected_fg[2]);
      set_int_conf ("General/Editor/BGRed", set -> bg[0]);
      set_int_conf ("General/Editor/BGGreen", set -> bg[1]);
      set_int_conf ("General/Editor/BGBlue", set -> bg[2]);
      set_int_conf ("General/Editor/FGRed", set -> fg[0]);
      set_int_conf ("General/Editor/FGGreen", set -> fg[1]);
      set_int_conf ("General/Editor/FGBlue", set -> fg[2]);
      set_bool_conf ("General/AutoSave/Backup", set -> backup);
      set_string_conf ("General/AutoSave/BackupExt", set -> backup_ext);
      set_int_conf ("General/AutoSave/Delay", set -> autosave_delay);
      set_string_conf ("General/RecentFiles/Directory", set -> directory);
      set_string_conf ("General/Editor/Font", set -> font);
      set_string_conf ("General/Misc/PrintCommand", set -> print_cmd);
      set_bool_conf ("General/Misc/Beep", set -> beep);
      set_string_conf ("General/Editor/Wordfile", set -> wordfile);
      set_bool_conf ("General/Adv/SynHigh", set -> syn_high);
      set_int_conf ("General/Adv/SynHighDepth", set -> syn_high_depth);
      set_bool_conf ("General/Adv/AutoIndent", set -> auto_indent);
      set_bool_conf ("General/Adv/AutoCorrect", set -> auto_correct);
    }
  (void)col; /* avoid the "unused parameter" warning */
}


void preferences_window_not_visible (void)
{
  PreferencesAreVisible = FALSE;
}


void font_window_not_visible (void)
{
  FontIsVisible = FALSE;
}


void color_window_not_visible (void)
{
  ColorIsVisible = FALSE;
}


void wordfile_window_not_visible (void)
{
  WordFileIsVisible = FALSE;
}


void toggle_sen (GtkWidget *Widget)
{
  if (GTK_WIDGET_SENSITIVE(Widget))
    gtk_widget_set_sensitive (Widget, FALSE);
  else
    gtk_widget_set_sensitive (Widget, TRUE);
}


gboolean text_preview (GtkWidget *Widget)
{
  GtkStyle *Style;

  Style = gtk_style_copy (gtk_widget_get_style(Widget));
  Style -> font = gdk_font_load (set_temp.font);
  Style -> bg[GTK_STATE_SELECTED].red = set_temp.selected_bg[0];
  Style -> bg[GTK_STATE_SELECTED].green = set_temp.selected_bg[1];
  Style -> bg[GTK_STATE_SELECTED].blue = set_temp.selected_bg[2];
  Style -> fg[GTK_STATE_SELECTED].red = set_temp.selected_fg[0];
  Style -> fg[GTK_STATE_SELECTED].green = set_temp.selected_fg[1];
  Style -> fg[GTK_STATE_SELECTED].blue = set_temp.selected_fg[2];
  Style -> base[GTK_STATE_NORMAL].red = set_temp.bg[0];
  Style -> base[GTK_STATE_NORMAL].green = set_temp.bg[1];
  Style -> base[GTK_STATE_NORMAL].blue = set_temp.bg[2];
  Style -> text[GTK_STATE_NORMAL].red = set_temp.fg[0];
  Style -> text[GTK_STATE_NORMAL].green = set_temp.fg[1];
  Style -> text[GTK_STATE_NORMAL].blue = set_temp.fg[2];
  gtk_widget_set_style (Widget, Style);
  return FALSE;
}


void apply_changes (GtkWidget *Widget, t_settings *set)
{
  GtkWidget *HandleBox;
  GtkWidget *Menu;

  *set = set_temp;
  set_preferences_to_disk (set, NULL);
  HandleBox = gtk_container_children
    (GTK_CONTAINER
     (GTK_BIN
      (gtk_widget_get_toplevel(MainNotebook)) -> child)) -> data;
  gtk_widget_destroy (GTK_BIN(HandleBox) -> child);
  Menu = menubar_new (gtk_widget_get_toplevel(HandleBox));
  gtk_container_add (GTK_CONTAINER(HandleBox), Menu);
  gtk_widget_show_all (Menu);
  if (set -> tab_position == 1)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_TOP);
  if (set -> tab_position == 2)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_BOTTOM);
  if (set -> tab_position == 3)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_LEFT);
  if (set -> tab_position == 4)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_RIGHT);
  if (set -> msgbar_display)
    show_msgbar ();
  else
    hide_msgbar ();
  autosave (set_temp.autosave_delay);
  if (OpenedFilesCnt)
    {
      GtkStyle *Style;
      gint i;

      Style = gtk_style_copy
	(gtk_widget_get_style(FPROPS(0, Text)));
      Style -> font = gdk_font_load (set -> font);
      Style -> bg[GTK_STATE_SELECTED].red = set -> selected_bg[0];
      Style -> bg[GTK_STATE_SELECTED].green = set -> selected_bg[1];
      Style -> bg[GTK_STATE_SELECTED].blue = set -> selected_bg[2];
      Style -> fg[GTK_STATE_SELECTED].red = set -> selected_fg[0];
      Style -> fg[GTK_STATE_SELECTED].green = set -> selected_fg[1];
      Style -> fg[GTK_STATE_SELECTED].blue = set -> selected_fg[2];
      Style -> base[GTK_STATE_NORMAL].red = set -> bg[0];
      Style -> base[GTK_STATE_NORMAL].green = set -> bg[1];
      Style -> base[GTK_STATE_NORMAL].blue = set -> bg[2];
      Style -> text[GTK_STATE_NORMAL].red = set -> fg[0];
      Style -> text[GTK_STATE_NORMAL].green = set -> fg[1];
      Style -> text[GTK_STATE_NORMAL].blue = set -> fg[2];
      for (i = 0; i < OpenedFilesCnt; i++)
	{
	  gtk_widget_set_style (FPROPS(i, Text), Style);
	  set_label (GTK_NOTEBOOK(MainNotebook), i);
	}
      if (set -> scrollbar_position == 1)
	for (i = 0; i < OpenedFilesCnt; i++)
	  gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW
					     (gtk_notebook_get_nth_page
					      (GTK_NOTEBOOK(MainNotebook), i)),
					     GTK_CORNER_TOP_RIGHT);
      if (set -> scrollbar_position == 2)
	for (i = 0; i < OpenedFilesCnt; i++)
	  gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW
					     (gtk_notebook_get_nth_page
					      (GTK_NOTEBOOK(MainNotebook), i)),
					     GTK_CORNER_TOP_LEFT);
      if (set -> toggle_wordwrap)
	for (i = 0; i < OpenedFilesCnt; i++)
	  gtk_text_set_word_wrap (GTK_TEXT(FPROPS(i, Text)), TRUE);
      else
	for (i = 0; i < OpenedFilesCnt; i++)
	  gtk_text_set_word_wrap (GTK_TEXT(FPROPS(i, Text)), FALSE);
    }
  (void)Widget; /* avoid the "unused parameter" warning */
  print_msg ("Preferences saved...");
}


void apply_font (GtkFontSelectionDialog *fsd)
{
  set_temp.font = gtk_font_selection_dialog_get_font_name (fsd);
}


void apply_wordfile_path (GtkWidget *filesel_button, GtkWidget *Widget)
{
  GtkWidget *filesel;
  gchar *FileName;
  struct stat Stats;

  filesel = gtk_widget_get_toplevel (filesel_button);
  FileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION(filesel));
  if (stat (FileName, &Stats) == -1) return;
  if ((Stats.st_mode & S_IFMT) == S_IFDIR) return;
  gtk_entry_set_text
  (GTK_ENTRY
   (gtk_container_children
    (GTK_CONTAINER(Widget -> parent)) -> next -> data), FileName);
  gtk_widget_destroy (filesel);
  WordFileIsVisible = FALSE;
}


void apply_font_button (GtkWidget *Button)
{
  gchar *Text1, *Text2;

  Text1 = g_strndup (set_temp.font, 40);
  Text2 = g_strconcat (Text1, "...", NULL);
  gtk_label_set_text (GTK_LABEL(GTK_BIN(Button) -> child), Text2);
  g_free (Text1);
  g_free (Text2);
}


void apply_color_button (GtkWidget *Widget, t_widgint *Data)
{
  GtkStyle *Style;
  gint Colors[3];
  gint i;
  
  if (Widget)
    {
      gdouble colors_from_csd[3];
      
      gtk_color_selection_get_color
	(GTK_COLOR_SELECTION
	 (((GtkColorSelectionDialog *)
	   gtk_widget_get_toplevel(Widget)) -> colorsel), colors_from_csd);
      for (i = 0; i <= 2; i++)
	Colors[i] = 0xffff * colors_from_csd[i];
      switch (Data -> op)
	{
	case 1:
	  for (i = 0; i <= 2; i++)
	    set_temp.fg[i] = Colors[i];
	  break;
	case 2:
	  for (i = 0; i <= 2; i++)
	    set_temp.bg[i] = Colors[i];
	  break;
	case 3:
	  for (i = 0; i <= 2; i++)
	    set_temp.selected_fg[i] = Colors[i];
	  break;
	case 4:
	  for (i = 0; i <= 2; i++)
	    set_temp.selected_bg[i] = Colors[i];
	  break;
	}
    }
  else
    {
      switch (Data -> op)
	{
	case 1:
	  for (i = 0; i <= 2; i++)
	    Colors[i] = set_temp.fg[i];
	  break;
	case 2:
	  for (i = 0; i <= 2; i++)
	    Colors[i] = set_temp.bg[i];
	  break;
	case 3:
	  for (i = 0; i <= 2; i++)
	    Colors[i] = set_temp.selected_fg[i];
	  break;
	case 4:
	  for (i = 0; i <= 2; i++)
	    Colors[i] = set_temp.selected_bg[i];
	  break;
	}
    }
  Style = gtk_style_copy (gtk_widget_get_style
			  (GTK_BIN(Data -> widget) -> child));
  Style -> base[GTK_STATE_NORMAL].red = Colors[0];
  Style -> base[GTK_STATE_NORMAL].green = Colors[1];
  Style -> base[GTK_STATE_NORMAL].blue = Colors[2];
  gtk_widget_set_style (GTK_BIN(Data -> widget) -> child, Style);
}


void refresh_prefs (GtkWidget *Widget, gchar *Data)
{
  if (!strcmp (Data, "save_win_size"))
    set_temp.main_window_size_autosave = TRUE;
  else if (!strcmp (Data, "fix_win_size"))
    set_temp.main_window_size_autosave = FALSE;
  else if (!strcmp (Data, "win_width"))
    set_temp.main_window_width = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "win_height"))
    set_temp.main_window_height = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "tab_pos_1"))
    set_temp.tab_position = 1;
  else if (!strcmp (Data, "tab_pos_2"))
    set_temp.tab_position = 2;
  else if (!strcmp (Data, "tab_pos_3"))
    set_temp.tab_position = 3;
  else if (!strcmp (Data, "tab_pos_4"))
    set_temp.tab_position = 4;
  else if (!strcmp (Data, "sb_pos_1"))
    set_temp.scrollbar_position = 1;
  else if (!strcmp (Data, "sb_pos_2"))
    set_temp.scrollbar_position = 2;
  else if (!strcmp (Data, "tab_len"))
    set_temp.max_tab_label_length = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "msg_bar"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.msgbar_display = TRUE;
      else
	set_temp.msgbar_display = FALSE;
    }
  else if (!strcmp (Data, "msg_len"))
    set_temp.msgbar_interval = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "beep"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.beep = TRUE;
      else
	set_temp.beep = FALSE;
    }
  else if (!strcmp (Data, "print_cmd"))
    set_temp.print_cmd = g_strdup (gtk_entry_get_text (GTK_ENTRY(Widget)));
  else if (!strcmp (Data, "wordwrap"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.toggle_wordwrap = TRUE;
      else
	set_temp.toggle_wordwrap = FALSE;
    }
  else if (!strncmp (Data, "color", 5))
    {
      gdouble colors[3];
      t_widgint *col_but;
      GtkColorSelectionDialog *csd;

      if (ColorIsVisible) return;
      col_but = g_malloc (sizeof(t_widgint));
      col_but -> widget = Widget;
      switch (Data[5] - '0')
	{
	  gint i;
	 
	case 1:
	  col_but -> op = 1;
	  for (i = 0; i <= 2; i++)
	    colors[i] = (gdouble)set_temp.fg[i]/0xffff;
	  break;
	case 2:
	  col_but -> op = 2;
	  for (i = 0; i <= 2; i++)
	    colors[i] = (gdouble)set_temp.bg[i]/0xffff;
	  break;
	case 3:
	  col_but -> op = 3;
	  for (i = 0; i <= 2; i++)
	    colors[i] = (gdouble)set_temp.selected_fg[i]/0xffff;
	  break;
	case 4:
	  col_but -> op = 4;
	  for (i = 0; i <= 2; i++)
	    colors[i] = (gdouble)set_temp.selected_bg[i]/0xffff;
	  break;
	}
      csd = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new
	("Select Color");
      gtk_signal_connect (GTK_OBJECT(csd), "delete_event",
			  (GtkSignalFunc) color_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(csd), "delete_event",
				 (GtkSignalFunc) gtk_widget_destroy,
				 GTK_OBJECT(csd));
      gtk_signal_connect (GTK_OBJECT (csd), "destroy",
			  (GtkSignalFunc) color_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(csd), "destroy",
				 (GtkSignalFunc) gtk_widget_destroy,
				 GTK_OBJECT(csd));
      gtk_color_selection_set_color (GTK_COLOR_SELECTION(csd -> colorsel),
				     colors);
      gtk_signal_connect (GTK_OBJECT(csd -> ok_button), "clicked",
			  (GtkSignalFunc)apply_color_button, col_but);
      gtk_signal_connect (GTK_OBJECT(csd -> ok_button), "clicked",
			  (GtkSignalFunc)color_window_not_visible, NULL);
      gtk_signal_connect_object	(GTK_OBJECT(csd -> ok_button), "clicked",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(csd));
      gtk_signal_connect (GTK_OBJECT(csd -> cancel_button), "clicked",
			  (GtkSignalFunc)color_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(csd -> cancel_button), "clicked",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(csd));
      gtk_widget_hide (csd -> help_button);
      ColorIsVisible = TRUE;
      gtk_widget_show (GTK_WIDGET(csd));
      print_msg ("Display Color selection window...");
    }
  else if (!strcmp (Data, "font"))
    {
      GtkFontSelectionDialog *fsd;

      if (FontIsVisible) return;
      fsd = (GtkFontSelectionDialog *)gtk_font_selection_dialog_new
	("Select Font");
      gtk_signal_connect (GTK_OBJECT(fsd), "delete_event",
			  (GtkSignalFunc) font_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(fsd), "delete_event",
				 (GtkSignalFunc) gtk_widget_destroy,
				 GTK_OBJECT(fsd));
      gtk_signal_connect (GTK_OBJECT (fsd), "destroy",
			  (GtkSignalFunc) font_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(fsd), "destroy",
				 (GtkSignalFunc) gtk_widget_destroy,
				 GTK_OBJECT(fsd));
      gtk_font_selection_dialog_set_font_name (fsd, set_temp.font);
      gtk_signal_connect_object (GTK_OBJECT(fsd -> ok_button), "clicked",
				 (GtkSignalFunc)apply_font, GTK_OBJECT(fsd));
      gtk_signal_connect_object (GTK_OBJECT(fsd -> ok_button), "clicked",
				 (GtkSignalFunc)apply_font_button,
				 GTK_OBJECT(Widget));
      gtk_signal_connect (GTK_OBJECT(fsd -> ok_button), "clicked",
			  (GtkSignalFunc)font_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(fsd -> ok_button), "clicked",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(fsd));
      gtk_signal_connect (GTK_OBJECT (fsd -> cancel_button), "clicked",
			  (GtkSignalFunc)font_window_not_visible, NULL);
      gtk_signal_connect_object (GTK_OBJECT(fsd -> cancel_button), "clicked",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(fsd));
      FontIsVisible = TRUE;
      gtk_widget_show (GTK_WIDGET(fsd));
      print_msg ("Display Font selection window...");
    }
  else if (!strcmp (Data, "backup"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.backup = TRUE;
      else
	set_temp.backup = FALSE;
    }
  else if (!strcmp (Data, "backup_ext"))
    {
      if (strlen(gtk_entry_get_text (GTK_ENTRY(Widget))))
	set_temp.backup_ext = g_strdup(gtk_entry_get_text (GTK_ENTRY(Widget)));
    }
  else if (!strcmp (Data, "autosave"))
    set_temp.autosave_delay = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "recent_files"))
    set_temp.recent_files = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "wordfile"))
    {
      if (strlen(gtk_entry_get_text (GTK_ENTRY(Widget))))
	set_temp.wordfile = g_strdup(gtk_entry_get_text (GTK_ENTRY(Widget)));
    }
  else if (!strcmp (Data, "word_browse"))
    {
      GtkWidget *FileSelector;
      GtkWidget *Button;
      
      FileSelector = gtk_file_selection_new ("Wordfile Path");
      gtk_file_selection_set_filename (GTK_FILE_SELECTION(FileSelector),
				       set_temp.wordfile);
      gtk_signal_connect (GTK_OBJECT(FileSelector), "delete_event",
			  (GtkSignalFunc)wordfile_window_not_visible,
			  NULL);
      gtk_signal_connect_object (GTK_OBJECT(FileSelector), "delete_event",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(FileSelector));
      gtk_signal_connect (GTK_OBJECT(FileSelector), "destroy_event",
			  (GtkSignalFunc)wordfile_window_not_visible,
			  NULL);
      gtk_signal_connect_object (GTK_OBJECT(FileSelector), "destroy_event",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(FileSelector));
      Button = GTK_FILE_SELECTION(FileSelector) -> ok_button;
      gtk_signal_connect (GTK_OBJECT(Button), "clicked",
			  (GtkSignalFunc)apply_wordfile_path,
			  Widget);
      Button = GTK_FILE_SELECTION (FileSelector) -> cancel_button;
      gtk_signal_connect (GTK_OBJECT(Button), "clicked",
			  (GtkSignalFunc)wordfile_window_not_visible,
			  NULL);
      gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
				 (GtkSignalFunc)gtk_widget_destroy,
				 GTK_OBJECT(FileSelector));
      WordFileIsVisible = TRUE;
      gtk_widget_show (FileSelector);
      print_msg ("Display Wordfile selection window...");
    }
  else if (!strcmp (Data, "comp_width"))
    set_temp.complete_window_width = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "comp_height"))
    set_temp.complete_window_height = gtk_spin_button_get_value_as_int
      (GTK_SPIN_BUTTON(Widget));
  else if (!strcmp (Data, "syn_high"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.syn_high = TRUE;
      else
	set_temp.syn_high = FALSE;
    }
  else if (!strcmp (Data, "syn_high_depth"))
    {
      set_temp.syn_high_depth = gtk_spin_button_get_value_as_int
	(GTK_SPIN_BUTTON(Widget));
    }
  else if (!strcmp (Data, "auto_indent"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.auto_indent = TRUE;
      else
	set_temp.auto_indent = FALSE;
    }
  else if (!strcmp (Data, "auto_correct"))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(Widget)))
	set_temp.auto_correct = TRUE;
      else
	set_temp.auto_correct = FALSE;
    }
}


void display_general (GtkNotebook *Notebook)
{
  GtkWidget *VBox, *FrameVBox, *FrameHBox;
  GtkWidget *Frame;
  GtkWidget *Table;
  GtkWidget *Label;
  GSList *Group;
  GtkWidget *Button;
  GtkWidget *Menu;
  GtkWidget *MenuItem;
  GtkWidget *Entry;
  GtkObject *Size;

  VBox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(VBox), 10);

  Frame = gtk_frame_new ("Window size");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  FrameVBox = gtk_vbox_new (FALSE, 0);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(FrameVBox), 5);
  gtk_container_add (GTK_CONTAINER(Frame), FrameVBox);
  Button = gtk_radio_button_new_with_label (NULL, "Save when you quit");
  gtk_signal_connect_object (GTK_OBJECT(Button), "toggled",
			     (GtkSignalFunc)toggle_sen, GTK_OBJECT(FrameHBox));
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "save_win_size");
  gtk_box_pack_start (GTK_BOX(FrameVBox), Button, FALSE, FALSE, 0);
  Group = gtk_radio_button_group (GTK_RADIO_BUTTON(Button));
  Button = gtk_radio_button_new_with_label (Group, "Fix the size");
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "fix_win_size");
  gtk_box_pack_start (GTK_BOX(FrameVBox), Button, FALSE, FALSE, 0);
  if (!set_temp.main_window_size_autosave)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
      gtk_widget_set_sensitive (FrameHBox, TRUE);
    }
  else
      gtk_widget_set_sensitive (FrameHBox, FALSE);
  gtk_box_pack_start (GTK_BOX(FrameVBox), FrameHBox, FALSE, FALSE, 5);
  Label = gtk_label_new ("Width");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.main_window_width, 10,
			     gdk_screen_width (), 1, 5, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_widget_set_usize (Button, 60, -1);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "win_width");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Button, FALSE, FALSE, 0);
  Label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 40);
  Label = gtk_label_new ("Height");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.main_window_height, 10,
			     gdk_screen_height (), 1, 5, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_widget_set_usize (Button, 60, -1); 
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "win_height");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Button, FALSE, FALSE, 0);

  Frame = gtk_frame_new ("Doc Tabs");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  Table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 0, 1);
  Label = gtk_label_new ("Tabs position");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 1, 2);
  Label = gtk_label_new ("Label stop width");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Menu = gtk_menu_new ();
  MenuItem = gtk_menu_item_new_with_label ("Top");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "tab_pos_1");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  MenuItem = gtk_menu_item_new_with_label ("Bottom");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "tab_pos_2");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  MenuItem = gtk_menu_item_new_with_label ("Left");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "tab_pos_3");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  MenuItem = gtk_menu_item_new_with_label ("Right");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "tab_pos_4");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  Button = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU(Button), Menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU(Button),
			       set_temp.tab_position - 1);
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 0, 1);
  Size = gtk_adjustment_new (set_temp.max_tab_label_length, 0, 99, 1, 5, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 2);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "tab_len");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 1, 2);

  Frame = gtk_frame_new ("Msg Bar");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  Table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Button = gtk_check_button_new_with_label ("Display Msg Bar");
  if (set_temp.msgbar_display)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "msg_bar");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 0, 1);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 1, 2);
  Label = gtk_label_new ("Message display length (ms)");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.msgbar_interval, 0, 9999, 1, 10, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "msg_len");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 1, 2);

  Frame = gtk_frame_new ("Miscellaneous");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  Table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Button = gtk_check_button_new_with_label ("Enable Beep");
  if (set_temp.beep)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "beep");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 0, 1);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 1, 2);
  Label = gtk_label_new ("Print Command :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(Entry), set_temp.print_cmd);
  gtk_signal_connect (GTK_OBJECT(Entry), "changed",
		      (GtkSignalFunc)refresh_prefs, "print_cmd");
  gtk_table_attach_defaults (GTK_TABLE(Table), Entry, 1, 2, 1, 2);

  Label = gtk_label_new ("General");
  gtk_notebook_append_page (Notebook, VBox, Label);
}


void display_document (GtkNotebook *Notebook)
{
  GtkWidget *VBox, *FrameHBox;
  GtkWidget *Frame;
  GtkWidget *Table;
  GtkWidget *Label;
  GtkWidget *Button;
  t_widgint *ColBut;
  GtkWidget *Menu;
  GtkWidget *MenuItem;
  GtkWidget *Entry;
  gchar *Text;

  VBox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(VBox), 10);

  Frame = gtk_frame_new ("Global preferences");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  Table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Button = gtk_check_button_new_with_label ("Wordwrap");
  if (set_temp.toggle_wordwrap)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "wordwrap");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 0, 1); 
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 1, 2);
  Label = gtk_label_new ("Scroll Bar position");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Menu = gtk_menu_new ();
  MenuItem = gtk_menu_item_new_with_label ("Left");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "sb_pos_1");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  MenuItem = gtk_menu_item_new_with_label ("Right");
  gtk_signal_connect (GTK_OBJECT(MenuItem), "activate",
		      (GtkSignalFunc)refresh_prefs, "sb_pos_2");
  gtk_menu_append (GTK_MENU(Menu), MenuItem);
  Button = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU(Button), Menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU(Button),
			       set_temp.scrollbar_position - 1);
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 1, 2);

  Frame = gtk_frame_new ("Colors and Font");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  Table = gtk_table_new (5, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Label = gtk_label_new ("Font :");
  gtk_table_attach_defaults (GTK_TABLE(Table), Label, 0, 1, 0, 1);
  Button = gtk_button_new_with_label ("");
  apply_font_button (Button);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc)refresh_prefs, "font");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 3, 0, 1);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(FrameHBox), 10);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 2, 3, 1, 5);
  Entry = gtk_text_new (NULL, NULL);
  gtk_widget_set_usize (Entry, 150, 140);
  gtk_signal_connect (GTK_OBJECT(Entry), "event",
		      (GtkSignalFunc)text_preview,
		      NULL);
  gtk_text_set_word_wrap (GTK_TEXT(Entry), TRUE);
  Text = g_strdup ("This is some sample text. Note that you may need to click "
		   "on it to refresh display after color or font changes.");
  text_preview (Entry);
  gtk_text_insert (GTK_TEXT(Entry), NULL, NULL, NULL, Text, strlen(Text));
  g_free (Text);
  gtk_box_pack_start (GTK_BOX(FrameHBox), Entry, TRUE, TRUE, 0);
  ColBut = g_malloc (sizeof(t_widgint));
  ColBut -> widget = gtk_button_new ();
  ColBut -> op = 1;
  gtk_widget_set_usize (ColBut -> widget, 40, 20);
  gtk_container_set_border_width (GTK_CONTAINER(ColBut -> widget), 5);
  Entry = gtk_text_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER(ColBut -> widget), Entry);
  gtk_signal_connect (GTK_OBJECT(ColBut -> widget), "clicked",
		      (GtkSignalFunc)refresh_prefs, "color1");
  apply_color_button (NULL, ColBut);
  gtk_table_attach_defaults (GTK_TABLE(Table), ColBut -> widget, 0, 1, 1, 2);
  ColBut -> widget = gtk_button_new ();
  ColBut -> op = 2;
  gtk_widget_set_usize (ColBut -> widget, 40, 20);
  gtk_container_set_border_width (GTK_CONTAINER(ColBut -> widget), 5);
  Entry = gtk_text_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER(ColBut -> widget), Entry);
  gtk_signal_connect (GTK_OBJECT(ColBut -> widget), "clicked",
		      (GtkSignalFunc)refresh_prefs, "color2");
  apply_color_button (NULL, ColBut);
  gtk_table_attach_defaults (GTK_TABLE(Table), ColBut -> widget, 0, 1, 2, 3);
  ColBut -> widget = gtk_button_new ();
  ColBut -> op = 3;
  gtk_widget_set_usize (ColBut -> widget, 40, 20);
  gtk_container_set_border_width (GTK_CONTAINER(ColBut -> widget), 5);
  Entry = gtk_text_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER(ColBut -> widget), Entry);
  gtk_signal_connect (GTK_OBJECT(ColBut -> widget), "clicked",
		      (GtkSignalFunc)refresh_prefs, "color3");
  apply_color_button (NULL, ColBut);
  gtk_table_attach_defaults (GTK_TABLE(Table), ColBut -> widget, 0, 1, 3, 4);
  ColBut -> widget = gtk_button_new ();
  ColBut -> op = 4;
  gtk_widget_set_usize (ColBut -> widget, 40, 20);
  gtk_container_set_border_width (GTK_CONTAINER(ColBut -> widget), 5);
  Entry = gtk_text_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER(ColBut -> widget), Entry);
  gtk_signal_connect (GTK_OBJECT(ColBut -> widget), "clicked",
		      (GtkSignalFunc)refresh_prefs, "color4");
  apply_color_button (NULL, ColBut);
  gtk_table_attach_defaults (GTK_TABLE(Table), ColBut -> widget, 0, 1, 4, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 1, 2, 1, 2);
  Label = gtk_label_new ("Text Foreground");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 1, 2, 2, 3);
  Label = gtk_label_new ("Text Background");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 1, 2, 3, 4);
  Label = gtk_label_new ("Highlighted Foreground");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 1, 2, 4, 5);
  Label = gtk_label_new ("Highlighted Background");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);

  Label = gtk_label_new ("Document");
  gtk_notebook_append_page (Notebook, VBox, Label);
}


void display_save (GtkNotebook *Notebook)
{
  GtkWidget *VBox, *FrameHBox;
  GtkWidget *Frame;
  GtkWidget *Table;
  GtkWidget *Label;
  GtkWidget *Button;
  GtkWidget *Entry;
  GtkObject *Size;

  VBox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(VBox), 10);

  Frame = gtk_frame_new ("Backup");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  Table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Button = gtk_check_button_new_with_label ("Write backup files");
  if (set_temp.backup)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "backup");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 0, 1);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 1, 2);
  Label = gtk_label_new ("Extension :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(Entry), set_temp.backup_ext);
  gtk_signal_connect (GTK_OBJECT(Entry), "changed",
		      (GtkSignalFunc)refresh_prefs, "backup_ext");
  gtk_table_attach_defaults (GTK_TABLE(Table), Entry, 1, 2, 1, 2);

  Frame = gtk_frame_new ("Autosave");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  Table = gtk_table_new (1, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 0, 1);
  Label = gtk_label_new ("Delay (s) (0 disables Autosave) :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.autosave_delay, 0, 9999, 1, 10, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "autosave");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 0, 1);
  Frame = gtk_frame_new ("Recent files menu");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  Table = gtk_table_new (1, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 0, 1);
  Label = gtk_label_new ("Maximum number of files :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.recent_files, 0, 50, 1, 10, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "recent_files");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 0, 1);

  Label = gtk_label_new ("Save");
  gtk_notebook_append_page (Notebook, VBox, Label);
}


void display_advanced (GtkNotebook *Notebook)
{
  GtkWidget *VBox, *FrameHBox;
  GtkWidget *Frame;
  GtkWidget *Table;
  GtkWidget *Label;
  GtkWidget *Button;
  GtkWidget *Entry;
  GtkObject *Size;

  VBox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(VBox), 10);

  Frame = gtk_frame_new ("Wordfile location");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 0);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(FrameHBox), 5);
  gtk_container_add (GTK_CONTAINER(Frame), FrameHBox);
  Label = gtk_label_new ("Path :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 0);
  Entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(Entry), set_temp.wordfile);
  gtk_signal_connect (GTK_OBJECT(Entry), "changed",
		      (GtkSignalFunc)refresh_prefs, "wordfile");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Entry, TRUE, TRUE, 5);
  Button = gtk_button_new_with_label (" Browse ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc)refresh_prefs, "word_browse");
  gtk_box_pack_end (GTK_BOX(FrameHBox), Button, FALSE, FALSE, 0);
  Frame = gtk_frame_new
    ("Synthax highlighting, auto indentation and correction");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  Table = gtk_table_new (2, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(Table), 5);
  gtk_table_set_row_spacings (GTK_TABLE(Table), 5);
  gtk_table_set_col_spacings (GTK_TABLE(Table), 5);
  gtk_container_add (GTK_CONTAINER(Frame), Table);
  Button = gtk_check_button_new_with_label ("Enable Auto indentation");
  if (set_temp.auto_indent)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "auto_indent");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 0, 1);
  Button = gtk_check_button_new_with_label ("Enable Auto correction");
  if (set_temp.auto_correct)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "toggled",
		      (GtkSignalFunc)refresh_prefs, "auto_correct");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 0, 1, 1, 2);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE(Table), FrameHBox, 0, 1, 2, 3);
  Label = gtk_label_new ("Highlighting depth (characters) :");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.syn_high_depth, 0, 99999, 1, 10, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 5);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "syn_high_depth");
  gtk_table_attach_defaults (GTK_TABLE(Table), Button, 1, 2, 2, 3);
  Frame = gtk_frame_new ("Completion Popup size");
  gtk_box_pack_start (GTK_BOX(VBox), Frame, FALSE, FALSE, 5);
  FrameHBox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(FrameHBox), 5);
  gtk_container_add (GTK_CONTAINER(Frame), FrameHBox);
  Label = gtk_label_new ("Width");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.complete_window_width, 10,
			     gdk_screen_width (), 1, 5, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_widget_set_usize (Button, 60, -1);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "comp_width");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Button, FALSE, FALSE, 0);
  Label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 40);
  Label = gtk_label_new ("Height");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Label, FALSE, FALSE, 5);
  Size = gtk_adjustment_new (set_temp.complete_window_height, 10,
			     gdk_screen_height (), 1, 5, 0);
  Button = gtk_spin_button_new (GTK_ADJUSTMENT(Size), 1, 0);
  gtk_widget_set_usize (Button, 60, -1); 
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(Button), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY(Button), 4);
  gtk_signal_connect (GTK_OBJECT(Button), "changed",
		      (GtkSignalFunc)refresh_prefs, "comp_height");
  gtk_box_pack_start (GTK_BOX(FrameHBox), Button, FALSE, FALSE, 0);

  Label = gtk_label_new ("Advanced");
  gtk_notebook_append_page (Notebook, VBox, Label);
}


void display_prefs (t_settings *set)
{
  GtkWidget *PrefsWindow;
  GtkWidget *PrefsNotebook;
  GtkWidget *Button;
  
  if (PreferencesAreVisible) return;
  set_temp = *set;
  PrefsWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(PrefsWindow), "Preferences"); 
  gtk_window_set_policy (GTK_WINDOW(PrefsWindow), FALSE, FALSE, FALSE);
  gtk_signal_connect (GTK_OBJECT(PrefsWindow), "delete_event",
		      (GtkSignalFunc) preferences_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(PrefsWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(PrefsWindow));
  gtk_signal_connect (GTK_OBJECT (PrefsWindow), "destroy",
		      (GtkSignalFunc) preferences_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT (PrefsWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(PrefsWindow));
  PrefsNotebook = gtk_notebook_new ();
  gtk_notebook_set_homogeneous_tabs (GTK_NOTEBOOK(PrefsNotebook), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER(PrefsNotebook), 10);
  /* WORK IN PROGRESS... */
  display_general (GTK_NOTEBOOK(PrefsNotebook));
  display_document (GTK_NOTEBOOK(PrefsNotebook));
  display_advanced (GTK_NOTEBOOK(PrefsNotebook));
  display_save (GTK_NOTEBOOK(PrefsNotebook));
  /* WORK IN PROGRESS... */
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(PrefsWindow) -> vbox),
		      PrefsNotebook, TRUE, TRUE, 0);
  Button = gtk_button_new_with_label (" OK ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) apply_changes, set);
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		       (GtkSignalFunc) preferences_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(PrefsWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(PrefsWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  Button = gtk_button_new_with_label (" Apply ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) apply_changes, set);
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(PrefsWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  Button = gtk_button_new_with_label (" Cancel ");
  gtk_signal_connect (GTK_OBJECT(Button), "clicked",
		      (GtkSignalFunc) preferences_window_not_visible, NULL);
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(PrefsWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(PrefsWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_show_all (PrefsWindow);
  print_msg ("Display Preferences window...");
  PreferencesAreVisible = TRUE;
}
