/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** interface.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Thu Nov  9 00:35:08 2000
** Description:   Beaver interface source header
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

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

/* Menu entries for the main menubar and the toolbar */

enum {
  NEW,
  OPEN,
  SAVE,
  SAVE_ALL,
  SAVE_AS,
  PRINT,
  CLOSE,
  CLOSE_ALL,
  QUIT,
  UNDO,
  REDO,
  CUT,
  COPY,
  PASTE,
  CLEAR,
  SELECT_ALL,
  COMPLETE,
  FIND,
  REPLACE,
  LINE,
  READONLY,
  CONVERTER,
  COLOR,
  INSERT_TIME,
  TO_UPPER,
  TO_LOWER,
  CAPITALIZE,
  INVERT_CASE,
  DOS_UNIX,
  UNIX_DOS,
  DOS_MAC,
  MAC_DOS,
  MAC_UNIX,
  UNIX_MAC,
  DOS_UNIX_ALL,
  UNIX_DOS_ALL,
  DOS_MAC_ALL,
  MAC_DOS_ALL,
  MAC_UNIX_ALL,
  UNIX_MAC_ALL,
  FILE_INFO,
  TOOLBAR,
  MSGBAR,
  WORDWRAP,
  TAB_POS_TOP,
  TAB_POS_BOTTOM,
  TAB_POS_LEFT,
  TAB_POS_RIGHT,
  SCROLLBAR_POS_LEFT,
  SCROLLBAR_POS_RIGHT,
  PREFS,
  LANG,
  HELP,
  ABOUT
};

void menu_items_treatment (GtkWidget *Widget,
			   guint Op);
GtkWidget *menubar_new (GtkWidget *Window);
GtkWidget *popup_menu_new (void);
gboolean popup_menu_show (GtkMenu *PopUpMenu,
			  GdkEvent *Event);
void add_page_in_notebook (GtkNotebook *Notebook,
			   gchar *Filename);
void  set_label (GtkNotebook *Notebook,
		 gint CurrentPage);
void set_title (GtkNotebook *Notebook,
		GtkNotebookPage *Page,
		gint CurrentPage);
void command_line (gint argc,
		   gchar *argv[]);
void interface (gint argc,
		gchar *argv[]);
void about (void);

#endif
