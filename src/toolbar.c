/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** toolbar.c
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:28:06 2000
** Description:   Beaver toolbar source
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
#include "filesops.h"
#include "interface.h"


/* Icons for the toolbar buttons */

#include "../pixmaps/stock_new.xpm"
#include "../pixmaps/stock_open.xpm"
#include "../pixmaps/stock_close.xpm"
#include "../pixmaps/stock_save.xpm"
#include "../pixmaps/stock_print.xpm"
#include "../pixmaps/stock_undo.xpm"
#include "../pixmaps/stock_redo.xpm"
#include "../pixmaps/stock_cut.xpm"
#include "../pixmaps/stock_copy.xpm"
#include "../pixmaps/stock_paste.xpm"
#include "../pixmaps/stock_find.xpm"
#include "../pixmaps/stock_replace.xpm"
#include "../pixmaps/stock_line.xpm"
#include "../pixmaps/stock_about.xpm"
#include "../pixmaps/stock_help.xpm"
#include "../pixmaps/stock_exit.xpm"


static GtkWidget *HandleBox;


/* This toolbar is one of the things that should evolve in a near future */

void init_toolbar (GtkBox *ParentBox)
{
  GtkWidget *ParentWindow;
  GtkWidget *ToolBar;
  GtkWidget *ButtonWithPixmap;
  GdkPixmap *Pixmap;
  GdkBitmap *Mask;
  GtkStyle  *Style;

  ParentWindow = gtk_widget_get_toplevel (GTK_WIDGET(ParentBox));
  gtk_widget_realize (ParentWindow);
  Style = gtk_widget_get_style (ParentWindow);
  HandleBox = gtk_handle_box_new();
  gtk_container_set_border_width (GTK_CONTAINER(HandleBox), 2);
  gtk_box_pack_start (GTK_BOX(ParentBox), HandleBox, FALSE, FALSE, 0);
  ToolBar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR(ToolBar), GTK_RELIEF_NONE);
  gtk_toolbar_set_space_size (GTK_TOOLBAR(ToolBar), 15);
  
  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_new_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Create a new Document",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)NEW);
  
  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_open_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Open a File",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)OPEN);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_close_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Close the current File",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)CLOSE);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_save_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Save the current File",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)SAVE);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_print_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Print the current File",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)PRINT);

  gtk_toolbar_append_space (GTK_TOOLBAR(ToolBar));

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_undo_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Undo last Operation",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)UNDO);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_redo_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Redo last Operation",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)REDO);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_cut_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Cut the Selection",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)CUT);
  
  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_copy_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Copy the Selection",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)COPY);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_paste_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Paste the Clipboard",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)PASTE);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_find_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Search for a String",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)FIND);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_replace_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Search and Replace",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)REPLACE);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_line_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Goto Line...",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)LINE);

  gtk_toolbar_append_space (GTK_TOOLBAR(ToolBar));

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_help_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "User Manual",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)HELP);

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_about_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "About Beaver",
			   NULL, ButtonWithPixmap, menu_items_treatment,
			   (void *)ABOUT);

  gtk_toolbar_append_space (GTK_TOOLBAR(ToolBar));

  Pixmap = gdk_pixmap_create_from_xpm_d (ParentWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 stock_exit_xpm);
  ButtonWithPixmap = gtk_pixmap_new (Pixmap, Mask);
  gtk_toolbar_append_item (GTK_TOOLBAR(ToolBar), NULL,
			   "Exit the Program",
			   NULL, ButtonWithPixmap, quit, NULL);
 
  gtk_container_add (GTK_CONTAINER(HandleBox), ToolBar);
}


void hide_toolbar (void)
{
  gtk_widget_hide (HandleBox);
}


void show_toolbar (void)
{
  gtk_widget_show (HandleBox);
}
