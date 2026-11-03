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
 *                           Regular update module                          *
 ****************************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include "mud.h"
#include "hint.h"
#include <string.h>

//######################################################################################//
// LOCAL VARIABLES														  				//
//######################################################################################//
void DREAMWALK_START		(void);
int hit_gain				(CHAR_DATA *ch);
int mana_gain				(CHAR_DATA *ch);
int move_gain				(CHAR_DATA *ch);
void mobile_update			(void);
void time_update			(void);
void char_update			(void);
void obj_update				(void);
void aggr_update			(void);
void room_act_update		(void);
void obj_act_update			(void);
void char_check				(void);
void drunk_randoms			(CHAR_DATA *ch);
void hallucinations			(CHAR_DATA *ch);
void subtract_times			(struct timeval *etime, struct timeval *sttime);
int create_virtual_mob		(CHAR_DATA *ch);
bool check_social			(CHAR_DATA *ch, const char *command, const char *argument );
void homebuy_update			();
//######################################################################################//
// GLOBAL VARIABLES														  				//
//######################################################################################//
CHAR_DATA *timechar;
extern int top_exit;
//######################################################################################//
// CORE TABLES														  					//
//######################################################################################//
const char *corpse_descs[] = {
	"The corpse of %s is in the last stages of decay.",
	"The corpse of %s is crawling with vermin.",
	"The corpse of %s fills the air with a foul stench.",
	"The corpse of %s is buzzing with flies.",
	"The corpse of %s lies here."
};
//######################################################################################//
// CORE SMAUG FUNCTIONS														  			//
//######################################################################################//
void advance_level								(CHAR_DATA *ch){
	char buf[MAX_STRING_LENGTH];
	int add_hp, add_mana, add_move, add_prac;

	snprintf( buf, MAX_STRING_LENGTH, "the %s", title_table[ch->Class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0] );
	set_title( ch, buf );

	add_hp 	 = ch->char_health / 10;
	add_mana = ch->char_magicka/ 10;
	add_move = ch->char_fatigue/ 10;
	add_prac = wis_app[get_curr_wis(ch)].practice;

	add_hp   = UMAX(1 , add_hp  );
	add_mana = UMAX(0 , add_mana);
	add_move = UMAX(10, add_move);

    //Bonus for deadlies//
	if( IS_PKILL( ch ) )
	{
		add_mana = (int)(add_mana + add_mana * .3);
		add_move = (int)(add_move + add_move * .3);
		add_hp += 1;
		send_to_char("Gravoc's Pandect steels your sinews.\r\n", ch);
	}

	ch->max_hit  += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	ch->practice += add_prac;

	if(!IS_NPC(ch))
    xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );

	if(ch->level == LEVEL_AVATAR)
	{
		DESCRIPTOR_DATA *d;

		for(d = first_descriptor; d; d = d->next)
			if(d->connected == CON_PLAYING && d->character != ch)
			{
				set_char_color(AT_WHITE, d->character);
				ch_printf(d->character, "%s has attained the rank of Avatar!\r\n", ch->name);
			}
		set_char_color(AT_WHITE, ch);
		do_help(ch, "M_ADVHERO_");
	}

	if(ch->level < LEVEL_IMMORTAL)
	{
		if( IS_VAMPIRE(ch))
			snprintf(buf, MAX_STRING_LENGTH, "Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\r\n",
				add_hp, ch->max_hit, 1, ch->level + 10, add_move, ch->max_move, add_prac, ch->practice);
		else
			snprintf(buf, MAX_STRING_LENGTH, "Your gain is: &R%d/%d hp&D, &B%d/%d mana&D, &G%d/%d mv&D, &Y%d/%d practice points&D.\r\n",
                add_hp, ch->max_hit, add_mana, ch->max_mana, add_move, ch->max_move, add_prac, ch->practice);
				
		ch->gold = ch->gold + 100;
		send_to_char("&WYou receive &Y100 &Wgold&D\r\n", ch);
		set_char_color(AT_WHITE, ch);
		send_to_char(buf, ch);
	}

	//Gain extra attacks//
    if(ch->level == 40) {ch->pcdata->learned[gsn_fifth_attack]  += 100;}
    if(ch->level == 30) {ch->pcdata->learned[gsn_fourth_attack] += 100;}
    if(ch->level == 20) {ch->pcdata->learned[gsn_third_attack]  += 100;}
    if(ch->level == 10) {ch->pcdata->learned[gsn_second_attack] += 100;}
}
void gain_exp									(CHAR_DATA *ch, int gain){
	double modgain;
	if(IS_NPC(ch) || ch->level >= LEVEL_AVATAR) return;

	//Bonus for deadly lowbies 
	modgain = gain;
	if(modgain > 0 && IS_PKILL(ch) && ch->level < 17)
	{
		if(ch->level <= 6) 						{send_to_char("The Favor of Gravoc fosters your learning.\r\n"		 , ch); modgain *= 2;	}
		if(ch->level <= 10 && ch->level >= 7)	{send_to_char("The Hand of Gravoc hastens your learning.\r\n"		 , ch); modgain *= 1.75;}
		if(ch->level <= 13 && ch->level >= 11)	{send_to_char("The Cunning of Gravoc succors your learning.\r\n"	 , ch); modgain *= 1.5;	}
		if(ch->level <= 16 && ch->level >= 14)	{send_to_char("The Patronage of Gravoc reinforces your learning.\r\n", ch);	modgain *= 1.25;}
	}

	//Per-race experience multipliers 
	modgain *= (race_table[ch->race]->exp_multiplier /100.0);

	//Deadly exp loss floor is exp floor of level.
	if(IS_PKILL(ch) && modgain < 0)
	{
		if(ch->exp + modgain < exp_level(ch, ch->level))
		{
			modgain = exp_level(ch, ch->level) - ch->exp;
			send_to_char("Gravoc's Pandect protects your insight.\r\n", ch);
		}
	}

	if(IS_PKILL(ch))  modgain = (modgain * sysdata.deadly_exp_mod)   /100;
	else			  modgain = (modgain * sysdata.peaceful_exp_mod) /100;

	//Exp cap to prevent any one event from giving enuf xp to gain more than one level. 
	modgain = UMIN((int)modgain, exp_level(ch, ch->level +2) - exp_level(ch, ch->level + 1));
	ch->exp = UMAX(0, ch->exp + (int)modgain);

	if(NOT_AUTHED(ch) && ch->exp >= exp_level(ch, ch->level + 1))
	{
		send_to_char("You can not ascend to a higher level until you are authorized.\r\n", ch);
		ch->exp = (exp_level(ch, (ch->level + 1)) - 1);
		return;
	}

	while(ch->level < LEVEL_AVATAR && ch->exp >= exp_level(ch, ch->level + 1))
	{
		set_char_color(AT_WHITE, ch);
		ch->level += 1;
		ch_printf( ch,"You have now obtained experience level %d!\r\n", ch->level);
		advance_level(ch);
	}
	save_char_obj(ch);
}
int hit_gain									(CHAR_DATA *ch){
	int gain;

	if(IS_NPC(ch))
	{
		gain = ch->level *3 /2;
	}
	else
	{
		gain = UMIN(5, ch->level);

		switch (ch->position)
		{
			case POS_DEAD:		return  0;
			case POS_MORTAL:	return -1;
			case POS_INCAP:		return -1;
			case POS_STUNNED:	return  1;
			case POS_SLEEPING:	gain += (int)(get_curr_con(ch) *2.0); 	break;
			case POS_RESTING:	gain += (int)(get_curr_con(ch) *1.25); 	break;
		}

		if(IS_VAMPIRE(ch))
		{
			if(ch->pcdata->condition[COND_BLOODTHIRST] <= 1)						gain /= 2;
			else if(ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level))		gain *= 2;
			
			if(IS_OUTSIDE(ch))
			{
				switch (time_info.sunlight)
				{
					case SUN_RISE:
					case SUN_SET:	gain /= 2;	break;
					case SUN_LIGHT:	gain /= 4;	break;
				}
			}
		}

		if(ch->pcdata->condition[COND_FULL]   == 0)	gain /= 2;
		if(ch->pcdata->condition[COND_THIRST] == 0)	gain /= 2;

	}

	if(IS_AFFECTED( ch, AFF_POISON))	
	gain /= 4;

	return UMIN(gain, ch->max_hit - ch->hit);
}
int mana_gain									(CHAR_DATA *ch){
	int gain;

	if(IS_NPC(ch))
	{
		gain = ch->level;
	}
	else
	{
		gain = UMIN(5, ch->level /2);

		if(ch->position < POS_SLEEPING)	return 0;
		
		switch ( ch->position )
		{
			case POS_SLEEPING:	gain += (int)(get_curr_int(ch) *3.25);	break;
			case POS_RESTING:	gain += (int)(get_curr_int(ch) *1.75);	break;
		}

		if(ch->pcdata->condition[COND_FULL]   == 0)	gain /= 2;
		if(ch->pcdata->condition[COND_THIRST] == 0) gain /= 2;
	}

	if(IS_AFFECTED(ch, AFF_POISON))
    gain /= 4;

	return UMIN(gain, ch->max_mana - ch->mana);
}
int move_gain									(CHAR_DATA *ch){
	int gain;

	if(IS_NPC(ch))
	{
		gain = ch->level;
	}
   else
   {
		gain = UMAX(15, 2 * ch->level);

		switch (ch->position)
		{
			case POS_DEAD:		return  0;
			case POS_MORTAL:	return -1;
			case POS_INCAP:		return -1;
			case POS_STUNNED:	return  1;
			case POS_SLEEPING:	gain += (int)(get_curr_dex(ch) *4.5);	break;
			case POS_RESTING:	gain += (int)(get_curr_dex(ch) *2.5);	break;
		}

		if(IS_VAMPIRE(ch))
		{
			if(ch->pcdata->condition[COND_BLOODTHIRST] <= 1)						gain /= 2;
			else if(ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level))		gain *= 2;
			
			if(IS_OUTSIDE(ch))
			{
				switch (time_info.sunlight)
				{
					case SUN_RISE:
					case SUN_SET:	gain /= 2; 	break;
					case SUN_LIGHT:	gain /= 4;	break;
				}
			}
		}

		if(ch->pcdata->condition[COND_FULL]   == 0)		gain /= 2;
		if(ch->pcdata->condition[COND_THIRST] == 0)		gain /= 2;
	}

	if(IS_AFFECTED( ch, AFF_POISON))
    gain /= 4;

	return UMIN(gain, ch->max_move - ch->move);
}
void gain_condition								(CHAR_DATA *ch, int iCond, int value){
	int condition;
	ch_ret retcode = rNONE;

	if(value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED(ch))
    return;

	condition = ch->pcdata->condition[iCond];
	
	if(iCond == COND_BLOODTHIRST)	ch->pcdata->condition[iCond] = URANGE(0, condition + value, 10 + ch->level);
	else							ch->pcdata->condition[iCond] = URANGE(0, condition + value, 99);

	if(ch->pcdata->condition[iCond] == 0)
	{
		switch (iCond)
		{
			case COND_FULL:
				if( ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color(AT_HUNGRY, ch);
					send_to_char("You are STARVING!\r\n", ch);
					act(AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
					
					if(!IS_PKILL(ch) || number_bits(1) == 0)	
					worsen_mental_state(ch, 1);
				
					retcode = damage(ch, ch, 1, TYPE_UNDEFINED);
            } break;

			case COND_THIRST:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_THIRSTY, ch);
					send_to_char("You are DYING of THIRST!\r\n", ch);
					act(AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
					
					worsen_mental_state(ch, IS_PKILL(ch) ? 1 : 2);
					
					retcode = damage(ch, ch, 2, TYPE_UNDEFINED);
				} break;

			case COND_BLOODTHIRST:
				if(ch->level < LEVEL_AVATAR)
				{
					set_char_color(AT_BLOOD, ch);
					send_to_char("You are starved to feast on blood!\r\n", ch);
					act(AT_BLOOD, "$n is suffering from lack of blood!", ch, NULL, NULL, TO_ROOM);
					
					worsen_mental_state(ch, 2);
					
					retcode = damage(ch, ch, ch->max_hit /20, TYPE_UNDEFINED);
				} break;
			case COND_DRUNK:
				if(condition != 0)
				{
					set_char_color(AT_SOBER, ch);
					send_to_char("You are sober.\r\n", ch);
				}
				retcode = rNONE;
				break;
			default:
				bug("Gain_condition: invalid condition type %d", iCond);
				retcode = rNONE;
				break;
		}
	}

	if(retcode != rNONE)
    return;

	if(ch->pcdata->condition[iCond] == 1)
	{
		switch (iCond)
		{
			case COND_FULL:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_HUNGRY, ch);
					send_to_char("You are really hungry.\r\n", ch);
					act(AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
					
					if(number_bits(1) == 0)
					worsen_mental_state(ch, 1);
				} break;

			case COND_THIRST:
				if( ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color(AT_THIRSTY, ch);
					send_to_char("You are really thirsty.\r\n", ch);
					worsen_mental_state( ch, 1 );
					act(AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
				} break;

			case COND_BLOODTHIRST:
				if(ch->level < LEVEL_AVATAR)
				{
					set_char_color(AT_BLOOD, ch);
					send_to_char("You have a growing need to feast on blood!\r\n", ch);
					act( AT_BLOOD, "$n gets a strange look in $s eyes...", ch, NULL, NULL, TO_ROOM);
					worsen_mental_state(ch, 1);
				} break;
				
			case COND_DRUNK:
				if(condition != 0)
				{
					set_char_color(AT_SOBER, ch);
					send_to_char("You are feeling a little less light headed.\r\n", ch);
				} break;
		}
	}

	if( ch->pcdata->condition[iCond] == 2 )
	{
		switch (iCond)
		{
			case COND_FULL:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_HUNGRY, ch);
					send_to_char("You are hungry.\r\n", ch);
				} break;

			case COND_THIRST:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_THIRSTY, ch);
					send_to_char("You are thirsty.\r\n", ch);
				} break;

			case COND_BLOODTHIRST:
				if(ch->level < LEVEL_AVATAR)
				{
					set_char_color(AT_BLOOD, ch);
					send_to_char("You feel an urgent need for blood.\r\n", ch);
				} break;
		}
	}

	if(ch->pcdata->condition[iCond] == 3)
	{
		switch ( iCond )
		{
			case COND_FULL:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_HUNGRY, ch);
					send_to_char("You are a mite peckish.\r\n", ch);
				} break;

			case COND_THIRST:
				if(ch->level < LEVEL_AVATAR && ch->Class != CLASS_VAMPIRE)
				{
					set_char_color(AT_THIRSTY, ch);
					send_to_char("You could use a sip of something refreshing.\r\n", ch);
				} break;

			case COND_BLOODTHIRST:
				if(ch->level < LEVEL_AVATAR)
				{
					set_char_color(AT_BLOOD, ch);
					send_to_char("You feel an aching in your fangs.\r\n", ch);
				} break;
		}
	}
}
void check_alignment							(CHAR_DATA *ch){

	//Race alignment restrictions, h
	if(ch->alignment < race_table[ch->race]->minalign) {set_char_color(AT_BLOOD, ch); send_to_char("Your actions have been incompatible with the ideals of your race. This troubles you.\r\n", ch);}
	if(ch->alignment > race_table[ch->race]->maxalign) {set_char_color(AT_BLOOD, ch); send_to_char("Your actions have been incompatible with the ideals of your race. This troubles you.\r\n", ch);}

	//Paladins need some restrictions, this is where we crunch 'em.
	if(ch->Class == CLASS_PALADIN)
	{
		if(ch->alignment < 250 )
		{
			set_char_color(AT_BLOOD, ch);
			send_to_char( "You are wracked with guilt and remorse for your craven actions!\r\n", ch);
			act(AT_BLOOD, "$n prostrates $mself, seeking forgiveness from $s Lord.", ch, NULL, NULL, TO_ROOM);
			worsen_mental_state(ch, 15);
			return;
		}

		if(ch->alignment < 500)
		{
			set_char_color(AT_BLOOD, ch);
			send_to_char("As you betray your faith, your mind begins to betray you.\r\n", ch);
			act(AT_BLOOD, "$n shudders, judging $s actions unworthy of a Paladin.", ch, NULL, NULL, TO_ROOM);
			worsen_mental_state(ch, 6);
			return;
		}
	}
}
void mobile_update								(void){
	//Mob autonomous action. (This function takes 25% to 35% of ALL Mud cpu time.)
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	EXIT_DATA *pexit;
	TRV_WORLD *lc;
	int door;
	ch_ret retcode;
	retcode = rNONE;

	//Examine all mobs. 
	lc = trworld_create(TR_CHAR_WORLD_BACK);
	
	for(ch = last_char; ch; ch = trvch_wnext(lc)) 
	{
		set_cur_char(ch);

		// Despawn random world encounters only after 99 mobile updates
		// with no player in the mob's current room.
		if(IS_NPC(ch) && ch->in_room && xIS_SET(ch->act, ACT_WORLD_SPAWN))
		{
			CHAR_DATA *person;
			bool player_present = FALSE;

			for(person = ch->in_room->first_person; person; person = person->next_in_room)
			{
				if(!IS_NPC(person))
				{
					player_present = TRUE;
					break;
				}
			}

			if(player_present)
				ch->timer = 99;
			else if(ch->timer <= 1 || --ch->timer <= 0)
			{
				if(ch->fighting) stop_fighting(ch, TRUE);
				extract_char(ch, TRUE);
				continue;
			}
		}

		if(!IS_NPC(ch)) 
		{
			drunk_randoms(ch);
			hallucinations(ch);
			continue;
		}

		if(!ch->in_room || IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_POSSESS))
        continue;

		//Clean up 'animated corpses' that are not charmed'
		if(ch->pIndexData->vnum == MOB_VNUM_ANIMATED_CORPSE && !IS_AFFECTED(ch, AFF_CHARM)) 
		{
			if(ch->in_room->first_person)
			act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);

			//Guard against purging switched?
			if(IS_NPC(ch))   
            extract_char(ch, TRUE);
			continue;
		}

		if(!xIS_SET(ch->act, ACT_RUNNING) && !xIS_SET(ch->act, ACT_SENTINEL) && !ch->fighting && ch->hunting) 
		{
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
			hunt_victim(ch);
			continue;
		}

		//Examine call for special procedure. 
		if(!xIS_SET( ch->act, ACT_RUNNING) && ch->spec_fun && !IS_AFFECTED(ch, AFF_POSSESS)) 
		{
			if((*ch->spec_fun) (ch)) continue;
			if(char_died(ch))		 continue;
		}

		//Check for mudprogram script on mob. 
		if(HAS_PROG(ch->pIndexData, SCRIPT_PROG) && !xIS_SET(ch->act, ACT_STOP_SCRIPT)) 
		{
			mprog_script_trigger( ch );
			continue;
		}

		if(ch != cur_char) {
			bug("%s: ch != cur_char after spec_fun", __func__);
			continue;
		}

		//That's all for sleeping / busy monster. 
		if(ch->position != POS_STANDING)
        continue;

		if(xIS_SET(ch->act, ACT_MOUNTED)) 
		{
			if(xIS_SET(ch->act, ACT_AGGRESSIVE) || xIS_SET(ch->act, ACT_META_AGGR))
			do_emote(ch, "snarls and growls.");
			continue;
		}

		if(xIS_SET(ch->in_room->room_flags, ROOM_SAFE) && (xIS_SET(ch->act, ACT_AGGRESSIVE) || xIS_SET(ch->act, ACT_META_AGGR)))
        do_emote(ch, "glares around and snarls.");

		//MOBprogram random trigger. 
		if(ch->in_room->area->nplayer > 0) 
		{
			mprog_random_trigger(ch);
			if(char_died(ch))				
			continue;
			if(ch->position < POS_STANDING)	
			continue;
		}

		//MOBprogram hour trigger: do something for an hour. 
		mprog_hour_trigger(ch);
		if(char_died(ch)) 				
		continue;

		rprog_hour_trigger(ch);
		if(char_died(ch))				
		continue;

		if(ch->position < POS_STANDING)	
		continue;


		//Scavenge.
		if(xIS_SET(ch->act, ACT_SCAVENGER) && ch->in_room->first_content && number_bits(2) == 0)
		{
			OBJ_DATA *obj;
			OBJ_DATA *obj_best;
			int max;

			max = 1;
			obj_best = NULL;
			
			for(obj = ch->in_room->first_content; obj; obj = obj->next_content)
			{
				if(IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && !xIS_SET(ch->act, ACT_PROTOTYPE))
				continue;
			
				if(CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max && !IS_OBJ_STAT(obj, ITEM_BURIED) && !IS_OBJ_STAT(obj, ITEM_HIDDEN))
				{
					obj_best = obj;
					max = obj->cost;
				}
			}

			if(obj_best)
			{
				obj_from_room(obj_best);
				obj_to_char(obj_best, ch);
				act(AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM);
			}
		}

		AFFECT_DATA af1,af2,af3,af4,af5;
		memset(&af1, 0, sizeof(af1)); memset(&af2, 0, sizeof(af2)); memset(&af3, 0, sizeof(af3));
		memset(&af4, 0, sizeof(af4)); memset(&af5, 0, sizeof(af5));

		if(ch->race == 95)	//DUNMER
		{
			if(!is_affected(ch, tes_effect_resistance_fire))
				{af1.type = tes_effect_resistance_fire; 
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1; 		//RIS_FIRE (FIRE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 93)	//BOSMER
		{
			if(!is_affected(ch, tes_effect_resistance_disease))
				{af1.type = tes_effect_resistance_disease; 
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 8; 		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 91) 	//ALTMER
		{	
			if(!is_affected(ch, tes_effect_resistance_disease))
				{af1.type = tes_effect_resistance_disease; 	
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 8; 		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_weakness_shock))
				{af2.type = tes_effect_weakness_shock; 		
					af2.duration = -1; af2.location = APPLY_SUSCEPTIBLE; af2.modifier = 4; 		//RIS_ELECTRICITY (SHOCK) 	
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}

			if(!is_affected(ch, tes_effect_weakness_frost))
				{af3.type = tes_effect_weakness_frost; 			
					af3.duration = -1; af3.location = APPLY_SUSCEPTIBLE; af3.modifier = 2; 		//RIS_COLD (FROST) 	
						xCLEAR_BITS(af3.bitvector); affect_to_char(ch, &af3);}

			if(!is_affected(ch, tes_effect_weakness_fire))
				{af4.type = tes_effect_weakness_fire; 		
					af4.duration = -1; af4.location = APPLY_SUSCEPTIBLE; af4.modifier = 1; 		//RIS_FIRE (FIRE) 	
						xCLEAR_BITS(af4.bitvector); affect_to_char(ch, &af4);}

			if(!is_affected(ch, tes_effect_weakness_magic))
				{af5.type = tes_effect_weakness_magic; 		
					af5.duration = -1; af5.location = APPLY_SUSCEPTIBLE; af5.modifier = 1048576; //RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af5.bitvector); affect_to_char(ch, &af5);}
		}
		if(ch->race == 98) 	//NORD 
		{
			if(!is_affected(ch, tes_effect_resistance_shock))
				{af1.type = tes_effect_resistance_shock;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 4;		//RIS_ELECTRICITY (SHOCK) 
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_frost))
				{af2.type = tes_effect_resistance_frost; 
					af2.duration = -1; af2.location = APPLY_IMMUNE; af2.modifier = 2;			//RIS_COLD (FROST)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}
		}
		if(ch->race == 94) 	//BRETON	
		{
			if(!is_affected(ch, tes_effect_resistance_magic))
				{af1.type = tes_effect_resistance_magic;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1048576;	//RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 99) 	//REDGUARD
		{ 	
			if(!is_affected(ch, tes_effect_resistance_poison))
				{af1.type = tes_effect_resistance_poison;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 256;		//RIS_POISON (POISON)
							xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_disease))
				{af2.type = tes_effect_resistance_disease;
					af2.duration = -1; af2.location = APPLY_RESISTANT; af2.modifier = 8;		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}
		}
		if(ch->race == 52) 	//ORC	
		{	
			if(!is_affected(ch, tes_effect_resistance_magic))
				{af1.type = tes_effect_resistance_magic;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1048576;	//RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 92)	//ARGONIAN
		{	
			if(!is_affected(ch, tes_effect_resistance_poison))
				{af1.type = tes_effect_resistance_poison;
					af1.duration = -1; af1.location = APPLY_IMMUNE; af1.modifier = 256;			//RIS_POISON (POISON)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_magic))
				{af2.type = tes_effect_resistance_magic;
					af2.duration = -1; af2.location = APPLY_RESISTANT; af2.modifier = 8;		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}

			if(!is_affected(ch, tes_effect_water_breathing))									//WATER_BREATHING
				{af3.type = tes_effect_water_breathing;
					af3.duration = -1; af3.location = APPLY_AFFECT; af3.modifier = 0;
				af3.bitvector = meb(AFF_AQUA_BREATH); affect_to_char(ch, &af3);}
		}
		if(ch->race == 97)	//KHAJIIT
		{	
			if(!is_affected(ch, tes_effect_detect_life))										//NIGHT_EYE
				{af1.type = tes_effect_detect_life;
					af1.duration = -1; af1.location = APPLY_AFFECT; af1.modifier = 0;
						af1.bitvector = meb(AFF_DETECT_HIDDEN); affect_to_char(ch, &af1);} 
       }

		//ENDING THE HIRE COMPANION
		CHAR_DATA *boss;
		for(boss = ch->in_room->first_person; boss; boss = boss->next_in_room)
		{
			if(!IS_AFFECTED(boss, AFF_HIRELING) && IS_NPC(boss))
			{
				if(boss->master)
				{
					stop_follower(boss);
					send_to_char("&WThe condract with your companion has ended.&D\r\n", ch);
				}
			}
		}
		
		//Wander 
		if(!xIS_SET(ch->act, ACT_RUNNING)
        && !xIS_SET(ch->act, ACT_SENTINEL)
        && !xIS_SET(ch->act, ACT_PROTOTYPE)
        && !xIS_SET(ch->act, ACT_NOWANDER)
        && !xIS_SET(ch->act, ACT_PET)
        && (door = number_bits(5)) <= 9
        && (pexit = get_exit( ch->in_room, door)) != NULL
        && pexit->to_room
        && !IS_SET(pexit->exit_info, EX_WINDOW)
        && (!IS_SET(pexit->exit_info, EX_CLOSED ) || (IS_AFFECTED( ch, AFF_PASS_DOOR ) && !IS_SET( pexit->exit_info, EX_NOPASSDOOR)))
        && !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
        && !xIS_SET(pexit->to_room->room_flags, ROOM_DEATH)
        && (!xIS_SET(ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area))
		{	
			retcode = move_char(ch, pexit, 0);

			//If ch changes position due to it's or someother mob's movement via MOBProgs, continue 
			if(char_died(ch)) continue;
			if(retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL) || ch->position < POS_STANDING) continue;
		}

		//Flee 
		if(ch->hit < ch->max_hit /2
        && (door = number_bits(4)) <= 9
        && (pexit = get_exit(ch->in_room, door)) != NULL
        && pexit->to_room
        && !IS_SET(pexit->exit_info, EX_WINDOW)
        && !IS_SET(pexit->exit_info, EX_CLOSED)
        && !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
        && !xIS_SET(pexit->to_room->room_flags, ROOM_DEATH)
        && (!xIS_SET( ch->act, ACT_STAY_AREA) || pexit->to_room->area == ch->in_room->area))
		{
			CHAR_DATA *rch;
			bool found;

			found = FALSE;
			for(rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
			{
				if(is_fearing(ch, rch))
				{
					switch (number_bits(2))
					{
						case 0:	snprintf(buf, MAX_STRING_LENGTH, "Get away from me, %s!", rch->name); 			break;
						case 1:	snprintf(buf, MAX_STRING_LENGTH, "Leave me be, %s!", rch->name);				break;
						case 2:	snprintf(buf, MAX_STRING_LENGTH, "%s is trying to kill me!  Help!", rch->name);	break;
						case 3: snprintf(buf, MAX_STRING_LENGTH, "Someone save me from %s!", rch->name);		break;
					}
					do_yell(ch, buf);
					found = TRUE;
					break;
				}
			}
			
			if(found) 
			retcode = move_char(ch, pexit, 0);
		}
	}
	trworld_dispose(&lc);
}
void char_calendar_update						(void){
	/* Anything that should be updating based on time should go here - like hunger/thirst for one */
	CHAR_DATA *ch;
	TRV_WORLD *lc;

	lc = trworld_create(TR_CHAR_WORLD_BACK);
	
	for(ch = last_char; ch; ch = trvch_wnext(lc))
	{
		if(!IS_NPC(ch) && !IS_IMMORTAL(ch))
		{
			gain_condition(ch, COND_DRUNK, -1);

			//Newbies won't starve now
			if(ch->in_room && ch->level > 3)
				gain_condition(ch, COND_FULL, -1 + race_table[ch->race]->hunger_mod);

			//Newbies won't dehydrate now 
			if(ch->in_room && ch->level > 3)
			{
				int sector;
				sector = ch->in_room->sector_type;

				switch (sector)
				{
					default: 				gain_condition(ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod); break;
					case SECT_DESERT:		gain_condition(ch, COND_THIRST, -3 + race_table[ch->race]->thirst_mod); break;
					case SECT_UNDERWATER:
					case SECT_OCEANFLOOR:
						if(number_bits(1) == 0)
						gain_condition( ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod );
						break;
				}
			}
		}
	}
	trworld_dispose(&lc);
}
void char_update								(void){
	//Update all chars, including mobs. (This function is performance sensitive.)

	CHAR_DATA *ch;
	CHAR_DATA *ch_save;
	TRV_WORLD *lc;
	short save_count = 0;

	ch_save = NULL;
	lc = trworld_create(TR_CHAR_WORLD_BACK);
		
	for(ch = last_char; ch; ch = trvch_wnext(lc))
	{
		set_cur_char(ch);
		
		if(!is_affected(ch, tes_effect_dreamwalk))
		{
		    ch->dreamwalk_timer = 0;
			ch->automatic_dreamwalk_decision = 0;
			ch->event_type = EVENT_NONE;
		}	

		//CHECKING FOR FACTION MEMBERSHIP (Works only for npcs)
		CHAR_FACTION_MEMBERSHIP(ch);
		
		//------------------------------------------------------//
		AFFECT_DATA *paf;

		//Bleed's continous damage health
		for(paf = ch->first_affect; paf; paf = paf->next)
		{
			if(paf->type == tes_action_bleed)
				ch->hit = UMAX(0, ch->hit + paf->modifier);
		}

		//Stab's continous damage health
		for(paf = ch->first_affect; paf; paf = paf->next)
		{
			if(paf->type == tes_action_stab)
				ch->hit = UMAX(0, ch->hit + paf->modifier);
		}	

		//Sting's continous damage fatigue
		for(paf = ch->first_affect; paf; paf = paf->next)
		{
			if(paf->type == tes_action_stab)
				ch->hit = UMAX(0, ch->move + paf->modifier);
		}		
		//------------------------------------------------------//
		AFFECT_DATA af1,af2,af3,af4,af5;
		memset(&af1, 0, sizeof(af1)); memset(&af2, 0, sizeof(af2)); memset(&af3, 0, sizeof(af3));
		memset(&af4, 0, sizeof(af4)); memset(&af5, 0, sizeof(af5));

		if(ch->race == 4) 	//DUNMER
		{
			if(!is_affected(ch, tes_effect_resistance_fire))
				{af1.type = tes_effect_resistance_fire; 
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1;		//RIS_FIRE (FIRE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 2) 	//BOSMER
		{
			if(!is_affected(ch, tes_effect_resistance_disease))	
				{af1.type = tes_effect_resistance_disease; 
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 8;		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 0) 	//ALTMER
		{
			if(!is_affected(ch, tes_effect_resistance_disease))
				{af1.type = tes_effect_resistance_disease; 	
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 8; 		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_weakness_shock))
				{af2.type = tes_effect_weakness_shock; 		
					af2.duration = -1; af2.location = APPLY_SUSCEPTIBLE; af2.modifier = 4; 		//RIS_ELECTRICITY (SHOCK) 	
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}

			if(!is_affected(ch, tes_effect_weakness_frost))
				{af3.type = tes_effect_weakness_frost; 			
					af3.duration = -1; af3.location = APPLY_SUSCEPTIBLE; af3.modifier = 2; 		//RIS_COLD (FROST)
						xCLEAR_BITS(af3.bitvector); affect_to_char(ch, &af3);}

			if(!is_affected(ch, tes_effect_weakness_fire))
				{af4.type = tes_effect_weakness_fire; 		
					af4.duration = -1; af4.location = APPLY_SUSCEPTIBLE; af4.modifier = 1;  	//RIS_FIRE (FIRE)
						xCLEAR_BITS(af4.bitvector); affect_to_char(ch, &af4);}

			if(!is_affected(ch, tes_effect_weakness_magic))
				{af5.type = tes_effect_weakness_magic; 		
					af5.duration = -1; af5.location = APPLY_SUSCEPTIBLE; af5.modifier = 1048576; //RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af5.bitvector); affect_to_char(ch, &af5);}
		}
		if(ch->race == 8) 	//NORD
		{
			if(!is_affected(ch, tes_effect_resistance_shock))									//RIS_ELECTRICITY (SHOCK)
				{af1.type = tes_effect_resistance_shock;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 4;
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_frost))
				{af2.type = tes_effect_resistance_frost; 
					af2.duration = -1; af2.location = APPLY_IMMUNE; af2.modifier = 2;			//RIS_COLD (FROST)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}
		}
		if(ch->race == 3) 	//BRETON
		{
			if(!is_affected(ch, tes_effect_resistance_magic))
				{af1.type = tes_effect_resistance_magic;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1048576;	//RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 10) 	//REDGUARD
		{
			if(!is_affected(ch, tes_effect_resistance_poison))
				{af1.type = tes_effect_resistance_poison;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 256;		//RIS_POISON (POISON)
							xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_disease))
				{af2.type = tes_effect_resistance_disease;
					af2.duration = -1; af2.location = APPLY_RESISTANT; af2.modifier = 8;		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}
		}
		if(ch->race == 9)	//ORC
		{
			if(!is_affected(ch, tes_effect_resistance_magic))
				{af1.type = tes_effect_resistance_magic;
					af1.duration = -1; af1.location = APPLY_RESISTANT; af1.modifier = 1048576;	//RIS_MAGIC (MAGIC)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}
		}
		if(ch->race == 1)	//ARGONIAN
		{
			if(!is_affected(ch, tes_effect_resistance_poison))
				{af1.type = tes_effect_resistance_poison;
					af1.duration = -1; af1.location = APPLY_IMMUNE; af1.modifier = 256;			//RIS_POISON (POISON)
						xCLEAR_BITS(af1.bitvector); affect_to_char(ch, &af1);}

			if(!is_affected(ch, tes_effect_resistance_disease))
				{af2.type = tes_effect_resistance_disease;
					af2.duration = -1; af2.location = APPLY_RESISTANT; af2.modifier = 8;		//RIS_ENERGY (DISEASE)
						xCLEAR_BITS(af2.bitvector); affect_to_char(ch, &af2);}

			if(!is_affected(ch, tes_effect_water_breathing))
				{af3.type = tes_effect_water_breathing;
					af3.duration = -1; af3.location = APPLY_AFFECT; af3.modifier = 0;			//WATER_BREATHING
				af3.bitvector = meb(AFF_AQUA_BREATH); affect_to_char(ch, &af3);}
		}
		if(ch->race == 7)	//KHAJIIT
		{
			if(!is_affected(ch, tes_effect_detect_life))
				{af1.type = tes_effect_detect_life;
					af1.duration = -1; af1.location = APPLY_AFFECT; af1.modifier = 0;			//NIGHT_EYE
						af1.bitvector = meb(AFF_DETECT_HIDDEN); affect_to_char(ch, &af1);} 
		}
 
		//Sun damage
		if(is_affected(ch, tes_effect_vampirism))
		{
			if(IS_OUTSIDE(ch) && !NO_WEATHER_SECT(ch->in_room->sector_type))
			{
				if(time_info.hour < 20 && time_info.hour > 8)
				{
					ch->hit = ch->hit - 30;
					ch_printf(ch, "&RYou are damaged from sun light for 30 points.&D\r\n"); 
				}
			}
		}
	
		//Do a room_prog rand check right off the bat if ch disappears (rprog might wax npc's), continue.
		if(!IS_NPC(ch)) 	rprog_random_trigger(ch);
		if(char_died(ch)) 	continue;
		if(IS_NPC(ch))		mprog_time_trigger(ch);
		if(char_died(ch))	continue;

		rprog_time_trigger(ch);
		if(char_died(ch)) 	continue;

		//See if player should be auto-saved.
		if(!IS_NPC(ch) && (!ch->desc || ch->desc->connected == CON_PLAYING) && ch->level >= 2 && current_time - ch->save_time > (sysdata.save_frequency * 60))
			ch_save = ch;
		else
			ch_save = NULL;

		if(ch->position >= POS_STUNNED)
		{
			if(ch->hit < ch->max_hit)   ch->hit  += hit_gain(ch);
			if(ch->mana < ch->max_mana) ch->mana += mana_gain(ch);
			if(ch->move < ch->max_move) ch->move += move_gain(ch);
		}

		if(ch->position == POS_STUNNED)
		update_pos(ch);

		//Expire variables 
		if(ch->variables)
		{
			VARIABLE_DATA *vd, *vd_next = NULL, *vd_prev = NULL;

			for(vd = ch->variables; vd; vd = vd_next)
			{
				vd_next = vd->next;
				if(vd->timer > 0 && --vd->timer == 0)
				{
					if(vd == ch->variables) ch->variables = vd_next;
					else if(vd_prev)		vd_prev->next = vd_next;
					delete_variable( vd );
				}
				else
				vd_prev = vd;
			}
		}

		//Morph timer expires 
		if(ch->morph)
		{
			if(ch->morph->timer > 0)
			{
				--ch->morph->timer;
				
				if(ch->morph->timer == 0)
				do_unmorph_char( ch );
			}
		}


		//To make people with a nuisance's flags life difficult 
		if(!IS_NPC( ch ) && ch->pcdata->nuisance)
		{
			time_t temp;

			if(ch->pcdata->nuisance->flags < MAX_NUISANCE_STAGE)
			{
				temp = ch->pcdata->nuisance->max_time - ch->pcdata->nuisance->set_time;
				temp *= ch->pcdata->nuisance->flags;
				temp /= MAX_NUISANCE_STAGE;
				temp += ch->pcdata->nuisance->set_time;
				
				if(temp < current_time)
				++ch->pcdata->nuisance->flags;
			}
		}

		if(!IS_NPC( ch ) && ch->level < LEVEL_IMMORTAL)
		{
			if(++ch->timer >= 12)
			{
				if(!IS_IDLE(ch))
				{

					//ch->was_in_room = ch->in_room;
					if(ch->fighting) stop_fighting(ch, TRUE);
				
					act(AT_ACTION, "$n disappears into the void.", ch, NULL, NULL, TO_ROOM);
					send_to_char("You disappear into the void.\r\n", ch);
				
					if(IS_SET(sysdata.save_flags, SV_IDLE))
					save_char_obj( ch );
			
					SET_BIT(ch->pcdata->flags, PCFLAG_IDLE);
					char_from_room(ch);
					char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
				}
			}

			if(ch->pcdata->condition[COND_DRUNK] > 8)
            worsen_mental_state(ch, ch->pcdata->condition[COND_DRUNK] / 8);
		
			if(ch->pcdata->condition[COND_FULL] > 1)
			{
				switch (ch->position)
				{
					case POS_SLEEPING	: better_mental_state(ch, 4); break;
					case POS_RESTING	: better_mental_state(ch, 3); break;
					case POS_SITTING	:
					case POS_MOUNTED	: better_mental_state(ch, 2); break;
					case POS_STANDING	: better_mental_state(ch, 1); break;
					case POS_FIGHTING	:
					case POS_EVASIVE	:
					case POS_DEFENSIVE	:
					case POS_AGGRESSIVE	:
					case POS_BERSERK	: if(number_bits(2) == 0) better_mental_state(ch, 1); break;
				}
			}

			if(ch->pcdata->condition[COND_THIRST] > 1)
			{	
				switch (ch->position)
				{
					case POS_SLEEPING	: better_mental_state(ch, 5); break;
					case POS_RESTING	: better_mental_state(ch, 3); break;
					case POS_SITTING	:
					case POS_MOUNTED	: better_mental_state(ch, 2); break;
					case POS_STANDING	: better_mental_state(ch, 1); break;
					case POS_FIGHTING	:
					case POS_EVASIVE	:
					case POS_DEFENSIVE	:
					case POS_AGGRESSIVE	:
					case POS_BERSERK	: if(number_bits(2) == 0) better_mental_state(ch, 1); break;
				}
			}

			//Function added on suggestion from Cronel
			check_alignment(ch);
			gain_condition(ch, COND_DRUNK, -1);
			gain_condition(ch, COND_FULL, -1 + race_table[ch->race]->hunger_mod);

			if(ch->Class == CLASS_VAMPIRE && ch->level >= 10)
			{
				if(time_info.hour < 21 && time_info.hour > 5) gain_condition( ch, COND_BLOODTHIRST, -1);
			}

			if(CAN_PKILL(ch) && ch->pcdata->condition[COND_THIRST] - 9 > 10)
			gain_condition(ch, COND_THIRST, -9);

			if(!IS_NPC(ch) && ch->pcdata->nuisance)
			{
				int value;

				value = ((0 - ch->pcdata->nuisance->flags) * ch->pcdata->nuisance->power);
				gain_condition(ch, COND_THIRST, value);
				gain_condition(ch, COND_FULL, --value);
			}
		}

		if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 && ch->pcdata->release_date <= current_time)
		{
			ROOM_INDEX_DATA *location;
			if( ch->pcdata->clan)	location = get_room_index(ch->pcdata->clan->recall);
			else					location = get_room_index(ROOM_VNUM_TEMPLE);
			
			if(!location) location = ch->in_room;
			
			MOBtrigger = FALSE;
			char_from_room(ch);
			char_to_room(ch, location);
			send_to_char("The gods have released you from hell as your sentance is up!\r\n", ch);
			do_look(ch, "auto");
			STRFREE(ch->pcdata->helled_by);
			ch->pcdata->helled_by = NULL;
			ch->pcdata->release_date = 0;
			save_char_obj(ch);
		}

		if(!char_died(ch))
		{
			//Careful with the damages here, MUST NOT refer to ch after damage taken, without checking return code and/or char_died as it may be lethal damage.
			if(IS_AFFECTED(ch, AFF_POISON))
			{
				act(AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
				act(AT_POISON, "You shiver and suffer.",  ch, NULL, NULL, TO_CHAR);
				ch->mental_state = URANGE(20, ch->mental_state + (IS_NPC(ch) ? 2 : IS_PKILL(ch) ? 3 : 4), 100);
				damage(ch, ch, 6, gsn_poison);
			}
			else if(ch->position == POS_INCAP)  damage(ch, ch, 1, TYPE_UNDEFINED);
			else if(ch->position == POS_MORTAL) damage(ch, ch, 4, TYPE_UNDEFINED);
		 
			if(char_died(ch)) continue;


			//Recurring spell affect
			if(IS_AFFECTED(ch, AFF_RECURRINGSPELL))
			{
				AFFECT_DATA *paf, *paf_next;
				SKILLTYPE *skill;
				bool found = FALSE, died = FALSE;

				for(paf = ch->first_affect; paf; paf = paf_next)
				{
					paf_next = paf->next;
					if(paf->location == APPLY_RECURRINGSPELL)
					{
						found = TRUE;
							
						if(IS_VALID_SN(paf->modifier) && (skill = skill_table[paf->modifier]) != NULL && skill->type == SKILL_SPELL)
						{
							if((*skill->spell_fun) (paf->modifier, ch->level, ch, ch) == rCHAR_DIED || char_died(ch))
							{
								died = TRUE;
								break;
							}
						}
					}
				}
				if(died) 	continue;
				if(!found)	xREMOVE_BIT(ch->affected_by, AFF_RECURRINGSPELL);
			}

			if(ch->mental_state >= 30)
			{
				switch ((ch->mental_state + 5) / 10)
				{
					case 3:
						send_to_char("You feel feverish.\r\n", ch);
						act(AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM);
						break;
					case 4:
						send_to_char("You do not feel well at all.\r\n", ch);
						act(AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM);
						break;
					case 5:
						send_to_char("You need help!\r\n", ch);
						act(AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM);
						break;
					case 6:
						send_to_char("Seekest thou a cleric.\r\n", ch);
						act(AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM);
						break;
					case 7:
						send_to_char("You feel reality slipping away...\r\n", ch);
						act(AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM);
						break;
					case 8:
						send_to_char("You begin to understand... everything.\r\n", ch);
						act(AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM);
						break;
					case 9:
						send_to_char("You are ONE with the universe.\r\n", ch);
						act(AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
						break;
					case 10:
						send_to_char("You feel the end is near.\r\n", ch);
						act(AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM);
						break;
				}	
			}

			if(ch->mental_state <= -30)
			{
				switch ((abs( ch->mental_state ) + 5) / 10)
				{
					case 10:
						if( ch->position > POS_SLEEPING )
						{
							if((ch->position == POS_STANDING || ch->position < POS_FIGHTING) && number_percent() + 10 < abs(ch->mental_state)) do_sleep(ch, "");
							else send_to_char("You're barely conscious.\r\n", ch);
						} break;
					case 9:
						if(ch->position > POS_SLEEPING)
						{
							if((ch->position == POS_STANDING || ch->position < POS_FIGHTING) && (number_percent() + 20) < abs(ch->mental_state)) do_sleep(ch, "");
							else send_to_char("You can barely keep your eyes open.\r\n", ch);
						} break;
					case 8:
						if(ch->position > POS_SLEEPING)
						{
							if(ch->position < POS_SITTING && (number_percent() + 30) < abs(ch->mental_state)) do_sleep(ch, "");
							else send_to_char("You're extremely drowsy.\r\n", ch);
						} break;
					case 7:
						if(ch->position > POS_RESTING)
							send_to_char("You feel very unmotivated.\r\n", ch);
							break;
					case 6:
						if(ch->position > POS_RESTING)
							send_to_char("You feel sedated.\r\n", ch);
							break;
					case 5:
						if(ch->position > POS_RESTING)
							send_to_char("You feel sleepy.\r\n", ch);
							break;
					case 4:
						if(ch->position > POS_RESTING)
							send_to_char("You feel tired.\r\n", ch);
							break;
					case 3:
						if(ch->position > POS_RESTING)
							send_to_char("You could use a rest.\r\n", ch);
							break;
				}
			}

			if(ch->timer > 24) 																	do_quit(ch, "");
			else if(ch == ch_save && IS_SET(sysdata.save_flags, SV_AUTO) && ++save_count < 10) 	save_char_obj(ch);
		}
	}
   trworld_dispose(&lc);
}
void obj_update									(void){
	//Update all objs. This function is performance sensitive.
	OBJ_DATA *obj;
	TRV_WORLD *lc;
	short AT_TEMP;

	lc = trworld_create(TR_OBJ_WORLD_BACK);
	
	for(obj = last_object; obj; obj = trvobj_wnext(lc))
	{
		CHAR_DATA *rch;
		const char *message;

		set_cur_obj(obj);

		if(obj->carried_by)											oprog_random_trigger(obj);
		else if(obj->in_room && obj->in_room->area->nplayer > 0) 	oprog_random_trigger(obj);

		if(obj_extracted(obj)) continue;

		if(obj->item_type == ITEM_LIGHT)
		{
			CHAR_DATA *tch;

			if((tch = obj->carried_by))
			{
				if(!IS_NPC(tch) && ((obj == get_eq_char(tch, WEAR_LIGHT)) || (IS_SET( obj->value[3], PIPE_LIT))) && (obj->value[2] > 0))
				{
					if(--obj->value[2] == 0 && tch->in_room)
					{
						tch->in_room->light -= obj->count;
						if(tch->in_room->light < 0) tch->in_room->light = 0;
						
						act(AT_ACTION, "$p goes out.", tch, obj, NULL, TO_ROOM);
						act(AT_ACTION, "$p goes out.", tch, obj, NULL, TO_CHAR);
						
						if(obj->serial == cur_obj) global_objcode = rOBJ_EXPIRED;
						extract_obj(obj);
						continue;
					}
				}
			}
			else if(obj->in_room)
			{
				if(IS_SET( obj->value[3], PIPE_LIT ) && (obj->value[2] > 0))
				{
					if(--obj->value[2] == 0)
					{
						obj->in_room->light -= obj->count;
						
						if(obj->in_room->light < 0) obj->in_room->light = 0;
						if((tch = obj->in_room->first_person))
						{
							act(AT_ACTION, "$p goes out.", tch, obj, NULL, TO_ROOM);
							act(AT_ACTION, "$p goes out.", tch, obj, NULL, TO_CHAR);
						}
						
						if(obj->serial == cur_obj) global_objcode = rOBJ_EXPIRED;
						extract_obj(obj);
						continue;
					}
				}
			}
		}

		if(obj->item_type == ITEM_PIPE)
		{
			if(IS_SET(obj->value[3], PIPE_LIT))
			{
				if(--obj->value[1] <= 0)
				{
					obj->value[1] = 0;
					REMOVE_BIT(obj->value[3], PIPE_LIT );
				}
				else if(IS_SET(obj->value[3], PIPE_HOT))
				{
					REMOVE_BIT( obj->value[3], PIPE_HOT);
				}
				else
				{
					if(IS_SET(obj->value[3], PIPE_GOINGOUT))
					{
						REMOVE_BIT( obj->value[3], PIPE_LIT );
						REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
					}
					else 
						SET_BIT( obj->value[3], PIPE_GOINGOUT );
				}
				
				if( !IS_SET(obj->value[3], PIPE_LIT)) 
				SET_BIT(obj->value[3], PIPE_FULLOFASH);
			}
			else
            REMOVE_BIT( obj->value[3], PIPE_HOT );
		}

		//Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn 
		if(obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
		{
			short timerfrac = UMAX(1, obj->timer - 1);
			
			if(obj->item_type == ITEM_CORPSE_PC)
				timerfrac = (int)(obj->timer / 8 + 1);

			if(obj->timer > 0 && obj->value[2] > timerfrac)
			{
				char buf[MAX_STRING_LENGTH];
				char name[MAX_STRING_LENGTH];
				const char *bufptr;

				bufptr = one_argument(obj->short_descr, name);
				bufptr = one_argument(bufptr, name);
				bufptr = one_argument(bufptr, name);

				separate_obj(obj);
				obj->value[2] = timerfrac;
				snprintf(buf, MAX_STRING_LENGTH, corpse_descs[UMIN(timerfrac - 1, 4 )], bufptr);
				STRFREE(obj->description);
				obj->description = STRALLOC(buf);
			}
		}

		//Don't let inventory decay 
		if(IS_OBJ_STAT(obj, ITEM_INVENTORY))
        continue;

		//Groundrot items only decay on the ground 
		if(IS_OBJ_STAT(obj, ITEM_GROUNDROT) && !obj->in_room)
        continue;

		if((obj->timer <= 0 || --obj->timer > 0))
        continue;

		//If we get this far, object's timer has expired. 
		AT_TEMP = AT_PLAIN;
		switch (obj->item_type)
		{
			default:
				message = "$p mysteriously vanishes.";
				AT_TEMP = AT_PLAIN;
				break;

			case ITEM_CONTAINER:
				message = "$p falls apart, tattered from age.";
				AT_TEMP = AT_OBJECT;
				break;

			case ITEM_PORTAL:
				message = "$p unravels and winks from existence.";
				remove_portal(obj);
				obj->item_type = ITEM_TRASH;  /* so extract_obj  */
				AT_TEMP = AT_MAGIC;  /* doesn't remove_portal */
				break;
				
			case ITEM_FOUNTAIN:
			case ITEM_PUDDLE:
				message = "$p dries up.";
				AT_TEMP = AT_BLUE;
				break;

			case ITEM_CORPSE_NPC:
				message = "$p decays into dust and blows away.";
				AT_TEMP = AT_OBJECT;
				break;

			case ITEM_CORPSE_PC:
				message = "$p is sucked into a swirling vortex of colors...";
				AT_TEMP = AT_MAGIC;
				break;

			case ITEM_COOK:
			case ITEM_FOOD:
				message = "$p is devoured by a swarm of maggots.";
				AT_TEMP = AT_HUNGRY;
				break;

			case ITEM_BLOOD:
				message = "$p slowly seeps into the ground.";
				AT_TEMP = AT_BLOOD;
				break;

			case ITEM_BLOODSTAIN:
				message = "$p dries up into flakes and blows away.";
				AT_TEMP = AT_ORANGE;
				break;

			case ITEM_SCRAPS:
				message = "$p crumble and decay into nothing.";
				AT_TEMP = AT_OBJECT;
				break;

			case ITEM_FIRE:
				message = "$p burns out.";
				AT_TEMP = AT_FIRE;
		}

		if(obj->carried_by)	
			act(AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR);
		else if(obj->in_room && (rch = obj->in_room->first_person) != NULL && !IS_OBJ_STAT(obj, ITEM_BURIED))
		{
			act(AT_TEMP, message, rch, obj, NULL, TO_ROOM);
			act(AT_TEMP, message, rch, obj, NULL, TO_CHAR);
		}

		if(obj->serial == cur_obj) global_objcode = rOBJ_EXPIRED;
		extract_obj(obj);
	}
	trworld_dispose(&lc);
}
void char_check									(void){
	//Function to check important stuff happening to a player (This function should take about 5% of mud cpu time.)
	CHAR_DATA *ch;
	TRV_WORLD *lc1;
	OBJ_DATA *obj;
	EXIT_DATA *pexit;
	static int cnt = 0;
	int door, retcode;
	
	//This little counter can be used to handle periodic events 
	cnt = (cnt + 1) % SECONDS_PER_TICK;

	lc1 = trworld_create(TR_CHAR_WORLD_FORW);
	
	for(ch = first_char; ch; ch = trvch_wnext(lc1))
	{
		set_cur_char(ch);
		will_fall(ch, 0);

		if(char_died(ch)) continue;

		if(IS_NPC(ch))
		{
			if((cnt & 1)) continue;
			
			//Running mobs
			if(xIS_SET(ch->act, ACT_RUNNING))
			{
				if(!xIS_SET(ch->act, ACT_SENTINEL) && ch->position == POS_STANDING && !xIS_SET(ch->act, ACT_MOUNTED) && !ch->fighting && ch->hunting)
				{
					WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
					hunt_victim(ch);
					continue;
				}

				if(ch->spec_fun)
				{
					if((*ch->spec_fun) (ch))	continue;
					if(char_died(ch))			continue;
				}

				if(!xIS_SET( ch->act, ACT_SENTINEL)
                && ch->position == POS_STANDING
                && !xIS_SET(ch->act, ACT_MOUNTED)
                && !xIS_SET(ch->act, ACT_PROTOTYPE)
                && (door = number_bits(4)) <= 9
                && (pexit = get_exit( ch->in_room, door)) != NULL
                && pexit->to_room
                && !IS_SET(pexit->exit_info, EX_CLOSED)
                && !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
                && !xIS_SET(pexit->to_room->room_flags, ROOM_DEATH)
                && (!xIS_SET(ch->act, ACT_STAY_AREA) || pexit->to_room->area == ch->in_room->area))
				{
					retcode = move_char( ch, pexit, 0 );
					
					if( char_died(ch))
					continue;
				
					if(retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL) || ch->position < POS_STANDING)
					continue;
				}
			}
			continue;
		}
		else
		{
			if(ch->mount && ch->in_room != ch->mount->in_room)
			{
				xREMOVE_BIT(ch->mount->act, ACT_MOUNTED);
				ch->mount = NULL;
				ch->position = POS_STANDING;
				send_to_char("No longer upon your mount, you fall to the ground...\r\nOUCH!\r\n", ch);
			}

			if((ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER) || (ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR))
			{
				if(!IS_AFFECTED(ch, AFF_AQUA_BREATH))
				{
					if(ch->level < LEVEL_IMMORTAL)
					{
						int dam;

						//Changed level of damage at Brittany's request.
						dam = number_range(ch->max_hit / 100, ch->max_hit / 50);
						dam = UMAX(1, dam);
						
						if(number_bits(3) == 0)
						send_to_char("You cough and choke as you try to breathe water!\r\n", ch);
					
						damage(ch, ch, dam, TYPE_UNDEFINED);
					}
				}
			}

			if( char_died( ch ) )
            continue;

			if(ch->in_room && ((ch->in_room->sector_type == SECT_WATER_NOSWIM) || (ch->in_room->sector_type == SECT_WATER_SWIM)))
         	{
				if(!IS_AFFECTED(ch, AFF_FLYING)  && !IS_AFFECTED(ch, AFF_FLOATING) && !IS_AFFECTED(ch, AFF_AQUA_BREATH) && !ch->mount)
				{
					for(obj = ch->first_carrying; obj; obj = obj->next_content)
						if( obj->item_type == ITEM_BOAT )
						break;

						if(!obj)
						{
							if(ch->level < LEVEL_IMMORTAL)
							{
								int mov;
								int dam;

								if(ch->move > 0)
								{
									mov = number_range(ch->max_move / 20, ch->max_move / 5);
									mov = UMAX(1, mov);

									if(ch->move - mov < 0) 	ch->move = 0;
									else					ch->move -= mov;
								}
								else
								{
									dam = number_range(ch->max_hit / 20, ch->max_hit / 5);
									dam = UMAX(1, dam);

									if(number_bits(3) == 0)
									send_to_char("Struggling with exhaustion, you choke on a mouthful of water.\r\n", ch);
									damage(ch, ch, dam, TYPE_UNDEFINED);
								}
							}
						}
				}
			}

			//Beat up on link dead players 
			if(!ch->desc)
			{
				CHAR_DATA *wch;
				TRV_DATA *lc2;

				lc2 = trvch_create(ch, TR_CHAR_ROOM_FORW);
				
				for(wch = ch->in_room->first_person; wch; wch = trvch_next(lc2))
				{
					if(!IS_NPC(wch) || wch->fighting || IS_AFFECTED(wch, AFF_CHARM) || !IS_AWAKE(wch) || (xIS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch)) || !can_see(wch, ch))
					continue;

					if(is_hating(wch, ch))
					{
						found_prey(wch, ch);
						continue;
					}

					if((!xIS_SET( wch->act, ACT_AGGRESSIVE) && !xIS_SET(wch->act, ACT_META_AGGR)) || xIS_SET(wch->act, ACT_MOUNTED) || xIS_SET(wch->in_room->room_flags, ROOM_SAFE))
					continue;
				
					global_retcode = multi_hit(wch, ch, TYPE_UNDEFINED);
				}
				trv_dispose(&lc2);
			}
		}
	}
	trworld_dispose(&lc1);
}
void aggr_update								(void){
	//Aggress for each descriptor, for each mob in room, aggress on some random PC
	//This function should take 5% to 10% of ALL mud cpu time.
	//Unfortunately, checking on each PC move is too tricky, because we don't the mob to just attack the first PC who leads the party into the room.
	DESCRIPTOR_DATA *d, *dnext;
	TRV_DATA *lc;
	CHAR_DATA *wch, *ch, *vch, *victim;
	struct act_prog_data *apdtmp;

    //Check mobprog act queue 
	while((apdtmp = mob_act_list) != NULL)
	{
		wch = (CHAR_DATA *) mob_act_list->vo;
		
		if(!char_died(wch) && wch->mpactnum > 0)
		{
			MPROG_ACT_LIST *tmp_act;

			while((tmp_act = wch->mpact) != NULL)
			{
				if(tmp_act->obj && obj_extracted( tmp_act->obj)) {tmp_act->obj = NULL;}
				if(tmp_act->ch && !char_died(tmp_act->ch))		 {mprog_wordlist_check(tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->victim, tmp_act->target, ACT_PROG);}
				wch->mpact = tmp_act->next;
				DISPOSE(tmp_act->buf);
				DISPOSE(tmp_act);
			}
			wch->mpactnum = 0;
			wch->mpact = NULL;
		}
		mob_act_list = apdtmp->next;
		DISPOSE( apdtmp );
	}

    //Just check descriptors here for victims to aggressive mobs (We can check for linkdead victims in char_check.)
	for(d = first_descriptor; d; d = dnext)
	{
		dnext = d->next;
		
		if((d->connected != CON_PLAYING && d->connected != CON_EDITING) || (wch = d->character) == NULL) continue;
		if(char_died(wch) || IS_NPC(wch) || wch->level >= LEVEL_IMMORTAL || !wch->in_room) continue;

		lc = trvch_create(wch, TR_CHAR_ROOM_FORW);
		
		for(ch = wch->in_room->first_person; ch; ch = trvch_next(lc))
		{
			int count;

			if(!IS_NPC(ch) || ch->fighting || IS_AFFECTED(ch, AFF_CHARM) || !IS_AWAKE(ch) || ( xIS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) ) || !can_see(ch, wch)) 	continue;
			if(is_hating(ch, wch)) {found_prey(ch, wch); 																												continue;}
			if((!xIS_SET(ch->act, ACT_AGGRESSIVE) && !xIS_SET(ch->act, ACT_META_AGGR)) || xIS_SET(ch->act, ACT_MOUNTED) || xIS_SET(ch->in_room->room_flags, ROOM_SAFE)) continue;

			//Ok we have a 'wch' player character and a 'ch' npc aggressor. Now make the aggressor fight a RANDOM pc victim in the room, giving each 'vch' an equal chance of selection.
			//Depending on flags set, the mob may attack another mob.
			count = 0;
			victim = NULL;
			
			for(vch = wch->in_room->first_person; vch; vch = vch->next_in_room)
			{
				if((!IS_NPC(vch) || xIS_SET(ch->act, ACT_META_AGGR) 
				|| xIS_SET(vch->act, ACT_ANNOYING)) && vch->level < LEVEL_IMMORTAL && (!xIS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch)) && can_see(ch, vch))
				{	
					if(number_range(0, count ) == 0) {victim = vch;}
					++count;
				}
			}

			if(!victim) {bug("%s: null victim. %d", __func__, count); continue;}

			//Backstabbing mobs.
			if(IS_NPC(ch) && xIS_SET(ch->attacks, ATCK_BACKSTAB))
			{
				OBJ_DATA *obj;

				if(!ch->mount && (obj = get_eq_char(ch, WEAR_WIELD)) != NULL && (obj->value[3] == 11 || obj->value[3] == 2) && !victim->fighting && victim->hit >= victim->max_hit)
				{
					check_attacker(ch, victim);
					WAIT_STATE(ch, skill_table[tes_skill_backstab_creature]->beats);
					
					if( !IS_AWAKE(victim) || number_percent() + 5 < ch->level)	{global_retcode = multi_hit( ch, victim, tes_skill_backstab_creature); continue;}
					else														{global_retcode = damage(ch, victim, 0, tes_skill_backstab_creature);  continue;}
				}
			}
			else if(IS_NPC(ch) && xIS_SET(ch->attacks, ATCK_POUNCE))
			{
				if(!ch->mount && !victim->fighting)
				{
					check_attacker(ch, victim);
					if(!IS_AWAKE(victim) || number_percent() + 5 < ch->level)	{global_retcode = multi_hit( ch, victim, tes_action_pounce);continue;}
					else														{global_retcode = damage(ch, victim, 0, tes_action_pounce);	continue;}
				}
			}
			global_retcode = multi_hit(ch, victim, TYPE_UNDEFINED);
		}
		trv_dispose(&lc);
	}
}
void drunk_randoms								(CHAR_DATA *ch){
	//Drunk randoms.
	CHAR_DATA *rvch = NULL;
	CHAR_DATA *vch;
	short drunk;
	short position;

	if(IS_NPC(ch) || ch->pcdata->condition[COND_DRUNK] <= 0)
    return;

	if(number_percent() < 30)
    return;

	drunk = ch->pcdata->condition[COND_DRUNK];
	position = ch->position;
	ch->position = POS_STANDING;

	if(number_percent() < (2 * drunk / 20)) 		check_social(ch, "burp", "");
	else if(number_percent() < (2 * drunk / 20))	check_social(ch, "hiccup", "");
	else if(number_percent() < (2 * drunk / 20))	check_social(ch, "drool", "");
	else if(number_percent() < (2 * drunk / 20))	check_social(ch, "fart", "");
	else if(drunk > (10 + (get_curr_con(ch) / 5)) && number_percent() < (2 * drunk / 18))
	{
		for(vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
			if(number_percent() < 10)
				rvch = vch;
			
		check_social(ch, "puke", ( rvch ? rvch->name : (char *)""));
	}

	ch->position = position;
	return;
}
void hallucinations								(CHAR_DATA *ch){
	//Random hallucinations for those suffering from an overly high mentalstate.
	if(ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0)
	{
		const char *t;

		switch ( number_range( 1, UMIN( 21, ( ch->mental_state + 5 ) / 5 ) ) )
		{
			default:
			case 1:  t = "You feel very restless... you can't sit still.\r\n"; 					break;
			case 2:  t = "You're tingling all over.\r\n"; 										break;
			case 3:	 t = "Your skin is crawling.\r\n"; 											break;
			case 4:	 t = "You suddenly feel that something is terribly wrong.\r\n";				break;
			case 5:	 t = "Those damn little fairies keep laughing at you!\r\n";					break;
			case 6:	 t = "You can hear your mother crying...\r\n";								break;
			case 7:	 t = "Have you been here before, or not?  You're not sure...\r\n";			break;
			case 8:	 t = "Painful childhood memories flash through your mind.\r\n";				break;
			case 9:	 t = "You hear someone call your name in the distance...\r\n";				break;
			case 10: t = "Your head is pulsating... you can't think straight.\r\n";				break;
			case 11: t = "The ground... seems to be squirming...\r\n";							break;
			case 12: t = "You're not quite sure what is real anymore.\r\n";						break;
			case 13: t = "It's all a dream... or is it?\r\n";									break;
			case 14: t = "You hear your grandchildren praying for you to watch over them.\r\n";	break;
			case 15: t = "They're coming to get you... coming to take you away...\r\n";			break;
			case 16: t = "You begin to feel all powerful!\r\n";									break;
			case 17: t = "You're light as air... the heavens are yours for the taking.\r\n";	break;
			case 18: t = "Your whole life flashes by... and your future...\r\n";				break;
			case 19: t = "You are everywhere and everything... you know all and are all!\r\n";	break;
			case 20: t = "You feel immortal!\r\n";												break;
			case 21: t = "Ahh... the power of a Supreme Entity... what to do...\r\n";			break;
		}
		send_to_char(t, ch);
	}
	return;
}
void tele_update								(void){
	TELEPORT_DATA *tele, *tele_next;

	if(!first_teleport) return;
	
	for(tele = first_teleport; tele; tele = tele_next)
	{
		tele_next = tele->next;
		
		if(--tele->timer <= 0)
		{
			if(tele->room->first_person)
			{
				if( xIS_SET(tele->room->room_flags, ROOM_TELESHOWDESC)) teleport(tele->room->first_person, tele->room->tele_vnum, TELE_SHOWDESC | TELE_TRANSALL);
				else													teleport(tele->room->first_person, tele->room->tele_vnum, TELE_TRANSALL);
			}
			UNLINK(tele, first_teleport, last_teleport, next, prev);
			DISPOSE(tele);
		}
	}
}
void auth_update								(void){
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH];
	char log_buf[MAX_STRING_LENGTH];
	bool found_hit = FALSE; /* was at least one found? */

	mudstrlcpy(log_buf, "Pending authorizations:\r\n", MAX_STRING_LENGTH);
	
	for(d = first_descriptor; d; d = d->next)
	{
		if((victim = d->character) && IS_WAITING_FOR_AUTH(victim))
		{
			found_hit = TRUE;
			snprintf(buf, MAX_INPUT_LENGTH, " %s@%s new %s %s\r\n", victim->name, victim->desc->host, race_table[victim->race]->race_name, class_table[victim->Class]->who_name);
			mudstrlcat(log_buf, buf, MAX_STRING_LENGTH);
		}
	}
	
	if(found_hit) to_channel(log_buf, CHANNEL_AUTH, "Auth", 1);
}
void update_handler								(void){
	//Handle all kinds of updates.
	//Called once per pulse from game loop.
	//Random times to defeat tick-timing clients and players.
	static int pulse_passive_fight;
	static int pulse_area;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_second;
	static int pulse_time;
	static int pulse_houseauc;
	struct timeval sttime;
	struct timeval etime;

	if(timechar)
	{
		set_char_color(AT_PLAIN, timechar);
		send_to_char("Starting update timer.\r\n", timechar);
		gettimeofday(&sttime, NULL);
	}

	if(--pulse_passive_fight <= 0)
	{
		pulse_passive_fight = 5 * PULSE_PER_SECOND;
		DREAMWALK_START();
	}

	if(--pulse_houseauc  <= 0)
	{
		pulse_houseauc = 1800 * PULSE_PER_SECOND;
		homebuy_update();
	}

	if(--pulse_area <= 0)
	{
		pulse_area = number_range(PULSE_AREA /2, 3 * PULSE_AREA /2);
		area_update();
	}

	if(--pulse_mobile <= 0)
	{
		pulse_mobile = PULSE_MOBILE;
		mobile_update();
	}

	if( --pulse_violence <= 0 )
	{
		pulse_violence = PULSE_VIOLENCE;
		violence_update(  );
	}

	if(--pulse_time <= 0)
	{
		pulse_time = sysdata.pulsecalendar;
		char_calendar_update();
	}

	if(--pulse_point <= 0)
	{
		pulse_point = number_range((int)(PULSE_TICK * 0.75), (int)(PULSE_TICK * 1.25));

		auth_update();  	/* Gorog */
		time_update();  	/* If looking for slower passing time, move this to just above char_calendar_update(  ); */
		UpdateWeather(); 	/* New Weather Updater -Kayle */
		hint_update();
		char_update();
		obj_update();
		clear_vrooms(); 	/* remove virtual rooms */
	}

	if(--pulse_second <= 0)
	{
		pulse_second = PULSE_PER_SECOND;
		char_check();
		check_dns();
		reboot_check(0);
	}

	if(--auction->pulse <= 0)
	{
		auction->pulse = PULSE_AUCTION;
		auction_update();
	}

	mpsleep_update();  		/* Check for sleeping mud progs -rkb */
	tele_update();
	aggr_update();
	obj_act_update();
	room_act_update();
	clean_obj_queue(); 		/* dispose of extracted objects */
	clean_char_queue();   	/* dispose of dead mobs/quitting chars */
	
	if(timechar)
	{
		gettimeofday(&etime, NULL);
		set_char_color(AT_PLAIN, timechar);
		send_to_char("Update timing complete.\r\n", timechar);
		subtract_times(&etime, &sttime);
		ch_printf(timechar, "Timing took %ld.%06ld seconds.\r\n", (time_t) etime.tv_sec, (time_t) etime.tv_usec);
		timechar = NULL;
	}
	tail_chain();
}
void remove_portal								(OBJ_DATA *portal){
	ROOM_INDEX_DATA *fromRoom, *toRoom;
	EXIT_DATA *pexit;
	bool found;

	if(!portal) {bug("%s", "remove_portal: portal is NULL"); return;}

	fromRoom = portal->in_room;
	found = FALSE;
	
	if(!fromRoom) {bug("%s", "remove_portal: portal->in_room is NULL"); return;}

	for(pexit = fromRoom->first_exit; pexit; pexit = pexit->next)
		if(IS_SET( pexit->exit_info, EX_PORTAL))
		{
			found = TRUE;
			break;
		}	

	if(!found) 							   {bug("remove_portal: portal not found in room %d!", fromRoom->vnum); return;}
	if(pexit->vdir != DIR_PORTAL) 	 		bug("remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir);
	if((toRoom = pexit->to_room) == NULL) 	bug("%s", "remove_portal: toRoom is NULL");

	extract_exit(fromRoom, pexit);
}
void reboot_check								(time_t reset){
	static const char *tmsg[] = 
	{ 
		"You feel the ground shake as the end comes near!",
		"Lightning crackles in the sky above!",
		"Crashes of thunder sound across the land!",
		"The sky has suddenly turned midnight black.",
		"You notice the life forms around you slowly dwindling away.",
		"The seas across the realm have turned frigid.",
		"The aura of magic that surrounds the realms seems slightly unstable.",
		"You sense a change in the magical forces surrounding you."
	};
   
	static const int times[] = {60, 120, 180, 240, 300, 600, 900, 1800};
	static const int timesize = UMIN( sizeof(times) / sizeof(*times), sizeof(tmsg) / sizeof(*tmsg));
	char buf[MAX_STRING_LENGTH];
	static int trun;
	static bool init = FALSE;

	if(!init || reset >= current_time)
	{
		for(trun = timesize - 1; trun >= 0; trun--)
			if(reset >= current_time + times[trun])
				break;
			
		init = TRUE;
		return;
	}

	if(new_boot_time_t - boot_time < 60 * 60 * 18 && !set_boot_time->manual)
    return;

	if(new_boot_time_t <= current_time)
	{
		CHAR_DATA *vch;

		if(auction->item)
		{
			snprintf(buf, MAX_STRING_LENGTH, "Sale of %s has been stopped by mud.", auction->item->short_descr);
			talk_auction(buf);
			obj_to_char(auction->item, auction->seller);
			auction->item = NULL;
			
			if(auction->buyer && auction->buyer != auction->seller)
			{
				auction->buyer->gold += auction->bet;
				send_to_char("Your money has been returned.\r\n", auction->buyer);
			}
		}
		
		echo_to_all(AT_YELLOW, 	"You are forced from these realms by a strong "
								"magical presence\r\nas life here is reconstructed.", ECHOTAR_ALL);
		log_string("Automatic Reboot");
		
		for(vch = first_char; vch; vch = vch->next)
			if(!IS_NPC(vch))
				save_char_obj(vch);
			
		mud_down = TRUE;
		return;
	}

	if(trun != -1 && new_boot_time_t - current_time <= times[trun])
	{
		echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
		
		if(trun <= 5)
			sysdata.DENY_NEW_PLAYERS = TRUE;
		
		--trun;
		return;
	}
}
void auction_update								(void){
	//The auction update
	int tax, pay;
	char buf[MAX_STRING_LENGTH];

	if(!auction->item)
	{
		if(AUCTION_MEM > 0 && auction->history[0] && ++auction->hist_timer == 6 * AUCTION_MEM)
		{
			int i;

			for(i = AUCTION_MEM - 1; i >= 0; i--)
			{
				if(auction->history[i])
				{
					auction->history[i] = NULL;
					auction->hist_timer = 0;
					break;
				}
			}
		}
		return;
	}

	//Increase the going state
	switch (++auction->going)
	{
		case 1: /* going once */
		case 2: /* going twice */
			if(auction->bet > auction->starting) 
				snprintf(buf, MAX_STRING_LENGTH, "%s: going %s for %s.", auction->item->short_descr, ((auction->going == 1) ? "once" : "twice"), num_punct( auction->bet));
			else
				snprintf(buf, MAX_STRING_LENGTH, "%s: going %s (bid not received yet).", auction->item->short_descr, ((auction->going == 1 ) ? "once" : "twice"));

			talk_auction(buf);
			break;

		case 3: /* SOLD! */
			if(!auction->buyer && auction->bet)
			{
				bug( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );
				auction->bet = 0;
			}
			
			if(auction->bet > 0 && auction->buyer != auction->seller)
			{
				snprintf(buf, MAX_STRING_LENGTH, "%s sold to %s for %s.", 
					auction->item->short_descr, IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name, num_punct(auction->bet));
					
				talk_auction(buf);

				act(AT_ACTION, "The auctioneer materializes before you, and hands you $p.", auction->buyer, auction->item, NULL, TO_CHAR);
				act(AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.", 	auction->buyer, auction->item, NULL, TO_ROOM );

				if((auction->buyer->carry_weight + get_obj_weight(auction->item)) > can_carry_w( auction->buyer))
				{
					act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
					act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", 	auction->buyer, auction->item, NULL, TO_ROOM );
					obj_to_room( auction->item, auction->buyer->in_room );
				}
				else obj_to_char(auction->item, auction->buyer);
				
				pay = (int)(auction->bet * 0.9);
				tax = (int)(auction->bet * 0.1);
				boost_economy(auction->seller->in_room->area, tax);
				auction->seller->gold += pay; /* give him the money, tax 10 % */
				ch_printf(auction->seller, "The auctioneer pays you %s gold, charging an auction fee of ", num_punct(pay));
				ch_printf(auction->seller, "%s.\r\n", num_punct(tax));

				auction->item = NULL;   /* reset item */
				
				if(IS_SET(sysdata.save_flags, SV_AUCTION))
				{
					save_char_obj(auction->buyer);
					save_char_obj(auction->seller);
				}
			}
			else  /* not sold */
			{
				snprintf(buf, MAX_STRING_LENGTH, "No bids received for %s - removed from auction.", auction->item->short_descr);
				talk_auction(buf);
				act(AT_ACTION, "The auctioneer appears before you to return $p to you.", auction->seller, auction->item, NULL, TO_CHAR);
				act(AT_ACTION, "The auctioneer appears before $n to return $p to $m.", 	 auction->seller, auction->item, NULL, TO_ROOM);

				if((auction->seller->carry_weight + get_obj_weight(auction->item)) > can_carry_w( auction->seller))
				{
					act(AT_PLAIN, "You drop $p as it is just too much to carry with everything else you're carrying.", 	auction->seller, auction->item, NULL, TO_CHAR);
					act(AT_PLAIN, "$n drops $p as it is too much extra weight for $m with everything else.", 			auction->seller, auction->item, NULL, TO_ROOM);
					obj_to_room( auction->item, auction->seller->in_room );
				}
				else obj_to_char(auction->item, auction->seller);

				tax = (int)(auction->item->cost * 0.05);
				boost_economy(auction->seller->in_room->area, tax);
				ch_printf(auction->seller, "The auctioneer charges you an auction fee of %s.\r\n", num_punct(tax));
				
				if((auction->seller->gold - tax) < 0) 	auction->seller->gold = 0;
				else									auction->seller->gold -= tax;
				
				if(IS_SET(sysdata.save_flags, SV_AUCTION))
				save_char_obj( auction->seller );
			}
			auction->item = NULL;   /* clear auction */
	} 
} 
void subtract_times								(struct timeval *etime, struct timeval *sttime){
	etime->tv_sec -= sttime->tv_sec;
	etime->tv_usec -= sttime->tv_usec;
	
	while( etime->tv_usec < 0 )
	{
		etime->tv_usec += 1000000;
		etime->tv_sec--;
	}
}
void time_update								(void){
	//update the time
	DESCRIPTOR_DATA *d;
	int n;
	const char *echo; /* echo string */
	int echo_color;   /* color for the echo */

	n = number_bits(2);
	echo = NULL;
	echo_color = AT_GREY;

	++time_info.hour; 

	if(time_info.hour == sysdata.hourdaybegin 
	|| time_info.hour == sysdata.hoursunrise
    || time_info.hour == sysdata.hournoon 
	|| time_info.hour == sysdata.hoursunset
    || time_info.hour == sysdata.hournightbegin)
	{
		for(d = first_descriptor; d; d = d->next)
		{
			if(d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && !NO_WEATHER_SECT(d->character->in_room->sector_type) && IS_AWAKE(d->character))
			{
				struct WeatherCell *cell = getWeatherCell(d->character->in_room->area);

				switch( time_info.hour )
				{
					case 6:
					{
						const char *echo_strings[4] = 
						{
							"The day has begun.\r\n",
							"The day has begun.\r\n",
							"The sky slowly begins to glow.\r\n",
							"The sun slowly embarks upon a new day.\r\n"
						};
					
						time_info.sunlight = SUN_RISE;
						echo = echo_strings[n];
						echo_color = AT_YELLOW;
						break;
					}

					case 7:
					{
						const char *echo_strings[4] = 
						{
							"The sun rises in the east.\r\n",
							"The sun rises in the east.\r\n",
							"The hazy sun rises over the horizon.\r\n",
							"Day breaks as the sun lifts into the sky.\r\n"
						};
						
						time_info.sunlight = SUN_LIGHT;
						echo = echo_strings[n];
						echo_color = AT_ORANGE;
						break;
					}

					case 12:
					{
						if(getCloudCover(cell) > 21)
						{
							echo = "It's noon.\r\n";
						}
						else
						{
							const char *echo_strings[2] = 
							{
								"The intensity of the sun heralds the noon hour.\r\n",
								"The sun's bright rays beat down upon your shoulders.\r\n"
							};

							echo = echo_strings[n % 2];
						}
						
						time_info.sunlight = SUN_LIGHT;
						echo_color = AT_WHITE;
						break;
					}

					case 18:
					{
						const char *echo_strings[4] = 
						{
							"The sun slowly disappears in the west.\r\n",
							"The reddish sun sets past the horizon.\r\n",
							"The sky turns a reddish orange as the sun ends its journey.\r\n",
							"The sun's radiance dims as it sinks in the sky.\r\n"
						};
						
						time_info.sunlight = SUN_SET;
						echo = echo_strings[n];
						echo_color = AT_RED;
						break;
					}

					case 19:
					{
						if( getCloudCover(cell) > 21)
						{
							const char *echo_strings[2] = 
							{
								"The night begins.\r\n",
								"Twilight descends around you.\r\n"
							};

							echo = echo_strings[n % 2];
						}
						else
						{
							const char *echo_strings[2] = 
							{
								"The moon's gentle glow diffuses through the night sky.\r\n",
								"The night sky gleams with glittering starlight.\r\n"
							};

							echo = echo_strings[n % 2];
						}
						
						time_info.sunlight = SUN_DARK;
						echo_color = AT_DBLUE;
						break;
					}
				}

				if(!echo) continue;
				
				set_char_color(echo_color, d->character);
				send_to_char(echo, d->character);
			}
		}
	}

	if(time_info.hour == sysdata.hourmidnight)
	{
		time_info.hour = 0;
		time_info.day++;
		RandomizeCells();
	}

	if(time_info.day >= sysdata.dayspermonth)
	{
		time_info.day = 0;
		time_info.month++;
	}

	if(time_info.month >= sysdata.monthsperyear)
	{
		time_info.month = 0;
		time_info.year++;
	}
	
	calc_season();
	save_timedata();
}
void hint_update								(){
	DESCRIPTOR_DATA *d;

	if(time_info.hour % 1 == 0)
	{
		for(d = first_descriptor; d; d = d->next)
		{
			if(d->connected == CON_PLAYING && IS_AWAKE( d->character ) && d->character->pcdata)
			{
				if(IS_SET( d->character->pcdata->flags, PCFLAG_HINTS) && number_bits(1) == 0)
				{
					if(d->character->level > LEVEL_AVATAR) 	ch_printf_color(d->character, "&p( &wHINT&p ):  &P%s\r\n", get_hint(LEVEL_AVATAR));
					else									ch_printf_color(d->character, "&p( &wHINT&p ):  &P%s\r\n", get_hint(d->character->level));
				}
			}
		}
	}
}


