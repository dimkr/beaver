/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** prefs.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:23:45 2000
** Description:   Beaver preferences tool source header
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

#ifndef __PREFS_H__
#define __PREFS_H__

/* Define some properties */

#define MAIN_WINDOW_SIZE_AUTOSAVE Settings.main_window_size_autosave
#define MAIN_WINDOW_WIDTH         Settings.main_window_width
#define MAIN_WINDOW_HEIGHT        Settings.main_window_height
#define TAB_POSITION              Settings.tab_position
#define MAX_TAB_LABEL_LENGTH      Settings.max_tab_label_length
#define SCROLLBAR_POSITION        Settings.scrollbar_position
#define TOOLBAR_DISPLAY           Settings.toolbar_display
#define TOGGLE_WORDWRAP           Settings.toggle_wordwrap
#define FONT                      Settings.font
#define PRINT_CMD                 Settings.print_cmd
#define SELECTED_BG_RED           Settings.selected_bg[0]
#define SELECTED_BG_GREEN         Settings.selected_bg[1]
#define SELECTED_BG_BLUE          Settings.selected_bg[2]
#define SELECTED_FG_RED           Settings.selected_fg[0]
#define SELECTED_FG_GREEN         Settings.selected_fg[1]
#define SELECTED_FG_BLUE          Settings.selected_fg[2]
#define BG_RED                    Settings.bg[0]
#define BG_GREEN                  Settings.bg[1]
#define BG_BLUE                   Settings.bg[2]
#define FG_RED                    Settings.fg[0]
#define FG_GREEN                  Settings.fg[1]
#define FG_BLUE                   Settings.fg[2]
#define WORDFILE                  Settings.wordfile
#define AUTOSAVE_DELAY            Settings.autosave_delay
#define BACKUP                    Settings.backup
#define BACKUP_EXT                Settings.backup_ext
#define MSGBAR_DISPLAY            Settings.msgbar_display
#define MSGBAR_INTERVAL           Settings.msgbar_interval
#define COMPLETE_WINDOW_WIDTH	  Settings.complete_window_width
#define COMPLETE_WINDOW_HEIGHT	  Settings.complete_window_height
#define DIRECTORY                 Settings.directory
#define RECENT_FILES_MAX_NB       Settings.recent_files
#define BEEP                      Settings.beep

typedef struct
{
  GtkWidget *widget;
  gint op;
} t_widgint;

t_settings init_settings (void);
void set_preferences_to_disk (t_settings *set, t_colors *col);
void preferences_window_not_visible (void);
void apply_changes (GtkWidget *Widget, t_settings *set);
void refresh_prefs (GtkWidget *Widget, gchar *Data);
void toggle_sen (GtkWidget *Widget);
void display_general (GtkNotebook *Notebook);
void display_document (GtkNotebook *Notebook);
void display_prefs (t_settings *set);

#endif
