/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** completion.c
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
**              Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Last update: 2000-05-04
** Description: Do auto-completion stuff
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
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "struct.h"
#include "msgbar.h"
#include "prefs.h"
#include "completion.h"


extern GtkWidget	*MainNotebook;
extern GArray		*FileProperties;
extern t_settings	Settings;


/*
** This fcn proceed auto-completion, it is called in interface.c by
** a ItemFactory method... (standart accelerator: <Alt>space)
**
** Parameters :
**  Editor		The GtkText where completion is needed
**
** Return values :
## -2                   The Language has no delimiters
** -1			Language not recognized
** 0			OK
*/

gint			auto_completion(GtkText *Editor)
{
  GtkWidget		*CompletionWindow;
  GtkWidget		*scrolled_window;
  GtkWidget		*gtklist;
  GList			*dlist = NULL;
  GtkWidget		*gtklistitem;
  GdkWindow		*beaver_gdkwin;
  gint			beaver_x;
  gint			beaver_y;
  gint			*coord;
  static t_auco_datas	datas;
  gchar			*bok; /* Beginning Of Keyword wrote in Editor */
  gint			ibok; /* Index of beginning of bok in Editor */
  gint			ebok; /* End of bok in Editor */
  gchar			*keyword;
  gint			Lg;
  gint			col;

  Lg = FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
	      WidgetInfo.Lg);
  if (Lg == -1)
    {
      print_msg ("Cannot complete: language not recognized...");
      return -1;
    }
  ebok = gtk_editable_get_position(GTK_EDITABLE(Editor));
  ibok = ebok - 1;
  if (!Prefs.L[Lg].Delimiters)
    return -2;
  if (Prefs.L[Lg].IsHTML)
    while (ibok > 0 &&
	   (GTK_TEXT_INDEX(Editor, (guint)ibok) == '<' ||
	    GTK_TEXT_INDEX(Editor, (guint)ibok) == '/' ||
	    !strchr(Prefs.L[Lg].Delimiters,
		    GTK_TEXT_INDEX(Editor, (guint)ibok))))
      ibok--;
  else
    while (ibok > 0 &&
	   !strchr(Prefs.L[Lg].Delimiters,
		   GTK_TEXT_INDEX(Editor, (guint)ibok)))
      ibok--;
  if (strchr(Prefs.L[Lg].Delimiters, GTK_TEXT_INDEX(Editor, (guint)ibok)))
    ibok++;
  bok = gtk_editable_get_chars(GTK_EDITABLE(Editor), (guint)ibok, ebok);
  if (bok)
    {
      for (col = 0; col < MAX_COL; col++)
	if (Prefs.L[Lg].IsDefined && (keyword = Prefs.L[Lg].C[col].Keywords))
	  {
	    keyword++;
	    while (*keyword)
	      {
		gchar	*eok; /* End Of Keyword */
		
		eok = strchr(keyword, ' ');
		*eok = 0;
		if (Prefs.L[Lg].IsCaseSensitive)
		  {
		    if (!strncmp(bok, keyword, ebok - ibok))
		      {
			gtklistitem = gtk_list_item_new_with_label(keyword);
			gtk_signal_connect (GTK_OBJECT(gtklistitem),
					    "button_press_event",
					    GTK_SIGNAL_FUNC
					    (auto_completion_double_clic),
					    &datas);
			dlist = g_list_append(dlist, gtklistitem);
		      }
		  }
		else
		  {
		    if (!strncasecmp(bok, keyword, ebok - ibok))
		      {
			gtklistitem = gtk_list_item_new_with_label(keyword);
			gtk_signal_connect (GTK_OBJECT(gtklistitem),
					    "button_press_event",
					    GTK_SIGNAL_FUNC
					    (auto_completion_double_clic),
					    &datas);
			dlist = g_list_append(dlist, gtklistitem);
		      }
		  }
		*eok = ' ';
		keyword = eok + 1;
	      }
	  }
      g_free(bok);
      dlist = g_list_sort(dlist, (GCompareFunc)compare_listitem);
    }
  if (!bok || (dlist == NULL))
    {
      print_msg ("No matching string...");
      return 0;
    }
  if (!dlist->next)
    {
      gint		current_pos;
      gchar		*label, *msg;

      /*
      ** Only 1 keyword, insert it directly
      */
      current_pos = gtk_editable_get_position(GTK_EDITABLE(Editor));
      gtk_label_get(GTK_LABEL(GTK_BIN(dlist->data)->child), &label);
      gtk_editable_delete_text(GTK_EDITABLE(Editor), ibok, current_pos);
      gtk_editable_insert_text(GTK_EDITABLE(Editor),
			       label, strlen(label), &current_pos);
      msg = g_strconcat ("String \"", label, "\" inserted...", NULL);
      print_msg (msg);
      g_free (msg);
      return 0;
    }
  /*
  ** GtkList contains 2 or more elements. So, create a window, which will
  ** contain auto-completion words
  */
  CompletionWindow = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_title (GTK_WINDOW(CompletionWindow), "Complete");
  gtk_window_set_policy (GTK_WINDOW(CompletionWindow), FALSE, FALSE, FALSE);
  gtk_window_set_modal(GTK_WINDOW(CompletionWindow), TRUE);
  gtk_window_set_transient_for
    (GTK_WINDOW(CompletionWindow),
     GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(Editor))));
  gtk_widget_set_usize(CompletionWindow,
		       COMPLETE_WINDOW_WIDTH, COMPLETE_WINDOW_HEIGHT);
  beaver_gdkwin = gtk_widget_get_parent_window(GTK_WIDGET(Editor));
  gdk_window_get_origin(beaver_gdkwin, &beaver_x, &beaver_y);
  coord = g_malloc (2*sizeof(gint));
  coord[0] =
    beaver_x + GTK_WIDGET(GTK_WIDGET(MainNotebook)->parent)->allocation.x +
    GTK_WIDGET(Editor)->allocation.x + Editor->cursor_pos_x + 3;
  coord[1] =
    beaver_y + GTK_WIDGET(GTK_WIDGET(MainNotebook)->parent)->allocation.y +
    GTK_WIDGET(Editor)->allocation.y + Editor->cursor_pos_y + 4;
  if (coord[0] > gdk_screen_width() - COMPLETE_WINDOW_WIDTH)
    coord[0] -= COMPLETE_WINDOW_WIDTH;
  if (coord[1] > gdk_screen_height() - COMPLETE_WINDOW_HEIGHT)
    coord[1] -= COMPLETE_WINDOW_HEIGHT + 15;
  gtk_widget_set_uposition(CompletionWindow, coord[0], coord[1]);
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_window),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER(CompletionWindow), scrolled_window);
  gtklist = gtk_list_new();
  gtk_list_set_selection_mode(GTK_LIST(gtklist), GTK_SELECTION_BROWSE);
  gtk_scrolled_window_add_with_viewport
    (GTK_SCROLLED_WINDOW (scrolled_window), gtklist);
  gtk_container_set_focus_vadjustment
    (GTK_CONTAINER (gtklist),
     gtk_scrolled_window_get_vadjustment
     (GTK_SCROLLED_WINDOW (scrolled_window)));
  gtk_container_set_focus_hadjustment
    (GTK_CONTAINER (gtklist),
     gtk_scrolled_window_get_hadjustment
     (GTK_SCROLLED_WINDOW (scrolled_window)));
  gtk_list_append_items(GTK_LIST(gtklist), dlist);
  gtk_window_set_focus(GTK_WINDOW(CompletionWindow),
		       GTK_WIDGET(GTK_LIST(gtklist)->children->data));
  gtk_widget_show_all(CompletionWindow);
  /*
  ** Now, the window is displayed and we need to connect signals and
  ** do other stuff...
  */
  datas.ebok = ebok;
  datas.ibok = ibok;
  gtk_signal_connect(GTK_OBJECT(CompletionWindow), "key_press_event",
		     GTK_SIGNAL_FUNC(auto_completion_key_press), &datas);
  gtk_signal_connect(GTK_OBJECT(CompletionWindow), "button_press_event",
		     GTK_SIGNAL_FUNC(stop_completion), coord);
  gtk_signal_connect(GTK_OBJECT(CompletionWindow), "delete_event",
		     GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL);
  gtk_signal_connect(GTK_OBJECT(CompletionWindow), "destroy_event",
		     GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL);
  return 0;
}

/*
** This fcn is called when a a mouse button is pressed out of the completion
** window: it destroys the window...
**
** Parameters :
**  See a Gtk reference manual (callback function)
**
** Return values :
**  TRUE		Ok
*/
void			stop_completion(GtkWidget *widget,
					GdkEventButton *ev,
					gint *coord)
{
  if ((ev -> x_root < coord[0]) ||
      (ev -> y_root < coord[1]) ||
      (ev -> x_root > coord[0]+COMPLETE_WINDOW_WIDTH) ||
      (ev -> y_root > coord[1]+COMPLETE_WINDOW_HEIGHT))
    gtk_widget_destroy (gtk_widget_get_toplevel(widget));
}

/*
** This fcn is called when a a mouse button is pressed on a gtklistitem:
## if it is a double-clic, then the selected string is inserted...
**
** Parameters :
**  See a Gtk reference manual (callback function)
**
** Return values :
** void
*/
void			auto_completion_double_clic(GtkWidget *widget,
						    GdkEventButton *ev,
						    t_auco_datas *datas)
{
  if (ev -> type == GDK_2BUTTON_PRESS)
    {
      gchar		*label, *msg;
      gint			CurrentPage, current_pos;

      CurrentPage = gtk_notebook_get_current_page
	(GTK_NOTEBOOK(MainNotebook));
      current_pos = gtk_editable_get_position
	(GTK_EDITABLE(FPROPS(CurrentPage, Text)));
      gtk_label_get (GTK_LABEL(GTK_BIN(widget)->child), &label);
      if (current_pos > datas->ibok)
	gtk_editable_delete_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				  datas->ibok, current_pos);
      gtk_editable_insert_text(GTK_EDITABLE(FPROPS(CurrentPage, Text)),
			       label, strlen (label), &current_pos);
      msg = g_strconcat ("String \"", label, "\" inserted...", NULL);
      print_msg (msg);
      g_free (msg);
      gtk_widget_destroy(gtk_widget_get_toplevel(widget));
    }
}

/*
** This fcn is called when a key is pressed in the auto-completion window
**
** Parameters :
**  See a Gtk reference manual (callback function)
**
** Return values :
** void 
*/
void			auto_completion_key_press(GtkWidget *window,
						  GdkEventKey *event,
						  t_auco_datas *datas)
{
  gint			CurrentPage, current_pos;
  
  CurrentPage = gtk_notebook_get_current_page
    (GTK_NOTEBOOK(MainNotebook));
  current_pos = gtk_editable_get_position
    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
  if (event->string[0] == '\t' ||
      event->string[0] == '\r' ||
      event->string[0] == ' ')
    {
      gchar		*label, *msg;

      gtk_label_get (GTK_LABEL
		     (GTK_BIN
		      (GTK_LIST
		       (GTK_BIN
			(GTK_BIN
			 (GTK_BIN
			  (window)
			  ->child)
			 ->child)
			->child)
		       ->selection->data)
		      ->child), &label);
      if (current_pos > datas->ibok)
	gtk_editable_delete_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				  datas->ibok, current_pos);
      gtk_editable_insert_text(GTK_EDITABLE(FPROPS(CurrentPage, Text)),
			       label, strlen (label), &current_pos);
      msg = g_strconcat ("String \"", label, "\" inserted...", NULL);
      print_msg (msg);
      g_free (msg);
      gtk_widget_destroy(window);
    }
  else if (event->string[0] == '\b')
    {
      if (current_pos > datas->ebok)
	{
	  gchar *word;
	  
	  gtk_editable_delete_text (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				    current_pos-1, current_pos);
	  word = gtk_editable_get_chars
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), datas->ibok,
	     gtk_editable_get_position
	     (GTK_EDITABLE(FPROPS(CurrentPage, Text))));
	  refresh_selection (GTK_LIST
			     (GTK_BIN
			      (GTK_BIN
			       (GTK_BIN
				(window)
				->child)
			       ->child)
			      ->child), word);
	  g_free (word);
	}
      else
	{
	  if (BEEP) gdk_beep();
	  print_msg ("Beginning of auto-completion buffer...");
	}
    }
  else if (event->string[0] > ' ' && event->string[0] <= '~')
    {
      gchar *word;
      gint len;

      word = gtk_editable_get_chars (GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				     datas->ibok, current_pos);
      len = strlen(word);
      word = g_realloc (word, len+2);
      word[len] = event->string[0];
      word[len+1] = 0; 
      if (refresh_selection (GTK_LIST
			     (GTK_BIN
			      (GTK_BIN
			       (GTK_BIN
				(window)
				->child)
			       ->child)
			      ->child), word))
	gtk_editable_insert_text(GTK_EDITABLE(FPROPS(CurrentPage, Text)),
				 event->string, 1, &current_pos);
      else
	{
	  if (BEEP) gdk_beep();
	  print_msg ("No matching string...");
	}
      g_free (word);
    }
  else if (event->keyval != 0xffe1 &&   /* Shift Left */
	   event->keyval != 0xffe2 &&   /* Shift Right */
	   event->keyval != 0xff7e &&   /* Alt Gr */
	   event->keyval != 0xff52 &&   /* Up      */
	   event->keyval != 0xff97 &&   /* KP_Up   */		     
	   event->keyval != 0xff54 &&   /* Down    */
	   event->keyval != 0xff99 &&   /* KP_Down */
	   event->keyval != 0xff51 &&   /* Left    */
	   event->keyval != 0xff96 &&   /* KP_Left */
	   event->keyval != 0xff53 &&   /* Right   */
	   event->keyval != 0xff98 &&   /* KP_Right*/
	   event->keyval != 0xff55 &&   /* Prior */
	   event->keyval != 0xff56 &&   /* Next */
	   event->keyval != 0xff9a &&   /* KP_Prior */
	   event->keyval != 0xff9b &&   /* KP_Next */
	   event->keyval != 0xff50 &&   /* Home */
	   event->keyval != 0xff57 &&   /* End */
	   event->keyval != 0xff95 &&   /* KP_Home */
	   event->keyval != 0xff9c &&   /* KP_End */
	   event->keyval != 0xffe5)     /* Caps Lock */
    gtk_widget_destroy(window);
}

/*
** Fnc that compare 2 GtkListItem (with labels) for sorting the GList
*/
gint			compare_listitem(gconstpointer i1, gconstpointer i2)
{
  gchar			*label1, *label2;

  gtk_label_get(GTK_LABEL(GTK_BIN(i1) -> child), &label1);
  gtk_label_get(GTK_LABEL(GTK_BIN(i2) -> child), &label2);
  return strcmp(label1, label2);
}

/*
** Update selection in gtklist
*/
gboolean		refresh_selection(GtkList *gtklist, gchar *word)
{
  GList			*item;
  gchar			*label;
  gint			Lg;

  Lg = FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
	      WidgetInfo.Lg);
  item = gtklist->children;
  if (Prefs.L[Lg].IsCaseSensitive)
    while (item &&
	   (gtk_label_get(GTK_LABEL(GTK_BIN(item->data)->child), &label),
	    strncmp(word, label, strlen(word))))
      item = item->next;
  else
    while (item &&
	   (gtk_label_get(GTK_LABEL(GTK_BIN(item->data)->child), &label),
	    strncasecmp(word, label, strlen(word))))
      item = item->next;
  if (item)
    {
      gint len;

      gtk_list_select_child(gtklist, item->data);
      len = g_list_length(gtklist->children);
      if (len > COMPLETE_WINDOW_HEIGHT/15)
	{
	  gfloat		position;

	  position = g_list_index(gtklist->children, (gpointer)item->data);
	  position /= len - 1;
	  gtk_list_scroll_vertical(gtklist, GTK_SCROLL_JUMP, position);
	}
      return TRUE;
    }
  else
    return FALSE;
}
