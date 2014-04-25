/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** undoredo.c
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
** Last update: 2000-05-01
** Description: Provide Undo/Redo function
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "editor.h"
#include "struct.h"
#include "msgbar.h"
#include "prefs.h"
#include "undoredo.h"


extern GtkWidget	*MainNotebook;
extern GArray		*FileProperties;
extern t_settings	Settings;


void			init_undoredo(void)
{
  GList			**current_action;

  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  *current_action = NULL;
  *current_action = g_list_append(*current_action, NULL);
  undoredo_activated = TRUE;
}

void			record_action(t_action *action)
{
  GList			**current_action;
  GList			*first_action;
  gint			position;
  gint			last_action_index;
  gint			i;

  if (!undoredo_activated)
    return;
  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  first_action = g_list_first(*current_action);
  position = g_list_position(first_action, *current_action);
  last_action_index = (gint)g_list_length(first_action) - 1;
  /*
  ** If the user has just undone some actions, these can be redone
  ** (they are saved in a list). But if he has manually done another
  ** action (and this is the case: the current function has been
  ** called) we must free the saved actions, because they are lost
  ** forever. This can seem complicated, but it is exactly the same
  ** behaviour that the 'next' and 'back' button in any browser.
  ** -- After
  */
  for (i = last_action_index; i > position; i--)
    first_action =
      g_list_remove(first_action, g_list_nth_data(first_action, i));
  *current_action =
    g_list_last(first_action = g_list_append(first_action, action));
}

gboolean		undo_is_possible(void)
{
  GList			**current_action;

  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  if ((*current_action)->data)
      return TRUE;
  else
      return FALSE;
}

gboolean		redo_is_possible(void)
{
  GList			**current_action;

  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  if ((*current_action)->next)
      return TRUE;
  else
      return FALSE;
}

void			proceed_redo(void)
{
  GList			**current_action;
  GtkWidget		*Editor;

  if (!undoredo_activated)
    return;
  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  Editor = FPROPS (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
		   Text);
  if (redo_is_possible())
    {
      *current_action = (*current_action)->next;
      switch (((t_action *)(*current_action)->data)->type)
	{
	case insert:
	  {
	    gint	position;

	    position = ((t_action *)(*current_action)->data)->start;
	    /* The following line of code is needed because of an
	       obscur bug, probably in editor.c. For further
	       informations, please see comments in
	       text_has_been_inserted() */
	    gtk_editable_set_position(GTK_EDITABLE(Editor), position);
	    undoredo_activated = FALSE;
	    gtk_editable_insert_text
	      (GTK_EDITABLE(Editor),
	       ((t_action *)(*current_action)->data)->text,
	       ((t_action *)(*current_action)->data)->end -
	       ((t_action *)(*current_action)->data)->start,
	       &position);
	    undoredo_activated = TRUE;
	    break;
	  }
	case delete:
	  {
	    undoredo_activated = FALSE;
	    gtk_editable_delete_text(GTK_EDITABLE(Editor),
				     ((t_action *)(*current_action)->data)
				     ->start,
				     ((t_action *)(*current_action)->data)
				     ->end);
	    undoredo_activated = TRUE;
	    break;
	  }
	}
      print_msg("Redo...");
    }
  else
    {
      if (BEEP) gdk_beep();
      print_msg("Nothing to redo !");
    }
}

void			proceed_undo(void)
{
  GList			**current_action;
  GtkWidget		*Editor;

  if (!undoredo_activated)
    return;
  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  Editor = FPROPS (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
		   Text);
  if (undo_is_possible())
    {
      switch (((t_action *)(*current_action)->data)->type)
	{
	case insert:
	  {
	    undoredo_activated = FALSE;
	    gtk_editable_delete_text(GTK_EDITABLE(Editor),
				     ((t_action *)(*current_action)->data)
				     ->start,
				     ((t_action *)(*current_action)->data)
				     ->end);
	    undoredo_activated = TRUE;;
	    break;
	  }
	case delete:
	  {
	    gint	position;

	    position = ((t_action *)(*current_action)->data)->start;
	    /* The following line of code is needed because of an
	       obscur bug, probably in editor.c. For further
	       informations, please see comments in
	       text_has_been_inserted() */
	    gtk_editable_set_position(GTK_EDITABLE(Editor), position);
	    undoredo_activated = FALSE;
	    gtk_editable_insert_text
	      (GTK_EDITABLE(Editor),
	       ((t_action *)(*current_action)->data)->text,
	       ((t_action *)(*current_action)->data)->end -
	       ((t_action *)(*current_action)->data)->start,
	       &position);
	    undoredo_activated = TRUE;
	    break;
	  }
	}
      *current_action = (*current_action)->prev;
      print_msg("Undo...");
    }
  else
    {
      if (BEEP) gdk_beep();
      print_msg("Nothing to undo !");
    }
}

/*
** This function is only for debugging purpose.
*/
void			view_undoredo_list(gchar *prefix)
{
  GList			**current_action;
  GList			*action;

  current_action = &FPROPS
    (gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
     WidgetInfo.current_action);
  action = g_list_first(*current_action);
  g_print("%s: **** Start of undoredo list\n", prefix);
  while (action)
    {
      if (action->data)
	{
	  g_print("%s: %s in [%i; %i] of \"%s\"",
		  prefix,
		  ((t_action *)action->data)->type == insert ?
		  "Insertion" : "Deletion",
		  ((t_action *)action->data)->start,
		  ((t_action *)action->data)->end,
		  ((t_action *)action->data)->text);
	}
      else
	g_print("%s: Data == NULL: The 1st null `action\'", prefix);
      if (action == *current_action)
	g_print(" <== This action is the previous\n");
      else
	g_print("\n");
      action = action->next;
    }
  g_print("%s: **** End of undoredo list\n", prefix);
}
