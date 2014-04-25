/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** tools.h
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Thu May  4 00:52:59 2000
** Description:   Beaver tools source header
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

gint min_from_selection (GtkEditable *Text);
gint max_from_selection (GtkEditable *Text);
gchar *get_selection (GtkEditable *Text);
/* UpDown: 1 = upper case, 2 = lower case, 3 = invert case, 4 = capitalize */
void change_case (gint UpDown);
void insert_time (gint CurrentPage);
void converter_window_not_visible (void);
void refresh_converter (GtkWidget *Widget, gint EntryID);
void converter (void);
void color_picker_not_visible (void);
void insert_color (GtkColorSelection *csd);
void color_picker (void);
gint remove_char (char *string, char to_remove);
gint replace_char (char *string, char to_replace, char new_char);
void convert_all(void (*conv_func)(void));
void conv_unix_to_dos (void);
void conv_dos_to_unix (void);
void conv_dos_to_mac (void);
void conv_mac_to_unix (void);
void conv_unix_to_mac (void);
void conv_mac_to_dos (void);
void file_info (gint CurrentPage);
void print_buffer (void);

#endif
