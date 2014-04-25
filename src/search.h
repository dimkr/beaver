/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** search.h
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:27:37 2000
** Description:   Beaver search & replace functions source header
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

#ifndef __SEARCH_H__
#define __SEARCH_H__

gint char_cmp (gboolean case_sen,
	       gchar char1,
	       gchar char2);
void goto_search (GtkList *MyList,
		  GtkWidget *Widget);
t_search_results *find (gint page,
			gboolean case_sen,
			gboolean reg_exp,
			gint start_pos,
			gchar *string);
gint replace_all (gint page,
		  gboolean case_sen,
		  gboolean reg_exp,
		  gint start_pos,
		  gchar *string_in,
		  gchar *string_out);
void search_window_not_visible (void);
void refresh_search (GtkWidget *Widget,
		     gchar *Data);
void search (gboolean Replace);
void set_line (void);
void goto_line_window_not_visible (void);
void goto_line (void);

#endif
