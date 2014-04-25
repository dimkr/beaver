/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** msgbar.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:27:37 2000
** Description:   Beaver message bar manipulation source
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
#include "conf.h"
#include "editor.h"
#include "struct.h"
#include "tools.h"
#include "prefs.h"
#include "msgbar.h"

extern GtkWidget *MainNotebook;
extern GArray *FileProperties;
extern t_settings Settings;
extern gint OpenedFilesCnt;
static GtkWidget *MsgBar;
static GtkWidget *MsgLabel;
static GtkWidget *LCLabel;
static gboolean MessagesCnt = 0;
static gchar *LastMessage;


/**************************** PUBLIC FUNCTIONS *******************************/


void init_msgbar (GtkBox *ParentBox)
{
  GtkWidget *MsgBox; 
  GtkWidget *ViewPort;

  MsgBar = gtk_hbox_new (FALSE, 0);
  ViewPort = gtk_viewport_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX(MsgBar), ViewPort, TRUE, TRUE, 0);
  MsgBox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER(ViewPort), MsgBox);
  MsgLabel = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(MsgBox), MsgLabel, FALSE, FALSE, 0);
  LCLabel = gtk_label_new ("");
  gtk_box_pack_end (GTK_BOX(MsgBox), LCLabel, FALSE, FALSE, 5);
  gtk_box_pack_end (ParentBox, MsgBar, FALSE, FALSE, 0);
}


void hide_msgbar (void)
{
  gtk_widget_hide (MsgBar);
}


void show_msgbar (void)
{
  gtk_widget_show (MsgBar);
}


void clear_msgbar (void)
{
  if (!MSGBAR_DISPLAY) return;
  LastMessage = g_strdup("");
  gtk_label_set_text (GTK_LABEL(MsgLabel), "");
}


void print_msg (gchar *Message)
{
  gint *TimeOutId;

  if (!MSGBAR_DISPLAY) return;
  TimeOutId = g_malloc(sizeof (gint));
  LastMessage = g_strdup (Message);
  gtk_label_set_text (GTK_LABEL(MsgLabel), g_strconcat ("  ", Message, NULL));
  *TimeOutId = gtk_timeout_add (MSGBAR_INTERVAL, (GtkFunction)end_msg,
				TimeOutId);
  MessagesCnt++;
}


gint get_line (void)
{
  GtkWidget *Editor;
  gint line;
  guint i;

  Editor = FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
		  Text);
  line = 1;
  for (i = 0; i < GTK_TEXT(Editor)->editable.current_pos; i++)
    if (GTK_TEXT_INDEX(GTK_TEXT(Editor), i) == '\n')
      line++;
  return line;
}


gint get_column (void)
{
  GtkWidget *Editor;
  gint i;

  Editor = FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
		  Text);
  i = GTK_TEXT(Editor)->editable.current_pos - 1;
  while ((i >= 0) && (GTK_TEXT_INDEX(GTK_TEXT(Editor), (guint)i) != '\n'))
    i--;
  return (gint)GTK_TEXT(Editor)->editable.current_pos - i;
}


gint get_percent (void)
{
  GtkWidget *Editor;
  gint i, j;

  Editor = FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
		  Text);
  i = 100 * GTK_TEXT(Editor)->editable.current_pos;
  if ((j = gtk_text_get_length(GTK_TEXT(Editor))) == 0)
    return 0;
  else
    return i/j;
}


gint get_selection_size (void)
{
  gint CurrentPage;

  CurrentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook));
  return max_from_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text))) -
    min_from_selection (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
}


gboolean display_line_column (void)
{
  gchar *line_column;
  gint sel_size;
  
  if (!MSGBAR_DISPLAY) return TRUE;
  if (!OpenedFilesCnt)
    {
      gtk_label_set_text (GTK_LABEL(LCLabel), "");
      return TRUE;
    }
  while (gtk_events_pending ())
    gtk_main_iteration ();
  if ((sel_size = get_selection_size ()))
    line_column = g_strdup_printf ("S %03d   L %03d   C %03d   %02d%%",
				   sel_size, get_line (), get_column (),
				   get_percent ());
  else
    line_column = g_strdup_printf ("L %03d   C %03d   %02d%%",
				   get_line (), get_column (), get_percent ());
  gtk_label_set_text (GTK_LABEL(LCLabel), line_column);
  g_free (line_column);
  return TRUE;
}


/************************ AUTHORIZED PERSONAL ONLY ***************************/


void end_msg (gint *TimeOutId)
{
  if (MessagesCnt <= 1) gtk_label_set_text (GTK_LABEL(MsgLabel), "");
  else gtk_label_set_text (GTK_LABEL(MsgLabel),
			   g_strconcat ("  ", LastMessage, NULL));
  gtk_timeout_remove (*TimeOutId);
  MessagesCnt--;
}
