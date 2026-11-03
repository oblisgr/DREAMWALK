/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.8 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 * Comments: 'notes' attached to players to keep track of outstanding       * 
 *           and problem players.  -haus 6/25/1995                          * 
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void note_attach( CHAR_DATA * ch );

void comment_remove( CHAR_DATA * victim, NOTE_DATA * pnote )
{
   if( !victim->comments )
   {
      bug( "%s: null board", __func__ );
      return;
   }

   if( !pnote )
   {
      bug( "%s: null pnote", __func__ );
      return;
   }

   /*
    * Remove comment from linked list.
    */
   if( !pnote->prev )
      victim->comments = pnote->next;
   else
      pnote->prev->next = pnote->next;

   /*
    * Fixed a big crash bug here - Luc 08/2000 
    */
   if( pnote->next )
      pnote->next->prev = pnote->prev;

   STRFREE( pnote->text );
   STRFREE( pnote->subject );
   STRFREE( pnote->to_list );
   STRFREE( pnote->date );
   STRFREE( pnote->sender );
   DISPOSE( pnote );

   /*
    * Rewrite entire list.
    */
   save_char_obj( victim );

   return;
}

void do_comment( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   NOTE_DATA *pnote;
   CHAR_DATA *victim;
   int vnum;
   int anum;

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs can't use the comment command."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      bug( "%s", "do_comment: no descriptor" );
      return;
   }

   /*
    * Put in to prevent crashing when someone issues a comment command
    * from within the editor. -Narn 
    */
   if( ch->desc->connected == CON_EDITING )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't use the comment command from within the editor."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_WRITING_NOTE:
         if( !ch->pnote )
         {
            bug( "%s: note got lost?", __func__ );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your note got lost!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            stop_editing( ch );
            return;
         }
         if( ch->dest_buf != ch->pnote )
            bug( "%s: sub_writing_note: ch->dest_buf != ch->pnote", __func__ );
         STRFREE( ch->pnote->text );
         ch->pnote->text = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   set_char_color( AT_NOTE, ch );
   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "about" ) )
   {
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They're not logged on!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}  /* maybe fix this? */
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No comments about mobs"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "list" ) )
   {
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They're not logged on!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}  /* maybe fix this? */
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No comments about mobs"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( victim ) >= get_trust( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're not of the right caliber to do this..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !victim->comments )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no relevant comments."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      vnum = 0;
      for( pnote = victim->comments; pnote; pnote = pnote->next )
      {
         vnum++;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%2d) %-10s [%s] %s", vnum, pnote->sender, pnote->date, pnote->subject); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }

      /*
       * act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_ROOM ); 
       */
      return;
   }

   if( !str_cmp( arg, "read" ) )
   {
      bool fAll;

      argument = one_argument( argument, arg1 );
      victim = get_char_world( ch, arg1 );
      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They're not logged on!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}  /* maybe fix this? */
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No comments about mobs"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( victim ) >= get_trust( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're not of the right caliber to do this..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !victim->comments )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no relevant comments."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( argument, "all" ) )
      {
         fAll = TRUE;
         anum = 0;
      }
      else if( is_number( argument ) )
      {
         fAll = FALSE;
         anum = atoi( argument );
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Note read which number?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      vnum = 0;
      for( pnote = victim->comments; pnote; pnote = pnote->next )
      {
         vnum++;
         if( vnum == anum || fAll )
         {
            ch_printf( ch, "[%3d] %s: %s\r\n%s\r\nTo: %s\r\n",
                       vnum, pnote->sender, pnote->subject, pnote->date, pnote->to_list );
            send_to_char( pnote->text, ch );
            send_to_char( "\r\n", ch );
            /*
             * act( AT_ACTION, "$n reads a note.", ch, NULL, NULL, TO_ROOM ); 
             */
            if( !fAll ) return;
         }
      }

      if( !fAll ) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such comment."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "write" ) )
   {
      note_attach( ch );
      ch->substate = SUB_WRITING_NOTE;
      ch->dest_buf = ch->pnote;
      start_editing( ch, ch->pnote->text );
      return;
   }

   if( !str_cmp( arg, "subject" ) )
   {
      char clean[MAX_STRING_LENGTH];
      mudstrlcpy( clean, argument, sizeof( clean ) );
      smash_tilde( clean );
      note_attach( ch );
      STRFREE( ch->pnote->subject );
      ch->pnote->subject = STRALLOC( clean );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "to" ) )
   {
      char clean[MAX_STRING_LENGTH];
      mudstrlcpy( clean, argument, sizeof( clean ) );
      smash_tilde( clean );
      note_attach( ch );
      STRFREE( ch->pnote->to_list );
      ch->pnote->to_list = STRALLOC( clean );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "clear" ) )
   {
      if( ch->pnote )
      {
         STRFREE( ch->pnote->text );
         STRFREE( ch->pnote->subject );
         STRFREE( ch->pnote->to_list );
         STRFREE( ch->pnote->date );
         STRFREE( ch->pnote->sender );
         DISPOSE( ch->pnote );
      }
      ch->pnote = NULL;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "show" ) )
   {
      if( !ch->pnote )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no comment in progress."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      ch_printf( ch, "%s: %s\r\nTo: %s\r\n", ch->pnote->sender, ch->pnote->subject, ch->pnote->to_list );
      send_to_char( ch->pnote->text, ch );
      return;
   }

   if( !str_cmp( arg, "post" ) )
   {
      char strtime[64];

      if( !ch->pnote )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no comment in progress."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      argument = one_argument( argument, arg1 );
      victim = get_char_world( ch, arg1 );
      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They're not logged on!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}  /* maybe fix this? */
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No comments about mobs"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( victim ) >= get_trust( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're not of the right caliber to do this..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * act( AT_ACTION, "$n posts a note.", ch, NULL, NULL, TO_ROOM ); 
       */

      mudstrlcpy( strtime, ctime( &current_time ), sizeof( strtime ) );
      if( strtime[0] && strtime[strlen( strtime ) - 1] == '\n' )
         strtime[strlen( strtime ) - 1] = '\0';
      STRFREE( ch->pnote->date );
      ch->pnote->date = STRALLOC( strtime );

      pnote = ch->pnote;
      ch->pnote = NULL;

      /*
       * LIFO to make life easier 
       */
      pnote->next = victim->comments;
      if( victim->comments )
         victim->comments->prev = pnote;
      pnote->prev = NULL;
      victim->comments = pnote;

      save_char_obj( victim );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "remove" ) )
   {
      argument = one_argument( argument, arg1 );
      victim = get_char_world( ch, arg1 );
      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They're not logged on!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}  /* maybe fix this? */
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No comments about mobs"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( get_trust( victim ) >= get_trust( ch ) ) || ( get_trust( ch ) < LEVEL_GOD ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're not of the right caliber to do this..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * argument = one_argument(argument, arg); 
       */
      if( !is_number( argument ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Comment remove which number?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * Cleaned it somewhat here - Luc 08/2000 
       */
      anum = atoi( argument );
      for( pnote = victim->comments; pnote; pnote = pnote->next )
      {
         if( get_trust( ch ) >= LEVEL_GOD && !( --anum ) )
         {
            comment_remove( victim, pnote );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such comment."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?  Type 'help comment' for usage (i hope!)."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void fwrite_comments( CHAR_DATA * ch, FILE * fp )
{
   NOTE_DATA *pnote, *last = NULL;

   if( !ch->comments )
      return;

   /*
    * fread_comment will push the last note read on top of the others,
    * so let's write them in reverse to preserve note order's - Luc 08/2000 
    */
   for( pnote = ch->comments; pnote; pnote = pnote->next )
      last = pnote;
   for( pnote = last; pnote; pnote = pnote->prev )
   {
      fprintf( fp, "#COMMENT\n" );
      fprintf( fp, "sender  %s~\n", pnote->sender );
      fprintf( fp, "date    %s~\n", pnote->date );
      fprintf( fp, "to      %s~\n", pnote->to_list );
      fprintf( fp, "subject %s~\n", pnote->subject );
      fprintf( fp, "text\n%s~\n", pnote->text );
   }
   return;
}

void fread_comment( CHAR_DATA * ch, FILE * fp )
{
   NOTE_DATA *pnote;

   for( ;; )
   {
      int letter;

      do
      {
         letter = getc( fp );
         if( feof( fp ) )
         {
            return;
         }
      }
      while( isspace( letter ) );
      ungetc( letter, fp );

      CREATE( pnote, NOTE_DATA, 1 );

      if( str_cmp( fread_word( fp ), "sender" ) )
         break;
      pnote->sender = fread_string( fp );

      if( str_cmp( fread_word( fp ), "date" ) )
         break;
      pnote->date = fread_string( fp );

      if( str_cmp( fread_word( fp ), "to" ) )
         break;
      pnote->to_list = fread_string( fp );

      if( str_cmp( fread_word( fp ), "subject" ) )
         break;
      pnote->subject = fread_string( fp );

      if( str_cmp( fread_word( fp ), "text" ) )
         break;
      pnote->text = fread_string( fp );

      /*
       * Fixed a big bug here - Luc 08/2000 
       */
      pnote->next = ch->comments;
      if( ch->comments )
         ch->comments->prev = pnote;

      pnote->prev = NULL;
      ch->comments = pnote;
      return;
   }

   bug( "%s: bad key word. strap in!", __func__ );
   if( pnote->text ) STRFREE( pnote->text );
   if( pnote->subject ) STRFREE( pnote->subject );
   if( pnote->to_list ) STRFREE( pnote->to_list );
   if( pnote->date ) STRFREE( pnote->date );
   if( pnote->sender ) STRFREE( pnote->sender );
   DISPOSE( pnote );
}
