/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** interface.c
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed Jan 10 20:31:17 2001
** Description:   Beaver interface source
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
#include <time.h>
#include "main.h"
#include "editor.h"
#include "struct.h"
#include "filesops.h"
#include "msgbar.h"
#include "toolbar.h"
#include "conf.h"
#include "prefs.h"
#include "languages.h"
#include "tools.h"
#include "search.h"
#include "completion.h"
#include "undoredo.h"
#include "interface.h"


/* The 'about' pixmap */

#include "../pixmaps/about.xpm"


/* Thess variables HAVE to be global ones: I know this is bad, but... */
 
GtkWidget *MainWindow;
GtkWidget *MainNotebook;
t_settings Settings;
GArray *FileProperties;
gint NewFilesCnt = 0;
gint OpenedFilesCnt = 0;
static gboolean MsgBarToggleDisplay;
static gboolean ToolBarToggleDisplay;
static gboolean ToggleWordwrap;


/***************************** MENUS MANAGEMENT ******************************/


/* What to do when you chose a menu item ???
   The answer is there... */

void menu_items_treatment (GtkWidget *Widget, guint Op)
{
  gint CurrentPage, i;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  switch(Op)
    {
    case NEW                  : new_file ();                   break;
    case OPEN                 : open_file ();                  break;
    case SAVE                 : save_file ();                  break;
    case SAVE_ALL             : save_all ();                   break;
    case SAVE_AS              : save_file_as ();               break;
    case PRINT                : print_buffer ();               break;
    case CLOSE                : close_file ();                 break;
    case CLOSE_ALL            : close_all ();                  break;
    case QUIT                 : quit ();                       break;
    case UNDO                 :
      if (OpenedFilesCnt)
	{
	  proceed_undo ();
	  break;
	 } 
    case REDO                 :
      if (OpenedFilesCnt)
	{
	  proceed_redo ();
	  break;
	}
    case CUT                  :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  gtk_editable_cut_clipboard (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
	  print_msg ("Selection cut to Clipboard...");
	}
      break;
    case COPY                 :
      if (OpenedFilesCnt)
	{
	  gtk_editable_copy_clipboard
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
	  print_msg ("Selection copied to Clipboard...");
	}
	break;
    case PASTE                :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  gtk_editable_paste_clipboard
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)));
	  print_msg ("Text pasted from Clipboard...");
	}
      break;
    case CLEAR                :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  gtk_editable_delete_text
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
	  print_msg ("Buffer cleared...");
	}
      break;
    case SELECT_ALL           :
      if (OpenedFilesCnt)
	{
	  gtk_editable_select_region
	    (GTK_EDITABLE(FPROPS(CurrentPage, Text)), 0, -1);
	  print_msg ("All Text selected...");
	}
      break;
    case COMPLETE             :
      if (OpenedFilesCnt)
	auto_completion (GTK_TEXT(FPROPS(CurrentPage, Text)));
      break;
    case FIND                 :
      if (OpenedFilesCnt)
	search (FALSE);
      break;
    case REPLACE              :
      if (OpenedFilesCnt)
	search (TRUE);
      break;
    case LINE                 :
      if (OpenedFilesCnt)
	goto_line ();
      break;
    case READONLY             : toggle_readonly ();            break;
    case CONVERTER            : converter ();                  break;
    case COLOR                : color_picker ();               break;
    case INSERT_TIME          : insert_time (CurrentPage);     break;
    case TO_UPPER             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  change_case (1);
	  print_msg ("Selection converted to upper Case...");
	}
      break;
    case TO_LOWER             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  change_case (2);
	  print_msg ("Selection converted to Lower Case...");
	}
      break;
    case CAPITALIZE           :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  change_case (3);
	  print_msg ("Selection Capitalized...");
	}
      break; 
    case INVERT_CASE          :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  change_case (4);
	  print_msg ("Case inverted...");
	}
      break;
    case UNIX_DOS             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_unix_to_dos ();
	}
      break;
    case UNIX_MAC             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_unix_to_mac ();
	}
      break;
    case DOS_UNIX             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_dos_to_unix ();
	}
      break;
    case DOS_MAC              :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_dos_to_mac ();
	}
      break;
    case MAC_DOS              :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_mac_to_dos ();
	}
      break;
    case MAC_UNIX             :
      if ((OpenedFilesCnt) && !FPROPS(CurrentPage, ReadOnly))
	{
	  conv_mac_to_unix ();
	}
      break;
    case UNIX_DOS_ALL             :
      convert_all(conv_unix_to_dos);
      break;
    case UNIX_MAC_ALL             :
      convert_all(conv_unix_to_mac);
      break;
    case DOS_UNIX_ALL             :
      convert_all(conv_dos_to_unix);
      break;
    case DOS_MAC_ALL              :
      convert_all(conv_dos_to_mac);
      break;
    case MAC_DOS_ALL              :
      convert_all(conv_mac_to_dos);
      break;
    case MAC_UNIX_ALL             :
      convert_all(conv_mac_to_unix);
      break;
    case FILE_INFO            : file_info (CurrentPage);       break;
    case TOOLBAR              :
      if ((!ToolBarToggleDisplay) && (TOOLBAR_DISPLAY))
	ToolBarToggleDisplay = TRUE;
      else
	{
	  if (TOOLBAR_DISPLAY)
	    {
	      TOOLBAR_DISPLAY = FALSE;
	      hide_toolbar ();
	      print_msg ("Hide Tool Bar...");
	    }
	  else
	    {
	      TOOLBAR_DISPLAY = TRUE;
	      show_toolbar ();
	      if (!ToolBarToggleDisplay) ToolBarToggleDisplay = TRUE;
	      print_msg ("Display Tool Bar...");
	    }
	}
      break;
    case MSGBAR               :
      if ((!MsgBarToggleDisplay) && (MSGBAR_DISPLAY))
	MsgBarToggleDisplay = TRUE;
      else
	{
	  if (MSGBAR_DISPLAY)
	    {
	      MSGBAR_DISPLAY = FALSE;
	      hide_msgbar ();
	    }
	  else
	    {
	      MSGBAR_DISPLAY = TRUE;
	      show_msgbar ();
	      if (!MsgBarToggleDisplay) MsgBarToggleDisplay = TRUE;
	      print_msg ("Display Msg Bar...");
	    }
	}
      break;
    case WORDWRAP        :
     if ((!ToggleWordwrap) && (TOGGLE_WORDWRAP))
	ToggleWordwrap = TRUE;
      else
	{
	  if (TOGGLE_WORDWRAP)
	    {
	      TOGGLE_WORDWRAP = FALSE;
	      for (i = 0; i < OpenedFilesCnt; i++)
		gtk_text_set_word_wrap (GTK_TEXT(FPROPS(CurrentPage, Text)),
					FALSE);
	      print_msg ("Wordwrap disabled...");
	    }
	  else
	    {
	      TOGGLE_WORDWRAP = TRUE;
	      for (i = 0; i < OpenedFilesCnt; i++)
		gtk_text_set_word_wrap (GTK_TEXT(FPROPS(CurrentPage, Text)),
					TRUE);
	      if (!ToggleWordwrap) ToggleWordwrap = TRUE;
	      print_msg ("Wordwrap enabled...");
	    }
	}
      break;
    case TAB_POS_TOP          :
      TAB_POSITION = 1;
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_TOP);
      break;
    case TAB_POS_BOTTOM       :
      TAB_POSITION = 2;
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_BOTTOM);
      break;
    case TAB_POS_LEFT         :
      TAB_POSITION = 3;
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_LEFT);
      break;
    case TAB_POS_RIGHT        :
      TAB_POSITION = 4;
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK(MainNotebook), GTK_POS_RIGHT);  
      break;
    case SCROLLBAR_POS_LEFT   :
      SCROLLBAR_POSITION = 1;
      for (i = 0; i < OpenedFilesCnt; i++)
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW
					   (gtk_notebook_get_nth_page
					    (GTK_NOTEBOOK(MainNotebook), i)),
					   GTK_CORNER_TOP_RIGHT);
      break;
    case SCROLLBAR_POS_RIGHT  :
      SCROLLBAR_POSITION = 2;
      for (i = 0; i < OpenedFilesCnt; i++)
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW
					   (gtk_notebook_get_nth_page
					    (GTK_NOTEBOOK(MainNotebook), i)),
					   GTK_CORNER_TOP_LEFT);
      break;
    case PREFS                : display_prefs (&Settings);     break;
    case HELP                 :
      print_msg ("No help available yet...");
      break;
    case ABOUT                : about ();                      break;
    }
  (void)Widget; /* avoid the "unused parameter" warning */
}


/* Menu entries */

static GtkItemFactoryEntry MenuEntries[] = {
  /* {"/_File", NULL, NULL, 0, "<Branch>"}, */
  {"/_File/_New", "<control>N", menu_items_treatment, NEW, "<Item>"},
  {"/File/_Open", "<control>O", menu_items_treatment, OPEN, "<Item>"},
  {"/File/_Save", "<control>S", menu_items_treatment, SAVE, "<Item>"},
  {"/File/Save All", NULL, menu_items_treatment, SAVE_ALL, "<Item>"},
  {"/File/Save _As", NULL, menu_items_treatment, SAVE_AS, "<Item>"},
  {"/File/sep", NULL, NULL, 0, "<Separator>"},
  {"/File/_Recent Files", NULL, NULL, 0, "<Branch>"},
  {"/File/sep", NULL, NULL, 0, "<Separator>"},
  {"/File/Print", NULL, menu_items_treatment, PRINT, "<Item>"},
  {"/File/sep", NULL, NULL, 0, "<Separator>"},
  {"/File/_Close", "<control>W", menu_items_treatment, CLOSE, "<Item>"},
  {"/File/Close All", NULL, menu_items_treatment, CLOSE_ALL, "<Item>"},
  {"/File/E_xit", "<control>Q", menu_items_treatment, QUIT, "<Item>"},
  /*{"/_Edit", NULL, NULL, 0, "<Branch>"}, */
  {"/_Edit/_Undo", "<control>Z", menu_items_treatment, UNDO, "<Item>"},
  {"/Edit/_Redo", "<control>R", menu_items_treatment, REDO, "<Item>"},
  {"/Edit/sep", NULL, NULL, 0, "<Separator>"},
  {"/Edit/Cu_t", "<control>X", menu_items_treatment, CUT, "<Item>"},
  {"/Edit/_Copy", "<control>C", menu_items_treatment, COPY, "<Item>"},
  {"/Edit/_Paste", "<control>V", menu_items_treatment, PASTE, "<Item>"},
  {"/Edit/C_lear", NULL, menu_items_treatment, CLEAR, "<Item>"},
  {"/Edit/_Select All", "<control>A", menu_items_treatment, SELECT_ALL, "<Item>"},
  {"/Edit/Co_mplete", "<alt>space", menu_items_treatment, COMPLETE, "<Item>"},
  {"/Edit/sep", NULL, NULL, 0, "<Separator>"},
  {"/Edit/Toggle _Readonly", NULL, menu_items_treatment, READONLY, "<Item>"},
  {"/Edit/sep", NULL, NULL, 0, "<Separator>"},
  {"/Edit/_Find", "F6", menu_items_treatment, FIND, "<Item>"},
  {"/Edit/_Replace", "F7", menu_items_treatment, REPLACE, "<Item>"},
  {"/Edit/sep", NULL, NULL, 0, "<Separator>"},
  {"/Edit/_Goto Line...", NULL, menu_items_treatment, LINE, "<Item>"}, 
  /* {"/_Tools", NULL, NULL, 0, "<Branch>"}, */
  /* {"/_Tools/_Conversions", NULL, NULL, 0, "<Branch>"}, */
  {"/_Tools/_Conversions/UNIX to DOS", NULL, menu_items_treatment,
   UNIX_DOS, "<Item>"},
  {"/Tools/Conversions/UNIX to MAC", NULL, menu_items_treatment,
   UNIX_MAC, "<Item>"},
  {"/Tools/Conversions/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/Conversions/DOS to UNIX", NULL, menu_items_treatment,
   DOS_UNIX, "<Item>"},
  {"/Tools/Conversions/DOS to MAC", NULL, menu_items_treatment,
   DOS_MAC, "<Item>"},
  {"/Tools/Conversions/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/Conversions/MAC to DOS", NULL, menu_items_treatment,
   MAC_DOS, "<Item>"},
  {"/Tools/Conversions/MAC to UNIX", NULL, menu_items_treatment,
   MAC_UNIX, "<Item>"},
  {"/_Tools/_Conversions (All)/UNIX to DOS", NULL, menu_items_treatment,
   UNIX_DOS_ALL, "<Item>"},
  {"/Tools/Conversions (All)/UNIX to MAC", NULL, menu_items_treatment,
   UNIX_MAC_ALL, "<Item>"},
  {"/Tools/Conversions (All)/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/Conversions (All)/DOS to UNIX", NULL, menu_items_treatment,
   DOS_UNIX_ALL, "<Item>"},
  {"/Tools/Conversions (All)/DOS to MAC", NULL, menu_items_treatment,
   DOS_MAC_ALL, "<Item>"},
  {"/Tools/Conversions (All)/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/Conversions (All)/MAC to DOS", NULL, menu_items_treatment,
   MAC_DOS_ALL, "<Item>"},
  {"/Tools/Conversions (All)/MAC to UNIX", NULL, menu_items_treatment,
   MAC_UNIX_ALL, "<Item>"},
  {"/Tools/sep", NULL, NULL, 0, "<Separator>"},
  {"/_Tools/_Base Converter", NULL, menu_items_treatment, CONVERTER, "<Item>"},
  {"/_Tools/_Color Picker", NULL, menu_items_treatment, COLOR, "<Item>"},
  {"/Tools/Insert _Time", NULL, menu_items_treatment, INSERT_TIME, "<Item>"},
  {"/Tools/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/To _Upper Case", NULL, menu_items_treatment, TO_UPPER, "<Item>"},
  {"/Tools/To _Lower Case", NULL, menu_items_treatment, TO_LOWER, "<Item>"},
  {"/Tools/Ca_pitalize", NULL, menu_items_treatment, CAPITALIZE, "<Item>"},
  {"/Tools/_Invert Case", NULL, menu_items_treatment, INVERT_CASE, "<Item>"},
  {"/Tools/sep", NULL, NULL, 0, "<Separator>"},
  {"/Tools/_File Info", "<control>I", menu_items_treatment,
   FILE_INFO, "<Item>"},
  /* {"/_Settings", NULL, NULL, 0, "<Branch>"}, */
  {"/_Settings/Tool Bar", NULL, menu_items_treatment, TOOLBAR, "<CheckItem>"},
  {"/Settings/Msg Bar", NULL, menu_items_treatment, MSGBAR, "<CheckItem>"},
  {"/Settings/Wordwrap", NULL, menu_items_treatment, WORDWRAP, "<CheckItem>"},
  /* {"/Settings/Doc Tabs", NULL, NULL, 0, "<Branch>"}, */
  {"/Settings/Doc Tabs/_Top", NULL, menu_items_treatment,
   TAB_POS_TOP, "<RadioItem>"},
  {"/Settings/Doc Tabs/_Bottom", NULL, menu_items_treatment,
   TAB_POS_BOTTOM, "/Settings/Doc Tabs/Top"},
  {"/Settings/Doc Tabs/_Left", NULL, menu_items_treatment,
   TAB_POS_LEFT, "/Settings/Doc Tabs/Top"},
  {"/Settings/Doc Tabs/_Right", NULL, menu_items_treatment,
   TAB_POS_RIGHT, "/Settings/Doc Tabs/Top"},
  /* {"/Settings/Scroll Bar", NULL, NULL, 0, "<Branch>"}, */
  {"/Settings/Scroll Bar/_Left", NULL, menu_items_treatment,
   SCROLLBAR_POS_LEFT, "<RadioItem>"},
  {"/Settings/Scroll Bar/_Right", NULL, menu_items_treatment,
   SCROLLBAR_POS_RIGHT, "/Settings/Scroll Bar/Left"},
  {"/Settings/sep", NULL, NULL, 0, "<Separator>"},
  {"/Settings/_Preferences", NULL, menu_items_treatment, PREFS, "<Item>"},
  {"/_Languages", NULL, NULL, 0, "<Branch>"},
  /* {"/_Help", NULL, NULL, 0, "<Branch>"}, */
  {"/_Help/Help", NULL, menu_items_treatment, HELP, "<Item>"},
  {"/Help/_About", NULL, menu_items_treatment, ABOUT, "<Item>"},
};


/* Return the menubar widget based on 'MenuEntries' */

GtkWidget *menubar_new (GtkWidget *Window)
{
  GtkWidget *Menu;
  GtkItemFactory *ItemFactory;
  GtkAccelGroup *AccelGroup;
  gchar *AccelRC;
  guint NbMenuEntries = sizeof (MenuEntries) / sizeof (MenuEntries[0]);

  MsgBarToggleDisplay = FALSE;
  ToolBarToggleDisplay = FALSE;
  ToggleWordwrap = FALSE;
  AccelGroup = gtk_accel_group_new();
  ItemFactory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", AccelGroup);
  gtk_item_factory_create_items (ItemFactory, NbMenuEntries,
				 MenuEntries, NULL);
  gtk_window_add_accel_group (GTK_WINDOW (Window), AccelGroup);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/File")), Menu);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/File/Recent Files")), Menu);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Edit")), Menu);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Tools")), Menu);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Tools/Conversions")), Menu);
  Menu = gtk_tearoff_menu_item_new (); 
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Settings")), Menu);
  Menu = gtk_tearoff_menu_item_new (); 
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Settings/Doc Tabs")), Menu);
  if (MSGBAR_DISPLAY)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Msg Bar")),
				    TRUE);
  if (TOOLBAR_DISPLAY)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Tool Bar")),
				    TRUE);
  if (TOGGLE_WORDWRAP)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Wordwrap")),
				    TRUE);
  if (TAB_POSITION == 1)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Doc Tabs/Top")),
				    TRUE);
  if (TAB_POSITION == 2)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Doc Tabs/Bottom")),
				    TRUE);
  if (TAB_POSITION == 3)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Doc Tabs/Left")),
				    TRUE);
  if (TAB_POSITION == 4)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Doc Tabs/Right")),
				    TRUE);
  Menu = gtk_tearoff_menu_item_new (); 
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Settings/Scroll Bar")),
		    Menu);
  if (SCROLLBAR_POSITION == 1)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Scroll Bar/Left")),
				    TRUE);
  if (SCROLLBAR_POSITION == 2)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
				    (gtk_item_factory_get_widget
				     (ItemFactory,
				      "<main>/Settings/Scroll Bar/Right")),
				    TRUE);
  Menu = gtk_tearoff_menu_item_new ();
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Languages")), Menu);
  Menu = gtk_tearoff_menu_item_new (); 
  gtk_menu_prepend (GTK_MENU(gtk_item_factory_get_widget
			     (ItemFactory, "<main>/Help")), Menu);
  Menu = gtk_item_factory_get_widget (ItemFactory, "<main>");
  AccelRC = g_strconcat (g_get_home_dir (), PATH_SEP_STRING, CONF_DIR,
                         PATH_SEP_STRING, "AccelRC", NULL);
  gtk_item_factory_parse_rc (AccelRC);
  g_free (AccelRC);
  init_recent_files ();
  init_languages_menu ();
  gtk_menu_bar_set_shadow_type (GTK_MENU_BAR(Menu), GTK_SHADOW_NONE);
  return (Menu);
}


/* PopUp Menu Entries */

static GtkItemFactoryEntry PopUpMenuEntries[] = {
  {"/Cu_t", NULL, menu_items_treatment, CUT, "<Item>"},
  {"/_Copy", NULL, menu_items_treatment, COPY, "<Item>"},
  {"/_Paste", NULL, menu_items_treatment, PASTE, "<Item>"},
  {"/_Select All", NULL, menu_items_treatment, SELECT_ALL, "<Item>"},
  {"/sep", NULL, NULL, 0, "<Separator>"},
  {"/_Open", NULL, menu_items_treatment, OPEN, "<Item>"},
  {"/_Save", NULL, menu_items_treatment, SAVE, "<Item>"},
  {"/Save _As", NULL, menu_items_treatment, SAVE_AS, "<Item>"},
  {"/sep", NULL, NULL, 0, "<Separator>"},
  {"/Close", NULL, menu_items_treatment, CLOSE, "<Item>"},
};


/* Return the popup menu widget based on 'PopUpMenuEntries' */

GtkWidget *popup_menu_new (void)
{
  GtkItemFactory *ItemFactory;
  GtkWidget *PopUpMenu;
  guint NbMenuEntries;

  NbMenuEntries = sizeof (PopUpMenuEntries) / sizeof (PopUpMenuEntries[0]);
  ItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<popup_main>", NULL);
  gtk_item_factory_create_items (ItemFactory, NbMenuEntries,
				 PopUpMenuEntries, NULL);
  PopUpMenu = gtk_item_factory_get_widget (ItemFactory, "<popup_main>");
  return (PopUpMenu);
}


/* Display the popup menu widget based on 'PopUpMenuEntries' */

gboolean popup_menu_show (GtkMenu *PopUpMenu, GdkEvent *Event)
{
  GdkEventButton *EventPointer = (GdkEventButton *)Event;
  
  if (Event -> type == GDK_BUTTON_PRESS)
    if (EventPointer -> button == 3)
      {
	gtk_menu_popup(PopUpMenu, NULL, NULL, NULL, NULL,
		       EventPointer -> button, EventPointer -> time);
	return (TRUE);
      }
  return (FALSE);
}


/**************************** NOTEBOOK MANAGEMENT ****************************/


/* Add a Page in a Notebook */

void add_page_in_notebook (GtkNotebook *Notebook, gchar *FileName)
{
  GtkWidget *VScrolledWindow;
  GtkWidget *TabLabel, *MenuLabel;
  GtkWidget *PopUpMenu;
  GtkStyle *Style;
  t_fprops Storage;
  
  g_array_append_val (FileProperties, Storage);
  if (!FileName)
    {
      gchar *Untitled;
      
      Untitled = g_strdup_printf (UNTITLED " %d", ++NewFilesCnt);
      init_file_properties (Untitled, OpenedFilesCnt);
      g_free (Untitled);
    }
  else init_file_properties (FileName, OpenedFilesCnt);
  VScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(VScrolledWindow),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  if (SCROLLBAR_POSITION == 1)
    gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW(VScrolledWindow),
				       GTK_CORNER_TOP_RIGHT);
  if (SCROLLBAR_POSITION == 2)
    gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW(VScrolledWindow),
				       GTK_CORNER_TOP_LEFT);
  FPROPS(OpenedFilesCnt, Text) = gtk_text_new (NULL, NULL);
  Style = gtk_style_copy (gtk_widget_get_style(FPROPS(OpenedFilesCnt, Text)));
  Style -> font = gdk_fontset_load (FONT);
  Style -> bg[GTK_STATE_SELECTED].red = SELECTED_BG_RED;
  Style -> bg[GTK_STATE_SELECTED].green = SELECTED_BG_GREEN;
  Style -> bg[GTK_STATE_SELECTED].blue = SELECTED_BG_BLUE;
  Style -> fg[GTK_STATE_SELECTED].red = SELECTED_FG_RED;
  Style -> fg[GTK_STATE_SELECTED].green = SELECTED_FG_GREEN;
  Style -> fg[GTK_STATE_SELECTED].blue = SELECTED_FG_BLUE;
  Style -> base[GTK_STATE_NORMAL].red = BG_RED;
  Style -> base[GTK_STATE_NORMAL].green = BG_GREEN;
  Style -> base[GTK_STATE_NORMAL].blue = BG_BLUE;
  Style -> text[GTK_STATE_NORMAL].red = FG_RED;
  Style -> text[GTK_STATE_NORMAL].green = FG_GREEN;
  Style -> text[GTK_STATE_NORMAL].blue = FG_BLUE;
  gtk_widget_set_style (FPROPS(OpenedFilesCnt, Text), Style);
  if (!FPROPS(OpenedFilesCnt, ReadOnly))
    gtk_text_set_editable (GTK_TEXT(FPROPS(OpenedFilesCnt, Text)), TRUE);
  if (TOGGLE_WORDWRAP)
    gtk_text_set_word_wrap (GTK_TEXT(FPROPS(OpenedFilesCnt, Text)),
			    TRUE);
  PopUpMenu = popup_menu_new ();
  gtk_signal_connect_object (GTK_OBJECT(FPROPS(OpenedFilesCnt, Text)),
			     "event", (GtkSignalFunc)popup_menu_show,
			     GTK_OBJECT(PopUpMenu));
  gtk_container_add (GTK_CONTAINER(VScrolledWindow),
		     FPROPS(OpenedFilesCnt, Text));
  TabLabel = gtk_label_new ("");
  MenuLabel = gtk_label_new ("");
  gtk_notebook_append_page_menu (Notebook, VScrolledWindow,
				 TabLabel, MenuLabel);
  gtk_widget_show_all (GTK_WIDGET(Notebook));
  set_label (Notebook, OpenedFilesCnt);
  gtk_notebook_set_page (GTK_NOTEBOOK(Notebook), OpenedFilesCnt++);
}


/* Set the current page Tab & Menu Labels */

void  set_label (GtkNotebook *Notebook, gint CurrentPage)
{
  gchar BaseTitle[MAX_TAB_LABEL_LENGTH];
  gint i = 0, j = 0;

  while (FPROPS(CurrentPage, BaseName[i]) != '\0')
    {
      if (i < MAX_TAB_LABEL_LENGTH)
	BaseTitle[j++] = FPROPS(CurrentPage, BaseName[i++]);
      else
	{
	  for (i = -3; i <= -1; i++) BaseTitle[i+j] = '.';
	  break;
	}
    }
  BaseTitle[j] = '\0';
  if (FPROPS(CurrentPage, Changed[0]))
    {
      if (!FPROPS(CurrentPage, ReadOnly))
	{
	  gchar *Title;
	  
	  Title = g_strconcat ("*", BaseTitle, NULL);
	  gtk_notebook_set_tab_label_text (Notebook, gtk_notebook_get_nth_page
					   (Notebook, CurrentPage), Title);
	  gtk_notebook_set_menu_label_text (Notebook, gtk_notebook_get_nth_page
					    (Notebook, CurrentPage), Title);
	  g_free (Title);
	}      
      else
	{
	  gchar *Title;
	  
	  Title = g_strconcat ("RO *", BaseTitle, NULL);
	  gtk_notebook_set_tab_label_text (Notebook, gtk_notebook_get_nth_page
					   (Notebook, CurrentPage), Title);
	  gtk_notebook_set_menu_label_text (Notebook, gtk_notebook_get_nth_page
					    (Notebook, CurrentPage), Title);
	  g_free (Title);
	}
    }
  else
    {
      if (FPROPS(CurrentPage, ReadOnly))
	{
	  gchar *Title;
	  
	  Title = g_strconcat ("RO ", BaseTitle, NULL);
	  gtk_notebook_set_tab_label_text (Notebook, gtk_notebook_get_nth_page
					   (Notebook, CurrentPage), Title);
	  gtk_notebook_set_menu_label_text (Notebook, gtk_notebook_get_nth_page
					    (Notebook, CurrentPage), Title);
	  g_free (Title);
	}
      else
	{
	  gtk_notebook_set_tab_label_text (Notebook, gtk_notebook_get_nth_page
					   (Notebook, CurrentPage), BaseTitle);
	  gtk_notebook_set_menu_label_text (Notebook, gtk_notebook_get_nth_page
					    (Notebook, CurrentPage),
					    BaseTitle);
	}
    }
}


/* Set the Window title (used when you change the active page with your
   little mouse...) */

void set_title (GtkNotebook *Notebook, GtkNotebookPage *Page,
		gint CurrentPage)
{
  gchar *Title;

  if (CurrentPage == -1)
    gtk_window_set_title (GTK_WINDOW(MainWindow),
			  (Title = g_strconcat
			   (WELCOME_MSG, " ", APP_NAME, " ",
			    VERSION_NUMBER, NULL)));
  else gtk_window_set_title (GTK_WINDOW(MainWindow),
			     (Title = g_strconcat
			      (FPROPS(CurrentPage, Name), " - ",
			       APP_NAME, " ", VERSION_NUMBER, NULL)));
  g_free (Title);
  (void)Notebook; /* avoid the "unused parameter" warning */
  (void)Page; /* avoid the "unused parameter" warning */
}


/***************************** INTERFACE DISPLAY *****************************/


/* Parse the command line and load files 'beaver [FILE1] [FILE2] ...' */

void command_line (gint argc, gchar *argv[])
{
  if (argc >= 2)
    {
      gint i;
      gboolean FirstTime = TRUE;
      
      for (i = 1; i < argc; i++)
	{
	  struct stat Stats;
	  gchar *BaseName;
	  gchar *FileName;
	  gint CurrentPage;

	  if (stat (argv[i], &Stats) != -1)
	    if ((Stats.st_mode & S_IFMT) == S_IFDIR) continue;
	  if (!strcmp ((BaseName = str_get_last_part
			(argv[i], PATH_SEP, TRUE)), "")) continue;
	  if ((!strcmp (argv[i], "--")) && (FirstTime == TRUE))
	    {
	      FirstTime = FALSE;
	      continue;
	    }
	  FileName = get_absolute_path (argv[i]);
	  if (stat (argv[i], &Stats) != -1) put_recent_file (FileName);
	  add_page_in_notebook (GTK_NOTEBOOK(MainNotebook), FileName);
	  CurrentPage = gtk_notebook_get_current_page
	    (GTK_NOTEBOOK(MainNotebook));
	  open_file_in_editor(GTK_WIDGET(FPROPS(CurrentPage, Text)), FileName);
	  g_free (BaseName);
	  g_free (FileName);
	  FPROPS(CurrentPage, Changed[1]) =
	    gtk_signal_connect (GTK_OBJECT(FPROPS(CurrentPage, Text)),
				"changed", (GtkSignalFunc) set_changed, NULL);
	}
    }
}


/* The main function in the shaping process of the interface */  

void interface (gint argc, gchar *argv[])
{
  GtkWidget *MenuBar;
  GtkWidget *VBox;
  GtkWidget *HandleBox;

  gtk_set_locale();  
  gtk_init (&argc, &argv);
  Settings = init_settings ();
  MainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(MainWindow), g_strconcat
			(WELCOME_MSG, " ", APP_NAME, " ",
			 VERSION_NUMBER, NULL));
  gtk_window_set_policy (GTK_WINDOW(MainWindow), TRUE, TRUE, FALSE);
  gtk_widget_set_usize (MainWindow, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
  gtk_signal_connect (GTK_OBJECT(MainWindow), "delete_event",
		      (GtkSignalFunc) quit, NULL); 
  gtk_signal_connect (GTK_OBJECT(MainWindow), "destroy",
		      (GtkSignalFunc) quit, NULL);
  FileProperties = g_array_new (TRUE, FALSE, sizeof(t_fprops));
  VBox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER(MainWindow), VBox);
  HandleBox = gtk_handle_box_new();
  gtk_container_set_border_width (GTK_CONTAINER(HandleBox), 2);
  gtk_box_pack_start (GTK_BOX(VBox), HandleBox, FALSE, FALSE, 0);
  init_toolbar (GTK_BOX(VBox));
  MainNotebook = gtk_notebook_new ();
  read_uedit_wordfile (WORDFILE);
  editor_init();
  MenuBar = menubar_new (MainWindow);
  gtk_container_add (GTK_CONTAINER(HandleBox), MenuBar);
  gtk_notebook_popup_enable (GTK_NOTEBOOK(MainNotebook));
  gtk_notebook_set_homogeneous_tabs (GTK_NOTEBOOK(MainNotebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK(MainNotebook), TRUE);
  gtk_box_pack_start (GTK_BOX(VBox), MainNotebook, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT(MainNotebook), "switch_page",
		      (GtkSignalFunc) set_title, NULL);
  init_msgbar (GTK_BOX(VBox));
  print_msg ("You're the welcome...");
  command_line (argc, argv);
  autosave (AUTOSAVE_DELAY);
  gtk_widget_show_all (MainWindow);
  if (!MSGBAR_DISPLAY) hide_msgbar ();
  if (!TOOLBAR_DISPLAY) hide_toolbar ();
  gtk_timeout_add (80, (GtkFunction)display_line_column, NULL);
  gtk_main ();
  set_preferences_to_disk (&Settings, NULL);
  gtk_item_factory_dump_rc (g_strconcat (g_get_home_dir (), PATH_SEP_STRING,
					 CONF_DIR, PATH_SEP_STRING,
					 "AccelRC", NULL), NULL, FALSE);
}


/* Display a nice little 'About...' Dialog box :) */

void about (void)
{
  GtkWidget *AboutWindow;
  GtkWidget *PixmapWidget;
  GtkWidget *HSeparator;
  GdkPixmap *Pixmap;
  GdkBitmap *Mask;
  GtkStyle  *Style;
  GtkWidget *Button;
  GtkWidget *AboutLabel;  
  
  AboutWindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(AboutWindow), "About " APP_NAME); 
  gtk_window_set_policy (GTK_WINDOW(AboutWindow), FALSE, FALSE, FALSE);
  gtk_window_set_position (GTK_WINDOW(AboutWindow), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW(AboutWindow), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW(AboutWindow),
				GTK_WINDOW(MainWindow));
  gtk_signal_connect_object (GTK_OBJECT(AboutWindow), "delete_event",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(AboutWindow));
  gtk_signal_connect_object (GTK_OBJECT (AboutWindow), "destroy",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(AboutWindow));
  gtk_widget_realize (AboutWindow);
  Style = gtk_widget_get_style (AboutWindow);
  Pixmap = gdk_pixmap_create_from_xpm_d (AboutWindow->window, &Mask,
					 &Style->bg[GTK_STATE_NORMAL],
					 about_xpm);
  PixmapWidget = gtk_pixmap_new (Pixmap, Mask);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(AboutWindow) -> vbox),
		      PixmapWidget, FALSE, FALSE, 0);
  HSeparator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(AboutWindow) -> vbox),
		      HSeparator, FALSE, FALSE, 0);
  AboutLabel = gtk_label_new (APP_NAME " " VERSION_NUMBER "\n"
			      APP_MOTTO "\n\n" APP_URL);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(AboutWindow) -> vbox),
		      AboutLabel, FALSE, FALSE, 10);
  Button = gtk_button_new_with_label (" Resume ");
  gtk_signal_connect_object (GTK_OBJECT(Button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT(AboutWindow));
  GTK_WIDGET_SET_FLAGS (Button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(AboutWindow) -> action_area),
		      Button, TRUE, TRUE, 0);
  gtk_widget_grab_default (Button);
  gtk_widget_show_all (AboutWindow);
  print_msg ("Display About window...");
}
