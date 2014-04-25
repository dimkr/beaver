/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** filesops.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:22:46 2000
** Description:   Files operations source header
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

#ifndef __FILESOPS_H__
#define __FILESOPS_H__

gchar *str_get_last_part (gchar *String, gchar Separator,
			  gboolean ReturnIfSeparatorNotFound);
gchar *get_absolute_path (gchar *FileName);
void set_changed (void);
void toggle_readonly (void);
void init_file_properties (gchar *FileName, gint CurrentPage);
gchar *get_file_mode (struct stat Stats);
void init_recent_files (void);
void put_recent_file (gchar *FileName);
void display_recent_files (void);
void open_recent_file (GtkWidget *Toto, gchar *FileNumber);
void autosave (gint Delay);
void file_selection_window_not_visible (void);
GtkWidget *file_selection_window_new (guint Op);
void save_file_as_func (GtkFileSelection *FileSelector);
void open_file_func (GtkFileSelection *FileSelector);
void new_file (void);
void open_file (void);
void save_file (void);
void save_file_as (void);
void save_all (void);
void question_window_not_visible (void);
GtkWidget *question_window_new (guint Op, gint CurrentPage);
void close_file_func (GtkWidget *Widget, gint CurrentPage);
void save_file_before_close_func (void);
void close_file (void);
void close_all (void);
void quit (void);

#endif
