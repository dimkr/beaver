/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** msgbar.h
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Wed May  3 04:23:32 2000
** Description:   Beaver message bar manipulation source header
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

#ifndef __MSGBAR_H__
#define __MSGBAR_H__

void init_msgbar (GtkBox *ParentBox);
void hide_msgbar (void);
void show_msgbar (void);
void clear_msgbar (void);
void print_msg (gchar *Message);
gint get_line (void);
gint get_column (void);
gint get_percent (void);
gboolean display_line_column (void);

/* You shouldn't need to use the following functions... */

void end_msg (gint *TimeOutId);

#endif
