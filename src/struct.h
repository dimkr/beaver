/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** struct.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:24:02 2000
** Description:   Beaver main structures, types definitions and macros
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

#ifndef __STRUCT_H__
#define __STRUCT_H__

/* Type: t_widget_info */

typedef struct
{
  GList *current_action;
  guint	SigConnectIns[2];
  guint SigConnectDel[2];
  guint	SigConnectChg[2];
  guint	SigConnectChgAft[2];
  gint Lg;
} t_widget_info;

/* Type: t_fprops */

typedef struct
{
  gchar *Name;
  gchar *BaseName;
  gchar *Type;
  t_widget_info WidgetInfo;
  gint ReadOnly;
  guint Changed[2];
  struct stat Stats;
  GtkWidget *Text;
} t_fprops;

/* Type: t_search_prefs */

typedef struct
{
  gchar *FileName;
  gchar *StringToSearch;
  gboolean BeginCursorPos;
  gboolean CaseSen;
  gboolean RegExp;
  gboolean RepAll;
  gboolean RepAllBuffers;
} t_search_prefs;

/* Type: t_search_tab */

typedef struct
{
  gint Line;
  gint Begin;
  gint End;
} t_search_results;

/* Type: t_settings */

typedef struct
{
  gint     recent_files;
  gboolean main_window_size_autosave;
  gint     main_window_width;
  gint     main_window_height;
  gboolean msgbar_display;
  gint     msgbar_interval;
  gboolean toggle_wordwrap;
  gboolean toolbar_display;
  gint     max_tab_label_length;
  gint     tab_position;
  gint     scrollbar_position;
  gint     complete_window_width;
  gint     complete_window_height;
  gint     bg[3];
  gint     fg[3];
  gint     selected_bg[3];
  gint     selected_fg[3];
  gboolean backup;
  gchar    *backup_ext;
  gint     autosave_delay;
  gchar    *directory;
  gchar    *font;
  gchar    *print_cmd;
  gchar    *wordfile;
  gboolean beep;
  gboolean syn_high;
  gint     syn_high_depth;
  gboolean auto_indent;
  gboolean auto_correct;
} t_settings;

/* Type: t_colors */

typedef struct
{
  gchar lg_descript[MAX_LANG][MAXLEN_LANG_DESCRIPTION];
  gint  colors[MAX_LANG][MAX_COL][3];
  gchar colors_descript[MAX_LANG][MAX_COL][MAXLEN_COL_DESCRIPTION];
  gboolean syhi_enable;
  gboolean auto_correct_enable;
  gboolean auto_indent_enable;
  gint syhi_depth;
} t_colors;

/* Type: t_conf */

typedef struct
{
  gchar **file_content;
  gint	section[2];
  gchar	*line;
}	t_conf;

/* Some usefull macros... */

#define FPROPS(Page, Elt) g_array_index (FileProperties, t_fprops, Page).Elt

#endif
