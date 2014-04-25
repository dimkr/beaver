/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** editor.h
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
**              Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Last update: 2000-05-01
** Description: Header for auto-completion stuff
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

/*
** Struct for handle datas in signal connection for auto-completion
**
** NOTE_REMARK: auco stands for AUto-COmpletion (same trick as syhi :)
*/

#ifndef __COMPLETION_H__
# define __COMPLETION_H__

/*
** egcs-1.1.1 is not C99-compliant :-(
*/
#ifndef __func__
# define __func__	__FUNCTION__
#endif /* __func__ */

typedef struct		s_auco_datas
{
  gint			ibok, ebok;
}			t_auco_datas;

gint			auto_completion(GtkText *Editor);
void			stop_completion(GtkWidget *widget,
					GdkEventButton *ev,
					gint *coord);
void			auto_completion_double_clic(GtkWidget *widget,
						    GdkEventButton *ev,
						    t_auco_datas *datas);
void			auto_completion_key_press(GtkWidget *window,
						  GdkEventKey *event,
						  t_auco_datas *datas);
gint			compare_listitem(gconstpointer l1, gconstpointer l2);
gboolean		refresh_selection (GtkList *gtklist, gchar *word);

#endif /* !__COMPLETION_H__ */
