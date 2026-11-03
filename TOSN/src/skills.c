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
 *                          Player skills module                            *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mud.h"

bool validate_spec_fun								(const char *name);
void remove_bexit_flag								(EXIT_DATA *pexit, int flag);
const char *const spell_flag[] 			= { 
	"water", 	"earth", 		"air", 			"astral", 		"area", 		"distant", "reverse",
	"noself", 	"_unused2_", 	"accumulative", "recastable", 	"noscribe",
	"nobrew", 	"group", 		"object", 		"character", 	"secretskill", 	"pksensitive",
	"stoponfail", "nofight", "nodispel", "randomtarget", "nomob", 
	"r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
};
const char *const spell_saves[] 	  	= {"none", "poison_death", "wands", "para_petri", "breath", "spell_staff"};
const char *const spell_save_effect[] 	= {"none", "negate", "eightdam", "quarterdam", "halfdam", "3qtrdam", "reflect", "absorb"};
const char *const spell_damage[] 	  	= {"none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };
const char *const spell_action[] 	 	= {"none", "create", "destroy", "resist", "suscept", "divinate", "obscure", "change"};
const char *const spell_power[]  	  	= {"none", "minor", "greater", "major"};
const char *const spell_class[]  	 	= {"none", "lunar", "solar", "travel", "summon", "life", "death", "illusion"};
const char *const target_type[]  	  	= {"ignore", "offensive", "defensive", "self", "objinv"};
void show_char_to_char								(CHAR_DATA *list, CHAR_DATA *ch);
int ris_save										(CHAR_DATA *ch, int schance, int ris);
bool check_illegal_psteal							(CHAR_DATA *ch, CHAR_DATA *victim);
void failed_casting									(struct skill_type *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj);
void free_skill										(SKILLTYPE *skill){
	SMAUG_AFF *aff, *aff_next;

	if(skill->first_affect)
	{
		for(aff = skill->first_affect; aff; aff = aff_next)
		{
			aff_next = aff->next;
			UNLINK(aff, skill->first_affect, skill->last_affect, next, prev);
			DISPOSE(aff->duration);
			DISPOSE(aff->modifier);
			DISPOSE(aff );
		}
	}
	
	DISPOSE( skill->name 		);
	DISPOSE( skill->noun_damage );
	DISPOSE( skill->msg_off 	);
	DISPOSE( skill->hit_char 	);
	DISPOSE( skill->hit_vict 	);
	DISPOSE( skill->hit_room 	);
	DISPOSE( skill->hit_dest 	);
	DISPOSE( skill->miss_char 	);
	DISPOSE( skill->miss_vict 	);
	DISPOSE( skill->miss_room 	);
	DISPOSE( skill->die_char 	);
	DISPOSE( skill->die_vict 	);
	DISPOSE( skill->die_room 	);
	DISPOSE( skill->imm_char 	);
	DISPOSE( skill->imm_vict 	);
	DISPOSE( skill->imm_room 	);
	DISPOSE( skill->dice 		);
	
	if(skill->spell_fun_name) {DISPOSE(skill->spell_fun_name);}
	if(skill->skill_fun_name) {DISPOSE(skill->skill_fun_name);}
	
	DISPOSE( skill->components );
	DISPOSE( skill->teachers );
	skill->spell_fun = NULL;
	skill->skill_fun = NULL;
	DISPOSE( skill );

	return;
}
void free_skills									(void){
	SKILLTYPE *skill;
	int hash = 0;

	for( hash = 0; hash < num_skills; ++hash )
	{
		skill = skill_table[hash];
		free_skill( skill );
		skill_table_bytype[hash] = NULL;
	}

	for( hash = 0; hash < top_herb; hash++ )
	{
		skill = herb_table[hash];
		free_skill( skill );
	}
	return;
}
void skill_notfound							   		(CHAR_DATA *ch, const char *argument){
	send_to_char( "Huh?\r\n", ch );
	return;
}
int get_ssave										(const char *name){
	unsigned int x;
	for( x = 0; x < sizeof( spell_saves) / sizeof(spell_saves[0]); x++)
		if(!str_cmp( name, spell_saves[x])) return x;
	return -1;
}
int get_starget										(const char *name){
	unsigned int x;
	for(x = 0; x < sizeof(target_type) / sizeof(target_type[0] ); x++)
		if(!str_cmp(name, target_type[x])) return x;
	return -1;
}
int get_sflag										(const char *name){
	unsigned int x;
	for(x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++)
		if(!str_cmp(name, spell_flag[x])) return x;
	return -1;
}
int get_sdamage										(const char *name){
	unsigned int x;

	for(x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++)
		if(!str_cmp(name, spell_damage[x])) return x;
	return -1;
}
int get_saction										(const char *name){
	unsigned int x;
	for(x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++)
		if(!str_cmp(name, spell_action[x])) return x;
	return -1;
}
int get_ssave_effect								(const char *name){
   unsigned int x;
	for(x = 0; x < sizeof(spell_save_effect) / sizeof(spell_save_effect[0]); x++)
		if(!str_cmp(name, spell_save_effect[x]))return x;
	return -1;
}
int get_spower										(const char *name){
	unsigned int x;
	for(x = 0; x < sizeof(spell_power) / sizeof(spell_power[0] ); x++)
		if(!str_cmp(name, spell_power[x])) return x;
	return -1;
}
int get_sclass										(const char *name){
	unsigned int x;
	for(x = 0; x < sizeof(spell_class) / sizeof(spell_class[0]); x++)
		if(!str_cmp(name, spell_class[x])) return x;
	return -1;
}
bool is_legal_kill									(CHAR_DATA *ch, CHAR_DATA *vch){
   if(IS_NPC(ch) || IS_NPC(vch)) 									return TRUE;
   if(!IS_PKILL(ch) || !IS_PKILL(vch)) 								return FALSE;
   if(ch->pcdata->clan && ch->pcdata->clan == vch->pcdata->clan) 	return FALSE;
   return TRUE;
}
bool check_ability					  				(CHAR_DATA *ch, char *command, char *argument){
	int sn, mana, blood;
	struct timeval time_used;

	//bsearch for the ability
	sn = find_ability( ch, command, TRUE );

	if(sn == -1) {return FALSE;}

	//some additional checks
	if(!( skill_table[sn]->skill_fun || (skill_table[sn]->spell_fun && skill_table[sn]->spell_fun != spell_null)) || !can_use_skill(ch, 0, sn)) {return FALSE;}
	if(!check_pos(ch, skill_table[sn]->minimum_position)) 														{return TRUE;}

	if( IS_NPC(ch) && (IS_AFFECTED( ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_POSSESS)))
	{
		send_to_char( "For some reason, you seem unable to perform that...\r\n", ch );
		act( AT_GREY, "$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
		return TRUE;
	}

	//Check if mana is required
	if(skill_table[sn]->min_mana)
	{
		mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana, 100 / UMAX(1, 2 + ch->level - skill_table[sn]->race_level[ch->race]));
		blood = (mana / 2);
	  
		if(!IS_NPC(ch) && IS_VAMPIRE(ch))
		{
			if(ch->pcdata->condition[COND_BLOODTHIRST] < blood)
			{
				send_to_char("You don't have enough blood power.\r\n", ch);
				return TRUE;
			}
		}
		else if( !IS_NPC( ch ) && ch->mana < mana )
		{
			send_to_char("You don't have enough mana.\r\n", ch);
			return TRUE;
		}
	}
	else
	{
		mana = 0;
		blood = 0;
	}

	//Is this a real do-fun, or a really a spell?
	if(!skill_table[sn]->skill_fun)
	{
		ch_ret retcode 		= rNONE;
		void *vo 			= NULL;
		CHAR_DATA *victim 	= NULL;
		OBJ_DATA *obj 		= NULL;
		target_name 		= "";

		switch (skill_table[sn]->target)
		{
			default:
				bug("Check_ability: bad target for sn %d.", sn);
				send_to_char("Something went wrong...\r\n", ch);
			return TRUE;

			case TAR_IGNORE:
				vo = NULL;
				if(!argument || argument[0] == '\0')
				{
					if( (victim = who_fighting(ch)) != NULL)
					target_name = victim->name;
				}
				else
				target_name = argument;
            break;

			case TAR_CHAR_OFFENSIVE:
			{
				if((!argument || argument[0] == '\0') && !(victim = who_fighting(ch)))
				{
					ch_printf(ch, "Confusion overcomes you as your '%s' has no target.\r\n", skill_table[sn]->name);
					return TRUE;
				}
				else if( argument[0] != '\0' && !( victim = get_char_room( ch, argument ) ) )
				{
					send_to_char("They aren't here.\r\n", ch);
					return TRUE;
				}
			}
			
            if(is_safe(ch, victim, TRUE)) return TRUE;

            if(ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
				send_to_char("You can't target yourself!\r\n", ch);
				return TRUE;
            }

			if(!IS_NPC(ch))
            {
               if(!IS_NPC(victim))
               {
					if(get_timer(ch, TIMER_PKILLED) > 0)
					{
						send_to_char("You have been killed in the last 5 minutes.\r\n", ch);
						return TRUE;
					}

					if(get_timer(victim, TIMER_PKILLED) > 0)
					{
						send_to_char("This player has been killed in the last 5 minutes.\r\n", ch);
						return TRUE;
					}


					//Too many illegal pk attempts with cuffs, swats etc lately. - Luc 
					if(xIS_SET(ch->act, PLR_NICE))
					{
						send_to_char("You feel too nice to do that.\r\n", ch);
						return TRUE;
					}

					if(victim != ch)
                    send_to_char( "You really shouldn't do this to another player...\r\n", ch );
				}

				if(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
				{
					send_to_char("You can't do that on your own follower.\r\n", ch);
					return TRUE;
				}
            }

            if(check_illegal_pk(ch, victim))
            {
				send_to_char( "You can't do that to another player!\r\n", ch );
				return TRUE;
            }
            vo = ( void * )victim;
            break;

			case TAR_CHAR_DEFENSIVE:
			{
				if(argument[0] != '\0' && !(victim = get_char_room(ch, argument)))
				{
					send_to_char("They aren't here.\r\n", ch);
					return TRUE;
				}
				if(!victim) victim = ch;
			}

            if(ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
				send_to_char("You can't target yourself!\r\n", ch);
				return TRUE;
            }
            vo = (void *)victim;
            break;

			case TAR_CHAR_SELF:
				victim = ch;
				vo = (void *)ch;
				break;

			case TAR_OBJ_INV:
			{
				if( !(obj = get_obj_carry(ch, argument)))
				{
					send_to_char( "You can't find that.\r\n", ch);
				return TRUE;
				}
			}
            vo = (void *)obj;
            break;
		}

		//Waitstate
		WAIT_STATE( ch, skill_table[sn]->beats );
		
		//Check for failure
		if( ( number_percent(  ) + skill_table[sn]->difficulty * 5 ) > ( IS_NPC( ch ) ? 75 : LEARNED( ch, sn ) ) )
		{
			failed_casting( skill_table[sn], ch, victim, obj );
			
			if(mana)
			{
				if(!IS_NPC(ch) && IS_VAMPIRE(ch)) 
				gain_condition(ch, COND_BLOODTHIRST, -blood / 2);
            else
               ch->mana -= mana / 2;
			}
			
			return TRUE;
		}
		
		if(mana)
		{
			if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
            gain_condition( ch, COND_BLOODTHIRST, -blood );
			else
            ch->mana -= mana;
		}
		
		start_timer( &time_used );
		retcode = ( *skill_table[sn]->spell_fun ) ( sn, ch->level, ch, vo );
		end_timer( &time_used );
		update_userec( &time_used, &skill_table[sn]->userec );

		if(retcode == rCHAR_DIED || retcode == rERROR) 	{return TRUE;}
		if(char_died(ch))								{return TRUE;}

		if( retcode == rSPELL_FAILED )
		{
			retcode = rNONE;
		}
		else
			ability_learn_from_success( ch, sn );

		if( skill_table[sn]->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died( victim ) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			for( vch = ch->in_room->first_person; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;
					if( victim == vch && !victim->fighting && victim->master != ch )
					{
						retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
						break;
					}
			}
		}
		return TRUE;
	}

	if( mana )
	{
		if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
        gain_condition( ch, COND_BLOODTHIRST, -blood );
		else
        ch->mana -= mana;
	}
	ch->prev_cmd = ch->last_cmd;  /* haus, for automapping */
	ch->last_cmd = skill_table[sn]->skill_fun;
	start_timer( &time_used );
	(*skill_table[sn]->skill_fun) (ch, argument);
	end_timer( &time_used );
	update_userec( &time_used, &skill_table[sn]->userec );

	tail_chain(  );
	return TRUE;
}
bool check_skill					  				(CHAR_DATA *ch, char *command, char *argument){
   int sn, mana, blood;
   struct timeval time_used;

   /*
    * bsearch for the skill
    */
   sn = find_skill( ch, command, TRUE );

   if( sn == -1 )
      return FALSE;

   // some additional checks
   if( !( skill_table[sn]->skill_fun || (skill_table[sn]->spell_fun && skill_table[sn]->spell_fun != spell_null) ) || !can_use_skill( ch, 0, sn ) )
   {
      return FALSE;
   }

   if( !check_pos( ch, skill_table[sn]->minimum_position ) )
      return TRUE;

   if( IS_NPC( ch ) && ( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
   {
      send_to_char( "For some reason, you seem unable to perform that...\r\n", ch );
      act( AT_GREY, "$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
      return TRUE;
   }

   /*
    * check if mana is required 
    */
   if( skill_table[sn]->min_mana )
   {
      mana = IS_NPC( ch ) ? 0 : UMAX( skill_table[sn]->min_mana,
                                      100 / UMAX(1, 2 + ch->level - skill_table[sn]->skill_level[ch->Class]) );
      blood = UMAX( 1, ( mana + 4 ) / 8 );   /* NPCs don't have PCDatas. -- Altrag */
      if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
      {
         if( ch->pcdata->condition[COND_BLOODTHIRST] < blood )
         {
            send_to_char( "You don't have enough blood power.\r\n", ch );
            return TRUE;
         }
      }
      else if( !IS_NPC( ch ) && ch->mana < mana )
      {
         send_to_char( "You don't have enough mana.\r\n", ch );
         return TRUE;
      }
   }
   else
   {
      mana = 0;
      blood = 0;
   }

   /*
    * Is this a real do-fun, or a really a spell?
    */
   if( !skill_table[sn]->skill_fun )
   {
      ch_ret retcode = rNONE;
      void *vo = NULL;
      CHAR_DATA *victim = NULL;
      OBJ_DATA *obj = NULL;

      target_name = "";

      switch ( skill_table[sn]->target )
      {
         default:
            bug( "Check_skill: bad target for sn %d.", sn );
            send_to_char( "Something went wrong...\r\n", ch );
            return TRUE;

         case TAR_IGNORE:
            vo = NULL;
            if( argument[0] == '\0' )
            {
               if( ( victim = who_fighting( ch ) ) != NULL )
                  target_name = victim->name;
            }
            else
               target_name = argument;
            break;

         case TAR_CHAR_OFFENSIVE:
         {
            if( argument[0] == '\0' && ( victim = who_fighting( ch ) ) == NULL )
            {
               ch_printf( ch, "Confusion overcomes you as your '%s' has no target.\r\n", skill_table[sn]->name );
               return TRUE;
            }
            else if( argument[0] != '\0' && ( victim = get_char_room( ch, argument ) ) == NULL )
            {
               send_to_char( "They aren't here.\r\n", ch );
               return TRUE;
            }
         }
            if( is_safe( ch, victim, TRUE ) )
               return TRUE;

            if( ch == victim && SPELL_FLAG( skill_table[sn], SF_NOSELF ) )
            {
               send_to_char( "You can't target yourself!\r\n", ch );
               return TRUE;
            }

            if( !IS_NPC( ch ) )
            {
               if( !IS_NPC( victim ) )
               {
                  /*
                   * Sheesh! can't do anything
                   * send_to_char( "You can't do that on a player.\r\n", ch );
                   * return TRUE;
                   */
                  if( get_timer( ch, TIMER_PKILLED ) > 0 )
                  {
                     send_to_char( "You have been killed in the last 5 minutes.\r\n", ch );
                     return TRUE;
                  }

                  if( get_timer( victim, TIMER_PKILLED ) > 0 )
                  {
                     send_to_char( "This player has been killed in the last 5 minutes.\r\n", ch );
                     return TRUE;
                  }

                  if( victim != ch )
                     send_to_char( "You really shouldn't do this to another player...\r\n", ch );
               }

               if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
               {
                  send_to_char( "You can't do that on your own follower.\r\n", ch );
                  return TRUE;
               }
            }

            check_illegal_pk( ch, victim );
            vo = ( void * )victim;
            break;

         case TAR_CHAR_DEFENSIVE:
         {
            if( argument[0] != '\0' && ( victim = get_char_room( ch, argument ) ) == NULL )
            {
               send_to_char( "They aren't here.\r\n", ch );
               return TRUE;
            }
            if( !victim )
               victim = ch;
         }

            if( ch == victim && SPELL_FLAG( skill_table[sn], SF_NOSELF ) )
            {
               send_to_char( "You can't target yourself!\r\n", ch );
               return TRUE;
            }

            vo = ( void * )victim;
            break;

         case TAR_CHAR_SELF:
            vo = ( void * )ch;
            break;

         case TAR_OBJ_INV:
         {
            if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
            {
               send_to_char( "You can't find that.\r\n", ch );
               return TRUE;
            }
         }
            vo = ( void * )obj;
            break;
      }

      /*
       * waitstate 
       */
      WAIT_STATE( ch, skill_table[sn]->beats );
      /*
       * check for failure 
       */
      if( ( number_percent(  ) + skill_table[sn]->difficulty * 5 ) > ( IS_NPC( ch ) ? 75 : LEARNED( ch, sn ) ) )
      {
         failed_casting( skill_table[sn], ch, victim, obj );
         if( mana )
         {
            if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
               gain_condition( ch, COND_BLOODTHIRST, -blood / 2 );
            else
               ch->mana -= mana / 2;
         }
         return TRUE;
      }
      if( mana )
      {
         if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
            gain_condition( ch, COND_BLOODTHIRST, -blood );
         else
            ch->mana -= mana;
      }
      start_timer( &time_used );
      retcode = ( *skill_table[sn]->spell_fun ) ( sn, ch->level, ch, vo );
      end_timer( &time_used );
      update_userec( &time_used, &skill_table[sn]->userec );

      if( retcode == rCHAR_DIED || retcode == rERROR )
         return TRUE;

      if( char_died( ch ) )
         return TRUE;

      if( retcode == rSPELL_FAILED )
      {
         retcode = rNONE;
      }
      else
         learn_from_success( ch, sn );

      if( skill_table[sn]->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died( victim ) )
      {
         CHAR_DATA *vch;
         CHAR_DATA *vch_next;

         for( vch = ch->in_room->first_person; vch; vch = vch_next )
         {
            vch_next = vch->next_in_room;
            if( victim == vch && !victim->fighting && victim->master != ch )
            {
               retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
               break;
            }
         }
      }
      return TRUE;
   }

   if( mana )
   {
      if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
         gain_condition( ch, COND_BLOODTHIRST, -blood );
      else
         ch->mana -= mana;
   }
   ch->prev_cmd = ch->last_cmd;  /* haus, for automapping */
   ch->last_cmd = skill_table[sn]->skill_fun;
   start_timer( &time_used );
   ( *skill_table[sn]->skill_fun ) ( ch, argument );
   end_timer( &time_used );
   update_userec( &time_used, &skill_table[sn]->userec );

   tail_chain(  );
   return TRUE;
}
void do_skin								   		(CHAR_DATA *ch, const char *argument){
   OBJ_INDEX_DATA *korps;
   OBJ_DATA *corpse;
   OBJ_DATA *obj;
   OBJ_DATA *skin;
   const char *name;
   char buf[MAX_STRING_LENGTH];

   if( !IS_PKILL( ch ) && !IS_IMMORTAL( ch ) )
   {
      send_to_char( "Leave the hideous defilings to the killers!\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Whose corpse do you wish to skin?\r\n", ch );
      return;
   }

   if( ( corpse = get_obj_here( ch, argument ) ) == NULL )
   {
      send_to_char( "You cannot find that here.\r\n", ch );
      return;
   }

   if( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
   {
      send_to_char( "You have no weapon with which to perform this deed.\r\n", ch );
      return;
   }

   if( !IS_IMMORTAL( ch ) && !IS_OBJ_STAT( corpse, ITEM_CLANCORPSE ) )
   {
      send_to_char( "You can only skin the bodies of player characters.\r\n", ch );
      return;
   }

   if( ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3 && obj->value[3] != 11 ) || IS_OBJ_STAT( corpse, ITEM_SKINNED ) )
   {
      send_to_char( "There is nothing you can do with this corpse.\r\n", ch );
      return;
   }

   if( !IS_IMMORTAL( ch ) )
   {
      if( IS_OBJ_STAT( corpse, ITEM_CLANCORPSE ) )
      {
         if( corpse->action_desc && is_name( ch->name, corpse->action_desc ) );
         else
         {
            send_to_char( "There is nothing you can do with this corpse.\r\n", ch );
            return;
         }
      }
      else
      {
         send_to_char( "There is nothing you can do with this corpse.\r\n", ch );
         return;
      }
   }

   if( get_obj_index( OBJ_VNUM_SKIN ) == NULL )
   {
      bug( "Vnum %d (OBJ_VNUM_SKIN) not found for do_skin!", OBJ_VNUM_SKIN );
      return;
   }

   if( !( korps = get_obj_index( OBJ_VNUM_CORPSE_PC ) ) )
   {
      bug( "Vnum %d (OBJ_VNUM_CORPSE_PC) not found for %s!", OBJ_VNUM_CORPSE_PC, __func__ );
      return;
   }

   skin = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 );
   name = IS_NPC( ch ) ? korps->short_descr : corpse->short_descr;
   snprintf( buf, MAX_STRING_LENGTH, skin->short_descr, name );
   STRFREE( skin->short_descr );
   skin->short_descr = STRALLOC( buf );
   snprintf( buf, MAX_STRING_LENGTH, skin->description, name );
   STRFREE( skin->description );
   skin->description = STRALLOC( buf );
   act( AT_BLOOD, "$n strips the skin from $p.", ch, corpse, NULL, TO_ROOM );
   act( AT_BLOOD, "You strip the skin from $p.", ch, corpse, NULL, TO_CHAR );
   separate_obj( corpse );
   xSET_BIT( corpse->extra_flags, ITEM_SKINNED );
   obj_to_char( skin, ch );
   return;
}
void do_slookup								   		(CHAR_DATA *ch, const char *argument){
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   int sn;
   int iClass, iRace;
   SKILLTYPE *skill = NULL;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Slookup what?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      for( sn = 0; sn < num_skills && skill_table[sn] && skill_table[sn]->name; ++sn )
         pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\r\n",
                       sn, skill_table[sn]->slot, skill_table[sn]->name, spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
   }
   else if( !str_cmp( arg, "herbs" ) )
   {
      for( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
         pager_printf( ch, "%d) %s\r\n", sn, herb_table[sn]->name );
   }
   else
   {
      SMAUG_AFF *aff;
      int cnt = 0;

      if( arg[0] == 'h' && is_number( arg + 1 ) )
      {
         sn = atoi( arg + 1 );
         if( !IS_VALID_HERB( sn ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid herb."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         skill = herb_table[sn];
      }
      else if( is_number( arg ) )
      {
         sn = atoi( arg );
         if( ( skill = get_skilltype( sn ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid sn."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         sn %= 1000;
      }
      else if( ( sn = skill_lookup( arg ) ) >= 0 )
         skill = skill_table[sn];
      else if( ( sn = herb_lookup( arg ) ) >= 0 )
         skill = herb_table[sn];
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such skill, spell, proficiency or tongue."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !skill )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not created yet."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Sn: %4d Slot: %4d %s: '%-20s'", sn, skill->slot, skill_tname[skill->type], skill->name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->info )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "DamType: %s  ActType: %s   ClassType: %s   PowerType: %s", spell_damage[SPELL_DAMAGE( skill )], spell_action[SPELL_ACTION( skill )], spell_class[SPELL_CLASS( skill )], spell_power[SPELL_POWER( skill )]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->flags )
      {
         int x;

         mudstrlcpy( buf, "Flags:", MAX_STRING_LENGTH );
         for( x = 0; x < 32; ++x )
            if( SPELL_FLAG( skill, 1 << x ) )
            {
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
               mudstrlcat( buf, spell_flag[x], MAX_STRING_LENGTH );
            }
         mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
         send_to_char( buf, ch );
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Saves: %s  SaveEffect: %s", spell_saves[( int )skill->saves], spell_save_effect[SPELL_SAVE( skill )]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( skill->difficulty != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Difficulty: %d", ( int )skill->difficulty); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d", skill_tname[skill->type], target_type[URANGE( TAR_IGNORE, skill->target, TAR_OBJ_INV )], skill->minimum_position, skill->min_mana, skill->beats, skill->range); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Flags: %d  Guild: %d  Value: %d  Info: %d  Code: %s", skill->flags, skill->guild, skill->value, skill->info, skill->skill_fun ? skill->skill_fun_name : skill->spell_fun_name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Sectors Allowed: %s", skill->spell_sector ? flag_string( skill->spell_sector, sec_flags ) : "All"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch_printf( ch, "Dammsg: %s\r\nWearoff: %s\n", skill->noun_damage, skill->msg_off ? skill->msg_off : "(none set)" );
      if( skill->dice && skill->dice[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Dice: %s", skill->dice); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->teachers && skill->teachers[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Teachers: %s", skill->teachers); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->components && skill->components[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Components: %s", skill->components); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->participants )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Participants: %d", ( int )skill->participants); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->userec.num_uses )
         send_timer( &skill->userec, ch );
      for( aff = skill->first_affect; aff; aff = aff->next )
      {
         if( aff == skill->first_affect )
            send_to_char( "\r\n", ch );
         snprintf( buf, MAX_STRING_LENGTH, "Affect %d", ++cnt );
         if( aff->location )
         {
            mudstrlcat( buf, " modifies ", MAX_STRING_LENGTH );
            mudstrlcat( buf, a_types[aff->location % REVERSE_APPLY], MAX_STRING_LENGTH );
            mudstrlcat( buf, " by '", MAX_STRING_LENGTH );
            mudstrlcat( buf, aff->modifier, MAX_STRING_LENGTH );
            if( aff->bitvector != -1 )
               mudstrlcat( buf, "' and", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, "'", MAX_STRING_LENGTH );
         }
         if( aff->bitvector != -1 )
         {
            mudstrlcat( buf, " applies ", MAX_STRING_LENGTH );
            mudstrlcat( buf, a_flags[aff->bitvector], MAX_STRING_LENGTH );
         }
         if( aff->duration[0] != '\0' && aff->duration[0] != '0' )
         {
            mudstrlcat( buf, " for '", MAX_STRING_LENGTH );
            mudstrlcat( buf, aff->duration, MAX_STRING_LENGTH );
            mudstrlcat( buf, "' rounds", MAX_STRING_LENGTH );
         }
         if( aff->location >= REVERSE_APPLY )
            mudstrlcat( buf, " (affects caster only)", MAX_STRING_LENGTH );
         mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
         send_to_char( buf, ch );

         if( !aff->next )
            send_to_char( "\r\n", ch );
      }

      if( skill->hit_char && skill->hit_char[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hitchar   : %s", skill->hit_char); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->hit_vict && skill->hit_vict[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hitvict   : %s", skill->hit_vict); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->hit_room && skill->hit_room[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hitroom   : %s", skill->hit_room); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->hit_dest && skill->hit_dest[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hitdest   : %s", skill->hit_dest); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->miss_char && skill->miss_char[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Misschar  : %s", skill->miss_char); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->miss_vict && skill->miss_vict[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Missvict  : %s", skill->miss_vict); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->miss_room && skill->miss_room[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Missroom  : %s", skill->miss_room); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->die_char && skill->die_char[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Diechar   : %s", skill->die_char); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->die_vict && skill->die_vict[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Dievict   : %s", skill->die_vict); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->die_room && skill->die_room[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Dieroom   : %s", skill->die_room); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->imm_char && skill->imm_char[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Immchar   : %s", skill->imm_char); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->imm_vict && skill->imm_vict[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Immvict   : %s", skill->imm_vict); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->imm_room && skill->imm_room[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Immroom   : %s", skill->imm_room); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( skill->type != SKILL_HERB )
      {
         if( skill->type != SKILL_RACIAL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "--------------------------[CLASS USE]--------------------------"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            for( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
            {
               mudstrlcpy( buf, class_table[iClass]->who_name, MAX_STRING_LENGTH );
               snprintf( buf + 3, MAX_STRING_LENGTH - 3, ") lvl: %3d max: %2d%%", skill->skill_level[iClass],
                         skill->skill_adept[iClass] );
               if( iClass % 3 == 2 )
                  mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               else
                  mudstrlcat( buf, "  ", MAX_STRING_LENGTH );
               send_to_char( buf, ch );
            }
         }
         else
         {
            send_to_char( "\r\n--------------------------[RACE USE]--------------------------\r\n", ch );
            for( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
            {
               snprintf( buf, MAX_STRING_LENGTH, "%8.8s) lvl: %3d max: %2d%%",
                         race_table[iRace]->race_name, skill->race_level[iRace], skill->race_adept[iRace] );
               if( !strcmp( race_table[iRace]->race_name, "unused" ) )
                  snprintf( buf, MAX_STRING_LENGTH, "                           " );
               if( ( iRace > 0 ) && ( iRace % 2 == 1 ) )
                  mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               else
                  mudstrlcat( buf, "  ", MAX_STRING_LENGTH );
               send_to_char( buf, ch );
            }
         }
      }
      send_to_char( "\r\n", ch );
   }
   return;
}
void do_sset								   		(CHAR_DATA *ch, const char *argument){
   char arg1[1000];
   char arg2[1000];
   CHAR_DATA *victim;
   int value;
   int sn, i;
   bool fAll;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: sset <victim> <skill> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset <victim> all     <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( get_trust( ch ) > LEVEL_SUB_IMPLEM )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset save skill table"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset save herb table"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset create skill 'new skill'"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset create herb 'new herb'"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset create ability 'new ability'"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      if( get_trust( ch ) > LEVEL_GREATER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "or:     sset <sn>     <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         send_to_char( "\r\nField being one of:\r\n", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  name code target minpos slot mana beats dammsg wearoff guild minlevel"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  type damtype acttype classtype powertype seffect flag dice value difficulty"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  affect rmaffect level adept hit miss die imm (char/vict/room)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  components teachers racelevel raceadept"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  sector"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Affect having the fields: <location> <modfifier> [duration] [bitvector]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Skill being any skill or spell."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg1, "save" ) && !str_cmp( argument, "table" ) )
   {
      if( !str_cmp( arg2, "skill" ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving skill table..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         save_skill_table(  );
         save_classes(  );
         save_races(  );
         return;
      }
      if( !str_cmp( arg2, "herb" ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving herb table..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         save_herb_table(  );
         return;
      }
   }
   if( get_trust( ch ) > LEVEL_SUB_IMPLEM
       && !str_cmp( arg1, "create" )
       && ( !str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" ) || !str_cmp( arg2, "ability" ) ) )
   {
      struct skill_type *skill;
      short type = SKILL_UNKNOWN;

      if( !str_cmp( arg2, "herb" ) )
      {
         type = SKILL_HERB;
         if( top_herb >= MAX_HERB )
         {
            ch_printf( ch, "The current top herb is %d, which is the maximum.  "
                       "To add more herbs,\r\nMAX_HERB will have to be "
                       "raised in mud.h, and the mud recompiled.\r\n", top_herb );
            return;
         }
      }
      else if( num_skills >= MAX_SKILL )
      {
         ch_printf( ch, "The current top sn is %d, which is the maximum.  "
                    "To add more skills,\r\nMAX_SKILL will have to be "
                    "raised in mud.h, and the mud recompiled.\r\n", num_skills );
         return;
      }
      CREATE( skill, struct skill_type, 1 );
      skill->slot = 0;
      if( type == SKILL_HERB )
      {
         int max, x;

         herb_table[top_herb++] = skill;
         for( max = x = 0; x < top_herb - 1; x++ )
            if( herb_table[x] && herb_table[x]->slot > max )
               max = herb_table[x]->slot;
         skill->slot = max + 1;
      }
      else  // add the skill. See db.c for why we are adding without sorting.
         skill_table[num_skills++] = skill;
      skill->min_mana = 0;
      skill->name = str_dup( argument );
      skill->noun_damage = str_dup( "" );
      skill->msg_off = str_dup( "" );
      skill->spell_fun = spell_smaug;
      skill->type = type;
      skill->spell_sector = 0;
      skill->guild = -1;
      if( !str_cmp( arg2, "ability" ) )
         skill->type = SKILL_RACIAL;

      for( i = 0; i < MAX_PC_CLASS; i++ )
      {
         skill->skill_level[i] = LEVEL_IMMORTAL;
         skill->skill_adept[i] = 95;
      }
      for( i = 0; i < MAX_PC_RACE; i++ )
      {
         skill->race_level[i] = LEVEL_IMMORTAL;
         skill->race_adept[i] = 95;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( arg1[0] == 'h' )
      sn = atoi( arg1 + 1 );
   else
      sn = atoi( arg1 );
   if( get_trust( ch ) > LEVEL_GREATER
       && ( ( arg1[0] == 'h' && is_number( arg1 + 1 ) && ( sn = atoi( arg1 + 1 ) ) >= 0 )
            || ( is_number( arg1 ) && ( sn = atoi( arg1 ) ) >= 0 ) ) )
   {
      struct skill_type *skill;

      if( arg1[0] == 'h' )
      {
         if( sn >= top_herb )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Herb number out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         skill = herb_table[sn];
      }
      else
      {
         if( ( skill = get_skilltype( sn ) ) == NULL )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Skill number out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         sn %= 1000;
      }

      if( !str_cmp( arg2, "difficulty" ) )
      {
         skill->difficulty = atoi( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "participants" ) )
      {
         skill->participants = atoi( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "damtype" ) )
      {
         int x = get_sdamage( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell damage type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_SDAM( skill, x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "acttype" ) )
      {
         int x = get_saction( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell action type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_SACT( skill, x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "classtype" ) )
      {
         int x = get_sclass( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell class type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_SCLA( skill, x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "powertype" ) )
      {
         int x = get_spower( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell power type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_SPOW( skill, x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "seffect" ) )
      {
         int x = get_ssave_effect( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell save effect type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_SSAV( skill, x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "flag" ) )
      {
         int x = get_sflag( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell flag."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            TOGGLE_BIT( skill->flags, 1 << x );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "saves" ) )
      {
         int x = get_ssave( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a saving type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            skill->saves = x;
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }

      if( !str_cmp( arg2, "code" ) )
      {
         SPELL_FUN *spellfun;
         DO_FUN *dofun;

         if( !str_prefix( "do_", argument ) && ( dofun = skill_function( argument ) ) != skill_notfound )
         {
            skill->skill_fun = dofun;
            skill->spell_fun = NULL;
            DISPOSE( skill->skill_fun_name );
            skill->skill_fun_name = str_dup( argument );
         }
         else if( ( spellfun = spell_function( argument ) ) != spell_notfound )
         {
            skill->spell_fun = spellfun;
            skill->skill_fun = NULL;
            DISPOSE( skill->skill_fun_name );
            skill->spell_fun_name = str_dup( argument );
         }
         else if( validate_spec_fun( argument ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Cannot use a spec_fun for skills or spells."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         else
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a spell or skill."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "target" ) )
      {
         int x = get_starget( argument );

         if( x == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a valid target type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            skill->target = x;
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         }
         return;
      }
      if( !str_cmp( arg2, "minpos" ) )
      {
         skill->minimum_position = URANGE( POS_DEAD, atoi( argument ), POS_DRAG );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "minlevel" ) )
      {
         skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "sector" ) )
      {
         char tmp_arg[MAX_STRING_LENGTH];

         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, tmp_arg );
            value = get_secflag( tmp_arg );
            if( value < 0 || value > MAX_SECFLAG )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", tmp_arg); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               TOGGLE_BIT( skill->spell_sector, ( 1 << value ) );
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "slot" ) )
      {
         skill->slot = URANGE( 0, atoi( argument ), 30000 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "mana" ) )
      {
         skill->min_mana = URANGE( 0, atoi( argument ), 2000 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "beats" ) )
      {
         skill->beats = URANGE( 0, atoi( argument ), 120 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "range" ) )
      {
         skill->range = URANGE( 0, atoi( argument ), 20 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "guild" ) )
      {
         skill->guild = atoi( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "value" ) )
      {
         skill->value = atoi( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "type" ) )
      {
         skill->type = get_skill( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( arg2, "rmaffect" ) )
      {
         SMAUG_AFF *aff, *aff_next;
         int num = atoi( argument );
         int cnt = 0;

         if( !skill->first_affect )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This spell has no special affects to remove."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         for( aff = skill->first_affect; aff; aff = aff_next )
         {
            aff_next = aff->next;

            if( ++cnt == num )
            {
               UNLINK( aff, skill->first_affect, skill->last_affect, next, prev );
               DISPOSE( aff->duration );
               DISPOSE( aff->modifier );
               DISPOSE( aff );
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      /*
       * affect <location> <modifier> <duration> <bitvector>
       */
      if( !str_cmp( arg2, "affect" ) )
      {
         char location[MAX_INPUT_LENGTH];
         char modifier[MAX_INPUT_LENGTH];
         char duration[MAX_INPUT_LENGTH];
         int loc, bit, tmpbit;
         SMAUG_AFF *aff;

         argument = one_argument( argument, location );
         argument = one_argument( argument, modifier );
         argument = one_argument( argument, duration );

         if( location[0] == '!' )
            loc = get_atype( location + 1 ) + REVERSE_APPLY;
         else
            loc = get_atype( location );
         if( ( loc % REVERSE_APPLY ) < 0 || ( loc % REVERSE_APPLY ) >= MAX_APPLY_TYPE )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown affect location.  See AFFECTTYPES."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         bit = -1;
         if( argument[0] != '\0' )
         {
            if( ( tmpbit = get_aflag( argument ) ) == -1 )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown bitvector: %s.  See AFFECTED_BY", argument); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               bit = tmpbit;
         }
         CREATE( aff, SMAUG_AFF, 1 );
         if( !str_cmp( duration, "0" ) )
            duration[0] = '\0';
         if( !str_cmp( modifier, "0" ) )
            modifier[0] = '\0';
         aff->duration = str_dup( duration );
         aff->location = loc;
         if( loc == APPLY_AFFECT )
         {
            int modval = get_aflag( modifier );

            if( modval < 0 )
               modval = 0;
            snprintf( modifier, MAX_INPUT_LENGTH, "%d", modval );
         }
         else if( loc == APPLY_RESISTANT || loc == APPLY_IMMUNE || loc == APPLY_SUSCEPTIBLE )
         {
            int modval = get_risflag( modifier );
            if( modval < 0 )
               modval = 0;
            snprintf( modifier, MAX_INPUT_LENGTH, "%d", modval );
         }
         aff->modifier = str_dup( modifier );
         aff->bitvector = bit;
         LINK( aff, skill->first_affect, skill->last_affect, next, prev );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "level" ) )
      {
         char arg3[MAX_INPUT_LENGTH];
         int Class;

         argument = one_argument( argument, arg3 );
         Class = atoi( arg3 );
         if( Class >= MAX_PC_CLASS || Class < 0 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a valid class."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            skill->skill_level[Class] = URANGE( 0, atoi( argument ), MAX_LEVEL );
         return;
      }

      if( !str_cmp( arg2, "racelevel" ) )
      {
         char arg3[MAX_INPUT_LENGTH];
         int race;

         argument = one_argument( argument, arg3 );
         race = atoi( arg3 );
         if( race >= MAX_PC_RACE || race < 0 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a valid race."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            skill->race_level[race] = URANGE( 0, atoi( argument ), MAX_LEVEL );
         return;
      }

      if( !str_cmp( arg2, "adept" ) )
      {
         char arg3[MAX_INPUT_LENGTH];
         int Class;

         argument = one_argument( argument, arg3 );
         Class = atoi( arg3 );
         if( Class >= MAX_PC_CLASS || Class < 0 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a valid class."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            skill->skill_adept[Class] = URANGE( 0, atoi( argument ), 100 );
         return;
      }

      if( !str_cmp( arg2, "raceadept" ) )
      {
         char arg3[MAX_INPUT_LENGTH];
         int race;

         argument = one_argument( argument, arg3 );
         race = atoi( arg3 );
         if( race >= MAX_PC_RACE || race < 0 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not a valid race."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            skill->race_adept[race] = URANGE( 0, atoi( argument ), 100 );
         return;
      }

      if( !str_cmp( arg2, "name" ) )
      {
         DISPOSE( skill->name );
         skill->name = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "dammsg" ) )
      {
         DISPOSE( skill->noun_damage );
         if( !str_cmp( argument, "clear" ) )
            skill->noun_damage = str_dup( "" );
         else
            skill->noun_damage = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "wearoff" ) )
      {
         DISPOSE( skill->msg_off );
         if( str_cmp( argument, "clear" ) )
            skill->msg_off = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "hitchar" ) )
      {
         if( skill->hit_char )
            DISPOSE( skill->hit_char );
         if( str_cmp( argument, "clear" ) )
            skill->hit_char = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "hitvict" ) )
      {
         if( skill->hit_vict )
            DISPOSE( skill->hit_vict );
         if( str_cmp( argument, "clear" ) )
            skill->hit_vict = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "hitroom" ) )
      {
         if( skill->hit_room )
            DISPOSE( skill->hit_room );
         if( str_cmp( argument, "clear" ) )
            skill->hit_room = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "hitdest" ) )
      {
         if( skill->hit_dest )
            DISPOSE( skill->hit_dest );
         if( str_cmp( argument, "clear" ) )
            skill->hit_dest = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "misschar" ) )
      {
         if( skill->miss_char )
            DISPOSE( skill->miss_char );
         if( str_cmp( argument, "clear" ) )
            skill->miss_char = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "missvict" ) )
      {
         if( skill->miss_vict )
            DISPOSE( skill->miss_vict );
         if( str_cmp( argument, "clear" ) )
            skill->miss_vict = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "missroom" ) )
      {
         if( skill->miss_room )
            DISPOSE( skill->miss_room );
         if( str_cmp( argument, "clear" ) )
            skill->miss_room = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "diechar" ) )
      {
         if( skill->die_char )
            DISPOSE( skill->die_char );
         if( str_cmp( argument, "clear" ) )
            skill->die_char = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "dievict" ) )
      {
         if( skill->die_vict )
            DISPOSE( skill->die_vict );
         if( str_cmp( argument, "clear" ) )
            skill->die_vict = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "dieroom" ) )
      {
         if( skill->die_room )
            DISPOSE( skill->die_room );
         if( str_cmp( argument, "clear" ) )
            skill->die_room = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "immchar" ) )
      {
         if( skill->imm_char )
            DISPOSE( skill->imm_char );
         if( str_cmp( argument, "clear" ) )
            skill->imm_char = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "immvict" ) )
      {
         if( skill->imm_vict )
            DISPOSE( skill->imm_vict );
         if( str_cmp( argument, "clear" ) )
            skill->imm_vict = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "immroom" ) )
      {
         if( skill->imm_room )
            DISPOSE( skill->imm_room );
         if( str_cmp( argument, "clear" ) )
            skill->imm_room = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "dice" ) )
      {
         if( skill->dice )
            DISPOSE( skill->dice );
         if( str_cmp( argument, "clear" ) )
            skill->dice = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "components" ) )
      {
         if( skill->components )
            DISPOSE( skill->components );
         if( str_cmp( argument, "clear" ) )
            skill->components = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg2, "teachers" ) )
      {
         if( skill->teachers )
            DISPOSE( skill->teachers );
         if( str_cmp( argument, "clear" ) )
            skill->teachers = str_dup( argument );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      do_sset( ch, "" );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      if( ( sn = skill_lookup( arg1 ) ) >= 0 )
      {
         snprintf( arg1, MAX_INPUT_LENGTH, "%d %s %s", sn, arg2, argument );
         do_sset( ch, arg1 );
      }
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_NPC( victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   fAll = !str_cmp( arg2, "all" );
   sn = 0;
   if( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such skill or spell."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Snarf the value.
    */
   if( !is_number( argument ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value must be numeric."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   value = atoi( argument );
   if( value < 0 || value > 100 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( fAll )
   {
      for( sn = 0; sn < num_skills; ++sn )
      {
         /*
          * Fix by Narn to prevent ssetting skills the player shouldn't have. 
          */
         if( victim->level >= skill_table[sn]->skill_level[victim->Class]
             || victim->level >= skill_table[sn]->race_level[victim->race] )
         {
            // Bugfix by Sadiq - Modified slightly by Samson. No need to call GET_ADEPT more than once each time this loop runs.
            int adept = GET_ADEPT( victim, sn );

            if( value > adept && !IS_IMMORTAL( victim ) )
               victim->pcdata->learned[sn] = adept;
            else
               victim->pcdata->learned[sn] = value;
         }
      }
   }
   else
      victim->pcdata->learned[sn] = value;
}
void ability_learn_from_success						(CHAR_DATA *ch, int sn){
   int adept, gain, sklvl, learn, percent, schance;
   char buf1[256];

   if(IS_NPC( ch ) || ch->pcdata->learned[sn] <= 0)
   return;

   adept = skill_table[sn]->race_adept[ch->race];

   sklvl = skill_table[sn]->race_level[ch->race];

   if(sklvl == 0)
      sklvl = ch->level;

   if(ch->pcdata->learned[sn] < adept)
     {
     schance = ch->pcdata->learned[sn];
     percent = number_percent();
     
      if(percent >= schance)
        {
        learn = 1;
        snprintf(buf1, 256, "&WYour practice improves &B%s&W to &Y%d&W points.&D", skill_table[sn]->name, ch->pcdata->learned[sn] + 1);
        act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
        }

      if(percent < schance)
        {
        learn = 0;
        }

      ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + learn );

      if(ch->pcdata->learned[sn] == adept)
        {
        gain = 1000 * sklvl;
        snprintf(buf1, 256, "&WYou are now a master of &B%s&W.\n\rYou gain &Y%d&W experience points.", skill_table[sn]->name, gain);
        act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
        }
      else
        {
        gain = 20 * sklvl;
        if( !ch->fighting && sn != tes_skill_hide_in_shadows && sn != tes_skill_move_silently )
          {
          snprintf(buf1, 256, "&WYou gain &Y%d&W experience points.&D", gain);
          act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
          }
        }
      gain_exp( ch, gain );
     }
}
void learn_from_success								(CHAR_DATA *ch, int sn){
   int adept, gain, sklvl, learn, percent, schance;
   char buf1[256];

   if(IS_NPC(ch) || ch->pcdata->learned[sn] <= 0)
      return;

   adept = GET_ADEPT( ch, sn );
   sklvl = skill_table[sn]->skill_level[ch->Class];

   if(sklvl == 0)
     sklvl = ch->level;

   if(ch->pcdata->learned[sn] < adept)
     {
      schance = ch->pcdata->learned[sn];
      percent = number_percent();

      if(percent >= schance)
        {
        learn = 1;
        snprintf(buf1, 256, "&WYour practice improves &B%s&W to &Y%d&W points.&D", skill_table[sn]->name, ch->pcdata->learned[sn] + 1);
        act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
        }

      if(percent < schance)
        {
        learn = 0;
        }

      ch->pcdata->learned[sn] = UMIN(adept, ch->pcdata->learned[sn] + learn);

      if(ch->pcdata->learned[sn] == adept)
        {
        gain = 1000 * sklvl;
        snprintf(buf1, 256, "&WYou are now a master of &B%s&W.\n\r&WYou gain &Y%d&W experience points.", skill_table[sn]->name, gain);
        act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
        }
      else
        {
        gain = 20 * sklvl;
        
        if(!ch->fighting && sn != tes_skill_hide_in_shadows && sn != tes_skill_move_silently)
          {
          snprintf(buf1, 256, "&WYou gain &Y%d&W experience points.&D", gain);
          act(AT_SKILL, buf1, ch, NULL, NULL, TO_CHAR);
          }
        }
      gain_exp(ch, gain);
     }
}
void learn_from_failure   							(CHAR_DATA *ch, int sn){
   int adept, schance;

   if(IS_NPC( ch ) || ch->pcdata->learned[sn] <= 0)
   return;

   schance = ch->pcdata->learned[sn];

   if(schance - number_percent() > 25)
   return;

   adept = GET_ADEPT( ch, sn );

   //A difficult failure can occasionally teach something, but never above the adept cap.
   if(ch->pcdata->learned[sn] < (adept - 1) && number_percent() <= 5)
   {
      ch->pcdata->learned[sn]++;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WYou learn from your mistake with &B%s&W. (&Y%d&W)&D", skill_table[sn]->name, ch->pcdata->learned[sn]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   }
}
void do_rescue    							   		(CHAR_DATA* ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CHAR_DATA *fch;
   int dice;
   int check;
   int cost;

   dice = number_percent();
   check = (LEARNED(ch, gsn_rescue)/2) + get_curr_lck(ch);
   cost = 10;
   check = URANGE(5, check, 95);

   if(IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou can't concentrate enough for that.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(IS_AFFECTED(ch, AFF_BERSERK))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou aren't thinking clearly.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   one_argument(argument, arg);

   if(arg[0] == '\0')
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YRescue whom?&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if((victim = get_char_room(ch, arg)) == NULL)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThey aren't here.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(victim == ch)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YHow about fleeing instead?&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->mount)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou can't do that while mounted.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(!IS_NPC(ch) && IS_NPC(victim))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThey don't need your help!&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if((fch = who_fighting(victim)) == NULL)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThey are not fighting right now.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(who_fighting(victim) == ch)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YJust running away would be better.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(IS_AFFECTED(victim, AFF_BERSERK))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YStepping in front of a berserker would not be an intelligent decision.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->move < cost)
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou are too exhausted to rush to their aid.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE(ch, PULSE_VIOLENCE);
   ch->move = UMAX(0, ch->move - cost);

   if(dice > check)
   {
      switch(number_range(1,3))
      {
         case 1: act(AT_SKILL, "&YYou try to intercept $N's attacker, but your timing is off.&D", ch, NULL, victim, TO_CHAR); break;
         case 2: act(AT_SKILL, "&YYou lunge to protect $N, but cannot get between the combatants.&D", ch, NULL, victim, TO_CHAR); break;
         default: act(AT_SKILL, "&YYou reach for $N, but the enemy keeps you at bay.&D", ch, NULL, victim, TO_CHAR); break;
      }
      act(AT_SKILL, "&Y$n tries to shield you from the attack!&D", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "&Y$n tries to intervene on behalf of $N.&D", ch, NULL, victim, TO_NOTVICT);
      learn_from_failure(ch, gsn_rescue);
      return;
   }

   act(AT_SKILL, "&GYou step into the attack and pull $N to safety!&D", ch, NULL, victim, TO_CHAR);
   act(AT_SKILL, "&G$n intercepts your attacker, giving you room to recover!&D", ch, NULL, victim, TO_VICT);
   act(AT_SKILL, "&Y$n moves decisively between $N and the enemy.&D", ch, NULL, victim, TO_NOTVICT);
   learn_from_success(ch, gsn_rescue);

   stop_fighting(fch, FALSE);
   stop_fighting(victim, FALSE);

   if(ch->fighting)
     stop_fighting(ch, FALSE);

   set_fighting(ch, fch);
   set_fighting(fch, ch);
   return;
}
void do_trance								   		(CHAR_DATA *ch, const char *argument){
   char *arg = NULL;
   int percent;
   int managain = ( ch->Class == CLASS_DRUID ? 0 : 50 );

   switch ( ch->substate )
   {
      default:
         if( IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYou enter a peaceful trance, collecting mana from the cosmos."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         add_timer( ch, TIMER_DO_FUN, UMAX( 2, ( skill_table[gsn_trance]->beats / 8 ) ), do_trance, 1 );
         ch->alloc_ptr = str_dup( argument );
         return;

      case 1:
         if( !ch->alloc_ptr )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYour trance was interrupted."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s: alloc_ptr NULL or not numeric", __func__ );
            return;
         }
         arg = str_dup( ch->alloc_ptr );
         DISPOSE( ch->alloc_ptr );
         break;

      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYou come out of your trance."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         DISPOSE( ch->alloc_ptr );
         return;
   }

   switch ( ch->in_room->sector_type )
   {
      case SECT_DUNNO:
      case SECT_UNDERGROUND:
      default:
         break;

      case SECT_INSIDE:
      case SECT_FIELD:
      case SECT_HILLS:
      case SECT_FOREST:
      case SECT_MOUNTAIN:
         if( ch->Class == CLASS_DRUID )
            managain = 50;
         else
            managain += 2;
         break;

      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
      case SECT_UNDERWATER:
      case SECT_OCEANFLOOR:
         if( ch->race == RACE_SEA_ELF )
            managain += 3;
         else if( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
            managain -= 2;
         break;

      case SECT_AIR:
         if( ch->race == RACE_PIXIE )
            managain += 3;
         else if( !IS_AFFECTED( ch, AFF_FLYING ) )
            managain -= 2;
         break;

      case SECT_DESERT:
      case SECT_LAVA:
      case SECT_SWAMP:
         managain -= 2;
         break;
   }

   percent = UMAX( 1, number_percent(  ) + get_curr_wis( ch ) - 10 );
   if( can_use_skill( ch, percent, gsn_trance ) )
   {
      learn_from_success( ch, gsn_trance );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BIn your peaceful trance, you collect mana from the cosmos."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->mana = UMIN( ch->max_mana, ch->mana + managain );
   }
   else
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYou spend several minutes in a deep trance, but fail to collect any mana."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   }

   if( ch->mana < ch->max_mana )
   {
      TIMER *timer;
      timer = get_timerptr( ch, TIMER_DO_FUN );
      if(timer)
      {
         timer->count += UMAX(2, skill_table[gsn_trance]->beats / 8);
         ch->alloc_ptr = str_dup(arg);
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYour trance comes to an end."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         ch->substate = SUB_NONE;
      }
   }
   else
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&BYou complete your trance."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->substate = SUB_NONE;
   }
   if(arg) DISPOSE(arg);
   return;
}
void do_bite								   		(CHAR_DATA* ch, const char *argument){
   CHAR_DATA *victim;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_NPC( ch ) && ch->level < skill_table[gsn_bite]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That isn't quite one of your natural skills."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = who_fighting( ch ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_bite]->beats );
   if( can_use_skill( ch, number_percent(  ), gsn_bite ) )
   {
      learn_from_success( ch, gsn_bite );
      global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_bite );
   }
   else
   {
      global_retcode = damage( ch, victim, 0, gsn_bite );
   }
   return;
}
void do_claw								   		(CHAR_DATA* ch, const char *argument){
   CHAR_DATA *victim;

   if( !IS_NPC( ch ) && ch->level < skill_table[gsn_claw]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That isn't quite one of your natural skills."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = who_fighting( ch ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_claw]->beats );
   if( can_use_skill( ch, number_percent(  ), gsn_claw ) )
   {
      learn_from_success( ch, gsn_claw );
      global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_claw );
   }
   else
   {
      global_retcode = damage( ch, victim, 0, gsn_claw );
   }
   return;
}
void do_sting								   		(CHAR_DATA* ch, const char *argument){
   CHAR_DATA *victim;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_NPC( ch ) && ch->level < skill_table[gsn_sting]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That isn't quite one of your natural skills."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = who_fighting( ch ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_sting]->beats );
   if( can_use_skill( ch, number_percent(  ), gsn_sting ) )
   {
      learn_from_success( ch, gsn_sting );
      global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_sting );
   }
   else
   {
      global_retcode = damage( ch, victim, 0, gsn_sting );
   }
   return;
}
void do_tail								   		(CHAR_DATA* ch, const char *argument){
   CHAR_DATA *victim;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_NPC( ch ) && ch->level < skill_table[gsn_tail]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That isn't quite one of your natural skills."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = who_fighting( ch ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_tail]->beats );
   if( can_use_skill( ch, number_percent(  ), gsn_tail ) )
   {
      learn_from_success( ch, gsn_tail );
      global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_tail );
   }
   else
   {
      global_retcode = damage( ch, victim, 0, gsn_tail );
   }
   return;
}
void do_bloodlet							   		(CHAR_DATA* ch, const char *argument){
   OBJ_DATA *obj;

   if( IS_NPC( ch ) || !IS_VAMPIRE( ch ) )
      return;

   if( ch->fighting )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're too busy fighting..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ch->pcdata->condition[COND_BLOODTHIRST] < 10 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are too drained to offer any blood..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, PULSE_VIOLENCE );
   if( can_use_skill( ch, number_percent(  ), gsn_bloodlet ) )
   {
      gain_condition( ch, COND_BLOODTHIRST, -7 );
      act( AT_BLOOD, "Tracing a sharp nail over your skin, you let your blood spill.", ch, NULL, NULL, TO_CHAR );
      act( AT_BLOOD, "$n traces a sharp nail over $s skin, spilling a quantity of blood to the ground.", ch, NULL, NULL, TO_ROOM );
      learn_from_success( ch, gsn_bloodlet );
      obj = create_object( get_obj_index( OBJ_VNUM_BLOODLET ), 0 );
      obj->timer = 1;
      obj->value[1] = 6;
      obj_to_room( obj, ch->in_room );
      damage( ch, ch, ch->level / 5, gsn_bloodlet );
   }
   else
   {
      act( AT_BLOOD, "You cannot manage to draw much blood...", ch, NULL, NULL, TO_CHAR );
      act( AT_BLOOD, "$n slices open $s skin, but no blood is spilled...", ch, NULL, NULL, TO_ROOM );
   }
   return;
}
void do_feed								   		(CHAR_DATA* ch, const char *argument){
   CHAR_DATA *victim;
   short dam;
   short temphit;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_NPC( ch ) && !IS_VAMPIRE( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It is not of your nature to feed on living creatures."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( !can_use_skill( ch, 0, gsn_feed ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet practiced your new teeth."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = who_fighting( ch ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ch->mount )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that while mounted."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_feed]->beats );
   if( can_use_skill( ch, number_percent(  ), gsn_feed ) )
   {
      dam = number_range( 1, ch->level );
      global_retcode = damage( ch, victim, dam, gsn_feed );
      if( global_retcode == rNONE && !IS_NPC( ch ) && dam
          && ch->fighting && ch->pcdata->condition[COND_BLOODTHIRST] < ( 10 + ch->level ) )
      {
         gain_condition( ch, COND_BLOODTHIRST,
                         UMIN( number_range( 1, ( ch->level + victim->level / 20 ) + 3 ),
                               ( 10 + ch->level ) - ch->pcdata->condition[COND_BLOODTHIRST] ) );
         if( ch->pcdata->condition[COND_FULL] <= 37 )
            gain_condition( ch, COND_FULL, 2 );
         gain_condition( ch, COND_THIRST, 2 );
         act( AT_BLOOD, "You manage to suck a little life out of $N.", ch, NULL, victim, TO_CHAR );
         act( AT_BLOOD, "$n sucks some of your blood!", ch, NULL, victim, TO_VICT );
         learn_from_success( ch, gsn_feed );
         temphit = ch->hit;
         ch->hit += 1 + ch->level / 5;
         if( ch->hit > ch->max_hit )
            ch->hit = ch->max_hit;
         if( ch->hit < temphit )
            ch->hit = temphit;
      }
   }
   else
   {
      global_retcode = damage( ch, victim, 0, gsn_feed );
      if( global_retcode == rNONE && !IS_NPC( ch )
          && ch->fighting && ch->pcdata->condition[COND_BLOODTHIRST] < ( 10 + ch->level ) )
      {
         act( AT_BLOOD, "The smell of $N's blood is driving you insane!", ch, NULL, victim, TO_CHAR );
         act( AT_BLOOD, "$n is lusting after your blood!", ch, NULL, victim, TO_VICT );
      }
   }
   return;
}
void trip										  	(CHAR_DATA *ch, CHAR_DATA *victim){
   if( IS_AFFECTED( victim, AFF_FLYING ) || IS_AFFECTED( victim, AFF_FLOATING ) )
      return;

   if( victim->mount )
   {
      if( IS_AFFECTED( victim->mount, AFF_FLYING ) || IS_AFFECTED( victim->mount, AFF_FLOATING ) )
         return;
      act( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT );
      act( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR );
      act( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
      xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
      victim->mount = NULL;
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
      victim->position = POS_RESTING;
      return;
   }

   if( victim->wait == 0 )
   {
      act( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT );
      act( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR );
      act( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
      victim->position = POS_RESTING;
   }
   return;
}
void do_mistwalk							  		(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool allowday;

   set_char_color( AT_DGREEN, ch );
   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that right now."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ch->mount )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "And scare your mount to death?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Who will be your victim?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_mistwalk]->beats );
   if( ( victim = get_char_world( ch, arg ) ) == NULL || victim == ch )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are unable to sense your victim."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_PKILL( ch ) && ch->pcdata->condition[COND_BLOODTHIRST] > 22 )
      allowday = TRUE;
   else
      allowday = FALSE;

   if( ( time_info.hour < 21 && time_info.hour > 5 && !allowday )
       || !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= ch->level + 15
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( ch->level, victim ) )
       || ( CAN_PKILL( victim ) && !IS_NPC( ch ) && !CAN_PKILL( ch ) )
       || !in_hard_range( ch, victim->in_room->area )
       || ( IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( ch ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are unable to sense your victim."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Subtract 22 extra bp for mist walk from 0500 to 2100 SB 
    */
   if( time_info.hour < 21 && time_info.hour > 5 && !IS_NPC( ch ) )
      gain_condition( ch, COND_BLOODTHIRST, -22 );
   act( AT_DGREEN, "Your surroundings blur as you assume a form of churning mist!", ch, NULL, NULL, TO_CHAR );
   act( AT_DGREEN, "$n dissolves into a cloud of glowing mist, then vanishes!", ch, NULL, NULL, TO_ROOM );
   learn_from_success( ch, gsn_mistwalk );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   act( AT_DGREEN, "A cloud of glowing mist engulfs you, then withdraws to unveil $n!", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return;
}
void do_broach								   		(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
   EXIT_DATA *pexit;

   set_char_color( AT_DGREEN, ch );

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Attempt this in which direction?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ch->mount )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You should really dismount first."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_broach]->beats );
   if( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
   {
      EXIT_DATA *pexit_rev;
      if( !IS_SET( pexit->exit_info, EX_CLOSED )
          || !IS_SET( pexit->exit_info, EX_LOCKED )
          || IS_SET( pexit->exit_info, EX_PICKPROOF ) || !can_use_skill( ch, number_percent(  ), gsn_broach ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou probe the lock, but cannot find a way to release it.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         learn_from_failure(ch, gsn_broach);
         check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
         return;
      }
      REMOVE_BIT( pexit->exit_info, EX_LOCKED );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You successfully broach the exit..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      learn_from_success( ch, gsn_broach );
      adjust_favor( ch, 9, 1 );
      if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
      {
         REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
      }
      check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
      return;
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your attempt fails."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_visible								   		(CHAR_DATA *ch, const char *argument){
   affect_strip( ch, tes_effect_invisibility );
   affect_strip( ch, gsn_mass_invis );
   affect_strip( ch, tes_skill_move_silently );
   xREMOVE_BIT( ch->affected_by, AFF_HIDE );
   xREMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
   xREMOVE_BIT( ch->affected_by, AFF_SNEAK );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_recall								   		(CHAR_DATA *ch, const char *argument){
   ROOM_INDEX_DATA *location;
   CHAR_DATA *opponent;

   location = NULL;

   if(!IS_NPC(ch) && ch->pcdata->clan)
     location = get_room_index( ch->pcdata->clan->recall);

   if(!IS_NPC(ch) && !location && ch->level >= 5 && IS_SET(ch->pcdata->flags, PCFLAG_DEADLY))
      location = get_room_index(ROOM_VNUM_DEADLY);

   if(!location && !IS_NPC(ch))
      location = get_room_index(race_table[ch->race]->race_recall);

   if(!location)
      location = get_room_index(ROOM_VNUM_TEMPLE);

   if(!location)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are completely lost."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->in_room == location)
     return;

   if(xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For some strange reason... nothing happens."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(IS_AFFECTED(ch, AFF_CURSE))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are cursed and cannot recall!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if((opponent = who_fighting(ch)) != NULL)
     { 
     int lose;

      if( number_bits(1) == 0 || (!IS_NPC(opponent) && number_bits(3) > 1))
        {
        WAIT_STATE(ch, 4);
        lose = (int)((exp_level(ch, ch->level + 1) - exp_level(ch, ch->level)) * 0.1);
        if(ch->desc)
        lose /= 2;
        gain_exp( ch, 0 - lose );
        {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You failed!  You lose %d exps.", lose); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
        return;
      }

     lose = (int)((exp_level( ch, ch->level + 1) - exp_level(ch, ch->level)) * 0.2);
     if( ch->desc )
     lose /= 2;
     gain_exp( ch, 0 - lose );
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You recall from combat!  You lose %d exps.", lose); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     stop_fighting(ch, TRUE);
     }

   act(AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, location);
   if(ch->mount)
     {
     char_from_room( ch->mount );
     char_to_room( ch->mount, location );
     }

   act( AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );

   return;
}
void do_aid									   		(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int dice;
   int check;
   int cost;

   dice = number_percent();
   check = (LEARNED(ch, gsn_aid)/2) + get_curr_wis(ch);
   cost = 10;
   check = URANGE(5, check, 95);

   if( IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   one_argument(argument, arg);

   if(arg[0] == '\0')
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Aid whom?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if((victim = get_char_room(ch, arg)) == NULL)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(IS_NPC(victim))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can aid only players."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->mount)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that while mounted."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(victim == ch)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "How to aid yourself?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(victim->position > POS_STUNNED)
     {
     act(AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim, TO_CHAR);
     return;
     }

   if(victim->hit <= -6)
     {
     act(AT_PLAIN, "$N's condition is beyond your aiding ability.", ch, NULL, victim, TO_CHAR);
     return;
     }

   if(ch->move < cost)
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou are too tired to administer aid.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   WAIT_STATE(ch, PULSE_VIOLENCE);

   if(dice > check)
     {
     act(AT_MAGIC, "&YYou bind $N's wounds, but cannot stabilize them yet.&D", ch, NULL, victim, TO_CHAR);
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YYou have %d%% chance.&D", check); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YResult: &RFailure&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     ch_printf(ch, "\r\n");

     ch->move = UMAX(0, ch->move - cost);
     learn_from_failure(ch, gsn_aid);
     return;
     }

   act(AT_MAGIC, "&GYou carefully tend $N's wounds and steady their breathing.&D", ch, NULL, victim, TO_CHAR);
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YYou have %d%% chance.&D", check); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YResult: &GSuccess&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   ch_printf(ch, "\r\n");

   act(AT_SKILL, "$n aids $N!", ch, NULL, victim, TO_NOTVICT);

   learn_from_success(ch, gsn_aid);
   ch->alignment = UMIN(999, ch->alignment + 10);
   ch->move = UMAX(0, ch->move - cost);

   if(victim->hit < 1)
      victim->hit = 1;

   update_pos(victim);
   act(AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT);
   return;
}
void do_mount								   		(CHAR_DATA *ch, const char *argument){
   CHAR_DATA *victim;
   int dice;
   int check;
   int cost;

   dice = number_percent();
   check = (LEARNED(ch, gsn_mount)/2) + get_curr_dex(ch);
   cost = 10;

   if(!IS_NPC(ch) && ch->level < skill_table[gsn_mount]->skill_level[ch->Class])
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YI don't think that would be a good idea.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->mount)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou're already mounted!&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if((victim = get_char_room(ch, argument)) == NULL)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou can't find that here.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(!IS_NPC(victim) || !xIS_SET(victim->act, ACT_MOUNTABLE))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou can't mount that!&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(xIS_SET(victim->act, ACT_MOUNTED))
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat mount already has a rider.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(victim->position < POS_STANDING)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYour mount must be standing.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(victim->position == POS_FIGHTING || victim->fighting)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYour mount is moving around too much.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(ch->move < cost)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou don't have enough fatigue.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(dice < check)
     {
      xSET_BIT(victim->act, ACT_MOUNTED);
      ch->mount = victim;
      act(AT_SKILL, "&YYou mount &G$N&Y.&D", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "&Y$n&W skillfully mounts &Y$N.&D", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "&R$n&Y mounts you.&D", ch, NULL, victim, TO_VICT);
      learn_from_success(ch, gsn_mount);
      ch->position = POS_MOUNTED;
      ch->move = ch->move - cost;
     }
   else
     {
     act(AT_SKILL, "&YYou unsuccessfully try to mount &G$N&Y.&D", ch, NULL, victim, TO_CHAR);
     act(AT_SKILL, "&Y$n&W unsuccessfully attempts to mount &Y$N&W.&D", ch, NULL, victim, TO_NOTVICT);
     act(AT_SKILL, "&R$n&Y tries to mount you.&D", ch, NULL, victim, TO_VICT);
      ch->move = ch->move - cost;
     }
   return;
}
void do_dismount							   		(CHAR_DATA *ch, const char *argument){
   CHAR_DATA *victim;
   int dice;
   int check;
   int cost;
   int damage;

   dice = number_percent();
   check = LEARNED(ch, gsn_mount) + get_curr_dex(ch);
   cost = 20;

   damage = rand() %10;

   if((victim = ch->mount) == NULL)
     {
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou're not mounted.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     return;
     }

   if(dice < check)
     {
     act(AT_SKILL, "&YYou dismount &G$N&Y.&D", ch, NULL, victim, TO_CHAR);
     act(AT_SKILL, "&Y$n&W skillfully dismounts &Y$N&W.&D", ch, NULL, victim, TO_NOTVICT);
     act(AT_SKILL, "&R$n&Y dismounts you.&D", ch, NULL, victim, TO_VICT);
     xREMOVE_BIT(victim->act, ACT_MOUNTED);
     ch->mount = NULL;
     ch->position = POS_STANDING;
     learn_from_success(ch, gsn_mount);
     ch->move = UMAX(0, ch->move - cost);
     }
   else
     {
     act( AT_SKILL, "&YYou fall off while dismounting &G$N&Y.&D", ch, NULL, victim, TO_CHAR);
     act( AT_SKILL, "&Y$n&W falls off of &Y$N&W while dismounting.&D", ch, NULL, victim, TO_NOTVICT);
     act( AT_SKILL, "&R$n&Y falls off your back.&D", ch, NULL, victim, TO_VICT);
     xREMOVE_BIT(victim->act, ACT_MOUNTED);
     ch->mount = NULL;
     ch->position = POS_SITTING;
     {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&RYou receive %d points of damage.&D", damage); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
     ch->hit = ch->hit - damage;
     update_pos(ch);
     ch->move = UMAX(0, ch->move - cost);
   }
   return;
}
void do_poison_weapon						   		(CHAR_DATA *ch, const char *argument){
   OBJ_DATA *obj;
   OBJ_DATA *pobj;
   OBJ_DATA *wobj;
   char arg[MAX_INPUT_LENGTH];
   int percent;

   if( !IS_NPC( ch ) && ch->level < skill_table[gsn_poison_weapon]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What do you think you are, a thief?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What are you trying to poison?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( ch->fighting )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "While you're fighting?  Nice try."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( ms_find_obj( ch ) )
      return;

   if( !( obj = get_obj_carry( ch, arg ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have that weapon."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( obj->item_type != ITEM_WEAPON )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That item is not a weapon."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_POISONED ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That weapon is already poisoned."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_CLANOBJECT ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It doesn't appear to be fashioned of a poisonable material."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   /*
    * Now we have a valid weapon...check to see if we have the powder. 
    */
   for( pobj = ch->first_carrying; pobj; pobj = pobj->next_content )
   {
      if( pobj->pIndexData->vnum == OBJ_VNUM_BLACK_POWDER )
         break;
   }
   if( !pobj )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have the black poison powder."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   /*
    * Okay, we have the powder...do we have water? 
    */
   for( wobj = ch->first_carrying; wobj; wobj = wobj->next_content )
   {
      if( wobj->item_type == ITEM_DRINK_CON && wobj->value[1] > 0 && wobj->value[2] == 0 )
         break;
   }
   if( !wobj )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no water to mix with the powder."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   /*
    * Great, we have the ingredients...but is the thief smart enough? 
    */
   if( !IS_NPC( ch ) && get_curr_wis( ch ) < 16 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't quite remember what to do..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   /*
    * And does the thief have steady enough hands? 
    */
   if( !IS_NPC( ch ) && ( ( get_curr_dex( ch ) < 17 ) || ch->pcdata->condition[COND_DRUNK] > 0 ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your hands aren't steady enough to properly mix the poison."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   WAIT_STATE( ch, skill_table[gsn_poison_weapon]->beats );

   percent = ( number_percent(  ) - get_curr_lck( ch ) - 14 );

   /*
    * Check the skill percentage 
    */
   separate_obj( pobj );
   separate_obj( wobj );
   if( !can_use_skill( ch, percent, gsn_poison_weapon ) )
   {
      set_char_color( AT_RED, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You failed and spill some on yourself.  Ouch!"); act(AT_RED, "$t", ch, cMSG, NULL, TO_CHAR);}
      set_char_color( AT_GREY, ch );
      damage( ch, ch, ch->level, gsn_poison_weapon );
      act( AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM );
      extract_obj( pobj );
      extract_obj( wobj );
      return;
   }
   separate_obj( obj );
   /*
    * Well, I'm tired of waiting.  Are you? 
    */
   act( AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR );
   act( AT_RED, "$n mixes $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_ROOM );
   act( AT_GREEN, "You pour the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_CHAR );
   act( AT_GREEN, "$n pours the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_ROOM );
   xSET_BIT( obj->extra_flags, ITEM_POISONED );
   obj->cost *= 2;
   /*
    * Set an object timer.  Don't want proliferation of poisoned weapons 
    */
   obj->timer = UMIN( obj->level, ch->level );

   if( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      obj->timer *= 2;

   if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      obj->timer *= 2;

   /*
    * WHAT?  All of that, just for that one bit?  How lame. ;) 
    */
   act( AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR );
   act( AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM );
   extract_obj( pobj );
   extract_obj( wobj );
   learn_from_success( ch, gsn_poison_weapon );
   return;
}
void do_circle								   		(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg );

   if( ch->mount )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't circle while mounted."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Circle around whom?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( victim == ch )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "How can you sneak up on yourself?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( is_safe( ch, victim, TRUE ) )
      return;

   if( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL || ( obj->value[3] != 11 && obj->value[3] != 2 ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to wield a piercing or stabbing weapon."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->fighting )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't circle when you aren't fighting."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !victim->fighting )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't circle around a person who is not fighting."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( victim->num_fighting < 2 )
   {
      act( AT_PLAIN, "You can't circle around them without a distraction.", ch, NULL, victim, TO_CHAR );
      return;
   }

   percent = number_percent(  ) - ( get_curr_lck( ch ) - 16 ) + ( get_curr_lck( victim ) - 13 );

   check_attacker( ch, victim );
   WAIT_STATE( ch, skill_table[gsn_circle]->beats );
   if( can_use_skill( ch, percent, gsn_circle ) )
   {
      learn_from_success( ch, gsn_circle );
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      global_retcode = multi_hit( ch, victim, gsn_circle );
      adjust_favor( ch, 10, 1 );
      check_illegal_pk( ch, victim );
   }
   else
   {
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      global_retcode = damage( ch, victim, 0, gsn_circle );
   }
   return;
}
ch_ret one_hit args									((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
bool check_illegal_psteal						 	(CHAR_DATA *ch, CHAR_DATA *victim){
/*   char log_buf[MAX_STRING_LENGTH]; */

   if( !IS_NPC( victim ) && !IS_NPC( ch ) )
   {
      if( ( !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
            || ch->level - victim->level > 10
            || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
          && ( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 ) && ch != victim )
      {
         /*
          * snprintf( log_buf, MAX_STRING_LENGTH, "%s illegally stealing from %s at %d",
          * (IS_NPC(ch) ? ch->short_descr : ch->name),
          * victim->name,
          * victim->in_room->vnum );
          * log_string( log_buf );
          * to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
          */
         return TRUE;
      }
   }
   return FALSE;
}
//A remote sighting is not movement; avoid triggering entry/exit programs and wilderness spawns.
static bool SKILL_CAN_SCAN_ROOM                   (CHAR_DATA *ch, ROOM_INDEX_DATA *room){
   if(!room) return FALSE;
   if(room_is_dark(room) && !IS_AFFECTED(ch, AFF_TRUESIGHT) && !IS_AFFECTED(ch, AFF_INFRARED)
   && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_HOLYLIGHT))) return FALSE;
   return TRUE;
}
static CHAR_DATA *SKILL_FIND_REMOTE_TARGET        (CHAR_DATA *ch, ROOM_INDEX_DATA *room, const char *name){
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   int number, count, matchtype, vnum;

   if(!SKILL_CAN_SCAN_ROOM(ch, room)) return NULL;
   number = number_argument(name, arg);
   vnum = get_trust(ch) >= LEVEL_SAVIOR && is_number(arg) ? atoi(arg) : -1;
   for(matchtype = 0; matchtype < 2; matchtype++)
   {
      count = 0;
      for(victim = room->first_person; victim; victim = victim->next_in_room)
      {
         if(victim == ch || !can_see(ch, victim)) continue;
         if(matchtype == 0 ? (!nifty_is_name(arg, victim->name) && (!IS_NPC(victim) || vnum != victim->pIndexData->vnum))
                          : (vnum != -1 || !nifty_is_name_prefix(arg, victim->name))) continue;
         if(number == 0 && !IS_NPC(victim)) return victim;
         if(++count == number) return victim;
      }
   }
   return NULL;
}
void do_scan                                      (CHAR_DATA *ch, const char *argument){
   ROOM_INDEX_DATA *room;
   EXIT_DATA *pexit;
   OBJ_DATA *obj;
   CHAR_DATA *victim;
   int dir, dist, max_dist, cost, check, seen;

   if(!ch->in_room) return;
   if(IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED(ch, AFF_TRUESIGHT)
   && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_HOLYLIGHT)))
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou cannot scan while blinded.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if(!argument || argument[0] == '\0' || (dir = get_door(argument)) < 0)
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YScan which direction? Try: scan north.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   cost = 10;
   if(ch->move < cost)
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou are too exhausted to search the distance.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   ch->move = UMAX(0, ch->move - cost);
   WAIT_STATE(ch, PULSE_VIOLENCE);

   check = URANGE(5, (IS_NPC(ch) ? 70 : LEARNED(ch, gsn_scan) / 2 + get_curr_lck(ch)), 95);
   if(number_percent() > check)
   {
      act(AT_SKILL, "&YYou peer into the distance, but cannot make out anything useful.&D", ch, NULL, NULL, TO_CHAR);
      learn_from_failure(ch, gsn_scan);
      return;
   }

   max_dist = 8;
   if(ch->level < 50) --max_dist;
   if(ch->level < 40) --max_dist;
   if(ch->level < 30) --max_dist;
   if(IS_VAMPIRE(ch) && time_info.hour > 5 && time_info.hour < 21) max_dist = 1;

   room = ch->in_room;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&CYou search the distance to the %s...&D", dir_name[dir]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   for(dist = 1; dist <= max_dist;)
   {
      pexit = get_exit(room, dir);
      if(!pexit || !pexit->to_room)
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYour view ends at an impassable barrier.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         break;
      }
      if(IS_SET(pexit->exit_info, EX_CLOSED))
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YA closed passage blocks your view.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         break;
      }
      if(room_is_private(pexit->to_room) && ch->level < LEVEL_GREATER)
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou cannot see into that private place.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         break;
      }

      room = pexit->to_room;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&W[%d] &Y%s&D", dist, room->name ? room->name : "Unknown location"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      if(!SKILL_CAN_SCAN_ROOM(ch, room))
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "    &zDeep shadows conceal any details.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         break;
      }

      seen = 0;
      for(obj = room->first_content; obj; obj = obj->next_content)
      {
         if(!can_see_obj(ch, obj)) continue;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "    &w%s&D", obj->short_descr ? obj->short_descr : "Something"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         seen++;
         if(seen >= 10) break;
      }
      for(victim = room->first_person; victim; victim = victim->next_in_room)
      {
         if(victim == ch || !can_see(ch, victim)) continue;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "    &Y%s&D", PERS(victim, ch)); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         seen++;
         if(seen >= 15) break;
      }
      if(!seen) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "    &wNothing unusual catches your eye.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      switch(room->sector_type)
      {
         case SECT_FOREST:
         case SECT_CITY:
         case SECT_DESERT:
         case SECT_HILLS:           dist += 2; break;
         case SECT_WATER_SWIM:
         case SECT_WATER_NOSWIM:    dist += 3; break;
         case SECT_MOUNTAIN:
         case SECT_UNDERWATER:
         case SECT_OCEANFLOOR:     dist += 4; break;
         default:                   dist++;      break;
      }
   }
   learn_from_success(ch, gsn_scan);
}
CHAR_DATA *scan_for_victim                      (CHAR_DATA *ch, EXIT_DATA *pexit, const char *name){
   ROOM_INDEX_DATA *room;
   short dist, max_dist, dir;
   CHAR_DATA *victim;

   if(!ch || !ch->in_room || IS_AFFECTED(ch, AFF_BLIND) || !pexit || !name || !*name) return NULL;
   room = ch->in_room;
   max_dist = 8;
   if(IS_VAMPIRE(ch) && time_info.hour < 21 && time_info.hour > 5) max_dist = 1;
   if(ch->level < 50) --max_dist;
   if(ch->level < 40) --max_dist;
   if(ch->level < 30) --max_dist;
   dir = pexit->vdir;

   for(dist = 1; dist <= max_dist;)
   {
      if(IS_SET(pexit->exit_info, EX_CLOSED) || !pexit->to_room) break;
      if(room_is_private(pexit->to_room) && ch->level < LEVEL_GREATER) break;
      room = pexit->to_room;
      if((victim = SKILL_FIND_REMOTE_TARGET(ch, room, name)) != NULL) return victim;
      if(!SKILL_CAN_SCAN_ROOM(ch, room)) break;

      switch(room->sector_type)
      {
         case SECT_FOREST:
         case SECT_CITY:
         case SECT_DESERT:
         case SECT_HILLS:         dist += 2; break;
         case SECT_WATER_SWIM:
         case SECT_WATER_NOSWIM:  dist += 3; break;
         case SECT_MOUNTAIN:
         case SECT_UNDERWATER:
         case SECT_OCEANFLOOR:   dist += 4; break;
         default:                 dist++;      break;
      }
      if(dist >= max_dist) break;
      pexit = get_exit(room, dir);
      if(!pexit) break;
   }
   return NULL;
}
OBJ_DATA *find_projectile							(CHAR_DATA *ch, int type){
	//Search inventory for an appropriate projectile to fire. (Also search open quivers.)
   OBJ_DATA *obj, *obj2;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( obj->item_type == ITEM_QUIVER && !IS_SET( obj->value[1], CONT_CLOSED ) )
         {
            for( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
            {
               if( obj2->item_type == ITEM_PROJECTILE && obj2->value[3] == type )
                  return obj2;
            }
         }
         if( obj->item_type == ITEM_PROJECTILE && obj->value[3] == type )
            return obj;
      }
   }

   return NULL;
}
ch_ret spell_attack									(int, int, CHAR_DATA *, void *);
ch_ret ranged_got_target							(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon, OBJ_DATA *projectile, short dist, short dt, const char *stxt, short color){
	//Perform the actual attack on a victim -Thoric
	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
      /*
       * safe room, bubye projectile 
       */
      if( projectile )
      {
         ch_printf( ch, "Your %s is blasted from existance by a godly presense.", myobj( projectile ) );
         act( color, "A godly presence smites $p!", ch, projectile, NULL, TO_ROOM );
         extract_obj( projectile );
      }
      else
      {
         ch_printf( ch, "Your %s is blasted from existance by a godly presense.", stxt );
         act( color, "A godly presence smites $t!", ch, aoran( stxt ), NULL, TO_ROOM );
      }
      return rNONE;
   }

   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_SENTINEL ) && ch->in_room != victim->in_room )
   {
      /*
       * letsee, if they're high enough.. attack back with fireballs
       * long distance or maybe some minions... go herne! heh..
       *
       * For now, just always miss if not in same room  -Thoric
       */

      if( projectile )
      {

         /*
          * 50% chance of projectile getting lost 
          */
         if( number_percent(  ) < 50 )
            extract_obj( projectile );
         else
         {
            if( projectile->in_obj )
               obj_from_obj( projectile );
            if( projectile->carried_by )
               obj_from_char( projectile );
            obj_to_room( projectile, victim->in_room );
         }
      }
      return damage( ch, victim, 0, dt );
   }

   if( number_percent(  ) > 50 || ( projectile && weapon && can_use_skill( ch, number_percent(  ), gsn_missile_weapons ) ) )
   {
      if( projectile )
         global_retcode = projectile_hit( ch, victim, weapon, projectile, dist );
      else
         global_retcode = spell_attack( dt, ch->level, ch, victim );
   }
   else
   {
      global_retcode = damage( ch, victim, 0, dt );

      if( projectile )
      {
         /*
          * 50% chance of getting lost 
          */
         if( number_percent(  ) < 50 )
            extract_obj( projectile );
         else
         {
            if( projectile->in_obj )
               obj_from_obj( projectile );
            if( projectile->carried_by )
               obj_from_char( projectile );
            obj_to_room( projectile, victim->in_room );
         }
      }
   }
   return global_retcode;
}
ch_ret ranged_attack								(CHAR_DATA *ch, const char *argument, OBJ_DATA *weapon, OBJ_DATA *projectile, short dt, short range){
	//Generic use ranged attack function			-Thoric & Tricops
   CHAR_DATA *victim, *vch;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *was_in_room;
   ROOM_INDEX_DATA *sight_room;
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char temp[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   SKILLTYPE *skill = NULL;
   short dir = -1, dist = 0, color = AT_GREY;
   const char *dtxt = "somewhere";
   const char *stxt = "burst of energy";
   int count;

   if( argument && argument[0] != '\0' && argument[0] == '\'' )
   {
      one_argument( argument, temp );
      argument = temp;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg1 );

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Where?  At who?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   victim = NULL;

   /*
    * get an exit or a victim 
    */
   if( ( pexit = find_door( ch, arg, TRUE ) ) == NULL )
   {
      if( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Aim in what direction?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }
      else
      {
         if( who_fighting( ch ) == victim )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They are too close to release that type of attack!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rNONE;
         }
         /*
          * Taken out because of waitstate 
          * if ( !IS_NPC(ch) && !IS_NPC(victim) )
          * {
          * send_to_char("Pkill like a real pkiller.\r\n", ch );
          * return rNONE;
          * }
          */
      }
   }
   else
      dir = pexit->vdir;

   /*
    * check for ranged attacks from private rooms, etc 
    */
   if( !victim )
   {
      if( xIS_SET( ch->in_room->room_flags, ROOM_PRIVATE ) || xIS_SET( ch->in_room->room_flags, ROOM_SOLITARY ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot perform a ranged attack from a private room."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }
      if( ch->in_room->tunnel > 0 )
      {
         count = 0;
         for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
            ++count;
         if( count >= ch->in_room->tunnel )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This room is too cramped to perform such an attack."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rNONE;
         }
      }
   }

   if( IS_VALID_SN( dt ) )
      skill = skill_table[dt];

   if( pexit && !pexit->to_room )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Are you expecting to fire through a wall!?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   /*
    * Check for obstruction 
    */
   if( pexit && IS_SET( pexit->exit_info, EX_CLOSED ) )
   {
      if( IS_SET( pexit->exit_info, EX_SECRET ) || IS_SET( pexit->exit_info, EX_DIG ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Are you expecting to fire through a wall!?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Are you expecting to fire through a door!?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   vch = NULL;
   if( pexit && arg1[0] != '\0' )
   {
      if( ( vch = scan_for_victim( ch, pexit, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot see your target."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }

      /*
       * don't allow attacks on mobs stuck in another area?
       * if ( IS_NPC(vch) && xIS_SET(vch->act, ACT_STAY_AREA)
       * &&   ch->in_room->area != vch->in_room->area) )
       * {
       * }
       */
      /*
       * don't allow attacks on mobs that are in a no-missile room --Shaddai 
       */
      if( xIS_SET( vch->in_room->room_flags, ROOM_NOMISSILE ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't get a clean shot off."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }
      /*
       * Taken out cause of wait state
       * if ( !IS_NPC(ch) && !IS_NPC(vch) )
       * {
       * send_to_char("Pkill like a real pkiller.\r\n", ch );
       * return rNONE;
       * }
       */

      /*
       * can't properly target someone heavily in battle 
       */
      if( vch->num_fighting > MAX_FIGHT )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is too much activity there for you to get a clear shot."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }
   }

   if( vch )
   {
      if( !IS_NPC( vch ) && !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NICE ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're too nice to do that!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rNONE;
      }
      if( vch && is_safe( ch, vch, TRUE ) )
         return rNONE;
   }
   was_in_room = ch->in_room;

   if( projectile )
   {
      separate_obj( projectile );
      if( pexit )
      {
         if( weapon )
         {
            act( AT_GREY, "You fire $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
            act( AT_GREY, "$n fires $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
         }
         else
         {
            act( AT_GREY, "You throw $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
            act( AT_GREY, "$n throw $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
         }
      }
      else
      {
         if( weapon )
         {
            act( AT_GREY, "You fire $p at $N.", ch, projectile, victim, TO_CHAR );
            act( AT_GREY, "$n fires $p at $N.", ch, projectile, victim, TO_NOTVICT );
            act( AT_GREY, "$n fires $p at you!", ch, projectile, victim, TO_VICT );
         }
         else
         {
            act( AT_GREY, "You throw $p at $N.", ch, projectile, victim, TO_CHAR );
            act( AT_GREY, "$n throws $p at $N.", ch, projectile, victim, TO_NOTVICT );
            act( AT_GREY, "$n throws $p at you!", ch, projectile, victim, TO_VICT );
         }
      }
   }
   else if( skill )
   {
      if( skill->noun_damage && skill->noun_damage[0] != '\0' )
         stxt = skill->noun_damage;
      else
         stxt = skill->name;
      /*
       * a plain "spell" flying around seems boring 
       */
      if( !str_cmp( stxt, "spell" ) )
         stxt = "magical burst of energy";
      if( skill->type == SKILL_SPELL )
      {
         color = AT_MAGIC;
         if( pexit )
         {
            act( AT_MAGIC, "You release $t $T.", ch, aoran( stxt ), dir_name[dir], TO_CHAR );
            act( AT_MAGIC, "$n releases $t $T.", ch, aoran(stxt), dir_name[dir], TO_ROOM );
         }
         else
         {
            act( AT_MAGIC, "You release $t at $N.", ch, aoran( stxt ), victim, TO_CHAR );
            act( AT_MAGIC, "$n releases $s $t at $N.", ch, stxt, victim, TO_NOTVICT );
            act( AT_MAGIC, "$n releases $s $t at you!", ch, stxt, victim, TO_VICT );
         }
      }
   }
   else
   {
      bug( "Ranged_attack: no projectile, no skill dt %d", dt );
      return rNONE;
   }

   /*
    * victim in same room 
    */
   if( victim )
   {
      check_illegal_pk( ch, victim );
      check_attacker( ch, victim );
      return ranged_got_target( ch, victim, weapon, projectile, 0, dt, stxt, color );
   }

   /*
    * assign scanned victim 
    */
   victim = vch;

   /*
    * reverse direction text from move_char 
    */
   dtxt = rev_exit( pexit->vdir );

   //Trace rooms, not the shooter. Virtual travel would trigger room entry/exit logic.
   sight_room = was_in_room;
   while(dist <= range)
   {
      if(!pexit || !pexit->to_room)
      {
         act(color, "&YYour shot strikes an impassable wall.&D", ch, NULL, NULL, TO_CHAR);
         break;
      }
      if(IS_SET(pexit->exit_info, EX_CLOSED))
      {
         act(color, "&YYour shot strikes a closed passage.&D", ch, NULL, NULL, TO_CHAR);
         break;
      }

      sight_room = pexit->to_room;
      if(xIS_SET(sight_room->room_flags, ROOM_NOMISSILE))
      {
         act(color, "&YYour shot cannot enter that area.&D", ch, NULL, NULL, TO_CHAR);
         break;
      }

      //Show distant observers the missile travelling through their room.
      for(vch = sight_room->first_person; vch; vch = vch->next_in_room)
      {
         if(vch == ch) continue;
         if(projectile)
            ch_printf(vch, "&w%s flies in from %s.&D\r\n", projectile->short_descr ? projectile->short_descr : "A projectile", dtxt);
         else
            ch_printf(vch, "&B%s surges in from %s.&D\r\n", stxt, dtxt);
      }

      if(!victim)
      {
         for(vch = sight_room->first_person; vch; vch = vch->next_in_room)
         {
            if(((IS_NPC(ch) && !IS_NPC(vch)) || (!IS_NPC(ch) && IS_NPC(vch))) && number_bits(1) == 0)
            {
               victim = vch;
               break;
            }
         }
         if(victim && is_safe(ch, victim, TRUE)) victim = NULL;
      }

      if(victim && sight_room == victim->in_room)
      {
         check_illegal_pk(ch, victim);
         check_attacker(ch, victim);
         return ranged_got_target(ch, victim, weapon, projectile, dist, dt, stxt, color);
      }

      if(dist == range || dir == 10)
      {
         if(projectile)
            act(color, "&YYour shot falls harmlessly to the ground in the distance.&D", ch, NULL, NULL, TO_CHAR);
         else
            act(color, "&YYour magic fades before finding a target.&D", ch, NULL, NULL, TO_CHAR);
         break;
      }

      pexit = get_exit(sight_room, dir);
      dist++;
   }

   if(projectile)
   {
      if(projectile->in_obj) obj_from_obj(projectile);
      if(projectile->carried_by) obj_from_char(projectile);
      obj_to_room(projectile, sight_room);
   }

   return rNONE;
}
void do_fire								   		(CHAR_DATA *ch, const char *argument){
	//Fire <direction> <target>
	//Fire a projectile from a missile weapon (bow, crossbow, etc)									
   OBJ_DATA *arrow;
   OBJ_DATA *bow;
   short max_dist;

   if( argument[0] == '\0' || !str_cmp( argument, " " ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fire where at who?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A magical force prevents you from attacking."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Find the projectile weapon
    */
   if( ( bow = get_eq_char( ch, WEAR_MISSILE_WIELD ) ) != NULL )
      if( !( bow->item_type == ITEM_MISSILE_WEAPON ) )
         bow = NULL;

   if( !bow )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not wielding a missile weapon."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * modify maximum distance based on bow-type and ch's class/str/etc 
    */
   max_dist = URANGE( 1, bow->value[4], 10 );

   if( ( arrow = find_projectile( ch, bow->value[3] ) ) == NULL )
   {
      const char *msg = "You have nothing to fire...\r\n";

      switch ( bow->value[3] )
      {
         case DAM_BOLT:
            msg = "You have no bolts...\r\n";
            break;
         case DAM_ARROW:
            msg = "You have no arrows...\r\n";
            break;
         case DAM_DART:
            msg = "You have no darts...\r\n";
            break;
         case DAM_STONE:
            msg = "You have no slingstones...\r\n";
            break;
         case DAM_PEA:
            msg = "You have no peas...\r\n";
            break;
      }
      send_to_char( msg, ch );
      return;
   }

   /*
    * Add wait state to fire for pkill, etc... 
    */
   WAIT_STATE( ch, 6 );

   /*
    * handle the ranged attack 
    */
   ranged_attack( ch, argument, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

   return;
}
bool mob_fire								       	(CHAR_DATA *ch, const char *name){
	//Attempt to fire at a victim. (Returns FALSE if no attempt was made)
   OBJ_DATA *arrow;
   OBJ_DATA *bow;
   short max_dist;

   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      return FALSE;

   if( ( bow = get_eq_char( ch, WEAR_MISSILE_WIELD ) ) != NULL )
      if( !( bow->item_type == ITEM_MISSILE_WEAPON ) )
         bow = NULL;

   if( !bow )
      return FALSE;

   /*
    * modify maximum distance based on bow-type and ch's class/str/etc 
    */
   max_dist = URANGE( 1, bow->value[4], 10 );

   if( ( arrow = find_projectile( ch, bow->value[3] ) ) == NULL )
      return FALSE;

   ranged_attack( ch, name, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

   return TRUE;
}
void do_slice								   		(CHAR_DATA *ch, const char *argument){
   OBJ_DATA *corpse;
   OBJ_DATA *obj;
   OBJ_DATA *slice;
   MOB_INDEX_DATA *pMobIndex;
   char buf[MAX_STRING_LENGTH];

   /*
    * Noticed that it was checking gsn_kick.  Bug report by Li'l Lukey
    */

   if( !IS_NPC( ch ) && !IS_IMMORTAL( ch ) && ch->level < skill_table[gsn_slice]->skill_level[ch->Class] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not learned in this skill."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "From what do you wish to slice meat?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }


   if( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
       || ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3 && obj->value[3] != 11 ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need to wield a sharp weapon."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( corpse = get_obj_here( ch, argument ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't find that here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( corpse->item_type != ITEM_CORPSE_NPC || corpse->value[3] < 75 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not a suitable source of meat."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( ( pMobIndex = get_mob_index( ( short )-( corpse->value[2] ) ) ) == NULL )
   {
      bug( "%s", "Can not find mob for value[2] of corpse, do_slice" );
      return;
   }

   if( get_obj_index( OBJ_VNUM_SLICE ) == NULL )
   {
      bug( "Vnum %d not found for do_slice!", OBJ_VNUM_SLICE );
      return;
   }

   if( !can_use_skill( ch, number_percent(  ), gsn_slice ) && !IS_IMMORTAL( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You fail to slice the meat properly."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( number_percent(  ) + ( get_curr_dex( ch ) - 13 ) < 10 )
      {
         act( AT_BLOOD, "You cut yourself!", ch, NULL, NULL, TO_CHAR );
         damage( ch, ch, ch->level, gsn_slice );
      }
      return;
   }

   slice = create_object( get_obj_index( OBJ_VNUM_SLICE ), 0 );

   snprintf( buf, MAX_STRING_LENGTH, "meat fresh slice %s", pMobIndex->player_name );
   STRFREE( slice->name );
   slice->name = STRALLOC( buf );

   snprintf( buf, MAX_STRING_LENGTH, "a slice of raw meat from %s", pMobIndex->short_descr );
   STRFREE( slice->short_descr );
   slice->short_descr = STRALLOC( buf );

   snprintf( buf, MAX_STRING_LENGTH, "A slice of raw meat from %s lies on the ground.", pMobIndex->short_descr );
   STRFREE( slice->description );
   slice->description = STRALLOC( buf );

   act( AT_BLOOD, "$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM );
   act( AT_BLOOD, "You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR );

   obj_to_char( slice, ch );
   corpse->value[3] -= 25;
   learn_from_success( ch, gsn_slice );
   return;
}
void do_style								   		(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];
/*  char buf[MAX_INPUT_LENGTH];
    int percent; */

   if( IS_NPC( ch ) )
      return;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wChoose: standard, defensive, evasive, aggressive, berserk. (current: %s&w)", ch->style == STYLE_BERSERK ? "&Rberserk" :
                       ch->style == STYLE_AGGRESSIVE ? "&Raggressive" :
                       ch->style == STYLE_DEFENSIVE ? "&Ydefensive" :
                       ch->style == STYLE_EVASIVE ? "&Yevasive" : "standard"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_prefix( arg, "evasive" ) )
   {
      if( ch->level < skill_table[tes_action_evasive_style]->skill_level[ch->Class] )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet learned enough to fight evasively."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      WAIT_STATE( ch, skill_table[tes_action_evasive_style]->beats );
      if( number_percent(  ) < LEARNED( ch, tes_action_evasive_style ) )
      {
         /*
          * success 
          */
         if( ch->fighting )
         {
            ch->position = POS_EVASIVE;
            learn_from_success( ch, tes_action_evasive_style );
            if( ch->in_room )
               act( AT_ACTION, "$n falls back into an evasive stance.", ch, NULL, NULL, TO_ROOM );
         }
         ch->style = STYLE_EVASIVE;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You adopt an evasive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         /*
          * failure 
          */
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You nearly trip in a lame attempt to adopt an evasive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !str_prefix( arg, "defensive" ) )
   {
      if( ch->level < skill_table[tes_action_defensive_style]->skill_level[ch->Class] )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet learned enough to fight defensively."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      WAIT_STATE( ch, skill_table[tes_action_defensive_style]->beats );
      if( number_percent(  ) < LEARNED( ch, tes_action_defensive_style ) )
      {
         /*
          * success 
          */
         if( ch->fighting )
         {
            ch->position = POS_DEFENSIVE;
            learn_from_success( ch, tes_action_defensive_style );
            if( ch->in_room )
               act( AT_ACTION, "$n moves into a defensive posture.", ch, NULL, NULL, TO_ROOM );
         }
         ch->style = STYLE_DEFENSIVE;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You adopt a defensive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         /*
          * failure 
          */
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You nearly trip in a lame attempt to adopt a defensive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !str_prefix( arg, "standard" ) )
   {
      if( ch->level < skill_table[tes_action_standard_style]->skill_level[ch->Class] )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet learned enough to fight in the standard style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      WAIT_STATE( ch, skill_table[tes_action_standard_style]->beats );
      if( number_percent(  ) < LEARNED( ch, tes_action_standard_style ) )
      {
         /*
          * success 
          */
         if( ch->fighting )
         {
            ch->position = POS_FIGHTING;
            learn_from_success( ch, tes_action_standard_style );
            if( ch->in_room )
               act( AT_ACTION, "$n switches to a standard fighting style.", ch, NULL, NULL, TO_ROOM );
         }
         ch->style = STYLE_FIGHTING;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You adopt a standard fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         /*
          * failure 
          */
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You nearly trip in a lame attempt to adopt a standard fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !str_prefix( arg, "aggressive" ) )
   {
      if( ch->level < skill_table[tes_action_aggressive_style]->skill_level[ch->Class] )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet learned enough to fight aggressively."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      WAIT_STATE( ch, skill_table[tes_action_aggressive_style]->beats );
      if( number_percent(  ) < LEARNED( ch, tes_action_aggressive_style ) )
      {
         /*
          * success 
          */
         if( ch->fighting )
         {
            ch->position = POS_AGGRESSIVE;
            learn_from_success( ch, tes_action_aggressive_style );
            if( ch->in_room )
               act( AT_ACTION, "$n assumes an aggressive stance.", ch, NULL, NULL, TO_ROOM );
         }
         ch->style = STYLE_AGGRESSIVE;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You adopt an aggressive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         /*
          * failure 
          */
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You nearly trip in a lame attempt to adopt an aggressive fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !str_prefix( arg, "berserk" ) )
   {
      if( ch->level < skill_table[tes_action_berserk_style]->skill_level[ch->Class] )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have not yet learned enough to fight as a berserker."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      WAIT_STATE( ch, skill_table[tes_action_berserk_style]->beats );
      if( number_percent(  ) < LEARNED( ch, tes_action_berserk_style ) )
      {
         /*
          * success 
          */
         if( ch->fighting )
         {
            ch->position = POS_BERSERK;
            learn_from_success( ch, tes_action_berserk_style );
            if( ch->in_room )
               act( AT_ACTION, "$n enters a wildly aggressive style.", ch, NULL, NULL, TO_ROOM );
         }
         ch->style = STYLE_BERSERK;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You adopt a berserk fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         /*
          * failure 
          */
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You nearly trip in a lame attempt to adopt a berserk fighting style."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Adopt which fighting style?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   return;
}
bool can_use_skill							   		(CHAR_DATA *ch, int percent, int gsn){
/*  New check to see if you can use skills to support morphs --Shaddai */

   bool check = FALSE;
   if( IS_NPC( ch ) && percent < 85 )
      check = TRUE;
   else if( !IS_NPC( ch ) && percent < LEARNED( ch, gsn ) )
      check = TRUE;
   else if( ch->morph && ch->morph->morph && ch->morph->morph->skills &&
            ch->morph->morph->skills[0] != '\0' &&
            is_name( skill_table[gsn]->name, ch->morph->morph->skills ) && percent < 85 )
      check = TRUE;
   if( ch->morph && ch->morph->morph && ch->morph->morph->no_skills &&
       ch->morph->morph->no_skills[0] != '\0' && is_name( skill_table[gsn]->name, ch->morph->morph->no_skills ) )
      check = FALSE;
   if( skill_table[gsn]->guild == 99 && !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
      check = FALSE;
   return check;
}






