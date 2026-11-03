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
 *                            Battle & death module                         *
 ****************************************************************************/

#include <stdio.h>
#include "mud.h"
//--------------------------------------------------------------------------------------------------//
extern char lastplayercmd[MAX_INPUT_LENGTH];
//--------------------------------------------------------------------------------------------------//
// Used to figure out which weapon later
OBJ_DATA *used_weapon;  
static bool dreamwalk_shield_retaliation = FALSE;
static bool dreamwalk_reflect_magic_damage = FALSE;

static ch_ret DREAMWALK_SHIELD_RETALIATE		(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt){
	bool previous = dreamwalk_shield_retaliation;
	ch_ret retcode;

	dreamwalk_shield_retaliation = TRUE;
	retcode = damage(ch, victim, dam, dt);
	dreamwalk_shield_retaliation = previous;
	return retcode;
}
//==================================================================================================//
void new_dam_message							(CHAR_DATA *ch, CHAR_DATA *victim, int dam, unsigned int dt, OBJ_DATA *obj);
void group_gain									(CHAR_DATA *ch, CHAR_DATA *victim );
int xp_compute									(CHAR_DATA *gch, CHAR_DATA *victim);
int align_compute								(CHAR_DATA *gch, CHAR_DATA *victim);
ch_ret one_hit									(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool use_offhand);
int obj_hitroll									(OBJ_DATA *obj);
void show_condition								(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// NPC COMBAT INTELLIGENCE: choose useful abilities, never grant additional attacks.				//
//==================================================================================================//
static int NPC_ATTACK_WEIGHT					(CHAR_DATA *ch, CHAR_DATA *victim, int type){
	int weight = 12;
	int health = (victim->hit * 100) / UMAX(1, victim->max_hit);

	switch(type)
	{
		case ATCK_BASH:
		
		case ATCK_STUN:			
			if(victim->position <= POS_STUNNED) 											
			return 0; 															
			break;
			
		case ATCK_GOUGE:
		
		case ATCK_BLINDNESS:	
			if(IS_AFFECTED(victim, AFF_BLIND)) 												
			return 0; 	
			weight = (health > 35) ? 16 : 5; 						
			break;
			
		case ATCK_FEED:
			weight = (ch->hit * 100 < ch->max_hit * 60) ? 18 : 9;	
			break;
		
		case ATCK_DRAIN:		
			if(IS_SET(victim->immune, RIS_DRAIN) && !IS_SET(victim->no_immune, RIS_DRAIN)) 	
			return 0;															
			break;
			
		case ATCK_FIREBREATH:
		case ATCK_FLAMESTRIKE:
		
		case ATCK_FIREBALL:		
			if(IS_SET(victim->immune, RIS_FIRE) && !IS_SET(victim->no_immune, RIS_FIRE)) 
			return 0;
			weight = IS_SET(victim->resistant, RIS_FIRE) ? 5 : 12;
			
			if(IS_SET(victim->susceptible, RIS_FIRE)) weight += 6;
			break;
		case ATCK_FROSTBREATH:	
			if(IS_SET(victim->immune, RIS_COLD) && !IS_SET(victim->no_immune, RIS_COLD)) 
			return 0;
			weight = IS_SET(victim->resistant, RIS_COLD) ? 5 : 12;
			
			if(IS_SET(victim->susceptible, RIS_COLD)) weight += 6;
			break;
		case ATCK_ACIDBREATH:		
			if(IS_SET(victim->immune, RIS_ACID) && !IS_SET(victim->no_immune, RIS_ACID)) 
			return 0;
			weight = IS_SET(victim->resistant, RIS_ACID) ? 5 : 12;
							
			if(IS_SET(victim->susceptible, RIS_ACID)) weight += 6;
			break;
			
		case ATCK_LIGHTNBREATH:	
			if(IS_SET(victim->immune, RIS_ELECTRICITY) && !IS_SET(victim->no_immune, RIS_ELECTRICITY)) 
			return 0;
			weight = IS_SET(victim->resistant, RIS_ELECTRICITY) ? 5 : 12;
							
			if(IS_SET(victim->susceptible, RIS_ELECTRICITY)) weight += 6;
			break;
			
		case ATCK_GASBREATH:
		
		case ATCK_POISON:			
			if(IS_SET(victim->immune, RIS_POISON) && !IS_SET(victim->no_immune, RIS_POISON)) 
			return 0;
			
			if(type == ATCK_POISON && IS_AFFECTED(victim, AFF_POISON)) return 0;
			weight = (health > 45) ? 15 : 6;
			break;
			
		case ATCK_CURSE:
		
		case ATCK_WEAKEN:			
			weight = (health > 50) ? 15 : 5;
			break;
			
		case ATCK_SPIRALBLAST:
		case ATCK_CAUSESERIOUS:
		case ATCK_CAUSECRITICAL:
		case ATCK_EARTHQUAKE:
		case ATCK_HARM:
		case ATCK_COLORSPRAY:		
			break;
		default:
			return 0; //Unused attack flags never consume a decision.
	}

	return weight;
}
static int NPC_DEFENSE_WEIGHT					(CHAR_DATA *ch, CHAR_DATA *victim, int type){
	int health = (ch->hit * 100) / UMAX(1, ch->max_hit);

	switch(type)
	{
		case DFND_CURELIGHT:		return (health < 75) ? 10 : 0;
		case DFND_CURESERIOUS:		return (health < 60) ? 15 : 0;
		case DFND_CURECRITICAL:		return (health < 45) ? 20 : 0;
		case DFND_HEAL:				return (health < 35) ? 22 : 0;
		case DFND_DISPELMAGIC:		return victim->first_affect ? 12 : 0;
		case DFND_DISPELEVIL:		return IS_EVIL(victim) ? 10 : 0;
		case DFND_TELEPORT:			return (health < 20) ? 9 : 0;
		case DFND_SHOCKSHIELD:		return !IS_AFFECTED(ch, AFF_SHOCKSHIELD) ? 10 : 0;
		case DFND_VENOMSHIELD:		return !IS_AFFECTED(ch, AFF_VENOMSHIELD) ? 10 : 0;
		case DFND_ACIDMIST:			return !IS_AFFECTED(ch, AFF_ACIDMIST) ? 10 : 0;
		case DFND_FIRESHIELD:		return !IS_AFFECTED(ch, AFF_FIRESHIELD) ? 10 : 0;
		case DFND_ICESHIELD:		return !IS_AFFECTED(ch, AFF_ICESHIELD) ? 10 : 0;
		case DFND_TRUESIGHT:		return (!IS_AFFECTED(ch, AFF_TRUESIGHT) && !can_see(ch, victim)) ? 12 : 0;
		case DFND_SANCTUARY:		return !IS_AFFECTED(ch, AFF_SANCTUARY) ? 16 : 0;
		default:					return 0;
	}
}
static int NPC_CHOOSE_SPECIAL					(CHAR_DATA *ch, CHAR_DATA *victim, bool defense){
	int type, first, last, weight, total = 0, choice;

	first = defense ? 2 : 7;
	last = defense ? MAX_DEFENSE_TYPE : MAX_ATTACK_TYPE;

	for(type = first; type < last; type++)
	{
		if(defense && !xIS_SET(ch->defenses, type)) continue;
		if(!defense && !xIS_SET(ch->attacks, type)) continue;
		weight = defense ? NPC_DEFENSE_WEIGHT(ch, victim, type) : NPC_ATTACK_WEIGHT(ch, victim, type);
		total += weight;
	}

	if(total <= 0) return -1;
	choice = number_range(1, total);

	for(type = first; type < last; type++)
	{
		if(defense && !xIS_SET(ch->defenses, type)) continue;
		if(!defense && !xIS_SET(ch->attacks, type)) continue;
		weight = defense ? NPC_DEFENSE_WEIGHT(ch, victim, type) : NPC_ATTACK_WEIGHT(ch, victim, type);
		if((choice -= weight) <= 0 && weight > 0) return type;
	}
	return -1;
}
static void NPC_CHOOSE_STANCE					(CHAR_DATA *ch, CHAR_DATA *victim){
	//Humanoid veterans protect themselves when hurt; they take risks to finish a fight.
	//A defensive stance lowers outgoing damage; an aggressive one exposes the NPC.
	if(!IS_NPC(ch) || IS_CREATURE_MOB(ch) || ch->level < 10 || IS_AFFECTED(ch, AFF_BERSERK)) return;
	if(ch->max_hit <= 0 || victim->max_hit <= 0) return;

	if(ch->hit * 100 < ch->max_hit * 35)				ch->position = POS_DEFENSIVE;
	else if(victim->hit * 100 < victim->max_hit * 25 && ch->hit * 100 > ch->max_hit * 60)
														ch->position = POS_AGGRESSIVE;
	else												ch->position = POS_FIGHTING;
}
bool loot_coins_from_corpse						(CHAR_DATA *ch, OBJ_DATA *corpse){
	OBJ_DATA *content, *content_next;
	int oldgold = ch->gold;

	if(!corpse) 
	{bug("%s: NULL corpse!", __func__); return FALSE;}

	for(content = corpse->first_content; content; content = content_next)
	{
		content_next = content->next_content;

		if(content->item_type != ITEM_MONEY)										continue;
		if(!can_see_obj(ch, content))												continue;
		if(!CAN_WEAR(content, ITEM_TAKE) && ch->level < sysdata.level_getobjnotake)	continue;
		if(IS_OBJ_STAT(content, ITEM_PROTOTYPE) && !can_take_proto(ch))				continue;

		act(AT_ORANGE, "You get &Y$p&D from &Y$P&D", ch, content, corpse, TO_CHAR);
		act(AT_ORANGE, "$n gets &Y$p&D from &Y$P&D", ch, content, corpse, TO_ROOM);
		
		obj_from_obj(content); 
		check_for_trap(ch, content, TRAP_GET);
		if(char_died(ch)) return FALSE;

		oprog_get_trigger(ch, content);
		if(char_died(ch)) return FALSE;

		ch->gold += content->value[0] * content->count;
		extract_obj(content);
	}

	if(ch->gold - oldgold > 1 && ch->position > POS_SLEEPING)
	{
		char buf[MAX_INPUT_LENGTH];
		snprintf(buf, MAX_INPUT_LENGTH, "%d", ch->gold - oldgold);
		do_split(ch, buf);
	}
	return TRUE;
}
bool is_attack_supressed						(CHAR_DATA *ch){
	//Check to see if player's attacks are (still?) suppressed
	TIMER *timer;

	if(IS_NPC(ch))						return FALSE;
	if(IS_AFFECTED(ch, AFF_GRAPPLE))	return TRUE;

	timer = get_timerptr(ch, TIMER_ASUPRESSED);

	if(!timer) return FALSE;


	//Perma-supression -- bard? (can be reset at end of fight, or spell, etc) 
	if(timer->value == -1) return TRUE;

	//This is for timed supressions 
	if(timer->count >= 1) return TRUE;

	return FALSE;
}
bool is_wielding_poisoned						(CHAR_DATA *ch){
	//Check to see if weapon is poisoned.
	OBJ_DATA *obj;

	if(!used_weapon) return FALSE;
	if((obj = get_eq_char(ch, WEAR_WIELD))      != NULL && used_weapon == obj && IS_OBJ_STAT(obj, ITEM_POISONED)) return TRUE;
	if((obj = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL && used_weapon == obj && IS_OBJ_STAT(obj, ITEM_POISONED)) return TRUE;
	if((obj = get_eq_char(ch, WEAR_HOLD))       != NULL && used_weapon == obj && IS_OBJ_STAT(obj, ITEM_POISONED)) return TRUE;

	return FALSE;
}
bool is_hunting									(CHAR_DATA *ch, CHAR_DATA *victim){
	//Hunting, hating and fearing code.
	if(!ch->hunting || ch->hunting->who != victim) return FALSE;
	return TRUE;
}
bool is_hating									(CHAR_DATA *ch, CHAR_DATA *victim){
	if(!ch->hating || ch->hating->who != victim) return FALSE;
	return TRUE;
}
bool is_fearing									(CHAR_DATA *ch, CHAR_DATA *victim){
   if(!ch->fearing || ch->fearing->who != victim) return FALSE;
   return TRUE;
}
void stop_hunting								(CHAR_DATA *ch){
	if(ch->hunting)
	{
		STRFREE(ch->hunting->name);
		DISPOSE(ch->hunting);
		ch->hunting = NULL;
	}
	return;
}
void stop_hating								(CHAR_DATA *ch){
	if(ch->hating)
	{
		STRFREE(ch->hating->name);
		DISPOSE(ch->hating);
		ch->hating = NULL;
	}
	return;
}
void stop_fearing								(CHAR_DATA *ch){
	if(ch->fearing)
	{
		STRFREE(ch->fearing->name);
		DISPOSE(ch->fearing);
		ch->fearing = NULL;
	}
	return;
}
void start_hunting								(CHAR_DATA *ch, CHAR_DATA *victim){
	
	if(ch->hunting) {stop_hunting(ch);}

	CREATE(ch->hunting, HHF_DATA, 1);
	ch->hunting->name = QUICKLINK(victim->name);
	ch->hunting->who = victim;
	return;
}
void start_hating								(CHAR_DATA *ch, CHAR_DATA *victim){
	
	if(ch->hating) {stop_hating(ch);}
	
	CREATE(ch->hating, HHF_DATA, 1);
	ch->hating->name = QUICKLINK(victim->name);
	ch->hating->who = victim;
	return;
}
void start_fearing								(CHAR_DATA *ch, CHAR_DATA *victim){
	
	if(ch->fearing) {stop_fearing(ch);}

	CREATE(ch->fearing, HHF_DATA, 1);
	ch->fearing->name = QUICKLINK(victim->name);
	ch->fearing->who = victim;
	return;
}
void strip_grapple								(CHAR_DATA *ch){
	if(IS_NPC(ch)) return;
	
	if(IS_AFFECTED(ch, AFF_GRAPPLE))
	{
		affect_strip(ch, tes_action_grapple);
		set_char_color(AT_WEAROFF, ch);
		send_to_char(skill_table[tes_action_grapple]->msg_off, ch);
		send_to_char("\r\n", ch);
		
		if(get_timer(ch, TIMER_PKILLED) > 0) {remove_timer(ch, TIMER_ASUPRESSED);}
	}

	if(who_fighting(ch))
	{
		if(who_fighting(ch->fighting->who) == ch && IS_AFFECTED(who_fighting(ch), AFF_GRAPPLE))
		{
			affect_strip(who_fighting(ch), tes_action_grapple);
			set_char_color(AT_WEAROFF, who_fighting(ch));
			send_to_char(skill_table[tes_action_grapple]->msg_off, who_fighting(ch));
			send_to_char("\r\n", who_fighting(ch));
			
			if(get_timer(who_fighting(ch), TIMER_PKILLED) > 0) {remove_timer(who_fighting(ch), TIMER_ASUPRESSED);}
		}
	}
	return;
}
short VAMP_AC									(CHAR_DATA *ch){
	
	//Get the current armor class for a vampire based on time of day.
	if(IS_VAMPIRE(ch) && IS_OUTSIDE(ch))
	{
		switch (time_info.sunlight)
		{
			case SUN_DARK: 	return -10;
			case SUN_RISE:	return 5;
			case SUN_LIGHT:	return 10;
			case SUN_SET:	return 2;
			default:		return 0;
		}
	}
	return 0;
}
void violence_update							(void){
	//Control the fights going on. Called periodically by update_handler.
	//Many hours spent fixing bugs in here by Thoric, as noted by residual debugging checks.  
	//If you never get any of these error messages again in your logs... then you can comment out some of the checks without worry.
	//This function also handles some non-violence updates.									
	CHAR_DATA *ch; CHAR_DATA *victim; CHAR_DATA *rch;
	TRV_WORLD *lcw; TRV_DATA *lcr;
	AFFECT_DATA *paf, *paf_next;
	TIMER *timer, *timer_next;
	ch_ret retcode;
	int attacktype;
	SKILLTYPE *skill;
	static int pulse = 0;

	//The main combat pulse is called by update_handler; this counter gates periodic logic.
	pulse = (pulse + 1) %100;

	lcw = trworld_create(TR_CHAR_WORLD_BACK);
	
	for(ch = last_char; ch; ch = trvch_wnext(lcw))
	{
		set_cur_char( ch );

		//See if we got a pointer to someone who recently died... if so, either the pointer is bad... or it's a player who "died", and is back at the healer...
		//Since he/she's in the char_list, it's likely to be the later... and should not already be in another fight already
		if(char_died(ch)) continue;

		//Experience gained during battle deceases as battle drags on
		if(ch->fighting && (++ch->fighting->duration %24) == 0) {ch->fighting->xp = ((ch->fighting->xp *9) /10);}

		//CHARACTER TIMERS: countdown and abort delayed commands when combat begins.
		for(timer = ch->first_timer; timer; timer = timer_next)
		{
			timer_next = timer->next;

			if(ch->fighting && timer->type == TIMER_DO_FUN)
			{
				int tempsub;
				tempsub = ch->substate;
				ch->substate = SUB_TIMER_DO_ABORT;
				(timer->do_fun) (ch,"");
				if(char_died(ch)) break;
				ch->substate = tempsub;
				extract_timer(ch, timer); continue;
			}

			if(--timer->count <= 0)
			{
				if(timer->type == TIMER_ASUPRESSED)
				{
					if(timer->value == -1) {timer->count = 1000; continue;}
				}

				if(timer->type == TIMER_NUISANCE) DISPOSE(ch->pcdata->nuisance);

				if(timer->type == TIMER_DO_FUN)
				{
					int tempsub;
					tempsub = ch->substate;
					ch->substate = timer->value;
					(timer->do_fun) (ch, "");
					if(char_died(ch)) break;
					
					ch->substate = tempsub;
					if(timer->count > 0) continue;
				}
				extract_timer( ch, timer );
			}
		}

		if(char_died(ch)) continue;

		//We need spells that have shorter durations than an hour. So a melee round sounds good to me...
		//AFFECT DURATIONS: wear off timed buffs/debuffs and despawn expired summons.
		for(paf = ch->first_affect; paf; paf = paf_next)
		{
			paf_next = paf->next;
			
			if(paf->duration > 0) paf->duration--;
			else if(paf->duration < 0);
			else
			{
				if(IS_NPC(ch) && (paf->type == tes_effect_summon_daedra || paf->type == tes_effect_summon_beast || paf->type == tes_effect_summon_automaton || paf->type == tes_effect_summon_atronach || paf->type == tes_effect_summon_undead))
				{
					act(AT_MAGIC, "$n fades away.", ch, NULL, NULL, TO_ROOM);
					extract_char(ch, TRUE);
					break;
				}

				if(!paf_next || paf_next->type != paf->type || paf_next->duration > 0)
				{
					skill = get_skilltype(paf->type);
					if(paf->type > 0 && skill && skill->msg_off)
					{
						set_char_color(AT_WEAROFF, ch);
						send_to_char(skill->msg_off, ch);
						send_to_char("\r\n", ch);
					}
				}
				
				if(paf->type == gsn_possess)
				{
					ch->desc->character = ch->desc->original;
					ch->desc->original = NULL;
					ch->desc->character->desc = ch->desc;
					ch->desc->character->switched = NULL;
					ch->desc = NULL;
				}
				affect_remove(ch, paf);
			}
		}

		if(char_died(ch)) continue;

		//ROOM FORCES: pull/teleport triggers may move or kill actors; validate after calling.
		//Check for exits moving players around 
		if((retcode = pullcheck(ch, pulse)) == rCHAR_DIED || char_died(ch)) continue;

		//MELEE ROUND: choose stance, call multi_hit, then process NPC special attacks.
		//Let the battle begin! 
		if((victim = who_fighting(ch)) == NULL || IS_AFFECTED(ch, AFF_PARALYSIS)) continue;
		retcode = rNONE;
	  
		if(xIS_SET(ch->in_room->room_flags, ROOM_SAFE)) 			{log_printf("violence_update: %s fighting %s in a SAFE room.", ch->name, victim->name); stop_fighting(ch, TRUE);}
		else if(IS_AWAKE(ch) && ch->in_room == victim->in_room)
		{
			NPC_CHOOSE_STANCE(ch, victim);
			retcode = multi_hit(ch, victim, TYPE_UNDEFINED);
		}
		else													 	{stop_fighting(ch, FALSE);}

		if(char_died(ch))													continue;
		if(retcode == rCHAR_DIED || (victim = who_fighting(ch)) == NULL) 	continue;

		//Mob triggers (Added some victim death checks, because it IS possible)
		rprog_rfight_trigger(ch); 			if(char_died(ch) || char_died(victim)) continue;
		mprog_hitprcnt_trigger(ch, victim); if(char_died(ch) || char_died(victim)) continue;
		mprog_fight_trigger(ch, victim);	if(char_died(ch) || char_died(victim)) continue;

		//NPC SPECIAL ATTACKS/DEFENSES: weighted choices, no extra free weapon swing.
		if(IS_NPC(ch))
		{
			//NPC special attack flags
			if(!xIS_EMPTY(ch->attacks))
			{
				attacktype = -1;
				//Reduced frequency balances the better decisions: no bonus attack is granted.
				if(number_percent() <= UMIN(35, 15 + ch->level / 6))
				{
					attacktype = NPC_CHOOSE_SPECIAL(ch, victim, FALSE);
					switch (attacktype)
					{	
						case ATCK_BASH: 			AUTOMATIC_ACTION_BASH(ch, victim); 	retcode = global_retcode; 																	break;
						case ATCK_STUN:				AUTOMATIC_ACTION_STUN(ch, victim); 	retcode = global_retcode; 																	break;
						case ATCK_GOUGE:			AUTOMATIC_ACTION_GOUGE(ch,victim); 	retcode = global_retcode; 																	break;
						case ATCK_FEED:				do_feed(ch, "");					retcode = global_retcode; 																	break;
						case ATCK_DRAIN:												retcode = spell_energy_drain(skill_lookup("energy drain")		 , ch->level, ch, victim); 	break;
						case ATCK_FIREBREATH:											retcode = spell_fire_breath(skill_lookup("fire breath")			 , ch->level, ch, victim);	break;
						case ATCK_FROSTBREATH:											retcode = spell_frost_breath(skill_lookup("frost breath")		 , ch->level, ch, victim);	break;
						case ATCK_ACIDBREATH:											retcode = spell_acid_breath(skill_lookup("acid breath")			 , ch->level, ch, victim);	break;
						case ATCK_LIGHTNBREATH:											retcode = spell_lightning_breath(skill_lookup("lightning breath"), ch->level, ch, victim);	break;
						case ATCK_GASBREATH:											retcode = spell_gas_breath(skill_lookup("gas breath")		 	 , ch->level, ch, victim);	break;
						case ATCK_SPIRALBLAST:											retcode = spell_spiral_blast(skill_lookup("spiral blast")    	 , ch->level, ch, victim);	break;
						case ATCK_POISON:												retcode = spell_poison(gsn_poison, ch->level, ch, victim);									break;
						case ATCK_NASTYPOISON:																																		break;
						case ATCK_GAZE:																																				break;
						case ATCK_BLINDNESS:											retcode = spell_blindness(gsn_blindness						 	 , ch->level, ch, victim);	break;
						case ATCK_CAUSESERIOUS:											retcode = spell_cause_serious(skill_lookup("cause serious")  	 , ch->level, ch, victim);	break;
						case ATCK_EARTHQUAKE:											retcode = spell_earthquake(skill_lookup("earthquake")		 	 , ch->level, ch, victim);	break;
						case ATCK_CAUSECRITICAL:										retcode = spell_cause_critical(skill_lookup("cause critical")	 , ch->level, ch, victim);	break;
						case ATCK_CURSE:												retcode = spell_curse(skill_lookup("curse"), ch->level		 	 , ch, victim);				break;
						case ATCK_FLAMESTRIKE:											retcode = spell_flamestrike(skill_lookup("flamestrike")		 	 , ch->level, ch, victim);	break;
						case ATCK_HARM:													retcode = spell_harm(skill_lookup("harm")					 	 , ch->level, ch, victim);	break;
						case ATCK_FIREBALL:												retcode = spell_fireball(skill_lookup("fireball")			 	 , ch->level, ch, victim);	break;
						case ATCK_COLORSPRAY:											retcode = spell_colour_spray(skill_lookup("colour spray")	 	 , ch->level, ch, victim);	break;
						case ATCK_WEAKEN:												retcode = spell_weaken(skill_lookup("weaken")				 	 , ch->level, ch, victim);	break;
					}
				
					if(retcode == rCHAR_DIED || retcode == rVICT_DIED || char_died(ch) || char_died(victim) || who_fighting(ch) != victim)
					continue;
				}
			}

			// NPC special defense flags
			if(!xIS_EMPTY(ch->defenses))
			{	
				if(char_died(ch) || char_died(victim))
				continue;

				attacktype = -1;
				if(number_percent() <= UMIN(50, 30 + ch->level / 6))
				{
					attacktype = NPC_CHOOSE_SPECIAL(ch, victim, TRUE);
					switch (attacktype)
					{
						case DFND_CURELIGHT:
							if(ch->hit < ch->max_hit)
							{	
								act(AT_BLUE, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM); 
								retcode = spell_smaug( skill_lookup("cure light"), ch->level, ch, ch);
							} break;

						case DFND_CURESERIOUS:
							if(ch->hit < ch->max_hit)
							{
								act(AT_BLUE, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup("cure serious"), ch->level, ch, ch);
							} break;

						case DFND_CURECRITICAL:
							if(ch->hit < ch->max_hit)
							{
								act(AT_BLUE, "$n mutters a few incantations...and looks healthier.", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup("cure critical"), ch->level, ch, ch);
							} break;

						case DFND_HEAL:
							if(ch->hit < ch->max_hit)
							{
								act(AT_BLUE, "$n mutters a few incantations...and looks much healthier.", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup( "heal" ), ch->level, ch, ch);
							} break;

						case DFND_DISPELMAGIC:
							if(victim->first_affect)
							{
								act(AT_BLUE, "$n utters an incantation...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_dispel_magic( skill_lookup("dispel magic"), ch->level, ch, victim);
							} break;

						case DFND_DISPELEVIL:
							act( AT_BLUE, "$n utters an incantation...", ch, NULL, NULL, TO_ROOM );
							retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
							break;

						case DFND_TELEPORT:
							retcode = spell_teleport(skill_lookup("teleport"), ch->level, ch, ch);
							break;

						case DFND_SHOCKSHIELD:
							if(!IS_AFFECTED(ch, AFF_SHOCKSHIELD))
							{
								act(AT_BLUE, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup("shockshield"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_VENOMSHIELD:
							if(!IS_AFFECTED(ch, AFF_VENOMSHIELD))
							{
								act(AT_BLUE, "$n utters a few incantations ...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup("venomshield"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_ACIDMIST:
							if(!IS_AFFECTED(ch, AFF_ACIDMIST))
							{
								act(AT_BLUE, "$n utters a few incantations ...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup( "acidmist"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_FIRESHIELD:
							if(!IS_AFFECTED(ch, AFF_FIRESHIELD))
							{
								act(AT_BLUE, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup( "fireshield"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_ICESHIELD:
							if(!IS_AFFECTED(ch, AFF_ICESHIELD))
							{
								act(AT_BLUE, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug(skill_lookup("iceshield"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_TRUESIGHT:
							if(!IS_AFFECTED(ch, AFF_TRUESIGHT))
							{								
								retcode = spell_smaug(skill_lookup("true"), ch->level, ch, ch);
							}
							else retcode = rNONE;
							break;

						case DFND_SANCTUARY:
							if(!IS_AFFECTED(ch, AFF_SANCTUARY))
							{
								act(AT_BLUE, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
								retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
							}
							else retcode = rNONE;
							break;
					}
					if(retcode == rCHAR_DIED || retcode == rVICT_DIED || char_died(ch) || char_died(victim) || who_fighting(ch) != victim)
					continue;
				}
			}
		}

		//Fun for the whole family!
		//GROUP ASSIST: call multi_hit on qualifying allies and nearby cooperating mobs.
		lcr = trvch_create(ch, TR_CHAR_ROOM_FORW);
		
		for(rch = ch->in_room->first_person; rch; rch = trvch_next(lcr))
		{
			//Group Fighting Styles Support: If ch is tanking. If rch is using a more aggressive style than ch. Then rch is the new tank
			if((!IS_NPC(ch) && !IS_NPC(rch)) && (rch != ch) && (rch->fighting) && (who_fighting(rch->fighting->who) == ch) && (!xIS_SET(rch->fighting->who->act, ACT_AUTONOMOUS)) && (rch->style < ch->style))
			{
				stop_fighting(rch->fighting->who, FALSE);
				set_fighting(rch->fighting->who, rch); 
			}

			if(IS_AWAKE(rch) && !rch->fighting)
			{          
				//PC's auto-assist others in their group. */
				if(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
				{
					if(((!IS_NPC(rch) && rch->desc) || IS_AFFECTED(rch, AFF_CHARM)) && is_same_group(ch, rch) && !is_safe(rch, victim, TRUE))
					{
						multi_hit(rch, victim, TYPE_UNDEFINED); 
						continue;
					}
				}

				//NPC's auto-assist PCs in their group.
				if(!IS_NPC(ch) && IS_NPC(rch) && is_same_group(ch, rch) && !is_safe(rch, victim, TRUE))
				{
					multi_hit(rch, victim, TYPE_UNDEFINED);
					continue;
				}

				//NPC's assist NPC's of same type or 12.5% chance regardless. */
				if(IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM) && !xIS_SET(rch->act, ACT_NOASSIST) && !xIS_SET(rch->act, ACT_PET))
				{
					if(char_died(ch)) break;
					if(rch->pIndexData == ch->pIndexData || number_bits(3) == 0)
					{
						CHAR_DATA *vch; CHAR_DATA *target; int number;
						target = NULL; number = 0;
						
						for(vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
						{
							if(can_see(rch, vch) && is_same_group(vch, victim) && number_range(0,number) == 0)
							{
								if(vch->mount && vch->mount == rch) {target = NULL;}
								else 								{target = vch; number++;}
							}
						}		
						if(target) {multi_hit(rch, target, TYPE_UNDEFINED);}
					}
					}
			}
		}
		trv_dispose(&lcr);
	}
	trworld_dispose( &lcw );
	return;
}
ch_ret multi_hit								(CHAR_DATA *ch, CHAR_DATA *victim, int dt){
	int schance;
	OBJ_DATA *mainhand, *offhand;
	bool two_weapons, was_fighting;
	ch_ret retcode;

	//PK TIMER: recent PvP hostility prevents instant logout/other exploits.
	//Add timer to pkillers
	if(!IS_NPC(ch) && !IS_NPC(victim))
	{
		if(xIS_SET(ch->act, PLR_NICE)) return rNONE;

		add_timer(ch, TIMER_RECENTFIGHT, 11, NULL, 0);
		add_timer(victim, TIMER_RECENTFIGHT, 11, NULL, 0);
	}

	if(is_attack_supressed(ch)) 																	return rNONE;
	if(IS_NPC(ch) && xIS_SET(ch->act, ACT_NOATTACK)) 												return rNONE;

	//ONE AUTOMATIC DECISION PER NORMAL ROUND: spells/skills do not consume a hand attack.
	//Automatic actions resolve once per normal round and no longer consume a weapon swing.
	was_fighting = who_fighting(ch) == victim;
	if(dt == TYPE_UNDEFINED)
	{
		used_weapon = NULL;
		COMBAT_AUTOMATIC_ACTION(ch, victim);
		if(char_died(ch)) return rCHAR_DIED;
		if(char_died(victim)) return rVICT_DIED;
		if(ch->in_room != victim->in_room || victim->position == POS_DEAD) return rNONE;
		if(was_fighting && who_fighting(ch) != victim) return rNONE;
	}

	//WEAPON SELECTION: offhand must be a real weapon; two-handed weapons block dual wielding.
	//A normal round strikes with the mainhand, then the offhand if another weapon is equipped.
	mainhand = get_eq_char(ch, WEAR_WIELD);
	if(mainhand && mainhand->item_type != ITEM_WEAPON) mainhand = NULL;

	offhand = get_eq_char(ch, WEAR_DUAL_WIELD);
	if(!offhand || offhand->item_type != ITEM_WEAPON)
		offhand = get_eq_char(ch, WEAR_HOLD);
	if(offhand && offhand->item_type != ITEM_WEAPON) offhand = NULL;

	two_weapons = mainhand && offhand && mainhand != offhand && !IS_TWO_HANDED_WEAPON(ch);

	//1. Mainhand attack (or the normal bare-hand attack if no weapon is equipped).
	if((retcode = one_hit(ch, victim, dt, FALSE)) != rNONE) return retcode;
	if(char_died(ch)) return rCHAR_DIED;
	if(char_died(victim)) return rVICT_DIED;
	if(who_fighting(ch) != victim || ch->in_room != victim->in_room || dt == tes_skill_backstab_creature || dt == gsn_circle || dt == tes_action_pounce) return rNONE;

	//2. Offhand attack; WEAR_HOLD counts only if it contains an actual weapon.
	if(two_weapons)
	{
		if((retcode = one_hit(ch, victim, dt, TRUE)) != rNONE) return retcode;
		if(char_died(ch)) return rCHAR_DIED;
		if(char_died(victim)) return rVICT_DIED;
		if(who_fighting(ch) != victim || ch->in_room != victim->in_room) return rNONE;
	}

	COMBAT_MOVEMENT(ch, victim);

	//EXTRA SWING CAP: only one chance after the normal hands have attacked.
	//ATTACK OF OPPORTUNITY: LEVEL-BASED, MAXIMUM ONE BONUS ATTACK PER MULTI_HIT.
	//Level 1: 2%, Level 20: 9%, Level 40: 16%, Level 65: 25%.
	schance = URANGE(2, 2 + (ch->level * 23 / UMAX(1, MAX_LEVEL)), 25);

	if(number_percent() <= schance)
	{
		act(AT_HIT,    "&GYou spot an opening and make an attack of opportunity against $N!&D", ch, NULL, victim, TO_CHAR);
		act(AT_HITME,  "&R$n seizes an opening and attacks you again!&D", ch, NULL, victim, TO_VICT);
		act(AT_ACTION, "&Y$n seizes an opening and strikes at $N again!&D", ch, NULL, victim, TO_NOTVICT);

		//3. One conditional opportunity strike, using the mainhand.
		retcode = one_hit(ch, victim, dt, FALSE);
		if(retcode != rNONE || who_fighting(ch) != victim) return retcode;
	}

	//FATIGUE: charge movement cost once after resolving the round.
	//Preserve the original combat-round fatigue/encumbrance charge.
	if(retcode == rNONE)
	{
		int move;
		if(!IS_AFFECTED(ch, AFF_FLYING) && !IS_AFFECTED(ch, AFF_FLOATING)) 	{move = encumbrance(ch, movement_loss[URANGE(0, ch->in_room->sector_type, SECT_MAX - 1)]);}
		else																{move = encumbrance(ch, 1);}

		if(ch->move) {ch->move = UMAX(0, ch->move - move);}
	}
	return retcode;
}
int weapon_prof_bonus_check						(CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr){
	int bonus = 0;

	if(!gsn_ptr) return 0;
	*gsn_ptr = -1;

	//Only a player with an actual weapon can gain a weapon proficiency bonus.
	//IMPORTANT: WEAPON value[4] is the TES weapon subtype (1..41). value[5] is RARITY.
	//value[3] remains the weapon damage type; never choose a proficiency from rarity.
	//Do not substitute a family skill: each item has its own independently trained skill.
	if(ch && !IS_NPC(ch) && ch->pcdata && wield && (wield->item_type == ITEM_WEAPON || wield->item_type == ITEM_MISSILE_WEAPON))
	{
		switch(wield->value[4])
		{
			//THROWING: Dart / Star / Knife
			case  1: *gsn_ptr = tes_passive_weapon_dart;             break; //DART
			case  2: *gsn_ptr = tes_passive_weapon_star;             break; //STAR
			case  3: *gsn_ptr = tes_passive_weapon_knife;            break; //KNIFE
			//HAND-TO-HAND WEAPONS
			case  4: *gsn_ptr = tes_passive_weapon_claw;             break; //CLAW
			case  5: *gsn_ptr = tes_passive_weapon_katar;            break; //KATAR
			case  6: *gsn_ptr = tes_passive_weapon_knuckle;          break; //KNUCKLE
			case  7: *gsn_ptr = tes_passive_weapon_cestus;           break; //CESTUS
			//SMALL DAGGERS
			case  8: *gsn_ptr = tes_passive_weapon_dirk;             break; //DIRK
			case  9: *gsn_ptr = tes_passive_weapon_dagger;           break; //DAGGER
			//SHORT BLADES
			case 10: *gsn_ptr = tes_passive_weapon_tanto;            break; //TANTO
			case 11: *gsn_ptr = tes_passive_weapon_wakizashi;        break; //WAKIZASHI
			case 12: *gsn_ptr = tes_passive_weapon_shortsword;       break; //SHORTSWORD
			case 13: *gsn_ptr = tes_passive_weapon_rapier;           break; //RAPIER
			//LONG BLADES
			case 14: *gsn_ptr = tes_passive_weapon_cutlass;          break; //CUTLASS
			case 15: *gsn_ptr = tes_passive_weapon_saber;            break; //SABER
			case 16: *gsn_ptr = tes_passive_weapon_falchion;         break; //FALCHION
			case 17: *gsn_ptr = tes_passive_weapon_katana;           break; //KATANA
			case 18: *gsn_ptr = tes_passive_weapon_longsword;        break; //LONGSWORD
			case 19: *gsn_ptr = tes_passive_weapon_broadsword;       break; //BROADSWORD
			case 20: *gsn_ptr = tes_passive_weapon_claymore;         break; //CLAYMORE
			case 21: *gsn_ptr = tes_passive_weapon_daikatana;        break; //DAIKATANA
			//AXES
			case 22: *gsn_ptr = tes_passive_weapon_sickle;           break; //SICKLE
			case 23: *gsn_ptr = tes_passive_weapon_handaxe;          break; //HANDAXE
			case 24: *gsn_ptr = tes_passive_weapon_waraxe;           break; //WARAXE
			case 25: *gsn_ptr = tes_passive_weapon_pickaxe;          break; //PICKAXE
			case 26: *gsn_ptr = tes_passive_weapon_battleaxe;        break; //BATTLEAXE
			//BLUNT WEAPONS AND STAFF
			case 27: *gsn_ptr = tes_passive_weapon_club;             break; //CLUB
			case 28: *gsn_ptr = tes_passive_weapon_mace;             break; //MACE
			case 29: *gsn_ptr = tes_passive_weapon_flail;            break; //FLAIL
			case 30: *gsn_ptr = tes_passive_weapon_morningstar;      break; //MORNINGSTAR
			case 31: *gsn_ptr = tes_passive_weapon_staff;            break; //STAFF
			case 32: *gsn_ptr = tes_passive_weapon_warhammer;        break; //WARHAMMER
			case 33: *gsn_ptr = tes_passive_weapon_maul;             break; //MAUL
			//SPEARS AND POLEARMS
			case 34: *gsn_ptr = tes_passive_weapon_spear;            break; //SPEAR
			case 35: *gsn_ptr = tes_passive_weapon_glaive;           break; //GLAIVE
			case 36: *gsn_ptr = tes_passive_weapon_halberd;          break; //HALBERD
			case 37: *gsn_ptr = tes_passive_weapon_lance;            break; //LANCE
			case 38: *gsn_ptr = tes_passive_weapon_pike;             break; //PIKE
			//BOWS AND CROSSBOWS
			case 39: *gsn_ptr = tes_passive_weapon_shortbow;         break; //SHORTBOW
			case 40: *gsn_ptr = tes_passive_weapon_longbow;          break; //LONGBOW
			case 41: *gsn_ptr = tes_passive_weapon_crossbow;         break; //CROSSBOW

			//Unknown weapon subtype: no bonus and no proficiency training.
			default: *gsn_ptr = -1; break;
		}

		//Only registered skills may be read or passed to learn_from_success/failure.
		//CHECK YOUR db.c/skills.dat: all 41 symbols must be registered separately.
		if(*gsn_ptr < 0 || *gsn_ptr >= num_skills || !IS_VALID_SN(*gsn_ptr)) *gsn_ptr = -1;

		//0% = not learned: no bonus; learning from 0 depends on your trainer system.
		//Accuracy increases +1 for every complete 10% of the matching skill, max +10.
		//Example: Dagger 80% = +8, Dirk 0% = +0, even when both are short blades.
		if(*gsn_ptr != -1 && LEARNED(ch, *gsn_ptr) > 0)
			bonus = URANGE(0, LEARNED(ch, *gsn_ptr) / 10, 10);
	}
	return bonus;
}
int obj_hitroll									(OBJ_DATA *obj){		
	int tohit = 0;
	AFFECT_DATA *paf;

	//Some callers have no weapon; never dereference a missing object/prototype.
	if(!obj || !obj->pIndexData) return 0;

	for(paf = obj->pIndexData->first_affect; paf; paf = paf->next)
		if(paf->location == APPLY_HITROLL) {tohit += paf->modifier;}
	
	for(paf = obj->first_affect; paf; paf = paf->next)
		if(paf->location == APPLY_HITROLL) {tohit += paf->modifier;}
	
	return tohit;
}
short off_shld_lvl								(CHAR_DATA *ch, CHAR_DATA *victim){
	
	//Offensive shield level modifier
	short lvl;

	if(!IS_NPC(ch))  /*Players get much less effect*/
	{
		lvl = UMAX(1, (ch->level - 10) /2);	
		
		if(number_percent() + (victim->level - lvl) <40)
		{
			if(CAN_PKILL(ch) && CAN_PKILL(victim)) 	{return ch->level;}
			else 									{return lvl;}
		}
		else return 0;
	}
	else
	{
		lvl = ch->level /2;
		
		if(number_percent() + (victim->level - lvl) <70) {return lvl;}
		else {return 0;}
	}
}
ch_ret one_hit									(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool use_offhand){
	OBJ_DATA *wield, *mainhand, *offhand;
	int victim_ac, thac0, thac0_00, thac0_32, plusris, dam, diceroll, attacktype, cnt, prof_bonus, prof_gsn = -1;
	ch_ret retcode = rNONE;

	if(victim->position == POS_DEAD || ch->in_room != victim->in_room) {return rVICT_DIED;}
	
	//WEAPON CHOICE: used_weapon is also read by poison and damage-message handlers.
	//WEAR_HOLD is an offhand only when the held item is a real weapon.
	//Use the other available weapon if the requested hand is empty.
	mainhand = get_eq_char(ch, WEAR_WIELD);
	if(mainhand && mainhand->item_type != ITEM_WEAPON) mainhand = NULL;

	offhand = get_eq_char(ch, WEAR_DUAL_WIELD);
	if(!offhand || offhand->item_type != ITEM_WEAPON)
		offhand = get_eq_char(ch, WEAR_HOLD);
	if(offhand && offhand->item_type != ITEM_WEAPON) offhand = NULL;

	wield = use_offhand ? (offhand ? offhand : mainhand) : (mainhand ? mainhand : offhand);
	used_weapon = wield;

	if(wield) 	{prof_bonus= weapon_prof_bonus_check(ch, wield, &prof_gsn);}
	else		{prof_bonus = 0;}

	//NPC NATURAL ATTACKS: chance to substitute a bite/claw/tail/etc. for a weapon strike.
	if(ch->fighting  && dt == TYPE_UNDEFINED && IS_NPC(ch) && !xIS_EMPTY(ch->attacks))
    {
		cnt = 0;

		for(;;)
        {
			attacktype = number_range( 0, 6 );
			if(xIS_SET(ch->attacks, attacktype)) {				   break;}
			if(cnt++ > 16) 						 {attacktype = -1; break;}
        }

		if(attacktype == ATCK_BACKSTAB) 	{attacktype = -1;}
		if(wield && number_percent() >25) 	{attacktype = -1;}
		if(!wield && number_percent() >50)	{attacktype = -1;}

		switch ( attacktype )
		{
			default: 																			break;
			case ATCK_BITE	: do_bite(ch,""); 						retcode = global_retcode; 	break;
			case ATCK_CLAWS	: do_claw(ch,""); 						retcode = global_retcode; 	break;
			case ATCK_TAIL	: do_tail(ch,""); 						retcode = global_retcode;	break;
			case ATCK_STING	: do_sting(ch,"");						retcode = global_retcode;	break;
			case ATCK_PUNCH	: AUTOMATIC_ACTION_PUNCH(ch, victim);	retcode = global_retcode;	break;
			case ATCK_KICK	: AUTOMATIC_ACTION_KICK(ch, victim);	retcode = global_retcode;	break;
			case ATCK_TRIP	: attacktype = 0;													break;
		}
		
		if(attacktype >= 0) return retcode;
	}

	//DAMAGE TYPE: weapon->value[3] is damage style, NOT the weapon proficiency category.
	if(dt == TYPE_UNDEFINED)
    {
		dt = TYPE_HIT;
		if(wield && wield->item_type == ITEM_WEAPON) {dt += wield->value[3];}
    }

	//THAC0 SOURCES: monster base or player class progression.
	if(IS_NPC(ch)) 
	{
		thac0_00 = ch->mobthac0; 
		thac0_32 = 0;
	}
	else
    {
		thac0_00 = class_table[ch->Class]->thac0_00;
		thac0_32 = class_table[ch->Class]->thac0_32;
    }

	//Lower THAC0 is better: use the skill belonging to THIS hand's weapon.
	//Do not modify ch->hitroll permanently (switching weapons must switch the bonus).
	thac0 = interpolate(ch->level, thac0_00, thac0_32) - GET_HITROLL(ch) - prof_bonus;
	victim_ac = (GET_AC(victim) /10);

	if(wield && !can_see_obj(victim, wield)) 	{victim_ac += 1;}
	if(!can_see(ch, victim))					{victim_ac -= 4;}

	//THAC0: lower armor class is harder to hit. Each stance has a tradeoff.
	if(victim->position == POS_DEFENSIVE) 	{victim_ac -= 2;}
	if(victim->position == POS_EVASIVE) 	{victim_ac -= 4;}
	if(victim->position == POS_AGGRESSIVE)	{victim_ac += 2;}
	if(victim->position == POS_BERSERK) 	{victim_ac += 4;}

	if(ch->fighting && ch->fighting->who == victim)
    {
		short times = ch->fighting->timeskilled;

		if(times)
		{
			short intdiff = get_curr_int(ch) - get_curr_int(victim);
			
			if(intdiff != 0) 
			{victim_ac += (intdiff * times) /10;}
		}
    }

	diceroll = number_range(1,20);

	int attack_chance;
	attack_chance = thac0 - diceroll; 

	//Natural 1 always misses; natural 20 always hits (and may crit below).
	//On a miss, practice the correct weapon skill without granting damage.
	if(diceroll == 1 || (diceroll != 20 && attack_chance > victim_ac))
    {
		if(IS_VALID_SN(prof_gsn)) learn_from_failure(ch, prof_gsn);
		retcode = damage(ch, victim, 0, dt);
		if(retcode != rNONE) return retcode;
		if(char_died(ch)) return rCHAR_DIED;
		if(char_died(victim)) return rVICT_DIED;
		tail_chain();
		return rNONE;
    }

	//DAMAGE ROLL: actual weapon min/max values or unarmed dice, then stance modifiers.
	int dice;
	if(!wield) 	{dice = number_range(ch->barenumdie, ch->baresizedie * ch->barenumdie) + ch->damplus;}
	else		{dice = number_range(wield->value[1], wield->value[2]);}
  
	dam = dice;
	dam = dam + GET_DAMROLL(ch);

	//Small legacy damage modifier: floating-point division must not truncate early.
	//At +10 hitroll this provides +1 damage; accuracy remains the primary benefit.
	float bonus = (float)prof_bonus / 10.0f;
	if(prof_bonus) {dam = dam + bonus;}

	float pos_atk = 1.00;

	if(ch->position == POS_BERSERK) 	{pos_atk = 1.60;}
	if(ch->position == POS_AGGRESSIVE)	{pos_atk = 1.20;}
	if(ch->position == POS_FIGHTING)	{pos_atk = 1.00;}
	if(ch->position == POS_DEFENSIVE)	{pos_atk = 0.80;}
	if(ch->position == POS_EVASIVE)		{pos_atk = 0.40;}

	dam = dam * pos_atk;

/*
   char buf1[256], buf2[256], buf3[256];

   if(attack_chance < victim_ac)
     {
     dam = 0;
     tail_chain();
     return rNONE;

     snprintf( buf1, 256, "&Y$n fails to attack.&D");
     snprintf( buf2, 256, "&GYou fail to attack.&D");
     snprintf( buf3, 256, "&Y$n fails to attack.&D");

     act(AT_MAGIC, buf1, ch, NULL, victim, TO_CHAR);
     act(AT_MAGIC, buf2, ch, NULL, victim, TO_VICT);
     act(AT_MAGIC, buf3, ch, NULL, victim, TO_NOTVICT);
     }
*/

	//CRITICAL: natural-20 hit doubles damage and adds a narrative body location.
	if(diceroll == 20)
    {
		CRITICAL_LOCATION(ch, victim);
		dam = dam * 2;
    }


/*
   int total_dmg;

   if(attack_chance < victim_ac)
     {
     dam = 0;
     total_dmg = 0;
     tail_chain();
     return rNONE;
     }
   else
     {
     total_dmg = dice + GET_DAMROLL(ch);
     }
*/

	if(!IS_AWAKE(victim))					dam *= 2;
	if(dt == tes_skill_backstab_creature)	dam *= (2 + URANGE(2, ch->level - (victim->level / 4), 30) / 8);
	if(dt == tes_action_pounce)				dam *= (2 + URANGE(2, ch->level - (victim->level / 4), 30) / 11);
	if(dt == gsn_circle)					dam *= (2 + URANGE(2, ch->level - (victim->level / 4), 30) / 16);
	if(dam <= 0)							dam = 0;

	//RESISTANCES: ordinary/magical immunity and PLUS1..PLUS6 protections.
	plusris = 0;

	if(wield)
    {
		if(IS_OBJ_STAT(wield, ITEM_MAGIC))	dam = ris_damage(victim, dam, RIS_MAGIC);
		else								dam = ris_damage(victim, dam, RIS_NONMAGIC);

		plusris = obj_hitroll(wield);
    }
	else dam = ris_damage(victim, dam, RIS_NONMAGIC);

	if(dam)
	{
		int x, res, imm, sus, mod;
		//PLUS1 starts at RIS_PLUS1; shift by (bonus - 1), not bonus.
		//Clamp above PLUS6 to avoid an unsupported resistance bit.
		if(plusris > 0)	plusris = RIS_PLUS1 << URANGE(0, plusris - 1, 5);

		//Initialize values to handle a zero plusris 
		imm = res = -1;
		sus = 1;

		//Find high ris 
		for(x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1)
		{
			if(IS_SET(victim->immune, x)) 		imm = x;
			if(IS_SET(victim->resistant, x))	res = x;
			if(IS_SET(victim->susceptible, x))	sus = x;
		}
		
		mod = 10;
		if(imm >= plusris)	mod -= 10;
		if(res >= plusris)	mod -= 2;
		if(sus <= plusris)	mod += 2;

		//Check if immune 
		if(mod <= 0)		dam = -1;
		else if(mod != 10) dam = (dam * mod) /10;
	}
 
	//TRAINING: successful damaging strikes can improve THIS EXACT weapon proficiency.
	if(IS_VALID_SN(prof_gsn))
	{
		if(dam > 0) {learn_from_success(ch, prof_gsn);}
	}

	if(dam == -1)
    {
		if(dt >= 0 && dt < num_skills)
		{
			SKILLTYPE *skill = skill_table[dt];
			bool found = FALSE;

			if(skill->imm_char && skill->imm_char[0] != '\0') {act( AT_HIT,    skill->imm_char, ch,  NULL, victim, TO_CHAR); 	found = TRUE;}
			if(skill->imm_vict && skill->imm_vict[0] != '\0') {act( AT_HITME,  skill->imm_vict, ch,  NULL, victim, TO_VICT); 	found = TRUE;}
			if(skill->imm_room && skill->imm_room[0] != '\0') {act( AT_ACTION, skill->imm_room, ch,  NULL, victim, TO_NOTVICT);	found = TRUE;}
			if(found) return rNONE;
		}
		dam = 0;
	}

	//Humanoid opponents react to the state of the fight instead of shouting at random.
	//Low frequency keeps multi-attack rounds readable.
	if(IS_NPC(ch) && !IS_CREATURE_MOB(ch) && number_percent() <= 6)
	{
		int quote = number_range(1, 5);

		if(ch->hit * 100 < ch->max_hit * 30)
		{
			if(quote == 1) do_say(ch, "Stay back!");
			if(quote == 2) do_say(ch, "I won't fall here!");
			if(quote == 3) do_say(ch, "You're tougher than you look.");
			if(quote == 4) do_say(ch, "This fight isn't over!");
			if(quote == 5) do_say(ch, "You'll have to do better than that!");
		}
		else if(victim->hit * 100 < victim->max_hit * 30)
		{
			if(quote == 1) do_say(ch, "Yield while you still can!");
			if(quote == 2) do_say(ch, "Your strength is failing!");
			if(quote == 3) do_say(ch, "It's nearly over!");
			if(quote == 4) do_say(ch, "Stand down!");
			if(quote == 5) do_say(ch, "You can barely stand!");
		}
		else if(victim->level > ch->level + 5)
		{
			if(quote == 1) do_say(ch, "I underestimated you!");
			if(quote == 2) do_say(ch, "You won't intimidate me!");
			if(quote == 3) do_say(ch, "A worthy opponent, then!");
			if(quote == 4) do_say(ch, "I can still win this!");
			if(quote == 5) do_say(ch, "Let's see what you can do!");
		}
		else
		{
			if(quote == 1) do_say(ch, "Stand and fight!");
			if(quote == 2) do_say(ch, "Watch your footing!");
			if(quote == 3) do_say(ch, "You won't get past me!");
			if(quote == 4) do_say(ch, "Come on, then!");
			if(quote == 5) do_say(ch, "Try that again, fetcher!");
		}
	}
/*
	char buf[256];

	snprintf(buf, 256, "&rYou roll the dice: Thac0 - 1d20 = %d - %d = %d to hit AC: %d&D", thac0, diceroll, attack_chance, victim_ac); 	
	act(AT_SKILL, buf, ch, NULL, victim, TO_CHAR);
	snprintf(buf, 256, "&r$n rolls the dice: Thac0 - 1d20 = %d - %d = %d to hit AC: %d&D", thac0, diceroll, attack_chance, victim_ac);	
	act(AT_SKILL, buf, ch, NULL, victim, TO_VICT);

	if(!wield)
    {
		snprintf(buf, 256, "&pYou roll the dice: %dd%d%-+d x pos = %d x %0.2f = %d&D", ch->barenumdie, ch->baresizedie, GET_DAMROLL(ch), total_dmg, pos_atk, dam);	
		act(AT_SKILL, buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, 256, "&p$n rolls the dice: %dd%d%-+d x pos = %d x %0.2f = %d&D", ch->barenumdie, ch->baresizedie, GET_DAMROLL(ch), total_dmg, pos_atk, dam);
		act(AT_SKILL, buf, ch, NULL, victim, TO_VICT);
    }

	if(wield)
    {
		snprintf(buf, 256, "&pYou roll the dice: (%dd%d%-+d + bonus) x pos = (%d + %2.2f) x %0.2f = %d&D", wield->value[1], wield->value[2], GET_DAMROLL(ch), total_dmg, bonus, pos_atk, dam);
		act(AT_SKILL, buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, 256, "&p$n rolls the dice: (%dd%d%-+d + bonus) x pos = (%d + %2.2f) x %0.2f = %d&D", wield->value[1], wield->value[2], GET_DAMROLL(ch), total_dmg, bonus, pos_atk, dam);
		act(AT_SKILL, buf, ch, NULL, victim, TO_VICT);
    }
*/

	//FINAL DAMAGE: downstream death/loot/scripts may invalidate either pointer.
	if((retcode = damage(ch, victim, dam, dt)) != rNONE) 	return retcode;
	if(char_died(ch)) 										return rCHAR_DIED;
	if(char_died(victim)) 									return rVICT_DIED;
	retcode = rNONE;

	if(dam == 0) return retcode;

//WEAPON ENCHANTMENTS: hit-triggered spells must stop if either participant dies.
    //Weapon spell support (Each successful hit casts a spell.)
	if( wield && !IS_SET( victim->immune, RIS_MAGIC ) && !xIS_SET( victim->in_room->room_flags, ROOM_NO_MAGIC ) )
	{
		AFFECT_DATA *aff;

		for(aff = wield->pIndexData->first_affect; aff; aff = aff->next)
			if(aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
            retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level + 3 ) / 3, ch, victim);
		
		if(retcode == rSPELL_FAILED) retcode = rNONE;
		if(retcode != rNONE || char_died(ch) || char_died(victim)) return retcode;
		
		for(aff = wield->first_affect; aff; aff = aff->next)
			if(aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
            retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level + 3) / 3, ch, victim);

		if(retcode == rSPELL_FAILED) retcode = rNONE;
		if(retcode != rNONE || char_died(ch) || char_died(victim)) return retcode;
	}

    //Magic shields that retaliates.
	if(IS_AFFECTED(victim, AFF_FIRESHIELD) && !IS_AFFECTED(ch, AFF_FIRESHIELD)) 		retcode = DREAMWALK_SHIELD_RETALIATE(victim, ch, 3, tes_effect_damage_fire);
	if(retcode != rNONE || char_died(ch) || char_died(victim)) 							return retcode;

	if(IS_AFFECTED( victim, AFF_ICESHIELD) && !IS_AFFECTED( ch, AFF_ICESHIELD)) 		retcode = DREAMWALK_SHIELD_RETALIATE(victim, ch, 3, tes_effect_damage_frost);
	if(retcode != rNONE || char_died(ch) || char_died(victim)) 							return retcode;

	if(IS_AFFECTED(victim, AFF_SHOCKSHIELD) && !IS_AFFECTED(ch, AFF_SHOCKSHIELD)) 		retcode = DREAMWALK_SHIELD_RETALIATE(victim, ch, 3, tes_effect_damage_shock);
	if(retcode != rNONE || char_died(ch) || char_died(victim)) 							return retcode;

	if(IS_AFFECTED(victim, AFF_ACIDMIST) && !IS_AFFECTED(ch, AFF_ACIDMIST)) 			retcode = spell_smaug(skill_lookup("acidshot"), off_shld_lvl(victim, ch), victim, ch);
	if(retcode != rNONE || char_died(ch) || char_died( victim))							return retcode;

	if(IS_AFFECTED(victim, AFF_VENOMSHIELD) && !IS_AFFECTED(ch, AFF_VENOMSHIELD)) 		retcode = DREAMWALK_SHIELD_RETALIATE(victim, ch, 3, tes_effect_damage_poison);
	if(retcode != rNONE || char_died(ch) || char_died(victim))							return retcode;

	if(IS_AFFECTED(victim, AFF_DRAIN_FATIGUE) && !IS_AFFECTED(ch, AFF_DRAIN_FATIGUE))	retcode = spell_smaug(skill_lookup("Fatigue leech"), off_shld_lvl(victim, ch), victim, ch);
	if(retcode != rNONE || char_died(ch) || char_died(victim))							return retcode;

	if(IS_AFFECTED(victim, AFF_DRAIN_MAGICKA) && !IS_AFFECTED(ch, AFF_DRAIN_MAGICKA))	retcode = spell_smaug(skill_lookup( "Magicka leech"), off_shld_lvl(victim, ch), victim, ch);
	if(retcode != rNONE || char_died(ch) || char_died(victim))							return retcode;

	if(IS_AFFECTED(victim, AFF_DRAIN_HEALTH) && !IS_AFFECTED(ch, AFF_DRAIN_HEALTH))		retcode = DREAMWALK_SHIELD_RETALIATE(victim, ch, 3, tes_effect_drain_health);
	if(retcode != rNONE || char_died(ch) || char_died(victim))							return retcode;

	tail_chain();
	return retcode;
}
ch_ret projectile_hit							(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, OBJ_DATA *projectile, short dist){
	//Hit one guy with a projectile.
	//Handles use of missile weapons (wield = missile weapon) or thrown items/weapons
	int victim_ac, thac0, thac0_00, thac0_32, plusris, dam, diceroll;
	int prof_bonus;
	int prof_gsn = -1;
	int proj_bonus;
	int dt;
	ch_ret retcode;

	//PROJECTILE DATA: thrown ITEM_WEAPON objects train Dart/Star/Knife; bows train their own skill.
	if(!projectile) return rNONE;
	if(projectile->item_type == ITEM_PROJECTILE || projectile->item_type == ITEM_WEAPON)
	{
		dt = TYPE_HIT + projectile->value[3];
		proj_bonus = number_range(projectile->value[1], projectile->value[2]);
	}
	else
	{
		dt = TYPE_UNDEFINED;
		proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile), 100));
	}

	//Can't beat a dead char!
	if(victim->position == POS_DEAD || char_died(victim))
	{
		extract_obj(projectile);
		return rVICT_DIED;
	}

	if(wield) 	{prof_bonus = weapon_prof_bonus_check(ch, wield, &prof_gsn);}
	else		{prof_bonus = 0;}

	//A thrown Dart, Star or Knife always uses its OWN skill, never the wielded bow skill.
	//Only real ITEM_WEAPON subtypes in value[4] 1..3 override the bow. Bow ammunition
	//(ITEM_PROJECTILE) continues to use the individual Shortbow/Longbow/Crossbow skill.
	if(projectile->item_type == ITEM_WEAPON && projectile->value[4] >= 1 && projectile->value[4] <= 3)
		prof_bonus = weapon_prof_bonus_check(ch, projectile, &prof_gsn);

	if(dt == TYPE_UNDEFINED)
	{
		dt = TYPE_HIT;
		
		if(wield && wield->item_type == ITEM_MISSILE_WEAPON) 
		dt += wield->value[3];
	}

	//Calculate to-hit-armor-class-0 versus armor.
	if(IS_NPC(ch))
	{
		thac0_00 = ch->mobthac0;
		thac0_32 = 0;
	}
	else
	{
		thac0_00 = class_table[ch->Class]->thac0_00;
		thac0_32 = class_table[ch->Class]->thac0_32;
	}
	
	//DISTANCE: each room of range makes hitting harder by 2 THAC0.
	thac0 = interpolate(ch->level, thac0_00, thac0_32) - GET_HITROLL(ch) + (dist * 2);
	victim_ac = UMAX(-19, (int)(GET_AC(victim) /10));

	//If you can't see what's coming... 
	if(!can_see_obj(victim, projectile)) 	victim_ac += 1;
	if(!can_see(ch, victim))				victim_ac -= 4;

	//Ranged proficiency: raising effective target AC makes THAC0 easier to beat.
	//The helper returns the SAME +1 hitroll per 10 skill points as melee.
	//Do not also subtract this from thac0 here or it would count twice.
	victim_ac += prof_bonus;

	//The moment of excitement!
	//D20 ACCURACY: roll 0..19 here (legacy system), with 0 an automatic miss.
	while( (diceroll = number_bits(5)) >= 20) ;

	if(diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
	{
       //Miss. 
		if(IS_VALID_SN(prof_gsn)) learn_from_failure(ch, prof_gsn);

		//Do something with the projectile 
		if(number_percent() < 50) {extract_obj(projectile);}
		else
		{
			if(projectile->in_obj) 		obj_from_obj(projectile);
			if(projectile->carried_by) 	obj_from_char(projectile);
			obj_to_room(projectile, victim->in_room);
		}
		damage(ch, victim, 0, dt);
		tail_chain();
		return rNONE;
	}
	
	//RANGED DAMAGE: bow + projectile or thrown projectile; then stance/resistances.
	//Hit. (Calc damage.)
	if(!wield) 	dam = proj_bonus;
	else		dam = number_range(wield->value[1], wield->value[2]) + (proj_bonus /10);

	//Bonuses.
	dam += GET_DAMROLL(ch);
	if(prof_bonus) {dam += prof_bonus /4;}

	//Calculate Damage Modifiers from Victim's Fighting Style
		 if(victim->position == POS_BERSERK) 	dam = (int)(1.2 * dam);
	else if(victim->position == POS_AGGRESSIVE) dam = (int)(1.1 * dam);
	else if(victim->position == POS_DEFENSIVE)  dam = (int)(.85 * dam);
	else if(victim->position == POS_EVASIVE)	dam = (int)( .8 * dam);

	if(!IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0)
	{
		dam += (int)(dam * LEARNED(ch, gsn_enhanced_damage ) /120);
		learn_from_success( ch, gsn_enhanced_damage );
	}

	if(!IS_AWAKE(victim)) 	dam *= 2;
	if(dam <= 0)			dam  = 1;
	
	plusris = 0;

	if(IS_OBJ_STAT(projectile, ITEM_MAGIC)) dam = ris_damage(victim, dam, RIS_MAGIC);
	else									dam = ris_damage(victim, dam, RIS_NONMAGIC);

	//Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll.
	if(wield) {plusris = obj_hitroll(wield);}

	//Check for RIS_PLUSx
	if(dam)
	{
		int x, res, imm, sus, mod;

		//PLUS1 starts at RIS_PLUS1; clamp to the available PLUS1..PLUS6 bits.
		if(plusris > 0) {plusris = RIS_PLUS1 << URANGE(0, plusris - 1, 5);}

		//Initialize values to handle a zero plusris 
		imm = res = -1;
		sus = 1;

		//Find high ris 
		for(x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1)
		{
			if(IS_SET(victim->immune, x))		imm = x;
			if(IS_SET(victim->resistant, x))	res = x;
			if(IS_SET(victim->susceptible, x))	sus = x;
		}
		
		mod = 10;
		if(imm >= plusris)	mod -= 10;
		if(res >= plusris)	mod -= 2;
		if(sus <= plusris)	mod += 2;

		//Check if immune 
		if(mod <= 0)	dam = -1;
		else if(mod != 10) dam = (dam * mod) /10;
	}

	if(IS_VALID_SN(prof_gsn))
	{
		if(dam > 0) learn_from_success(ch, prof_gsn);
		else		learn_from_failure(ch, prof_gsn);
	}

	//Immune to damage 
	if(dam == -1)
	{
		if(dt >= 0 && dt < num_skills)
		{
			SKILLTYPE *skill = skill_table[dt];
			bool found = FALSE;

			if(skill->imm_char && skill->imm_char[0] != '\0') {act(AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR); 	  found = TRUE;}
			if(skill->imm_vict && skill->imm_vict[0] != '\0') {act(AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT); 	  found = TRUE;}
			if(skill->imm_room && skill->imm_room[0] != '\0') {act(AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT); found = TRUE;}
			
			if(found)
			{
				if(number_percent() < 50) {extract_obj(projectile);}
				else
				{
					if(projectile->in_obj) 		{obj_from_obj(projectile);	}
					if(projectile->carried_by)	{obj_from_char( projectile);}
					
					obj_to_room( projectile, victim->in_room );
				}
				return rNONE;
			}
		}
		dam = 0;
   }
   
	if((retcode = damage(ch, victim, dam, dt)) != rNONE)	{extract_obj(projectile); 	return retcode;	  }
	if(char_died(ch))										{extract_obj(projectile);	return rCHAR_DIED;}
	if(char_died(victim))									{extract_obj(projectile); 	return rVICT_DIED;}

	retcode = rNONE;
	if(dam == 0)
	{
		if(number_percent() < 50) {extract_obj(projectile);}
		else
		{
			if(projectile->in_obj) 		{obj_from_obj(projectile); }
			if(projectile->carried_by) 	{obj_from_char(projectile);}
			
			obj_to_room(projectile, victim->in_room);
		}
		return retcode;
	}

	//PROJECTILE ENCHANTMENTS: ensure fired ammo is extracted or lands safely.
	//Weapon spells.
	if(wield && !IS_SET(victim->immune, RIS_MAGIC) && !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC))
	{
		AFFECT_DATA *aff;

		//Prototype and instance enchantments: stop immediately if one kills a combatant.
		for(aff = wield->pIndexData->first_affect; aff; aff = aff->next)
		{
			if(aff->location != APPLY_WEAPONSPELL || !IS_VALID_SN(aff->modifier) || !skill_table[aff->modifier]->spell_fun) continue;
			retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level +3 ) /3, ch, victim);
			if(retcode != rNONE || char_died(ch) || char_died(victim))
			{extract_obj(projectile); return retcode;}
		}

		for(aff = wield->first_affect; aff; aff = aff->next)
		{
			if(aff->location != APPLY_WEAPONSPELL || !IS_VALID_SN(aff->modifier) || !skill_table[aff->modifier]->spell_fun) continue;
			retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level + 3) /3, ch, victim);
			if(retcode != rNONE || char_died(ch) || char_died(victim))
			{extract_obj(projectile); return retcode;}
		}
	}

	extract_obj(projectile);
	tail_chain();
	return retcode;
}
short ris_damage								(CHAR_DATA *ch, short dam, int ris){
//Calculate damage based on resistances, immunities and suceptibilities

	short modifier;

	modifier = 10;
	if(IS_SET(ch->immune, ris) && !IS_SET(ch->no_immune, ris))				modifier -= 10;
	if(IS_SET(ch->resistant, ris) && !IS_SET(ch->no_resistant, ris))		modifier -= 2;
	
	if(IS_SET(ch->susceptible, ris) && !IS_SET(ch->no_susceptible, ris))
	{
		if( IS_NPC(ch) && IS_SET(ch->immune, ris))							modifier += 0;
		else																modifier += 2;
	}
	if(modifier <= 0)	return -1;
	if(modifier == 10)	return dam;
	
	return (dam * modifier) /10;
}
ch_ret damage									(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt){
	//Inflict damage from a hit. This is one damn big function.
	char log_buf[MAX_STRING_LENGTH];
	char filename[256];
	OBJ_DATA *damobj;
	int maxdam;		//Damage limit can exceed the range of 'short' at high levels.
	bool npcvict;
	bool loot;
	int xp_gain;
	ch_ret retcode;
	short dampmod;
	int reflected_damage = 0;
	CHAR_DATA *gch; 	//lch
	short anopc = 0;  	//# of (non-pkill) pc in a (ch)
	short bnopc = 0;  	//# of (non-pkill) pc in b (victim)

	retcode = rNONE;

	if(!ch)			{bug("%s: null ch!", __func__); 	return rERROR;	  }
	if(!victim)		{bug("%s: null victim!", __func__);	return rVICT_DIED;}
	if(victim->position == POS_DEAD)					return rVICT_DIED;

	npcvict = IS_NPC( victim );

	//DAMAGE PIPELINE STEP 1: element, disease, or physical resistances.
	//Check damage types for RIS
	if(!dreamwalk_reflect_magic_damage && dam && dt != TYPE_UNDEFINED)
	{
		     if(IS_FIRE(dt))						dam = ris_damage(victim, dam, RIS_FIRE);
		else if(IS_COLD(dt))						dam = ris_damage(victim, dam, RIS_COLD);
		else if(IS_ACID(dt))						dam = ris_damage(victim, dam, RIS_ACID);
		else if(IS_ELECTRICITY(dt))					dam = ris_damage(victim, dam, RIS_ELECTRICITY);
		else if(IS_ENERGY(dt))						dam = ris_damage(victim, dam, RIS_ENERGY);
		else if(IS_DRAIN(dt))						dam = ris_damage(victim, dam, RIS_DRAIN);
		else if(dt == gsn_poison || IS_POISON(dt))	dam = ris_damage(victim, dam, RIS_POISON);
		
		else if(dt == (TYPE_HIT + DAM_POUND) || dt == (TYPE_HIT + DAM_CRUSH)  || dt == (TYPE_HIT + DAM_STONE) || dt == (TYPE_HIT + DAM_PEA)) 	
			dam = ris_damage(victim, dam, RIS_BLUNT);
		
		else if(dt == (TYPE_HIT + DAM_STAB ) || dt == (TYPE_HIT + DAM_PIERCE) || dt == (TYPE_HIT + DAM_BITE) || dt == (TYPE_HIT + DAM_BOLT) || dt == (TYPE_HIT + DAM_DART )  || dt == (TYPE_HIT + DAM_ARROW))								
			dam = ris_damage( victim, dam, RIS_PIERCE );
		
		else if( dt == (TYPE_HIT + DAM_SLICE) || dt == (TYPE_HIT + DAM_SLASH) || dt == (TYPE_HIT + DAM_WHIP) || dt == (TYPE_HIT + DAM_CLAW))
			dam = ris_damage( victim, dam, RIS_SLASH );

		if(dam == -1)
		{
			if(dt >= 0 && dt < num_skills)
			{
				bool found = FALSE;
				SKILLTYPE *skill = skill_table[dt];

				if(skill->imm_char && skill->imm_char[0] != '\0') {act(AT_HIT   , skill->imm_char, ch, NULL, victim, TO_CHAR); 	  found = TRUE;}
				if(skill->imm_vict && skill->imm_vict[0] != '\0') {act(AT_HITME , skill->imm_vict, ch, NULL, victim, TO_VICT); 	  found = TRUE;}
				if(skill->imm_room && skill->imm_room[0] != '\0') {act(AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT); found = TRUE;}
				
				if(found) return rNONE;
			}
			dam = 0;
		}
	}


	//Precautionary step mainly to prevent people in Hell from finding a way out.
	if(xIS_SET(victim->in_room->room_flags, ROOM_SAFE)) dam = 0;

	if(dam && npcvict && ch != victim)
	{
		if(!xIS_SET(victim->act, ACT_SENTINEL))
		{
			if(victim->hunting)
			{
				if(victim->hunting->who != ch)
				{
					STRFREE(victim->hunting->name);
					victim->hunting->name = QUICKLINK(ch->name);
					victim->hunting->who = ch;
				}
			}
			else if(!xIS_SET(victim->act, ACT_PACIFIST))
			start_hunting(victim, ch);
		}

		if(victim->hating)
		{
			if(victim->hating->who != ch)
			{
				STRFREE(victim->hating->name);
				victim->hating->name = QUICKLINK(ch->name);
				victim->hating->who = ch;
			}
		}
		else if(!xIS_SET(victim->act, ACT_PACIFIST))
		start_hating( victim, ch );
	}


	//DAMAGE LIMIT: cap single hits before passing to PvP/fighting code.
	//Stop up any residual loopholes.
	if(dt == tes_skill_backstab_creature) 	{maxdam = ch->level * 80;}
	else									{maxdam = ch->level * 40;}

	if(!dreamwalk_reflect_magic_damage && dam > maxdam)
	{
		bug("Damage: %d more than %d points!", dam, maxdam);
		bug("** %s (lvl %d) -> %s **", ch->name, ch->level, victim->name);
		dam = maxdam;
	}

	//COMBAT ENGAGEMENT: legality, battle positions, groups, and aggression.
	if(victim != ch)
	{
		//Certain attacks are forbidden. (Most other attacks are returned.)
		// Stop any damage.
		if(victim == supermob) 			return rNONE;
		if(is_safe(ch, victim, TRUE)) 	return rNONE;
		
		check_attacker(ch, victim);
		if(victim->position > POS_STUNNED)
		{
			if(!victim->fighting && victim->in_room == ch->in_room) {set_fighting(victim, ch);}

			//vwas: victim->position = POS_FIGHTING; 
			if(IS_NPC(victim) && victim->fighting)
			{
				if(victim->position != POS_DEFENSIVE && victim->position != POS_AGGRESSIVE && victim->position != POS_EVASIVE && victim->position != POS_BERSERK)
					victim->position = POS_FIGHTING;
			}
			else if(victim->fighting)
			{
				switch (victim->style)
				{
					case (STYLE_EVASIVE ):   victim->position = POS_EVASIVE;    break;
					case (STYLE_DEFENSIVE):  victim->position = POS_DEFENSIVE;  break;
					case (STYLE_AGGRESSIVE): victim->position = POS_AGGRESSIVE; break;
					case (STYLE_BERSERK):    victim->position = POS_BERSERK;    break;
					default: 				 victim->position = POS_FIGHTING;
				}	
			}
		}

		if(victim->position > POS_STUNNED)
		{
			if(!ch->fighting && victim->in_room == ch->in_room) {set_fighting( ch, victim );}

			//If victim is charmed, ch might attack victim's master.
			if( IS_NPC(ch) && npcvict && IS_AFFECTED(victim, AFF_CHARM) && victim->master && victim->master->in_room == ch->in_room && number_bits(3) == 0)
			{
				stop_fighting(ch, FALSE);
				retcode = multi_hit(ch, victim->master, TYPE_UNDEFINED);
				return retcode;
			}
		}
		
		//More charm stuff.
		if(victim->master == ch) stop_follower(victim);

		//Pkill stuff.  If a deadly attacks another deadly or is attacked by one, then ungroup any nondealies.  Disabled untill I can figure out the right way to do it.
		
		//Count the # of non-pkill pc in a (not including == ch) 
		for(gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
			if(is_same_group(ch, gch) && !IS_NPC(gch) && !IS_PKILL(gch) && (ch != gch)) 
				{anopc++;}

		//Count the # of non-pkill pc in b (not including == victim) 
		for(gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
			if(is_same_group( victim, gch ) && !IS_NPC(gch) && !IS_PKILL(gch) && (victim != gch)) 
				{bnopc++;}

		//Only consider disbanding if both groups have 1(+) non-pk pc, or when one participant is pc, and the other group has 1(+) pk pc's (in the case that participant is only pk pc in group) 
		if((bnopc > 0 && anopc > 0) || (bnopc > 0 && !IS_NPC(ch)) || (anopc > 0 && !IS_NPC(victim)))
		{
			//Disband from same group first 
			if(is_same_group(ch, victim))
			{
				//Messages to char and master handled in stop_follower 
				act(AT_GREY, "&W$n&D disbands from &W$N&D's group.&D", (ch->leader == victim) ? victim : ch, NULL, (ch->leader == victim) ? victim->master : ch->master, TO_NOTVICT);
				
				if(ch->leader == victim) stop_follower(victim);
				else					 stop_follower(ch);	
			}

			//If leader isnt pkill, leave the group and disband ch 
			if(ch->leader != NULL && !IS_NPC(ch->leader) && !IS_PKILL(ch->leader))
			{
				act(AT_GREY, "&W$n&D disbands from &W$N&D's group.&D", ch, NULL, ch->master, TO_NOTVICT);
				stop_follower(ch);
			}
			else
			{
				for(gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
					if(is_same_group(gch, ch) && !IS_NPC(gch) && !IS_PKILL(gch) && gch != ch)
					{
						act(AT_GREY, "&W$n&D disbands from &W$N&D's group.&D", ch, NULL, gch->master, TO_NOTVICT);
						stop_follower( gch );
					}
			}

			//If leader isnt pkill, leave the group and disband victim 
			if(victim->leader != NULL && !IS_NPC(victim->leader) && !IS_PKILL(victim->leader))
			{
				act(AT_GREY, "&W$n&D disbands from &W$N&D's group.&D", victim, NULL, victim->master, TO_NOTVICT);
				stop_follower(victim);
			}
			else
			{
				for(gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
					if(is_same_group(gch, victim) && !IS_NPC(gch) && !IS_PKILL(gch) && gch != victim)
					{
						act(AT_GREY, "&W$n&D disbands from &W$N&D's group.&D", gch, NULL, gch->master, TO_NOTVICT);
						stop_follower(gch);
					}
			}
		}
		
		//Inviso attacks... not.
		if(IS_AFFECTED(ch, AFF_INVISIBLE))
		{
			affect_strip(ch, tes_effect_invisibility);
			affect_strip(ch, gsn_mass_invis);
			xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
			act(AT_GREY, "&W$n&D fades into existence.&D", ch, NULL, NULL, TO_ROOM);
		}

		//Take away Hide 
		if(IS_AFFECTED(ch, AFF_HIDE)) {xREMOVE_BIT(ch->affected_by, AFF_HIDE);}

		//Damage modifiers.
		if(!dreamwalk_reflect_magic_damage && IS_AFFECTED(victim, AFF_SANCTUARY)) {dam /= 2;}
		if(!dreamwalk_reflect_magic_damage && IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch)) {dam -= (int)(dam / 4);}
		if(dam < 0) 										 {dam = 0;}

		//Check for disarm, trip, parry, dodge and tumble.
		//Automatic spells and maneuvers are resolved once in multi_hit, before weapon strikes.


		//SCALING: sysdata control multipliers for NPC/PC combinations.
	//Check control panel settings and modify damage
		if(IS_NPC(ch))
		{
			if(npcvict) 
				{dampmod = sysdata.dam_mob_vs_mob;}
			else
			{
				if(victim->level < LEVEL_AVATAR) {dampmod = sysdata.dam_mob_vs_nonav;}
				else 							 {dampmod = sysdata.dam_mob_vs_plr;}
			}
		}
		else
		{
			if(npcvict)
			{
				if(ch->level < LEVEL_AVATAR) {dampmod = sysdata.dam_nonav_vs_mob;}
				else						 {dampmod = sysdata.dam_plr_vs_mob;}
			}
			else {dampmod = sysdata.dam_plr_vs_plr;}
		}

		if(dampmod > 0 && !dreamwalk_reflect_magic_damage) {dam = (dam * dampmod) / 100;}
	}

	//Code to handle equipment getting damaged, and also support bonuses/penalties for having or not having equipment where hit
	/*
	if(dam > 10 && dt != TYPE_UNDEFINED)
	{
      dameq = number_range(WEAR_LIGHT, WEAR_ANKLE_R);
      damobj = get_eq_char(victim, dameq);
	  
      if(damobj)
      {
			if(dam > get_obj_resistance(damobj) && number_bits(1) == 0)
			{
				set_cur_obj(damobj);
				damage_obj(damobj);
			}
			dam -= 5;   * add a bonus for having something to block the blow *
      }
      else
			dam += 5;   * add penalty for bare skin! *
	}
	*/
	
	//REFLECT MAGIC: split final spell damage after resistance and scaling.
	if(!dreamwalk_reflect_magic_damage && !dreamwalk_shield_retaliation && dam > 1 && ch != victim
	&& dt >= 0 && dt < num_skills && skill_table[dt] && skill_table[dt]->type == SKILL_SPELL
	&& is_affected(victim, tes_effect_reflect_magic))
	{
		reflected_damage = dam / 2;
		dam -= reflected_damage;
	}

	if(ch != victim) {dam_message(ch, victim, dam, dt);}

	//APPLY DAMAGE: hit-point loss, kill handling, XP, status, and fleeing.
	//Hurt the victim. Inform the victim of his new state.
	victim->hit -= dam;

	//Get experience based on % of damage done.
	if(dam && ch != victim && !IS_NPC(ch) && ch->fighting && ch->fighting->xp)
	{
		if(ch->fighting->who == victim) 						  {xp_gain = (int)(ch->fighting->xp * dam) / victim->max_hit;}
		else													  {xp_gain = (int)( xp_compute(ch, victim ) * 0.85 * dam) / victim->max_hit;}

		if(dt == tes_skill_backstab_creature || dt == gsn_circle) {xp_gain = (int)(xp_gain * 0.05);}

		gain_exp(ch, xp_gain);
	}

	if(!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1) 
	victim->hit = 1;

	//Make sure newbies dont die.
	if(!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
    victim->hit = 1;

	if(dam > 0 && dt > TYPE_HIT && !IS_AFFECTED(victim, AFF_POISON) && is_wielding_poisoned(ch) && !IS_SET(victim->immune, RIS_POISON) && !saves_poison_death(ch->level, victim))
	{
		AFFECT_DATA af;
		af.type = gsn_poison;
		af.duration = 20;
		af.location = APPLY_STR;
		af.modifier = -2;
		af.bitvector = meb(AFF_POISON);
		affect_join(victim, &af);
		victim->mental_state = URANGE(20, victim->mental_state + (IS_PKILL(victim) ? 1 : 2), 100);
	}

	//Vampire self preservation.
	if(IS_VAMPIRE(victim))
	{
		if(dam >= (victim->max_hit /10)) 
		gain_condition(victim, COND_BLOODTHIRST, -1 - (victim->level /20));
		
		if(victim->hit <= (victim->max_hit /8) && victim->pcdata->condition[COND_BLOODTHIRST] > 5)
		{
			gain_condition(victim, COND_BLOODTHIRST, -URANGE(3, victim->level / 10, 8));
			victim->hit += URANGE(4, (victim->max_hit /30), 15);
			set_char_color(AT_BLOOD, victim);
			send_to_char("You howl with rage as the beast within stirs!\r\n", victim);
		}
	}

	if(!npcvict && get_trust(victim) >= LEVEL_IMMORTAL && get_trust(ch) >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;

	update_pos(victim);

	switch (victim->position)
	{
		case POS_MORTAL:
			act(AT_YELLOW, "$n is mortally wounded."  , victim, NULL, NULL, TO_ROOM);
			act(AT_YELLOW, "You are mortally wounded.", victim, NULL, NULL, TO_CHAR);
			break;

		case POS_INCAP:
			act(AT_YELLOW, "$n is incapacitated."  , victim, NULL, NULL, TO_ROOM);
			act(AT_YELLOW, "You are incapacitated.", victim, NULL, NULL, TO_CHAR);
			break;

		case POS_STUNNED:
			if(!IS_AFFECTED(victim, AFF_PARALYSIS))
			{
				act(AT_YELLOW, "$n is stunned."  , victim, NULL, NULL, TO_ROOM);
				act(AT_YELLOW, "You are stunned.", victim, NULL, NULL, TO_CHAR);
			} break;

		case POS_DEAD:
			if(dt >= 0 && dt < num_skills)
			{
				SKILLTYPE *skill = skill_table[dt];

				if(skill->die_char && skill->die_char[0] != '\0') act(AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR);
				if(skill->die_vict && skill->die_vict[0] != '\0') act(AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT);
				if(skill->die_room && skill->die_room[0] != '\0') act(AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT);
			}
			act(AT_YELLOW, "$n is dead!"			  , victim, 0, 0, TO_ROOM);
			act(AT_YELLOW, "You have been killed!\r\n", victim, 0, 0, TO_CHAR);
			
			//******************************************//
			  CHAR_FACTION_KILL_ADJUSTMENT (ch, victim);	
			//******************************************//
			break;

		default:
			//Victim mentalstate affected, not attacker -- oops ;)
			if(dam > victim->max_hit /4)
			{
				act(AT_YELLOW, "That really did hurt!", victim, 0, 0, TO_CHAR);
				if(number_bits(3) == 0) 
					worsen_mental_state(victim, 1);
			}
			
			if(victim->hit < victim->max_hit /4)
			{
				act(AT_YELLOW, "You bleed so much!", victim, 0, 0, TO_CHAR);
				if(number_bits(2) == 0)
					worsen_mental_state( victim, 1 );
			}
			break;
	}

	//Sleep spells and extremely wounded folks.
	//lets make NPC's not slaughter PC's.
	if(!IS_AWAKE(victim) && !IS_AFFECTED(victim, AFF_PARALYSIS))
	{
		if(victim->fighting && victim->fighting->who->hunting && victim->fighting->who->hunting->who == victim)
        stop_hunting(victim->fighting->who);

		if(victim->fighting && victim->fighting->who->hating && victim->fighting->who->hating->who == victim)
        stop_hating(victim->fighting->who);

		if(!npcvict && IS_NPC(ch)) 	stop_fighting(victim, TRUE);
		else						stop_fighting(victim, FALSE);
	}

	//Payoff for killing things.
	if(victim->position == POS_DEAD)
	{
		OBJ_DATA *new_corpse;
		
		group_gain(ch, victim);

		if(!npcvict)
		{
			if(!victim->desc) 	{add_loginmsg(victim->name, 17, (IS_NPC(ch) ? ch->short_descr : ch->name));}

			snprintf(log_buf, MAX_STRING_LENGTH, "%s (%d) killed by %s at %d", victim->name, victim->level, (IS_NPC(ch) ? ch->short_descr : ch->name), victim->in_room->vnum);
			log_string_plus(log_buf, LOG_ALL, LEVEL_SUPREME);
			to_channel(log_buf, CHANNEL_DEATH, "Death", LEVEL_IMMORTAL);

			if( !IS_NPC( ch ) && !IS_IMMORTAL(ch) && ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD && victim != ch)
			{
				snprintf(filename, 256, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name);
				snprintf(log_buf, MAX_STRING_LENGTH, "&P(%2d) %-12s &wvs &P(%2d) %s &P%s ... &w%s",
                      ch->level, ch->name, victim->level, !CAN_PKILL(victim) ? "&W<Peaceful>" : victim->pcdata->clan ? victim->pcdata->clan->badge : "&P(&WUnclanned&P)", 
						victim->name, ch->in_room->area->name);
						
				if(victim->pcdata && victim->pcdata->clan && victim->pcdata->clan->name == ch->pcdata->clan->name) ;
				else append_to_file(filename, log_buf);
			}

			if(!IS_NPC( victim) && !IS_IMMORTAL(victim) && victim->pcdata->clan && victim->pcdata->clan->clan_type != CLAN_ORDER
				&& victim->pcdata->clan->clan_type != CLAN_GUILD && ch != victim && !IS_NPC( ch ) )
			{
				snprintf(filename, 256, "%s%s.defeats", CLAN_DIR, victim->pcdata->clan->name);
				snprintf(log_buf, MAX_STRING_LENGTH, "&P(%2d) %-12s &wdefeated by &P(%2d) %s &P%s ... &w%s",
                     victim->level, victim->name, ch->level, !CAN_PKILL(ch) ? "&W<Peaceful>" : ch->pcdata->clan ? ch->pcdata->clan->badge : "&P(&WUnclanned&P)", 
						ch->name, victim->in_room->area->name );
						
				if(ch->pcdata && ch->pcdata->clan && ch->pcdata->clan->name == victim->pcdata->clan->name) ;
				else append_to_file( filename, log_buf );
			}

			//Dying penalty: 1/2 way back to previous level.
			if(!IS_PKILL(victim))
			{
				if(victim->exp > exp_level(victim, victim->level)) 
					gain_exp(victim, (exp_level(victim, victim->level) - victim->exp) /2);
			}	

			//New penalty... go back to the beginning of current level.
			/*
			victim->exp = exp_level(victim, victim->level);
			*/

		}
		//Keep track of mob vnum killed
		else if(!IS_NPC(ch) && IS_NPC(victim))
		{
			add_kill(ch, victim);

			//Add to kill tracker for grouped chars, as well.
			for(gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
				if(is_same_group(gch, ch) && !IS_NPC(gch) && gch != ch)
					add_kill(gch, victim);
		}

		check_killer(ch, victim);

		if(!IS_NPC(ch) && ch->pcdata->clan) 		update_roster(ch);	   
		if(!IS_NPC(victim) && victim->pcdata->clan)	update_roster(victim);

		if(ch->in_room == victim->in_room)			loot = legal_loot(ch, victim);
		else										loot = FALSE;

		set_cur_char(victim);
		new_corpse = raw_kill(ch, victim);
		victim = NULL;

		if(!IS_NPC( ch ) && loot && new_corpse && new_corpse->item_type == ITEM_CORPSE_NPC && new_corpse->in_room == ch->in_room && can_see_obj(ch, new_corpse) && ch->position > POS_SLEEPING)
		{
			if(xIS_SET(ch->act, PLR_AUTOGOLD) && !loot_coins_from_corpse(ch, new_corpse))
			return rBOTH_DIED;

			if(new_corpse && !obj_extracted(new_corpse) && new_corpse->in_room == ch->in_room && ch->position > POS_SLEEPING && can_see_obj(ch, new_corpse))
			{
				if(xIS_SET( ch->act, PLR_AUTOLOOT)) do_get(ch, "all corpse");
				else								do_look(ch, "in corpse");
			
				if(!char_died(ch) && xIS_SET(ch->act, PLR_AUTOSAC) && !obj_extracted(new_corpse) && new_corpse->in_room == ch->in_room && ch->position > POS_SLEEPING && can_see_obj(ch, new_corpse))
				do_sacrifice(ch, "corpse");
			}
		}
	
		
		if(IS_SET(sysdata.save_flags, SV_KILL)) {save_char_obj(ch);}
		return rVICT_DIED;
    }

	//REFLECT MAGIC: apply returned damage after the original victim's death checks.
	if(reflected_damage > 0 && victim->hit > 0)
	{
		dreamwalk_reflect_magic_damage = TRUE;
		retcode = damage(victim, ch, reflected_damage, dt);
		dreamwalk_reflect_magic_damage = FALSE;

		if(retcode == rVICT_DIED) return rCHAR_DIED;
		if(retcode == rCHAR_DIED) return rVICT_DIED;
		if(retcode != rNONE) return retcode;
	}

	if(victim == ch) return rNONE;

	//ESCAPE LOGIC: linkdead recalls, NPC wimpy fleeing, player wimpy fleeing.
	//Take care of link dead people.
	if(!npcvict && !victim->desc && !IS_SET(victim->pcdata->flags, PCFLAG_NORECALL))
	{
		if(number_range(0, victim->wait) == 0) 	
		{
			do_recall(victim, ""); 
			return rNONE;
		}
	}

    //Wimp out?
	if(npcvict && dam > 0)
	{
		if((xIS_SET( victim->act, ACT_WIMPY) && number_bits(1) == 0 && victim->hit < victim->max_hit /2) 
			||(IS_AFFECTED( victim, AFF_CHARM) && victim->master && victim->master->in_room != victim->in_room))
		{
			start_fearing(victim, ch);
			stop_hunting(victim);
			do_flee(victim, "");
		}
	}

	if(!npcvict && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0)	do_flee(victim, "");
	else if(!npcvict && xIS_SET(victim->act, PLR_FLEE))										do_flee(victim, "");

	tail_chain();
	return rNONE;
}
bool is_safe									(CHAR_DATA *ch, CHAR_DATA *victim, bool show_messg){
	//Changed is_safe to have the show_messg boolian.  This is so if you don't want to show why you can't kill someone you can't turn it off.  
	//This is useful for things like area attacks.
	//Check pointers BEFORE calling char_died() or who_fighting().
	if(!ch) 			{bug("Is_safe: NULL attacker!"); return TRUE;}
	if(!victim) 		{bug("Is_safe: %s opponent does not exist!", ch->name); return TRUE;}
	if(char_died(victim) || char_died(ch)) return TRUE;

	if(who_fighting(ch) == ch)
    return FALSE;
	
	if(!victim->in_room) 
	{ 
		bug("Is_safe: %s has no physical location!", victim->name); 
		return TRUE; 
	}

	//ROOM RESTRICTION: a safe room always forbids combat.
	if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
	{
		if( show_messg )
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YA magical force prevents you from attacking.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		} 	return TRUE;
	}

	//PACIFIST RESTRICTIONS: attacker and target both matter.
	if(IS_PACIFIST(ch))
	{
		if( show_messg )
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YYou are a pacifist and will not fight.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		} 	return TRUE;
	}

	if(IS_PACIFIST(victim))
	{
		char buf[MAX_STRING_LENGTH];
		if(show_messg)
		{
			snprintf(buf, MAX_STRING_LENGTH, "&Y%s is a pacifist and will not fight.&D\r\n", capitalize(victim->short_descr));
			send_to_char(buf, ch);
		} 	return TRUE;
	}

	//STAFF OVERRIDE: immortal attacker bypasses remaining mortal PvP checks.
	if(!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL) 
	return FALSE;

	if(!IS_NPC(ch) && !IS_NPC(victim) && ch != victim && IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL))
	{
		if( show_messg )
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThe gods have forbidden player killing in this area.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		}   return TRUE;
	}

	if(IS_NPC(ch) || IS_NPC(victim))
    return FALSE;
/*
	if(calculate_age(ch) < 16 || ch->level < 1)
	{
		if(show_messg)
		{
         set_char_color(AT_WHITE, ch);
         send_to_char("You are not yet ready, needing age or experience, if not both. \r\n", ch);
		}
		return TRUE;
	}
*/
	if(calculate_age(victim) < 16 || victim->level < 1)
	{
		if(show_messg)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThey are yet too young to die.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		}   return TRUE;
	}

	if(ch->level - victim->level > 5 || victim->level - ch->level > 5)
	{
		if( show_messg )
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThe gods do not allow murder when there is such a difference in level.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		} 	return TRUE;
	}

	if(get_timer(victim, TIMER_PKILLED) > 0)
	{
		if(show_messg)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat character has died within the last 5 minutes.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		} 	return TRUE;
	}

	if(get_timer(ch, TIMER_PKILLED) > 0)
	{
		if(show_messg)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou have been killed within the last 5 minutes.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		}   return TRUE;
	}

	return FALSE;
}
bool legal_loot									(CHAR_DATA *ch, CHAR_DATA *victim){
	//Just verify that a corpse looting is legal

    //Anyone can loot mobs 
	if(IS_NPC(victim)) 
	return TRUE;
 
	//Non-charmed mobs can loot anything 
	if(IS_NPC(ch) && !ch->master) 
	return TRUE;

	//Members of different clans can loot too! -Thoric 
	if(!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) && IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
    return TRUE;

	return FALSE;
}
void check_killer								(CHAR_DATA *ch, CHAR_DATA *victim){
	//See if an attack justifies a KILLER flag.
 
    //NPC's are fair game.
	if(IS_NPC(victim))
	{
		if(!IS_NPC(ch))
		{
			int level_ratio;
			if(victim->level < 1)	level_ratio = URANGE(1, ch->level, MAX_LEVEL);
			else					level_ratio = URANGE(1, ch->level / victim->level, MAX_LEVEL);
			
			if(ch->pcdata->clan)	ch->pcdata->clan->mkills++;
			
			ch->pcdata->mkills++;
			ch->in_room->area->mkills++;
			
			if(ch->pcdata->deity)
			{
				if(victim->race == ch->pcdata->deity->npcrace)		adjust_favor(ch, 3, level_ratio);
				else if(victim->race == ch->pcdata->deity->npcfoe)	adjust_favor(ch, 17, level_ratio);
				else												adjust_favor(ch, 2, level_ratio);
			}
		}
		return;
	}

	//If you kill yourself nothing happens.
	if(ch == victim || ch->level >= LEVEL_IMMORTAL)
    return;

	//Any character in the arena is ok to kill. (Added pdeath and pkills here.)
	if(in_arena(ch))
	{
		if(!IS_NPC(ch) && !IS_NPC(victim))
		{
			ch->pcdata->pkills++;
			victim->pcdata->pdeaths++;
		}
		return;
	}

	//So are killers and thieves.
	if( xIS_SET(victim->act, PLR_KILLER) || xIS_SET(victim->act, PLR_THIEF))
	{
		if(!IS_NPC(ch))
		{
			if(ch->pcdata->clan)
			{
				     if(victim->level < 10)		ch->pcdata->clan->pkills[0]++;
				else if(victim->level < 15)		ch->pcdata->clan->pkills[1]++;
            	else if(victim->level < 20)		ch->pcdata->clan->pkills[2]++;
				else if(victim->level < 30)		ch->pcdata->clan->pkills[3]++;
				else if(victim->level < 40)		ch->pcdata->clan->pkills[4]++;
				else if(victim->level < 50)		ch->pcdata->clan->pkills[5]++;
				else							ch->pcdata->clan->pkills[6]++;
			}
			ch->pcdata->pkills++;
			ch->in_room->area->pkills++;
		}
		return;
	}


	//Clan checks
	if(!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) && IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
	{
       //Not of same clan? Go ahead and kill!!! 
		if(!ch->pcdata->clan || !victim->pcdata->clan || (ch->pcdata->clan->clan_type != CLAN_NOKILL && victim->pcdata->clan->clan_type != CLAN_NOKILL && ch->pcdata->clan != victim->pcdata->clan))
		{
			if(ch->pcdata->clan)
			{
					 if(victim->level < 10)		ch->pcdata->clan->pkills[0]++;
				else if(victim->level < 15)		ch->pcdata->clan->pkills[1]++;
				else if(victim->level < 20)		ch->pcdata->clan->pkills[2]++;
				else if(victim->level < 30)		ch->pcdata->clan->pkills[3]++;
				else if(victim->level < 40)		ch->pcdata->clan->pkills[4]++;
				else if(victim->level < 50)		ch->pcdata->clan->pkills[5]++;
				else							ch->pcdata->clan->pkills[6]++;
			}
			
			ch->pcdata->pkills++;
			ch->hit = ch->max_hit;
			ch->mana = ch->max_mana;
			ch->move = ch->max_move;
			
			if(ch->pcdata) {ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level);}
			
			update_pos(victim);
			
			if(victim != ch)
			{
				act(AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into $n.",  ch, victim->name, NULL, TO_ROOM);
				act(AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into you.", ch, victim->name, NULL, TO_CHAR);
			}
			
			if(victim->pcdata->clan)
			{
					 if(ch->level < 10)		victim->pcdata->clan->pdeaths[0]++;
				else if(ch->level < 15) 	victim->pcdata->clan->pdeaths[1]++;
				else if(ch->level < 20)		victim->pcdata->clan->pdeaths[2]++;
				else if(ch->level < 30)		victim->pcdata->clan->pdeaths[3]++;
				else if(ch->level < 40)		victim->pcdata->clan->pdeaths[4]++;
				else if(ch->level < 50)		victim->pcdata->clan->pdeaths[5]++;
				else						victim->pcdata->clan->pdeaths[6]++;
			}
			
			victim->pcdata->pdeaths++;
			adjust_favor(victim, 11, 1);
			adjust_favor(ch, 2, 1);
			add_timer(victim, TIMER_PKILLED, 115, NULL, 0);
			WAIT_STATE(victim, 3 * PULSE_VIOLENCE);
			return;
		}
	}

	//Charm-o-rama.
	if(IS_AFFECTED(ch, AFF_CHARM))
	{
		if(!ch->master)
		{
			bug("Check_killer: %s bad AFF_CHARM", IS_NPC(ch) ? ch->short_descr : ch->name);
			affect_strip(ch, gsn_charm_person);
			xREMOVE_BIT(ch->affected_by, AFF_CHARM);
			return;
		}

		//Stop_follower(ch); 
		if(ch->master)	check_killer(ch->master, victim);
		return;
	}

	//NPC's are cool of course (as long as not charmed). Hitting yourself is cool too (bleeding). 
	//So is being immortal (Alander's idea). And current killers stay as they are.
	if(IS_NPC(ch))
	{
		if(!IS_NPC(victim))
		{
			int level_ratio;
			if(victim->pcdata->clan)	victim->pcdata->clan->mdeaths++;
			
			victim->pcdata->mdeaths++;
			victim->in_room->area->mdeaths++;
			level_ratio = URANGE(1, ch->level / victim->level, LEVEL_AVATAR);
			
			if(victim->pcdata->deity)
			{
					 if(ch->race == victim->pcdata->deity->npcrace)		adjust_favor(victim, 12, level_ratio);
				else if(ch->race == victim->pcdata->deity->npcfoe)		adjust_favor(victim, 15, level_ratio);
				else													adjust_favor( victim, 11, level_ratio );
			}
		}
		return;
	}

	if(!IS_NPC(ch))
	{
		if(ch->pcdata->clan)	ch->pcdata->clan->illegal_pk++;
		
		ch->pcdata->illegal_pk++;
		ch->in_room->area->illegal_pk++;
	}

	if(!IS_NPC(victim))
	{
		if(victim->pcdata->clan)
		{
				 if(ch->level < 10)		victim->pcdata->clan->pdeaths[0]++;
			else if(ch->level < 15)		victim->pcdata->clan->pdeaths[1]++;
			else if(ch->level < 20)		victim->pcdata->clan->pdeaths[2]++;
			else if(ch->level < 30)		victim->pcdata->clan->pdeaths[3]++;
			else if(ch->level < 40)		victim->pcdata->clan->pdeaths[4]++;
			else if(ch->level < 50)		victim->pcdata->clan->pdeaths[5]++;
			else						victim->pcdata->clan->pdeaths[6]++;
		}
		victim->pcdata->pdeaths++;
		victim->in_room->area->pdeaths++;
	}

	if(xIS_SET(ch->act, PLR_KILLER))
    return;

	set_char_color(AT_WHITE, ch);	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A strange feeling grows deep inside you, and a tingle goes up your spine..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
	set_char_color(AT_WHITE, ch);	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
	set_char_color(AT_WHITE, ch);	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You feel as if your soul has been revealed for all to see."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
	xSET_BIT(ch->act, PLR_KILLER);
	
	if(xIS_SET(ch->act, PLR_ATTACKER))	
		xREMOVE_BIT(ch->act, PLR_ATTACKER);
	
	save_char_obj(ch);
	return;
}
void check_attacker								(CHAR_DATA *ch, CHAR_DATA *victim){
	//See if an attack justifies a ATTACKER flag.
	
	//NPC's are fair game. (So are killers and thieves.)
	if(IS_NPC(victim) || xIS_SET(victim->act, PLR_KILLER) || xIS_SET(victim->act, PLR_THIEF))	
	return;

	//Deadly char check
	if(!IS_NPC(ch) && !IS_NPC(victim) && CAN_PKILL(ch) && CAN_PKILL(victim))
    return;

	//Charm-o-rama.
	
	if(IS_AFFECTED(ch, AFF_CHARM))
	{
		if(!ch->master)
		{
			bug("Check_attacker: %s bad AFF_CHARM", IS_NPC( ch ) ? ch->short_descr : ch->name);
			affect_strip(ch, gsn_charm_person);
			xREMOVE_BIT(ch->affected_by, AFF_CHARM);
			return;
		}
		return;
	}

	//NPC's are cool of course (as long as not charmed). Hitting yourself is cool too (bleeding). So is being immortal.
    //And current killers stay as they are.
	if(IS_NPC(ch) || ch == victim || ch->level >= LEVEL_IMMORTAL || xIS_SET(ch->act, PLR_ATTACKER) || xIS_SET(ch->act, PLR_KILLER))
    return;

	xSET_BIT(ch->act, PLR_ATTACKER);
	save_char_obj(ch);
	return;
}
void update_pos									(CHAR_DATA *victim){
	//Set position of a victim.
	
	if(!victim)
	{
		bug("%s", "update_pos: null victim");
		return;
	}

	if(victim->hit > 0)
	{
		if(victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;

		return;
	}

	if(IS_NPC(victim) || victim->hit <= -11)
	{
		if(victim->mount)
		{
			act(AT_ACTION, "$n falls from $N.", victim, NULL, victim->mount, TO_ROOM);
			xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
			victim->mount = NULL;
		}
		victim->position = POS_DEAD;
		return;
	}

		 if(victim->hit <= -6)	victim->position = POS_DEAD;
	else if(victim->hit <= -3)	victim->position = POS_DEAD;
	else						victim->position = POS_DEAD;

	if(victim->position > POS_STUNNED && IS_AFFECTED(victim, AFF_PARALYSIS)) 
	victim->position = POS_STUNNED;

	if(victim->mount)
	{
		act(AT_ACTION, "$n falls unconscious from $N.", victim, NULL, victim->mount, TO_ROOM);
		xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
		victim->mount = NULL;
	}
	return;
}
void set_fighting								(CHAR_DATA *ch, CHAR_DATA *victim){
	//Start fights.
	FIGHT_DATA *fight;

	if(ch->fighting)				{bug( "%s: %s -> %s (already fighting %s)", __func__, ch->name, victim->name, ch->fighting->who->name ); return;}
	if(IS_AFFECTED(ch, AFF_SLEEP))	{affect_strip(ch, gsn_sleep);}

    //Limit attackers
	if(victim->num_fighting > MAX_FIGHT) 
	{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThere are too many people fighting for you to join in.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return;}

	CREATE(fight, FIGHT_DATA, 1);
	fight->who = victim;
	fight->xp = (int)(xp_compute(ch, victim) * 0.85);
	fight->align = align_compute(ch, victim);
	
	if(!IS_NPC(ch) && IS_NPC(victim))
	fight->timeskilled = times_killed(ch, victim);

	ch->num_fighting = 1;
	ch->fighting = fight;

    //ch->position = POS_FIGHTING; 
	
	if(IS_NPC(ch))	
		ch->position = POS_FIGHTING;
	else
		switch (ch->style)
		{
			case (STYLE_EVASIVE): 		ch->position = POS_EVASIVE;		break;
			case (STYLE_DEFENSIVE):		ch->position = POS_DEFENSIVE;	break;
			case (STYLE_AGGRESSIVE):	ch->position = POS_AGGRESSIVE;	break;
			case (STYLE_BERSERK):		ch->position = POS_BERSERK;		break;
			default:					ch->position = POS_FIGHTING;
		}
		
	victim->num_fighting++;
		
	if(victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
	{
      send_to_char( "&YYou are disturbed!&D\r\n", victim->switched);
      do_return(victim->switched, "");
	}
	return;
}
CHAR_DATA *who_fighting							(CHAR_DATA *ch){
	if(!ch)				{bug( "%s", "who_fighting: null ch" ); 	return NULL;}
	if(!ch->fighting)											return NULL;
	return ch->fighting->who;
}
void free_fight									(CHAR_DATA *ch){
	if(!ch)		
	{bug("%s: null ch!", __func__); return;}

	if(ch->fighting)
	{
		if(!char_died(ch->fighting->who)) --ch->fighting->who->num_fighting;
		DISPOSE(ch->fighting);
	}

	ch->fighting = NULL;
	
	if(ch->mount)	ch->position = POS_MOUNTED;
	else			ch->position = POS_STANDING;

	//Berserk wears off after combat. 

	if(IS_AFFECTED(ch, AFF_BERSERK))
	{
		affect_strip(ch, gsn_berserk);
		set_char_color(AT_WEAROFF, ch);
		send_to_char(skill_table[gsn_berserk]->msg_off, ch);
		send_to_char("\r\n", ch);
	}
	return;
}
void stop_fighting								(CHAR_DATA *ch, bool fBoth){
	//Stop fights.
	CHAR_DATA *fch;

	strip_grapple(ch);
	free_fight(ch);
	update_pos(ch);

	if(!fBoth) return;

	for(fch = first_char; fch; fch = fch->next)
	{
		if( who_fighting(fch) == ch)
		{
			free_fight(fch);
			update_pos(fch);
		}
	}
	return;
}
int part_vnums[] = { 
	//Vnums for the various bodyparts
	12,   /* Head */
	14,   /* arms */
	15,   /* legs */
	13,   /* heart */
	44,   /* brains */
	16,   /* guts */
	45,   /* hands */
	46,   /* feet */
	47,   /* fingers */
	48,   /* ear */
	49,   /* eye */
	50,   /* long_tongue */
	51,   /* eyestalks */
	52,   /* tentacles */
	53,   /* fins */
	54,   /* wings */
	55,   /* tail */
	56,   /* scales */
	59,   /* claws */
	87,   /* fangs */
	58,   /* horns */
	57,   /* tusks */
	55,   /* tailattack */
	85,   /* sharpscales */
	84,   /* beak */
	86,   /* haunches */
	83,   /* hooves */
	82,   /* paws */
	81,   /* forelegs */
	80,   /* feathers */
	79,	  /* husk_shell */
	0     /* r2 */
};
const char *part_messages[] = {
//Messages for flinging off the various bodyparts	
	"$n's severed head plops from its neck.",
	"$n's arm is sliced from $s dead body.",
	"$n's leg is sliced from $s dead body.",
	"$n's heart is torn from $s chest.",
	"$n's brains spill grotesquely from $s head.",
	"$n's guts spill grotesquely from $s torso.",
	"$n's hand is sliced from $s dead body.",
	"$n's foot is sliced from $s dead body.",
	"A finger is sliced from $n's dead body.",
	"$n's ear is sliced from $s dead body.",
	"$n's eye is gouged from its socket.",
	"$n's tongue is torn from $s mouth.",
	"An eyestalk is sliced from $n's dead body.",
	"A tentacle is severed from $n's dead body.",
	"A fin is sliced from $n's dead body.",
	"A wing is severed from $n's dead body.",
	"$n's tail is sliced from $s dead body.",
	"A scale falls from the body of $n.",
	"A claw is torn from $n's dead body.",
	"$n's fangs are torn from $s mouth.",
	"A horn is wrenched from the body of $n.",
	"$n's tusk is torn from $s dead body.",
	"$n's tail is sliced from $s dead body.",
	"A ridged scale falls from the body of $n.",
	"$n's beak is sliced from $s dead body.",
	"$n's haunches are sliced from $s dead body.",
	"A hoof is sliced from $n's dead body.",
	"A paw is sliced from $n's dead body.",
	"$n's foreleg is sliced from $s dead body.",
	"Some feathers fall from $n's dead body.",
	"$n's shell remains.",
	"r2 message."
};
void death_cry									(CHAR_DATA *ch){
	//Improved Death_cry contributed by Diavolo.
	ROOM_INDEX_DATA *was_in_room;
	const char *msg;
	EXIT_DATA *pexit;
	int vnum, shift, cindex, i;

	if(!ch)	{bug("%s: null ch!", __func__); return;}

	vnum = 0; 
	msg = NULL;

	switch (number_range(0, 5))
	{
		default:	msg = "You hear $n's death cry.";												break;
		case 0:		msg = "$n screams furiously as $e falls to the ground in a heap!";				break;
		case 1:		msg = "$n hits the ground ... DEAD.";											break;
		case 2:		msg = "$n catches $s guts in $s hands as they pour through $s fatal wound!";	break;
		case 3:		msg = "$n splatters blood on your armor.";										break;
		case 4:		msg = "$n gasps $s last breath and blood spurts out of $s mouth and ears.";		break;
		case 5:		
			shift = number_range(0, 31);
			cindex = 1 << shift;

			for(i = 0; i < 32 && ch->xflags; i++)
			{
				if(HAS_BODYPART(ch, cindex))
				{
					msg = part_messages[shift];
					vnum = part_vnums[shift];
					break;
				}
				else
				{
					shift = number_range(0, 31);
					cindex = 1 << shift;
				}
			}

			if(!msg) msg = "You hear $n's death cry."; break;
	}

	act(AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM);

	if(vnum)
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		const char *name;

		if(!get_obj_index(vnum)) 	{bug("%s: invalid vnum", __func__); return;}

		name = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj = create_object(get_obj_index( vnum), 0);
		obj->timer = number_range(4, 7);
		
		if(IS_AFFECTED(ch, AFF_POISON))	obj->value[3] = 10;

		snprintf(buf, MAX_STRING_LENGTH, obj->short_descr, name);
		STRFREE(obj->short_descr);
		obj->short_descr = STRALLOC(buf);

		snprintf(buf, MAX_STRING_LENGTH, obj->description, name);
		STRFREE(obj->description);
		obj->description = STRALLOC(buf);

		obj = obj_to_room(obj, ch->in_room);
	}

	if(IS_NPC(ch))	msg = "&WYou hear something's death cry.";
	else			msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	
	for(pexit = was_in_room->first_exit; pexit; pexit = pexit->next)
	{
		if(pexit->to_room && pexit->to_room != was_in_room)
		{
			ch->in_room = pexit->to_room;
			act(AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM);
		}
	}
	ch->in_room = was_in_room;
}
OBJ_DATA *raw_kill								(CHAR_DATA *ch, CHAR_DATA *victim){
	
	OBJ_DATA *corpse_to_return = NULL;

	if(!victim)	{bug("%s: null victim!", __func__);	return NULL;}

    //Backup in case hp goes below 1 
	if(NOT_AUTHED(victim))	{bug("%s: killing unauthed", __func__); return NULL;}

	stop_fighting( victim, TRUE );

	//Take care of morphed characters 
	if(victim->morph)
	{
		do_unmorph_char(victim);
		return raw_kill(ch, victim);
	}

	mprog_death_trigger(ch, victim);
	
	if(char_died(victim))
	return NULL;

	//death_cry(victim); 
	rprog_death_trigger(victim);
	
	if(char_died(victim))
    return NULL;

	corpse_to_return = make_corpse(victim, ch);
	
	if(victim->in_room->sector_type == SECT_OCEANFLOOR || victim->in_room->sector_type == SECT_UNDERWATER || victim->in_room->sector_type == SECT_WATER_SWIM || victim->in_room->sector_type == SECT_WATER_NOSWIM)
		{act(AT_BLOOD, "$n's blood slowly clouds the surrounding water.", victim, NULL, NULL, TO_ROOM);}
	else if(victim->in_room->sector_type == SECT_AIR)
		{act(AT_BLOOD, "$n's blood sprays wildly through the air.", victim, NULL, NULL, TO_ROOM);}
	else make_blood(victim);

	if(IS_NPC(victim))
	{
		victim->pIndexData->killed++;
		extract_char(victim, TRUE);
		victim = NULL;
		return corpse_to_return;
	}

	set_char_color(AT_DIEMSG, victim);
	
	if(victim->pcdata->mdeaths + victim->pcdata->pdeaths < 3)	do_help(victim, "new_death");
	else														do_help(victim, "_DIEMSG_");

	extract_char(victim, FALSE);
	
	if(!victim) {bug("%s: oops! extract_char destroyed pc char", __func__); return NULL;}
	
	//Remove every remaining buff before resetting the surviving player's stats.
	while(victim->first_affect)
	{
		affect_remove(victim, victim->first_affect);
	}
		victim->affected_by 	= race_table[victim->race]->affected;
		victim->resistant 		= 0;
		victim->susceptible 	= 0;
		victim->immune 			= 0;
		victim->carry_weight 	= 0;
		victim->armor 			= 100;
		victim->armor 		   += race_table[victim->race]->ac_plus;
		victim->attacks 		= race_table[victim->race]->attacks;
		victim->defenses 		= race_table[victim->race]->defenses;
		victim->mod_str 		= 0;
		victim->mod_dex 		= 0;
		victim->mod_wis 		= 0;
		victim->mod_int 		= 0;
		victim->mod_con 		= 0;
		victim->mod_cha 		= 0;
		victim->mod_lck 		= 0;
		victim->damroll 		= 0;
		victim->hitroll 		= 0;
		victim->mental_state 	= -10;
		
		victim->alignment 			= URANGE(-1000, victim->alignment, 1000);
		//victim->alignment			= race_table[victim->race]->alignment;
		victim->saving_poison_death = race_table[victim->race]->saving_poison_death;
		victim->saving_wand 		= race_table[victim->race]->saving_wand;
		victim->saving_para_petri 	= race_table[victim->race]->saving_para_petri;
		victim->saving_breath 		= race_table[victim->race]->saving_breath;
		victim->saving_spell_staff 	= race_table[victim->race]->saving_spell_staff;
		victim->position 			= POS_RESTING;
		victim->hit 				= UMAX( 1, victim->hit );

	//Shut down some of those naked spammer killers 
	if(victim->level < LEVEL_AVATAR)	victim->mana = UMAX(1, victim->mana);
	else								victim->mana = 1;
	
	victim->move = UMAX( 1, victim->move );


	//Pardon crimes...
	if(xIS_SET(victim->act, PLR_KILLER)) 
		{xREMOVE_BIT(victim->act, PLR_KILLER); send_to_char("The gods have pardoned you for your murderous acts.\r\n", victim);}
	
	if(xIS_SET(victim->act, PLR_THIEF))  
		{xREMOVE_BIT( victim->act, PLR_THIEF); send_to_char("The gods have pardoned you for your thievery.\r\n", victim);}
	
	victim->pcdata->condition[COND_FULL] 	= 12;
	victim->pcdata->condition[COND_THIRST] 	= 12;
	
	if(IS_VAMPIRE(victim)) {victim->pcdata->condition[COND_BLOODTHIRST] = (victim->level /2);}
	
	if(IS_SET(sysdata.save_flags, SV_DEATH)) {save_char_obj( victim);}
	
	return corpse_to_return;
}
void group_gain									(CHAR_DATA *ch, CHAR_DATA *victim){
	CHAR_DATA *gch, *gch_next;
	CHAR_DATA *lch;
	int xp;
	int members;

	//Monsters don't get kill xp's or alignment changes. Dying of mortal wounds or poison doesn't give xp to anyone!
	if(IS_NPC(ch) || victim == ch) return;

	members = 0;
	for(gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
	{
      if(is_same_group(gch, ch)) 	members++;
	}

	if(members == 0)
	{
		bug("%s: members %d", __func__, members);	members = 1;
	}

	lch = ch->leader ? ch->leader : ch;

	for( gch = ch->in_room->first_person; gch; gch = gch_next )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		gch_next = gch->next_in_room;

		if(!is_same_group(gch, ch))	continue;
		if( gch->level - lch->level > 8 ) 	{send_to_char("You are too high for this group.\r\n", gch);	continue;}
		if( gch->level - lch->level < -8 )	{send_to_char("You are too low for this group.\r\n", gch);  continue;}

		xp = (int)(xp_compute(gch, victim) * 0.1765) / members;
		if(!gch->fighting) {xp /= 2;}
		
		gch->alignment = align_compute(gch, victim);
		ch_printf(gch, "&WYou receive &Y%d&W experience points.&D\r\n", xp);
		gain_exp(gch, xp);

		for(obj = gch->first_carrying; obj; obj = obj_next)
		{
			obj_next = obj->next_content;
			if(obj->wear_loc == WEAR_NONE) continue;

			if((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(gch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(gch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch)))
			{
				act(AT_MAGIC, "You are zapped by $p.", 	gch, obj, NULL, TO_CHAR);
				act(AT_MAGIC, "$n is zapped by $p.", 	gch, obj, NULL, TO_ROOM);

				obj_from_char(obj);
				
				if(in_arena(ch)) 	obj = obj_to_char( obj, ch);
				else				obj = obj_to_room( obj, ch->in_room);
				
				oprog_zap_trigger(gch, obj);
				
				if(char_died(gch)) break;
			}
		}
	}
	return;
}
int align_compute								(CHAR_DATA *gch, CHAR_DATA *victim){
	int align, newalign, divalign;
	
	align = gch->alignment - victim->alignment;

	//Slowed movement in good & evil ranges by a factor of 5. (Added divalign to keep neutral chars shifting faster.)
	if(gch->alignment > -350 && gch->alignment < 350) 	divalign = 4;
	else												divalign = 20;

	if(align > 500)			newalign = UMIN(gch->alignment + (align -500) / divalign, +1000);
	else if(align < -500)	newalign = UMAX(gch->alignment + (align +500) / divalign, -1000);
	else					newalign = gch->alignment - (int)(gch->alignment / divalign);

	return newalign;
}
int xp_compute									(CHAR_DATA *gch, CHAR_DATA *victim){
//Calculate how much XP gch should gain for killing victim (Lots of redesigning for new exp system)
	int align;
	int xp;
	int xp_ratio;
	int gchlev = gch->level;
	
	xp = (get_exp_worth(victim) * URANGE(0, ( victim->level - gchlev) +10, 13) ) /10;
	align = gch->alignment - victim->alignment;

	//Bonus for attacking opposite alignment 
	     if(align > 990 || align < -990)		  	xp = (xp *5) >> 2;
	else if(gch->alignment > 300 && align < 250) 	xp = (xp *3) >> 2;
	else 											xp = number_range( (xp *3) >> 2, (xp *5) >> 2);

	     if(!IS_NPC(victim)) xp /= 4;
	else if(!IS_NPC(gch))
	{
		int times = times_killed( gch, victim );

		     if(times >= 20) xp = 0;
		else if(times)
		{
			xp = (xp * (20 - times)) /20;
			
			     if(times > 15) xp /= 3;
			else if(times > 10) xp >>= 1;
		}
	}

	//Semi-intelligent experienced player vs. novice player xp gain based on time played vs. level
	if(!IS_NPC(gch) && gchlev > 5)
	{
		xp_ratio = (int)gch->played / gchlev;
		
		     if(xp_ratio > 20000)  	xp = (xp *5 ) >> 2;
		else if(xp_ratio > 16000)	xp = (xp *3 ) >> 2;
		else if(xp_ratio > 10000) 	xp >>= 1;
		else if( xp_ratio > 5000)	xp >>= 2;
		else if( xp_ratio > 3500)	xp >>= 3;
		else if( xp_ratio > 2000)	xp >>= 4;
	}


	//Level based experience gain cap. Cannot get more experience for a kill than the amount for your current experience level.
	return URANGE(0, xp, exp_level(gch, gchlev + 1) - exp_level(gch, gchlev));
}
void new_dam_message							(CHAR_DATA *ch, CHAR_DATA *victim, int dam, unsigned int dt, OBJ_DATA * obj){
	char buf1[256], buf2[256], buf3[256];
	const char *vs;
	const char *vp;
	const char *attack;
	char punct;
	int dampc;
	struct skill_type *skill = NULL;
	bool gcflag = FALSE;
	bool gvflag = FALSE;
	int d_index, w_index;
	ROOM_INDEX_DATA *was_in_room;
	//WARNING: only the actual equipped attacking weapon is eligible for weapon text.
	obj = used_weapon;
	//Only identify an equipped weapon belonging to the actual attacker.
	if(obj && (obj->item_type != ITEM_WEAPON || obj->carried_by != ch ||
	   (obj->wear_loc != WEAR_WIELD && obj->wear_loc != WEAR_DUAL_WIELD && obj->wear_loc != WEAR_HOLD))) obj = NULL;

	if(!dam) 	dampc = 0;
	else		dampc = ((dam *1000) / UMAX(1, victim->max_hit)) + (50 - ((victim->hit *50) / UMAX(1, victim->max_hit)));

	if(ch->in_room != victim->in_room)
	{
		was_in_room = ch->in_room;
		char_from_room( ch );
		char_to_room( ch, victim->in_room );
	}
	else was_in_room = NULL;

	//MESSAGE LOOKUP: translate dt to standard attack or skill-specific text.
	//Get the weapon index 
	     if(dt < (unsigned int)num_skills) 														{w_index = 0;}
	else if(dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0]))	{w_index = dt - TYPE_HIT;}
	else
	{
		bug( "%s: bad dt %d from %s in %d.", __func__, dt, ch->name, ch->in_room->vnum );
		dt = TYPE_HIT;
		w_index = 0;
	}

	//Get the damage index 
	     if(dam == 0) 		d_index = 0;
	else if(dampc < 0)		d_index = 1;
	else if(dampc <= 100)	d_index = 1 + dampc /10;
	else if(dampc <= 200)	d_index = 11 + (dampc -100) /20;
	else if(dampc <= 900)	d_index = 16 + (dampc -200 ) /100;
	else					d_index = 23;

	//Lookup the damage message.
	vs = s_message_table[w_index][d_index];
	vp = p_message_table[w_index][d_index];

	int hpcnt = URANGE(0, ((victim->hit - dam) * 100) / UMAX(1, victim->max_hit), 100); 
	punct = (dampc <= 30) ? '.' : '!';

	if(dam == 0 && (!IS_NPC(ch) && (IS_SET(ch->pcdata->flags, PCFLAG_GAG))))			gcflag = TRUE;
	if(dam == 0 && (!IS_NPC( victim) && (IS_SET(victim->pcdata->flags, PCFLAG_GAG))))	gvflag = TRUE;
	if(dt >= 0 && dt < (unsigned int )num_skills)										skill = skill_table[dt];

	//A normal missed weapon strike can fail in several believable ways.
	if(dam == 0 && dt >= TYPE_HIT && IS_AWAKE(victim) && number_percent() <= 75)
	{
		switch(number_range(1, 4))
		{
			case 1:
				snprintf(buf1, 256, "&Y$n attacks $N, but the blow goes wide.&D");
				snprintf(buf2, 256, "&GYou attack $N, but your blow goes wide.&D");
				snprintf(buf3, 256, "&R$n attacks you, but the blow goes wide.&D");
				break;
			case 2:
				snprintf(buf1, 256, "&Y$n fails to find an opening against $N.&D");
				snprintf(buf2, 256, "&GYou fail to find an opening against $N.&D");
				snprintf(buf3, 256, "&R$n fails to find an opening against you.&D");
				break;
			case 3:
				snprintf(buf1, 256, "&Y$n strikes at $N, but the attack fails to connect.&D");
				snprintf(buf2, 256, "&GYou strike at $N, but fail to connect.&D");
				snprintf(buf3, 256, "&R$n strikes at you, but fails to connect.&D");
				break;
			default:
				snprintf(buf1, 256, "&Y$n's attack misses $N by inches.&D");
				snprintf(buf2, 256, "&GYour attack misses $N by inches.&D");
				snprintf(buf3, 256, "&R$n's attack misses you by inches.&D");
				break;
		}

					act(AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT);
		if(!gcflag)	act(AT_HIT,    buf2, ch, NULL, victim, TO_CHAR);
		if(!gvflag)	act(AT_HITME,  buf3, ch, NULL, victim, TO_VICT);
		if(was_in_room) {char_from_room(ch); char_to_room(ch, was_in_room);}
		return;
	}

	if(dreamwalk_reflect_magic_damage)
	{
		snprintf(buf1, 256, "&Y$n reflects magic damage against $N for %d points.&D", dam);
		snprintf(buf2, 256, "&GYou reflect magic damage against $N for %d points.\r\n&r$N's condition: %d%%&D", dam, hpcnt);
		snprintf(buf3, 256, "&R$n reflects your spell damage against you for %d points.&D", dam);
	}
	else if(dreamwalk_shield_retaliation)
	{
		const char *shield = "magic";
		     if(dt == tes_effect_damage_fire)   shield = "fire";
		else if(dt == tes_effect_damage_frost)  shield = "frost";
		else if(dt == tes_effect_damage_shock)  shield = "shock";
		else if(dt == tes_effect_damage_poison) shield = "poison";
		else if(dt == tes_effect_drain_health)  shield = "health-draining";

		snprintf(buf1, 256, "&Y$n's %s shield retaliates against $N for %d points.&D", shield, dam);
		snprintf(buf2, 256, "&GYour %s shield retaliates against $N for %d points.\r\n&r$N's condition: %d%%&D", shield, dam, hpcnt);
		snprintf(buf3, 256, "&R$n's %s shield retaliates against you for %d points.&D", shield, dam);
	}
	else if(dt == TYPE_HIT)
	{
		//TYPE_HIT is also used by weapons whose damage type is zero.
		//Show their actual name; retain the original wording for bare hands.
		if(obj && obj->short_descr && obj->short_descr[0] != '\0')
		{
			snprintf(buf1, 256, "&Y$n %s $N with %s for %d points.&D", vp, obj->short_descr, dam);
			snprintf(buf2, 256, "&GYou %s $N with %s for %d points.\r\n&r$N's condition: %d%%&D", vs, obj->short_descr, dam, hpcnt);
			snprintf(buf3, 256, "&R$n %s you with %s for %d points.&D", vp, obj->short_descr, dam);
		}
		else
		{
			snprintf(buf1, 256, "&Y$n %s $N for %d points.&D", vp, dam);
			snprintf(buf2, 256, "&GYou %s $N for %d points.\r\n&r$N's condition: %d%%&D", vs, dam, hpcnt);
			snprintf(buf3, 256, "&R$n %s you for %d points.&D", vp, dam);
		}
	}
	else if(dt > TYPE_HIT && is_wielding_poisoned(ch))
	{
		if(obj && obj->short_descr && obj->short_descr[0] != '\0')
			attack = obj->short_descr;
		else if(dt < TYPE_HIT + sizeof(attack_table) / sizeof( attack_table[0]))	
			attack = attack_table[dt - TYPE_HIT];
		else
		{
			bug("Dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum);
			dt = TYPE_HIT;
			attack = attack_table[0];
		}

		if(attack && attack[0] != '\0')
		{
			snprintf(buf1, 256, "&Y$n %s $N with poisoned %s for %d points%c&D", vp, attack, dam, punct);
			snprintf(buf2, 256, "&GYou %s $N with poisoned %s for %d points%c\r\n&r$N's condition: %d%%&D", vs, attack, dam, punct, hpcnt);
			snprintf(buf3, 256, "&R$n %s you with poisoned %s for %d points%c&D", vp, attack, dam, punct);
		}
		else
		{
			snprintf(buf1, 256, "&Y$n %s $N with a poisoned weapon for %d points%c&D", vp, dam, punct);
			snprintf(buf2, 256, "&GYou %s $N with a poisoned weapon for %d points%c\r\n&r$N's condition: %d%%&D", vs, dam, punct, hpcnt);
			snprintf(buf3, 256, "&R$n %s you with a poisoned weapon for %d points%c&D", vp, dam, punct);
		}
	}
	else
	{
		if(skill)
		{
			attack = skill->noun_damage;
			if(dam == 0)
			{
				bool found = FALSE;

				if( skill->miss_char && skill->miss_char[0] != '\0' )
				{
					act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
					found = TRUE;
				}
				
				if(skill->miss_vict && skill->miss_vict[0] != '\0')
				{	
					act(AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT);
					found = TRUE;
				}
				
				if(skill->miss_room && skill->miss_room[0] != '\0')
				{
					if(str_cmp( skill->miss_room, "supress")) act(AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT);
					found = TRUE;
				}
				
				if(found) // miss message already sent
				{
					if(was_in_room)
					{
						char_from_room(ch);
						char_to_room(ch, was_in_room);
					}
					return;
				}
			}
			else
			{
				if(skill->hit_char && skill->hit_char[0] != '\0' ) act(AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR);
				if(skill->hit_vict && skill->hit_vict[0] != '\0' ) act(AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT);
				if(skill->hit_room && skill->hit_room[0] != '\0' ) act(AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT);
			}
		}
		else if(dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) /sizeof( attack_table[0]))
		{
			if(obj)	attack = obj->short_descr;
			else	attack = attack_table[dt - TYPE_HIT];
		}
		else
		{
			bug("Dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum);
			attack = attack_table[0];
		}
		
		if(attack && attack[0] != '\0')
		{
			snprintf(buf1, 256, "&Y$n %s $N with %s for %d points.&D", vp, attack, dam);
			snprintf(buf2, 256, "&GYou %s $N with %s for %d points.\r\n&r$N's condition: %d%%&D", vs, attack, dam, hpcnt);
			snprintf(buf3, 256, "&R$n %s you with %s for %d points.&D", vp, attack, dam);
		}
		else
		{
			snprintf(buf1, 256, "&Y$n %s $N for %d points.&D", vp, dam);
			snprintf(buf2, 256, "&GYou %s $N for %d points.\r\n&r$N's condition: %d%%&D", vs, dam, hpcnt);
			snprintf(buf3, 256, "&R$n %s you for %d points.&D", vp, dam);
		}
	}

				act(AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT);
	if(!gcflag)	act(AT_HIT,    buf2, ch, NULL, victim, TO_CHAR);
	if(!gvflag)	act(AT_HITME,  buf3, ch, NULL, victim, TO_VICT);

	//Only truly significant hits get an extra line; avoid flooding multi-hit rounds.
	if(dam > 0 && dam >= UMAX(1, victim->max_hit / 4) && number_percent() <= 25)
	{
		act(AT_ACTION, "&OThe impact echoes through the clash of combat!&D", ch, NULL, victim, TO_NOTVICT);
		act(AT_HIT,    "&OYour attack lands with tremendous force!&D", ch, NULL, victim, TO_CHAR);
		act(AT_HITME,  "&RThe impact leaves you reeling!&D", ch, NULL, victim, TO_VICT);
	}

	if(was_in_room)
	{
		char_from_room(ch);
		char_to_room(ch, was_in_room);
	}
}
#ifndef dam_message
//--------------------------------------------------------------------------------------------------//
// FUNCTION: dam_message
// PURPOSE : Routes a combat event into the detailed damage-message formatter.
// EDITING : Do not cause additional damage here; text generation should be separate.
//--------------------------------------------------------------------------------------------------//
void dam_message				(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) 	{
	new_dam_message( ch, victim, dam, dt, used_weapon );
}
#endif
void do_kill									(CHAR_DATA *ch, const char* argument){
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument(argument, arg);

	if(arg[0] == '\0')							  
		{act(AT_BLOOD, "Kill whom?", ch, NULL, NULL, TO_CHAR); return;}

	if((victim = get_char_room(ch, arg)) == NULL) 
		{act(AT_BLOOD, "They aren't here.", ch, NULL, NULL, TO_CHAR); return;}

	if(IS_NPC(victim) && victim->morph)			  
		{act(AT_BLOOD, "This creature appears strange to you. Look upon it more closely before attempting to kill it.", ch, NULL, NULL, TO_CHAR); return;}
	
	if(!IS_NPC(victim))
	{
		if(!xIS_SET( victim->act, PLR_KILLER) && !xIS_SET(victim->act, PLR_THIEF)) 
			{act(AT_BLOOD, "You must MURDER a player.", ch, NULL, NULL, TO_CHAR); return;}
	}
	
	if(victim == ch)
	{
		act(AT_BLOOD, "You hit yourself. Ouch!", ch, NULL, NULL, TO_CHAR);
		multi_hit(ch, ch, TYPE_UNDEFINED);
		return;
	}

	if(is_safe(ch, victim, TRUE)) return;
	if(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) 
		{act(AT_BLOOD, "$N is your beloved master.", ch, NULL, victim, TO_CHAR); return;}

	if(ch->position == POS_FIGHTING || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
		{act(AT_BLOOD, "You do the best you can!", ch, NULL, NULL, TO_CHAR); return;}

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	check_attacker(ch, victim);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}
void do_murde									(CHAR_DATA *ch, const char *argument){
	act(AT_BLOOD, "If you want to MURDER, spell it out.", ch, NULL, NULL, TO_CHAR);
	return;
}
void do_murder									(CHAR_DATA *ch, const char *argument){
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if(arg[0] == '\0') 							  	{act(AT_BLOOD, "Murder whom?"			 , ch, NULL, NULL, TO_CHAR); return;}
	if((victim = get_char_room(ch, arg)) == NULL) 	{act(AT_BLOOD, "They aren't here."		 , ch, NULL, NULL, TO_CHAR); return;}
	if(victim == ch)								{act(AT_BLOOD, "Suicide is a mortal sin.", ch, NULL, NULL, TO_CHAR); return;}
	if(is_safe(ch, victim, TRUE)) return;
	if(IS_AFFECTED(ch, AFF_CHARM))
	{
		if(ch->master == victim) {act(AT_BLOOD, "$N is your beloved master.", ch, NULL, victim, TO_CHAR); return;}
		else				     {if(ch->master) xSET_BIT( ch->master->act, PLR_ATTACKER);}
	}

	if(ch->position == POS_FIGHTING  || ch->position == POS_EVASIVE 
		|| ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE 
		|| ch->position == POS_BERSERK)										{act(AT_BLOOD, "You do the best you can!"     , ch, NULL, NULL, TO_CHAR); return;}
	if(!IS_NPC(victim) && xIS_SET(ch->act, PLR_NICE))						{act(AT_BLOOD, "You feel too nice to do that!", ch, NULL, NULL, TO_CHAR); return;}

    //if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_PK))

	if(!IS_NPC(victim)) {log_printf_plus(LOG_NORMAL, ch->level, "%s: murder %s.", ch->name, victim->name);}

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	snprintf(buf, MAX_STRING_LENGTH, "Help! I am being attacked by %s!", IS_NPC(ch) ? ch->short_descr : ch->name);
	
	if(IS_PKILL(victim)) do_wartalk(victim, buf);
	else				 do_yell(victim, buf);
	
	check_illegal_pk(ch, victim);
	check_attacker(ch, victim);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}
bool in_arena									(CHAR_DATA *ch){
//Check to see if the player is in an "Arena".	
	if(xIS_SET(ch->in_room->room_flags, ROOM_ARENA)) 		return TRUE;
	if(IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL))	return TRUE;
	if(ch->in_room->vnum >= 29 && ch->in_room->vnum <= 43)	return TRUE;
	if(!str_cmp( ch->in_room->area->filename, "arena.are"))	return TRUE;
	return FALSE;
}
bool check_illegal_pk							(CHAR_DATA *ch, CHAR_DATA *victim){
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char log_buf[MAX_STRING_LENGTH];

	if(!IS_NPC(victim) && !IS_NPC(ch))
	{
		if((!IS_SET( victim->pcdata->flags, PCFLAG_DEADLY) || ch->level - victim->level > 10 || 
		!IS_SET( ch->pcdata->flags, PCFLAG_DEADLY)) && !in_arena(ch) && ch != victim && !(IS_IMMORTAL(ch) && IS_IMMORTAL(victim)))
		{
			if(IS_NPC(ch))     {snprintf(buf1, MAX_STRING_LENGTH, " (%s)", ch->name);}
			if(IS_NPC(victim)) {snprintf(buf2, MAX_STRING_LENGTH, " (%s)", victim->name );}

			snprintf(log_buf, MAX_STRING_LENGTH, "&p%s on %s%s in &W***&rILLEGAL PKILL&W*** &pattempt at %d",
				(lastplayercmd), (IS_NPC(victim) ? victim->short_descr : victim->name), (IS_NPC(victim) ? buf2 : ""), victim->in_room->vnum);
			
			last_pkroom = victim->in_room->vnum;
			log_string(log_buf);
			to_channel(log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
			return TRUE;
		}
	}
	return FALSE;
}
void do_flee									(CHAR_DATA *ch, const char *argument){
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	char buf[MAX_STRING_LENGTH];
	int attempt, los;
	short door;
	EXIT_DATA *pexit;

	if(!who_fighting(ch))
	{
		if(ch->position == POS_FIGHTING || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
		{
			if(ch->mount)	ch->position = POS_MOUNTED;
			else			ch->position = POS_STANDING;
		}
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't fighting anyone."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	if(IS_AFFECTED(ch, AFF_BERSERK)) 				{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Flee while berserking?  You aren't thinking very clearly..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 	return;}
	if(IS_AFFECTED(ch, AFF_GRAPPLE))				{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're too wrapped up to flee!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}								return;}
	if(ch->move <= 0)								{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You're too exhausted to flee from combat!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}						return;}
	if(!IS_NPC(ch) && ch->position < POS_FIGHTING)	{send_to_char("You can't flee in an aggressive stance...\r\n", ch);						return;}
	if( IS_NPC(ch) && ch->position <= POS_SLEEPING)																							return;

	was_in = ch->in_room;
	for(attempt = 0; attempt < 8; attempt++)
	{
		door = number_door();
		if((pexit = get_exit(was_in, door)) == NULL
          || !pexit->to_room
          || IS_SET(pexit->exit_info, EX_NOFLEE)
          || (IS_PKILL(ch) && xIS_SET(pexit->to_room->room_flags, ROOM_DEATH))
          || (IS_SET(pexit->exit_info, EX_CLOSED) && !IS_AFFECTED( ch, AFF_PASS_DOOR))
          || (IS_NPC(ch) && xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
        continue;
		 
		affect_strip(ch, tes_skill_move_silently);
		xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
		
		if(ch->mount && ch->mount->fighting) {stop_fighting(ch->mount, TRUE);}
		move_char(ch, pexit, 0);
		
		if((now_in = ch->in_room) == was_in)
        continue;
	
		ch->in_room = was_in;	act(AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;	act(AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);
		
		if(!IS_NPC(ch))
		{
			CHAR_DATA *wf = who_fighting(ch);
			act(AT_FLEE, "You flee head over heels from combat!", ch, NULL, NULL, TO_CHAR);
			los = 50;
			
			if(ch->level < LEVEL_AVATAR)
			{
				if(!IS_PKILL(ch))
				{
					if(ch->level > 1)
					{
						snprintf(buf, MAX_STRING_LENGTH, "&WCurse the gods, you've lost &Y%d&W experience!&D", los);
						act(AT_FLEE, buf, ch, NULL, NULL, TO_CHAR);
						gain_exp(ch, 0 - los);
					}
				}
			}

			if(wf && ch->pcdata->deity)
			{
				int level_ratio = URANGE(1, wf->level / ch->level, MAX_LEVEL);

				if(wf && wf->race == ch->pcdata->deity->npcrace)	 {adjust_favor(ch, 1,  level_ratio);}
				else if(wf && wf->race == ch->pcdata->deity->npcfoe) {adjust_favor(ch, 16, level_ratio);}
				else												 {adjust_favor(ch, 0,  level_ratio);}
			}
		}
		stop_fighting(ch, TRUE);
		return;
	}

	los = 50;
	act(AT_FLEE, "You attempt to flee from combat but can't escape!", ch, NULL, NULL, TO_CHAR);

	if( ch->level < LEVEL_AVATAR && number_bits( 3 ) == 1 )
	{
		if(!IS_PKILL(ch))
		{
			if(ch->level > 1)
			{
				snprintf(buf, MAX_STRING_LENGTH, "&WCurse the gods, you've lost &Y%d&W experience!&D\n\r", los);
				act(AT_FLEE, buf, ch, NULL, NULL, TO_CHAR);
				gain_exp(ch, 0 - los);
			}
		}
	}
	return;
}
void do_sla										(CHAR_DATA *ch, const char *argument){
	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "If you want to SLAY, spell it out."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	return;
}
void do_slay									(CHAR_DATA *ch, const char *argument){
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);
	
	if(arg[0] == '\0') 											{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Slay whom?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 				return;}
	if((victim = get_char_room(ch, arg)) == NULL)				{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}			return;}
	if(ch == victim)											{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Suicide is a mortal sin."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 	return;}
	if(!IS_NPC(victim) && get_trust(victim) >= get_trust(ch)) 	{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You failed."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}				return;}

	if(!str_cmp(arg2, "immolate"))
	{
		act(AT_FIRE, "Your fireball turns $N into a blazing inferno.", 		ch, NULL, victim, TO_CHAR);
		act(AT_FIRE, "$n releases a searing fireball in your direction.", 	ch, NULL, victim, TO_VICT);
		act(AT_FIRE, "$n points at $N, who bursts into a flaming inferno.", ch, NULL, victim, TO_NOTVICT);
	}
	else if(!str_cmp(arg2, "shatter"))
	{
		act(AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR);
		act(AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT);
		act(AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",   ch, NULL, victim, TO_NOTVICT);
	}
	else if(!str_cmp( arg2, "demon"))
	{
		act(AT_WHITE, "You gesture, and a slavering demon appears. With a horrible grin, the",    ch, NULL, victim, TO_CHAR);
		act(AT_WHITE, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,TO_CHAR);
		act(AT_WHITE, "$n gestures, and a slavering demon appears. The foul creature turns on",   ch, NULL, victim, TO_VICT);
		act(AT_WHITE, "you with a horrible grin.   You scream in panic before being eaten alive.", ch, NULL, victim,TO_VICT);
		act(AT_WHITE, "$n gestures, and a slavering demon appears. With a horrible grin, the",    ch, NULL, victim,TO_NOTVICT);
		act(AT_WHITE, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,TO_NOTVICT);
	}
	else if(!str_cmp(arg2, "pounce"))
	{
		act(AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...", 							     ch, NULL, victim, TO_CHAR);
		act(AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat! Your blood sprays and pours to the ground as your life ends...", 			 ch, NULL, victim, TO_VICT);
		act(AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat. As blood sprays and gushes to the ground, $n tosses $N's dying body away.", ch, NULL, victim, TO_NOTVICT);
	}
	else if(!str_cmp(arg2, "slit"))
	{
		act(AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR);
		act(AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT);
		act(AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT);
	}
	else if(!str_cmp(arg2, "dog"))
	{
		act(AT_BLOOD, "You order your dogs to rip $N to shreds.", ch, NULL, victim, TO_CHAR);
		act(AT_BLOOD, "$n orders $s dogs to rip you apart.", 	  ch, NULL, victim, TO_VICT);
		act(AT_BLOOD, "$n orders $s dogs to rip $N to shreds.",   ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
		act(AT_WHITE, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR);
		act(AT_WHITE, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
		act(AT_WHITE, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);
	}
	set_cur_char(victim);
	raw_kill(ch, victim);
	return;
}
