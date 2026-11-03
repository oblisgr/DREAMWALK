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
 *             Commands for personal player settings/statictics             *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"
#include <math.h>
#include <stdarg.h>

//Locals
const char *tiny_affect_loc_name	(int location);
char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];	
void do_gold						(CHAR_DATA *ch, const char *argument){
   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "You have %s gold pieces.\r\n", num_punct( ch->gold ) );
   return;
}
void do_worth						(CHAR_DATA *ch, const char *argument){
	if(IS_NPC(ch))
    return;

	set_pager_color(AT_SCORE, ch);
	pager_printf(ch, "\r\nWorth for %s%s.\r\n", ch->name, ch->pcdata->title ? ch->pcdata->title : "");
	send_to_pager(" --------------------------------------------------------------\r\n", ch);
		 if(!ch->pcdata->deity) 		snprintf(buf, MAX_STRING_LENGTH, "%s", "N/A"		);
	else if(ch->pcdata->favor >  2250) 	snprintf(buf, MAX_STRING_LENGTH, "%s", "Loved"		);
	else if(ch->pcdata->favor >  2000) 	snprintf(buf, MAX_STRING_LENGTH, "%s", "Cherished"	);
	else if(ch->pcdata->favor >  1750)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Honored" 	);
	else if(ch->pcdata->favor >  1500)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Praised" 	);
	else if(ch->pcdata->favor >  1250)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Favored" 	);
	else if(ch->pcdata->favor >  1000)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Respected" 	);
	else if(ch->pcdata->favor >   750)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Liked" 		);
	else if(ch->pcdata->favor >   250)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Tolerated" 	);
	else if(ch->pcdata->favor >  -250)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Ignored" 	);
	else if(ch->pcdata->favor >  -750)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Shunned" 	);
	else if(ch->pcdata->favor > -1000)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Disliked" 	);
	else if(ch->pcdata->favor > -1250)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Dishonored" );
	else if(ch->pcdata->favor > -1500)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Disowned" 	);
	else if(ch->pcdata->favor > -1750)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Abandoned" 	);
	else if(ch->pcdata->favor > -2000)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Despised" 	);
	else if(ch->pcdata->favor > -2250)	snprintf(buf, MAX_STRING_LENGTH, "%s", "Hated" 		);
	else								snprintf(buf, MAX_STRING_LENGTH, "%s", "Damned" 	);

	if( ch->level < 10 )
	{
		     if(ch->alignment >  900) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Lawful good" 	);
		else if(ch->alignment >  700) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Neutral good" 	);
		else if(ch->alignment >  350) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Chaotic neutral" );
		else if(ch->alignment >  100) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Lawful neutral" 	);
		else if(ch->alignment > -100) snprintf(buf2, MAX_STRING_LENGTH, "%s", "True neutral" 	);
		else if(ch->alignment > -350) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Chaotic neutral" );
		else if(ch->alignment > -700) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Lawful evil" 	);
		else if(ch->alignment > -900) snprintf(buf2, MAX_STRING_LENGTH, "%s", "Neutral evil" 	);
		else 						  snprintf(buf2, MAX_STRING_LENGTH, "%s", "Chaotic evil" 	);
	}
	else
	snprintf(buf2, MAX_STRING_LENGTH, "%d", ch->alignment);
	pager_printf(ch, " Level: %-4d  Favor: %-11.11s  Alignment: %-16.16s\r\n", ch->level, buf, buf2);
	pager_printf(ch, " Experience: %d  Gold: %d\r\n", ch->exp, ch->gold);
	send_to_pager(" --------------------------------------------------------------\r\n", ch);
	
	switch (ch->style)
	{
		case STYLE_EVASIVE:
			snprintf(buf, MAX_STRING_LENGTH, "%s", "Evasive");
			break;
		case STYLE_DEFENSIVE:
			snprintf(buf, MAX_STRING_LENGTH, "%s", "Defensive");
			break;
		case STYLE_AGGRESSIVE:
			snprintf(buf, MAX_STRING_LENGTH, "%s", "Aggressive");
			break;
		case STYLE_BERSERK:
			snprintf(buf, MAX_STRING_LENGTH, "%s", "Berserk");
			break;
		default:
			snprintf(buf, MAX_STRING_LENGTH, "%s", "Standard");
			break;
	}
	pager_printf(ch, " Glory: %-4d  Weight: %-5d  Style: %-13.13s\r\n", ch->pcdata->quest_curr, ch->carry_weight, buf);
	send_to_pager(" --------------------------------------------------------------\r\n", ch);
	
	if(ch->level < 15 && !IS_PKILL(ch))
		pager_printf(ch, " Hitroll: hidden    Damroll: hidden\r\n");
	else
		pager_printf(ch, " Hitroll: %+d  Damroll: %+d\r\n", GET_HITROLL(ch), GET_DAMROLL(ch));
		send_to_pager(" --------------------------------------------------------------\r\n", ch);
	return;
}
void do_score											(CHAR_DATA *ch, const char *argument){
	char arg[MAX_INPUT_LENGTH], page[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = ch;

	argument = one_argument(argument, arg);
	page[0] = '\0';

	//CHECK IF ARGUMENT IS A REPORT OR A CHARACTER NAME
	if(arg[0] 	&& str_cmp(arg, "character"	) 
				&& str_cmp(arg, "charsheet"	) 
				&& str_cmp(arg, "talent"	) 
				&& str_cmp(arg, "talentbook") 
				&& str_cmp(arg, "spell"		) 
				&& str_cmp(arg, "spellbook"	) 
				&& str_cmp(arg, "magic"		) 
				&& str_cmp(arg, "equipment"	) 
				&& str_cmp(arg, "inventory"	) 
				&& str_cmp(arg, "report"	)
				&& str_cmp(arg, "faction"	) 
				&& str_cmp(arg, "guild"	    ))
	{
		victim = get_char_room(ch, arg);
		if(!victim) {send_to_char("They aren't here.\r\n", ch); return;}
		one_argument(argument, page);
	}
	else
		mudstrlcpy(page, arg, sizeof(page));

	//SELECT WHICH REPORT TO DISPLAY
	     if(!str_cmp(page, "talent"	  )  || !str_cmp(page, "talentbook"))							    DREAMWALK_SHOW_SCORE_TALENT(ch, victim);
	else if(!str_cmp(page, "spell"	  )  || !str_cmp(page, "spellbook" )  || !str_cmp(page, "magic"))	DREAMWALK_SHOW_SCORE_SPELL_SKILL(ch, victim);
	else if(!str_cmp(page, "equipment")  || !str_cmp(page, "inventory" ))								DREAMWALK_SHOW_SCORE_EQUIPMENT_INVENTORY(ch, victim);
	else if(!str_cmp(page, "faction"  )  || !str_cmp(page, "guild" ))									DREAMWALK_SHOW_SCORE_FACTION(ch, victim);
	else if(!str_cmp(page, "report"   ))																DREAMWALK_SHOW_SCORE_REPORT(ch, victim);
	else																							    DREAMWALK_SHOW_SCORE_CHARACTER(ch, victim);

	//ONLY THE DEFAULT PLAYER SCORE SHOWS CONDITION WARNINGS
	if(victim != ch || IS_NPC(ch) || page[0]) return;

	if(ch->pcdata->condition[COND_DRUNK] > 10)		send_to_char("&RYou are drunk.\r\n", ch);
	if(ch->pcdata->condition[COND_THIRST] == 0)		send_to_char("&RYou are dehydrated.\r\n", ch);
	if(ch->pcdata->condition[COND_FULL] == 0)		send_to_char("&RYou are starving.\r\n", ch);
	if(ch->mental_state <= -60)						send_to_char("&YYou feel extremely weary.\r\n", ch);
	else if(ch->mental_state >= 60)					send_to_char("&YYour thoughts are racing.\r\n", ch);
}
const char *tiny_affect_loc_name						(int location){
	//Return ascii name of an affect location.	
	switch (location)
	{
		case APPLY_NONE:    		return "NIL"					  ;
		case APPLY_STR:	    		return "Strength"				  ;
		case APPLY_DEX:     		return "Agility"				  ;
		case APPLY_INT:	    		return "Intelligence"			  ;
		case APPLY_WIS:	    		return "Willpower"				  ;
		case APPLY_CON:	   		 	return "Endurance"				  ;
		case APPLY_CHA:     		return "Personality"			  ;
		case APPLY_LCK:     		return "Luck"					  ;
		case APPLY_SEX:	    		return "Gender"					  ;
		case APPLY_CLASS:   		return "Class"					  ;
		case APPLY_LEVEL:   		return "Level"					  ;
		case APPLY_AGE:     		return "Age"					  ;
		case APPLY_MANA:    		return "Magicka"				  ;
		case APPLY_HIT:	    		return "Health"					  ;
		case APPLY_MOVE:    		return "Fatigue"				  ;
		case APPLY_GOLD:    		return "Gold"					  ;
		case APPLY_EXP:     		return "Experience"				  ;
		case APPLY_AC:      		return "Armor class"  			  ;
		case APPLY_HITROLL: 		return "Attack dice roll"		  ;
		case APPLY_DAMROLL: 		return "Damage dice roll"		  ;
		case APPLY_SAVING_POISON: 	return "Saving throw vs poison"	  ;
		case APPLY_SAVING_ROD:    	return "Saving throw vs rods"	  ;
		case APPLY_SAVING_PARA:   	return "Saving throw vs paralysis";
		case APPLY_SAVING_BREATH: 	return "Saving throw vs breath"	  ;
		case APPLY_SAVING_SPELL:  	return "Saving throw vs spells"	  ;
		case APPLY_HEIGHT:			return "Height"					  ;
		case APPLY_WEIGHT:			return "Weight"					  ;
		case APPLY_AFFECT:			return "Affected by"			  ;
		case APPLY_RESISTANT:		return "Resistance"				  ;
		case APPLY_IMMUNE:			return "Immunity"				  ;
		case APPLY_SUSCEPTIBLE:		return "Weakness"				  ;
		case APPLY_WEAPONSPELL:		return "Weapon spell"			  ;
		case APPLY_BACKSTAB:		return "Backstab"				  ;
		case APPLY_PICK:			return "Picklock"				  ;
		case APPLY_TRACK:			return "Track"					  ;
		case APPLY_STEAL:			return "Pickpocket"				  ;
		case APPLY_SNEAK:			return "Sneak"					  ;
		case APPLY_HIDE:			return "Hide"					  ;
		case APPLY_PALM:			return "Palm"					  ;
		case APPLY_DETRAP:			return "Detrap"					  ;
		case APPLY_DODGE:			return "Dodge"					  ;
		case APPLY_PEEK:			return "Peek"					  ;
		case APPLY_SCAN:			return "Scan"					  ;
		case APPLY_GOUGE:			return "Gouge"					  ;
		case APPLY_SEARCH:			return "Search"					  ;
		case APPLY_MOUNT:			return "Mount"					  ;
		case APPLY_DISARM:			return "Disarm"					  ;
		case APPLY_KICK:			return "Kick"					  ;
		case APPLY_PARRY:			return "Parry"					  ;
		case APPLY_BASH:			return "Bash"					  ;
		case APPLY_STUN:			return "Stun"					  ;
		case APPLY_PUNCH:			return "Punch"					  ;
		case APPLY_CLIMB:			return "Climb"					  ;
		case APPLY_GRIP:			return "Grip"					  ;
		case APPLY_SCRIBE:			return "Scribe"					  ;
		case APPLY_BREW:			return "Brew"					  ;
		case APPLY_WEARSPELL:		return "Wear spell"				  ;
		case APPLY_REMOVESPELL:		return "Remove spell"			  ;
		case APPLY_EMOTION:			return "Emotion"				  ;
		case APPLY_MENTALSTATE:		return "Mental state"			  ;
		case APPLY_STRIPSN:			return "Dispel"					  ;
		case APPLY_REMOVE:			return "Remove"					  ;
		case APPLY_DIG:				return "Dig"					  ;
		case APPLY_FULL:			return "Hunger"					  ;
		case APPLY_THIRST:			return "Thirst"					  ;
		case APPLY_DRUNK:			return "Drunk"					  ;
		case APPLY_BLOOD:			return "Blood"					  ;
		case APPLY_COOK:			return "Cook"					  ;
		case APPLY_RECURRINGSPELL:	return "Recurring"				  ;
		case APPLY_CONTAGIOUS:		return "Contagious"				  ;
		case APPLY_ODOR:			return "Odor"					  ;
		case APPLY_ROOMFLAG:		return "Room flag"				  ;
		case APPLY_SECTORTYPE:		return "Sector type"			  ;
		case APPLY_ROOMLIGHT:		return "Light"					  ;
		case APPLY_TELEVNUM:		return "TELEVN"					  ;
		case APPLY_TELEDELAY:		return "TELEDY"					  ;
   };
   bug("Affect_location_name: unknown location %d.", location);
   return "(?)";
}
const char *get_class									(CHAR_DATA *ch){
	if(IS_NPC(ch) && ch->Class < MAX_NPC_CLASS && ch->Class >= 0) 		return (npc_class[ch->Class]);
	else if(!IS_NPC(ch) && ch->Class < MAX_PC_CLASS && ch->Class >= 0) 	return class_table[ch->Class]->who_name;
	return ("Unknown");
}
const char *get_race									(CHAR_DATA *ch){
	if(IS_NPC(ch))
	{
		if(ch->race >= 0 && ch->race < MAX_NPC_RACE) return npc_race[ch->race];
	}
	else
	{
		if(ch->race >= 0 && ch->race < MAX_PC_RACE) return race_table[ch->race]->race_name;
	}
	return "Unknown";
}
void do_level											(CHAR_DATA *ch, const char *argument){
	//Display current exp, level, and surrounding level exp requirements
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int x, lowlvl, hilvl;

	if(ch->level == 1) lowlvl = 1;
	else 			   lowlvl = UMAX(2, ch->level - 5);
  
	hilvl = URANGE(ch->level, ch->level + 5, MAX_LEVEL);
	set_char_color(AT_SCORE, ch);
	ch_printf(ch, "\r\nExperience required, levels %d to %d:\r\n______________________________________________\r\n\r\n", lowlvl, hilvl);
	snprintf(buf,  MAX_STRING_LENGTH, " experience:  (Current: %12s)", num_punct(ch->exp));
	snprintf(buf2, MAX_STRING_LENGTH, " experience:  (Needed:  %12s)", num_punct(exp_level(ch, ch->level + 1) - ch->exp));
   
	for(x = lowlvl; x <= hilvl; x++)
		ch_printf(ch, " (%2d) %12s%s\r\n", x, num_punct(exp_level(ch, x)), (x == ch->level) ? buf : (x == ch->level + 1) ? buf2 : " experience" );
		send_to_char("______________________________________________\r\n", ch);
}
void do_remains											(CHAR_DATA *ch, const char *argument){
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	bool found = FALSE;

	if(IS_NPC(ch))
    return;

	set_char_color(AT_MAGIC, ch);
	if(!ch->pcdata->deity)
	{
		send_to_pager("You have no deity from which to seek such assistance...\r\n", ch);
		return;
	}

	if(ch->pcdata->favor < ch->level * 2)
	{
		send_to_pager("Your favor is insufficient for such assistance...\r\n", ch);
		return;
	}

	pager_printf(ch, "%s appears in a vision, revealing that your remains... ", ch->pcdata->deity->name);
	snprintf(buf, MAX_STRING_LENGTH, "the corpse of %s", ch->name);
	
	for(obj = first_object; obj; obj = obj->next)
	{
		if(obj->in_room && !str_cmp(buf, obj->short_descr) && (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC))
		{
			found = TRUE;
			pager_printf(ch, "\r\n  - at %s will endure for %d ticks", obj->in_room->name, obj->timer);
		}
	}
	
	if(!found)
		send_to_pager(" no longer exist.\r\n", ch);
	else
	{
		send_to_pager("\r\n", ch);
		ch->pcdata->favor -= ch->level * 2;
	}
	return;
}
void do_affected										(CHAR_DATA *ch, const char *argument){
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	SKILLTYPE *skill;

	if(IS_NPC(ch))
    return;

	set_char_color(AT_SCORE, ch);
	argument = one_argument(argument, arg);
	
	if(!str_cmp(arg, "by"))
	{
		send_to_char_color("\r\n&BImbued with:\r\n", ch);
		ch_printf_color(ch, "&C%s\r\n", !xIS_EMPTY(ch->affected_by) ? affect_bit_name(&ch->affected_by) : "nothing");
		
		if( ch->level >= 20 )
		{
			send_to_char( "\r\n", ch );
			if( ch->resistant > 0 )
			{
				send_to_char_color("&BResistances: ", ch);
				ch_printf_color(ch, "&C%s\r\n", flag_string(ch->resistant, ris_flags));
			}
			
			if(ch->immune > 0)
			{
				send_to_char_color("&BImmunities: ", ch);
				ch_printf_color( ch, "&C%s\r\n", flag_string( ch->immune, ris_flags ) );
			}
			if(ch->susceptible > 0)
			{
				send_to_char_color("&Weaknesses:     ", ch);
				ch_printf_color(ch, "&C%s\r\n", flag_string(ch->susceptible, ris_flags));
			}
		}
		return;
	}

	if(!ch->first_affect)
	{
		send_to_char_color("\r\n&CNo cantrip or skill affects you.\r\n", ch);
	}
	else
	{
		send_to_char("\r\n", ch);
		
		for(paf = ch->first_affect; paf; paf = paf->next)
			if( (skill = get_skilltype(paf->type)) != NULL)
			{
				set_char_color(AT_BLUE, ch);
				send_to_char("Affected:  ", ch);
				set_char_color(AT_SCORE, ch);
				
				if(ch->level >= 20 || IS_PKILL(ch))
				{
					if(paf->duration < 25) set_char_color(AT_WHITE, ch);
					if(paf->duration < 6) set_char_color(AT_WHITE + AT_BLINK, ch);
					ch_printf(ch, "(%5d)   ", paf->duration);
				}
				ch_printf( ch, "%-18s\r\n", skill->name );
			}
	}
	return;
}
void do_inventory										(CHAR_DATA *ch, const char *argument){
	CHAR_DATA *victim = ch;

	// Inventory and equipment now share the same graphical Armory and Pack.
	// Immortals may still inspect characters remotely.
	if(argument && argument[0] && IS_IMMORTAL(ch))
	{
		victim = get_char_world(ch, argument);
		if(!victim) {ch_printf(ch, "There is nobody named %s online.\r\n", argument); return;}
	}
	CHAR_RECALCULATE_FULL(victim);
	DREAMWALK_SHOW_SCORE_EQUIPMENT_INVENTORY(ch, victim);
}
void do_equipment										(CHAR_DATA *ch, const char *argument){
	CHAR_DATA *victim = ch;

	// The normal equipment command now opens the same graphical equipment tab.
	// Preserve the original immortal-only remote inspection behavior.
	if(argument && argument[0] && get_trust(ch) > LEVEL_GOD)
	{
		victim = get_char_world(ch, argument);
		if(!victim) {send_to_char("They're not here.\r\n", ch); return;}
	}
	CHAR_RECALCULATE_FULL(victim);
	DREAMWALK_SHOW_SCORE_EQUIPMENT_INVENTORY(ch, victim);
}
void set_title											(CHAR_DATA *ch, const char *title){
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "%s: NPC.", __func__ );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      mudstrlcpy( buf + 1, title, MAX_STRING_LENGTH - 1 );
   }
   else
      mudstrlcpy( buf, title, MAX_STRING_LENGTH );

   STRFREE( ch->pcdata->title );
   ch->pcdata->title = STRALLOC( buf );
}
void do_title											(CHAR_DATA *ch, const char *argument){
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_WHITE, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The Gods prohibit you from changing your title."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Change your title to what?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   char title[50];
   mudstrlcpy(title, argument, 50);

   smash_tilde( title );
   set_title( ch, title );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
}
void do_homepage										(CHAR_DATA *ch, const char *argument){
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOHOMEPAGE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The Gods prohibit you from changing your homepage."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->homepage )
         ch->pcdata->homepage = str_dup( "" );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your homepage is: %s", show_tilde( ch->pcdata->homepage )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->homepage )
         DISPOSE( ch->pcdata->homepage );
      ch->pcdata->homepage = str_dup( "" );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Homepage cleared."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( strstr( argument, "://" ) )
      mudstrlcpy( buf, argument, MAX_STRING_LENGTH );
   else
      snprintf( buf, MAX_STRING_LENGTH, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->homepage )
      DISPOSE( ch->pcdata->homepage );
   ch->pcdata->homepage = str_dup( buf );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Homepage set."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
}
void do_description										(CHAR_DATA *ch, const char *argument){
	//Set your personal description
   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Monsters are too dumb to do that!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_SET( ch->pcdata->flags, PCFLAG_NODESC ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot set your description."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      bug( "%s: no descriptor", __func__ );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "%s: illegal substate", __func__ );
         return;

      case SUB_RESTRICTED:
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot use this command from within another command."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing( ch, ch->description );
         return;

      case SUB_PERSONAL_DESC:
         STRFREE( ch->description );
         ch->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}
void do_bio												(CHAR_DATA *ch, const char *argument){	
	//Ripped off do_description for whois bio's
   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs cannot set a bio."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOBIO ) )
   {
      set_char_color( AT_RED, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The gods won't allow you to do that!"); act(AT_RED, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      bug( "%s: no descriptor", __func__ );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "%s: illegal substate", __func__ );
         return;

      case SUB_RESTRICTED:
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot use this command from within another command."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing( ch, ch->pcdata->bio );
         return;

      case SUB_PERSONAL_BIO:
         STRFREE( ch->pcdata->bio );
         ch->pcdata->bio = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}
void do_statreport										(CHAR_DATA *ch, const char *argument){
	//New stat and statreport command
   char buf[MAX_STRING_LENGTH];

   if(IS_NPC(ch))
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

	if( IS_VAMPIRE( ch ) )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.", ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		snprintf(buf, MAX_STRING_LENGTH, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.", 
			ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp);
		act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
	}
	else
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		snprintf( buf, MAX_STRING_LENGTH, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.", 
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);
		act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
	}

	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your base stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.", ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	snprintf(buf, MAX_STRING_LENGTH, "$n's base stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.", 
		ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck);
	act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);

	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.", get_curr_str(ch), get_curr_wis(ch), get_curr_int(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch)); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	snprintf( buf, MAX_STRING_LENGTH, "$n's current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
             get_curr_str(ch), get_curr_wis(ch), get_curr_int(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch));
	act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
   return;
}
void do_stat											(CHAR_DATA *ch, const char *argument){
   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_VAMPIRE( ch ) )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.", ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   else
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.", ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.", get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ), get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_report											(CHAR_DATA *ch, const char *argument){
   char buf[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that in your current state of mind!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }


   if( IS_VAMPIRE( ch ) )
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit,
                 ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   if( IS_VAMPIRE( ch ) )
      snprintf( buf, MAX_INPUT_LENGTH, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                ch->hit, ch->max_hit,
                ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      snprintf( buf, MAX_INPUT_LENGTH, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
                ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   return;
}
void do_fprompt											(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }

   char prompt[128];
   mudstrlcpy(prompt, argument ? argument : "", sizeof(prompt));
   smash_tilde(prompt);
   one_argument(prompt, arg);
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current fighting prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", (!ch->pcdata->fprompt || !ch->pcdata->fprompt[0]) ? "(default prompt)" : ch->pcdata->fprompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }

   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", (!ch->pcdata->fprompt || !ch->pcdata->fprompt[0]) ? "(default prompt)" : ch->pcdata->fprompt );
   if( ch->pcdata->fprompt )
      STRFREE( ch->pcdata->fprompt );

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->fprompt = STRALLOC( "" );
   else if( !str_cmp( arg, "none" ) )
      ch->pcdata->fprompt = STRALLOC( ch->pcdata->prompt ? ch->pcdata->prompt : "" );
   else
      ch->pcdata->fprompt = STRALLOC( prompt );
   return;
}
void do_prompt											(CHAR_DATA *ch, const char *argument){
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }
   char prompt[128];
   mudstrlcpy(prompt, argument ? argument : "", sizeof(prompt));
   smash_tilde(prompt);
   one_argument(prompt, arg);
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", (!ch->pcdata->prompt || !ch->pcdata->prompt[0]) ? "(default prompt)" : ch->pcdata->prompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", (!ch->pcdata->prompt || !ch->pcdata->prompt[0]) ? "(default prompt)" : ch->pcdata->prompt );
   if( ch->pcdata->prompt )
      STRFREE( ch->pcdata->prompt );

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->prompt = STRALLOC( "" );
   else if( !str_cmp( arg, "fprompt" ) )
      ch->pcdata->prompt = STRALLOC( ch->pcdata->fprompt ? ch->pcdata->fprompt : "" );
   else
      ch->pcdata->prompt = STRALLOC( prompt );
   return;
}
void do_compass											(CHAR_DATA *ch, const char *argument){
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );

	if(arg[0] == '\0')
	{
		if( xIS_SET(ch->act, PLR_COMPASS))
		{
			xREMOVE_BIT(ch->act, PLR_COMPASS);
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Compass is now off."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		}
		else
		{
			xSET_BIT(ch->act, PLR_COMPASS);
			do_look(ch, "auto");
		}
	}
}
void do_favor											(CHAR_DATA *ch, const char *argument){
	//Command to show current favor
	char buf[MAX_STRING_LENGTH];

	if(IS_NPC(ch))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	set_char_color( AT_GREEN, ch );
	if(!ch->pcdata->deity)
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not follow a deity."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}
	else if(ch->pcdata->favor >  2250) mudstrlcpy(buf, "loved", 	 MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  2000) mudstrlcpy(buf, "cherished",  MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  1750) mudstrlcpy(buf, "honored", 	 MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  1500) mudstrlcpy(buf, "praised",    MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  1250) mudstrlcpy(buf, "favored",    MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  1000) mudstrlcpy(buf, "respected",  MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >   750) mudstrlcpy(buf, "liked",      MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >   250) mudstrlcpy(buf, "tolerated",  MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  -250) mudstrlcpy(buf, "ignored",    MAX_STRING_LENGTH);
	else if(ch->pcdata->favor >  -750) mudstrlcpy(buf, "shunned",    MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -1000) mudstrlcpy(buf, "disliked",   MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -1250) mudstrlcpy(buf, "dishonored", MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -1500) mudstrlcpy(buf, "disowned",   MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -1750) mudstrlcpy(buf, "abandoned",  MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -2000) mudstrlcpy(buf, "despised",   MAX_STRING_LENGTH);
	else if(ch->pcdata->favor > -2250) mudstrlcpy(buf, "hated",      MAX_STRING_LENGTH);
	else                               mudstrlcpy(buf, "damned",     MAX_STRING_LENGTH);

	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s considers you to be %s.", ch->pcdata->deity->name, buf); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	return;
}
