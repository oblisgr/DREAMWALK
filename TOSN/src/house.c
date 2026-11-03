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
 *                     Housing Module Source File                           *
 ****************************************************************************
 * Author : Senir                                                           *
 * E-Mail : oldgaijin@yahoo.com                                             *
 * Rewrite for RoD - Edmond - during 10/01 and 11/01                        *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "mud.h"
#include "house.h"

bool check_parse_name( const char *name, bool newchar );
void wipe_resets( ROOM_INDEX_DATA * room );

HOMEBUY_DATA *first_homebuy;
HOMEBUY_DATA *last_homebuy;
HOME_DATA *first_home;
HOME_DATA *last_home;
ACCESSORIES_DATA *first_accessory;
ACCESSORIES_DATA *last_accessory;

void fwrite_house( HOME_DATA * homedata )
{
   FILE *fpout;
   ROOM_INDEX_DATA *location = NULL;
   OBJ_DATA *obj = NULL;
   short i, j = 0;
   char strsave[256];

   if( !homedata || !homedata->name || homedata->name[0] == '\0' )
   {
      bug( "%s: NULL homedata", __func__ );
      return;
   }

   snprintf( strsave, 256, "%s%s", HOUSE_DIR, capitalize( homedata->name ) );

   if( ( fpout = fopen( strsave, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for writing.", __func__, strsave );
      return;
   }

   fprintf( fpout, "#HOME\n" );
   fprintf( fpout, "Name        %s~\n", homedata->name );

   for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
   {
      if( homedata->vnum[i] > 0 )
         fprintf( fpout, "Vnum    %d\n", homedata->vnum[i] );
   }

   fprintf( fpout, "Apartment   %d\n", homedata->apartment );
   fprintf( fpout, "End\n\n" );

   for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
   {
      if( homedata->vnum[i] > 0 && ( location = get_room_index( homedata->vnum[i] ) ) != NULL )
      {
         obj = location->last_content;
         j = supermob->level;
         supermob->level = MAX_LEVEL;

         if( obj )
            fwrite_obj( supermob, obj, fpout, 0, OS_VAULT, FALSE );
         supermob->level = j;
      }
   }

   fprintf( fpout, "#END\n" );
   fclose( fpout );
   fpout = NULL;
}

void save_residence( ROOM_INDEX_DATA * location )
{
   AREA_DATA *tarea = NULL;
   char filename[256];

   if( !location )
   {
      bug( "%s: NULL Location", __func__ );
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      if( tarea == location->area )
      {
         fold_area( location->area, location->area->filename, FALSE );
         return;
      }
   }

   for( tarea = first_build; tarea; tarea = tarea->next )
   {
      if( tarea == location->area )
      {
         snprintf( filename, 256, "%s%s", BUILD_DIR, location->area->filename );
         fold_area( location->area, filename, FALSE );
         return;
      }
   }

   bug( "%s: Location doesn't have an area.", __func__ );
   return;
}

bool in_same_house( CHAR_DATA * ch, CHAR_DATA * vch )
{
   HOME_DATA *home;
   int x, y;

   if( IS_IMMORTAL( vch ) )
      return TRUE;

   for( home = first_home; home; home = home->next )
      for( x = 0; x < MAX_HOUSE_ROOMS; x++ )
         if( home->vnum[x] == ch->in_room->vnum )
         {
            for( y = 0; y < MAX_HOUSE_ROOMS; y++ )
               if( home->vnum[y] == vch->in_room->vnum )
                  return TRUE;

            return FALSE;
         }

   return FALSE;
}

void update_house_list(  )
{
   HOME_DATA *tmphome;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", HOUSE_DIR, HOUSE_LIST );

   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: FATAL: cannot open %s for writing!", __func__, filename );
      return;
   }

   for( tmphome = first_home; tmphome; tmphome = tmphome->next )
      fprintf( fpout, "%s\n", capitalize( tmphome->name ) );

   fprintf( fpout, "$\n" );
   fclose( fpout );
   fpout = NULL;
}

bool set_house( CHAR_DATA * ch, int vnum, bool apartment )
{
   HOME_DATA *tmphome, *shome = NULL;
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA *location = NULL;
   EXIT_DATA *pexit = NULL;
   OBJ_INDEX_DATA *obj, *key = NULL;

   if( !ch || !ch->name || vnum <= 0 )
      return FALSE;

   CREATE( tmphome, HOME_DATA, 1 );

   tmphome->name = STRALLOC( ch->name );
   tmphome->apartment = apartment;
   tmphome->vnum[0] = vnum;

   if( first_home )
   {
      bool found = FALSE;

      for( shome = first_home; shome; shome = shome->next )
      {
         if( strcmp( tmphome->name, shome->name ) < 0 )
         {
            INSERT( tmphome, shome, first_home, next, prev );
            found = TRUE;
            break;
         }
      }

      if( !found )
         LINK( tmphome, first_home, last_home, next, prev );
   }
   else
   {
      LINK( tmphome, first_home, last_home, next, prev );
   }

   update_house_list(  );
   fwrite_house( tmphome );

   if( ( location = get_room_index( vnum ) ) == NULL )
      return FALSE;

   if( apartment )
      snprintf( buf, MAX_STRING_LENGTH, "%s's Apartment", tmphome->name );
   else
      snprintf( buf, MAX_STRING_LENGTH, "%s's House", tmphome->name );

   STRFREE( location->name );
   location->name = STRALLOC( buf );
   STRFREE( location->description );
   if( apartment )
      location->description = STRALLOC( "A room surrounds you, filled with the dirt and filth of previous tenants.\r\nA spray painted frame of the last tenant can be seen on the floor in a \r\nfar corner.\r\n" );
   else
      location->description = STRALLOC( "This is your desc. You can edit this with HOUSE DESC.\r\n" );

   location->sector_type = 0;
   location->max_weight = 2000;
   xSET_BIT( location->room_flags, ROOM_NO_SUMMON );
   xSET_BIT( location->room_flags, ROOM_NO_ASTRAL );
   xSET_BIT( location->room_flags, ROOM_INDOORS );
   xSET_BIT( location->room_flags, ROOM_HOUSE );
   xREMOVE_BIT( location->room_flags, ROOM_PROTOTYPE );

   for( pexit = location->first_exit; pexit; pexit = pexit->next )
   {
      SET_BIT( pexit->exit_info, EX_ISDOOR );
      SET_BIT( pexit->exit_info, EX_CLOSED );
      SET_BIT( pexit->exit_info, EX_LOCKED );
      SET_BIT( pexit->exit_info, EX_NOPASSDOOR );
      SET_BIT( pexit->exit_info, EX_PICKPROOF );
      SET_BIT( pexit->exit_info, EX_BASHPROOF );

      pexit->key = location->vnum;
      add_reset( location, 'D', 0, location->vnum, pexit->vdir, 2 );

      if( pexit->rexit )
      {
         ROOM_INDEX_DATA *revroom;

         SET_BIT( pexit->rexit->exit_info, EX_ISDOOR );
         SET_BIT( pexit->rexit->exit_info, EX_CLOSED );
         SET_BIT( pexit->rexit->exit_info, EX_LOCKED );
         SET_BIT( pexit->rexit->exit_info, EX_NOPASSDOOR );
         SET_BIT( pexit->rexit->exit_info, EX_PICKPROOF );
         SET_BIT( pexit->rexit->exit_info, EX_BASHPROOF );

         pexit->rexit->key = location->vnum;

         if( ( revroom = get_room_index( pexit->vnum ) ) )
         {
            add_reset( revroom, 'D', 0, revroom->vnum, pexit->rexit->vdir, 2 );
            fold_area( revroom->area, revroom->area->filename, FALSE );
         }

      }

   }

/*	if (apartment)
		return TRUE; */

   if( ( obj = get_obj_index( location->vnum ) ) != NULL )
   {
      delete_obj( obj );
      save_residence( location );
   }

   snprintf( buf, MAX_STRING_LENGTH, "%s %s key", tmphome->name, ( apartment ? "apartment" : "house" ) );
   key = make_object( location->vnum, 0, buf );
   key->value[0] = location->vnum;
   key->item_type = ITEM_HOUSEKEY;
   key->level = 1;
   snprintf( buf, MAX_STRING_LENGTH, "%s's %s Key", tmphome->name, ( apartment ? "Apartment" : "House" ) );
   STRFREE( key->short_descr );
   key->short_descr = STRALLOC( buf );
   STRFREE( key->description );
   mudstrlcpy( buf, "A small golden key is here.", MAX_STRING_LENGTH );
   key->description = STRALLOC( buf );
   SET_BIT( key->wear_flags, ITEM_TAKE );
   SET_BIT( key->wear_flags, ITEM_HOLD );

   xREMOVE_BIT( key->extra_flags, ITEM_PROTOTYPE );

   save_residence( location );

   return TRUE;
}

bool remove_house( CHAR_DATA * ch )
{
   HOME_DATA *home;
   ROOM_INDEX_DATA *location;
   OBJ_INDEX_DATA *key;
   AREA_DATA *area;
   EXIT_DATA *pexit;
   short i;
   char filename[256];

   for( home = first_home; home; home = home->next )
      if( !str_cmp( home->name, ch->name ) )
         break;

   if( !home )
      return FALSE;

   if( ( location = get_room_index( home->vnum[0] ) ) == NULL )
      return FALSE;

   snprintf( filename, 256, "%s%s", HOUSE_DIR, capitalize( home->name ) );
   remove( filename );

   if( home->vnum[1] > 0 )
   {
      ROOM_INDEX_DATA *addloc;

      for( i = 1; i < MAX_HOUSE_ROOMS; i++ )
      {
         if( home->vnum[i] <= 0 )
            continue;

         if( ( addloc = get_room_index( home->vnum[i] ) ) == NULL )
         {
            home->vnum[i] = 0;
            continue;
         }

         area = addloc->area;

         for( pexit = addloc->first_exit; pexit; pexit = pexit->next )
         {
            if( pexit->rexit )
               extract_exit( pexit->to_room, pexit->rexit );
         }

         wipe_resets( addloc );

         home->vnum[i] = 0;

         delete_room( addloc );
         fold_area( area, area->filename, FALSE );
      }
   }

   if( ( key = get_obj_index( location->vnum ) ) != NULL )
   {
      delete_obj( key );
   }

   STRFREE( location->name );
   location->name = STRALLOC( "Vacant Residence" );
   STRFREE( location->description );
   location->description = STRALLOC( "" );

   xREMOVE_BIT( location->room_flags, ROOM_NO_SUMMON );
   xREMOVE_BIT( location->room_flags, ROOM_NO_ASTRAL );
   xREMOVE_BIT( location->room_flags, ROOM_INDOORS );
   xREMOVE_BIT( location->room_flags, ROOM_HOUSE );
   xSET_BIT( location->room_flags, ROOM_PROTOTYPE );

   for( pexit = location->first_exit; pexit; pexit = pexit->next )
   {
      pexit->key = -1;
      REMOVE_BIT( pexit->exit_info, EX_ISDOOR );
      REMOVE_BIT( pexit->exit_info, EX_CLOSED );
      REMOVE_BIT( pexit->exit_info, EX_LOCKED );
      REMOVE_BIT( pexit->exit_info, EX_NOPASSDOOR );
      REMOVE_BIT( pexit->exit_info, EX_PICKPROOF );
      REMOVE_BIT( pexit->exit_info, EX_BASHPROOF );

      if( pexit->rexit )
      {
         pexit->rexit->key = -1;
         REMOVE_BIT( pexit->rexit->exit_info, EX_ISDOOR );
         REMOVE_BIT( pexit->rexit->exit_info, EX_CLOSED );
         REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
         REMOVE_BIT( pexit->rexit->exit_info, EX_NOPASSDOOR );
         REMOVE_BIT( pexit->rexit->exit_info, EX_PICKPROOF );
         REMOVE_BIT( pexit->rexit->exit_info, EX_BASHPROOF );
      }
   }

   wipe_resets( location );

   save_residence( location );
   UNLINK( home, first_home, last_home, next, prev );
   DISPOSE( home );
   update_house_list(  );

   return TRUE;
}

bool add_room( HOME_DATA * home, ROOM_INDEX_DATA * location, const char *argument )
{
   ROOM_INDEX_DATA *addloc = NULL;
   AREA_DATA *pArea;
   EXIT_DATA *pexit, *rexit;
   char buf[MAX_STRING_LENGTH];
   int i;

   if( !location || !home || !argument || argument[0] == '\0' )
   {
      bug( "%s: passed null information", __func__ );
      return FALSE;
   }

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( !str_cmp( pArea->filename, ADDED_ROOM_HOUSING_AREA ) )
         break;
   }

   if( !pArea )
   {
      bug( "%s: housing area file not found in installed areas.", __func__ );
      return FALSE;
   }

   for( i = pArea->low_r_vnum; i < pArea->hi_r_vnum; i++ )
   {
      if( ( get_room_index( i ) ) == NULL )
      {
         if( ( addloc = make_room( i, pArea ) ) == NULL )
         {
            bug( "%s: make_room unable to complete.", __func__ );
            return FALSE;
         }
         break;
      }
   }

   if( !addloc )
   {
      bug( "%s: housing area appears to be full.", __func__ );
      return FALSE;
   }

   if( ( pexit = make_exit( location, addloc, get_dir( argument ) ) ) == NULL
       || ( rexit = make_exit( addloc, location, rev_dir[get_dir( argument )] ) ) == NULL )
   {
      delete_room( addloc );
      bug( "%s: exits not able to be created for room.  location deleted.", __func__ );
      return FALSE;
   }

   pexit->keyword = STRALLOC( "" );
   pexit->description = STRALLOC( "" );
   pexit->key = -1;
   pexit->exit_info = 0;
   rexit->keyword = STRALLOC( "" );
   rexit->description = STRALLOC( "" );
   rexit->key = -1;
   rexit->exit_info = 0;
   pexit->rexit = rexit;
   rexit->rexit = pexit;

   snprintf( buf, MAX_STRING_LENGTH, "%s's Additional House Room", capitalize( home->name ) );
   STRFREE( addloc->name );
   addloc->name = STRALLOC( buf );
   STRFREE( addloc->description );
   addloc->description = STRALLOC( "This is your desc. You can edit this with HOUSE DESC." );
   addloc->sector_type = 0;
   addloc->max_weight = 2000;
   xSET_BIT( addloc->room_flags, ROOM_NO_SUMMON );
   xSET_BIT( addloc->room_flags, ROOM_NO_ASTRAL );
   xSET_BIT( addloc->room_flags, ROOM_INDOORS );
   xSET_BIT( addloc->room_flags, ROOM_HOUSE );
   xREMOVE_BIT( addloc->room_flags, ROOM_PROTOTYPE );

   addloc->area = pArea;

   for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
   {
      if( home->vnum[i] <= 0 )
      {
         home->vnum[i] = addloc->vnum;
         break;
      }
   }

   fwrite_house( home );
   save_residence( addloc );

   if( addloc->area != location->area )
      save_residence( location );

   return TRUE;
}

bool give_key( CHAR_DATA * ch, int vnum, const char *name, bool apt )
{
   OBJ_INDEX_DATA *keyindex = NULL;
   OBJ_DATA *key = NULL;

   if( !ch || vnum <= 0 )
      return FALSE;

   if( ( keyindex = get_obj_index( vnum ) ) == NULL )
   {
      if( get_trust( ch ) >= LEVEL_GREATER )
      {
         char buf[MAX_STRING_LENGTH];

         snprintf( buf, MAX_STRING_LENGTH, "%s %s key", name, ( apt ? "apartment" : "house" ) );
         keyindex = make_object( vnum, 0, buf );
         keyindex->value[0] = vnum;
         keyindex->item_type = ITEM_HOUSEKEY;
         keyindex->level = 1;
         snprintf( buf, MAX_STRING_LENGTH, "%s's %s Key", name, ( apt ? "Apartment" : "House" ) );
         STRFREE( keyindex->short_descr );
         keyindex->short_descr = STRALLOC( buf );
         STRFREE( keyindex->description );
         keyindex->description = STRALLOC( buf );
         SET_BIT( keyindex->wear_flags, ITEM_TAKE );
         SET_BIT( keyindex->wear_flags, ITEM_HOLD );
         xREMOVE_BIT( keyindex->extra_flags, ITEM_PROTOTYPE );

         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Key previously not created. You must fold houses.are to save this key."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
         return FALSE;
   }

   if( ( key = create_object( keyindex, 1 ) ) == NULL )
      return FALSE;

   STRFREE( key->owner );
   key->owner = STRALLOC( name );
   obj_to_char( key, ch );

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A house key appears in your inventory."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   return TRUE;
}

void do_house( CHAR_DATA * ch, const char *argument )
{
   bool apt = FALSE;
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   ROOM_INDEX_DATA *location = NULL;
   HOME_DATA *homedata;
   short i;

   set_char_color( AT_GREEN, ch );

   switch ( ch->substate )
   {
      default:
         break;

      case SUB_ROOM_DESC:
         location = ( ROOM_INDEX_DATA * ) ch->dest_buf;
         if( !location )
         {
            bug( "%s: sub_room_desc: NULL ch->dest_buf", __func__ );
            location = ch->in_room;
         }
         STRFREE( location->description );
         location->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         save_residence( location );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "House room description set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
   }

   if( argument[0] == '\0' )
   {
      set_char_color( AT_GREEN, ch );

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " house <argument>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Where the argument is one of:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( " name <Desired Title of Room>\r\n desc\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " addroom <direction>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " givekey"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( IS_IMMORTAL( ch ) )
      {
         set_char_color( AT_WHITE, ch );
         send_to_char( "\r\nImmortal Arguments:\r\n", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " set <character name> <vnum> (apartment)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " set <character name> addroom <current house vnum> <direction>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " remove <character name>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " givekey <character name>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles do not have houses."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( homedata = first_home; homedata; homedata = homedata->next )
      if( !str_cmp( homedata->name, ch->name ) )
      {
         apt = homedata->apartment;
         break;
      }

   if( !homedata && get_trust( ch ) < LEVEL_DEMI )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not own a residence."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "givekey" ) )
   {
      if( get_trust( ch ) < LEVEL_DEMI )
      {
         if( ch->gold >= DEFAULT_MOB_PRICE )
            victim = ch;
         else
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have the necessary gold to make a key."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      else
      {
         if( !argument || argument[0] == '\0' )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax:  house givekey <character name>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( ( victim = get_char_world( ch, argument ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That character is not online."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         for( homedata = first_home; homedata; homedata = homedata->next )
            if( !str_cmp( homedata->name, victim->name ) )
               break;

         if( !homedata )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They do not own a home."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !give_key( ch, homedata->vnum[0], homedata->name, homedata->apartment ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A key was not able to be made."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch == victim )
         ch->gold -= DEFAULT_MOB_PRICE;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The requested key has been made."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( homedata )
   {
      for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
         if( homedata->vnum[i] == ch->in_room->vnum )
         {
            location = ch->in_room;
            break;
         }

      if( !location )
         if( !IS_IMMORTAL( ch ) || ( location = get_room_index( homedata->vnum[0] ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be in your house to make changes to it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

      if( !str_cmp( arg, "desc" ) )
      {
         ch->tempnum = SUB_NONE;
         ch->substate = SUB_ROOM_DESC;
         ch->dest_buf = location;
         log_printf( "%s: house desc", capitalize( ch->name ) );
         start_editing( ch, location->description );
         return;
      }

      if( !argument || argument[0] == '\0' )
      {
         do_house( ch, "" );
         return;
      }

      if( !str_cmp( arg, "name" ) )
      {
         STRFREE( location->name );
         location->name = STRALLOC( argument );
         save_residence( location );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Room title set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         log_printf( "%s: house %s %s", capitalize( ch->name ), arg, argument );
         return;
      }

      if( !str_cmp( arg, "addroom" ) && !apt )
      {
         if( ch->gold < ADDITIONAL_ROOM_COST )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have enough money for an additional room."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
            if( homedata->vnum[i] == 0 )
               break;

         if( i == MAX_HOUSE_ROOMS )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You currently have the maximum number of additional rooms."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( ( get_exit( location, get_dir( argument ) ) ) != NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "An exit already exists in that direction."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( !add_room( homedata, location, argument ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "An exit could not be created.  You may need to contact an immortal."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         ch->gold -= ADDITIONAL_ROOM_COST;
         log_printf( "%s: house %s %s", capitalize( ch->name ), arg, argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Additional room added."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( get_trust( ch ) < LEVEL_GREATER )
   {
      do_house( ch, "" );
      return;
   }

   set_char_color( AT_WHITE, ch );
   log_printf( "%s: house %s %s", capitalize( ch->name ), arg, argument );

   if( !str_cmp( arg, "set" ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      char arg3[MAX_INPUT_LENGTH];

      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );

      if( arg2[0] == '\0' || arg3[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: house set <character name> <vnum> (apartment)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "        house set <character name> addroom <current vnum> <direction>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( victim = get_char_world( ch, arg2 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That character is not online."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles can not have residences."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      for( homedata = first_home; homedata; homedata = homedata->next )
         if( !str_cmp( homedata->name, victim->name ) )
         {
            apt = homedata->apartment;
            break;
         }

      if( !str_cmp( arg3, "addroom" ) )
      {
         bool found = FALSE;

         argument = one_argument( argument, arg3 );
         if( argument[0] == '\0' || arg3[0] == '\0' || !is_number( arg3 ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: house set <character name> addroom <current vnum> <direction>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( !homedata )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They do not have a house."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         else if( apt )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Apartments cannot have additional rooms."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( ( location = get_room_index( atoi( arg3 ) ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That location does not exist."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
         {
            if( homedata->vnum[i] == atoi( arg3 ) )
               found = TRUE;
            if( homedata->vnum[i] == 0 )
               break;
         }

         if( !found )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That vnum is not part of their house."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( i == MAX_HOUSE_ROOMS )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That character already has the maximum number of additional rooms."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( ( get_exit( location, get_dir( argument ) ) ) != NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "An exit already exists in that direction."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( !add_room( homedata, location, argument ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Error: Room could not be created."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Additional room added to that house."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( homedata )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They already have a house."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( location = get_room_index( atoi( arg3 ) ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room does not exist."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( argument && argument[0] != '\0' && !str_cmp( argument, "apartment" ) )
         apt = TRUE;

      if( !set_house( victim, atoi( arg3 ), apt ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That residence could not be created."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Residence created successfully."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "remove" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: house remove <character name>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( victim = get_char_world( ch, argument ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That character is not online."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Moblies cannot have residences."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      for( homedata = first_home; homedata; homedata = homedata->next )
         if( !str_cmp( homedata->name, victim->name ) )
            break;

      if( !homedata )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That character does not appear to have a residence."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !remove_house( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The residence could not be removed."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Residence removed successfully."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_house( ch, "" );
   return;
}

void do_gohome( CHAR_DATA * ch, const char *argument )
{
   HOME_DATA *home;
   ROOM_INDEX_DATA *room;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->desc )
      return;

   for( home = first_home; home; home = home->next )
      if( !str_cmp( home->name, ch->name ) )
         break;

   if( !home )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not own a home."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( room = get_room_index( home->vnum[0] ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your home no longer exists."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A magical force prevents you from returning home."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->position )
   {
      default:
         break;
      case POS_DEAD:
      case POS_MORTAL:
      case POS_INCAP:
      case POS_STUNNED:
         send_to_char_color( "&RYou are hurt far too badly for that!", ch );
         return;
      case POS_SLEEPING:
      case POS_RESTING:
      case POS_SITTING:
         do_stand( ch, "" );
         break;
      case POS_BERSERK:
      case POS_AGGRESSIVE:
      case POS_FIGHTING:
      case POS_DEFENSIVE:
      case POS_EVASIVE:
      case POS_SHOVE:
      case POS_DRAG:
         send_to_char_color( "&RNo way! You are too busy!", ch );
         return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "%s disappears in a vortex of luminescent light.", ch->name );
   act( AT_GREY, buf, ch, NULL, ch, TO_ROOM );

   char_from_room( ch );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, room );
   }
   char_to_room( ch, room );

   snprintf( buf, MAX_STRING_LENGTH, "As a vortex of luminescent light forms, %s slides gracefully into their home.\r\n", ch->name );
   act( AT_GREY, buf, ch, NULL, ch, TO_ROOM );

   do_look( ch, "" );
   return;
}

void do_residence( CHAR_DATA * ch, const char *argument )
{
   HOME_DATA *home;
   ROOM_INDEX_DATA *room;
   short i, z = 1;

   if( !first_home )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are currently no residences."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G                               Current Residences"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&O###&G|   &OPlayer Name   &G|         &OHousing Area         &G|&OType&G| &ORooms &G| &OStart Vnum &G|"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G-------------------------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   for( home = first_home; home; home = home->next )
   {
      if( ( room = get_room_index( home->vnum[0] ) ) == NULL )
      {
         bug( "%s: Residence: NULL start room vnum - C:%s V:%d", __func__, home->name, home->vnum[0] );
         continue;
      }

      for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
         if( home->vnum[i] <= 0 )
            break;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&w%3d&G| &w%-15.15s &G| &w%-28.28s &G|  &w%s &G| &w%5d &G| &w%10d &G|", z, home->name, room->area->name, home->apartment ? "A" : "H", i, home->vnum[0]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      z++;
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G-------------------------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_accessories( CHAR_DATA * ch, const char *argument )
{
   ACCESSORIES_DATA *acc;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   short i = 0;

   set_char_color( AT_GREEN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs to not own homes to furnish."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  accessories <argument>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\nWhere <argument> is one of:\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  list"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  buy <accessory #>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  show <accessory #>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( get_trust( ch ) >= LEVEL_GREATER )
      {
         set_char_color( AT_WHITE, ch );

         send_to_char( "\r\nImmortal Arguments:\r\n", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  add <vnum>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  add <vnum> (mob)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  remove <accessory #>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  setprice <accessory #> <cost>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg, "list" ) )
   {
      OBJ_INDEX_DATA *obj;
      MOB_INDEX_DATA *mob;
      char *name;

      if( !first_accessory )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are currently no accessories."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      i = 1;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G                House Accessories"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( !IS_IMMORTAL( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&g## &G|        &gAccessory Name        &G|   &gPrice    &G|"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&g## &G|        &gAccessory Name        &G|   &gPrice    &G|    &gVnum    &G|"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G-------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }

      for( acc = first_accessory; acc; acc = acc->next )
      {
         if( !acc->mob )
         {
            if( ( obj = get_obj_index( acc->vnum ) ) == NULL )
            {
               bug( "%s: Accessories list: object vnum %d does not exist.", __func__, acc->vnum );
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&g%3d&G| &RAccessory currently unavailable.", i); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               ++i;
               continue;
            }
            name = str_dup( obj->short_descr );
         }
         else
         {
            if( ( mob = get_mob_index( acc->vnum ) ) == NULL )
            {
               bug( "%s: Accessories list: mob vnum %d does not exist.", __func__, acc->vnum );
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&g%3d&G| &RAccessory currently unavailable.", i); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               ++i;
               continue;
            }
            name = str_dup( mob->short_descr );
         }

         if( !IS_IMMORTAL( ch ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&g%3d&G| &g%-28.28s &G| &g%10d &G|", i, name, acc->price); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         else
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&g%3d&G| &g%-28.28s &G| &g%10d &G| &g%10d &G|", i, name, acc->price, acc->vnum); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         ++i;
      }

      if( !IS_IMMORTAL( ch ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G-------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      do_accessories( ch, "" );
      return;
   }

   if( str_cmp( arg, "show" ) )
   {
      log_printf( "%s: accessories %s %s", capitalize( ch->name ), arg, argument );
   }

   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg, "add" ) && ( get_trust( ch ) >= LEVEL_GREATER ) )
   {
      OBJ_INDEX_DATA *obj = NULL;
      MOB_INDEX_DATA *mob = NULL;
      int vnum;
      ACCESSORIES_DATA *newacc;

      if( arg2[0] == '\0' || !is_number( arg2 ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: accessories add <vnum> (mob)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      vnum = atoi( arg2 );

      if( argument && !str_cmp( argument, "mob" ) )
      {
         if( ( mob = get_mob_index( vnum ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mob does not exist."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !mob && ( obj = get_obj_index( vnum ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object does not exist."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      for( acc = first_accessory; acc; acc = acc->next )
      {
         if( acc->vnum == vnum )
         {
            if( mob && acc->mob )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mob is already an accessory."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }
         else
         {
            if( !mob && !acc->mob )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object is already an accessory."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }
      }

      CREATE( newacc, ACCESSORIES_DATA, 1 );
      LINK( newacc, first_accessory, last_accessory, next, prev );

      newacc->vnum = vnum;
      if( mob )
      {
         newacc->price = DEFAULT_MOB_PRICE;
         newacc->mob = TRUE;
      }
      else
      {
         newacc->price = obj->cost;
         newacc->mob = FALSE;
      }

      save_accessories(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Accessory added."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( acc = first_accessory; acc; acc = acc->next )
      if( is_number( arg2 ) && ++i == atoi( arg2 ) )
         break;

   if( !acc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not a valid accessory."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "buy" ) )
   {
      HOME_DATA *home;
      MOB_INDEX_DATA *mobindex;
      OBJ_INDEX_DATA *objindex;
      ROOM_INDEX_DATA *location = NULL;

      for( home = first_home; home; home = home->next )
         if( !str_cmp( home->name, ch->name ) )
            break;

      if( !home || home->apartment )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need a house to buy accessories."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->gold < acc->price )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have enough gold."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
      {
         if( ch->in_room->vnum == home->vnum[i] )
         {
            location = ch->in_room;
            break;
         }
      }

      if( !location )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be in your house to furnish it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( acc->mob && ( mobindex = get_mob_index( acc->vnum ) ) != NULL )
      {
         CHAR_DATA *mob;

         mob = create_mobile( mobindex );
         char_to_room( mob, location );
         add_reset( location, 'M', 1, mobindex->vnum, mobindex->count, location->vnum );
         save_residence( location );
      }
      else if( !acc->mob && ( objindex = get_obj_index( acc->vnum ) ) != NULL )
      {
         OBJ_DATA *obj;

         obj = create_object( objindex, objindex->level );
         obj_to_room( obj, location );
      }
      else
      {
         char buf[MAX_STRING_LENGTH];

         sprintf( buf, "Accessory buy: accessory does not exist - V: %d  #: %d", acc->vnum, i );
         bug( buf, 0 );
         return;
      }

      ch->gold -= acc->price;
      fwrite_house( home );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "show" ) )
   {
      OBJ_INDEX_DATA *obj;
      MOB_INDEX_DATA *mob;

      if( acc->mob && ( mob = get_mob_index( acc->vnum ) ) != NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&cMob Accessory Information"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gName: &G%s", mob->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         if( get_trust( ch ) > LEVEL_CREATOR )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gVnum: &G%d", mob->vnum); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gRace: &G%s", ( mob->race < MAX_NPC_RACE && mob->race >= 0 ? npc_race[mob->race] : "unknown" )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gClass: &G%s", ( mob->Class < MAX_NPC_CLASS && mob->Class >= 0 ? npc_class[mob->Class] : "unknown" )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gAverage Hp: &G%d", ( !mob->hitnodice ? ( mob->level * 8 + number_range( mob->level * mob->level / 4, mob->level * mob->level ) )
                                            : ( mob->hitnodice * number_range( 1, mob->hitsizedice ) + mob->hitplus ) )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gAverage AC: &G%d", ( mob->ac ? mob->ac : interpolate( mob->level, 100, -100 ) )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&gNumber of Attacks(Per Round): &G%d", mob->numattacks); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

         return;
      }
      else if( !acc->mob && ( obj = get_obj_index( acc->vnum ) ) != NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&C&cObject Accessory Information"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C&gName: &G%s", obj->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         if( get_trust( ch ) > LEVEL_CREATOR )
            ch_printf_color( ch, "&gVnum: &G%d", obj->vnum );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C&gType: &G%s", o_types[obj->item_type]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C&gFlags: &G%s", ext_flag_string( &obj->extra_flags, o_flags )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C&gMagical Flags: &G%s", magic_bit_name( obj->magic_flags )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C&gWeight: &G%d", obj->weight); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         bug( "%s: Accessory show: accessory does not exist - V: %d  #: %d", __func__, acc->vnum, i );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Error: Accessory doesn't exist."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( get_trust( ch ) < LEVEL_GREATER )
   {
      do_accessories( ch, "" );
      return;
   }

   if( !str_cmp( arg, "remove" ) )
   {
      UNLINK( acc, first_accessory, last_accessory, next, prev );

      acc->vnum = 0;
      acc->price = 0;
      acc->mob = FALSE;
      DISPOSE( acc );
      save_accessories(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "setprice" ) )
   {
      if( !argument || argument[0] == '\0' || !is_number( argument ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: accessories setprice <#> <cost>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      acc->price = atoi( argument );
      save_accessories(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_accessories( ch, "" );
   return;
}

void save_homebuy(  )
{
   FILE *fpout;
   HOMEBUY_DATA *tmphome = NULL;

   if( ( fpout = fopen( HOMEBUY_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for writing", __func__, HOMEBUY_FILE );
      perror( HOMEBUY_FILE );
      return;
   }

   for( tmphome = first_homebuy; tmphome; tmphome = tmphome->next )
   {
      if( ( get_room_index( tmphome->vnum ) ) == NULL )
         continue;

      fprintf( fpout, "#HOMEBUY\n" );
      fprintf( fpout, "Apartment   %d\n", tmphome->apartment );
      fprintf( fpout, "Bid         %d\n", tmphome->bid );
      fprintf( fpout, "Bidder      %s~\n", tmphome->bidder );
      fprintf( fpout, "BidIncPerc  %d\n", tmphome->incpercent );
      fprintf( fpout, "Endtime     %d\n", tmphome->endtime );
      fprintf( fpout, "Seller      %s~\n", tmphome->seller );
      fprintf( fpout, "Vnum        %d\n", tmphome->vnum );
      fprintf( fpout, "End\n\n" );
   }

   fprintf( fpout, "#END\n" );
   fclose( fpout );
   fpout = NULL;
}

bool add_homebuy( CHAR_DATA * seller, int vnum, bool apartment, int price )
{
   HOMEBUY_DATA *tmphome = NULL;

   if( vnum <= 0 || !seller || IS_NPC( seller ) )
      return FALSE;

   for( tmphome = first_homebuy; tmphome; tmphome = tmphome->next )
   {
      if( tmphome->vnum == vnum )
         break;
   }

   if( tmphome )
      return FALSE;

   if( ( get_room_index( vnum ) ) == NULL )
      return FALSE;

   CREATE( tmphome, HOMEBUY_DATA, 1 );
   LINK( tmphome, first_homebuy, last_homebuy, next, prev );

   tmphome->vnum = vnum;
   tmphome->apartment = apartment;
   tmphome->endtime = ( 7 * 48 );
   tmphome->incpercent = DEFAULT_BID_INCREMENT_PERCENTAGE;
   tmphome->bidder = STRALLOC( "None" );
   tmphome->seller = STRALLOC( seller->name );

   if( price <= 0 )
   {
      if( apartment )
         tmphome->bid = MIN_APARTMENT_BID;
      else
         tmphome->bid = MIN_HOUSE_BID;
   }
   else
   {
      tmphome->bid = price;
   }

   save_homebuy(  );

   return TRUE;
}

bool remove_homebuy( HOMEBUY_DATA * tmphome )
{
   if( !tmphome )
      return FALSE;

   STRFREE( tmphome->bidder );
   STRFREE( tmphome->seller );
   tmphome->incpercent = 0;
   tmphome->apartment = FALSE;
   tmphome->bid = 0;
   tmphome->vnum = 0;
   tmphome->endtime = 0;

   UNLINK( tmphome, first_homebuy, last_homebuy, next, prev );

   DISPOSE( tmphome );

   save_homebuy(  );

   return TRUE;
}

void do_homebuy( CHAR_DATA * ch, const char *argument )
{
   HOMEBUY_DATA *homeb;
   HOME_DATA *home;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int i = 0;
   int bidinc;

   set_char_color( AT_GREEN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs cannot purchase residences."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' || !str_cmp( argument, "list" ) )
   {
      ROOM_INDEX_DATA *location = NULL;
      int days;
      char name[MAX_INPUT_LENGTH];
      char area[MAX_INPUT_LENGTH];

      if( !first_homebuy )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No residences are currently on auction."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&G                 Residence Auction"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&c##&C| &cResidence Name       &C| &cHousing Area   &C|&cCurr. Bid &C| &cBid Inc. &C|&cRemaining Time&C|"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&C--------------------------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      i = 0;

      for( homeb = first_homebuy; homeb; homeb = homeb->next )
      {
         i++;

         if( ( location = get_room_index( homeb->vnum ) ) == NULL )
         {
            bug( "%s: Homebuy list: location does not exist.  -V:%d  #:%d", __func__, homeb->vnum, i );
            ch_printf_color( ch, "&c%2d&C| &RThis residence not currently available.", i );
            continue;
         }

         strcpy( area, location->area->name );
         if( homeb->apartment )
            snprintf( name, MAX_INPUT_LENGTH, "(A) %s", location->name );
         else
            mudstrlcpy( name, location->name, MAX_INPUT_LENGTH );

         bidinc = homeb->incpercent * ( homeb->bid / 100 );
         days = homeb->endtime / 48;

         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&c%2d&C|&c%-22.22s&C|&c%-16.16s&C|&c%10d&C|&c%10d&C|&c%2d &CDays &c%2d &CHrs|", i, name, area, homeb->bid, bidinc, days, ( ( homeb->endtime - ( days * 48 ) ) / 2 )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&C--------------------------------------------------------------------------------"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg, "syntax" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " homebuy <argument>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Where the argument is one of:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " list"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " syntax"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " show <# of auction item>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " bid <# of auction item> <amount>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( IS_IMMORTAL( ch ) )
      {
         set_char_color( AT_WHITE, ch );
         send_to_char( "\r\nImmortal Arguments:\r\n", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " add <residence vnum> (apartment)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " remove <# of auction item> "); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " setbid <# of auction item> <amount>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " bidincrement <# of auction item>  <percent>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " timeremainder <# of auction item> <days> (<hours>)"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", " clearbidder <# of auction item>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      }
      return;
   }

   if( arg2[0] == '\0' || !is_number( arg2 ) )
   {
      do_homebuy( ch, "syntax" );
      return;
   }

   if( !str_cmp( arg, "add" ) && get_trust( ch ) >= LEVEL_GREATER )
   {
      int vnum = atoi( arg2 );
      ROOM_INDEX_DATA *location;
      bool apartment = FALSE;

      if( argument[0] != '\0' && !str_cmp( argument, "apartment" ) )
         apartment = TRUE;

      if( ( location = get_room_index( vnum ) ) != NULL )
      {
         if( xIS_SET( location->room_flags, ROOM_HOUSE ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room is already a residence.  Use \"sellhouse\" or first remove that player's house."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      if( !add_homebuy( ch, vnum, apartment, 0 ) )
      {
         send_to_char( "Vnum could not be added to the auction.\r\n"
                       "Vnum may already be on auction or vnum does not exist.\r\n", ch );
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Residence added to auction successfully."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( homeb = first_homebuy; homeb; homeb = homeb->next )
      if( ++i == atoi( arg2 ) )
         break;

   if( !homeb )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not a valid auction #."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   bidinc = homeb->incpercent * ( homeb->bid / 100 );

   if( !str_cmp( arg, "show" ) )
   {
      ROOM_INDEX_DATA *location;
      int days;

      if( ( location = get_room_index( homeb->vnum ) ) == NULL )
      {
         bug( "%s: Homebuy show: location does not exist. -V: %d, #: %d", __func__, homeb->vnum, i );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That residence does not exist.  Please contact an immortal."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      days = homeb->endtime / 48;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&GHome Auction Residence Information"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CName: &c%s", location->name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CType: &c%s", ( homeb->apartment ? "Apartment" : "House" )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CHousing Area: &c%s", location->area->name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CCurrent Bid: &c%d", homeb->bid); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CBid Increment: &c%d", bidinc); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CTime Left: &c%d &CDays &c%d &CHours", days, ( ( homeb->endtime - ( days * 48 ) ) / 2 )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( IS_IMMORTAL( ch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CSeller: &c%s", homeb->seller); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CBidder: &c%s", homeb->bidder); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CVnum: &c%d", homeb->vnum); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg, "bid" ) )
   {
      HOMEBUY_DATA *checkhome;
      int bid;

      if( !argument || argument[0] == '\0' || !is_number( argument ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: homebuy bid <# of auction> <amount>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      bid = atoi( argument );

      for( home = first_home; home; home = home->next )
         if( !str_cmp( home->name, ch->name ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You already own a home. You must sell that one first."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

      for( checkhome = first_homebuy; checkhome; checkhome = checkhome->next )
         if( !str_cmp( checkhome->bidder, ch->name ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are already bidding on a home."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

      if( !str_cmp( homeb->seller, ch->name ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot bid on your own home."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( homeb->bid + bidinc ) < 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot bid more than the current bid."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( bid < homeb->bid || bid < ( homeb->bid + bidinc ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must bid more than the current bid plus the bid increment."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->gold < bid )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have that much gold."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      STRFREE( homeb->bidder );
      homeb->bidder = STRALLOC( ch->name );
      homeb->bid = bid;
      save_homebuy(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have successfully placed a bid."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < LEVEL_GREATER )
   {
      do_homebuy( ch, "syntax" );
      return;
   }

   if( !str_cmp( arg, "remove" ) )
   {
      if( !remove_homebuy( homeb ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Residence could not be removed from auction."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Residence removed from auction successfully."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "clearbidder" ) )
   {
      STRFREE( homeb->bidder );
      homeb->bidder = STRALLOC( "None" );
      save_homebuy(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Bidder cleared."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "timeremainder" ) )
   {
      char arg3[MAX_INPUT_LENGTH];
      short hours, days;

      argument = one_argument( argument, arg3 );

      if( arg3[0] == '\0' || !is_number( arg3 ) || argument[0] == '\0' || !is_number( argument ) )
      {
         do_homebuy( ch, "syntax" );
         return;
      }

      hours = atoi( argument );
      days = atoi( arg3 );
      if( days > 31 || days < 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Days can be from 0 to 31."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( hours < 0 || hours > 23 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Hours can be from 0 to 23."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      homeb->endtime = ( days * 48 ) + ( hours * 2 );
      save_homebuy(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Time remainder set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' || !is_number( argument ) )
   {
      do_homebuy( ch, "syntax" );
      return;
   }

   if( !str_cmp( arg, "setbid" ) )
   {
      homeb->bid = atoi( argument );
      save_homebuy(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Bid set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "bidincrement" ) )
   {
      homeb->incpercent = atoi( argument );
      save_homebuy(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Bid increment precentage set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_homebuy( ch, "syntax" );
   return;
}

void do_sellhouse( CHAR_DATA * ch, const char *argument )
{
   HOME_DATA *home;
   int bid;

   set_char_color( AT_GREEN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs cannot sell homes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( home = first_home; home; home = home->next )
      if( !str_cmp( home->name, ch->name ) )
         break;

   if( !home )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have a home to sell."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Options for Sellhouse Command:"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  Sellhouse yes"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "    Places your residence on auction with default starting bid."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  Sellhouse <amount>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "   Places your residence on auction with the given amount as starting bid."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   bid = ( is_number( argument ) ? atoi( argument ) : 0 );

   if( str_cmp( argument, "yes" ) )
   {
      if( ( home->apartment && bid < MIN_APARTMENT_BID ) || bid < MIN_HOUSE_BID )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Bid amount must be equal to or greater than the minimum bid for the residence type."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !add_homebuy( ch, home->vnum[0], home->apartment, bid ) )
   {
      send_to_char( "Your residence could not be added to the auction.\r\n"
                    "Please contact an immortal for assistance.\r\n", ch );
      return;
   }

   send_to_char( "You have successfully placed your residence on auction for the given starting bid.\r\n"
                 "Profit from the sale should occur when your house has been bought on auction.\r\n"
                 "Loss of your residence should also occur at that time.\r\n", ch );
   return;
}

int fread_house( FILE * fp )
{
   short i = 0;
   HOME_DATA *homedata = NULL;

   CREATE( homedata, HOME_DATA, 1 );

   for( ;; )
   {
      const char *word;
      bool fMatch;

      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {

         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !homedata->name )
               {
                  bug( "%s: NULL Name", __func__ );
                  DISPOSE( homedata );
                  return -1;
               }

               LINK( homedata, first_home, last_home, next, prev );

               return ( homedata->vnum[0] <= 0 ? 2 : homedata->vnum[0] );
            }
            break;

         case 'A':
            KEY( "Apartment", homedata->apartment, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", homedata->name, fread_string( fp ) );
            break;

         case 'V':
            if( !str_cmp( word, "Vnum" ) )
            {
               int vnum;

               fMatch = TRUE;
               vnum = fread_number( fp );
               if( !get_room_index( vnum ) )
               {
                  DISPOSE( homedata );
                  return -1;
               }
               homedata->vnum[i++] = vnum;
            }
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

static OBJ_DATA *rgObjNest[MAX_NEST];

bool load_house_file( const char *name )
{
   FILE *fp;
   char filename[256];
   int iNest, vnum = 0;
   OBJ_DATA *obj = NULL;
   ROOM_INDEX_DATA *pRoom = NULL;

   snprintf( filename, 256, "%s%s", HOUSE_DIR, name );

   if( ( fp = fopen( filename, "r" ) ) == NULL )
      return FALSE;

   for( iNest = 0; iNest < MAX_NEST; iNest++ )
      rgObjNest[iNest] = NULL;

   for( ;; )
   {
      char letter;
      char *word;

      letter = fread_letter( fp );

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      if( letter != '#' )
      {

         bug( "%s: # not found in %s.", __func__, filename );
         break;
      }

      word = fread_word( fp );

      if( !str_cmp( word, "HOME" ) )
      {
         pRoom = get_room_index( vnum = fread_house( fp ) );
         if( !pRoom )
         {
            bug( "%s: bad house vnum in %s.  Vnum %d", __func__, filename, vnum );
            return FALSE;
         }
         rset_supermob( pRoom );
         continue;
      }
      else if( !str_cmp( word, "OBJECT" ) )
      {
         fread_obj( supermob, fp, OS_VAULT );
         continue;
      }
      else if( !str_cmp( word, "END" ) )
      {
         if( !pRoom )
            return FALSE;
         break;
      }
      else
      {
         bug( "%s: bad section: %s in file %s.", __func__, word, filename );
         break;
      }
   }

   fclose( fp );
   fp = NULL;

   while( ( obj = supermob->first_carrying ) != NULL )
   {
      obj_from_char( obj );
      obj_to_room( obj, pRoom );
   }

   release_supermob(  );

   return TRUE;
}

void load_homedata(  )
{
   FILE *fpList;
   const char *filename;
   char list[256];

   first_home = NULL;
   last_home = NULL;

   snprintf( list, 256, "%s%s", HOUSE_DIR, HOUSE_LIST );

   if( ( fpList = fopen( list, "r" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for reading. Home loading aborted.", __func__, HOUSE_LIST );
      return;
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );

      if( filename[0] == '$' )
         break;

      if( !load_house_file( filename ) )
      {
         bug( "%s: Cannot load house file: %s", __func__, filename );
      }

   }

   fclose( fpList );
   fpList = NULL;
   return;
}

void save_house_by_vnum( int vnum )
{
   HOME_DATA *tmphome = NULL;
   short i = 0;

   for( tmphome = first_home; tmphome; tmphome = tmphome->next )
      if( !tmphome->apartment )
         for( i = 0; i < MAX_HOUSE_ROOMS; i++ )
            if( tmphome->vnum[i] == vnum )
               fwrite_house( tmphome );

   return;
}

void save_accessories(  )
{
   FILE *fpout;
   ACCESSORIES_DATA *tmpacc = NULL;
   OBJ_INDEX_DATA *obj = NULL;
   MOB_INDEX_DATA *mob = NULL;

   if( ( fpout = fopen( ACCESSORIES_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for writing", __func__, ACCESSORIES_FILE );
      perror( ACCESSORIES_FILE );
      return;
   }

   for( tmpacc = first_accessory; tmpacc; tmpacc = tmpacc->next )
   {
      if( !tmpacc->mob )
      {
         if( ( obj = get_obj_index( tmpacc->vnum ) ) == NULL )
            continue;
      }
      else
      {
         if( ( mob = get_mob_index( tmpacc->vnum ) ) == NULL )
            continue;
      }

      fprintf( fpout, "#ACCESSORIES\n" );
      fprintf( fpout, "Vnum        %d\n", tmpacc->vnum );
      fprintf( fpout, "Price       %d\n", tmpacc->price );
      fprintf( fpout, "Mob         %d\n", tmpacc->mob );
      fprintf( fpout, "End\n\n" );
   }

   fprintf( fpout, "#END\n" );
   fclose( fpout );
   fpout = NULL;
}

void load_accessories(  )
{
   FILE *fp;

   if( ( fp = fopen( ACCESSORIES_FILE, "r" ) ) == NULL )
   {
      bug( "%s: Cannot open %s. Aborting loadup of accessories data.", __func__, ACCESSORIES_FILE );
      return;
   }

   for( ;; )
   {
      char letter;
      char *word;

      letter = fread_letter( fp );

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      if( letter != '#' )
      {
         bug( "%s: # not found.", __func__ );
         break;
      }

      word = fread_word( fp );

      if( !str_cmp( word, "ACCESSORIES" ) )
      {
         fread_accessories( fp );
         continue;
      }
      else if( !str_cmp( word, "END" ) )
         break;
      else
      {
         bug( "%s: bad section.", __func__ );
         continue;
      }
   }

   fclose( fp );
   fp = NULL;
}

void fread_accessories( FILE * fp )
{
   ACCESSORIES_DATA *newacc = NULL;

   CREATE( newacc, ACCESSORIES_DATA, 1 );

   for( ;; )
   {
      const char *word;
      bool fMatch;

      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {

         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( ( newacc->mob && ( get_mob_index( newacc->vnum ) ) == NULL )
                   || ( !newacc->mob && ( get_obj_index( newacc->vnum ) ) == NULL ) )
               {
                  bug( "%s: Accessory doesn't exist", __func__ );
                  DISPOSE( newacc );
                  return;
               }

               LINK( newacc, first_accessory, last_accessory, next, prev );

               return;
            }
            break;

         case 'M':
            KEY( "Mob", newacc->mob, fread_number( fp ) );
            break;


         case 'P':
            KEY( "Price", newacc->price, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Vnum", newacc->vnum, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

void fread_homebuy( FILE * fp )
{
   HOMEBUY_DATA *newhome = NULL;

   CREATE( newhome, HOMEBUY_DATA, 1 );

   for( ;; )
   {
      const char *word;
      bool fMatch;

      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {

         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Apartment", newhome->apartment, fread_number( fp ) );
            break;

         case 'B':
            KEY( "Bid", newhome->bid, fread_number( fp ) );
            KEY( "Bidder", newhome->bidder, fread_string( fp ) );
            KEY( "BidIncPerc", newhome->incpercent, fread_number( fp ) );
            break;

         case 'E':
            KEY( "Endtime", newhome->endtime, fread_number( fp ) );

            if( !str_cmp( word, "End" ) )
            {
               if( ( get_room_index( newhome->vnum ) ) == NULL )
               {
                  bug( "%s: Residence doesn't exist", __func__ );
                  DISPOSE( newhome );
                  return;
               }

               LINK( newhome, first_homebuy, last_homebuy, next, prev );

               return;
            }

            break;

         case 'S':
            KEY( "Seller", newhome->seller, fread_string( fp ) );
            break;

         case 'V':
            KEY( "Vnum", newhome->vnum, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

void load_homebuy(  )
{
   FILE *fp;

   if( ( fp = fopen( HOMEBUY_FILE, "r" ) ) == NULL )
   {
      bug( "%s: Cannot open %s. Aborting loadup of home auction data.", __func__, HOMEBUY_FILE );
      return;
   }

   for( ;; )
   {
      char letter;
      char *word;

      letter = fread_letter( fp );

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      if( letter != '#' )
      {
         bug( "%s: # not found.", __func__ );
         break;
      }

      word = fread_word( fp );

      if( !str_cmp( word, "HOMEBUY" ) )
      {
         fread_homebuy( fp );
         continue;
      }
      else if( !str_cmp( word, "END" ) )
         break;
      else
      {
         bug( "%s: bad section: %s", __func__, word );
         continue;
      }
   }

   fclose( fp );
   fp = NULL;
}

CHAR_DATA *load_player( const char *pname )
{
   CHAR_DATA *onlinechar = NULL;
   DESCRIPTOR_DATA *d = NULL;
   struct stat fst;
   char filename[256];
   char name[256];
   int oldvnum = 0;

   mudstrlcpy( name, pname, 256 );
   snprintf( filename, 256, "%s%c/%s", PLAYER_DIR, LOWER( name[0] ), capitalize( name ) );

   for( onlinechar = first_char; onlinechar; onlinechar = onlinechar->next )
   {
      if( IS_NPC( onlinechar ) )
         continue;

      if( !str_cmp( onlinechar->name, name ) )
         return onlinechar;
   }

   if( !check_parse_name( name, FALSE ) )
      return NULL;

   if( stat( filename, &fst ) == -1 )
      return NULL;

   CREATE( d, DESCRIPTOR_DATA, 1 );
   d->next = NULL;
   d->prev = NULL;
   d->connected = CON_GET_NAME;
   d->outsize = 2000;
   CREATE( d->outbuf, char, d->outsize );
   load_char_obj( d, name, FALSE, FALSE );
   add_char( d->character );

   oldvnum = ( ( get_room_index( d->character->in_room->vnum ) ) != NULL ) ? d->character->in_room->vnum : ROOM_VNUM_LIMBO;

   char_to_room( d->character, get_room_index( oldvnum ) );

   d->character->retran = oldvnum;
   d->character->desc = NULL;
   if( d->character->pcdata->prev_site )
      d->character->pcdata->recent_site = STRALLOC( d->character->pcdata->prev_site );
   d->character = NULL;
   DISPOSE( d->outbuf );
   DISPOSE( d );

   for( onlinechar = first_char; onlinechar; onlinechar = onlinechar->next )
   {
      if( IS_NPC( onlinechar ) )
         continue;

      if( !str_cmp( onlinechar->name, name ) )
         return onlinechar;
   }
   return NULL;
}

void logoff( CHAR_DATA * ch )
{
   int x, y;

   x = y = 0;

   if( !ch || IS_NPC( ch ) || ch->desc != NULL || ch->switched != NULL )
      return;

   if( ch->position == POS_MOUNTED )
      do_dismount( ch, "" );

   set_char_color( AT_GREY, ch );

   if( !ch->in_room || ( get_room_index( ch->in_room->vnum ) ) == NULL )
      ch->in_room = get_room_index( ROOM_VNUM_LIMBO );

   quitting_char = ch;
   save_char_obj( ch );

   if( sysdata.save_pets && ch->pcdata && ch->pcdata->pet )
      extract_char( ch->pcdata->pet, TRUE );

   if( ch->pcdata && ch->pcdata->clan )
      save_clan( ch->pcdata->clan );

   saving_char = NULL;

   extract_char( ch, TRUE );

   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         save_equipment[x][y] = NULL;

   return;
}

void homebuy_update(  )
{
   HOMEBUY_DATA *home, *nexthome;
   CHAR_DATA *seller = NULL, *bidder = NULL;

   for( home = first_homebuy; home; home = nexthome )
   {
      nexthome = home->next;

      if( --home->endtime > 0 && ( ( home->bid + ( home->incpercent * ( home->bid / 100 ) ) ) > 0 ) )
      {
         save_homebuy(  );
         continue;
      }

      if( home->bidder[0] != '\0' && str_cmp( home->bidder, "None" ) && ( bidder = load_player( home->bidder ) ) != NULL )
      {
         if( bidder->gold < home->bid )
         {
            if( ( bidder->gold -= ( home->bid / 100 * PENALTY_PERCENTAGE ) ) < 0 )
               bidder->gold = 0;
            home->endtime = 144;
            STRFREE( home->bidder );
            home->bidder = STRALLOC( "None" );

            if( bidder->desc )
               send_to_char( "You did not have enough money for the residence you bid on.\r\n"
                             "It has been readded to the auction and you have been penalized.\r\n", bidder );
            else
               add_loginmsg( bidder->name, 1, NULL );

            save_homebuy(  );
            logoff( bidder );
            continue;
         }
      }

      if( home->seller[0] == '\0' || ( seller = load_player( home->seller ) ) == NULL )
      {
         bug( "%s: Seller of residence with vnum %d could not be found.", __func__, home->vnum );

         if( !remove_homebuy( home ) )
         {
            bug( "%s: this is an error", __func__ );
            return;
         }

         if( bidder->desc )
            send_to_char( "There was an error while looking up the seller for the residence you had\r\n"
                          "bid on.  That residence was removed and no interaction has taken place.\r\n", bidder );
         else
            add_loginmsg( bidder->name, 2, NULL );

         continue;
      }

      if( !bidder )
      {
         if( ( seller->gold -= home->bid / 100 * PENALTY_PERCENTAGE ) < 0 )
            seller->gold = 0;

         if( seller->desc )
            send_to_char( "There was no bid on your residence.  It has been removed from auction\r\n"
                          "and you have been penalized.\r\n", seller );
         else
            add_loginmsg( seller->name, 3, NULL );

         remove_homebuy( home );
         logoff( seller );
         continue;
      }

      if( get_trust( seller ) < LEVEL_GREATER )
         if( !remove_house( seller ) )
         {
            bug( "%s: residence could not be removed from seller after sale.", __func__ );
            bug( "%s: Vnum: %d  Seller: %s   Bidder: %s", __func__, home->vnum, seller->name, bidder->name );

            logoff( bidder );
            logoff( seller );
            continue;
         }

      if( !set_house( bidder, home->vnum, home->apartment ) )
      {
         bug( "%s: residence could not be setup after sale.", __func__ );
         bug( "%s: Vnum: %d  Seller: %s   Bidder: %s", __func__, home->vnum, seller->name, bidder->name );

         logoff( bidder );
         logoff( seller );
         continue;
      }

      seller->gold += home->bid;
      bidder->gold -= home->bid;

      log_printf( "%s: Home successfully sold B:%s S:%s V:%d", __func__, bidder->name, seller->name, home->vnum );


      if( !give_key( bidder, home->vnum, bidder->name, home->apartment ) )
      {
         bug( "%s: Key for residence with vnum %d could not be given.", __func__, home->vnum );
      }

      remove_homebuy( home );

      if( bidder->desc )
         send_to_char( "You have successfully received your new residence.\r\n", bidder );
      else
         add_loginmsg( bidder->name, 4, NULL );

      if( seller->desc )
         send_to_char( "You have successfully sold your residence.\r\n", seller );
      else
         add_loginmsg( seller->name, 5, NULL );

      logoff( bidder );
      logoff( seller );
   }
   return;
}
