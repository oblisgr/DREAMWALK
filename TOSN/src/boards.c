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
 *                           Special boards module                          *
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

/* Defines for voting on notes. -- Edmond - update for ballot */
#define VOTE_NONE 0
#define VOTE_CLOSED 1
#define VOTE_BCLOSED 2
#define VOTE_OPEN 3
#define VOTE_BALLOT 4

BOARD_DATA *first_board;
BOARD_DATA *last_board;

bool is_note_to( CHAR_DATA * ch, NOTE_DATA * pnote );
void note_attach( CHAR_DATA * ch );
void note_remove( BOARD_DATA * board, NOTE_DATA * pnote );
void do_note( CHAR_DATA * ch, const char *arg_passed, bool IS_MAIL );

void delete_project( PROJECT_DATA * project )
{
   NOTE_DATA *nlog, *tlog;

   for( nlog = project->first_log; nlog; nlog = tlog )
   {
      tlog = nlog->next;

      UNLINK( nlog, project->first_log, project->last_log, next, prev );
      free_note( nlog );
   }
   DISPOSE( project->coder );
   STRFREE( project->description );
   DISPOSE( project->name );
   STRFREE( project->owner );
   STRFREE( project->status );
   STRFREE( project->date );
   UNLINK( project, first_project, last_project, next, prev );
   DISPOSE( project );
}

void free_projects( void )
{
   PROJECT_DATA *project, *project_next;

   for( project = first_project; project; project = project_next )
   {
      project_next = project->next;
      delete_project( project );
   }
   return;
}

bool can_remove( CHAR_DATA * ch, BOARD_DATA * board )
{
   /*
    * If your trust is high enough, you can remove it. 
    */
   if( get_trust( ch ) >= board->min_remove_level )
      return TRUE;

   if( board->extra_removers[0] != '\0' )
   {
      if( is_name( ch->name, board->extra_removers ) )
         return TRUE;
   }
   return FALSE;
}

bool can_read( CHAR_DATA * ch, BOARD_DATA * board )
{
   /*
    * If your trust is high enough, you can read it. 
    */
   if( get_trust( ch ) >= board->min_read_level )
      return TRUE;

   /*
    * Your trust wasn't high enough, so check if a read_group or extra 
    * readers have been set up. 
    */
   if( !IS_NPC(ch) && ch->pcdata && board->read_group[0] != '\0' )
   {
      if( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->read_group ) )
         return TRUE;
      if( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->read_group ) )
         return TRUE;
   }
   if( board->extra_readers[0] != '\0' )
   {
      if( is_name( ch->name, board->extra_readers ) )
         return TRUE;
   }
   return FALSE;
}

bool can_post( CHAR_DATA * ch, BOARD_DATA * board )
{
   /*
    * If your trust is high enough, you can post. 
    */
   if( get_trust( ch ) >= board->min_post_level )
      return TRUE;

   /*
    * Your trust wasn't high enough, so check if a post_group has been set up. 
    */
   if( !IS_NPC(ch) && ch->pcdata && board->post_group[0] != '\0' )
   {
      if( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->post_group ) )
         return TRUE;
      if( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->post_group ) )
         return TRUE;
   }
   return FALSE;
}

void free_board( BOARD_DATA * board )
{
   NOTE_DATA *pnote, *next_note;

   DISPOSE( board->extra_readers );
   DISPOSE( board->extra_removers );
   DISPOSE( board->read_group );
   DISPOSE( board->post_group );
   DISPOSE( board->extra_ballots );
   DISPOSE( board->note_file );
   DISPOSE( board->otakemessg );
   DISPOSE( board->ocopymessg );
   DISPOSE( board->oreadmessg );
   DISPOSE( board->oremovemessg );
   DISPOSE( board->olistmessg );
   DISPOSE( board->opostmessg );
   DISPOSE( board->postmessg );

   for( pnote = board->first_note; pnote; pnote = next_note )
   {
      next_note = pnote->next;
      UNLINK( pnote, board->first_note, board->last_note, next, prev );
      free_note( pnote );
   }
   UNLINK( board, first_board, last_board, next, prev );
   DISPOSE( board );
}

void free_boards( void )
{
   BOARD_DATA *board, *board_next;

   for( board = first_board; board; board = board_next )
   {
      board_next = board->next;
      free_board( board );
   }
   return;
}

/*
 * board commands.
 */
void write_boards_txt( void )
{
   BOARD_DATA *tboard;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", BOARD_DIR, BOARD_FILE );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open %s for writing!", filename );
      return;
   }

   for( tboard = first_board; tboard; tboard = tboard->next )
   {
      fprintf( fpout, "Filename          %s~\n", tboard->note_file );
      fprintf( fpout, "Vnum              %d\n", tboard->board_obj );
      fprintf( fpout, "Min_read_level    %d\n", tboard->min_read_level );
      fprintf( fpout, "Min_post_level    %d\n", tboard->min_post_level );
      fprintf( fpout, "Min_remove_level  %d\n", tboard->min_remove_level );
      fprintf( fpout, "Max_posts         %d\n", tboard->max_posts );
      fprintf( fpout, "Type 	           %d\n", tboard->type );
      fprintf( fpout, "Read_group        %s~\n", tboard->read_group );
      fprintf( fpout, "Post_group        %s~\n", tboard->post_group );
      fprintf( fpout, "Extra_readers     %s~\n", tboard->extra_readers );
      fprintf( fpout, "Extra_removers    %s~\n", tboard->extra_removers );
      fprintf( fpout, "Extra_ballots     %s~\n", tboard->extra_ballots );
      fprintf( fpout, "Ballot_level      %d\n", tboard->min_ballot_level );
      if( tboard->ocopymessg )
         fprintf( fpout, "OCopymessg	   %s~\n", tboard->ocopymessg );
      if( tboard->olistmessg )
         fprintf( fpout, "OListmessg	   %s~\n", tboard->olistmessg );
      if( tboard->opostmessg )
         fprintf( fpout, "OPostmessg	   %s~\n", tboard->opostmessg );
      if( tboard->oreadmessg )
         fprintf( fpout, "OReadmessg	   %s~\n", tboard->oreadmessg );
      if( tboard->oremovemessg )
         fprintf( fpout, "ORemovemessg      %s~\n", tboard->oremovemessg );
      if( tboard->otakemessg )
         fprintf( fpout, "OTakemessg	   %s~\n", tboard->otakemessg );
      if( tboard->postmessg )
         fprintf( fpout, "Postmessg	   %s~\n", tboard->postmessg );
      fprintf( fpout, "End\n" );
   }
   fclose( fpout );
   fpout = NULL;
}

BOARD_DATA *get_board( OBJ_DATA * obj )
{
   BOARD_DATA *board;

   for( board = first_board; board; board = board->next )
      if( board->board_obj == obj->pIndexData->vnum )
         return board;
   return NULL;
}

BOARD_DATA *find_board( CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   BOARD_DATA *board;

   for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
   {
      if( ( board = get_board( obj ) ) != NULL )
         return board;
   }

   return NULL;
}

bool is_note_to( CHAR_DATA * ch, NOTE_DATA * pnote )
{
   if( !str_cmp( ch->name, pnote->sender ) )
      return TRUE;

   if( is_name( "all", pnote->to_list ) )
      return TRUE;

   if( IS_HERO( ch ) && is_name( "immortal", pnote->to_list ) )
      return TRUE;

   if( is_name( ch->name, pnote->to_list ) )
      return TRUE;

   return FALSE;
}

void note_attach( CHAR_DATA * ch )
{
   NOTE_DATA *pnote;

   if( ch->pnote )
      return;

   CREATE( pnote, NOTE_DATA, 1 );
   pnote->next = NULL;
   pnote->prev = NULL;
   pnote->sender = QUICKLINK( ch->name );
   pnote->date = STRALLOC( "" );
   pnote->to_list = STRALLOC( "" );
   pnote->subject = STRALLOC( "" );
   pnote->text = STRALLOC( "" );
   ch->pnote = pnote;
   return;
}

void write_board( BOARD_DATA * board )
{
   FILE *fp;
   char filename[256];
   NOTE_DATA *pnote;

   /*
    * Rewrite entire list.
    */
   snprintf( filename, 256, "%s%s", BOARD_DIR, board->note_file );
   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      perror( filename );
   }
   else
   {
      for( pnote = board->first_note; pnote; pnote = pnote->next )
      {
         fprintf( fp,
                  "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nVoting %d\nYesvotes %s~\nNovotes %s~\nAbstentions %s~\nYestally %d\nNotally %d\nAbstaintally %d\nNoremove %d\nText\n%s~\n\n",
                  pnote->sender, pnote->date, pnote->to_list, pnote->subject, pnote->voting, pnote->yesvotes, pnote->novotes,
                  pnote->abstentions, pnote->yestally, pnote->notally, pnote->abstaintally, pnote->no_remove, pnote->text );
      }
      fclose( fp );
      fp = NULL;
   }
   return;
}

void free_note( NOTE_DATA * pnote )
{
   STRFREE( pnote->text );
   STRFREE( pnote->subject );
   STRFREE( pnote->to_list );
   STRFREE( pnote->date );
   STRFREE( pnote->sender );
   if( pnote->yesvotes )
      DISPOSE( pnote->yesvotes );
   if( pnote->novotes )
      DISPOSE( pnote->novotes );
   if( pnote->abstentions )
      DISPOSE( pnote->abstentions );
   DISPOSE( pnote );
}

void note_remove( BOARD_DATA * board, NOTE_DATA * pnote )
{

   if( !board )
   {
      bug( "%s", "note remove: null board" );
      return;
   }

   if( !pnote )
   {
      bug( "%s", "note remove: null pnote" );
      return;
   }

   /*
    * Remove note from linked list.
    */
   UNLINK( pnote, board->first_note, board->last_note, next, prev );

   --board->num_posts;
   free_note( pnote );
   write_board( board );
}

OBJ_DATA *find_quill( CHAR_DATA * ch )
{
   OBJ_DATA *quill;

   for( quill = ch->last_carrying; quill; quill = quill->prev_content )
      if( quill->item_type == ITEM_PEN && can_see_obj( ch, quill ) )
         return quill;
   return NULL;
}

void do_noteroom( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;
   char arg[MAX_STRING_LENGTH];
   char arg_passed[MAX_STRING_LENGTH];

   mudstrlcpy( arg_passed, argument, MAX_STRING_LENGTH );

   switch ( ch->substate )
   {
      case SUB_WRITING_NOTE:
         do_note( ch, arg_passed, FALSE );
         break;

      default:

         argument = one_argument( argument, arg );
         if( !str_cmp( arg, "write" ) || !str_cmp( arg, "to" ) || !str_cmp( arg, "subject" ) || !str_cmp( arg, "show" ) )
         {
            do_note( ch, arg_passed, FALSE );
            return;
         }

         board = find_board( ch );
         if( !board )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no bulletin board here to look at."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( board->type != BOARD_NOTE )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only use note commands on a note board."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         else
         {
            do_note( ch, arg_passed, FALSE );
            return;
         }
   }
}

void do_mailroom( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;
   char arg[MAX_STRING_LENGTH];
   char arg_passed[MAX_STRING_LENGTH];

   mudstrlcpy( arg_passed, argument, MAX_STRING_LENGTH );

   switch ( ch->substate )
   {
      case SUB_WRITING_NOTE:
         do_note( ch, arg_passed, TRUE );
         break;

      default:

         argument = one_argument( argument, arg );
         if( !str_cmp( arg, "write" ) || !str_cmp( arg, "to" ) || !str_cmp( arg, "subject" ) || !str_cmp( arg, "show" ) )
         {
            do_note( ch, arg_passed, TRUE );
            return;
         }

         board = find_board( ch );
         if( !board )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no mail facility here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( board->type != BOARD_MAIL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only use mail commands in a post office."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         else
         {
            do_note( ch, arg_passed, TRUE );
            return;
         }
   }
}

void do_note( CHAR_DATA * ch, const char *arg_passed, bool IS_MAIL )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   NOTE_DATA *pnote;
   BOARD_DATA *board;
   int vnum;
   int anum;
   int first_list;
   OBJ_DATA *quill = NULL, *paper = NULL, *tmpobj = NULL;
   EXTRA_DESCR_DATA *ed = NULL;
   char notebuf[MAX_STRING_LENGTH];
   char short_desc_buf[MAX_STRING_LENGTH];
   char long_desc_buf[MAX_STRING_LENGTH];
   char keyword_buf[MAX_STRING_LENGTH];
   bool mfound = FALSE;
   char note_argument[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
      bug( "%s", "do_note: no descriptor" );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_WRITING_NOTE:
         if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
         {
            bug( "%s", "do_note: player not holding paper" );
            stop_editing( ch );
            return;
         }
         ed = ( EXTRA_DESCR_DATA * ) ch->dest_buf;
         if( !ed ) {stop_editing( ch ); return;}
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   set_char_color( AT_NOTE, ch );
   mudstrlcpy( note_argument, arg_passed ? arg_passed : "", sizeof(note_argument) );
   smash_tilde( note_argument );
   arg_passed = one_argument( note_argument, arg );

   /*
    * Reusing alot of code but this will have to do till I can think
    * * of a better way to do it. --Shaddai
    */
   if( !str_cmp( arg, "date" ) )
   {
      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no board here to look at."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot make any sense of the cryptic scrawl on this board..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      first_list = atoi( arg_passed );
      if( first_list )
      {
         if( IS_MAIL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot use a list number (at this time) with mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( first_list < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't read a note before 1!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !IS_MAIL )
      {
         set_pager_color( AT_NOTE, ch );
         vnum = 0;
         for( pnote = board->first_note; pnote; pnote = pnote->next )
         {
            vnum++;

            if( ( first_list && vnum >= first_list ) || !first_list )
               pager_printf( ch, "%2d%c %-12s%c %-24s : %-35s\n\r",
                             vnum,
                             is_note_to( ch, pnote ) ? ')' : '}',
                             pnote->sender,
                             ( pnote->voting != VOTE_NONE ) ?
                             ( ( pnote->voting > VOTE_BCLOSED ) ?
                               ( ( pnote->voting == VOTE_OPEN ) ? 'V' : 'B' ) : 'C' ) : ':', pnote->date, pnote->subject );
         }
         if( board->olistmessg )
            act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE );
         else
            act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_CANSEE );
         return;
      }
      else
      {
         vnum = 0;

         if( IS_MAIL )  /* SB Mail check for Brit */
         {
            for( pnote = board->first_note; pnote; pnote = pnote->next )
               if( is_note_to( ch, pnote ) )
                  mfound = TRUE;

            if( !mfound && get_trust( ch ) < sysdata.read_all_mail )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have no mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }

         for( pnote = board->first_note; pnote; pnote = pnote->next )
            if( is_note_to( ch, pnote ) || get_trust( ch ) >= sysdata.read_all_mail )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%2d%c %-12s: %-13s : %s", ++vnum, is_note_to( ch, pnote ) ? '-' : '}', pnote->sender, pnote->subject, pnote->text); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "list" ) || !str_cmp( arg, "mine" ) )
   {
      int mine = 0;
      bool name_list = FALSE;
      board = find_board( ch );

      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no board here to look at."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot make any sense of the cryptic scrawl on this board..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg, "mine" ) )
         mine = 1;
 
      /*
       * Check for note list <player> 
       */
      if( !is_number( arg_passed ) && arg_passed[0] != '\0' )
         name_list = TRUE;

      first_list = atoi( arg_passed );
      arg_passed = one_argument( arg_passed, arg );   /*For note list <player> */
      if( first_list )
      {
         if( IS_MAIL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot use a list number (at this time) with mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( first_list < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't read a note before 1!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !IS_MAIL )
      {
         set_pager_color( AT_NOTE, ch );
         vnum = 0;
         anum = 0;
         for( pnote = board->first_note; pnote; pnote = pnote->next )
         {
            vnum++;
            if( ( first_list && vnum >= first_list )
                || ( name_list && !str_cmp( arg, pnote->sender ) ) || ( !name_list && !first_list ) )
            {
               pager_printf( ch, "%2d%c %-12s%c %-12.12s : %s\r\n",
                             vnum,
                             is_note_to( ch, pnote ) ? ')' : '}',
                             pnote->sender,
                             ( pnote->voting != VOTE_NONE ) ?
                             ( ( pnote->voting > VOTE_BCLOSED ) ?
                               ( ( pnote->voting == VOTE_OPEN ) ? 'V' : 'B' )
                               : 'C' ) : ':', pnote->to_list, pnote->subject );
               anum++;
            }
         }
         if( anum == 0 && name_list )
            pager_printf( ch, "There are no notes posted by %s.\n\r", capitalize( arg ) );
         if( board->olistmessg )
            act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE );
         else
            act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_CANSEE );
         return;
      }
      else
      {
         vnum = 0;

         if( IS_MAIL )  /* SB Mail check for Brit */
         {
            for( pnote = board->first_note; pnote; pnote = pnote->next )
               if( is_note_to( ch, pnote ) )
                  mfound = TRUE;

            if( !mfound && get_trust( ch ) < sysdata.read_all_mail )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have no mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }

         for( pnote = board->first_note; pnote; pnote = pnote->next )
            if( is_note_to( ch, pnote ) || ( !mine && get_trust( ch ) >= sysdata.read_all_mail ) )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%2d%c %s: %s", ++vnum, is_note_to( ch, pnote ) ? '-' : '}', pnote->sender, pnote->subject); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            else if( mine && get_trust( ch ) >= sysdata.read_all_mail )
               vnum++;
         return;
      }
   }

   if( !str_cmp( arg, "read" ) )
   {
      bool fAll;

      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no board here to look at."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot make any sense of the cryptic scrawl on this board..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg_passed, "all" ) )
      {
         fAll = TRUE;
         anum = 0;
      }
      else if( is_number( arg_passed ) )
      {
         fAll = FALSE;
         anum = atoi( arg_passed );
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Note read which number?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      set_pager_color( AT_NOTE, ch );
      if( !IS_MAIL )
      {
         vnum = 0;
         for( pnote = board->first_note; pnote; pnote = pnote->next )
         {
            vnum++;
            if( vnum == anum || fAll )
            {
               pager_printf( ch, "[%3d] %s: %s\r\n%s\r\nTo: %s\r\n%s",
                             vnum, pnote->sender, pnote->subject, pnote->date, pnote->to_list, pnote->text );

               if( pnote->yesvotes[0] != '\0' || pnote->novotes[0] != '\0' || pnote->abstentions[0] != '\0' )
               {
                  send_to_pager( "------------------------------------------------------------\r\n", ch );
                  if( pnote->voting == VOTE_OPEN ||
                      pnote->voting == VOTE_CLOSED ||
                      get_trust( ch ) >= board->min_ballot_level ||
                      ( board->extra_ballots[0] != '\0' && is_name( ch->name, board->extra_ballots ) ) )
                     pager_printf( ch, "Votes:\r\nYes:  %s\r\nNo:  %s\r\nAbstain: %s\r\n", pnote->yesvotes, pnote->novotes,
                                   pnote->abstentions );
                  else
                     pager_printf( ch, "Votes:\r\nYes:  %d\r\nNo:  %d\r\nAbstain: %d\r\n", pnote->yestally, pnote->notally,
                                   pnote->abstaintally );
               }
               if( board->oreadmessg )
                  act( AT_ACTION, board->oreadmessg, ch, NULL, NULL, TO_CANSEE );
               else
                  act( AT_ACTION, "$n reads a note.", ch, NULL, NULL, TO_CANSEE );
               return;
            }
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         vnum = 0;
         for( pnote = board->first_note; pnote; pnote = pnote->next )
         {
            if( is_note_to( ch, pnote ) || get_trust( ch ) >= sysdata.read_all_mail )
            {
               vnum++;
               if( vnum == anum || fAll )
               {
                  if( ch->gold < 10 && get_trust( ch ) < sysdata.read_mail_free )
                  {
                     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It costs 10 gold coins to read a message."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
                     return;
                  }
                  if( get_trust( ch ) < sysdata.read_mail_free )
                     ch->gold -= 10;
                  pager_printf( ch, "[%3d] %s: %s\r\n%s\r\nTo: %s\r\n%s",
                                vnum, pnote->sender, pnote->subject, pnote->date, pnote->to_list, pnote->text );
                  return;
               }
            }
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such message."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "noremove" ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      arg_passed = one_argument( arg_passed, arg2 );

      if( !IS_IMMORTAL( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }

      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no bulletin board here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot read this board."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( is_number( arg2 ) )
         anum = atoi( arg2 );
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set noremove on which number?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      vnum = 1;
      for( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
         vnum++;

      if( !pnote )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( pnote->no_remove != 0 && pnote->no_remove > ch->level )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That note is already set to noremove."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      pnote->no_remove = ch->level;
      write_board( board );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Noremove flag set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Voting added by Narn, June '96 
    */
   if( !str_cmp( arg, "vote" ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      arg_passed = one_argument( arg_passed, arg2 );

      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no bulletin board here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot vote on this board."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( is_number( arg2 ) )
         anum = atoi( arg2 );
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Note vote which number?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      vnum = 1;
      for( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
         vnum++;
      if( !pnote )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * Options: open close yes no abstain 
       * If you're the author of the note and can read the board you can open 
       * and close voting, if you can read it and voting is open you can vote.
       */
      if( !str_cmp( arg_passed, "open" ) )
      {
         if( str_cmp( ch->name, pnote->sender ) && get_trust( ch ) < LEVEL_GREATER )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not the author of this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( pnote->voting )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This note has already been made a vote."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         pnote->voting = VOTE_OPEN;
         act( AT_ACTION, "$n opens voting on a note.", ch, NULL, NULL, TO_ROOM );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Voting opened."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }

      if( !str_cmp( arg_passed, "ballot" ) )
      {
         if( str_cmp( ch->name, pnote->sender ) && get_trust( ch ) < LEVEL_GREATER )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not the author of this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( pnote->voting )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This note has already been made a vote."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         pnote->voting = VOTE_BALLOT;
         act( AT_ACTION, "$n opens a ballot on a note.", ch, NULL, NULL, TO_CANSEE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ballot opened."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }

      if( !str_cmp( arg_passed, "close" ) )
      {
         if( str_cmp( ch->name, pnote->sender ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not the author of this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         pnote->voting = ( pnote->voting == VOTE_BALLOT ? VOTE_BCLOSED : VOTE_CLOSED );
         act( AT_ACTION, "$n closes voting on a note.", ch, NULL, NULL, TO_CANSEE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Voting closed."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }

      /*
       * Make sure the note is open for voting before going on. 
       */
      if( pnote->voting < VOTE_OPEN )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Voting is not open on this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * Can only vote once on a note. 
       */
      snprintf( buf, MAX_STRING_LENGTH, "%s %s %s", pnote->yesvotes, pnote->novotes, pnote->abstentions );
      if( is_name( ch->name, buf ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have already voted on this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg_passed, "yes" ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", pnote->yesvotes, ch->name );
         DISPOSE( pnote->yesvotes );
         pnote->yesvotes = str_dup( buf );
         pnote->yestally = ( ( pnote->yestally ) ? pnote->yestally + 1 : 1 );
         act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You vote yes on %s.", pnote->subject); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }

      if( !str_cmp( arg_passed, "no" ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", pnote->novotes, ch->name );
         DISPOSE( pnote->novotes );
         pnote->novotes = str_dup( buf );
         pnote->notally = ( ( pnote->notally ) ? pnote->notally + 1 : 1 );
         act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You vote no on %s.", pnote->subject); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }

      if( !str_cmp( arg_passed, "abstain" ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", pnote->abstentions, ch->name );
         DISPOSE( pnote->abstentions );
         pnote->abstentions = str_dup( buf );
         pnote->abstaintally = ( ( pnote->abstaintally ) ? pnote->abstaintally + 1 : 1 );
         act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You abstain on %s.", pnote->subject); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_board( board );
         return;
      }
      do_note( ch, "", FALSE );
   }

   if( !str_cmp( arg, "write" ) )
   {
      if( ch->substate == SUB_RESTRICTED )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot write a note from within another command."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( ch ) < sysdata.write_mail_free )
      {
         quill = find_quill( ch );
         if( !quill )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a quill to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( quill->value[0] < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your quill is dry."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         if( get_trust( ch ) < sysdata.write_mail_free )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to be holding a fresh piece of parchment to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
         if( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
            unequip_char( ch, tmpobj );
         paper = obj_to_char( paper, ch );
         equip_char( ch, paper, WEAR_HOLD );
         act( AT_MAGIC, "A piece of parchment magically appears in $n's hands!", ch, NULL, NULL, TO_CANSEE );
         act( AT_MAGIC, "A piece of parchment appears in your hands.", ch, NULL, NULL, TO_CHAR );
      }

      if( paper->value[0] < 2 )
      {
         paper->value[0] = 1;
         ed = SetOExtra( paper, "_text_" );
         ch->substate = SUB_WRITING_NOTE;
         ch->dest_buf = ed;
         if( get_trust( ch ) < sysdata.write_mail_free )
            --quill->value[0];
         start_editing( ch, ed->description );
         return;
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "subject" ) )
   {
      if( get_trust( ch ) < sysdata.write_mail_free )
      {
         quill = find_quill( ch );
         if( !quill )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a quill to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( quill->value[0] < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your quill is dry."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !arg_passed || arg_passed[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What do you wish the subject to be?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         if( get_trust( ch ) < sysdata.write_mail_free )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to be holding a fresh piece of parchment to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
         if( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
            unequip_char( ch, tmpobj );
         paper = obj_to_char( paper, ch );
         equip_char( ch, paper, WEAR_HOLD );
         act( AT_MAGIC, "A piece of parchment magically appears in $n's hands!", ch, NULL, NULL, TO_CANSEE );
         act( AT_MAGIC, "A piece of parchment appears in your hands.", ch, NULL, NULL, TO_CHAR );
      }

      if( paper->value[1] > 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         paper->value[1] = 1;
         ed = SetOExtra( paper, "_subject_" );
         STRFREE( ed->description );
         ed->description = STRALLOC( arg_passed );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "to" ) )
   {
      struct stat fst;
/*	char *pn;*/
      char fname[1024];

      if( get_trust( ch ) < sysdata.write_mail_free )
      {
         quill = find_quill( ch );
         if( !quill )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a quill to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( quill->value[0] < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your quill is dry."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !arg_passed || arg_passed[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Please specify an addressee."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         if( get_trust( ch ) < sysdata.write_mail_free )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to be holding a fresh piece of parchment to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
         if( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
            unequip_char( ch, tmpobj );
         paper = obj_to_char( paper, ch );
         equip_char( ch, paper, WEAR_HOLD );
         act( AT_MAGIC, "A piece of parchment magically appears in $n's hands!", ch, NULL, NULL, TO_CANSEE );
         act( AT_MAGIC, "A piece of parchment appears in your hands.", ch, NULL, NULL, TO_CHAR );
      }

      if( paper->value[2] > 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      // This now modifies our private input copy, not the command argument.
      ((char*)arg_passed)[0] = UPPER( arg_passed[0] );

      snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( arg_passed[0] ), capitalize( arg_passed ) );

      if( !IS_MAIL || stat( fname, &fst ) != -1 || !str_cmp( arg_passed, "all" ) )
      {
         paper->value[2] = 1;
         ed = SetOExtra( paper, "_to_" );
         STRFREE( ed->description );
         ed->description = STRALLOC( arg_passed );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No player exists by that name."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

   }

   if( !str_cmp( arg, "show" ) )
   {
      const char *subject, *to_list, *text;

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not holding a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( subject = get_extra_descr( "_subject_", paper->first_extradesc ) ) == NULL )
         subject = "(no subject)";
      if( ( to_list = get_extra_descr( "_to_", paper->first_extradesc ) ) == NULL )
         to_list = "(nobody)";
      ch_printf( ch, "%s: %s\r\nTo: %s\r\n", ch->name, subject, to_list );
      if( ( text = get_extra_descr( "_text_", paper->first_extradesc ) ) == NULL )
         text = "The note is blank.\r\n";
      send_to_char( text, ch );
      return;
   }

   if( !str_cmp( arg, "post" ) )
   {
      char *strtime;
      const char *to, *subj, *text /*, *np = NULL */ ;

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not holding a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( paper->value[0] == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is nothing written on this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( paper->value[1] == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This note has no subject."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( paper->value[2] == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This note is addressed to no one!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      strtime = ctime( &current_time );
      strtime[strlen( strtime ) - 1] = '\0';

      to = get_extra_descr( "_to_", paper->first_extradesc );
      subj = get_extra_descr( "_subject_", paper->first_extradesc );
      text = get_extra_descr( "_text_", paper->first_extradesc );

      board = find_board( ch );

      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no bulletin board here to post your note on."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_post( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A magical force prevents you from posting your note here..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( board->num_posts >= board->max_posts )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no room on this board to post your note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( board->opostmessg )
         act( AT_ACTION, board->opostmessg, ch, NULL, NULL, TO_CANSEE );
      else
         act( AT_ACTION, "$n posts a note.", ch, NULL, NULL, TO_CANSEE );

      CREATE( pnote, NOTE_DATA, 1 );
      pnote->date = STRALLOC( strtime );

      pnote->to_list = to ? STRALLOC( to ) : STRALLOC( "all" );
      pnote->text = text ? STRALLOC( text ) : STRALLOC( "" );
      pnote->subject = subj ? STRALLOC( subj ) : STRALLOC( "" );
      pnote->sender = QUICKLINK( ch->name );
      pnote->voting = 0;
      pnote->yesvotes = str_dup( "" );
      pnote->novotes = str_dup( "" );
      pnote->abstentions = str_dup( "" );
      pnote->no_remove = 0;

      LINK( pnote, board->first_note, board->last_note, next, prev );
      board->num_posts++;
      write_board( board );
      if( board->postmessg )
         act( AT_ACTION, board->postmessg, ch, NULL, NULL, TO_CHAR );
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You post your note on the board."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      extract_obj( paper );
      return;
   }

   if( !str_cmp( arg, "reply" ) )
   {
      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no bulletin board here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot read the notes on this board to reply to."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( is_number( arg_passed ) )
      {
         anum = atoi( arg_passed );
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Reply to which number?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !IS_MAIL )
      {
         vnum = 1;
         for( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
            vnum++;
      }
      else
      {
         vnum = 0;
         for( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
            if( is_note_to( ch, pnote ) || get_trust( ch ) >= sysdata.read_all_mail )
            {
               vnum++;
               if( vnum == anum )
                  break;
            }
      }

      if( !pnote )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->substate == SUB_RESTRICTED )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot write a note from within another command."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( ch ) < sysdata.write_mail_free )
      {
         quill = find_quill( ch );
         if( !quill )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a quill to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( quill->value[0] < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your quill is dry."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL || paper->item_type != ITEM_PAPER )
      {
         if( get_trust( ch ) < sysdata.write_mail_free )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to be holding a fresh piece of parchment to write a note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
         if( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
            unequip_char( ch, tmpobj );
         paper = obj_to_char( paper, ch );
         equip_char( ch, paper, WEAR_HOLD );
         act( AT_MAGIC, "A piece of parchment magically appears in $n's hands!", ch, NULL, NULL, TO_CANSEE );
         act( AT_MAGIC, "A piece of parchment appears in your hands.", ch, NULL, NULL, TO_CHAR );
      }

      if( paper->value[0] < 2 )
      {
         paper->value[0] = 1;
         paper->value[1] = 1;
         paper->value[2] = 1;
         ed = SetOExtra( paper, "_text_" );
         ch->substate = SUB_WRITING_NOTE;
         ch->dest_buf = ed;
         if( get_trust( ch ) < sysdata.write_mail_free )
            --quill->value[0];
         start_editing( ch, ed->description );
         ed = SetOExtra( paper, "_to_" );
         STRFREE( ed->description );
         ed->description = QUICKLINK( pnote->sender );
         snprintf( notebuf, sizeof(notebuf), "Re: %s", pnote->subject );
         ed = SetOExtra( paper, "_subject_" );
         STRFREE( ed->description );
         ed->description = STRALLOC( notebuf );
         return;
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg, "remove" ) || !str_cmp( arg, "take" ) || !str_cmp( arg, "copy" ) )
   {
      char take;

      board = find_board( ch );
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no board here to take a note from!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg, "take" ) )
         take = 1;
      else if( !str_cmp( arg, "copy" ) )
      {
         if( !IS_IMMORTAL( ch ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?  Type 'help note' for usage."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         take = 2;
      }
      else
         take = 0;

      if( !is_number( arg_passed ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Note remove which number?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_read( ch, board ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't make any sense of what's posted here, let alone remove anything!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      anum = atoi( arg_passed );
      vnum = 0;
      for( pnote = board->first_note; pnote; pnote = pnote->next )
      {
         if( IS_MAIL && ( ( is_note_to( ch, pnote ) ) || get_trust( ch ) >= sysdata.take_others_mail ) )
            vnum++;
         else if( !IS_MAIL )
            vnum++;
         if( ( is_note_to( ch, pnote ) || can_remove( ch, board ) ) && ( vnum == anum ) )
         {
            if( ( is_name( "all", pnote->to_list ) )
                && ( get_trust( ch ) < sysdata.take_others_mail ) && ( take == 1 ) && str_cmp( ch->name, pnote->sender ) )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Notes addressed to 'all' can only be taken by the author."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }

            if( pnote->no_remove != 0 && ( take == 1 || take == 0 ) && ch->level < pnote->no_remove )
            {
               if( take == 0 )
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't remove this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               else
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't take this note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }

            if( take != 0 )
            {
               if( ch->gold < 50 && get_trust( ch ) < sysdata.read_mail_free )
               {
                  if( take == 1 )
                     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It costs 50 coins to take your mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
                  else
                     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It costs 50 coins to copy your mail."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
                  return;
               }
               if( get_trust( ch ) < sysdata.read_mail_free )
                  ch->gold -= 50;
               paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
               ed = SetOExtra( paper, "_sender_" );
               STRFREE( ed->description );
               ed->description = QUICKLINK( pnote->sender );
               ed = SetOExtra( paper, "_text_" );
               STRFREE( ed->description );
               ed->description = QUICKLINK( pnote->text );
               ed = SetOExtra( paper, "_to_" );
               STRFREE( ed->description );
               ed->description = QUICKLINK( pnote->to_list );
               ed = SetOExtra( paper, "_subject_" );
               STRFREE( ed->description );
               ed->description = QUICKLINK( pnote->subject );
               ed = SetOExtra( paper, "_date_" );
               STRFREE( ed->description );
               ed->description = QUICKLINK( pnote->date );
               ed = SetOExtra( paper, "note" );
               STRFREE( ed->description );
               snprintf( notebuf, MAX_STRING_LENGTH, "%s", "From: " );
               mudstrlcat( notebuf, pnote->sender, MAX_STRING_LENGTH );
               mudstrlcat( notebuf, "\r\nTo: ", MAX_STRING_LENGTH );
               mudstrlcat( notebuf, pnote->to_list, MAX_STRING_LENGTH );
               mudstrlcat( notebuf, "\r\nSubject: ", MAX_STRING_LENGTH );
               mudstrlcat( notebuf, pnote->subject, MAX_STRING_LENGTH );
               mudstrlcat( notebuf, "\r\n\r\n", MAX_STRING_LENGTH );
               mudstrlcat( notebuf, pnote->text, MAX_STRING_LENGTH );
               mudstrlcat( notebuf, "\r\n", MAX_STRING_LENGTH );
               ed->description = STRALLOC( notebuf );
               paper->value[0] = 2;
               paper->value[1] = 2;
               paper->value[2] = 2;
               snprintf( short_desc_buf, MAX_STRING_LENGTH, "a note from %s to %s: %s", pnote->sender, pnote->to_list, pnote->subject );
               STRFREE( paper->short_descr );
               paper->short_descr = STRALLOC( short_desc_buf );
               snprintf( long_desc_buf, MAX_STRING_LENGTH, "A note from %s to %s lies on the ground.", pnote->sender,
                         pnote->to_list );
               STRFREE( paper->description );
               paper->description = STRALLOC( long_desc_buf );
               snprintf( keyword_buf, MAX_STRING_LENGTH, "note parchment paper %s", pnote->to_list );
               STRFREE( paper->name );
               paper->name = STRALLOC( keyword_buf );
            }
            if( take != 2 )
               note_remove( board, pnote );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            if( take == 1 )
            {
               if( board->otakemessg )
                  act( AT_ACTION, board->otakemessg, ch, NULL, NULL, TO_CANSEE );
               else
                  act( AT_ACTION, "$n takes a note.", ch, NULL, NULL, TO_CANSEE );
               obj_to_char( paper, ch );
            }
            else if( take == 2 )
            {
               if( board->ocopymessg )
                  act( AT_ACTION, board->ocopymessg, ch, NULL, NULL, TO_CANSEE );
               else
                  act( AT_ACTION, "$n copies a note.", ch, NULL, NULL, TO_CANSEE );
               obj_to_char( paper, ch );
            }
            else
            {
               if( board->oremovemessg )
                  act( AT_ACTION, board->oremovemessg, ch, NULL, NULL, TO_CANSEE );
               else
                  act( AT_ACTION, "$n removes a note.", ch, NULL, NULL, TO_CANSEE );
            }
            return;
         }
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such note."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?  Type 'help note' for usage."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

BOARD_DATA *read_board( FILE * fp )
{
   BOARD_DATA *board;
   const char *word;
   bool fMatch;
   char letter;

   do
   {
      letter = getc( fp );
      if( feof( fp ) )
      {
         fclose( fp );
         return NULL;
      }
   }
   while( isspace( letter ) );
   ungetc( letter, fp );

   CREATE( board, BOARD_DATA, 1 );
   /*
    * Setup pointers --Shaddai 
    */
   board->otakemessg = NULL;
   board->opostmessg = NULL;
   board->oremovemessg = NULL;
   board->olistmessg = NULL;
   board->ocopymessg = NULL;
   board->oreadmessg = NULL;
   board->postmessg = NULL;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'B':
            KEY( "Ballot_level", board->min_ballot_level, fread_number( fp ) );
            break;

         case 'E':
            KEY( "Extra_readers", board->extra_readers, fread_string_nohash( fp ) );
            KEY( "Extra_removers", board->extra_removers, fread_string_nohash( fp ) );
            KEY( "Extra_ballots", board->extra_ballots, fread_string_nohash( fp ) );
            if( !str_cmp( word, "End" ) )
            {
               board->num_posts = 0;
               board->first_note = NULL;
               board->last_note = NULL;
               board->next = NULL;
               board->prev = NULL;
               if( !board->read_group )
                  board->read_group = str_dup( "" );
               if( !board->post_group )
                  board->post_group = str_dup( "" );
               if( !board->extra_readers )
                  board->extra_readers = str_dup( "" );
               if( !board->extra_removers )
                  board->extra_removers = str_dup( "" );
               if( !board->extra_ballots )
                  board->extra_ballots = str_dup( "" );
               return board;
            }
            break;

         case 'F':
            KEY( "Filename", board->note_file, fread_string_nohash( fp ) );
            break;

         case 'M':
            KEY( "Min_read_level", board->min_read_level, fread_number( fp ) );
            KEY( "Min_post_level", board->min_post_level, fread_number( fp ) );
            KEY( "Min_remove_level", board->min_remove_level, fread_number( fp ) );
            KEY( "Max_posts", board->max_posts, fread_number( fp ) );
            break;

         case 'O':
            KEY( "OTakemessg", board->otakemessg, fread_string_nohash( fp ) );
            KEY( "OCopymessg", board->ocopymessg, fread_string_nohash( fp ) );
            KEY( "OReadmessg", board->oreadmessg, fread_string_nohash( fp ) );
            KEY( "ORemovemessg", board->oremovemessg, fread_string_nohash( fp ) );
            KEY( "OListmessg", board->olistmessg, fread_string_nohash( fp ) );
            KEY( "OPostmessg", board->opostmessg, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Post_group", board->post_group, fread_string_nohash( fp ) );
            KEY( "Postmessg", board->postmessg, fread_string_nohash( fp ) );
            break;

         case 'R':
            KEY( "Read_group", board->read_group, fread_string_nohash( fp ) );
            break;

         case 'T':
            KEY( "Type", board->type, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Vnum", board->board_obj, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

NOTE_DATA *read_note( FILE * fp )
{
   NOTE_DATA *pnote;
   char *word;

   for( ;; )
   {
      char letter;

      do
      {
         letter = getc( fp );
         if( feof( fp ) )
         {
            fclose( fp );
            return NULL;
         }
      }
      while( isspace( letter ) );
      ungetc( letter, fp );

      CREATE( pnote, NOTE_DATA, 1 );
      pnote->no_remove = 0;

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

      word = fread_word( fp );
      if( !str_cmp( word, "voting" ) )
      {
         pnote->voting = fread_number( fp );

         if( str_cmp( fread_word( fp ), "yesvotes" ) )
            break;
         pnote->yesvotes = fread_string_nohash( fp );

         if( str_cmp( fread_word( fp ), "novotes" ) )
            break;
         pnote->novotes = fread_string_nohash( fp );

         if( str_cmp( fread_word( fp ), "abstentions" ) )
            break;
         pnote->abstentions = fread_string_nohash( fp );

         word = fread_word( fp );

         if( !str_cmp( word, "yestally" ) )
         {
            pnote->yestally = fread_number( fp );

            if( str_cmp( fread_word( fp ), "notally" ) )
               break;
            pnote->notally = fread_number( fp );

            if( str_cmp( fread_word( fp ), "abstaintally" ) )
               break;
            pnote->abstaintally = fread_number( fp );

            word = fread_word( fp );
         }
      }

      if( !str_cmp( word, "Noremove" ) )
      {
         pnote->no_remove = fread_number( fp );
         word = fread_word( fp );
      }

      if( str_cmp( word, "text" ) )
         break;
      pnote->text = fread_string( fp );

      if( !pnote->yesvotes )
         pnote->yesvotes = str_dup( "" );
      if( !pnote->novotes )
         pnote->novotes = str_dup( "" );
      if( !pnote->abstentions )
         pnote->abstentions = str_dup( "" );
      pnote->next = NULL;
      pnote->prev = NULL;
      return pnote;
   }
   bug( "%s: bad key word.", __func__ );
   free_note( pnote );
   fclose( fp );
   return NULL;
}

/*
 * Load boards file.
 */
void load_boards( void )
{
   FILE *board_fp;
   FILE *note_fp;
   BOARD_DATA *board;
   NOTE_DATA *pnote;
   char boardfile[256];
   char notefile[256];

   first_board = NULL;
   last_board = NULL;

   snprintf( boardfile, 256, "%s%s", BOARD_DIR, BOARD_FILE );
   if( !( board_fp = fopen( boardfile, "r" ) ) )
      return;

   while( ( board = read_board( board_fp ) ) != NULL )
   {
      LINK( board, first_board, last_board, next, prev );
      snprintf( notefile, 256, "%s%s", BOARD_DIR, board->note_file );
      log_string( notefile );
      if( ( note_fp = fopen( notefile, "r" ) ) != NULL )
      {
         while( ( pnote = read_note( note_fp ) ) != NULL )
         {
            LINK( pnote, board->first_note, board->last_note, next, prev );
            board->num_posts++;
         }
      }
   }
   return;
}

void do_makeboard( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;
   char filename[MAX_INPUT_LENGTH];

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: makeboard <filename>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mudstrlcpy( filename, argument, sizeof(filename) );
   smash_tilde( filename );
   if( strchr( filename, '/' ) || strchr( filename, '\\' ) || strstr( filename, ".." ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid board filename."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( board = first_board; board; board = board->next )
      if( !str_cmp( board->note_file, filename ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That board already exists."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

   CREATE( board, BOARD_DATA, 1 );

   LINK( board, first_board, last_board, next, prev );
   board->note_file = str_dup( strlower( filename ) );
   board->read_group = str_dup( "" );
   board->post_group = str_dup( "" );
   board->extra_readers = str_dup( "" );
   board->extra_removers = str_dup( "" );
   board->extra_ballots = str_dup( "" );
   board->min_ballot_level = LEVEL_IMMORTAL;
}

void do_bset( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;
   bool found;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_NOTE, ch );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: bset <board filename> <field> value"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\nField being one of:\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  ovnum read post remove maxpost filename type"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  read_group post_group extra_readers extra_removers extra_ballots"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The following will affect how an action is sent:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  oremove otake olist oread ocopy opost postmessg"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   value = atoi( argument );
   found = FALSE;
   for( board = first_board; board; board = board->next )
      if( !str_cmp( arg1, board->note_file ) )
      {
         found = TRUE;
         break;
      }
   if( !found )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Board not found."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "ovnum" ) )
   {
      if( !get_obj_index( value ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such object."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->board_obj = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (board's object vnum set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "read" ) )
   {
      if( value < 0 || value > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value outside valid character level range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->min_read_level = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (minimum reading level set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "read_group" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No reading group specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      DISPOSE( board->read_group );
      if( !str_cmp( argument, "none" ) )
         board->read_group = str_dup( "" );
      else
         board->read_group = str_dup( argument );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (reading group set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "post_group" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No posting group specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      DISPOSE( board->post_group );
      if( !str_cmp( argument, "none" ) )
         board->post_group = str_dup( "" );
      else
         board->post_group = str_dup( argument );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (posting group set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "postmessg" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->postmessg )
         DISPOSE( board->postmessg );
      if( !str_cmp( argument, "none" ) )
         board->postmessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->postmessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "opost" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->opostmessg )
         DISPOSE( board->opostmessg );
      if( !str_cmp( argument, "none" ) )
         board->opostmessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->opostmessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "oremove" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->oremovemessg )
         DISPOSE( board->oremovemessg );
      if( !str_cmp( argument, "none" ) )
         board->oremovemessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->oremovemessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "otake" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->otakemessg )
         DISPOSE( board->otakemessg );
      if( !str_cmp( argument, "none" ) )
         board->otakemessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->otakemessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "ocopy" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->ocopymessg )
         DISPOSE( board->ocopymessg );
      if( !str_cmp( argument, "none" ) )
         board->ocopymessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->ocopymessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "oread" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message sepcified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->oreadmessg )
         DISPOSE( board->oreadmessg );
      if( !str_cmp( argument, "none" ) )
         board->oreadmessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->oreadmessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "olist" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No message specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( board->olistmessg )
         DISPOSE( board->olistmessg );
      if( !str_cmp( argument, "none" ) )
         board->olistmessg = NULL;
      else
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s", argument );
         board->olistmessg = str_dup( buf );
      }
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !str_cmp( arg2, "extra_removers" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No names specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( argument, "none" ) )
         buf[0] = '\0';
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", board->extra_removers, argument );
      DISPOSE( board->extra_removers );
      board->extra_removers = str_dup( buf );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (extra removers set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "extra_readers" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No names specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( argument, "none" ) )
         buf[0] = '\0';
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", board->extra_readers, argument );
      DISPOSE( board->extra_readers );
      board->extra_readers = str_dup( buf );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (extra readers set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No filename specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( strchr( argument, '/' ) || strchr( argument, '\\' ) || strstr( argument, ".." ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid board filename."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      DISPOSE( board->note_file );
      board->note_file = str_dup( argument );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (board's filename set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "post" ) )
   {
      if( value < 0 || value > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value outside valid character level range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->min_post_level = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (minimum posting level set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "remove" ) )
   {
      if( value < 0 || value > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value outside valid character level range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->min_remove_level = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (minimum remove level set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "ballot" ) )
   {
      if( value < 0 || value > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value outside valid character level range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->min_ballot_level = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (minimum ballot read level set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "extra_ballots" ) )
   {
      if( argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No names specified."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( argument, "none" ) )
         buf[0] = '\0';
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", board->extra_ballots, argument );
      DISPOSE( board->extra_ballots );
      board->extra_ballots = str_dup( buf );
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (extra ballot readers set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "maxpost" ) )
   {
      if( value < 1 || value > 999 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value out of range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->max_posts = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (maximum number of posts set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( value < 0 || value > 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value out of range."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      board->type = value;
      write_boards_txt(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done.  (board's type set)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_bset( ch, "" );
   return;
}

void do_bstat( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;
   bool found;
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   found = FALSE;
   for( board = first_board; board; board = board->next )
      if( !str_cmp( arg, board->note_file ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
   {
      if( argument && argument[0] != '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&GBoard not found.  Usage: bstat <board filename>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         board = find_board( ch );
         if( !board )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&GNo board present.  Usage: bstat <board filename>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
   }

   ch_printf_color( ch,
                    "\r\n&GFilename: &W%-15.15s &GOVnum: &W%-5d &GRead: &W%-2d &GPost: &W%-2d &GRemove: &W%-2d\r\n&GMaxpost: &W%d &GBallot: &W%-2d   &GType: &W%d\r\n&GPosts:    %d\r\n",
                    board->note_file, board->board_obj, board->min_read_level, board->min_post_level,
                    board->min_remove_level, board->max_posts, board->min_ballot_level, board->type, board->num_posts );

   ch_printf_color( ch,
                    "&GRead_group:  &W%s\r\n&GPost_group:  &W%s\r\n&GExtra_readers:  &W%s\r\n&GExtra_ballots: &W%s\r\n&GExtra_removers: &W%s\r\n",
                    board->read_group, board->post_group, board->extra_readers, board->extra_ballots,
                    board->extra_removers );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GPost Message:    %s", board->postmessg ? board->postmessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GOPost Message:   %s", board->opostmessg ? board->opostmessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GORead Message:   %s", board->oreadmessg ? board->oreadmessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GORemove Message: %s", board->oremovemessg ? board->oremovemessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GOTake Message:   %s", board->otakemessg ? board->otakemessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GOList Message:   %s", board->olistmessg ? board->olistmessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GOCopy Message:   %s", board->ocopymessg ? board->ocopymessg : "Default Message"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_boards( CHAR_DATA* ch, const char* argument)
{
   BOARD_DATA *board;

   if( !first_board )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "T&Ghere are no boards yet."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   for( board = first_board; board; board = board->next )
      pager_printf_color( ch, "&G%-15.15s #: %5d Read: %2d Post: %2d Rmv: %2d Max: %3d Posts: &g%3d &GType: %d\r\n",
                          board->note_file, board->board_obj,
                          board->min_read_level, board->min_post_level,
                          board->min_remove_level, board->max_posts, board->num_posts, board->type );
}

void mail_count( CHAR_DATA * ch )
{
   BOARD_DATA *board;
   NOTE_DATA *note;
   int cnt_to = 0, cnt_from = 0;

   for( board = first_board; board; board = board->next )
      if( board->type == BOARD_MAIL && can_read( ch, board ) )
         for( note = board->first_note; note; note = note->next )
         {
            if( is_name( ch->name, note->to_list ) )
               ++cnt_to;
            else if( !str_cmp( ch->name, note->sender ) )
               ++cnt_from;
         }
   if( cnt_to )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have %d mail message%swaiting for you.", cnt_to, ( cnt_to > 1 ) ? "s " : " "); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   if( cnt_from )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have %d mail message%swritten by you.", cnt_from, ( cnt_from > 1 ) ? "s " : " "); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

/*
 * Journal command. Allows users to write notes to an object of type "journal".
 * Options are Write, Read and Size. Write and Read options require a numerical
 * argument. Option Size retrives v0 or value0 from the object, which is indicitive
 * of how many pages are in the journal.
 *
 * Forced a maximum limit of 50 pages to all journals, just incase someone slipped
 * with a value command and we ended up with an object that could store 500 pages.
 * This is added in journal write and journal size. Leart.
 */
void do_journal( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   EXTRA_DESCR_DATA *ed;

   OBJ_DATA *quill = NULL, *journal = NULL;

   int pages;
   int anum = 0;
   const char *text;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
      bug( "%s: no descriptor", __func__ );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_JOURNAL_WRITE:
         if( ( journal = get_eq_char( ch, WEAR_HOLD ) ) == NULL || journal->item_type != ITEM_JOURNAL )
         {
            bug( "%s: Player not holding journal. (Player: %s)", __func__, ch->name );
            stop_editing( ch );
            return;
         }
         ed = ( EXTRA_DESCR_DATA * )ch->dest_buf;
         if( !ed ) {stop_editing( ch ); return;}
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   if( arg1[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: Journal <command>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Where command is one of:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "write read size"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /* 
    * Write option. Allows user to enter the buffer, adding an extra_desc to the
    * journal object called "PageX" where X is the argument associated with the write command
    */
   if( !str_cmp( arg1, "write" ) )
   {
      if( ( journal = get_eq_char( ch, WEAR_HOLD ) ) == NULL || journal->item_type != ITEM_JOURNAL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be holding a journal in order to write in it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg2[0] == '\0' || !is_number( arg2 ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: Journal write <number>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( get_trust( ch ) < sysdata.write_mail_free )
      {
         quill = find_quill( ch );
         if( !quill )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a quill to write in your journal."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( quill->value[0] < 1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your quill is dry."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( journal->value[0] < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no pages in this journal. Seek an immortal for assistance."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /* Force a max value of 50 */
      if( journal->value[0] > 50 )
      {
         journal->value[0] = 50;
         bug( "%s: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", __func__, ch->name );
      }

      set_char_color( AT_GREY, ch );
      pages = journal->value[0];
      if( is_number( arg2 ) )
      {
         anum = atoi( arg2 );
      }

      if( anum < 1 || pages < anum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That page does not exist in this journal."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /* Making the edits turn out to be "page1" etc - just so people can't/don't type "look 1" */
      mudstrlcpy( buf, "page", MAX_STRING_LENGTH );
      mudstrlcat( buf, arg2, MAX_STRING_LENGTH );

      ed = SetOExtra( journal, buf );
      ch->substate = SUB_JOURNAL_WRITE;
      ch->dest_buf = ed;
      if( get_trust( ch ) < sysdata.write_mail_free )
         --quill->value[0];
      start_editing( ch, ed->description );
      journal->value[1]++;
      return;
   }

   /* Size option, returns how many pages are in the journal */
   if( !str_cmp( arg1, "size" ) )
   {
      if( ( journal = get_eq_char( ch, WEAR_HOLD ) ) == NULL || journal->item_type != ITEM_JOURNAL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be holding a journal in order to check it's size."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( journal->value[0] < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no pages in this journal. Seek an immortal for assistance."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
      {
         if( journal->value[0] > 50 )
         {
            journal->value[0] = 50;
            bug( "%s: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", __func__, ch->name );
         }
         set_char_color( AT_GREY, ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "There are %d pages in this journal.", journal->value[0]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   /* Read option. Players can read the desc on the journal by typing "look page1", but I thought about putting
    * in this option anyway.
    */
   if( !str_cmp( arg1, "read" ) )
   {
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: Journal read <number>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !is_number( arg2 ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: Journal read <number>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( is_number( arg2 ) )
      {
         anum = atoi( arg2 );
      }

      mudstrlcpy( buf, "page", MAX_STRING_LENGTH );
      mudstrlcat( buf, arg2, MAX_STRING_LENGTH );

      if( ( journal = get_eq_char( ch, WEAR_HOLD ) ) == NULL || journal->item_type != ITEM_JOURNAL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be holding a journal in order to read it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( journal->value[0] > 50 )
      {
         journal->value[0] = 50;
         bug( "%s: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", __func__, ch->name );
      }

      set_char_color( AT_GREY, ch );
      pages = journal->value[0];
      if( anum < 1 || pages < anum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That page does not exist in this journal."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( ( text = get_extra_descr( buf, journal->first_extradesc ) ) == NULL )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That journal page is blank."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         send_to_char_color( text, ch );
      return;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: Journal <command>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   send_to_char( "\r\n", ch );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Where command is one of:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "write read size"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
