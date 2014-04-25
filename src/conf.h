/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** conf.h
**
** Author<s>:     Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Latest update: Thu Feb 15 20:12:04 2001
** Description:   Beaver preferences management source header
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

#ifndef __CONF_H__
#define __CONF_H__

#define COMMENT		'#'
#define DELIM_STR	"/"
#define TRUE_STR	"TRUE"

#define INT		1
#define BOOLEAN		2
#define STRING		3

gint		set_int_conf(gchar *key, gint value);
gint		set_bool_conf(gchar *key, gboolean value);
gint		set_string_conf(gchar *key, gchar *value);

gint		get_int_conf(gchar *key);
gboolean	get_bool_conf(gchar *key);
gchar		*get_string_conf(gchar *key);

#endif
