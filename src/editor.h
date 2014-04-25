/*
** Beaver's an Early AdVanced EditoR
** (C) 1999-2000 Marc Bevand, Damien Terrier and Emmanuel Turquin
**
** editor.h
**
** Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
** Last update: Mon Jul  8 08:22:44 CEST 2002
** Description: Beaver editing functions
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

/*
** NOTE_REMARK : syhi stands for SYntax HIghlighting, the main work to
**  be accomplished by the source
*/

/*
** TODO: handle the case where the compiler does not provide __func__
** (used in editor.c)
*/

/*
** Macro equivalent to the delta time "Tv2 - Tv1" in millisec
** NOTE_WARNING : needs sys/time.h because Tvx are struct timeval
*/
#define DELTA_TIME(Tv1, Tv2)						\
  ((double)((Tv2).tv_sec) * 1000.0 + (double)((Tv2).tv_usec) / 1000.0 - \
  (double)((Tv1).tv_sec) * 1000.0 - (double)((Tv1).tv_usec) / 1000.0)

/*
** Macro for recording time in the struct timeval Tv
** NOTE_WARNING : needs sys/time.h, unistd.h & stdio.h
*/
#define REC_TIME(Tv)							\
{									\
  if (gettimeofday(&(Tv), NULL))					\
    perror(__FILE__": REC_TIME() macro: gettimeofday() error");		\
}

/*
** Macro for filtering printable chars
*/
#define PRINTC(c)							\
  ((((c) >= ' ') && ((c) <= '~')) ? (c) : 0x7f)

/*
** Macro that return 1 if 2 letters are equivalent (ignoring the case), 0 else
*/
#define CHAR_CASE_CMP(C1, C2)						\
  ((C1 == C2) ||							\
   (char_alpha[(guchar)C1] && char_alpha[(guchar)C2] &&			\
    ((C1 | 0x20)  == (C2 | 0x20))))

/*
** Here is different limits (imposed by UEdit), but they are not very
** well respected: there exists some wordfiles that overtake these
** limits, that's why I have implemented a syntax highlighting system
** that can be easily reconfigured (by changing the values defined)
**
** Especially, the values of MAXLEN_LANG_DESCRIPTION and
** MAXLEN_COL_DESCRIPTION (which was originally specified to be 8 in the
** reference documentation provided by UEdit) have been changed to 128 in
** order to support lots of wordfiles which descriptions were often more
** than 8 chars long
*/
#define MAX_LANG		10
#define MAXLEN_LANG_DESCRIPTION	128
#define MAX_COL			8
#define MAXLEN_COL_DESCRIPTION	128
/* MAXLEN_LINE_COMMENT must be a 1-digit number */
#define MAXLEN_LINE_COMMENT	3
#define MAXLEN_BLOCK_COMMENT	5

/*
** Type of BeginningLine
*/
typedef enum		e_char_state
{
  Normal,
  Comment,
  CommentAlt,
  String0,
  String1
}			t_char_state;

/*
** Structure representing "preferences" such as visual aspect of
** text, keywords to be highlighted, UEdit wordfile, ...
*/
struct			s_Prefs
{
  /* Beginning of internal datas */
  GdkColormap		*ColMap;
  GdkFont		*TheFont;
  struct
  {
    struct
    {
      /* Description of this color + '\0' */
      gchar		Description[MAXLEN_COL_DESCRIPTION + 1];
      /*
      ** Keywords list: ' '+"keyword_0"+' '+...+' '+"keyword_n"+' '
      ** or NULL if not defined
      */
      gchar		*Keywords;
    }			C[MAX_COL];
    /* Keywords hash table */
    GHashTable		*Hash;
    gboolean		IsDefined;
    /* Description of this language + '\0' */
    gchar		Description[MAXLEN_LANG_DESCRIPTION + 1];
    /* ' '+list_of_extensions_separated_by_' '+' ' */
    gchar		*Extensions;
    gchar		*LineComment;
    gchar		*LineCommentAlt;
    gchar		*BlockCommentOn;
    gchar		*BlockCommentOff;
    gchar		*BlockCommentOnAlt;
    gchar		*BlockCommentOffAlt;
    gboolean		IsCaseSensitive;
    gboolean		HaveString;
    gchar		StringChar0;
    gchar		StringChar1;
    gchar		EscapeChar;
    gchar		*Delimiters;
    /* The following variable is used for matching_keyword() optimization */
    gchar		IsADelimiter[256];
    gchar		*IndentString;
    gchar		*UnindentString;
    gchar		*MarkerChars;
    gchar		*FunctionString;
    gboolean		IsHTML;
  }			L[MAX_LANG];
  /* End of internal datas */
  struct
  {
    struct
    {
      GdkColor		Comment;
      GdkColor		CommentAlt;
      GdkColor		String0;
      GdkColor		String1;
      GdkColor		Number;
      GdkColor		C[MAX_COL];
    }			L[MAX_LANG];
  }			Colors;
  /* Number of chars to syhi. In fact, syhi is the feature of Beaver that
     require the most of CPU time, so this variable is here to adapt the
     syhi depth in chars: when this limit is reached, syhi stops after
     the end of current block of chars of same color */
  gint			SyhiDepth;
  gboolean		AutoCorrection;
  gboolean		AutoIndentation;
  /*
  ** NOTE_TODO : In a GtkText, after the 2 first tabulations, the 3rd is only
  **  equivalent to 4 spaces. This causes indentation bugs when there are 
  **  spaces mixed with tabulations. To fix.
  */
  gint			TabSize;
}			Prefs;

/*
** Struct returned by matching_keyword()
*/
typedef struct		s_match
{
  gchar			*keyword;
  gint			color;
  //NOTE_REMARK: Added for optimization of matching_keyword
  gint			length;
  gboolean		good_case;
}			t_match;

/*
** Macros for use with refresh_editor()
*/
#define SYHI_AUTODETECT	(-1)
#define SYHI_DISABLE	(-2)

/*
** Prototypes of fcns
*/
extern void		editor_init(void);
extern void		open_file_in_editor(GtkWidget *Editor,
					    const gchar *Filename);
extern void		refresh_editor(GtkWidget *Editor, gint tos);
gint			guess_lang(void);
extern gint		read_uedit_wordfile(const gchar *wf_name);
gint			parse_language_section(gchar *buffer,
					       gint size,
					       gint *start);
gint		my_g_strcase_equal (gconstpointer v, gconstpointer v2);
guint		my_g_strcase_hash (gconstpointer key);
gint			parse_color_section(gchar *buffer,
					    gint size,
					    gint *start,
					    gint Lg);
gint			parse_line_of_keywords(gchar *buffer,
					       gint size,
					       gint *start,
					       gint Lg,
					       gint col);
gboolean		text_has_been_inserted(GtkEditable *Editor,
					       const gchar *Text,
					       gint length,
					       gint *position,
					       gpointer data);
gboolean		text_has_been_deleted(GtkEditable *Editor,
					      gint start,
					      gint end,
					      gpointer data);

#ifdef IGNORE_DATA
 gboolean		refresh_syhi(GtkEditable *Editor);
#else
 gboolean		refresh_syhi(GtkEditable *Editor, gpointer data);
#endif

gboolean		reconnect_sig(GtkEditable *Editor, gpointer data);
void			matching_keyword(gint Lg,
					 guchar *str,
					 t_match *match);
void			matching_keyword_case_ext(gint Lg,
						  const guchar *str,
						  t_match *match);

#endif /* !__EDITOR_H__ */
