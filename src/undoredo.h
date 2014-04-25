/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** undoredo.h
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
** Last update: 2000-05-01
** Description: Provide Undo/Redo function
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

#ifndef __UNDOREDO_H__
# define __UNDOREDO_H__

typedef enum		e_action_type
{
  insert,
  delete
}			t_action_type;

typedef struct		s_action
{
  t_action_type		type;
  gint			start;
  gint			end;
  gchar			*text;
}			t_action;

gboolean		undoredo_activated;

void			init_undoredo(void);
void			record_action(t_action *action);
gboolean		undo_is_possible(void);
gboolean		redo_is_possible(void);
void			proceed_undo(void);
void			proceed_redo(void);
void			view_undoredo_list(gchar *prefix);

#endif /* !__UNDOREDO_H__ */
