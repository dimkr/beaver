/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** editor.c
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
** Last update: Tue Jul 16 22:36:32 CEST 2002
** Description: Low-level text management (syntax highlighting,
**              auto-indentation, etc) and UltraEdit's "wordfile.txt"
**              parsing
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
** The content of editor.c, which is the core of Beaver, would
** need to be completly rewritten, because when I wrote this code:
**   - I was an extremly inexperienced programmer. Just take 1 min
**     to read some code, it's awful, isn't it ? :)
**   - I didn't know the standard concepts related to parsing
**     (token, scanner, parser, grammar, etc)
**   - I probably included some obscure bugs
**   - I produced a code that is now unmaintainable: extremely long
**     functions, variables name not explicit, etc
**   - My english was even more bad than now, just try to read some
**     of my comments :)
** Moreover, since editor.c represents about 30% of Beaver source
** code, and since Beaver is not yet too large, I think that a
** complete rewrite of Beaver is an option to study.
**   -- After
*/

/*
** NOTE_REMARK : syhi stands for SYntax HIghlighting, the main work to
**  be accomplished by the code in this file
*/

/*
** In order to view debugging messages, define some of these macros
*/
//#define DEBUG_AUTO_INDENT
//#define DEBUG_FCN
//#define DEBUG_WORDFILE
//#define DEBUG_EXT_REC
//#define DEBUG_SYHI
//#define DEBUG_CORRECT
//#define DEBUG_HASHTABLE
//#define DEBUG_FREEZE_THAW

#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h> // Added by Ender
#include <stdlib.h>
#include "editor.h"
#include "struct.h"
#include "msgbar.h"
#include "tools.h"
#include "undoredo.h"
#include "conf.h" // Added by Ender

/*
** This trick is probably not faster than using the libc isalpha()
** implementation. TODO: replace all occurences of char_alpha[c]
** by isalpha(c). -- After
*/
gchar			char_alpha[256] =
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
** To access WidgetInfo, we need some external variables, and we do :
** FPROPS(gtk_notebook_get_current_page(GTK_NOTEBOOK(MainNotebook)),
**        WidgetInfo.<an_element_of_structure>)
*/
extern GtkWidget	*MainNotebook;
extern GArray		*FileProperties;

/*
** Some internal global constants
*/
static gint		refresh_from = 0;
static gboolean		enter_pressed = FALSE;
static gint		word_to_correct = -1;

/*
** This func is called during beaver initialization
**
** Parameters :
**   void
**
** Return values :
**   void
*/
extern void		editor_init(void)
{
  gboolean		color_alloc_error;
  gint			i, j;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  color_alloc_error = FALSE;
  Prefs.ColMap = gdk_colormap_get_system();
  for (i = 0; (i < MAX_LANG) && !color_alloc_error; i++)
    {
      if (!gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].Comment)) ||
	  !gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].CommentAlt)) ||
	  !gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].String0)) ||
	  !gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].String1)) ||
	  !gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].Number)))
	color_alloc_error = TRUE;
      for (j = 0; (j < MAX_COL) && !color_alloc_error; j++)
	if (!gdk_color_alloc(Prefs.ColMap, &(Prefs.Colors.L[i].C[j])))
	  color_alloc_error = TRUE;
    }
  if (color_alloc_error)
      g_print(__FILE__": %s(): Warning: color allocation failure\n", __func__);
  // Modified by Ender : Begin
  Prefs.SyhiDepth = get_int_conf ("General/Adv/SynHighDepth");
  Prefs.AutoCorrection = get_bool_conf ("General/Adv/AutoCorrect");
  Prefs.AutoIndentation = get_bool_conf ("General/Adv/AutoIndent");
  // Modified by Ender : End
  Prefs.TabSize = 8; /* Note: TabSize is currently not used */
  for (i = 0; i < MAX_LANG; i++)
    {
      Prefs.Colors.L[i].Comment.red   = 0xd000;
      Prefs.Colors.L[i].Comment.green = 0x0000;
      Prefs.Colors.L[i].Comment.blue  = 0x0000;
      Prefs.Colors.L[i].CommentAlt.red   = 0xd000;
      Prefs.Colors.L[i].CommentAlt.green = 0x0000;
      Prefs.Colors.L[i].CommentAlt.blue  = 0x8000;
      Prefs.Colors.L[i].String0.red   = 0xd000; /* String0 is usually "str" */
      Prefs.Colors.L[i].String0.green = 0xb000;
      Prefs.Colors.L[i].String0.blue  = 0x3000;
      Prefs.Colors.L[i].String1.red   = 0x8000; /* String1 is usually 'a' */
      Prefs.Colors.L[i].String1.green = 0x4000;
      Prefs.Colors.L[i].String1.blue  = 0x8000;
      Prefs.Colors.L[i].Number.red   = 0x0000;
      Prefs.Colors.L[i].Number.green = 0x9000;
      Prefs.Colors.L[i].Number.blue  = 0x0000;
      Prefs.Colors.L[i].C[0].red   = 0x0000;
      Prefs.Colors.L[i].C[0].green = 0x4000;
      Prefs.Colors.L[i].C[0].blue  = 0xa000;
      Prefs.Colors.L[i].C[1].red   = 0x0000;
      Prefs.Colors.L[i].C[1].green = 0x8000;
      Prefs.Colors.L[i].C[1].blue  = 0x4000;
      Prefs.Colors.L[i].C[2].red   = 0xd000;
      Prefs.Colors.L[i].C[2].green = 0x5000;
      Prefs.Colors.L[i].C[2].blue  = 0x7000;
      Prefs.Colors.L[i].C[3].red   = 0xe000;
      Prefs.Colors.L[i].C[3].green = 0x9000;
      Prefs.Colors.L[i].C[3].blue  = 0x0000;
      Prefs.Colors.L[i].C[4].red   = 0x8000;
      Prefs.Colors.L[i].C[4].green = 0x8000;
      Prefs.Colors.L[i].C[4].blue  = 0x8000;
      Prefs.Colors.L[i].C[5].red   = 0x8000;
      Prefs.Colors.L[i].C[5].green = 0x8000;
      Prefs.Colors.L[i].C[5].blue  = 0x8000;
      Prefs.Colors.L[i].C[6].red   = 0x8000;
      Prefs.Colors.L[i].C[6].green = 0x8000;
      Prefs.Colors.L[i].C[6].blue  = 0x8000;
      Prefs.Colors.L[i].C[7].red   = 0x8000;
      Prefs.Colors.L[i].C[7].green = 0x8000;
      Prefs.Colors.L[i].C[7].blue  = 0x8000;
      /* And so on: Prefs.Colors.L[i].C[j].* where:
	   i belongs to [0..MAX_LANG-1]
	   j belongs to [0..MAX_COL-1] */
    }
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
}

/*
** Fcn called by external source to load content of a file in the widget
**
** Note: Filename can be NULL, in this case, the text widget is not filled
** in.
**
** Parameters :
**  Editor		The GtkWidget that display text
**  Filename		The file to be displayed in the widget
**
** Return values :
**  void
*/
extern void		open_file_in_editor(GtkWidget *Editor,
					    const gchar *Filename)
{
  FILE			*fp;
  gint			filesize;
  gchar			*buffer;
  gint			i, j;
  gint			CurrentPage;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  /* Read file */
  if (Filename && (fp = fopen(Filename, "rt")))
    {
      filesize = 0;
      while (getc(fp) != EOF)
	filesize++;
      if (filesize > 0)
	{
#ifdef DEBUG_FCN
	  g_print(__FILE__": %s(): Filesize > 0, malloc()'ing...\n", __func__);
#endif
	  buffer = g_malloc(filesize + 2);
	  fseek(fp, 0, SEEK_SET);
	  fread(buffer, filesize, 1, fp);
	  fclose(fp);
	  buffer[filesize] = '\n';
	  buffer[filesize + 1] = '\0';
#ifdef DEBUG_FCN
	  g_print(__FILE__": %s(): buffer has been filled in\n", __func__);
#endif
	}
      else
	buffer = NULL;
    }
  else
    {
      buffer = NULL;
      filesize = 0;
    }
  /* Insert file content in the widget */
  if (filesize > 0)
    {
      gtk_text_insert(GTK_TEXT(Editor), NULL, NULL, NULL,
		      buffer, filesize);
#ifdef DEBUG_SYHI
      g_print(__FILE__": %s(): Freeing buffer...\n", __func__);
#endif
      g_free(buffer);
#ifdef DEBUG_SYHI
      g_print(__FILE__": %s(): Buffer freed\n", __func__);
#endif
    }
  /* Init some vars */
  FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 0;
  FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 0;
  FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]) = 0;
  FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]) = 0;
  /* NOTE_WARNING: This fcn does not work !:
     gtk_text_set_point(GTK_TEXT(Editor), 0); */
  gtk_editable_set_position(GTK_EDITABLE(Editor), 0);
  /* Init undo/redo */
  init_undoredo ();
  /* Refresh the Editor */
  refresh_editor(Editor, SYHI_AUTODETECT);
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return ;
}

/*
** This func produces a total refresh of the syhi of the widget
** passed in argument
**
** Parameters :
**  Editor		Pointer to the GtkWidget to refresh
**  tos			Type Of Syhi, can be:
**			tos >= 0		manually selects the language
**			tos == SYHI_AUTODETECT	autodetects the language
**			tos == SYHI_DISABLE	syhi is disabled
**
** Return Values :
**   void
*/
extern void		refresh_editor(GtkWidget *Editor, gint tos)
{
  gint			Lg;
  gint			CurrentPage;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  /* Language detection */
  if (tos >= 0)
    {
      if (tos >= MAX_LANG)
	{
	  print_msg("*Bug!* Selected language number is not supported");
	  Lg = -1;
	}
      else if (!Prefs.L[tos].IsDefined)
	{
	  print_msg("*Bug!* Selected language not defined in the wordfile");
	  Lg = -1;
	}
      else
	  Lg = tos;
    }
  else if (tos == SYHI_AUTODETECT)
      Lg = guess_lang(); /* Note: guess_lang() can return -1 */
  else if (tos == SYHI_DISABLE)
      Lg = -1;
  else
    {
      print_msg("*Bug!* Invalid type of syhi");
      Lg = -1;
    }
  /* Save the language */
  FPROPS(CurrentPage, WidgetInfo.Lg) = Lg;
  /* Suppress old handlers */
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
	      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
	      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
	      FPROPS(CurrentPage, WidgetInfo.SigConnectChg[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
	      FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]) = 0;
    }
  /* Setup new handlers */
  if (Lg >= 0)
    {
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[1]) =
	  gtk_signal_connect(GTK_OBJECT(Editor), "insert_text",
		  (GtkSignalFunc)text_has_been_inserted, NULL);
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[1]) =
	  gtk_signal_connect(GTK_OBJECT(Editor), "delete_text",
		  (GtkSignalFunc)text_has_been_deleted, NULL);
      //NOTE_WARNING: the callback fcn refresh_syhi() needs the "changed"
      //signal to be produced after "delete_text" ans "insert_text" signals
      FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectChg[1]) =
	  gtk_signal_connect(GTK_OBJECT(Editor), "changed",
		  (GtkSignalFunc)refresh_syhi, (gpointer)0xeeeeeeee);
      /* Note: Oops, I don't remember what is the goal of 0xeeeeeeee,
	 maybe just to get a non-NULL value... */
      FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[1]) =
	  gtk_signal_connect_after(GTK_OBJECT(Editor), "changed",
		  (GtkSignalFunc)reconnect_sig, NULL);
    }
  /* Do the refesh */
  if (Lg == -1)
    {
      gint	cursorpos;
      gchar	*buffer;
      guint	buflen;

#ifdef DEBUG_FREEZE_THAW
      g_print(__FILE__": %s(): gtk_text_freeze()...\n", __func__);
#endif
      gtk_text_freeze(GTK_TEXT(Editor));
      cursorpos = gtk_editable_get_position(GTK_EDITABLE(Editor));
      buffer = gtk_editable_get_chars(GTK_EDITABLE(Editor), 0, -1);
      buflen = strlen(buffer);
      gtk_editable_delete_text(GTK_EDITABLE(Editor), 0, -1);
      /*
      ** We use gtk_text_insert() instead of gtk_editable_insert_text()
      ** because we want the text to be in the default color.
      */
      gtk_text_insert(GTK_TEXT(Editor), NULL, NULL, NULL, buffer, buflen);
#ifdef DEBUG_FREEZE_THAW
      g_print(__FILE__": %s(): gtk_text_thaw()...\n", __func__);
#endif
      /*
      ** gtk_text_thaw() must be called before gtk_editable_set_position(),
      ** if it is not the case, there is a segfault !
      */
      gtk_text_thaw(GTK_TEXT(Editor));
      gtk_editable_set_position(GTK_EDITABLE(Editor), cursorpos);
      g_free(buffer);
      /*
      ** text_has_been_inserted() and text_has_been_deleted() need to be
      ** connected even when Lg == -1 (syhi disabled), in order to have the
      ** undo/redo code working. They are connected only now because we
      ** don't want to have the previous actions (gtk_editable_delete_text()
      ** and gtk_editable_insert_text()) in the undo/redo actions list.
      */
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[1]) =
	  gtk_signal_connect(GTK_OBJECT(Editor), "insert_text",
		  (GtkSignalFunc)text_has_been_inserted, NULL);
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 1;
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[1]) =
	  gtk_signal_connect(GTK_OBJECT(Editor), "delete_text",
		  (GtkSignalFunc)text_has_been_deleted, NULL);
    }
  else
    {
      gint		depth;

      depth = Prefs.SyhiDepth;
      Prefs.SyhiDepth = G_MAXINT;
      refresh_from = 0;
      refresh_syhi(GTK_EDITABLE(Editor), NULL);
      /* TODO - if there are some strange bugs, try removing this call to
	 reconnect_sig() (I don't remember why it was needed, and signals
	 seems to be already connected) */
      reconnect_sig(GTK_EDITABLE(Editor), NULL);
      Prefs.SyhiDepth = depth;
    }
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return ;
}

/*
** This func returns the language number of the current Editor
**
** Parameters :
**  void
**
** Return Values :
**  -1			Unknown language (extension not recognized)
** 0..MAX_LANG - 1	Language number
**
** NOTE_WARNING: UEdit language numbers belongs to [1..MAX_LANG]
*/
gint			guess_lang(void)
{
  gchar			*ext_orig;
  gint			ext_len;
  gchar			*ext_up;
  gint			i;
  gint			CurrentPage;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  ext_orig = FPROPS(CurrentPage, Type);
#ifdef DEBUG_EXT_REC
  g_print(__FILE__": %s(): Getting length of ext_orig...\n", __func__);
#endif
  ext_len = strlen(ext_orig);
  /* This macro FPROPS is awful :) */
#ifdef DEBUG_EXT_REC
  g_print(__FILE__": %s(): Original extension = \"%s\", len = %i\n", __func__,
	  ext_orig, ext_len);
#endif
  ext_up = malloc(ext_len + 3); /* 1 space before, 1 after, and '\0' */
  strcpy(ext_up + 1, ext_orig);
  ext_up[0] = ' ';
  ext_up[ext_len + 1] = ' ';
  ext_up[ext_len + 2] = '\0';
  for (i = 1; i < ext_len + 1; i++)
    ext_up[i] = toupper(ext_up[i]);
  /* Now, ext_up contains the extension in upper case */
#ifdef DEBUG_EXT_REC
  g_print(__FILE__": %s(): ' ' + EXTENSION + ' ': \"%s\"\n", __func__, ext_up);
#endif
  for (i = 0; i < MAX_LANG; i++)
    {
#ifdef DEBUG_EXT_REC
      g_print(__FILE__": %s(): Ext of language /L%i = \"%s\"\n", __func__,
	      i + 1, Prefs.L[i].Extensions);
#endif
      if (Prefs.L[i].IsDefined && strstr(Prefs.L[i].Extensions, ext_up))
	{
	  free(ext_up);
#ifdef DEBUG_FCN
	  g_print(__FILE__": %s(): End\n", __func__);
#endif
	  return i;
	}
    }
  free(ext_up);
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return -1;
}


/*
** This fcn is used to read the UltraEdit wordfile to init different
** fields of the global variable Prefs
**
** Parameters :
**  wf_name		UEdit wordfile name
**
** Return Values :
**  0			All is right
**  -1			Can not open wordfile
**  -2			Parse error in a language section
*/
extern gint		read_uedit_wordfile(const gchar *wf_name)
{
  FILE			*fp;
  gint			filesize;
  gchar			*buffer;
  gint			i;
  gint			j;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  /* Initialisation of all variables */
  /* TODO: Free all these variables by creating a new fcn called when
     exiting Beaver */
  for (i = 0; i < MAX_LANG; i++)
    {
      for (j = 0; j < MAX_COL; j++)
	{
	  Prefs.L[i].C[j].Keywords = NULL;
	  Prefs.L[i].C[j].Description[0] = '\0';
	}
      Prefs.L[i].IsDefined = FALSE;
      Prefs.L[i].Description[0] = '\0';
      Prefs.L[i].Extensions = NULL;
      Prefs.L[i].LineComment = NULL;
      Prefs.L[i].LineCommentAlt = NULL;
      Prefs.L[i].BlockCommentOn = NULL;
      Prefs.L[i].BlockCommentOff = NULL;
      Prefs.L[i].BlockCommentOnAlt = NULL;
      Prefs.L[i].BlockCommentOffAlt = NULL;
      Prefs.L[i].IsCaseSensitive = TRUE;
      Prefs.L[i].HaveString = TRUE;
      Prefs.L[i].StringChar0 = '\"';
      Prefs.L[i].StringChar1 = '\0';
      Prefs.L[i].EscapeChar = '\0';
      Prefs.L[i].Delimiters = NULL;
      Prefs.L[i].IndentString = NULL;
      Prefs.L[i].UnindentString = NULL;
      Prefs.L[i].MarkerChars = NULL;
      Prefs.L[i].FunctionString = NULL;
      Prefs.L[i].IsHTML = FALSE;
    }
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): Openning \"%s\"...\n", __func__, wf_name);
#endif
  if (!(fp = fopen(wf_name, "rt")))
    {
      gchar *wf_not_found;
      
      wf_not_found = g_strconcat("WARNING: Cannot find \"", wf_name,
				 "\"...", NULL);
      print_msg (wf_not_found);
      g_free (wf_not_found);
      return -1;
    }
  filesize = 0;
  while (getc(fp) != EOF)
    filesize++;
  if (filesize > 0)
    {
      buffer = g_malloc(filesize + 2);
      fseek(fp, 0, SEEK_SET);
      for (i = 0; i < filesize; i++)
	buffer[i] = getc(fp);
      buffer[filesize] = '\n';
      buffer[filesize + 1] = '\0';
    }
  else
    buffer = NULL;
  fclose(fp);
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): Wordfile size = %i\n", __func__, filesize);
#endif
  /* Here: main work */
  i = 0;
  while (i + 1 <= filesize)
    {
      if (!strncmp(buffer + i, "/L", 2))
	{
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Calling "
		  "parse_language_section()\n", __func__);
#endif
	  if (parse_language_section(buffer, filesize, &i))
	    {
	      g_print(__FILE__": %s(): End, parse error in a "
		      "language section of \"%s\"\n", __func__, wf_name);
	      g_free(buffer);
	      return -2;
	    }
	}
      else
	i++;
    }
  g_free(buffer);
  for (i = 0; i < MAX_LANG; i++)
    {
      if (!Prefs.L[i].IsDefined)
	sprintf(Prefs.L[i].Description, "Language %i", i + 1);
      Prefs.L[i].Description[MAXLEN_LANG_DESCRIPTION] = 0;
      for (j = 0; j < MAX_COL; j++)
	{
	  if (!Prefs.L[i].C[j].Keywords)
	    sprintf(Prefs.L[i].C[j].Description, "Keywords Group %i", j + 1);
	  Prefs.L[i].C[j].Description[MAXLEN_COL_DESCRIPTION] = 0;
	}
    }
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return 0;
}

/*
** Parse a language section in a UEdit wordfile
**
** Parameters:
**  buffer		Is a null-terminated string, representing the /Lx
**			section in a wordfile
**  size		Size of the buffer
**  start		buffer + *start points on "/Lx..."
**
** Return values:
**  0			All is right
**  -1			Parse error in a color section
*/
gint			parse_language_section(gchar *buffer,
					       gint size,
					       gint *start)
{
  gint			Lg;
  gint			i;
  gint			j;
  gchar			c;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  i = *start;
  i = i + 2;
  /* buffer[i] is the 1st digit of language number */
  j = i;
  while (isdigit(buffer[j]))
    j++;
  /* buffer[j] is the first char after the last digit of language number */
  c = buffer[j];
  buffer[j] = 0;
  Lg = atoi(buffer + i) - 1;
  buffer[j] = c;
  if (Lg + 1 > MAX_LANG)
    {
      g_print(__FILE__": %s(): Warning, /L%i section detected "
	      "but only %i are allowed\n", __func__, Lg + 1, MAX_LANG);
      return -1;
    }
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): /L%i section detected\n", __func__, Lg + 1);
#endif
  Prefs.L[Lg].IsDefined = TRUE;
  i = j;
  if (buffer[i] == '\"')
    {
      /* There is a description of the language */
      j = ++i;
      while (buffer[j] != '\"')
	j++;
      /* buffer[i] is the 1st char of description and buffer[j] the 2nd '\"' */
      if ((j - i) > MAXLEN_LANG_DESCRIPTION)
	{
	  g_print(__FILE__": %s(): Warning, description of "
		  "language %i is longer than %i\n", __func__, Lg + 1,
		  MAXLEN_LANG_DESCRIPTION);
	  return -1;
	}
      strncpy(Prefs.L[Lg].Description, buffer + i, j - i);
      Prefs.L[Lg].Description[j - i] = '\0';
      i = ++j;
    }
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): Description of language %i = "
	  "\"%s\"\n", __func__, Lg + 1, Prefs.L[Lg].Description);
#endif
  /* Here, buffer[i (= j)] is next non-identified char */
  /* The while loop below reads the line /Lx of a wordfile */
  while (buffer[i] != '\n')
    {
      /* LineComment */
      if (!strncmp(buffer + i, "Line Comment = ", 15))
	{
	  Prefs.L[Lg].LineComment = g_malloc(MAXLEN_LINE_COMMENT + 1);
	  i += 15;
	  j = -1;
	  while ((++j < MAXLEN_LINE_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].LineComment[j] = buffer[i + j];
	  if ((j == MAXLEN_LINE_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Line Comment of "
		      "language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_LINE_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].LineComment[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Line Comment = \"%s\"\n", __func__,
		  Prefs.L[Lg].LineComment);
#endif
	}
      /* LineCommentAlt */
      else if (!strncmp(buffer + i, "Line Comment Alt = ", 19))
	{
	  Prefs.L[Lg].LineCommentAlt = g_malloc(MAXLEN_LINE_COMMENT + 1);
	  i += 19;
	  j = -1;
	  while ((++j < MAXLEN_LINE_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].LineCommentAlt[j] = buffer[i + j];
	  if ((j == MAXLEN_LINE_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Line Comment Alt of "
		      "language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_LINE_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].LineCommentAlt[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Line Comment Alt = \"%s\"\n", __func__,
		  Prefs.L[Lg].LineCommentAlt);
#endif
	}
      /* LineComment Num */
      else if (!strncmp(buffer + i, "Line Comment Num = ", 19))
	{
	  gint		line_cmt_len;

	  Prefs.L[Lg].LineComment = g_malloc(MAXLEN_LINE_COMMENT + 1);
	  line_cmt_len = buffer[i += 19] - '0';
	  if (line_cmt_len > MAXLEN_LINE_COMMENT)
	    {
	      g_print(__FILE__": %s(): Warning, Line Comment Num of "
		      "language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_LINE_COMMENT);
	      return -1;
	    }
	  i++;
	  /* Here buffer[i] is the 1st char of LineComment */
	  for (j = 0; j < line_cmt_len; j++)
	    Prefs.L[Lg].LineComment[j] = buffer[i + j];
	  Prefs.L[Lg].LineComment[j] = '\0';
	  i += line_cmt_len;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Line Comment Num = \"%s\"\n", __func__,
		  Prefs.L[Lg].LineComment);
#endif
	}
      /* BlockCommentOn */
      else if (!strncmp(buffer + i, "Block Comment On = ", 19))
	{
	  Prefs.L[Lg].BlockCommentOn = g_malloc(MAXLEN_BLOCK_COMMENT + 1);
	  i += 19;
	  j = -1;
	  while ((++j < MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].BlockCommentOn[j] = buffer[i + j];
	  if ((j == MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Block Comment On of "
		      "language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_BLOCK_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].BlockCommentOn[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Block Comment On = \"%s\"\n", __func__,
		  Prefs.L[Lg].BlockCommentOn);
#endif
	}
      /* BlockCommentOff */
      else if (!strncmp(buffer + i, "Block Comment Off = ", 20))
	{
	  Prefs.L[Lg].BlockCommentOff = g_malloc(MAXLEN_BLOCK_COMMENT + 1);
	  i += 20;
	  j = -1;
	  while ((++j < MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].BlockCommentOff[j] = buffer[i + j];
	  if ((j == MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Block Comment Off of "
		      "language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_BLOCK_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].BlockCommentOff[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Block Comment Off = \"%s\"\n", __func__,
		  Prefs.L[Lg].BlockCommentOff);
#endif
	}
      /* BlockCommentOnAlt */
      else if (!strncmp(buffer + i, "Block Comment On Alt = ", 23))
	{
	  Prefs.L[Lg].BlockCommentOnAlt = g_malloc(MAXLEN_BLOCK_COMMENT + 1);
	  i += 23;
	  j = -1;
	  while ((++j < MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].BlockCommentOnAlt[j] = buffer[i + j];
	  if ((j == MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Block Comment On Alt "
		      "of language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_BLOCK_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].BlockCommentOnAlt[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Block Comment On Alt = \"%s\"\n", __func__,
		  Prefs.L[Lg].BlockCommentOnAlt);
#endif
	}
      /* BlockCommentOffAlt */
      else if (!strncmp(buffer + i, "Block Comment Off Alt = ", 24))
	{
	  Prefs.L[Lg].BlockCommentOffAlt = g_malloc(MAXLEN_BLOCK_COMMENT + 1);
	  i += 24;
	  j = -1;
	  while ((++j < MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    Prefs.L[Lg].BlockCommentOffAlt[j] = buffer[i + j];
	  if ((j == MAXLEN_BLOCK_COMMENT) && (buffer[i + j] != ' '))
	    {
	      g_print(__FILE__": %s(): Warning, Block Comment Off Alt "
		      "of language /L%i is longer than %i\n", __func__,
		      Lg + 1, MAXLEN_BLOCK_COMMENT);
	      return -1;
	    }
	  Prefs.L[Lg].BlockCommentOffAlt[j] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Block Comment Off Alt = \"%s\"\n", __func__,
		  Prefs.L[Lg].BlockCommentOffAlt);
#endif
	}
      /* StringChars */
      else if (!strncmp(buffer + i, "String Chars = ", 15))
	{
	  i += 15;
	  if (buffer[i] != ' ')
	    Prefs.L[Lg].StringChar0 = buffer[i];
	  i++;
	  if (buffer[i] != ' ')
	    Prefs.L[Lg].StringChar1 = buffer[i];
	  i++;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): String Chars = \'%c\' & \'%c\'\n", __func__,
		  Prefs.L[Lg].StringChar0, Prefs.L[Lg].StringChar1);
#endif
	}
      /* EscapeChar */
      else if (!strncmp(buffer + i, "Escape Char = ", 14))
	{
	  i += 14;
	  if (buffer[i] != ' ')
	    Prefs.L[Lg].EscapeChar = buffer[i];
	  i++;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Escape Char = \'%c\'\n", __func__,
		  Prefs.L[Lg].EscapeChar);
#endif
	}
      /* FileExtensions */
      /* NOTE_WARNING: This sections is supposed to end the line /Lx */
      else if (!strncmp(buffer + i, "File Extensions = ", 18))
	{
	  i += 18;
	  j = 0;
	  while (buffer[i + j] != '\n')
	    j++;
	  Prefs.L[Lg].Extensions = g_malloc(1 + j + 1 + 1);
	  Prefs.L[Lg].Extensions[0] = ' ';
	  for (j = 0; buffer[i + j] != '\n'; j++)
	      Prefs.L[Lg].Extensions[1 + j] = buffer[i + j];
	  Prefs.L[Lg].Extensions[j + 1] = ' ';
	  Prefs.L[Lg].Extensions[j + 2] = '\0';
	  i += j;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): File Extensions = \"%s\"\n", __func__,
		  Prefs.L[Lg].Extensions);
#endif
	}
      /* Nocase */
      else if (!strncmp(buffer + i, "Nocase ", 7))
	{
	  i += 7;
	  Prefs.L[Lg].IsCaseSensitive = FALSE;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Nocase\n", __func__);
#endif
	}
      /* Noquote */
      else if (!strncmp(buffer + i, "Noquote ", 8))
	{
	  i += 8;
	  Prefs.L[Lg].HaveString = FALSE;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Noquote\n", __func__);
#endif
	}
      /* HTML */
      else if (!strncmp(buffer + i, "HTML_LANG ", 10))
	{
	  i += 10;
	  Prefs.L[Lg].IsHTML = TRUE;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): HTML_LANG\n", __func__);
#endif
	}
      /* FORTRAN_LANG */
      /* NOTE_TODO: Implement it: 'C', 'c' or '*' in the 1st column is a
	 line comment */
      else if (!strncmp(buffer + i, "FORTRAN_LANG ", 13))
	{
	  i += 13;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): FORTRAN_LANG\n", __func__);
#endif
	}
      /* LATEX_LANG */
      /* NOTE_REMARK: LATEX_LANG is ignored, Beaver does not need it ! */
      else if (!strncmp(buffer + i, "LATEX_LANG ", 11))
	{
	  i += 11;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): LATEX_LANG\n", __func__);
#endif
	}
      else if ((buffer[i] == ' ') || (buffer[i] == '\t'))
	i++;
      /* Nothing */
      else
	{
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Unreconized string in /Lx line, "
		  "beginning = \"%c%c%c%c...\"\n", __func__,
		  buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
#endif
	  i++;
	}
    } /* while (buffer[i] != '\n') */
  i++;
  while ((i + 1 < size) && strncmp(buffer + i, "/C", 2))
    {
      if (!strncmp(buffer + i, "/Delimiters = ", 14))
	{
	  /* Delimiters */
	  j = (i += 14);
	  while (buffer[j] != '\n')
	    j++;
	  j++;
	  /* NOTE_WARNING: Yes, the '\n' needs to be into the Delimiters */
	  Prefs.L[Lg].Delimiters = g_malloc(j - i + 1);
	  strncpy(Prefs.L[Lg].Delimiters, buffer + i, j - i);
	  Prefs.L[Lg].Delimiters[j - i] = '\0';
	  i = j;
	  for (j = 0; j < 256; j++)
	    Prefs.L[Lg].IsADelimiter[j] =
	      strchr(Prefs.L[Lg].Delimiters, j) ? 1 : 0;
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Delimiters = \"%s\"\n", __func__,
		  Prefs.L[Lg].Delimiters);
#endif
	}
      else if (!strncmp(buffer + i, "/Indent Strings =", 17))
	{
	  /* Indent Strings */
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Indent String = ", __func__);
#endif
	  i += 17;
	  while ((buffer[i] != '\n') && (buffer[i] != '\"'))
	    i++;
	  if (buffer[i] == '\"')
	    {
	      j = ++i;
	      while ((buffer[j] != '\n') && (buffer[j] != '\"'))
		j++;
	      /* Here, if buffer[j] == '\n', it is considered as a '\"' */
	      Prefs.L[Lg].IndentString = g_malloc(j - i + 1);
	      strncpy(Prefs.L[Lg].IndentString, buffer + i, j - i);
	      Prefs.L[Lg].IndentString[j - i] = '\0';
	      i = j;
	      /* NOTE_TODO : Support multiple indent string... */
	      while (buffer[i] != '\n')
		i++;
	      i++;
#ifdef DEBUG_WORDFILE
	      g_print("\"%s\"\n", Prefs.L[Lg].IndentString);
#endif
	    }
	  else
	    i++;
	}
      else if (!strncmp(buffer + i, "/Unindent Strings =", 19))
	{
	  /* Unindent Strings */
	  i += 19;
	  while ((buffer[i] != '\n') && (buffer[i] != '\"'))
	    i++;
	  if (buffer[i] == '\"')
	    {
	      j = ++i;
	      while ((buffer[j] != '\n') && (buffer[j] != '\"'))
		j++;
	      /* Here, if buffer[j] == '\n', it is considered as a '\"' */
	      Prefs.L[Lg].UnindentString = g_malloc(j - i + 1);
	      strncpy(Prefs.L[Lg].UnindentString, buffer + i, j - i);
	      Prefs.L[Lg].UnindentString[j - i] = '\0';
	      i = j;
	      /* NOTE_TODO : Support multiple unindent string... */
	      while (buffer[i] != '\n')
		i++;
	      i++;
#ifdef DEBUG_WORDFILE
	      g_print(__FILE__": %s(): Unindent String = \"%s\"\n", __func__,
		      Prefs.L[Lg].UnindentString);
#endif
	    }
	  else
	    i++;
	}
      else if (!strncmp(buffer + i, "/Marker Characters =", 20))
	{
	  /* Marker Characters */
#ifdef DEBUG_WORDFILE
	      g_print(__FILE__": %s(): Marker Chars = ", __func__);
#endif
	  i += 20;
	  while ((buffer[i] != '\n') && (buffer[i] != '\"'))
	    i++;
	  if (buffer[i] == '\"')
	    {
	      j = ++i;
	      while ((buffer[j] != '\n') && (buffer[j] != '\"'))
		j++;
	      /* Here, if buffer[j] == '\n', it is considered as a '\"' */
	      Prefs.L[Lg].MarkerChars = g_malloc(j - i + 1);
	      strncpy(Prefs.L[Lg].MarkerChars, buffer + i, j - i);
	      Prefs.L[Lg].MarkerChars[j - i] = '\0';
	      i = j;
	      /* NOTE_TODO : Support marker chars... */
	      while (buffer[i] != '\n')
		i++;
	      i++;
#ifdef DEBUG_WORDFILE
	      g_print("\"%s\"\n", Prefs.L[Lg].MarkerChars);
#endif
	    }
	  else
	    i++;
	}
      else if (!strncmp(buffer + i, "/Function String =", 18))
	{
	  /* Function String */
#ifdef DEBUG_WORDFILE
	      g_print(__FILE__": %s(): Function String = ", __func__);
#endif
	  i += 18;
	  while ((buffer[i] != '\n') && (buffer[i] != '\"'))
	    i++;
	  if (buffer[i] == '\"')
	    {
	      j = ++i;
	      while ((buffer[j] != '\n') && (buffer[j] != '\"'))
		j++;
	      /* Here, if buffer[j] == '\n', it is considered as a '\"' */
	      Prefs.L[Lg].FunctionString = g_malloc(j - i + 1);
	      strncpy(Prefs.L[Lg].FunctionString, buffer + i, j - i);
	      Prefs.L[Lg].FunctionString[j - i] = '\0';
	      i = j;
	      /* NOTE_TODO : Support function string */
	      while (buffer[i] != '\n')
		i++;
	      i++;
#ifdef DEBUG_WORDFILE
	      g_print("\"%s\"\n", Prefs.L[Lg].FunctionString);
#endif
	    }
	  else
	    i++;
	}
      else if ((buffer[i] == ' ') || (buffer[i] == '\t'))
	i++;
      else
	{
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): Unregonized string between /Lx "
		  "and /Cy: \"%c%c%c%c...\"\n", __func__,
		  buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
#endif
	  i++;
	}
    } /* while (strncmp(buffer + i, "/C", 2)) */
  while ((i + 1 <= size) && strncmp(buffer + i, "/L", 2))
    {
      if (!strncmp(buffer + i, "/C", 2))
	{
	  if (parse_color_section(buffer, size, &i, Lg))
	    {
	      g_print(__FILE__": %s(): End, parse error in a color "
		      "section of /L%i\n", __func__, Lg + 1);
	      return -1;
	    }
	}
      else
	i++;
    }
  *start = i;
  /*
  ** Now that all Prefs.L[Lg].C[_var_].Keywords are initialized, we can
  ** fill in the keywords hash tables
  */
#ifdef DEBUG_HASHTABLE
  g_print(__FILE__": %s(): Creating hash table for Lg %i\n", __func__, Lg);
#endif
  if (Prefs.L[Lg].IsCaseSensitive)
      Prefs.L[Lg].Hash = g_hash_table_new(g_str_hash, g_str_equal);
  else
      Prefs.L[Lg].Hash = g_hash_table_new(my_g_strcase_hash,
					  my_g_strcase_equal);
  for (i = 0; i < MAX_COL; i++)
      if (Prefs.L[Lg].C[i].Keywords)
	{
#ifdef DEBUG_HASHTABLE
	  g_print(__FILE__": %s(): Inserting keywords group %i\n", __func__, i);
#endif
	  j = 1;
	  while (Prefs.L[Lg].C[i].Keywords[j])
	    {
	      int	len = 0;

	      while (Prefs.L[Lg].C[i].Keywords[j + len] != ' ')
		  len++;
	      /*
	      ** Now, Keywords[j] is the 1st char of a word and len is its
	      ** length. We will insert it in the hash table. The value
	      ** associated to it will be the color plus 1 (i + 1).
	      */
	      Prefs.L[Lg].C[i].Keywords[j + len] = '\0';
#ifdef DEBUG_HASHTABLE
    	      g_print(__FILE__": %s(): Inserting keyword %s\n", __func__,
		      Prefs.L[Lg].C[i].Keywords + j);
#endif
	      g_hash_table_insert(Prefs.L[Lg].Hash,
		      g_strdup(Prefs.L[Lg].C[i].Keywords + j),
		      (gpointer)(i + 1));
	      Prefs.L[Lg].C[i].Keywords[j + len] = ' ';
	      j += len + 1;
	    }
	}
#ifdef DEBUG_HASHTABLE
  g_print(__FILE__": %s(): Hash table created for Lg %i\n", __func__, Lg);
#endif
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return 0;
}

/*
** Equivalent to the standard g_str_equal() function, but this one
** is case-independant
*/
gint		my_g_strcase_equal (gconstpointer v, gconstpointer v2)
{
  return strcasecmp ((const gchar*)v, (const gchar*)v2) == 0;
}

/*
** Equivalent to the standard g_str_hash() function, but this one
** is case-independant
*/
guint		my_g_strcase_hash (gconstpointer key)
{
  const char *p = key;
  guint h = *p;
  
  if (h)
    {
      if (h >= 'A' && h <= 'Z')
	  h |= 0x20;
      for (p += 1; *p != '\0'; p++)
	  h = (h << 5) - h + ((*p >= 'A' && *p <= 'Z') ? (*p | 0x20) : (*p));
    }
  return h;
}


/*
** Parse of color section in a UEdit wordfile
**
** Parameters:
**  buffer		Is a null-terminated string, representing the /Cx
**			section in a wordfile
**  size		Size of the buffer
**  start		buffer + *start points on "/Cx..."
**
** Return values:
**  0			No errors
**  -1			Parse error in a line
*/
gint			parse_color_section(gchar *buffer,
					    gint size,
					    gint *start,
					    gint Lg)
{
  gint			col;
  gint			i;
  gint			j;
  gchar			c;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  i = *start;
  i = i + 2;
  /* buffer[i] is the 1st digit of color number */
  j = i;
  while (isdigit(buffer[j]))
    j++;
  /* buffer[j] is the first char after the last digit of language number */
  c = buffer[j];
  buffer[j] = 0;
  col = atoi(buffer + i) - 1;
  buffer[j] = c;
  if (col + 1 > MAX_COL)
    {
      g_print(__FILE__": %s(): Warning, color %i section detected "
	      "but only %i are allowed\n", __func__, col + 1, MAX_COL);
      return -1;
    }
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): Color %i section detected\n", __func__, col + 1);
#endif
  i = j;
  if (buffer[i] == '\"' || buffer[i] == ' ')
    {
      /* There is a description of the color */
      j = ++i;
      while (buffer[j] != '\"' && buffer[j] != '\n')
	j++;
      /* buffer[i] is the 1st char of description and buffer[j] the 2nd '\"' */
      if ((j - i) > MAXLEN_COL_DESCRIPTION)
	{
	  g_print(__FILE__": %s(): Warning, description of color %i "
		  "of language %i is longer than %i\n", __func__, col + 1,
		  Lg + 1, MAXLEN_COL_DESCRIPTION);
	  return -1;
	}
      strncpy(Prefs.L[Lg].C[col].Description, buffer + i, j - i);
      Prefs.L[Lg].C[col].Description[j - i] = '\0';
      i = j;
    }
#ifdef DEBUG_WORDFILE
  g_print(__FILE__": %s(): Description of color %i = "
	  "\"%s\"\n", __func__, col + 1, Prefs.L[Lg].C[col].Description);
#endif
  while (buffer[i] != '\n')
    i++;
  j = ++i;
  /* Here, buffer[i] is the first char of keywords area */
  while ((i + 1 <= size) &&
	 strncmp(buffer + i, "/C", 2) &&
	 strncmp(buffer + i, "/L", 2))
    {
      if ((buffer[i] == '/') && (buffer[i + 1] != '/'))
	{
	  /* NOTE_TODO: Take into account this case (execute the 'command') */
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): In /L%i /C%i, command ignored\n", __func__,
		  Lg + 1, col + 1);
#endif
	  while (buffer[i] != '\n')
	    i++;
	  i++;
	}
      else  if (!strncmp(buffer + i, "**", 2))
	{
	  /* NOTE_TODO: Take into account this case (colorize all words
	     beginning with following prefixes) */
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): In /L%i, '**' ignored\n", __func__,
		  Lg + 1);
#endif
	  while (buffer[i] != '\n')
	    i++;
	  i++;
	}
      else
	{
	  if ((buffer[i] == '/') && (buffer[i + 1] == '/'))
	    {
#ifdef DEBUG_WORDFILE
	      g_print(__FILE__": %s(): In /L%i, '//' detected\n", __func__,
		      Lg + 1);
#endif
	      i += 2;
	    }
	  if (parse_line_of_keywords(buffer, size, &i, Lg, col))
	    {
	      g_print(__FILE__": %s(): End, parse error in a line of "
		      "keywords of /L%i /C%i\n", __func__, Lg + 1, col + 1);
	      return -1;
	    }
	}
    }
  *start = i;
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return 0;
}

/*
** Parse 1 line of keywords in a UEdit wordfile
**
** Parameters:
**  buffer		Is a null-terminated string, representing the /Cx
**			section in a wordfile
**  size		Size of the buffer
**  start		buffer + *start points on "/Cx..."
**
** Return values:
**  0			No errors
**  -1			Error while parsing line
*/
gint			parse_line_of_keywords(gchar *buffer,
					       gint size,
					       gint *start,
					       gint Lg,
					       gint col)
{
  gint			i;
  gint			j;
  gchar			c;
  gint			strlen_k;
  gint			strlen_b;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  (void)size;
  i = *start;
  while ((buffer[i] != '\r') && (buffer[i] != '\n'))
    {
      if ((buffer[i] == ' ') || (buffer[i] == '\t'))
	  i++;
      else
	{
	  /* buffer[i] is the first char of the keyword */
	  j = i;
	  while ((buffer[j] != ' ') && (buffer[j] != '\t') &&
		 (buffer[j] != '\r') && (buffer[j] != '\n'))
	    j++;
	  /* buffer[j] is the first char after the last one of the keyword */
	  c = buffer[j];
	  buffer[j] = '\0';
#ifdef DEBUG_WORDFILE
	  g_print(__FILE__": %s(): In /L%i /C%i, keyword \"%s\" "
		  "detected\n", __func__, Lg + 1, col + 1, buffer + i);
#endif
	  if (Prefs.L[Lg].C[col].Keywords == NULL)
	    {
#ifdef DEBUG_WORDFILE
	      g_print(__FILE__": %s(): Ah ! Keywords == NULL\n", __func__);
#endif
	      if (!(Prefs.L[Lg].C[col].Keywords = g_malloc(2)))
		{
		  g_print(__FILE__": %s(): End, g_malloc() failed\n", __func__);
		  return -1;
		}
	      Prefs.L[Lg].C[col].Keywords[0] = ' ';
	      Prefs.L[Lg].C[col].Keywords[1] = '\0';
	    }
	  strlen_k = strlen(Prefs.L[Lg].C[col].Keywords);
	  strlen_b = strlen(buffer + i);
	  Prefs.L[Lg].C[col].Keywords =
	    g_realloc(Prefs.L[Lg].C[col].Keywords, strlen_k + strlen_b + 2);
	  if (Prefs.L[Lg].C[col].Keywords == NULL)
	    {
	      g_print(__FILE__": %s(): End, g_realloc() failed\n", __func__);
	      return -1;
	    }
	  strcat(Prefs.L[Lg].C[col].Keywords, buffer + i);
	  strcat(Prefs.L[Lg].C[col].Keywords, " ");
	  buffer[j] = c;
	  i = j;
	} /* !((buffer[i] == ' ') || (buffer[i] == '\t')) */
    }
  if (buffer[i] == '\r')
    i++;
  *start = i + 1;
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return 0;
}

/*
** Callback fcn for insert_text signal
**
** Parameters :
**  See a gtk reference manual
**
** Return values :
**  TRUE		Always
**  FALSE		Never
**
** NOTE_REMARK : the signal is generated only by the keyboard (not by
**  gtk_text_insert() fcn but by gtk_editable_insert_text) and before the
**  insertion of text in the widget
*/
gboolean		text_has_been_inserted(GtkEditable *Editor,
					       const gchar *Text,
					       gint length,
					       gint *position,
					       gpointer data)
{
  t_action		*action;
  int			Lg;
  int			CurrentPage;
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  /* NOTE_WARNING: String pointed to by Text is not nul-terminated !*/
  (void)data;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  Lg = FPROPS(CurrentPage, WidgetInfo.Lg);
  /* NOTE_WARNING: Without affecting *position in the next line of
     code, there is a big bug: selecting text with mouse, and then
     pressing a key would delete selected text, and then would insert
     the key pressed at the end of text ! (there is not this problem
     when something is pasted instead of hitting a key).
    
     This is a very obscur bug that need to be corrected. If things
     are leaved as they are, we can't insert text with
     gtk_editable_insert_text() (which is used in the undo/redo code)
     in another position that the current cursor position.  Hopefully,
     there is a (temporary) workaround: move the cursor to the desired
     position before inserting text. */
  *position = refresh_from = gtk_editable_get_position(Editor);

  /* refresh_from is the index where Text will be inserted. Now
     checking for auto-correction or auto-indentation */
  if (Lg >= 0 && length == 1)
    {
      /* If AutoCorrection and if the char inserted is a Delimiter,
	 word_to_correct should be the index of the word to auto-correct.
	 But here, it points only on the first char after the word to be
	 corrected, final computing of its right value will be done in
	 refresh_syhi() */
      if (Prefs.AutoCorrection &&
	  Prefs.L[Lg].Delimiters && strchr(Prefs.L[Lg].Delimiters, *Text))
	word_to_correct = refresh_from;
      if (*Text == '\n')
	enter_pressed = TRUE;
    }
  action = malloc(sizeof (t_action));
  action->type = insert;
  action->start = refresh_from;
  action->end = refresh_from + length;
  action->text = malloc(length + 1);
  strncpy(action->text, Text, length);
  action->text[length] = 0;
  record_action(action);
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return TRUE;
}

/*
** Callback fcn for delete_text signal
**
** Parameters :
**  See a gtk reference manual
**
** Return values :
**  TRUE		Always
**  FALSE		Never
**
** NOTE_REMARK : the signal is generated only by the keyboard (not by
** gtk_text_*ward_delete() fcn. NOTE_WARNING : but by gtk_editable_delete_text)
** and before the deletion of text in the widget
*/
gboolean		text_has_been_deleted(GtkEditable *Editor,
					      gint start,
					      gint end,
					      gpointer data)
{
  t_action		*action;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  (void)Editor;
  (void)end;
  (void)data;
  refresh_from = start;
  action = malloc(sizeof *action);
  action->type = delete;
  action->start = start;
  action->end = end;
  action->text = gtk_editable_get_chars(Editor, start, end);
  record_action(action);
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return TRUE;
}

/*
** Callback fcn for changed signal. It refresh syntax highlight from
** char in editor indexed by refresh_from
**
** Parameters :
**  See a gtk reference manual
**
** Return values :
**  TRUE		Always
**  FALSE		Never
*/
gboolean		refresh_syhi(GtkEditable *Editor, gpointer data)
{
  gchar			*buffer;
  gint			buflen;
  t_char_state		refresh_from_state;
  glong			i, j;
  guint			CursorPosition;
  gint			Lg;
  gint			CurrentPage;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  (void)data;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  Lg = FPROPS(CurrentPage, WidgetInfo.Lg);
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): Language of file modified: /L%i \"%s\"\n", __func__,
	  Lg + 1, Prefs.L[Lg].Description);
#endif
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
			    FPROPS(CurrentPage, WidgetInfo.SigConnectIns[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
			    FPROPS(CurrentPage, WidgetInfo.SigConnectDel[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
			    FPROPS(CurrentPage, WidgetInfo.SigConnectChg[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]) = 0;
    }
  if (FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]))
    {
      gtk_signal_disconnect(GTK_OBJECT(Editor),
			    FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[1]));
      FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]) = 0;
    }
#ifdef DEBUG_FREEZE_THAW
  g_print(__FILE__": %s(): gtk_text_freeze()...\n", __func__);
#endif
  gtk_text_freeze(GTK_TEXT(Editor));
  /* Here is beginning the real work... */
  buffer = gtk_editable_get_chars(GTK_EDITABLE(Editor), 0, -1);
  buflen = strlen(buffer);
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): Length of buffer = %i\n", __func__, buflen);
#endif
  /* Finishing computing the good value for refresh_from */
  if (enter_pressed)
    refresh_from--;
  refresh_from = (refresh_from > 0) ? refresh_from - 1 : 0;
  while ((refresh_from > 0) && (buffer[refresh_from] != '\n'))
    refresh_from--;
  if (buffer[refresh_from] == '\n')
    refresh_from++;
  /* Here, refresh_from points to the first character of the line
     needed to be syhi-updated (where chars have been inserted or deleted).
     Or if RET has been pressed, refresh_from points on the new line */
  /* Now, Final computing of the right value for word_to_correct
     (Beginning in text_has_been_inserted())
     NOTE_REMINDER: word_to_correct has taken the value of cursor position
     before insertion of the delimiter */
#ifdef DEBUG_CORRECT
  g_print(__FILE__": %s(): "
	  "Before: word_to_correct = %i [%c][%c][%c]\n", __func__,
	  word_to_correct,
	  ((buffer[word_to_correct + 0] >= ' ') &&
	  (buffer[word_to_correct + 0] <= '~')) ?
	  buffer[word_to_correct + 0] : 0x7f,
	  ((buffer[word_to_correct + 1] >= ' ') &&
	  (buffer[word_to_correct + 1] <= '~')) ?
	  buffer[word_to_correct + 1] : 0x7f,
	  ((buffer[word_to_correct + 2] >= ' ') &&
	  (buffer[word_to_correct + 2] <= '~')) ?
	  buffer[word_to_correct + 2] : 0x7f);
#endif
  word_to_correct--; /* Points on the last char of supposed word to correct */
  if ((word_to_correct > -1) &&
      !strchr(Prefs.L[Lg].Delimiters, buffer[word_to_correct]))
    {
      /* If the char just before the Delimiters inserted is not
	 another Delimiter (so is the end of the word to correct) */
      while ((word_to_correct >= 0) &&
	     !strchr(Prefs.L[Lg].Delimiters, buffer[word_to_correct]))
	word_to_correct--;
      word_to_correct++;
      /* Here, word_to_auto_correct points on the 1st char of
	 the word to auto-correct */
    }
  else
    word_to_correct = -1;
#ifdef DEBUG_CORRECT
  g_print(__FILE__": %s(): "
	  "After : word_to_correct = %i [%c][%c][%c]\n", __func__,
	  word_to_correct,
	  ((buffer[word_to_correct + 0] >= ' ') &&
	  (buffer[word_to_correct + 0] <= '~')) ?
	  buffer[word_to_correct + 0] : 0x7f,
	  ((buffer[word_to_correct + 1] >= ' ') &&
	  (buffer[word_to_correct + 1] <= '~')) ?
	  buffer[word_to_correct + 1] : 0x7f,
	  ((buffer[word_to_correct + 2] >= ' ') &&
	  (buffer[word_to_correct + 2] <= '~')) ?
	  buffer[word_to_correct + 2] : 0x7f);
#endif
  i = 0;
  refresh_from_state = Normal;
  /* Now, computing refresh_from_state */
  while (i < refresh_from)
    {
      /*** Test for LineComment */
      if ((Prefs.L[Lg].LineComment != NULL) &&
	  !strncmp(buffer + i, Prefs.L[Lg].LineComment,
		   strlen(Prefs.L[Lg].LineComment)))
	{
	  /* NOTE_WARNING: I suppose there is no '\n' in LineComment */
	  while ((i < refresh_from) && (buffer[i] != '\n'))
	    i++;
	  i++;
	  refresh_from_state = Normal;
	}
      /*** Test for LineCommentAlt */
      else if ((Prefs.L[Lg].LineCommentAlt != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].LineCommentAlt,
			strlen(Prefs.L[Lg].LineCommentAlt)))
	{
	  /* NOTE_WARNING: I suppose there is no '\n' in LineComment */
	  while ((i < refresh_from) && (buffer[i] != '\n'))
	    i++;
	  i++;
	  refresh_from_state = Normal;
	}
      /*** Test for BlockCommentOn */
      else if ((Prefs.L[Lg].BlockCommentOn != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].BlockCommentOn,
			strlen(Prefs.L[Lg].BlockCommentOn)))
	{
	  if (Prefs.L[Lg].BlockCommentOff == NULL)
	    {
	      /* NOTE_WARNING: I suppose that BlockCommentOn does not contain
		 '\n' */
	      while ((i < refresh_from) && (buffer[i] != '\n'))
		i++;
	      i++;
	      refresh_from_state = Normal;
	    }
	  else if ((glong)NULL ==
		   (j = (glong)strstr(buffer + i +
				     strlen(Prefs.L[Lg].BlockCommentOn),
				     Prefs.L[Lg].BlockCommentOff)))
	    {
	      i = refresh_from;
	      refresh_from_state = Comment;
	    }
	  else
	    {
	      i = j + strlen(Prefs.L[Lg].BlockCommentOff) - (glong)buffer;
	      if (i <= refresh_from)
		refresh_from_state = Normal;
	      else
		refresh_from_state = Comment;
	    }
	}
      /*** Test for BlockCommentOnAlt */
      else if ((Prefs.L[Lg].BlockCommentOnAlt != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].BlockCommentOnAlt,
			strlen(Prefs.L[Lg].BlockCommentOnAlt)))
	{
	  if (Prefs.L[Lg].BlockCommentOffAlt == NULL)
	    {
	      /* NOTE_WARNING: I suppose that BlockCommentOnAlt does not
		 contain '\n' */
	      while ((i < refresh_from) && (buffer[i] != '\n'))
		i++;
	      i++;
	      refresh_from_state = Normal;
	    }
	  else if ((glong)NULL == 
		   (j = (glong)strstr(buffer + i +
				     strlen(Prefs.L[Lg].BlockCommentOnAlt),
				     Prefs.L[Lg].BlockCommentOffAlt)))
	    {
	      i = refresh_from;
	      refresh_from_state = CommentAlt;
	    }
	  else
	    {
	      i = j + strlen(Prefs.L[Lg].BlockCommentOffAlt) - (glong)buffer;
	      if (i <= refresh_from)
		refresh_from_state = Normal;
	      else
		refresh_from_state = CommentAlt;
	    }
	}
      /*** Test for StringChar0 */
      else if (Prefs.L[Lg].HaveString &&
	       (Prefs.L[Lg].StringChar0 != '\0') &&
	       (buffer[i] == Prefs.L[Lg].StringChar0))
	{
	  i++;
	  while ((i < refresh_from) && (buffer[i] != Prefs.L[Lg].StringChar0))
	    {
	      if (buffer[i] == Prefs.L[Lg].EscapeChar)
		i += 2;
	      else
		i++;
	    }
	  /* Here, i < refresh_from and points to the 2nd (closing) StringChar0
	     or i >= refresh_from: there is no 2nd (closing) StringChar0 until
	     i or refresh_from points to the 2nd (closing) StringChar0 */
	  if (i < refresh_from)
	    {
	      i++;
	      refresh_from_state = Normal;
	    }
	  else
	    {
	      i = refresh_from;
	      refresh_from_state = String0;
	    }
	}
      /*** Test for StringChar1 */
      else if (Prefs.L[Lg].HaveString &&
	       (Prefs.L[Lg].StringChar1 != '\0') &&
	       (buffer[i] == Prefs.L[Lg].StringChar1))
	{
	  i++;
	  while ((i < refresh_from) && (buffer[i] != Prefs.L[Lg].StringChar1))
	    {
	      if (buffer[i] == Prefs.L[Lg].EscapeChar)
		i += 2;
	      else
		i++;
	    }
	  /* Here, i < refresh_from and points to the 2nd (closing) StringChar1
	     or i >= refresh_from: there is no 2nd (closing) StringChar1 until
	     i or refresh_from points to the 2nd (closing) StringChar1 */
	  if (i < refresh_from)
	    {
	      i++;
	      refresh_from_state = Normal;
	    }
	  else
	    {
	      i = refresh_from;
	      refresh_from_state = String1;
	    }
	}
      /*** All tests failed */
      else
	{
	  i++;
	}
    } //while (i < refresh_from)
  /* Now, refresh_from_state is the state of the character pointed by buffer +
     refresh_from */
  /* NOTE_WARNING : this do not work !!!: CursorPosition =
     gtk_text_get_point(GTK_TEXT(Editor)); */
  CursorPosition = gtk_editable_get_position(GTK_EDITABLE(Editor));
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): CursorPosition = %i, deleting last chars "
	  "(from %i to the end)\n", __func__, CursorPosition, refresh_from);
#endif
  i = refresh_from;
  gtk_editable_delete_text(GTK_EDITABLE(Editor), refresh_from, -1);
  /* Now, we have to insert the text from buffer + i
     (at state refresh_from_state) to first char which state is Normal */
  /*** refresh_from is in Comment */
  if (refresh_from_state == Comment)
    {
      if (Prefs.L[Lg].BlockCommentOff == NULL)
	{
	  if ((j = (glong)strstr(buffer + i, "\n")) != (gint)NULL)
	    {
	      j = j + 1 - (glong)buffer;
	      gtk_text_insert(GTK_TEXT(Editor), NULL,
			      &(Prefs.Colors.L[Lg].Comment), NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor), NULL,
			      &(Prefs.Colors.L[Lg].Comment), NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
      else
	{
	  if ((j = (glong)strstr(buffer + i, Prefs.L[Lg].BlockCommentOff)) !=
	      (glong)NULL)
	    {
	      j = j + strlen(Prefs.L[Lg].BlockCommentOff) - (glong)buffer;
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL, &(Prefs.Colors.L[Lg].Comment),
			      NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL, &(Prefs.Colors.L[Lg].Comment),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
    }
  /*** refresh_from is in CommentAlt */
  else if (refresh_from_state == CommentAlt)
    {
      if (Prefs.L[Lg].BlockCommentOffAlt == NULL)
	{
	  if ((j = (glong)strstr(buffer + i, "\n")) != (glong)NULL)
	    {
	      j = j + 1 - (glong)buffer;
	      gtk_text_insert(GTK_TEXT(Editor), NULL,
			      &(Prefs.Colors.L[Lg].CommentAlt), NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL, &(Prefs.Colors.L[Lg].CommentAlt),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
      else
	{
	  if ((j = (glong)strstr(buffer + i, Prefs.L[Lg].BlockCommentOffAlt)) !=
	      (glong)NULL)
	    {
	      j = j + strlen(Prefs.L[Lg].BlockCommentOffAlt) - (glong)buffer;
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL, &(Prefs.Colors.L[Lg].CommentAlt),
			      NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL, &(Prefs.Colors.L[Lg].CommentAlt),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
    }
  /*** refresh_from is in String0 */
  else if (Prefs.L[Lg].HaveString && (refresh_from_state == String0))
    {
      j = i;
      while ((j < buflen) && (buffer[j] != Prefs.L[Lg].StringChar0))
	{
	  if (buffer[j] == Prefs.L[Lg].EscapeChar)
	    j += 2;
	  else
	    j++;
	}
      if (j >= buflen)
	{
	  gtk_text_insert(GTK_TEXT(Editor),
			  NULL, &(Prefs.Colors.L[Lg].String0), NULL,
			  buffer + i, -1);
	  i = buflen;
	}
      else
	{
	  gtk_text_insert(GTK_TEXT(Editor),
			  NULL, &(Prefs.Colors.L[Lg].String0), NULL,
			  buffer + i, j + 1 - i);
	  i = j + 1;
	}
    }
  /*** refresh_from is in String1 */
  else if (Prefs.L[Lg].HaveString && (refresh_from_state == String1))
    {
      j = i;
      while ((j < buflen) && (buffer[j] != Prefs.L[Lg].StringChar1))
	{
	  if (buffer[j] == Prefs.L[Lg].EscapeChar)
	    j += 2;
	  else
	    j++;
	}
      if (j >= buflen)
	{
	  gtk_text_insert(GTK_TEXT(Editor),
			  NULL, &(Prefs.Colors.L[Lg].String1), NULL,
			  buffer + i, -1);
	  i = buflen;
	}
      else
	{
	  gtk_text_insert(GTK_TEXT(Editor),
			  NULL, &(Prefs.Colors.L[Lg].String1), NULL,
			  buffer + i, j + 1 - i);
	  i = j + 1;
	}
    }
  /* Now, buffer + i points on '\0' (all have been inserted), or it points on
     a char, which state is "Normal"
     We have to continue to fill in the widget */
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): Text between refresh_from(%i) and 1st "
	  "state Normal (index %i) has been inserted\n", __func__,
	  refresh_from, i);
  g_print(__FILE__": %s(): "
	  "while (i < buflen && i < (gint)CursorPosition + Prefs.SyhiDepth)\n",
	  __func__);
  g_print(__FILE__": %s(): while (%i < %i && %i < %i + %i)\n", __func__,
	  i, buflen, i, (gint)CursorPosition, Prefs.SyhiDepth);
#endif
  /* Old: while (i < buflen) */
  while (i < buflen && i < (long long)CursorPosition + Prefs.SyhiDepth)
    {
      /*** Detection of BlockCommentOn */
      if ((Prefs.L[Lg].BlockCommentOn != NULL) &&
	  !strncmp(buffer + i, Prefs.L[Lg].BlockCommentOn,
		   strlen(Prefs.L[Lg].BlockCommentOn)))
	{
	  if (Prefs.L[Lg].BlockCommentOff == NULL)
	    {
	      if ((j = (glong)strstr(buffer + i, "\n")) != (glong)NULL)
		{
		  j = j + 1 - (glong)buffer;
		  gtk_text_insert(GTK_TEXT(Editor), NULL,
				  &(Prefs.Colors.L[Lg].Comment), NULL,
				  buffer + i, j - i);
		  i = j;
		}
	      else
		{
		  gtk_text_insert(GTK_TEXT(Editor),
				  NULL, &(Prefs.Colors.L[Lg].Comment),
				  NULL,
				  buffer + i, -1);
		  i = buflen;
		}
	    }
	  else
	    {
	      if ((glong)NULL !=
		  (j = (glong)strstr(buffer + i +
				    strlen(Prefs.L[Lg].BlockCommentOn),
				    Prefs.L[Lg].BlockCommentOff)))
		{
		  j = j + strlen(Prefs.L[Lg].BlockCommentOff) - (glong)buffer;
		  gtk_text_insert(GTK_TEXT(Editor),
				  NULL, &(Prefs.Colors.L[Lg].Comment),
				  NULL,
				  buffer + i, j - i);
		  i = j;
		}
	      else
		{
		  gtk_text_insert(GTK_TEXT(Editor),
				  NULL, &(Prefs.Colors.L[Lg].Comment),
				  NULL,
				  buffer + i, -1);
		  i = buflen;
		}
	    }
	}
      /*** Detection of BlockCommentOnAlt */
      else if ((Prefs.L[Lg].BlockCommentOnAlt != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].BlockCommentOnAlt,
			strlen(Prefs.L[Lg].BlockCommentOnAlt)))
	{
	  if (Prefs.L[Lg].BlockCommentOffAlt == NULL)
	    {
	      if ((j = (glong)strstr(buffer + i, "\n")) != (glong)NULL)
		{
		  j = j + 1 - (glong)buffer;
		  gtk_text_insert(GTK_TEXT(Editor), NULL,
				  &(Prefs.Colors.L[Lg].CommentAlt), NULL,
				  buffer + i, j - i);
		  i = j;
		}
	      else
		{
		  gtk_text_insert(GTK_TEXT(Editor),
				  NULL,
				  &(Prefs.Colors.L[Lg].CommentAlt),
				  NULL,
				  buffer + i, -1);
		  i = buflen;
		}
	    }
	  else
	    {
	      if ((glong)NULL !=
		  (j = (glong)strstr(buffer + i,
				    Prefs.L[Lg].BlockCommentOffAlt)))
		{
		  j = j + strlen(Prefs.L[Lg].BlockCommentOffAlt) -
		    (glong)buffer;
		  gtk_text_insert(GTK_TEXT(Editor), NULL,
				  &(Prefs.Colors.L[Lg].CommentAlt), NULL,
				  buffer + i, j - i);
		  i = j;
		}
	      else
		{
		  gtk_text_insert(GTK_TEXT(Editor),
				  NULL,
				  &(Prefs.Colors.L[Lg].CommentAlt),
				  NULL,
				  buffer + i, -1);
		  i = buflen;
		}
	    }
	}
      /*** Detection of String0 */
      else if (Prefs.L[Lg].HaveString && 
	       (Prefs.L[Lg].StringChar0 != '\0') &&
	       (buffer[i] == Prefs.L[Lg].StringChar0))
	{
	  j = i + 1;
	  while ((j < buflen) && (buffer[j] != Prefs.L[Lg].StringChar0))
	    {
	      if (buffer[j] == Prefs.L[Lg].EscapeChar)
		j += 2;
	      else
		j++;
	    }
	  if (j >= buflen)
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].String0),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].String0),
			      NULL,
			      buffer + i, j + 1 - i);
	      i = j + 1;
	    }
	}
      /*** Detection of String1 */
      else if (Prefs.L[Lg].HaveString && 
	       (Prefs.L[Lg].StringChar1 != '\0') &&
	       (buffer[i] == Prefs.L[Lg].StringChar1))
	{
	  j = i + 1;
	  while ((j < buflen) && (buffer[j] != Prefs.L[Lg].StringChar1))
	    {
	      if (buffer[j] == Prefs.L[Lg].EscapeChar)
		j += 2;
	      else
		j++;
	    }
	  if (j >= buflen)
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].String1),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].String1),
			      NULL,
			      buffer + i, j + 1 - i);
	      i = j + 1;
	    }
	}
      /*** Detection of LineComment */
      else if ((Prefs.L[Lg].LineComment != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].LineComment,
			strlen(Prefs.L[Lg].LineComment)))
	{
	  j = i + 1;
	  while ((j < buflen) && (buffer[j] != '\n'))
	      j++;
	  if (j < buflen)
	    {
	      j++;
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].Comment),
			      NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].Comment),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
      /*** Detection of LineCommentAlt */
      else if ((Prefs.L[Lg].LineCommentAlt != NULL) &&
	       !strncmp(buffer + i, Prefs.L[Lg].LineCommentAlt,
			strlen(Prefs.L[Lg].LineCommentAlt)))
	{
	  j = i + 1;
	  while ((j < buflen) && (buffer[j] != '\n'))
	      j++;
	  if (j < buflen)
	    {
	      j++;
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].CommentAlt),
			      NULL,
			      buffer + i, j - i);
	      i = j;
	    }
	  else
	    {
	      gtk_text_insert(GTK_TEXT(Editor),
			      NULL,
			      &(Prefs.Colors.L[Lg].CommentAlt),
			      NULL,
			      buffer + i, -1);
	      i = buflen;
	    }
	}
      /*** Detection of Number */
      else if (isdigit(buffer[i]))
	{
	  j = strcspn(buffer + i, Prefs.L[Lg].Delimiters);
	  gtk_text_insert(GTK_TEXT(Editor),
			  NULL, &(Prefs.Colors.L[Lg].Number), NULL,
			  buffer + i, j);
	  i += j;
	}
      /*** Detection of: Keywords or Delimiters or nothing */
      else
	{
	  t_match	match;

	  /* Detection of Keywords or Delimiters */
	  if ((word_to_correct == i) &&
	      Prefs.L[Lg].IsCaseSensitive && Prefs.AutoCorrection )
	    matching_keyword_case_ext(Lg, buffer + i, &match);
	  else
	    matching_keyword(Lg, buffer + i, &match);
	  /* If length is != 0, a matching keyword has been found */
	  if (match.length)
	    {
#ifdef DEBUG_SYHI
	      g_print(__FILE__": %s(): Insertion of Keyword or "
		      "Delimiters reconized, i = %li\n", __func__, i);
#endif
	      if (Prefs.L[Lg].IsCaseSensitive && Prefs.AutoCorrection &&
		  (word_to_correct == i))
		gtk_text_insert(GTK_TEXT(Editor), NULL, &(Prefs.Colors.L[Lg]\
		.C[match.color]), NULL, match.keyword, match.length),
		word_to_correct = -1;
	      else
		gtk_text_insert(GTK_TEXT(Editor), NULL, &(Prefs.Colors.L[Lg]\
		.C[match.color]), NULL, buffer + i, match.length);
	      i += match.length;
	    }
	  /* If length is 0, nothing has been detected */
	  else
	    {
#ifdef DEBUG_SYHI
	      g_print(__FILE__": %s(): Insertion of unknown string, "
		      "i = %li\n", __func__, i);
#endif
	      /*
	      ** NOTE_CHANGE: Changed on 07-oct-2000
	      ** if ((j = strcspn(buffer + i, Prefs.L[Lg].Delimiters)) == 0)
	      **  j = 1;
	      */
	      j = 0;
	      while (!Prefs.L[Lg].IsADelimiter[(guchar)buffer[i + j]])
		j++;
	      j = (j) ? (j) : (1);
	      /*
	      ** NOTE_CHANGE: End
	      */
	      gtk_text_insert(GTK_TEXT(Editor), NULL,NULL,NULL, buffer + i, j);
	      i += j;
	    }
	}
    } //while (i < buflen && ...)
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): "
	  "while (i < buflen && i < (gint)CursorPosition + Prefs.SyhiDepth)\n",
	  __func__);
  g_print(__FILE__": %s(): while (%i < %i && %i < %i + %i)\n", __func__,
	  i, buflen, i, (gint)CursorPosition, Prefs.SyhiDepth);
#endif
  if (i < buflen)
      /* So we have reached and/or gone beyond CursorPosition +
	 Prefs.SyhiUpdate */
      gtk_text_insert(GTK_TEXT(Editor), NULL,
		      NULL, NULL,
		      buffer + i, -1);
#ifdef DEBUG_AUTO_INDENT
  if (Prefs.AutoIndentation)
      g_print(__FILE__": %s(): * AutoIndentation activated\n", __func__);
  else
      g_print(__FILE__": %s(): * AutoIndentation desactivated\n", __func__);
  g_print(__FILE__": %s(): refresh_from = %i\n", __func__, refresh_from);
#endif
  /* If a new line has just been inserted, proceed for auto-indentation */
  if (Prefs.AutoIndentation && enter_pressed)
    {
      gboolean		indent;
      gboolean		unindent;
      gint		first;
      gint		first_nospc;
      gint		last_nospc;

#ifdef DEBUG_AUTO_INDENT
      g_print(__FILE__": %s(): AutoIndentation starting\n", __func__);
#endif
      enter_pressed = FALSE;
      /* NOTE_REMINDER: buffer[refresh_from] is the first char of
	 the previous line */
      first = refresh_from;
      /* Here buffer[first] is the first char of previous line */
#ifdef DEBUG_AUTO_INDENT
      g_print(__FILE__": %s(): buffer[first]: [%c,%c,%c,%c]\n", __func__,
	      PRINTC(buffer[first + 0]),
	      PRINTC(buffer[first + 1]),
	      PRINTC(buffer[first + 2]),
	      PRINTC(buffer[first + 3]));
#endif
      first_nospc = first; 
      while ((buffer[first_nospc] == ' ') || (buffer[first_nospc] == '\t'))
	first_nospc++;
      /* Here buffer[first_nospc] is the 1st non-space char of previous line */
#ifdef DEBUG_AUTO_INDENT
      g_print(__FILE__": %s(): buffer[first_nospc]: [%c,%c,%c,%c]\n", __func__,
	      PRINTC(buffer[first_nospc + 0]),
	      PRINTC(buffer[first_nospc + 1]),
	      PRINTC(buffer[first_nospc + 2]),
	      PRINTC(buffer[first_nospc + 3]));
#endif
      last_nospc = (glong)strchr(buffer + first_nospc, '\n') - 1 - (glong)buffer;
      while ((last_nospc > first_nospc) &&
	     ((buffer[last_nospc] == ' ') || (buffer[last_nospc] == '\t')))
	last_nospc--;
      last_nospc = (last_nospc < first_nospc) ? first_nospc : last_nospc;
      /* Here buffer[last_nospc] is the last non-space char of previous line */
#ifdef DEBUG_AUTO_INDENT
      g_print(__FILE__": %s(): buffer[last_nospc]: [%c,%c,%c,%c]\n", __func__,
	      PRINTC(buffer[last_nospc + 0]),
	      PRINTC(buffer[last_nospc + 1]),
	      PRINTC(buffer[last_nospc + 2]),
	      PRINTC(buffer[last_nospc + 3]));
#endif
      indent = unindent = FALSE;
      if (Prefs.L[Lg].UnindentString &&
	  (last_nospc + 1 - first_nospc >=
	   (glong)strlen(Prefs.L[Lg].UnindentString)) &&
	  !strncmp(buffer + first_nospc, Prefs.L[Lg].UnindentString,
		   strlen(Prefs.L[Lg].UnindentString)))
	/*** If the first non-space chars of previous line are the
	     UnindentString... auto-unindentation */
	unindent = TRUE;
      if (Prefs.L[Lg].IndentString &&
	  (last_nospc + 1 - first_nospc >=
	   (glong)strlen(Prefs.L[Lg].IndentString)) &&
	  !strncmp(buffer + last_nospc + 1 - strlen(Prefs.L[Lg].IndentString),
		   Prefs.L[Lg].IndentString, strlen(Prefs.L[Lg].IndentString)))
	/*** If the last non-space chars of previous line are the
	     IndentString... auto-indentation */
	indent = TRUE;
      if (unindent)
	{
	  gint		indent_space;

#ifdef DEBUG_AUTO_INDENT
	  g_print(__FILE__": %s(): --- Unindentation\n", __func__);
#endif /*DEBUG_AUTO_INDENT*/
	  indent_space = 0;
	  for (j = first; j < first_nospc; j++)
	    if (buffer[j] == ' ')
	      indent_space++;
	    else
	      indent_space += Prefs.TabSize - (indent_space % Prefs.TabSize);
	  /* Here, indent_space is the size in chars of indentation of
	     previous line */
#ifdef DEBUG_AUTO_INDENT
	  g_print("Indentation space of previous line = %i\n", indent_space);
#endif /*DEBUG_AUTO_INDENT*/
	  if (indent_space > 0)
	    {
	      gint	indent_chars;
	      gint	indent_space_from_bol; /* bol = Beginning Of Line */

	      if (!(indent_space % Prefs.TabSize))
		indent_space -= Prefs.TabSize;
	      else
		indent_space -= indent_space % Prefs.TabSize;
#ifdef DEBUG_AUTO_INDENT
	      g_print("Indentation space required for previous & current "
		      "line = %i\n", indent_space);
#endif /*DEBUG_AUTO_INDENT*/
	      /* Here, indent_space is the size in spaces of indentation that
		 current line should have. Now, indent_chars should be the
		 size in reals chars (tab, space...) of indentation that
		 current line should have. */
	      indent_space_from_bol = 0;
	      for (i = first; indent_space > 0; i++)
		if (buffer[i] == ' ')
		  {
		    indent_space--;
		    indent_space_from_bol++;
		  }
		else
		  {
		    gint isfb_tmp; /* Indent Space From Bol TeMP */

		    isfb_tmp = indent_space_from_bol;
		    indent_space -= Prefs.TabSize - isfb_tmp % Prefs.TabSize;
		    indent_space_from_bol += isfb_tmp % Prefs.TabSize;
		  }
	      if (indent_space != 0)
		g_print(__FILE__": %s(): Fatal error, indent_space "
			"!= 0 (== %i)\nYou should save all your documents "
			"and quit Beaver immediately !\n", __func__,
			indent_space);
	      indent_chars = i - first;
	      /* the (indent_chars)th first chars of previous line should
		 be used to indent the new line */
#ifdef DEBUG_AUTO_INDENT
	      g_print("Struct of indentation of prev. & act. lines: "
		      "[first=%i; first+indent_chars-1=%i] "
		      "(indent_chars = %i): \"",
		      first, first + indent_chars - 1, indent_chars);
	      for (i = first; i < first + indent_chars; i++)
		if (buffer[i] == ' ')
		  g_print(".");
		else
		  g_print("-");
	      g_print("\"\n");
	      g_print("first_nospc = %i; last_nospc = %i\n"
		      "refresh_from = %i =?= CursorPosition = %i\n",
		      first_nospc, last_nospc, refresh_from, CursorPosition);
	      g_print("Deleting [%i; %i] (%i chars)...\n",
		      first + indent_chars, first_nospc - 1,
		      first_nospc - (first + indent_chars));
#endif /*DEBUG_AUTO_INDENT*/
	      gtk_editable_delete_text(GTK_EDITABLE(Editor),
				       first + indent_chars,
				       first_nospc);
	      CursorPosition -= first_nospc - (first + indent_chars);
	      first_nospc -= first_nospc - (first + indent_chars);
#ifdef DEBUG_AUTO_INDENT
	      g_print("Now, CursorPosition = %i\n", CursorPosition);
#endif /*DEBUG_AUTO_INDENT*/
	      gtk_text_set_point(GTK_TEXT(Editor), CursorPosition);
	      if (indent_chars > 0)
		{
#ifdef DEBUG_AUTO_INDENT
		  g_print("New line had to be indented of %i chars\n",
			  indent_chars);
#endif /*DEBUG_AUTO_INDENT*/
		  gtk_text_insert(GTK_TEXT(Editor), NULL,
				  NULL, NULL,
				  buffer + first, indent_chars);
		  CursorPosition += indent_chars;
#ifdef DEBUG_AUTO_INDENT
		  g_print("Now, CursorPosition = %i\n", CursorPosition);
#endif /*DEBUG_AUTO_INDENT*/
		}
	    } /* if (indent_space > 0) */
	} /*** Unindentation */
      if (indent)
	{
#ifdef DEBUG_AUTO_INDENT
	  g_print(__FILE__": %s(): --- Indentation\n", __func__);
#endif /*DEBUG_AUTO_INDENT*/
	  indent = TRUE;
	  gtk_text_set_point(GTK_TEXT(Editor), CursorPosition);
#ifdef DEBUG_AUTO_INDENT
	  g_print("Now, CursorPosition = %i\n", CursorPosition);
#endif /*DEBUG_AUTO_INDENT*/
	  if (!unindent)
	    gtk_text_insert(GTK_TEXT(Editor), NULL,
			    NULL, NULL,
			    buffer + first, first_nospc - first);
	  gtk_text_insert(GTK_TEXT(Editor), NULL,
			  NULL, NULL,
			  "\t", 1);
	  if (!unindent)
	    CursorPosition += first_nospc - first + 1;
	  else
	    CursorPosition++;
	} /*** Indentation */
      if (!indent && !unindent)
	{
	  /*** Else keep same indentation as previous line */
#ifdef DEBUG_AUTO_INDENT
	  g_print(__FILE__": %s(): --- Same indentation\n", __func__);
#endif /*DEBUG_AUTO_INDENT*/
	  gtk_text_set_point(GTK_TEXT(Editor), CursorPosition);
	  gtk_text_insert(GTK_TEXT(Editor), NULL,
			  NULL, NULL,
			  buffer + first, first_nospc - first);
	  CursorPosition += first_nospc - first;
	} /*** Same indentation */
#ifdef DEBUG_SYHI
      g_print(__FILE__": %s(): End of AutoIndentation\n", __func__);
#endif
    }
  /*
  ** End of Auto indentation
  */
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): All text has been inserted, freeing...\n", __func__);
#endif
  g_free(buffer);
  /*
  ** gtk_text_thaw() must be called before gtk_editable_set_position(),
  ** if it is not the case, there is a segfault !
  */
#ifdef DEBUG_FREEZE_THAW
  g_print(__FILE__": %s(): gtk_text_thaw()...\n", __func__);
#endif
  gtk_text_thaw(GTK_TEXT(Editor));
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): Setting position...\n", __func__);
#endif
  /* I don't know why, but sometimes the gtk_text_set_point() does not work...
     gtk_text_set_point(GTK_TEXT(Editor), CursorPosition); */
  gtk_editable_set_position(GTK_EDITABLE(Editor), CursorPosition);
  /* Since refresh_syhi() is called only when some text is
     inserted/deleted, no text should be selected after the change
     (this is the normal behaviour in every text editor). But
     unfortunately, some text is sometimes selected (eg. when hitting
     shift+backspace), because the cursor is moved by the code whiled
     the shift key is pressed. That is why we disable the selection: */
  gtk_editable_select_region(GTK_EDITABLE(Editor),
			     CursorPosition, CursorPosition);
#ifdef DEBUG_SYHI
  g_print(__FILE__": %s(): Connecting_after...\n", __func__);
#endif
  FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[0]) = 1;
  FPROPS(CurrentPage, WidgetInfo.SigConnectChgAft[1]) =
    gtk_signal_connect_after(GTK_OBJECT(Editor), "changed",
			     (GtkSignalFunc)reconnect_sig, NULL);
  refresh_from = 0;
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return TRUE;
}

/*
** Callback fcn for changed signal ("connected_after" the refresh_syhi)
**
** Parameters :
**  See a gtk reference manual
**
** Return values :
**  TRUE		Always
**  FALSE		Never
*/

gboolean		reconnect_sig(GtkEditable *Editor, gpointer data)
{
  gint			CurrentPage;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  (void)data;

  CurrentPage = gtk_notebook_get_current_page (GTK_NOTEBOOK(MainNotebook));
  FPROPS(CurrentPage, WidgetInfo.SigConnectIns[0]) = 1;
  FPROPS(CurrentPage, WidgetInfo.SigConnectIns[1]) =
    gtk_signal_connect(GTK_OBJECT(Editor), "insert_text",
		       (GtkSignalFunc)text_has_been_inserted, NULL);
  FPROPS(CurrentPage, WidgetInfo.SigConnectDel[0]) = 1;
  FPROPS(CurrentPage, WidgetInfo.SigConnectDel[1]) =
    gtk_signal_connect(GTK_OBJECT(Editor), "delete_text",
		       (GtkSignalFunc)text_has_been_deleted, NULL);
  //NOTE_WARNING : the callback fcn refresh_syhi() needs the "changed"
  //signal to be produced after "delete_text" ans "insert_text" signals 
  FPROPS(CurrentPage, WidgetInfo.SigConnectChg[0]) = 1;
  FPROPS(CurrentPage, WidgetInfo.SigConnectChg[1]) =
    gtk_signal_connect(GTK_OBJECT(Editor), "changed",
		       (GtkSignalFunc)refresh_syhi, NULL);
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
  return TRUE;
}

/*
** This func finds the first keyword that match the string passed in
** arguments. A keyword must be followed by what is called a "delimiter"
** (some special rules apply to html).
**
** The .lenght and .color fields of the match structure are filled to
** indicate the result.
**
** Note: execution time of this function must be the shortest possible,
** because when some text is inserted/deleted (eg: a key is pressed), it
** is called thousand of times...
**
** Parameters :
**  Lg			The language of the string
**  str			The string this fcn try to match with a keyword
**  match		Structure representing the mathcing keyword filled
**			 by this function
**
** Return values :
**  void
*/
void			matching_keyword(gint Lg,
					 guchar *str,
					 t_match *match)
{
  guchar		backup;
  gpointer		found;
  gint			i;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  /*
  ** Objective: search the next delimiter or the end of string.
  ** Special case for html: since '<' and '>' are delimiters _and_ are
  ** part of keywords (this is one of the mysterious concepts in UEdit
  ** wordfiles), we ignore them in the "IsADelimiter" test and we always
  ** consider the char after '>' as being a delimiter
  */
  i = 0;
  if (Prefs.L[Lg].IsHTML)
      while ((str[i]) &&
	     (str[i]=='<' || str[i]=='>' || !Prefs.L[Lg].IsADelimiter[str[i]])
	     && ((i >= 1) ? (str[i - 1] != '>') : (1)))
	  i++;
  else
      while (str[i] && !Prefs.L[Lg].IsADelimiter[str[i]])
	  i++;
  /* check if the word pointed to by str is a keyword */
  backup = str[i]; str[i] = '\0';
  found = g_hash_table_lookup(Prefs.L[Lg].Hash, str);
  str[i] = backup;
  /* if yes, save the length of the keyword and its color, which (by
     convention) is the value associated to the string minus one
     (g_hash_table_lookup() returns either NULL or this value) */
  if (found)
    {
      match->length = i;
      match->color = (gint)found - 1;
    }
  /* else indicate that no match were found */
  else
      match->length = 0;
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
}

/*
** This func finds the longest keyword that match the beginning of the
** string passed in arguments, taking into account the value of IsHTML.
**
** It is said ``case extended'' because it fills 2 more fields of
**  match than matching_keyword(): .keyword and .good_case. This allows
**  to match some keywords even if they do not have the same case that
**  the keyword in the wordfile
**
**  This func returns its results in a struct s_match where:
**  - match.length is 0 if there are no keywords matching the string, or
**    match.lenght contains the length of match.keyword (a null-terminated
**    string matching the string, need to be freed ?). The case of this
**    match.keyword is always the one of the keyword written in the wordfile
**  - match.color is the number of the group color of the matching keyword
**
** Note: unlike matching_keyword(), matching_keyword_case_ext() speed
** is not critical since it is called only for the keyword under the cursor.
**
** Parameters :
**  Lg			The language of the string
**  str			The string this fcn try to match with a keyword
**  match		Pointer on the struct that will be filled in
**
** Return value :
**  void
*/
void			matching_keyword_case_ext(gint Lg,
						  const guchar *str,
						  t_match *match)
{
  gchar			*search_ptr;
  gchar			*tmp_ptr;
  gint			color;
  gint			len_search_ptr;
  gint			cmp;

#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): Begin\n", __func__);
#endif
  match->length = 0;
  for (color = 0; color < MAX_COL; color++)
    {
      if (Prefs.L[Lg].C[color].Keywords == NULL)
	continue;
      search_ptr = Prefs.L[Lg].C[color].Keywords + 1;
      /* NOTE_REMARK: Keywords are separated by spaces: " k1 k2 k3 " */
      while (search_ptr != NULL)
	{
	  tmp_ptr = strchr(search_ptr, ' ');
	  *tmp_ptr = '\0';
	  len_search_ptr = tmp_ptr - search_ptr;
	  if ((CHAR_CASE_CMP(*str, *search_ptr)) &&
	      (len_search_ptr > match->length) &&
	      (!(cmp = strncmp(str, search_ptr, len_search_ptr)) ||
	       !strncasecmp(str, search_ptr, len_search_ptr)) &&
	      (Prefs.L[Lg].IsADelimiter[str[len_search_ptr]] ||
	       (Prefs.L[Lg].IsHTML && str[len_search_ptr - 1] == '>')))
	    {
	      match->color = color;
	      match->length = len_search_ptr;
	      match->keyword = search_ptr;
	      match->good_case = !cmp ? TRUE : FALSE;
	    }
	  *tmp_ptr = ' ';
	  search_ptr = tmp_ptr + 1;
	  if (*search_ptr == '\0')
	    search_ptr = NULL;
	}
    }
  /*
  ** Here,
  **   - either match->length == 0
  **   - either match->length != 0 (in this case, match->keyword points to
  **     a matching keyword
  */
#ifdef DEBUG_FCN
  g_print(__FILE__": %s(): End\n", __func__);
#endif
}
