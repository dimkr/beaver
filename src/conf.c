/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** conf.c
**
** Author<s>:   Emmanuel Turquin (aka "Ender") <turqui_e@epita.fr>
** Last update: Thu Feb 15 20:12:09 2001
** Description: Beaver preferences management source
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "editor.h"
#include "struct.h"
#include "conf.h"

gchar		*parse_string(gchar *str)
{
  gint		i, j;
  gchar		*new_str;
  
  new_str = g_malloc(1);
  for (i = j = 0; str[i]; i++, j++)
    {
      if (str[i] == '\\')
	{
	  i++;
	  if (str[i] == '\\')
	    new_str[j] = '\\';
	  else if (str[i] == 'n')
	    new_str[j] = '\n';
	  else if (str[i] == 't')
	    new_str[j] = '\t';
	  else
	    new_str[j] = str[i];
	}
      else
	new_str[j] = str[i];
      new_str = g_realloc(new_str, j + 2);
    }
  new_str[j] = '\0';
  return (new_str);
}

gint		get_section_limits(gchar *section, t_conf *conf)
{
  gint		i;
  gint		len;
  gint		ret_val;
  gchar		*section_eq;

  ret_val = 1;
  section_eq = g_strconcat("[" , section, "]", NULL);
  len = strlen(section_eq);
  for (i = 0; conf->file_content[i]; i++)
    if (!g_strncasecmp(conf->file_content[i], section_eq, len))
      {
	conf->section[0] = ++i;
	while (conf->file_content[i] && conf->file_content[i][0] != '[')
	  i++;
	conf->section[1] = i;
	ret_val = 0;
	break;
      }
  g_free(section_eq);
  return (ret_val);
}

gint		get_key(gchar *key, t_conf *conf)
{
  gint		i, j;
  gint		len;
  gint		ret_val;
  gchar		*key_eq;

  ret_val = 1;
  key_eq = g_strconcat(key, " = ", NULL);
  len = strlen(key_eq);
  for (i = conf->section[0]; i < conf->section[1]; i++)
    if (!g_strncasecmp(conf->file_content[i], key_eq, len))
      {
	conf->line = g_realloc(conf->line,
			       strlen(conf->file_content[i]) - len + 1);
	for (j = len; conf->file_content[i][j]; j++)
	  conf->line[j - len] = conf->file_content[i][j];
	conf->line[j - len] = '\0';
	ret_val = 0;
	break;
      }
  g_free(key_eq);
  return (ret_val);
}

void		free_conf(t_conf *conf)
{
  g_strfreev(conf->file_content);
  g_free(conf->line);
}

gint		get_conf(gchar *key, t_conf *conf)
{
  gchar		**key_tab;
  gchar		*file_name;
  gchar		*buffer;
  gint		fd;
  gint		i, j;
  
  key_tab = g_strsplit(key, DELIM_STR, 0);
  file_name = g_strconcat(g_get_home_dir(), PATH_SEP_STRING, CONF_DIR,
			  PATH_SEP_STRING, key_tab[0], NULL);
  if ((fd = open(file_name, O_RDONLY)) == -1)
    {
      g_free(file_name);
      g_strfreev(key_tab);
      return (1);
    }
  g_free(file_name);
  i = 0;
  buffer = g_malloc(1);
  do {
    buffer = g_realloc(buffer, 1024 + i);
    j = read(fd, buffer + i, 1024);
    if (j == -1)
      {
	close(fd);
	g_free(buffer);
	g_strfreev(key_tab);
	return (1);
      }
    i += j;
  } while (j);
  buffer[i] = '\0';
  close(fd);
  conf->file_content = g_strsplit(buffer, "\n", 0);
  g_free(buffer);
  /* Comments management: Begin */
  for (i = 0; conf->file_content[i]; i++)
    for (j = 0; conf->file_content[i][j]; j++)
      if (conf->file_content[i][j] == COMMENT &&
	  (conf->file_content[i][j - 1] != '\\' || !j))
	{
	  conf->file_content[i][j] = '\0';
	  break;
	}
  /* Comments management: End */
  conf->line = g_malloc(1);
  conf->line[0] = '\0';
  if (get_section_limits(key_tab[1], conf))
    {
      g_strfreev(key_tab);
      return (1);
    }
  if (get_key(key_tab[2], conf))
    {
      g_strfreev(key_tab);
      return (1);
    }  
  g_strfreev(key_tab);
  return (0);
}

gint		get_int_conf(gchar *key)
{
  t_conf	conf;
  gint		ret_val;

  if (get_conf(key, &conf))
    {
      free_conf(&conf);
      return (0);
    }
  ret_val = (gint) g_strtod(conf.line, NULL);
  free_conf(&conf);
  return (ret_val);
}

gboolean	get_bool_conf(gchar *key)
{
  t_conf	conf;
  gboolean	ret_val;

  if (get_conf(key, &conf))
    {
      free_conf(&conf);
      return (0);
    }
  if (!g_strcasecmp(conf.line, TRUE_STR))
    ret_val = 1;
  else
    ret_val = 0;
  free_conf(&conf);
  return (ret_val);
}

gchar		*get_string_conf(gchar *key)
{
  t_conf	conf;
  gchar		*ret_val;

  if (get_conf(key, &conf))
    {
      free_conf(&conf);
      ret_val = g_malloc(1);
      ret_val[0] = '\0';
      return (ret_val);
    }
  ret_val = parse_string(conf.line);
  free_conf(&conf);
  return (ret_val);
}

gint		find_key(gchar *key, t_conf *conf)
{
  gint		i;
  gint		len;
  gint		ret_val;
  gchar		*key_eq;

  ret_val = 1;
  key_eq = g_strconcat(key, " = ", NULL);
  len = strlen(key_eq);
  for (i = conf->section[0]; i < conf->section[1]; i++)
    if (!g_strncasecmp(conf->file_content[i], key_eq, len))
      {
	conf->section[0] = i;
	ret_val = 0;
	break;
      }
  g_free(key_eq);
  return (ret_val);
}

gint		set_conf(gchar *key, t_conf *conf)
{
  gchar		**key_tab;
  gchar		*file_name;
  gchar		*buffer;
  gchar		*section;
  gchar		*line_to_add;
  gint		fd;
  gint		i, j;
  
  key_tab = g_strsplit(key, DELIM_STR, 0);
  file_name = g_strconcat(g_get_home_dir(), PATH_SEP_STRING, CONF_DIR,
			  PATH_SEP_STRING, key_tab[0], NULL);
  if ((fd = open(file_name, O_RDONLY)) == -1)
    {
      g_free(file_name);
      g_strfreev(key_tab);
      return (1);
    }
  i = 0;
  buffer = g_malloc(1);
  do {
    buffer = g_realloc(buffer, 1024 + i);
    j = read(fd, buffer + i, 1024);
    if (j == -1)
      {
	close(fd);
	g_free(file_name);
	g_free(buffer);
	g_strfreev(key_tab);
	return (1);
      }
    i += j;
  } while (j);
  buffer[i] = '\0';
  close(fd);
  conf->file_content = g_strsplit(buffer, "\n", 0);
  g_free(buffer);
  line_to_add = g_strconcat(key_tab[2], " = ", conf->line, "\n", NULL);
  if (get_section_limits(key_tab[1], conf))
    {
      if ((fd = open(file_name, O_APPEND)) == -1)
	{
	  g_free(file_name);
	  g_free(line_to_add);
	  g_strfreev(key_tab);
	  return (1);
	}
      section = g_strconcat("\n[", key_tab[1], "]\n", NULL);
      write(fd, section, strlen(section));
      g_free(section);
      write(fd, line_to_add, strlen(line_to_add));
    }
  else
    {
      j = find_key(key_tab[2], conf);
      if ((fd = open(file_name, O_WRONLY | O_TRUNC)) == -1)
	{
	  g_free(file_name);
	  g_free(line_to_add);
	  g_strfreev(key_tab);
	  return (1);
	}
      for (i = 0; i < conf->section[0]; i++)
	{
	  write(fd, conf->file_content[i], strlen(conf->file_content[i]));
	  write(fd, "\n", 1);
	}
      write(fd, line_to_add, strlen(line_to_add));
      if (!j)
	i++;
      while (conf->file_content[i])
	{
	  write(fd, conf->file_content[i], strlen(conf->file_content[i]));
	  write(fd, "\n", 1);
	  i++;
	}
    }
  close(fd);
  g_free(file_name);
  g_free(line_to_add);
  g_strfreev(key_tab);
  return (0);
}

gint		set_int_conf(gchar *key, gint value)
{
  t_conf	conf;

  conf.line = g_strdup_printf("%d", value);
  if (set_conf(key, &conf))
    {
      free_conf(&conf);
      return (1);
    }
  free_conf(&conf);
  return (0);
}

gint		set_bool_conf(gchar *key, gboolean value)
{
  t_conf	conf;

  if (value)
    conf.line = g_strdup("TRUE");
  else
    conf.line = g_strdup("FALSE");
  if (set_conf(key, &conf))
    {
      free_conf(&conf);
      return (1);
    }
  free_conf(&conf);
  return (0);
}

gint		set_string_conf(gchar *key, gchar *value)
{
  t_conf	conf;

  conf.line = g_strdup(value);
  if (set_conf(key, &conf))
    {
      free_conf(&conf);
      return (1);
    }
  free_conf(&conf);
  return (0);
}
