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
 *                            Spell handling module!                        *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

//Local functions.
static int bsearch_skill_prefix	(const char *name, int first, int top);
static int bsearch_skill_exact	(const char *name, int first, int top);
void say_spell					(CHAR_DATA *ch, int sn );
ch_ret spell_affect				(int sn, int level, CHAR_DATA *ch, void *vo);
ch_ret spell_affectchar			(int sn, int level, CHAR_DATA *ch, void *vo);
int dispel_casting				(AFFECT_DATA *paf, CHAR_DATA *ch, CHAR_DATA *victim, int affect, bool dispel);
bool can_charm					(CHAR_DATA *ch);
bool is_immune					(CHAR_DATA *ch, short damtype){									//Immunity to certain damage.
	//Honor temporary immunity overrides, just as ris_save() does.
	switch (damtype)
	{
		case SD_FIRE:			return IS_SET(ch->immune, RIS_FIRE) && !IS_SET(ch->no_immune, RIS_FIRE);
		case SD_COLD:			return IS_SET(ch->immune, RIS_COLD) && !IS_SET(ch->no_immune, RIS_COLD);
		case SD_ELECTRICITY:	return IS_SET(ch->immune, RIS_ELECTRICITY) && !IS_SET(ch->no_immune, RIS_ELECTRICITY);
		case SD_ENERGY:			return IS_SET(ch->immune, RIS_ENERGY) && !IS_SET(ch->no_immune, RIS_ENERGY);
		case SD_ACID:			return IS_SET(ch->immune, RIS_ACID) && !IS_SET(ch->no_immune, RIS_ACID);
		case SD_POISON:			return IS_SET(ch->immune, RIS_POISON) && !IS_SET(ch->no_immune, RIS_POISON);
		case SD_DRAIN:			return IS_SET(ch->immune, RIS_DRAIN) && !IS_SET(ch->no_immune, RIS_DRAIN);
	}
	return FALSE;
}
//--------------------------------------------------------------------------------------------------//
// MAGIC COMBAT: low-frequency atmospheric casting and meaningful concentration.
//--------------------------------------------------------------------------------------------------//
static int MAGIC_CONCENTRATION			(CHAR_DATA *ch, int sn)							{
	int skill = ch->pcdata->learned[sn];
	int fatigue;

	//Outside combat, spell proficiency behaves exactly as before.
	if(!ch->fighting || ch->max_move <= 0) return skill;

	fatigue = (ch->move * 100) / ch->max_move;

	if(fatigue < 20)		skill -= 9;
	else if(fatigue < 40)	skill -= 4;

	//A guarded caster concentrates better; an aggressive one takes more risks.
	if(ch->position == POS_DEFENSIVE)	skill += 3;
	if(ch->position == POS_AGGRESSIVE)	skill -= 3;
	if(ch->position == POS_BERSERK)	skill -= 7;

	return URANGE(1, skill, 100);
}
static void MAGIC_CAST_FLOURISH		(CHAR_DATA *ch, CHAR_DATA *victim, SKILLTYPE *skill){
	if(!victim || !ch->in_room || ch->in_room != victim->in_room) return;
	if(victim == ch || number_percent() > 24) return; //Keep large battles readable.

	if(skill->target == TAR_CHAR_DEFENSIVE)
	{
		act(AT_MAGIC, "&CYou trace a protective pattern of magicka toward $N.&D", ch, NULL, victim, TO_CHAR);
		act(AT_MAGIC, "&CA soft shimmer of magicka moves from $n toward you.&D", ch, NULL, victim, TO_VICT);
		act(AT_MAGIC, "&CA pale shimmer of magicka forms between $n and $N.&D", ch, NULL, victim, TO_NOTVICT);
		return;
	}
	if(skill->target != TAR_CHAR_OFFENSIVE) return;

	switch(SPELL_DAMAGE(skill))
	{
		case SD_FIRE:
			act(AT_MAGIC, "&RFlames curl around your hands as you direct your spell at $N.&D", ch, NULL, victim, TO_CHAR);
			act(AT_MAGIC, "&RFlames flare around $n's hands as $e turns toward you!&D", ch, NULL, victim, TO_VICT);
			act(AT_MAGIC, "&RFlames spring to life around $n as $e targets $N.&D", ch, NULL, victim, TO_NOTVICT);
			break;
		case SD_COLD:
			act(AT_MAGIC, "&CA veil of frost spreads from your fingertips toward $N.&D", ch, NULL, victim, TO_CHAR);
			act(AT_MAGIC, "&CThe air chills as frost gathers around $n's hands.&D", ch, NULL, victim, TO_VICT);
			act(AT_MAGIC, "&CA trail of frost curls through the air from $n toward $N.&D", ch, NULL, victim, TO_NOTVICT);
			break;
		case SD_ELECTRICITY:
			act(AT_MAGIC, "&YLightning crackles between your fingers as you face $N.&D", ch, NULL, victim, TO_CHAR);
			act(AT_MAGIC, "&YSparks leap across $n's hands as $e focuses on you!&D", ch, NULL, victim, TO_VICT);
			act(AT_MAGIC, "&YArcs of lightning flicker around $n as $e targets $N.&D", ch, NULL, victim, TO_NOTVICT);
			break;
		case SD_POISON:
			act(AT_MAGIC, "&GA sickly green mist gathers as you reach toward $N.&D", ch, NULL, victim, TO_CHAR);
			act(AT_MAGIC, "&GA faint, poisonous haze rises around $n's hands.&D", ch, NULL, victim, TO_VICT);
			act(AT_MAGIC, "&GA greenish haze curls away from $n toward $N.&D", ch, NULL, victim, TO_NOTVICT);
			break;
		default:
			act(AT_MAGIC, "&CMagicka gathers in your palms as you focus on $N.&D", ch, NULL, victim, TO_CHAR);
			act(AT_MAGIC, "&CThe air shimmers as $n channels magicka toward you.&D", ch, NULL, victim, TO_VICT);
			act(AT_MAGIC, "&CA ripple of magicka moves between $n and $N.&D", ch, NULL, victim, TO_NOTVICT);
			break;
	}
}
static void MAGIC_PARTIAL_SAVE			(CHAR_DATA *ch, CHAR_DATA *victim)				{
	if(number_percent() > 25) return;
	act(AT_MAGIC, "&WYou brace yourself and resist part of $n's spell.&D", ch, NULL, victim, TO_VICT);
	act(AT_MAGIC, "&W$N manages to resist part of your spell.&D", ch, NULL, victim, TO_CHAR);
}
int ch_slookup					(CHAR_DATA *ch, const char *name){												//Lookup a skill by name, only stopping at skills the player has.
	int sn;
	sn = skill_lookup(name);
	
	//Does this skill even exist?
	if(sn == -1) return -1;

	//Make sure that: 1. ch knows this skill 2. ch's level is high enough
	if(IS_NPC(ch)) return sn;
	if(ch->pcdata->learned[sn] > 0 && (ch->level >= skill_table[sn]->skill_level[ch->Class] || ch->level >= skill_table[sn]->race_level[ch->race])) {return sn;}
	else {return -1;}  //nope... ch doesn't have this skill.

	//Doing the below is silly -- it means a linear lookup over the entire skill table. (why???)
	//It is much more efficient to simply do the normal binary search, and *then* make sure that ch has it.
#if 0
	if(IS_NPC(ch)) return skill_lookup(name);
	
	for(sn = 0; sn < top_sn; sn++)
	{
		if(!skill_table[sn]->name) break;
		if(ch->pcdata->learned[sn] > 0 && ch->level >= skill_table[sn]->skill_level[ch->Class] && LOWER( name[0]) == LOWER(skill_table[sn]->name[0]) && !str_prefix(name, skill_table[sn]->name))
        return sn;
	}
	return -1;
#endif
}
int herb_lookup					(const char *name) {															//Lookup an herb by name.
	int sn;
	for(sn = 0; sn < top_herb; sn++)
	{
		if(!herb_table[sn] || !herb_table[sn]->name) 														return -1;
		if(LOWER( name[0]) == LOWER(herb_table[sn]->name[0]) && !str_prefix(name, herb_table[sn]->name))	return sn;
	}
   return -1;
}
int personal_lookup				(CHAR_DATA *ch, const char *name ){												//Lookup a personal skill.
	//Unused for now.  In place to allow a player to have a custom spell/skill.
	//When this is put in make sure you put in cleanup code if you do any sort of allocating memory in free_char.
	int sn;

	if(!ch->pcdata) return -1;
	
	for(sn = 0; sn < MAX_PERSONAL; sn++)
	{
		if(!ch->pcdata->special_skills[sn] || !ch->pcdata->special_skills[sn]->name) 														return -1;
		if(LOWER( name[0] ) == LOWER( ch->pcdata->special_skills[sn]->name[0]) && !str_prefix(name, ch->pcdata->special_skills[sn]->name))	return sn;
	}
	return -1;
}
int skill_lookup				(const char *name){																//Lookup a skill by name.
	//First tries to find an exact match. Then looks for a prefix match.
   int sn;

   // Try to find an exact match for this skill.
   if( ( sn = bsearch_skill_exact( name, 0, num_sorted_skills - 1 ) ) == -1 )
   {
      // We failed to find an exact match;
      // so try to find a prefix match.
      if( ( sn = bsearch_skill_prefix( name, 0, num_sorted_skills - 1 ) ) == -1 )
      {
         // We failed to find the skill in the sorted skills. It is
         // possible that the skill was added after the game booted;
         // so try looking through the unsorted skills.
         for( sn = num_sorted_skills; sn < num_skills; ++sn )
         {
            // have we reached the end of the skills?
            if( !skill_table[sn] || !skill_table[sn]->name )
            {
               bug( "%s: WARNING: skill table entry %d had bad skill", __func__, sn );
               return -1;
            }

            // is this the one we want?
            if( !str_prefix( name, skill_table[sn]->name ) )
               return sn;
         }
         return -1;
      }
   }
   return sn;
}
SKILLTYPE *get_skilltype		(int sn){																		//Return a skilltype pointer based on sn.
	//Returns NULL if bad, unused or personal sn.
	//This seems to be unaffected by resorting the skill table.
	if(sn >= TYPE_PERSONAL) return NULL;
	if(sn >= TYPE_HERB)		return IS_VALID_HERB(sn - TYPE_HERB) ? herb_table[sn - TYPE_HERB] : NULL;
	if(sn >= TYPE_HIT)		return NULL;
	return IS_VALID_SN(sn) ? skill_table[sn] : NULL;
}
static int bsearch_skill_prefix	(const char *name, int first, int top){											//Perform a binary search on a section of the skill table.
	//Each different section of the skill table is sorted alphabetically.
	//Check for prefix matches.
	//Made static by dchaley 2007-06-22. Nobody has any business calling the binary search directly; they should only use the skill_lookup or find_* functions. 
	//(A practical reason is that a bsearch call will prevent you from finding skills in the interval used for skills created after MUD boot.)	
	int sn;

	for(;;)
	{
		sn = (first + top) >> 1;
		
		if(!IS_VALID_SN(sn))																				return -1;
		if(LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) && !str_prefix(name, skill_table[sn]->name)) 	return sn;
		if(first >= top) 																					return -1;
		
		if(strcasecmp(name, skill_table[sn]->name) < 1)   top = sn - 1;
		else											first = sn + 1;
   }
}
static int bsearch_skill_exact	(const char *name, int first, int top){											//Perform a binary search on a section of the skill table.
	//Each different section of the skill table is sorted alphabetically.
	//Check for exact matches only.
	//Made static by dchaley 2007-06-22. Nobody has any business calling the binary search directly; they should only use the skill_lookup or find_* functions. 
	//(A practical reason is that a bsearch call will prevent you from finding skills in the interval used for skills created after MUD boot.)
	int sn;
	for(;;)
	{
		sn = (first + top) >> 1;

		if(!IS_VALID_SN(sn)) 							return -1;
		if(!strcasecmp(name, skill_table[sn]->name))	return sn;
		if(first >= top)						 		return -1;
		
		if(strcasecmp(name, skill_table[sn]->name) < 1)	  top = sn - 1;
		else											first = sn + 1;
	}
}
int find_skill_of_type			(CHAR_DATA *ch, const char *name, bool know, int expectedType){					//Look up a skill by name. 
	//If 'ch' is a PC and 'know' is true, make sure that ch actually knows the skill.
	//If 'ch' is not specified, or if 'ch' is an NPC, or if 'know' is false, just look up the skill by name.
	//If the skill was found, further check to make sure that its type was of type' expectedType'.
	//On success, returns the skill number. On failure, returns -1.
	int sn;

	if(!ch || IS_NPC(ch) || !know) sn = skill_lookup(name);
	else						   sn = ch_slookup(ch, name);

   // did we fail to find this skill?
   if(sn == -1) return -1;

   // make sure that the found skill is of the expected type.
   return (skill_table[sn]->type == expectedType ? sn : -1);
}
int find_spell					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_SPELL );
}
int find_skill					(CHAR_DATA *ch, const char *name, bool know){
	return find_skill_of_type( ch, name, know, SKILL_SKILL );
}
int find_ability				(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_RACIAL );
}
int find_weapon					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_WEAPON );
}
int find_tongue					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_TONGUE );
}
int find_action					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_ACTION );
}
int find_defence				(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_DEFENCE );
}
int find_school					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_SCHOOL );
}
int find_attack					(CHAR_DATA *ch, const char *name, bool know){
   return find_skill_of_type( ch, name, know, SKILL_ATTACK );
}
int slot_lookup					(int slot){																		//Lookup a skill by slot number. (Used for object loading)
	extern bool fBootDb;
	int sn;
	
	if(slot <= 0) return -1;
		for(sn = 0; sn < num_skills; ++sn)
			if(slot == skill_table[sn]->slot) return sn;
		
			if(fBootDb) {bug("%s: bad slot %d.", __func__, slot); abort();}
			return -1;
}
int dispel_casting				(AFFECT_DATA *paf, CHAR_DATA *ch, CHAR_DATA *victim, int affect, bool dispel){	//Handler to tell the victim which spell is being affected.
	char buf[MAX_STRING_LENGTH];
	const char *spell;
	SKILLTYPE *sktmp;
	bool is_mage = FALSE, has_detect = FALSE;
	EXT_BV ext_bv = meb(affect);

	if(IS_NPC(ch) || ch->Class == CLASS_MAGE) is_mage 	 = TRUE;
	if(IS_AFFECTED(ch, AFF_DETECT_MAGIC)) 	  has_detect = TRUE;
	if(paf)
	{
		if((sktmp = get_skilltype(paf->type)) == NULL) return 0;
		spell = sktmp->name;
	}
	else spell = affect_bit_name(&ext_bv);

	set_char_color(AT_MAGIC, ch);
	set_char_color(AT_HITME, victim);

	if(!can_see(ch, victim)) 	
		mudstrlcpy(buf, "Someone", MAX_STRING_LENGTH);
	else
	{
		mudstrlcpy(buf, (IS_NPC(victim) ? victim->short_descr : victim->name), MAX_STRING_LENGTH);
		buf[0] = toupper(buf[0]);
	}

	if(dispel)
	{
		ch_printf(victim, "Your %s vanishes.\r\n", spell);
		if(is_mage && has_detect) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s's %s vanishes.", buf, spell); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		else return 0;   /* So we give the default Ok. Message */
	}
	else
	{
		if(is_mage && has_detect) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s's %s wavers but holds.", buf, spell); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		else return 0;   /* The wonderful Failed. Message */
	}
	return 1;
}
void successful_casting			(SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj){			//Fancy message handling for a successful casting.
	short chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
	short chit = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);

	if(skill->target != TAR_CHAR_OFFENSIVE) {chit = chitroom;}

	if(ch && ch != victim)
	{
		if(skill->hit_char && skill->hit_char[0] != '\0')
		{
			if(str_cmp(skill->hit_char, SPELL_SILENT_MARKER)) 
				act(AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->type == SKILL_SPELL)
			act(AT_MAGIC, "&CYour incantation takes hold.&D", ch, NULL, NULL, TO_CHAR);
	}
	
	if(ch && skill->hit_room && skill->hit_room[0] != '\0' && str_cmp(skill->hit_room, SPELL_SILENT_MARKER))
    act( AT_MAGIC, skill->hit_room, ch, obj, victim, TO_NOTVICT );
	
	if(ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0')
	{
		if(str_cmp(skill->hit_vict, SPELL_SILENT_MARKER))
		{
			if(ch != victim) act(AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_VICT);
			else             act(AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_CHAR);
		}
	}
	else if(ch && ch == victim && skill->type == SKILL_SPELL)
		act(AT_MAGIC, "&CMagicka settles around you as the spell takes hold.&D", ch, NULL, NULL, TO_CHAR);
	else if(ch && ch == victim && skill->type == SKILL_SKILL)
	{
		if(skill->hit_char && (skill->hit_char[0] != '\0')) act(chit, skill->hit_char, ch, obj, victim, TO_CHAR);
		else												act(chit, "Ok.", ch, NULL, NULL, TO_CHAR);
	}
}
void failed_casting				(SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj){			//Fancy message handling for a failed casting.
	short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
	short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );

	if(skill->target != TAR_CHAR_OFFENSIVE) {chitme = chitroom;}

	if(ch && ch != victim)
	{
		if(skill->miss_char && skill->miss_char[0] != '\0')
		{
			if(str_cmp(skill->miss_char, SPELL_SILENT_MARKER))
				act(AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->type == SKILL_SPELL)
			act(chitme, "&CYour spell disperses without taking hold.&D", ch, NULL, NULL, TO_CHAR);
   }
   
	if(ch && skill->miss_room && skill->miss_room[0] != '\0' && str_cmp(skill->miss_room, SPELL_SILENT_MARKER) && str_cmp(skill->miss_room, "supress"))
    act(AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOTVICT);

	if(ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0')
	{
		if(str_cmp(skill->miss_vict, SPELL_SILENT_MARKER))
		{
			if(ch != victim) act(AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT);
			else			 act(AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR);
		}
	}
	else if(ch && ch == victim)
   {
		if(skill->miss_char && skill->miss_char[0] != '\0')
		{
			if(str_cmp(skill->miss_char, SPELL_SILENT_MARKER))
            act(AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->type == SKILL_SPELL)
			act(chitme, "&CYour spell disperses without taking hold.&D", ch, NULL, NULL, TO_CHAR);
	}
}
void immune_casting				(SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ){			//Fancy message handling for being immune to something.

	short chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
	short chit 	  = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);

	if(skill->target != TAR_CHAR_OFFENSIVE)
	{chit = chitroom;}

	if(ch && ch != victim)
	{
		if(skill->imm_char && skill->imm_char[0] != '\0')
		{
			if(str_cmp(skill->imm_char, SPELL_SILENT_MARKER))
				act(AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR);
		}
		else if( skill->miss_char && skill->miss_char[0] != '\0' )
		{
			if(str_cmp(skill->miss_char, SPELL_SILENT_MARKER))
				act( AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->type == SKILL_SPELL || skill->type == SKILL_SKILL)
			act(chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR);
	}
	
	if(ch && skill->imm_room && skill->imm_room[0] != '\0')
	{
		if(str_cmp( skill->imm_room, SPELL_SILENT_MARKER)) 					
			act(AT_MAGIC, skill->imm_room, ch, obj, victim, TO_NOTVICT);
	}
	else if(ch && skill->miss_room && skill->miss_room[0] != '\0')
	{
      if(str_cmp( skill->miss_room, SPELL_SILENT_MARKER)) 					
		act( AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOTVICT);
	}
	
	if(ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0')
	{
		if(str_cmp(skill->imm_vict, SPELL_SILENT_MARKER))
		{
			if(ch != victim) 	act(AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_VICT);
			else				act(AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_CHAR);
		}
	}
	else if(ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0')
	{
		if(str_cmp(skill->miss_vict, SPELL_SILENT_MARKER))
		{
			if(ch != victim)	act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT);
			else				act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR);
      }
	}
	else if(ch && ch == victim)
	{
		if(skill->imm_char && skill->imm_char[0] != '\0')
		{
			if(str_cmp(skill->imm_char, SPELL_SILENT_MARKER))
				act(AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->miss_char && skill->miss_char[0] != '\0')
		{
			if(str_cmp(skill->miss_char, SPELL_SILENT_MARKER))
				act(AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR);
		}
		else if(skill->type == SKILL_SPELL || skill->type == SKILL_SKILL)
			act(chit, "Your spell has no effect.", ch, NULL, NULL, TO_CHAR);
	}
}
void say_spell					(CHAR_DATA *ch, int sn){												//Latin incantations for successful spellcasting.
	char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	const char *words = "Vis arcana, mihi pare!";
	SKILLTYPE *skill = get_skilltype(sn);
	int i;

	if(!ch || !ch->in_room || !skill || !skill->name) return;

	//All TES creatures carry ACT_PET; non-pet humanoid NPCs and players can speak.
	if(IS_NPC(ch) && xIS_SET(ch->act, ACT_PET)) return;

	//Spell families share incantations, so related effects sound consistent.
	mudstrlcpy(name, skill->name, sizeof(name));
	for(i = 0; name[i]; i++) name[i] = (char)tolower((unsigned char)name[i]);

	if(strstr(name, "fortify"))
	{
		     if(strstr(name, "agility"))		words = "Agilior fiam!";
		else if(strstr(name, "speed"))		words = "Celerior fiam!";
		else if(strstr(name, "strength"))	words = "Fortior fiam!";
		else if(strstr(name, "endurance"))	words = "Firmior fiam!";
		else if(strstr(name, "intelligence"))	words = "Mens mea acuatur!";
		else if(strstr(name, "willpower"))	words = "Voluntas mea roboretur!";
		else if(strstr(name, "luck"))		words = "Fortuna mihi faveat!";
		else if(strstr(name, "personality"))	words = "Gratia mea crescat!";
		else					words = "Virtus mea augeatur!";
	}
	else if(strstr(name, "shield") || strstr(name, "barrier") || strstr(name, "ward"))
		words = "Scutum arcanum me protegat!";
	else if(strstr(name, "restore") || strstr(name, "heal") || strstr(name, "cure"))
		words = "Sanitas redeat!";
	else if(strstr(name, "absorb"))		words = "Vires tuas mihi sumam!";
	else if(strstr(name, "drain"))		words = "Vires tuae deficiant!";
	else if(strstr(name, "fire") || strstr(name, "flame") || strstr(name, "burn"))
		words = "Praesentiam tuam flammis exuram!";
	else if(strstr(name, "frost") || strstr(name, "cold"))
		words = "Frigore te vincam!";
	else if(strstr(name, "shock") || strstr(name, "lightning"))
		words = "Fulmen te feriat!";
	else if(strstr(name, "poison"))		words = "Venenum te consumat!";
	else if(strstr(name, "paraly"))		words = "Membra tua rigescant!";
	else if(strstr(name, "levitate") || strstr(name, "fly"))
		words = "Super terram surgam!";
	else if(strstr(name, "invisib") || strstr(name, "chameleon"))
		words = "Ex oculis evanescam!";
	else if(strstr(name, "summon") || strstr(name, "conjur"))
		words = "Spiritus, ad me veni!";
	else if(strstr(name, "detect"))		words = "Occulta mihi pateant!";
	else if(strstr(name, "dispel"))		words = "Magia aliena dissolvatur!";
	else if(strstr(name, "resist"))		words = "Nulla vis me frangat!";
	else if(strstr(name, "water"))		words = "Aqua mihi pareat!";
	else if(strstr(name, "light"))		words = "Lux tenebras pellat!";
	else if(strstr(name, "silence"))		words = "Vox tua taceat!";
	else
	{
		switch(SPELL_DAMAGE(skill))
		{
			case SD_FIRE:		words = "Praesentiam tuam flammis exuram!"; break;
			case SD_COLD:		words = "Frigore te vincam!"; break;
			case SD_ELECTRICITY:	words = "Fulmen te feriat!"; break;
			case SD_POISON:		words = "Venenum te consumat!"; break;
			default:
				if(skill->target == TAR_CHAR_OFFENSIVE)		words = "Potestas mea te feriat!";
				else if(skill->target == TAR_CHAR_DEFENSIVE)	words = "Vis arcana te tueatur!";
				break;
		}
	}

	snprintf(buf, sizeof(buf), "&CYou utter the words: '&W%s&C'.&D", words);
	act(AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR);
	snprintf(buf, sizeof(buf), "&C$n utters the words: '&W%s&C'.&D", words);
	act(AT_MAGIC, buf, ch, NULL, NULL, TO_ROOM);
}

int ris_save					(CHAR_DATA *ch, int schance, int ris){											//Make adjustments to saving throw based in RIS.
	short modifier;

	modifier = 10;
	if(IS_SET(ch->immune, ris) && !IS_SET(ch->no_immune, ris)) 				modifier -= 10;
	if(IS_SET(ch->resistant, ris) && !IS_SET(ch->no_resistant, ris))		modifier -= 2;
	if(IS_SET(ch->susceptible, ris) && !IS_SET(ch->no_susceptible, ris))
	{
		if(IS_NPC(ch) && IS_SET(ch->immune, ris))							modifier += 0;
		else																modifier += 2;
	}
   
	if(modifier <= 0)	return 1000;
	if(modifier == 10)	 return schance;
	
	return (schance * modifier) /10;
}
int rd_parse					(CHAR_DATA *ch, int level, char *texp){											//Fancy dice expression parsing complete with order of operations.
	//Used for spell dice parsing, ie: 3d8+L-6
	//Simple exponent support, dice support as well as a few extra variables: 
	//L = level, H = hp, M = mana, V = move, S = str, X = dex I = int, W = wis, C = con, A = cha, U = luck, A = age
	unsigned int x;
	int lop = 0, gop = 0, eop = 0;
	char operation;
	char *sexp[2];
	int total = 0;
	unsigned int len = 0;

	//Take care of nulls coming in. 
	if(!texp || !strlen(texp)) return 0;

	//Get rid of brackets if they surround the entire expresion.
	if((*texp == '(') && texp[strlen(texp) - 1] == ')')
	{
		texp[strlen(texp) - 1] = '\0';
		texp++;
	}

	//Check if the expresion is just a number 
	len = strlen(texp);
	if(len == 1 && isalpha(texp[0]))
	{
		switch (texp[0])
		{
			case 'L':
			case 'l': return level;
			case 'H':
			case 'h': return ch->hit;
			case 'M':
			case 'm': return ch->mana;
			case 'V':
			case 'v': return ch->move;
			case 'S':
			case 's': return get_curr_str(ch);
			case 'I':
			case 'i': return get_curr_int(ch);
			case 'W':
			case 'w': return get_curr_wis(ch);
			case 'X':
			case 'x': return get_curr_dex(ch);
			case 'C':
			case 'c': return get_curr_con(ch);
			case 'A':
			case 'a': return get_curr_cha(ch);
			case 'U':
			case 'u': return get_curr_lck(ch);
			case 'Y':
			case 'y': return calculate_age(ch);
		}
	}

	for(x = 0; x < len; ++x)
		if(!isdigit( texp[x]) && !isspace( texp[x])) break;
		
	if(x == len) return atoi(texp);

	//Break it into 2 parts 
	for(x = 0; x < strlen(texp); ++x)
		switch (texp[x])
		{
			case '^': if(!total) eop = x;
            break;
			case '-':
			case '+': if(!total) lop = x;
            break;
			case '*':
			case '/':
			case '%':
			case 'd':
			case 'D':
			case '<':
			case '>':
			case '{':
			case '}':
			case '=': if(!total) gop = x; 	break;
			case '(': ++total; 				break;
			case ')': --total;				break;
		}
		
	     if(lop) x = lop;
	else if(gop) x = gop;
	else         x = eop;
	
	operation = texp[x];
	texp[x] = '\0';
	sexp[0] = texp;
	sexp[1] = (char *)(texp + x + 1);

	//Work it out 
	total = rd_parse(ch, level, sexp[0]);
	
	switch (operation)
	{
		case '-': total -= rd_parse(ch, level, sexp[1]); break;
		case '+': total += rd_parse(ch, level, sexp[1]); break;
		case '*': total *= rd_parse(ch, level, sexp[1]); break;
		case '/': {int divisor = rd_parse(ch, level, sexp[1]); total = divisor ? total / divisor : 0;} break;
		case '%': {int divisor = rd_parse(ch, level, sexp[1]); total = divisor ? total % divisor : 0;} break;
		case 'd':
		case 'D': total = dice(total, rd_parse(ch, level, sexp[1]));   break;
		case '<': total = (total < rd_parse(ch, level, sexp[1])); 	   break;
		case '>': total = (total > rd_parse(ch, level, sexp[1]));	   break;
		case '=': total = (total == rd_parse(ch, level, sexp[1]));	   break;
		case '{': total = UMIN(total, rd_parse(ch, level, sexp[1]));   break;
		case '}': total = UMAX( total, rd_parse( ch, level, sexp[1])); break;
		case '^':
		{
			unsigned int y = rd_parse(ch, level, sexp[1]), z = total;

			for(x = 1; x < y; ++x, z *= total);
				total = z;
				break;
		}
	}
	return total;
}
int dice_parse					(CHAR_DATA *ch, int level, const char *texp ){   								//Wrapper function so as not to destroy exp.
	char buf[MAX_INPUT_LENGTH];
	mudstrlcpy(buf, texp, MAX_INPUT_LENGTH);
	return rd_parse(ch, level, buf);
}
bool saves_poison_death			(int level, CHAR_DATA *victim){													//Compute a saving throw.
	//Negative apply's make saving throw better.
	int save;
	save = 50 + (victim->level - level - victim->saving_poison_death) *5;
	save = URANGE(5, save, 95);
	return chance(victim, save);
}
bool saves_wands				(int level, CHAR_DATA *victim){													//Compute a saving throw.
	int save;
	if(IS_SET(victim->immune, RIS_MAGIC)) return TRUE;
	save = 50 + (victim->level - level - victim->saving_wand) *5;
	save = URANGE(5, save, 95);
	return chance(victim, save);
}
bool saves_para_petri			(int level, CHAR_DATA *victim){													//Compute a saving throw.
	int save;
	save = 50 + (victim->level - level - victim->saving_para_petri) *5;
	save = URANGE(5, save, 95);
	return chance(victim, save);
}
bool saves_breath				(int level, CHAR_DATA *victim){													//Compute a saving throw.
	int save;
	save = 50 + (victim->level - level - victim->saving_breath) *5;
	save = URANGE(5, save, 95);
	return chance(victim, save);
}
bool saves_spell_staff			(int level, CHAR_DATA *victim){													//Compute a saving throw.
   int save;
   if(IS_SET(victim->immune, RIS_MAGIC)) 	return TRUE;
   if(IS_NPC(victim) && level > 10)			level -= 5;
   save = 50 + (victim->level - level - victim->saving_spell_staff) *5;
   save = URANGE(5, save, 95);
   return chance(victim, save);
}
bool process_spell_components	(CHAR_DATA *ch, int sn ){														//Process the spell's required components, if any.
	// T###		check for item of type ###
	// V#####	check for item of vnum #####
	// Kword	check for item with keyword 'word'
	// G#####	check if player has ##### amount of gold
	// H####	check if player has #### amount of hitpoints

	//Special operators:
	// ! spell fails if player has this
	// + don't consume this component
	// @ decrease component's value[0], and extract if it reaches 0
	// # decrease component's value[1], and extract if it reaches 0
	// $ decrease component's value[2], and extract if it reaches 0
	// % decrease component's value[3], and extract if it reaches 0
	// ^ decrease component's value[4], and extract if it reaches 0
	// & decrease component's value[5], and extract if it reaches 0	
	
	SKILLTYPE *skill = get_skilltype(sn);
	const char *comp = skill->components;
	const char *check;
	char arg[MAX_INPUT_LENGTH];
	bool consume, fail, found;
	int val, value;
	OBJ_DATA *obj;

    //If no components necessary, then everything is cool 
	if(!comp || comp[0] == '\0') return TRUE;

	while(comp[0] != '\0')
	{
		comp = one_argument(comp, arg);
		consume = TRUE;
		fail = found = FALSE;
		val = -1;
		
		switch (arg[1])
		{
			default: 	check = arg + 1; 			  		break;
			case '!': 	check = arg + 2; fail = TRUE; 		break;
			case '+': 	check = arg + 2; consume = FALSE; 	break;
			case '@':	check = arg + 2; val = 0;			break;
			case '#':	check = arg + 2; val = 1;			break;
			case '$':	check = arg + 2; val = 2;			break;
			case '%':	check = arg + 2; val = 3;			break;
			case '^':	check = arg + 2; val = 4;			break;
			case '&':	check = arg + 2; val = 5;			break;
            //reserve '*', '(' and ')' for v6, v7 and v8   
		}
		
		value = atoi(check);
		obj = NULL;
		
		switch (UPPER(arg[0]))
		{
			case 'T':
				for(obj = ch->first_carrying; obj; obj = obj->next_content)
					if(obj->item_type == value)
					{
						if(fail) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something disrupts the casting of this spell..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return FALSE;}
						found = TRUE;
						break;
					} break;

			case 'V':
				for(obj = ch->first_carrying; obj; obj = obj->next_content)
					if(obj->pIndexData->vnum == value)
					{
						if(fail) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something disrupts the casting of this spell..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return FALSE;}
						found = TRUE;
						break;
					} break;

			case 'K':
				for(obj = ch->first_carrying; obj; obj = obj->next_content)
					if(nifty_is_name( check, obj->name))
					{
						if(fail) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something disrupts the casting of this spell..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return FALSE;}
						found = TRUE;
						break;
					} break;

			case 'G':
				if(ch->gold >= value)
				{
					if(fail) { {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something disrupts the casting of this spell..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return FALSE;}
					else 
					{
						if(consume)
						{
							set_char_color(AT_GOLD, ch);
							{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You feel a little lighter..."); act(AT_GOLD, "$t", ch, cMSG, NULL, TO_CHAR);}
							ch->gold -= value;
						}
						continue;
					}
				} break;

			case 'H':
				if(ch->hit >= value)
				{
					if(fail) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something disrupts the casting of this spell..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return FALSE;}
					else 
					{
						if(consume)
						{
							set_char_color(AT_BLOOD, ch);
							{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You feel a little weaker..."); act(AT_BLOOD, "$t", ch, cMSG, NULL, TO_CHAR);}
							ch->hit -= value;
							update_pos( ch );
						}
						continue;
					}
				} break;
		}

		//Having this component would make the spell fail... if we get here, then the caster didn't have that component.
		if(fail) continue;

		if(!found)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something is missing..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			return FALSE;
		}

		if(obj)
		{
			if(val >= 0 && val < 6)
			{
				separate_obj(obj);
				
				if(obj->value[val] <= 0)
				{
					act(AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
					act(AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
					extract_obj(obj);
					return FALSE;
				}
				else if(--obj->value[val] == 0)
				{
					act(AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
					act(AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
					extract_obj(obj);
				}
				else
					act(AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR);
			}
			else if(consume)
			{
				separate_obj(obj);
				act(AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
				act(AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
				extract_obj(obj);
			}
			else
			{
				int count = obj->count;
				obj->count = 1;
				act(AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR);
				obj->count = count;
			}
		}
	}
	return TRUE;
}
int pAbort; 																									//Locate targets.
bool silence_locate_targets; 																					//Turn off annoying message and just abort if needed.
void *locate_targets			(CHAR_DATA *ch, char *arg, int sn, CHAR_DATA **victim, OBJ_DATA **obj){
	SKILLTYPE *skill = get_skilltype(sn);
	void *vo = NULL;

	*victim = NULL;
	*obj 	= NULL;

	switch (skill->target)
	{
		default: bug( "%s: bad target for sn %d.", __func__, sn ); 
		return &pAbort;
		
		case TAR_IGNORE: break;

		case TAR_CHAR_OFFENSIVE:
		{
			if(arg[0] == '\0')
			{
				if((*victim = who_fighting(ch)) == NULL)
				{
					if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Cast the spell on whom?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return &pAbort;
				}
			}
			else
			{
				if( (*victim = get_char_room(ch, arg)) == NULL)
				{
					if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return &pAbort;
				}
			}

			//Offensive spells will choose the ch up to 92% of the time if the nuisance flag is set.
			if(!IS_NPC(ch) && ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 5 && number_percent() < (((ch->pcdata->nuisance->flags -5) * 8) + ch->pcdata->nuisance->power * 6))
			*victim = ch;

			if(is_safe(ch, *victim, TRUE))
			return &pAbort;

			if(ch == *victim)
			{
				if(SPELL_FLAG(get_skilltype(sn), SF_NOSELF))
				{
					if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't cast this on yourself!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return &pAbort;
				}
				
				if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Cast this on yourself?  Okay..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			}

			if(!IS_NPC(ch))
			{
				if(!IS_NPC(*victim))
				{
					if(get_timer( ch, TIMER_PKILLED) > 0)
					{
						if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have been killed in the last 5 minutes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						return &pAbort;
					}

					if(get_timer(*victim, TIMER_PKILLED) > 0)
					{
						if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This player has been killed in the last 5 minutes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						return &pAbort;
					}
					
					if(xIS_SET( ch->act, PLR_NICE) && ch != *victim)
					{
						if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are too nice to attack another player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						return &pAbort;
					}
					
					if(*victim != ch)
					{
						if(!silence_locate_targets) 		  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You really shouldn't do this to another player..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						else if(who_fighting(*victim) != ch)  return &pAbort;
					}
				}
			}

			if(IS_AFFECTED(ch, AFF_CHARM) && ch->master == *victim)
			{
				if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that on your own follower."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return &pAbort;
			}

			check_illegal_pk( ch, *victim );
			vo = ( void * )*victim;
		}
		break;

		case TAR_CHAR_DEFENSIVE:
			{
				if( arg[0] == '\0' ) *victim = ch;
				else
				{
					if( ( *victim = get_char_room( ch, arg ) ) == NULL )
					{
						if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						return &pAbort;
					}
				}
			}

			if(SPELL_FLAG( skill, SF_NOMOB ) && IS_NPC( *victim ) )
			{
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your magic fails to take hold."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return &pAbort;
			}


			//Nuisance flag will pick who you are fighting for defensive spells up to 36% of the time.
			if( !IS_NPC(ch) && ch->fighting && ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 5 && number_percent() < ((( ch->pcdata->nuisance->flags -5) *8) +6 * ch->pcdata->nuisance->power))
			*victim = who_fighting(ch);

			if(ch == *victim && SPELL_FLAG( get_skilltype(sn), SF_NOSELF))
			{
				if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't cast this on yourself!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return &pAbort;
			}

			vo = ( void * )*victim;
			break;

		case TAR_CHAR_SELF:
			if(arg[0] != '\0' && !nifty_is_name(arg, ch->name))
			{
				if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot cast this spell on another."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return &pAbort;
			}

			vo = ( void * )ch;
			break;

		case TAR_OBJ_INV:
			{
				if( arg[0] == '\0')
				{
					if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What should the spell be cast upon?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return &pAbort;
				}

				if(( *obj = get_obj_carry( ch, arg)) == NULL)
				{
					if(!silence_locate_targets) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not carrying that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return &pAbort;
				}
			}

			vo = (void *)*obj;
			break;
	}
	return vo;
}
const char *target_name;
const char *ranged_target_name = NULL;
void do_cast					(CHAR_DATA* ch, const char* argument){											//Cast a spell. Multi-caster and component support.
	char arg1[1000];
	char arg2[1000];
	static char staticbuf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo = NULL;
	int mana;
	int blood;
	int sn;
	ch_ret retcode;
	bool dont_wait = FALSE;
	SKILLTYPE *skill = NULL;
	struct timeval time_used;

	retcode = rNONE;

	switch (ch->substate)
	{
		default:
			//No ordering charmed mobs to cast spells 
			if(IS_NPC(ch) && (IS_AFFECTED( ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_POSSESS)))
			{
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't seem to do that right now..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return;
			}

			if(xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC) || IS_SET(ch->in_room->area->flags, AFLAG_NOMAGIC))
			{
				set_char_color(AT_MAGIC, ch);
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You failed."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
				return;
			}

			target_name = one_argument(argument, arg1);
			one_argument(target_name, arg2);
			
			if(ranged_target_name)  DISPOSE(ranged_target_name);
			
			ranged_target_name = str_dup(target_name);

			if(arg1[0] == '\0')
			{
				send_to_char("Cast which what where?\r\n", ch);
				return;
			}

			if(get_trust(ch) < LEVEL_GOD)
			{
				if((sn = find_spell( ch, arg1, TRUE)) < 0 
					|| (!IS_NPC(ch) && ch->level < skill_table[sn]->skill_level[ch->Class])) {send_to_char("You can't do that.\r\n", ch); 				return;}
				if((skill = get_skilltype(sn)) == NULL) 								 {send_to_char("You can't do that right now...\r\n", ch); 	return;}
			}
			else
			{
				if((sn = skill_lookup(arg1)) < 0) 										{send_to_char("We didn't create that yet...\r\n", ch); 					return;}				
				if(sn >= MAX_SKILL) 													{send_to_char("Hmm... that might hurt.\r\n", ch); 						return;}	
				if((skill = get_skilltype(sn)) == NULL)									{send_to_char("Something is severely wrong with that one...\r\n", ch); 	return;}
				if(skill->type != SKILL_SPELL)											{send_to_char("That isn't a spell.\r\n", ch); 							return;}
				if(!skill->spell_fun) 													{send_to_char("We didn't finish that one yet...\r\n", ch);				return;}
			}

			if(ch->position < skill->minimum_position && !IS_NPC(ch))
			{
				switch (ch->position)
				{
					default:			send_to_char("You can't concentrate enough.\r\n", ch); 					break;
					case POS_SITTING:	send_to_char("You can't summon enough energy sitting down.\r\n", ch); 	break;
					case POS_RESTING:	send_to_char("You're too relaxed to cast that spell.\r\n", ch);			break;
					case POS_FIGHTING:
						if(skill->minimum_position <= POS_EVASIVE) 	{send_to_char("This fighting style is too demanding for that!\r\n", ch);}
						else									   	{send_to_char("No way! You are still fighting!\r\n", ch);}
						break;
					case POS_DEFENSIVE:
						if(skill->minimum_position <= POS_EVASIVE) 	{send_to_char( "This fighting style is too demanding for that!\r\n", ch);}
						else 										{send_to_char("No way! You are still fighting!\r\n", ch);}
						break;
					case POS_AGGRESSIVE:
						if(skill->minimum_position <= POS_EVASIVE) 	{send_to_char("This fighting style is too demanding for that!\r\n", ch);}
						else 										{send_to_char("No way! You are still fighting!\r\n", ch);}
						break;
					case POS_BERSERK:
						if(skill->minimum_position <= POS_EVASIVE)	{send_to_char("This fighting style is too demanding for that!\r\n", ch);}
						else										{send_to_char("No way! You are still fighting!\r\n", ch);}
						break;
					case POS_EVASIVE:								 send_to_char("No way! You are still fighting!\r\n", ch); 		break;
					case POS_SLEEPING:							 	 send_to_char("You dream about great feats of magic.\r\n", ch);	break;
				}
				return;
			}

			if(skill->spell_fun == spell_null)										{send_to_char("That's not a spell!\r\n", ch); 									return;}
			if(!skill->spell_fun)													{send_to_char("You cannot cast that... yet.\r\n", ch); 							return;}
			
			if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && skill->guild != CLASS_NONE && (!ch->pcdata->clan || skill->guild != ch->pcdata->clan->Class))	
																					{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is only available to members of a certain guild."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}	return;}
																				
			if(!ch->in_room || (skill->spell_sector && !IS_SET(skill->spell_sector, (1 << ch->in_room->sector_type))))	
																					{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can not cast that here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}							return;}

			mana = IS_NPC(ch) ? 0 : UMAX( skill->min_mana, 100 / UMAX(1, 2 + ch->level - skill->skill_level[ch->Class]));
			blood = UMAX(1, (mana + 4) /8);
			vo = locate_targets(ch, arg2, sn, &victim, &obj);

			if(vo == &pAbort) return;

			if(!IS_NPC(ch) && victim && !IS_NPC(victim) && CAN_PKILL(victim) && !CAN_PKILL(ch) && !in_arena(ch) && !in_arena(victim))
			{
				set_char_color( AT_MAGIC, ch );
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The gods will not permit you to cast spells on that character."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
				return;
			}

			if(!IS_NPC(ch) && ch->mana < mana) 
				{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't have enough mana."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return;}

			if(skill->participants <= 1) break;

			//Multi-participant spells.
			add_timer(ch, TIMER_DO_FUN, UMIN( skill->beats / 10, 3 ), do_cast, 1);
			act(AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR);
			act(AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM);
			snprintf(staticbuf, MAX_INPUT_LENGTH, "%s %s", arg2, target_name);
			ch->alloc_ptr = str_dup(staticbuf);
			ch->tempnum = sn;
			return;
			
			case SUB_TIMER_DO_ABORT:
				DISPOSE(ch->alloc_ptr);
				
				if(IS_VALID_SN((sn = ch->tempnum)))
				{
					if((skill = get_skilltype(sn)) == NULL)
					{
						{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something went wrong..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
						bug("do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn);
						return;
					}
					
					mana = IS_NPC(ch) ? 0 : UMAX( skill->min_mana, 100 / UMAX(1, 2 + ch->level - skill->skill_level[ch->Class]));
					blood = UMAX( 1, (mana + 4) /8);
					
					if(IS_VAMPIRE(ch))	gain_condition( ch, COND_BLOODTHIRST, -UMAX(1, blood /3));
					else if(ch->level < LEVEL_IMMORTAL)  /* so imms dont lose mana */
					
					ch->mana -= mana / 3;
				}
				
				set_char_color(AT_MAGIC, ch);
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You stop chanting..."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
				return;
			case 1:
				sn = ch->tempnum;
				
				if((skill = get_skilltype(sn)) == NULL)
				{
					{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something went wrong..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					bug("do_cast: substate 1: bad sn %d", sn);
					return;
				}
				
				if(!ch->alloc_ptr || !IS_VALID_SN(sn) || skill->type != SKILL_SPELL)
				{
					{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Something cancels out the spell!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					bug("do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn);
					return;
				}
				
				mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana, 100 / UMAX(1, 2 + ch->level - skill->skill_level[ch->Class]));
				blood = UMAX(1, ( mana + 4) /8);
				mudstrlcpy(staticbuf, ch->alloc_ptr, MAX_INPUT_LENGTH);
				target_name = one_argument(staticbuf, arg2);
				DISPOSE(ch->alloc_ptr);
				ch->substate = SUB_NONE;
				
				if(skill->participants > 1)
				{
					int cnt = 1;
					CHAR_DATA *tmp;
					TIMER *t;

					for(tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room)
						
						if(tmp != ch && (t = get_timerptr(tmp, TIMER_DO_FUN)) != NULL && t->count >= 1 && t->do_fun == do_cast
							&& tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp(tmp->alloc_ptr, staticbuf))
								++cnt;
								
						if(cnt >= skill->participants)
						{
							for(tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room)
								if(tmp != ch && (t = get_timerptr(tmp, TIMER_DO_FUN)) != NULL && t->count >= 1 && t->do_fun == do_cast
									&& tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp(tmp->alloc_ptr, staticbuf))
								{
									extract_timer( tmp, t );
									act(AT_MAGIC, "Channeling your energy into $n, you help cast the spell!", ch, NULL, tmp, TO_VICT);
									act(AT_MAGIC, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR);
									act(AT_MAGIC, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT);
									learn_from_success(tmp, sn);
									
									if(IS_VAMPIRE(tmp)) 	gain_condition(tmp, COND_BLOODTHIRST, -blood);
									else				tmp->mana -= mana;
									
									tmp->substate = SUB_NONE;
									tmp->tempnum = -1;
									DISPOSE(tmp->alloc_ptr);
								}
								
							dont_wait = TRUE;
							{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You concentrate all the energy into a burst of mystical words!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
							vo = locate_targets(ch, arg2, sn, &victim, &obj);
							
							if(vo == &pAbort) return;
						}
						else
						{
							set_char_color(AT_MAGIC, ch);
							{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There was not enough power for the spell to succeed..."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}

							if(ch->level < LEVEL_IMMORTAL)  ch->mana -= mana /2;
                
							return;
						}
				}
	}

	if(!dont_wait)								WAIT_STATE(ch, skill->beats);
	if(!process_spell_components(ch, sn))
    {
		if(ch->level < LEVEL_IMMORTAL)  /* so imms dont lose mana */
		ch->mana -= mana /2;
		return;
    }
	
	/*****************************************************************************************************************/
	if(!IS_NPC(ch) && number_percent() > MAGIC_CONCENTRATION(ch, sn))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&RYour spellcasting fails as the magicka disperses.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

		if(IS_VAMPIRE(ch)) gain_condition(ch, COND_BLOODTHIRST, -UMAX(1, blood / 2));
		else if(ch->level < LEVEL_IMMORTAL)  /* so imms dont lose mana */
         
		ch->mana -= mana /2;
		learn_from_failure(ch, sn);
		return;
	}
	else
	{
		if(IS_VAMPIRE(ch)) gain_condition(ch, COND_BLOODTHIRST, -blood);

		//Check for immunity to magic if victim is known... and it is a TAR_CHAR_DEFENSIVE/SELF spell otherwise spells will have to check themselves.
		if((( skill->target == TAR_CHAR_DEFENSIVE || skill->target == TAR_CHAR_SELF) && victim && IS_SET(victim->immune, RIS_MAGIC)))
		{
			immune_casting(skill, ch, victim, NULL);
			retcode = rSPELL_FAILED;
		}
		else
		{
			start_timer(&time_used);
			retcode = (*skill->spell_fun) (sn, ch->level, ch, vo);
			end_timer(&time_used);
			update_userec(&time_used, &skill->userec);
		}
	}

	if(ch->in_room && IS_SET( ch->in_room->area->flags, AFLAG_SPELLLIMIT)) 					ch->in_room->area->curr_spell_count++;
	if(retcode == rCHAR_DIED || retcode == rERROR || char_died(ch))							return;
	if(retcode != rSPELL_FAILED)
	{
		//Only successful spells are spoken; ventriloquism stays secret.
		if(str_cmp(skill->name, "ventriloquate")) say_spell(ch, sn);
		if(retcode == rNONE && victim && !char_died(victim) && ch->fighting)
			MAGIC_CAST_FLOURISH(ch, victim, skill);
		learn_from_success(ch, sn);
	}
	if(victim && victim != ch && !IS_NPC(victim) && skill->target == TAR_CHAR_DEFENSIVE)	adjust_favor(ch, 7, 1);
	if(victim && victim != ch && !IS_NPC(ch) && skill->target == TAR_CHAR_DEFENSIVE)		adjust_favor(victim, 13, 1);
	if(victim && victim != ch && !IS_NPC(ch) && skill->target == TAR_CHAR_OFFENSIVE)		adjust_favor(ch, 4, 1);

	if(skill->target == TAR_CHAR_OFFENSIVE && victim && !char_died(victim) && victim != ch)
	{
		CHAR_DATA *vch, *vch_next;

		for(vch = ch->in_room->first_person; vch; vch = vch_next)
		{
			vch_next = vch->next_in_room;

				if(vch == victim)
				{
					if(vch->master != ch && !vch->fighting)  
						retcode = multi_hit( vch, ch, TYPE_UNDEFINED);
						break;
				}
		}
	}
	return;
}
ch_ret obj_cast_spell			(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj){			//Cast spells at targets using a magical object.
	void *vo;
	ch_ret retcode = rNONE;
	int levdiff = ch->level - level;
	SKILLTYPE *skill = get_skilltype(sn);
	struct timeval time_used;

	if(sn == -1) return retcode;
	if(!skill || !skill->spell_fun) {bug("%s: bad sn %d.", __func__, sn); return rERROR;}

	if(!ch->in_room || xIS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) || IS_SET(ch->in_room->area->flags, AFLAG_NOMAGIC))
	{
		set_char_color(AT_MAGIC, ch);
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Nothing seems to happen..."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
		return rNONE;
	}

	if(xIS_SET(ch->in_room->room_flags, ROOM_SAFE) && skill->target == TAR_CHAR_OFFENSIVE)
	{
		set_char_color(AT_MAGIC, ch);
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Nothing seems to happen..."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
		return rNONE;
	}

	//Basically this was added to cut down on level 5 players using level 40 scrolls in battle too often;)
	if(ch != victim && (skill->target == TAR_CHAR_OFFENSIVE || number_bits(7) == 1) && skill->type != SKILL_HERB && !chance(ch, 95 + levdiff))
	{
	   //1/128 chance if non-offensive.   
		switch (number_bits(2))
		{
			case 0:
				failed_casting(skill, ch, victim, NULL);
				break;
			case 1:
				act(AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR);
				if(victim) act(AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT);
				act(AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT);
				return damage(ch, ch, number_range(1, level), TYPE_UNDEFINED);
			case 2:
				failed_casting( skill, ch, victim, NULL );
				break;
			case 3:
				act(AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR);
				if(victim) act(AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT);
				act(AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT);
				return damage(ch, ch, number_range(1, level), TYPE_UNDEFINED);
			}
		return rNONE;
	}

	target_name = "";
	switch (skill->target)
	{
		default:
			bug("%s: bad target for sn %d.", __func__, sn);
			return rERROR;

		case TAR_IGNORE:
			vo = NULL;
			if(victim) 		target_name = victim->name;
			else if(obj) 	target_name = obj->name;
			break;

		case TAR_CHAR_OFFENSIVE:
			if(victim != ch)
			{
				if(!victim) victim = who_fighting(ch);
				
				if(!victim || (!IS_NPC( victim) && !in_arena(victim)))
				{
					{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
					return rNONE;
				}
			}
			
			if(ch != victim && is_safe(ch, victim, TRUE)) return rNONE;
			vo = (void *)victim;
			break;

		case TAR_CHAR_DEFENSIVE:
			if(victim == NULL) victim = ch;
			vo = (void *)victim;

			if(SPELL_FLAG(skill, SF_NOMOB) && IS_NPC(victim))
			{
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your magic fails to take hold."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return rNONE;
			}

			if(skill->type != SKILL_HERB && IS_SET(victim->immune, RIS_MAGIC))
			{
				immune_casting(skill, ch, victim, NULL);
				return rNONE;
			}
			break;

		case TAR_CHAR_SELF:
			vo = (void *)ch;
			if(skill->type != SKILL_HERB && IS_SET(ch->immune, RIS_MAGIC))
			{
				immune_casting(skill, ch, victim, NULL);
				return rNONE;
			}
			break;

		case TAR_OBJ_INV:
			if(obj == NULL)
			{
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return rNONE;
			}
			vo = ( void * )obj;
			break;
	}

	start_timer(&time_used);
	retcode = (*skill->spell_fun) (sn, level, ch, vo);
	end_timer(&time_used);
	update_userec(&time_used, &skill->userec);

	if(retcode == rSPELL_FAILED)					retcode = rNONE;
	if(retcode == rCHAR_DIED || retcode == rERROR)	return retcode;
	if(char_died(ch))								return rCHAR_DIED;
	
	if(skill->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died(victim))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for(vch = ch->in_room->first_person; vch; vch = vch_next)
		{
			vch_next = vch->next_in_room;
			
			if(victim == vch && !vch->fighting && vch->master != ch)
			{
				retcode = multi_hit( vch, ch, TYPE_UNDEFINED);
				break;
			}
		}
	}
	return retcode;
}
bool can_charm					(CHAR_DATA * ch){ 																//Able to charm?
   if(IS_NPC(ch) || IS_IMMORTAL(ch)) 					  return TRUE;
   if(((get_curr_cha(ch) /3) + 1) > ch->pcdata->charmies) return TRUE;
   return FALSE;
}
//======================================================================================================================//
// SMAUG CORE SPELLS																					 				//
//======================================================================================================================//
ch_ret spell_acid_blast			(int sn, int level, CHAR_DATA *ch, void *vo){						//ACID BLAST
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam = dice(level, 6);
	if(saves_spell_staff(level, victim)) dam /= 2;
	
	return damage(ch, victim, dam, sn);
}
ch_ret spell_blindness			(int sn, int level, CHAR_DATA *ch, void *vo){						//BLINDNESS
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int tmp;
	SKILLTYPE *skill = get_skilltype(sn);

	if(SPELL_FLAG( skill, SF_PKSENSITIVE) && !IS_NPC(ch) && !IS_NPC(victim))	tmp = level /2;
	else																		tmp = level;

	if(IS_SET(victim->immune, RIS_MAGIC))
	{
		immune_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}
	
	if(IS_AFFECTED( victim, AFF_BLIND) || saves_spell_staff(tmp, victim))
	{
		failed_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}

	af.type = sn;
	af.location = APPLY_HITROLL;
	af.modifier = -4;
	af.duration = (int)((1 + (level /3)) * DUR_CONV);
	af.bitvector = meb(AFF_BLIND);
	affect_to_char(victim, &af);
	set_char_color(AT_MAGIC, victim);
	send_to_char("You are blinded!\r\n", victim);
	
	if(ch != victim)
	{
		act(AT_MAGIC, "You weave a spell of blindness around $N.", ch, NULL, victim, TO_CHAR);
		act(AT_MAGIC, "$n weaves a spell of blindness about $N.",  ch, NULL, victim, TO_NOTVICT);
	}
	return rNONE;
}
ch_ret spell_burning_hands		(int sn, int level, CHAR_DATA *ch, void *vo){						//BURNING HANDS
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	static const short dam_each[] = 
	{
		0,
		0, 0, 0, 0, 14, 17, 20, 23, 26, 29,
		29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
		34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
		39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
		44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
		49, 49, 50, 50, 51, 51, 52, 52, 53, 53,
		54, 54, 55, 55, 56, 56, 57, 57, 58, 58
	};
	
	int dam;
	level = UMIN((unsigned int)level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level = UMAX(0, level);
	dam = number_range(dam_each[level] /2, dam_each[level] *2);
	
	if(saves_spell_staff(level, victim)) dam /= 2;
	return damage(ch, victim, dam, sn);
}
ch_ret spell_call_lightning		(int sn, int level, CHAR_DATA *ch, void *vo){						//CALL LIGHTNING
	ROOM_INDEX_DATA *where;
	CHAR_DATA *vch;
	TRV_WORLD *lc;
	int dam;
	bool ch_died;
	ch_ret retcode;
	struct WeatherCell *cell = getWeatherCell(ch->in_room->area);

	if(!IS_OUTSIDE(ch))								{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must be out of doors."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return rSPELL_FAILED;}
	if(getPrecip(cell) <40 && getEnergy(cell) <30)	{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You need bad weather."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}		return rSPELL_FAILED;}

	set_char_color(AT_MAGIC, ch);
	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "God's lightning strikes your foes!"); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
	act(AT_MAGIC, "$n calls God's lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);

	where = ch->in_room;
	ch_died = FALSE;
	retcode = rNONE;
	dam = dice(level /2, 8);
	lc = trworld_create(TR_CHAR_WORLD_FORW);
	
	for(vch = first_char; vch; vch = trvch_wnext(lc))
	{
		if(!vch->in_room)
         continue;

		if(vch->in_room == where)
		{
			if(IS_NPC(vch) && xIS_SET(vch->act, ACT_MOBINVIS)) 													continue;
			if(!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL) 		continue;
			if(vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)))											retcode = damage(ch, vch, saves_spell_staff( level, vch ) ? dam / 2 : dam, sn);
			if(retcode == rCHAR_DIED || char_died(ch)) 															ch_died = TRUE;
		}
		else if(vch->in_room->area == where->area && IS_OUTSIDE(vch) && IS_AWAKE(vch) && number_bits(2) == 0) 	send_to_char("&BLightning flashes in the sky.\r\n", vch);
	}

	trworld_dispose(&lc);
	return ch_died ? rCHAR_DIED : rNONE;
}
ch_ret spell_cause_light		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//CAUSE LIGHT WOUNDS
	return damage(ch, (CHAR_DATA *) vo, dice(1, 8) + level /3, sn);
}
ch_ret spell_cause_critical		(int sn, int level, CHAR_DATA *ch, void *vo){						//CAUSE CRITICAL WOUNDS
	return damage(ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn);
}
ch_ret spell_cause_serious		(int sn, int level, CHAR_DATA *ch, void *vo){						//CAUSE SERIOUS WOUNDS
	return damage(ch, (CHAR_DATA *) vo, dice(2, 8) + level /2, sn);
}
ch_ret spell_change_sex			(int sn, int level, CHAR_DATA *ch, void *vo){ 						//CHANGE SEX
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET(victim->immune, RIS_MAGIC))	{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	if( is_affected(victim, sn))			{failed_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	
	af.type = sn;
	af.duration = (int)(10 * level * DUR_CONV);
	af.location = APPLY_SEX;
	
	do {af.modifier = number_range( 0, 2 ) - victim->sex;}
	while(af.modifier == 0);
	
	xCLEAR_BITS(af.bitvector);
	affect_to_char(victim, &af);

	successful_casting(skill, ch, victim, NULL);
	return rNONE;
}
ch_ret spell_charm_person		(int sn, int level, CHAR_DATA *ch, void *vo){						//CHARM PERSON
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int schance;
	SKILLTYPE *skill = get_skilltype(sn);

	if(victim == ch)															{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You like yourself even better!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 					return rSPELL_FAILED;}
	if(get_timer(ch, TIMER_PKILLED) > 0) 										{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have been killed in the past five minutes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 	return rSPELL_FAILED;}
	if(IS_SET( victim->immune, RIS_MAGIC) || IS_SET(victim->immune, RIS_CHARM))	{immune_casting( skill, ch, victim, NULL); 									return rSPELL_FAILED;}

	if(!IS_NPC(victim) && !IS_NPC(ch))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "I don't think so..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		send_to_char("You feel charmed...\r\n", victim);
		return rSPELL_FAILED;
	}

	schance = ris_save(victim, level, RIS_CHARM);

	if(IS_AFFECTED( victim, AFF_CHARM) || schance == 1000 || IS_AFFECTED(ch, AFF_CHARM) || level < victim->level || circle_follow(victim, ch) || !can_charm(ch) || saves_spell_staff(schance, victim))
		{failed_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}

	//Cannot charm mobs while having just been killed/killer.
	if(get_timer(ch, TIMER_PKILLED) > 0) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are still too weak.."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return rSPELL_FAILED;}

	if(victim->master) stop_follower(victim);
	
	add_follower(victim, ch);
	af.type = sn;
	af.duration = (int)((number_fuzzy((level +1) /5) +1 ) * DUR_CONV);
	af.location = 0;
	af.modifier = 0;
	af.bitvector = meb(AFF_CHARM);
	affect_to_char(victim, &af);
	successful_casting(skill, ch, victim, NULL);

	log_printf_plus(LOG_NORMAL, ch->level, "%s has charmed %s.", ch->name, victim->name);
	
	if(!IS_NPC(ch))
      ch->pcdata->charmies++;
  
	if(IS_NPC(victim))
	{
		start_hating(victim, ch);
		start_hunting(victim, ch);
	}
	return rNONE;
}
ch_ret spell_chill_touch		(int sn, int level, CHAR_DATA *ch, void *vo){						//CHILL TOUCH
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const short dam_each[] = 
	{
		0,
		0, 0, 6, 7, 8, 9, 12, 13, 13, 13,
		14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
		17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
		20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
		24, 24, 24, 25, 25, 25, 26, 26, 26, 27,
		27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
		32, 33, 34, 34, 35, 35, 36, 37, 37, 38
	};
	AFFECT_DATA af;
	int dam;

	level = UMIN((unsigned int)level, sizeof(dam_each) / sizeof(dam_each[0]) -1);
	level = UMAX(0, level);
	dam = number_range(dam_each[level] /2, dam_each[level] *2);
	
	if(!saves_spell_staff(level, victim))
	{
		af.type = sn;
		af.duration = 14;
		af.location = APPLY_STR;
		af.modifier = -1;
		xCLEAR_BITS(af.bitvector);
		affect_join(victim, &af);
	}
	else
	{
		dam /= 2;
	}
	
	return damage(ch, victim, dam, sn);
}
ch_ret spell_colour_spray		(int sn, int level, CHAR_DATA *ch, void *vo){						//COLOR SPRAY
	CHAR_DATA *victim = (CHAR_DATA *) vo;
   
	static const short dam_each[] = 
	{
		0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
		58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
		65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
		73, 73, 74, 75, 76, 76, 77, 78, 79, 79,
		80, 80, 81, 82, 82, 83, 83, 84, 85, 85,
		86, 86, 87, 88, 88, 89, 89, 90, 91, 91
	};
	
	int dam;
	level = UMIN((unsigned int)level, sizeof(dam_each) / sizeof( dam_each[0]) - 1);
	level = UMAX(0, level);
	dam = number_range(dam_each[level] /2, dam_each[level] *2);
	
	if(saves_spell_staff(level, victim))
    dam /= 2;

	return damage(ch, victim, dam, sn);
}
ch_ret spell_control_weather	(int sn, int level, CHAR_DATA *ch, void *vo){ 						//CONTROL WEATHER
	SKILLTYPE *skill = get_skilltype(sn);
	int change;
	struct WeatherCell *cell = getWeatherCell(ch->in_room->area);

	change = URANGE(5, number_range(5, 15) + (ch->level /10), 15);

	     if(!str_cmp(target_name, "warmer")) 	IncreaseTemp(cell, change);
	else if(!str_cmp(target_name, "colder"))	DecreaseTemp(cell, change);
	else if(!str_cmp(target_name, "wetter"))	IncreasePrecip(cell, change);
	else if(!str_cmp(target_name, "drier"))		DecreasePrecip(cell, change);
	else if(!str_cmp(target_name, "stormier"))	IncreaseEnergy(cell, change);
	else if(!str_cmp(target_name, "calmer"))	DecreaseEnergy(cell, change);
	else
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Do you want it to get warmer, colder, wetter, drier, stormier, or calmer?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return rSPELL_FAILED;
	}

	successful_casting(skill, ch, NULL, NULL);
	return rNONE;
}
ch_ret spell_cure_blindness		(int sn, int level, CHAR_DATA *ch, void *vo){						//CURE BLINDNESS
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILLTYPE *skill = get_skilltype(sn);

	set_char_color(AT_MAGIC, ch);
	
	if(IS_SET(victim->immune, RIS_MAGIC)) {immune_casting( skill, ch, victim, NULL ); return rSPELL_FAILED;}
	if(!is_affected(victim, gsn_blindness))
	{
      if(ch != victim) 	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You work your cure, but it has no apparent effect."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't seem to be blind."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	  
      return rSPELL_FAILED;
	}
	
	affect_strip(victim, gsn_blindness);
	set_char_color(AT_MAGIC, victim);
	send_to_char("Your vision returns!\r\n", victim);
	
	if(ch != victim) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You work your cure, restoring vision."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	return rNONE;
}
ch_ret spell_sacral_divinity	(int sn, int level, CHAR_DATA *ch, void *vo){ 						//SACRAL DIVINITY
	AFFECT_DATA af;
	SKILLTYPE *skill = get_skilltype(sn);

	if(ch->alignment < 350) 				{act(AT_MAGIC, "Your prayer goes unanswered.", ch, NULL, NULL, TO_CHAR); return rSPELL_FAILED;}
	if(IS_SET(ch->immune, RIS_MAGIC))		{immune_casting(skill, ch, NULL, NULL); return rSPELL_FAILED;}
	if( IS_AFFECTED(ch, AFF_SANCTUARY)) 	return rSPELL_FAILED;
	
	af.type = sn;
	af.duration = level * 3;
	af.location = APPLY_AFFECT;
	af.modifier = 0;
	af.bitvector = meb(AFF_SANCTUARY);
	affect_to_char(ch, &af);
	
	act(AT_MAGIC, "A shroud of glittering light slowly wraps itself about $n.",   ch, NULL, NULL, TO_ROOM);
	act(AT_MAGIC, "A shroud of glittering light slowly wraps itself around you.", ch, NULL, NULL, TO_CHAR);
	return rNONE;
}
ch_ret spell_expurgation		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//EXPURATION
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET( victim->immune, RIS_MAGIC)) 		{immune_casting( skill, ch, victim, NULL); return rSPELL_FAILED;}
	if(!is_affected(victim, gsn_poison))		return rSPELL_FAILED;
	
	affect_strip(victim, gsn_poison);
	
	act(AT_MAGIC, "You speak an ancient prayer, begging your god for purification.", ch, NULL, NULL, TO_CHAR);
	act(AT_MAGIC, "$n speaks an ancient prayer begging $s god for purification.", 	 ch, NULL, NULL, TO_ROOM);
	return rNONE;
}
ch_ret spell_bethsaidean_touch	(int sn, int level, CHAR_DATA *ch, void *vo){ 						//BETHSAIDEAN TOUCH
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET(victim->immune, RIS_MAGIC)) 		{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	if(!is_affected(victim, gsn_blindness))		return rSPELL_FAILED;
	
	affect_strip(victim, gsn_blindness);
	
	set_char_color(AT_MAGIC, victim);
	send_to_char("Your sight is restored!\r\n", victim);
	
	if(ch != victim)
	{
		act(AT_MAGIC, "$n lays $s hands over your eyes and concentrates...", 			ch, NULL, victim, TO_VICT);
		act(AT_MAGIC, "$n lays $s hands over $N's eyes and concentrates...", 			ch, NULL, victim, TO_NOTVICT);
		act(AT_MAGIC, "Laying your hands on $N's eyes, you pray to lift $S blindness.", ch, NULL, victim, TO_CHAR);
	}
	return rNONE;
}
ch_ret spell_cure_poison		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//CURE POISON
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILLTYPE *skill = get_skilltype(sn);
	int x = 0;

	if(IS_SET(victim->immune, RIS_MAGIC))
	{
		immune_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}

	if(is_affected(victim, gsn_poison))
	{
		affect_strip(victim, gsn_poison);
		set_char_color(AT_MAGIC, victim);
		send_to_char("A warm feeling runs through your body.\r\n", victim);
		x = victim->mental_state < 0 ? -x : x;
		victim->mental_state = URANGE(-25, victim->mental_state, 25);
		
		if(ch != victim)
		{
			act(AT_MAGIC, "A flush of health washes over $N.",   ch, NULL, victim, TO_NOTVICT);
			act(AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR );
		}
		return rNONE;
	}
	else
	{
		set_char_color(AT_MAGIC, ch);
		
		if(ch != victim) 	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You work your cure, but it has no apparent effect."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		else				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't seem to be poisoned."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return rSPELL_FAILED;
	}
}
ch_ret spell_curse				(int sn, int level, CHAR_DATA *ch, void *vo){ 						//CURSE
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET(victim->immune, RIS_MAGIC)) 									{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	if(IS_AFFECTED(victim, AFF_CURSE) || saves_spell_staff(level, victim))	{failed_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	
	af.type = sn;
	af.duration = (int)((4 * level) * DUR_CONV);
	af.location = APPLY_HITROLL;
	af.modifier = -1;
	af.bitvector = meb( AFF_CURSE );
	affect_to_char(victim, &af);

	af.location = APPLY_SAVING_SPELL;
	af.modifier = 1;
	affect_to_char(victim, &af);

	set_char_color(AT_MAGIC, victim);
	send_to_char("You feel unclean.\r\n", victim);
	
	if(ch != victim)
	{
		act(AT_MAGIC, "You utter a curse upon $N.", ch, NULL, victim, TO_CHAR);
		act(AT_MAGIC, "$n utters a curse upon $N.", ch, NULL, victim, TO_NOTVICT);
	}
	return rNONE;
}
ch_ret spell_detect_poison		(int sn, int level, CHAR_DATA *ch, void *vo){						//DETECT POISON
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	set_char_color(AT_MAGIC, ch);
	
	if(obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD || obj->item_type == ITEM_COOK)
	{
		if(obj->item_type == ITEM_COOK && obj->value[2] == 0) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It looks undercooked."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		else if(obj->value[3] != 0)							  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You smell poisonous fumes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		else												  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It looks very delicious."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	}
	else
		{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It doesn't look poisoned."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}}

   return rNONE;
}
ch_ret spell_dispel_evil		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//DISPEL EVIL
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	SKILLTYPE *skill = get_skilltype(sn);

	if(!IS_NPC( ch ) && IS_EVIL(ch)) 		victim = ch;
	if(IS_GOOD(victim)) 					{act(AT_MAGIC, "Thoric protects $N.", ch, NULL, victim, TO_ROOM); return rSPELL_FAILED;}
	if(IS_NEUTRAL(victim))					{act(AT_MAGIC, "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);  return rSPELL_FAILED;}
	if(IS_SET(victim->immune, RIS_MAGIC))	{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}

	dam = dice(level, 4);
	if(saves_spell_staff(level, victim)) dam /= 2;
	return damage(ch, victim, dam, sn);
}
ch_ret spell_dispel_magic		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//DISPEL MAGIC
	//New version of dispel magic fixes alot of bugs, and allows players to not lose thie affects if they have the spell and the affect.
	//Also prints a message to the victim, and does various other things :)
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int cnt = 0, affect_num, affected_by = 0, times = 0;
	int schance;
	SKILLTYPE *skill = get_skilltype( sn );
	AFFECT_DATA *paf;
	bool found = FALSE, twice = FALSE, three = FALSE;
	bool is_mage = FALSE;

	set_char_color(AT_MAGIC, ch);

	schance = (get_curr_int(ch) - get_curr_int(victim));

	if(IS_SET(victim->immune, RIS_MAGIC)) {immune_casting(skill, ch, victim, NULL);  return rSPELL_FAILED;}
	if(IS_NPC(ch) || ch->Class == CLASS_MAGE)										 is_mage = TRUE;
	
	if(is_mage)	 schance += 5;
	else		 schance -= 15;

	//Bug Fix to prevent possesed mobs from being dispelled.
	if(IS_NPC(victim) && IS_AFFECTED(victim, AFF_POSSESS))  
	{
		immune_casting(skill, ch, victim, NULL); 
		return rVICT_IMMUNE;
	}

	if(ch == victim)
	{
		if(ch->first_affect)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You dispel magic effects from you."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			
			while(ch->first_affect)
				affect_remove(ch, ch->first_affect);
			
			if(!IS_NPC(ch))  update_aris(victim);
			return rNONE;
		}
		else
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not affected with anything."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			return rNONE;
		}
	}
	
	if(!is_mage && !IS_AFFECTED(ch, AFF_DETECT_MAGIC))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't sense a magical aura to dispel."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return rERROR; /* You don't cast it so don't attack */
	}

	if( number_percent(  ) > ( 75 - schance ) )
	{
		twice = TRUE;
		
		if(number_percent() > (75 - schance))
        three = TRUE;
	}

	start_loop:

//Grab affected_by from mobs first 
	if(IS_NPC(victim) && !xIS_EMPTY(victim->affected_by))
	{
		for(;;)
		{
			affected_by = number_range(0, MAX_AFFECTED_BY - 1);
			
			if(xIS_SET(victim->affected_by, affected_by))
			{
				found = TRUE;
				break;
			}
			
			if(cnt++ > 30)
			{
				found = FALSE;
				break;
			}
		}
		
		if(found ) // Ok lets see if it is a spell
		{
			for(paf = victim->first_affect; paf; paf = paf->next)
				
				if(xIS_SET(paf->bitvector, affected_by))
				break;
			
			if(paf) //It is a spell lets remove the spell too
			{

				if(level < victim->level || saves_spell_staff(level, victim))
				{
					if(!dispel_casting( paf, ch, victim, FALSE, FALSE))
						failed_casting(skill, ch, victim, NULL);
					
					return rSPELL_FAILED;
				}
				
				if(SPELL_FLAG( get_skilltype(paf->type), SF_NODISPEL))
				{
					if(!dispel_casting(paf, ch, victim, FALSE, FALSE))
						failed_casting(skill, ch, victim, NULL);
					
					return rSPELL_FAILED;
				}
				
				if(!dispel_casting(paf, ch, victim, FALSE, TRUE ) && times == 0)
					successful_casting(skill, ch, victim, NULL);
				
				affect_remove( victim, paf );
				
				if((twice && times < 1) || (three && times < 2))
				{
					times++;
					goto start_loop;
				}
				
				return rNONE;
			}
			else  // Nope not a spell just remove the bit *For Mobs Only*
			{ // <--- MISSING OPENING BRACE ADDED HERE
			
				if( level < victim->level || saves_spell_staff(level, victim))
				{
					if( !dispel_casting(NULL, ch, victim, affected_by, FALSE))
						failed_casting(skill, ch, victim, NULL );
					
					return rSPELL_FAILED;
				}
				
				if(!dispel_casting(NULL, ch, victim, affected_by, TRUE) && times == 0)
					successful_casting(skill, ch, victim, NULL);
				
				xREMOVE_BIT(victim->affected_by, affected_by);
				
				if((twice && times < 1) || (three && times < 2))
				{
					times++;
					goto start_loop;
				}
				return rNONE;
			} // <--- MISSING CLOSING BRACE ADDED HERE
		}
	}

	//Ok mob has no affected_by's or we didn't catch them lets go to first_affect.
	if(!victim->first_affect)
	{
		failed_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}

	cnt = 0;

	//Need to randomize the affects, yes you have to loop on average 1.5 times but dispel magic only takes at worst case 256 uSecs so who cares :)
	for(paf = victim->first_affect; paf; paf = paf->next)
    cnt++;

	paf = victim->first_affect;

	for(affect_num = number_range(0, (cnt - 1)); affect_num > 0; affect_num--)
     paf = paf->next;

	if( level < victim->level || saves_spell_staff( level, victim ) )
	{
		if(!dispel_casting(paf, ch, victim, FALSE, FALSE))
			failed_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}

	//Need to make sure we have an affect and it isn't no dispel 
	if(!paf || SPELL_FLAG(get_skilltype(paf->type), SF_NODISPEL))
	{
		if(!dispel_casting(paf, ch, victim, FALSE, FALSE))
			failed_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}
	
	if(!dispel_casting(paf, ch, victim, FALSE, TRUE ) && times == 0)
		successful_casting(skill, ch, victim, NULL);
	
	affect_remove( victim, paf );
	if((twice && times < 1) || (three && times < 2))
	{
		times++;
		goto start_loop;
	}

	//Have to reset victim affects 
	if(!IS_NPC(victim)) update_aris(victim);
	return rNONE;
}
ch_ret spell_polymorph			(int sn, int level, CHAR_DATA *ch, void *vo){ 						//POLYMORPH
	MORPH_DATA *morph;
	SKILLTYPE *skill = get_skilltype(sn);

	morph = find_morph(ch, target_name, TRUE);
	if(!morph) 						{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't morph into anything like that!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} 	return rSPELL_FAILED;}
	if(!do_morph_char(ch, morph))	{failed_casting(skill, ch, NULL, NULL);								return rSPELL_FAILED;}
	return rNONE;
}
ch_ret spell_earthquake			(int sn, int level, CHAR_DATA *ch, void *vo){  						//EARTHQUAKE
	ROOM_INDEX_DATA *where;
	CHAR_DATA *vch;
	TRV_WORLD *lc;
	bool ch_died;
	ch_ret retcode;

	if( xIS_SET(ch->in_room->room_flags, ROOM_SAFE))  {failed_casting(get_skilltype(sn), ch, NULL, NULL); return rSPELL_FAILED;}

	act(AT_MAGIC, "The earth trembles beneath your feet!",  ch, NULL, NULL, TO_CHAR);
	act(AT_MAGIC, "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

	ch_died = FALSE;
	retcode = rNONE;
	where = ch->in_room;
	lc = trworld_create(TR_CHAR_WORLD_FORW);
	
	for(vch = first_char; vch; vch = trvch_wnext(lc))
	{
		if(!vch->in_room) continue;

		if(vch->in_room == where)
		{
			if(IS_NPC(vch) && xIS_SET(vch->act, ACT_MOBINVIS)) 												continue;
			if(!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL)	continue;

			if(vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)) && !IS_AFFECTED(vch, AFF_FLYING) && !IS_AFFECTED(vch, AFF_FLOATING))
			retcode = damage(ch, vch, level + dice(2, 8), sn);
		
			if(retcode == rCHAR_DIED || char_died(ch))  {ch_died = TRUE;}
		}
		else if(vch->in_room->area == where->area && number_bits(2) == 0)
			send_to_char( "&BThe earth trembles and shivers.\r\n", vch );
	}
	trworld_dispose(&lc);
	return ch_died ? rCHAR_DIED : rNONE;
}
ch_ret spell_enchant_weapon     (int sn, int level, CHAR_DATA *ch, void *vo){  						//ENCHANT WEAPON
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if(obj->item_type != ITEM_WEAPON || IS_OBJ_STAT(obj, ITEM_MAGIC) || IS_OBJ_STAT(obj, ITEM_ENCHANTED) || obj->first_affect)
	{
		act(AT_MAGIC, "Your magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_CHAR);
		act(AT_MAGIC, "$n's magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_NOTVICT);
		return rSPELL_FAILED;
	}

	separate_obj(obj);
	CREATE(paf, AFFECT_DATA, 1);
	paf->type = -1;
	paf->duration = -1;
	paf->location = APPLY_HITROLL;
	paf->modifier = level /15;
	xCLEAR_BITS(paf->bitvector);
	LINK(paf, obj->first_affect, obj->last_affect, next, prev);

	CREATE(paf, AFFECT_DATA, 1);
	paf->type = -1;
	paf->duration = -1;
	paf->location = APPLY_DAMROLL;
	paf->modifier = level /15;
	xCLEAR_BITS(paf->bitvector);
	LINK(paf, obj->first_affect, obj->last_affect, next, prev);

	xSET_BIT(obj->extra_flags, ITEM_ENCHANTED);

	if(IS_GOOD(ch))
	{
		xSET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		act(AT_BLUE, "$p gleams with flecks of blue energy.", ch, obj, NULL, TO_ROOM);
		act(AT_BLUE, "$p gleams with flecks of blue energy.", ch, obj, NULL, TO_CHAR);
	}
	else if(IS_EVIL(ch))
	{
		xSET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
		act(AT_BLOOD, "A crimson stain flows slowly over $p.", ch, obj, NULL, TO_CHAR);
		act(AT_BLOOD, "A crimson stain flows slowly over $p.", ch, obj, NULL, TO_ROOM);
   }
   else
   {
      xSET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
      xSET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
      act( AT_YELLOW, "$p glows with a disquieting light.", ch, obj, NULL, TO_ROOM );
      act( AT_YELLOW, "$p glows with a disquieting light.", ch, obj, NULL, TO_CHAR );
   }
   return rNONE;
}
ch_ret spell_disenchant_weapon	(int sn, int level, CHAR_DATA *ch, void *vo){  						//DISENCHANT WEAPON
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf, *paf_next;

	if(obj->item_type != ITEM_WEAPON) 							{send_to_char("You can only disenchant weapons.", ch); 						return rSPELL_FAILED;}
	if(!IS_OBJ_STAT(obj, ITEM_ENCHANTED) || !obj->first_affect)	{send_to_char( "This weapon appears to have no enchantments on it.", ch ); 	return rSPELL_FAILED;}

	separate_obj( obj );
	for(paf = obj->first_affect; paf; paf = paf_next)
	{
		paf_next = paf->next;

		if(paf->location == APPLY_HITROLL || paf->location == APPLY_DAMROLL)
		{
			UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
			DISPOSE(paf);
		}
	}

	if(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
	{
		xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		act(AT_YELLOW, "$p momentarily absorbs all disquieting light around it.", ch, obj, NULL, TO_CHAR);  
	}
	
	if(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
	{
		xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		act(AT_RED, "$p momentarily absorbs all red light around it.", ch, obj, NULL, TO_CHAR);
	}
	
	if(IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
	{
		xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		act(AT_BLUE, "$p momentarily absorbs all blue light around it.", ch, obj, NULL, TO_CHAR);
	}
	
	successful_casting(get_skilltype(sn), ch, NULL, obj);
	return rNONE;
}
ch_ret spell_energy_drain		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//ENERGY DRAIN
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	int schance;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET( victim->immune, RIS_MAGIC)) 						{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}

	schance = ris_save( victim, victim->level, RIS_DRAIN );
	if(schance == 1000 || saves_spell_staff(schance, victim)) 	{failed_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}

	ch->alignment = UMAX(-1000, ch->alignment - 200);
	if(victim->level <= 2) dam = ch->hit +1;
	else
	{
		gain_exp(victim, 0 - number_range(level /2, 3 * level /2));
		victim->mana /= 2;
		victim->move /= 2;
		dam = dice(1, level);
		ch->hit += dam;
	}

	if(ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	return damage(ch, victim, dam, sn);
}
ch_ret spell_fireball			(int sn, int level, CHAR_DATA *ch, void *vo){						//FIREBALL
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	
	static const short dam_each[] = 
	{
		0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
		60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
		92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
		112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
		132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
		152, 154, 156, 158, 160, 162, 164, 166, 168, 170
	};
	
   int dam;
   level = UMIN((unsigned int)level, sizeof(dam_each) / sizeof( dam_each[0]) - 1);
   level = UMAX(0, level);
   dam = number_range(dam_each[level] /2, dam_each[level] *2);
   
	if(saves_spell_staff(level, victim))  dam /= 2;
	return damage(ch, victim, dam, sn);
}
ch_ret spell_flamestrike		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//FLAMESTRIKE
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam = dice(6, 8);
	
	if(saves_spell_staff(level, victim))  dam /= 2;
	return damage(ch, victim, dam, sn);
}
ch_ret spell_faerie_fire		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//FAERIE FIRE
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET(victim->immune, RIS_MAGIC))  		{immune_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	if(IS_AFFECTED(victim, AFF_FAERIE_FIRE)) 	{failed_casting(skill, ch, victim, NULL); return rSPELL_FAILED;}
	
	af.type = sn;
	af.duration = (int)(level * DUR_CONV);
	af.location = APPLY_AC;
	af.modifier = 2 * level;
	af.bitvector = meb(AFF_FAERIE_FIRE);
	affect_to_char(victim, &af);
	
	act(AT_PINK, "You are surrounded by a pink outline.", victim, NULL, NULL, TO_CHAR);
	act(AT_PINK, "$n is surrounded by a pink outline.",   victim, NULL, NULL, TO_ROOM);
	return rNONE;
}
ch_ret spell_faerie_fog			(int sn, int level, CHAR_DATA *ch, void *vo){  						//FAERIE FOG
	CHAR_DATA *ich;

	act(AT_MAGIC, "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	act(AT_MAGIC, "You conjure a cloud of purple smoke.", ch, NULL, NULL, TO_CHAR);

	for(ich = ch->in_room->first_person; ich; ich = ich->next_in_room)
	{
		if(!IS_NPC(ich) && xIS_SET(ich->act, PLR_WIZINVIS)) continue;
		if(ich == ch || saves_spell_staff(level, ich))		continue;

		affect_strip(ich, tes_effect_invisibility);
		affect_strip(ich, gsn_mass_invis);
		affect_strip(ich, tes_skill_move_silently);
	  
		xREMOVE_BIT(ich->affected_by, AFF_HIDE);
		xREMOVE_BIT(ich->affected_by, AFF_INVISIBLE);
		xREMOVE_BIT(ich->affected_by, AFF_SNEAK);
	  
		act(AT_MAGIC, "$n is revealed!",   ich, NULL, NULL, TO_ROOM);
		act(AT_MAGIC, "You are revealed!", ich, NULL, NULL, TO_CHAR);
	}
	return rNONE;
}
ch_ret spell_gate				(int sn, int level, CHAR_DATA *ch, void *vo){ 						//GATE
	MOB_INDEX_DATA *temp;
	
	if((temp = get_mob_index(MOB_VNUM_VAMPIRE)) == NULL)
	{
		bug("Spell_gate: Vampire vnum %d doesn't exist.", MOB_VNUM_VAMPIRE);
		return rSPELL_FAILED;
	}
	
	char_to_room(create_mobile(temp), ch->in_room);
	return rNONE;
}
ch_ret spell_harm				(int sn, int level, CHAR_DATA *ch, void *vo){ 						//HARM
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	SKILLTYPE *skill = get_skilltype(sn);

	if(IS_SET(victim->immune, RIS_MAGIC))
	{
		immune_casting(skill, ch, victim, NULL);
		return rSPELL_FAILED;
	}

	dam = UMAX(20, victim->hit - dice(1, 4));
	
	if(saves_spell_staff(level, victim))	dam = UMIN(50, dam /4);
	
	dam = UMIN( 100, dam );
	return damage(ch, victim, dam, sn);
}
ch_ret spell_identify			(int sn, int level, CHAR_DATA *ch, void *vo){						//INDENTIFY
/* Modified by Scryn to work on mobs/players/objs */
/* Made it show short descrs instead of keywords, seeing as you need
   to know the keyword anyways, we may as well make it look nice -- Alty */
   OBJ_DATA *obj;
   CHAR_DATA *victim;
   AFFECT_DATA *paf;
   SKILLTYPE *sktmp;
   SKILLTYPE *skill = get_skilltype( sn );
   const char *name;

   if( target_name[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What should the spell be cast upon?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( ( obj = get_obj_carry( ch, target_name ) ) != NULL )
   {
      set_char_color( AT_LBLUE, ch );
      ch_printf( ch, "\r\nObject '%s' is %s",
                 obj->short_descr, aoran( item_type_name( obj ) ) );
      if( obj->item_type != ITEM_LIGHT && obj->wear_flags - 1 > 0 )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, ", with wear location:  %s", flag_string( obj->wear_flags, w_flags )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch_printf( ch,
                 "Special properties:  %s\r\nIts weight is %d, value is %d, and level is %d.\r\n",
                 extra_bit_name( &obj->extra_flags ),
                 obj->weight, obj->cost, obj->level );
      if( IS_OBJ_STAT( obj, ITEM_PERSONAL ) && obj->owner[0] != '\0' )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&cOwner: &Y%s", obj->owner); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      set_char_color( AT_MAGIC, ch );

      switch ( obj->item_type )
      {
         case ITEM_CONTAINER:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s appears to be %s.", capitalize( obj->short_descr ), obj->value[0] < 76 ? "of a small capacity" :
                       obj->value[0] < 150 ? "of a small to medium capacity" :
                       obj->value[0] < 300 ? "of a medium capacity" :
                       obj->value[0] < 550 ? "of a medium to large capacity" :
                       obj->value[0] < 751 ? "of a large capacity" : "of a giant capacity"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case ITEM_PILL:
         case ITEM_SCROLL:
         case ITEM_POTION:
            ch_printf( ch, "Level %d spells of:", obj->value[0] );

            if( obj->value[1] >= 0 && ( sktmp = get_skilltype( obj->value[1] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }

            if( obj->value[2] >= 0 && ( sktmp = get_skilltype( obj->value[2] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }

            if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }

            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case ITEM_SALVE:
            ch_printf( ch, "Has %d(%d) applications of level %d", obj->value[1], obj->value[2], obj->value[0] );
            if( obj->value[4] >= 0 && ( sktmp = get_skilltype( obj->value[4] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }
            if( obj->value[5] >= 0 && ( sktmp = get_skilltype( obj->value[5] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case ITEM_WAND:
         case ITEM_STAFF:
            ch_printf( ch, "Has %d(%d) charges of level %d", obj->value[1], obj->value[2], obj->value[0] );

            if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
            {
               send_to_char( " '", ch );
               send_to_char( sktmp->name, ch );
               send_to_char( "'", ch );
            }

            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case ITEM_WEAPON:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Damage is %d to %d (average %d)%s", obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2, IS_OBJ_STAT( obj, ITEM_POISONED ) ? ", and is poisonous." : "."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case ITEM_ARMOR:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Armor class is %d.", obj->value[0]); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;
      }

      for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
         showaffect( ch, paf );

      for( paf = obj->first_affect; paf; paf = paf->next )
         showaffect( ch, paf );

      return rNONE;
   }

   else if( ( victim = get_char_room( ch, target_name ) ) != NULL )
   {

      if( IS_SET( victim->immune, RIS_MAGIC ) )
      {
         immune_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      /*
       * If they are morphed or a NPC use the appropriate short_desc otherwise
       * * use their name -- Shaddai
       */
      if( victim->morph && victim->morph->morph )
         name = capitalize( victim->morph->morph->short_desc );
      else if( IS_NPC( victim ) )
         name = capitalize( victim->short_descr );
      else
         name = victim->name;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s appears to be between level %d and %d.", name, victim->level - ( victim->level % 5 ), victim->level - ( victim->level % 5 ) + 5); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( IS_NPC( victim ) && victim->morph )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s appears to truly be %s.", name, ( ch->level > victim->level + 10 ) ? victim->name : "someone else"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s looks like %s, and follows the ways of the %s.", name, aoran( get_race( victim ) ), get_class( victim )); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( ( chance( ch, 50 ) && ch->level >= victim->level + 10 ) || IS_IMMORTAL( ch ) )
      {
         ch_printf( ch, "%s appears to be affected by: ", name );

         if( !victim->first_affect )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "nothing."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rNONE;
         }

         for( paf = victim->first_affect; paf; paf = paf->next )
         {
            if( victim->first_affect != victim->last_affect )
            {
               if( paf != victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
                  ch_printf( ch, "%s, ", sktmp->name );

               if( paf == victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
               {
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "and %s.", sktmp->name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
                  return rNONE;
               }
            }
            else
            {
               if( ( sktmp = get_skilltype( paf->type ) ) != NULL )
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s.", sktmp->name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               else
                  send_to_char( "\r\n", ch );
               return rNONE;
            }
         }
      }
   }

   else
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You can't find %s!", target_name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   return rNONE;
}
ch_ret spell_invis				(int sn, int level, CHAR_DATA *ch, void *vo){						//INVISIBILITY
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype( sn );

/* Modifications on 1/2/96 to work on player/object - Scryn */

   if( target_name[0] == '\0' )
      victim = ch;
   else
      victim = get_char_room( ch, target_name );

   if( victim )
   {
      AFFECT_DATA af;

      if( IS_SET( victim->immune, RIS_MAGIC ) )
      {
         immune_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      if( IS_AFFECTED( victim, AFF_INVISIBLE ) )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      act( AT_MAGIC, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
      af.type = sn;
      af.duration = ( int )( ( ( level / 4 ) + 12 ) * DUR_CONV );
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb( AFF_INVISIBLE );
      affect_to_char( victim, &af );
      act( AT_MAGIC, "You fade out of existence.", victim, NULL, NULL, TO_CHAR );
      return rNONE;
   }
   else
   {
      OBJ_DATA *obj;

      obj = get_obj_carry( ch, target_name );

      if( obj )
      {
         separate_obj( obj ); /* Fix multi-invis bug --Blod */
         if( IS_OBJ_STAT( obj, ITEM_INVIS ) || chance( ch, 40 + level / 10 ) )
         {
            failed_casting( skill, ch, NULL, NULL );
            return rSPELL_FAILED;
         }

         xSET_BIT( obj->extra_flags, ITEM_INVIS );
         act( AT_MAGIC, "$p fades out of existence.", ch, obj, NULL, TO_CHAR );
         return rNONE;
      }
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You can't find %s!", target_name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return rSPELL_FAILED;
}
ch_ret spell_know_alignment		(int sn, int level, CHAR_DATA *ch, void *vo){						//KNOW ALIGNMENT
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   const char *msg;
   int ap;
   SKILLTYPE *skill = get_skilltype( sn );

   if( !victim )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( IS_SET( victim->immune, RIS_MAGIC ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   ap = victim->alignment;

   if( ap > 700 )
      msg = "$N has an aura as white as the driven snow.";
   else if( ap > 350 )
      msg = "$N is of excellent moral character.";
   else if( ap > 100 )
      msg = "$N is often kind and thoughtful.";
   else if( ap > -100 )
      msg = "$N doesn't have a firm moral commitment.";
   else if( ap > -350 )
      msg = "$N lies to $S friends.";
   else if( ap > -700 )
      msg = "$N would just as soon kill you as look at you.";
   else
      msg = "I'd rather just not say anything at all about $N.";

   act( AT_MAGIC, msg, ch, NULL, victim, TO_CHAR );
   return rNONE;
}
ch_ret spell_lightning_bolt     (int sn, int level, CHAR_DATA *ch, void *vo){						//LIGHTNING BOLT
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   static const short dam_each[] = {
      0,
      0, 0, 0, 0, 0, 0, 0, 0, 25, 28,
      31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
      44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
      51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
      58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
      64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
      70, 71, 71, 72, 72, 73, 73, 74, 75, 75
   };
   int dam;

   level = UMIN( ( unsigned int )level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
   level = UMAX( 0, level );
   dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_locate_object		(int sn, int level, CHAR_DATA *ch, void *vo){						//LOCATE OBJECT
   char buf[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int cnt, found = 0;

   for( obj = first_object; obj; obj = obj->next )
   {
      if( !can_see_obj( ch, obj ) || !nifty_is_name( target_name, obj->name ) )
         continue;
      if( ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) || IS_OBJ_STAT( obj, ITEM_NOLOCATE ) ) && !IS_IMMORTAL( ch ) )
         continue;

      found++;

      for( cnt = 0, in_obj = obj; in_obj->in_obj && cnt < 100; in_obj = in_obj->in_obj, ++cnt )
         ;
      if( cnt >= MAX_NEST )
      {
         bug( "spell_locate_obj: object [%d] %s is nested more than %d times!",
              obj->pIndexData->vnum, obj->short_descr, MAX_NEST );
         continue;
      }

      if( in_obj->carried_by )
      {
         if( IS_IMMORTAL( in_obj->carried_by )
             && !IS_NPC( in_obj->carried_by )
             && ( get_trust( ch ) < in_obj->carried_by->pcdata->wizinvis )
             && xIS_SET( in_obj->carried_by->act, PLR_WIZINVIS ) )
         {
            found--;
            continue;
         }

         snprintf( buf, MAX_INPUT_LENGTH, "%s carried by %s.\r\n", obj_short( obj ), PERS( in_obj->carried_by, ch ) );
      }
      else
         snprintf( buf, MAX_INPUT_LENGTH, "%s in %s.\r\n", obj_short( obj ),
                   in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name );

      buf[0] = UPPER( buf[0] );
      set_char_color( AT_MAGIC, ch );

/*
	Gorog added this 98/09/02 but obj_short(obj) now nukes memory
	and crashes us when the resulting buffer is sent to the pager
	(not confined to just pager_printf).  Something else somewhere
	else must have changed recently to exacerbate this problem,
	as it is now a guaranteed crash cause. - Blodkai
	pager_printf( ch, buf );
*/
      send_to_char( buf, ch );
   }

   if( !found )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Nothing like that exists."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   return rNONE;
}
ch_ret spell_magic_missile		(int sn, int level, CHAR_DATA *ch, void *vo){						//MAGIC MISSILES
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   static const short dam_each[] = {
      0,
      3, 3, 4, 4, 5, 6, 6, 6, 6, 6,
      7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
      9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
      11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
      13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
      15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
      17, 17, 17, 17, 17, 18, 18, 18, 18, 18
   };
   int dam;

   level = UMIN( ( unsigned int )level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
   level = UMAX( 0, level );
   dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   /*
    * What's this?  You can't save vs. magic missile!      -Thoric
    * if ( saves_spell( level, victim ) )
    * dam /= 2;
    */
   return damage( ch, victim, dam, sn );
}
ch_ret spell_pass_door			(int sn, int level, CHAR_DATA *ch, void *vo){						//PASS DOOR
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype( sn );

   if( IS_SET( victim->immune, RIS_MAGIC ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = ( int )( number_fuzzy( level / 4 ) * DUR_CONV );
   af.location = APPLY_NONE;
   af.modifier = 0;
   af.bitvector = meb( AFF_PASS_DOOR );
   affect_to_char( victim, &af );
   act( AT_MAGIC, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
   act( AT_MAGIC, "You turn translucent.", victim, NULL, NULL, TO_CHAR );
   return rNONE;
}
ch_ret spell_poison				(int sn, int level, CHAR_DATA *ch, void *vo){						//POISON
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   int schance;
   bool first = TRUE;

   schance = ris_save( victim, level, RIS_POISON );
   if( schance == 1000 || saves_poison_death( schance, victim ) )
   {
      set_char_color( AT_MAGIC, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your magic fails to take hold."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   if( IS_AFFECTED( victim, AFF_POISON ) )
      first = FALSE;
   af.type = sn;
   af.duration = ( int )( level * DUR_CONV );
   af.location = APPLY_STR;
   af.modifier = -2;
   af.bitvector = meb( AFF_POISON );
   affect_join( victim, &af );
   set_char_color( AT_GREEN, victim );
   send_to_char( "You feel very sick.\r\n", victim );
   if( IS_PKILL( victim ) )
      victim->mental_state = URANGE( 10, victim->mental_state + ( first ? 5 : 0 ), 100 );
   else
      victim->mental_state = URANGE( 20, victim->mental_state + ( first ? 5 : 0 ), 100 );
   if( ch != victim )
   {
      act( AT_GREEN, "$N shivers as your poison spreads through $S body.", ch, NULL, victim, TO_CHAR );
      act( AT_GREEN, "$N shivers as $n's poison spreads through $S body.", ch, NULL, victim, TO_NOTVICT );
   }
   return rNONE;
}
ch_ret spell_remove_curse		(int sn, int level, CHAR_DATA *ch, void *vo){						//REMOVE CURSE
   OBJ_DATA *obj;
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   SKILLTYPE *skill = get_skilltype( sn );

   if( IS_SET( victim->immune, RIS_MAGIC ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( is_affected( victim, gsn_curse ) )
   {
      affect_strip( victim, gsn_curse );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "The weight of your curse is lifted.\r\n", victim );
      if( ch != victim )
      {
         act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
      }
   }
   else if( victim->first_carrying )
   {
      for( obj = victim->first_carrying; obj; obj = obj->next_content )
         if( !obj->in_obj && ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) || IS_OBJ_STAT( obj, ITEM_NODROP ) ) )
         {
            if( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
               xREMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
            if( IS_OBJ_STAT( obj, ITEM_NODROP ) )
               xREMOVE_BIT( obj->extra_flags, ITEM_NODROP );
            set_char_color( AT_MAGIC, victim );
            send_to_char( "You feel a burden released.\r\n", victim );
            if( ch != victim )
            {
               act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
               act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
            }
            return rNONE;
         }
   }
   return rNONE;
}
ch_ret spell_remove_trap		(int sn, int level, CHAR_DATA *ch, void *vo){						//REMOVE TRAP
   OBJ_DATA *obj;
   OBJ_DATA *trap;
   bool found;
   int retcode;
   SKILLTYPE *skill = get_skilltype( sn );

   if( !target_name || target_name[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Remove trap on what?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   found = FALSE;

   if( !ch->in_room->first_content )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't find that here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( target_name, obj->name ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't find that here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( ( trap = get_trap( obj ) ) == NULL )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }

   if( !chance( ch, 70 + get_curr_wis( ch ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ooops!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      retcode = spring_trap( ch, trap );
      if( retcode == rNONE )
         retcode = rSPELL_FAILED;
      return retcode;
   }

   extract_obj( trap );

   successful_casting( skill, ch, NULL, NULL );
   return rNONE;
}
ch_ret spell_shocking_grasp		(int sn, int level, CHAR_DATA *ch, void *vo){						//SHOCKING GRASP
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   static const int dam_each[] = {
      0,
      0, 0, 0, 0, 0, 0, 20, 25, 29, 33,
      36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
      43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
      48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
      53, 53, 54, 54, 55, 55, 56, 56, 57, 57,
      58, 58, 59, 59, 60, 60, 61, 61, 62, 62,
      63, 63, 64, 64, 65, 65, 66, 66, 67, 67
   };
   int dam;

   level = UMIN( ( unsigned int )level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
   level = UMAX( 0, level );
   dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_sleep				(int sn, int level, CHAR_DATA *ch, void *vo){						//SLEEP
   AFFECT_DATA af;
   int retcode;
   int schance;
   int tmp;
   CHAR_DATA *victim;
   char log_buf[MAX_STRING_LENGTH];
   SKILLTYPE *skill = get_skilltype( sn );

   if( ( victim = get_char_room( ch, target_name ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( !IS_NPC( victim ) && victim->fighting )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot sleep a fighting player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( is_safe( ch, victim, TRUE ) )
      return rSPELL_FAILED;

   if( IS_SET( victim->immune, RIS_MAGIC ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( SPELL_FLAG( skill, SF_PKSENSITIVE ) && !IS_NPC( ch ) && !IS_NPC( victim ) )
      tmp = level / 2;
   else
      tmp = level;

   if( IS_AFFECTED( victim, AFF_SLEEP )
       || ( schance = ris_save( victim, tmp, RIS_SLEEP ) ) == 1000
       || level < victim->level
       || ( victim != ch && xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) ) || saves_spell_staff( schance, victim ) )
   {
      failed_casting( skill, ch, victim, NULL );
      if( ch == victim )
         return rSPELL_FAILED;
      if( !victim->fighting )
      {
         retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
         if( retcode == rNONE )
            retcode = rSPELL_FAILED;
         return retcode;
      }
   }
   af.type = sn;
   af.duration = ( int )( ( 4 + level ) * DUR_CONV );
   af.location = APPLY_NONE;
   af.modifier = 0;
   af.bitvector = meb( AFF_SLEEP );
   affect_join( victim, &af );

   /*
    * Added by Narn at the request of Dominus. 
    */
   if( !IS_NPC( victim ) )
   {
      snprintf( log_buf, MAX_STRING_LENGTH, "%s has cast sleep on %s.", ch->name, victim->name );
      log_string_plus( log_buf, LOG_NORMAL, ch->level );
      to_channel( log_buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
   }

   if( IS_AWAKE( victim ) )
   {
      act( AT_MAGIC, "You feel very sleepy ..... zzzzzz.", victim, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
      victim->position = POS_SLEEPING;
   }
   if( IS_NPC( victim ) )
      start_hating( victim, ch );

   return rNONE;
}
ch_ret spell_summon				(int sn, int level, CHAR_DATA *ch, void *vo){						//SUMMON
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   SKILLTYPE *skill = get_skilltype( sn );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || !victim->in_room
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_SAFE )
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_SUMMON )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 3
       || victim->fighting
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_AGGRESSIVE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || !in_hard_range( victim, ch->in_room->area )
       || ( !IS_NPC( ch ) && !CAN_PKILL( ch ) && IS_PKILL( victim ) )
       || ( IS_SET( ch->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( victim ) )
       || ( !IS_NPC( ch ) && !IS_NPC( victim ) && IS_SET( victim->pcdata->flags, PCFLAG_NOSUMMON ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( ch->in_room->area != victim->in_room->area )
   {
      if( ( ( IS_NPC( ch ) != IS_NPC( victim ) ) && chance( ch, 30 ) )
          || ( ( IS_NPC( ch ) == IS_NPC( victim ) ) && chance( ch, 60 ) ) )
      {
         failed_casting( skill, ch, victim, NULL );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "You feel a strange pulling sensation...\r\n", victim );
         return rSPELL_FAILED;
      }
   }

   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "You feel a wave of nausea overcome you...", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n collapses, stunned!", ch, NULL, NULL, TO_ROOM );
      ch->position = POS_STUNNED;

      snprintf( buf, MAX_STRING_LENGTH, "%s summoned %s to room %d.", ch->name, victim->name, ch->in_room->vnum );
      log_string_plus( buf, LOG_NORMAL, ch->level );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
   }

   act( AT_MAGIC, "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
   char_from_room( victim );
   char_to_room( victim, ch->in_room );
   act( AT_MAGIC, "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
   act( AT_MAGIC, "$N has summoned you!", victim, NULL, ch, TO_CHAR );
   do_look( victim, "auto" );
   return rNONE;
}
ch_ret spell_astral_walk		(int sn, int level, CHAR_DATA *ch, void *vo){						//ASTRAL WALK
   CHAR_DATA *victim;
   struct skill_type *skill = get_skilltype( sn );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || !can_astral( ch, victim ) || !in_hard_range( ch, victim->in_room->area ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( skill->hit_char && skill->hit_char[0] != '\0' )
      act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );

   if( skill->hit_room && skill->hit_room[0] != '\0' )
      act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
   else
      act( AT_MAGIC, "$n disappears in a flash of light!", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   if( skill->hit_dest && skill->hit_dest[0] != '\0' )
      act( AT_MAGIC, skill->hit_dest, ch, NULL, victim, TO_NOTVICT );
   else
      act( AT_MAGIC, "$n appears in a flash of light!", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return rNONE;
}
ch_ret spell_teleport			(int sn, int level, CHAR_DATA *ch, void *vo){						//TELEPORT
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   ROOM_INDEX_DATA *pRoomIndex;
   SKILLTYPE *skill = get_skilltype( sn );

   if( !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
       || ( !IS_NPC( ch ) && victim->fighting )
       || ( victim != ch && ( saves_spell_staff( level, victim ) || saves_wands( level, victim ) ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
      if( pRoomIndex )
         if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL )
             && !IS_SET( pRoomIndex->area->flags, AFLAG_NOTELEPORT )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && in_hard_range( ch, pRoomIndex->area ) )
            break;
   }

   act( AT_MAGIC, "$n slowly fades out of view.", victim, NULL, NULL, TO_ROOM );
   char_from_room( victim );
   char_to_room( victim, pRoomIndex );
   if( !IS_NPC( victim ) )
      act( AT_MAGIC, "$n slowly fades into view.", victim, NULL, NULL, TO_ROOM );
   do_look( victim, "auto" );
   return rNONE;
}
int number_in_group				(CHAR_DATA *ch ){													//MEMBER IN GROUP
	//Returns the number ot people within a grouped party	
   CHAR_DATA *nch, *nch_next;
   ROOM_INDEX_DATA *start = ch->in_room;
   int number = 0;

   for( nch = start->first_person; nch; nch = nch_next )
   {
      nch_next = nch->next_in_room;
      if( is_same_group( nch, ch ) )
         number += 1;
   }
   return number;
}
ch_ret spell_group_teleport		(int sn, int level, CHAR_DATA *ch, void *vo){						//GROUP TELEPORT
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   CHAR_DATA *nch, *nch_next;
   ROOM_INDEX_DATA *start = ch->in_room, *pRoomIndex;
   SKILLTYPE *skill = get_skilltype( sn );
   int mana;

   if( !victim )
      victim = ch;

   if( !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
       || ( !IS_NPC( ch ) && victim->fighting )
       || ( victim != ch && ( saves_spell_staff( level, victim ) || saves_wands( level, victim ) ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
      if( pRoomIndex )
         if( pRoomIndex != start )
            if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
                && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
                && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL )
                && !IS_SET( pRoomIndex->area->flags, AFLAG_NOTELEPORT )
                && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
                && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && in_hard_range( ch, pRoomIndex->area ) )
               break;
   }

   mana = number_in_group( victim ) * 100;
   if( !IS_NPC( ch ) && ch->mana < mana )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't have enough mana."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   act( AT_MAGIC, "$n's arms rise towards the sky, a tornado engulfs the area.", victim, NULL, NULL, TO_ROOM );
   act( AT_MAGIC, "$n's group is sucked into the raging tornadoes.", victim, NULL, NULL, TO_ROOM );

   char_from_room( victim );
   char_to_room( victim, pRoomIndex );
   do_look( victim, "auto" );

   for( nch = start->first_person; nch; nch = nch_next )
   {
      nch_next = nch->next_in_room;
      if( is_same_group( nch, victim ) && ( in_hard_range( nch, pRoomIndex->area ) ) )
      {
         char_from_room( nch );
         char_to_room( nch, pRoomIndex );
         if( !IS_NPC( nch ) )
            do_look( nch, "auto" );
      }
   }
   victim->mana -= mana;
   return rNONE;
}
ch_ret spell_ventriloquate		(int sn, int level, CHAR_DATA *ch, void *vo){ 						//VENTRILOQUATE
   char buf1[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   char speaker[MAX_INPUT_LENGTH];
   CHAR_DATA *vch;

   target_name = one_argument( target_name, speaker );
   if( speaker[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your voice sputters as you try to cast it in random directions."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   snprintf( buf1, MAX_STRING_LENGTH, "%s says '%s'.\r\n", speaker, target_name );
   snprintf( buf2, MAX_STRING_LENGTH, "Someone makes %s say '%s'.\r\n", speaker, target_name );
   snprintf( buf3, MAX_STRING_LENGTH, "%s makes %s say '%s'.\r\n", capitalize( ch->name ), speaker, target_name );
   buf1[0] = UPPER( buf1[0] );

   for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
   {
      if( !is_name( speaker, vch->name ) )
      {
         set_char_color( AT_SAY, vch );
         if( get_trust( vch ) >= LEVEL_IMMORTAL )
            send_to_char( buf3, vch );
         else
            send_to_char( saves_spell_staff( level, vch ) ? buf2 : buf1, vch );
      }
   }
   return rNONE;
}
ch_ret spell_weaken				(int sn, int level, CHAR_DATA *ch, void *vo){						//WEAKEN
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype( sn );

   set_char_color( AT_MAGIC, ch );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( is_affected( victim, sn ) || saves_wands( level, victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your magic fails to take hold."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = ( int )( level / 2 * DUR_CONV );
   af.location = APPLY_STR;
   af.modifier = -2;
   xCLEAR_BITS( af.bitvector );
   affect_to_char( victim, &af );
   set_char_color( AT_MAGIC, victim );
   send_to_char( "Your muscles seem to atrophy!\r\n", victim );
   if( ch != victim )
   {
      if( ( ( ( !IS_NPC( victim ) && class_table[victim->Class]->attr_prime == APPLY_STR )
              || IS_NPC( victim ) ) && get_curr_str( victim ) < 25 ) || get_curr_str( victim ) < 20 )
      {
         act( AT_MAGIC, "$N labors weakly as your spell atrophies $S muscles.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$N labors weakly as $n's spell atrophies $S muscles.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
         act( AT_MAGIC, "You induce a mild atrophy in $N's muscles.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n induces a mild atrophy in $N's muscles.", ch, NULL, victim, TO_NOTVICT );
      }
   }
   return rNONE;
}
ch_ret spell_word_of_recall		(int sn, int level, CHAR_DATA *ch, void *vo){						//Recall
   do_recall( ( CHAR_DATA * ) vo, "" );
   return rNONE;
}
ch_ret spell_acid_breath		(int sn, int level, CHAR_DATA *ch, void *vo){						//ACID BREATH
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam;
   int hpch;

   if( chance( ch, 2 * level ) && !saves_breath( level, victim ) )
   {
      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         int iWear;

         obj_next = obj_lose->next_content;

         if( number_bits( 2 ) != 0 )
            continue;

         switch ( obj_lose->item_type )
         {
            case ITEM_ARMOR:
               if( obj_lose->value[0] > 0 )
               {
                  separate_obj( obj_lose );
                  act( AT_DAMAGE, "$p is pitted and etched!", victim, obj_lose, NULL, TO_CHAR );
                  if( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
                     victim->armor += apply_ac( obj_lose, iWear, ch);   /* <-- victim is LOSING the benefit of obj->value[0] */
                  obj_lose->value[0] -= 1;   /* so we need to ADD to his AC */
                  obj_lose->cost = 0;
                  if( iWear != WEAR_NONE )
                     victim->armor -= apply_ac( obj_lose, iWear, ch);
                  /*
                   * <-- victim now regains the benefit of the adjusted 
                   */
               }  /* obj->value[0] so we need to SUBTRACT to his AC */
               break;

            case ITEM_CONTAINER:
               separate_obj( obj_lose );
               act( AT_DAMAGE, "$p fumes and dissolves!", victim, obj_lose, NULL, TO_CHAR );
               act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM );
               act( AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR );
               empty_obj( obj_lose, NULL, victim->in_room );
               extract_obj( obj_lose );
               break;
         }
      }
   }

   hpch = UMAX( 10, ch->hit );
   dam = number_range( hpch / 16 + 1, hpch / 8 );
   if( saves_breath( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_fire_breath		(int sn, int level, CHAR_DATA *ch, void *vo){						//FIRE BREATH
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam;
   int hpch;

   if( chance( ch, 2 * level ) && !saves_breath( level, victim ) )
   {
      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         const char *msg;

         obj_next = obj_lose->next_content;
         if( number_bits( 2 ) != 0 )
            continue;

         switch ( obj_lose->item_type )
         {
            default:
               continue;
            case ITEM_CONTAINER:
               msg = "$p ignites and burns!";
               break;
            case ITEM_POTION:
               msg = "$p bubbles and boils!";
               break;
            case ITEM_SCROLL:
               msg = "$p crackles and burns!";
               break;
            case ITEM_STAFF:
               msg = "$p smokes and chars!";
               break;
            case ITEM_WAND:
               msg = "$p sparks and sputters!";
               break;
            case ITEM_COOK:
            case ITEM_FOOD:
               msg = "$p blackens and crisps!";
               break;
            case ITEM_PILL:
               msg = "$p melts and drips!";
               break;
         }

         separate_obj( obj_lose );
         act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
         if( obj_lose->item_type == ITEM_CONTAINER )
         {
            act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM );
            act( AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR );
            empty_obj( obj_lose, NULL, victim->in_room );
         }
         extract_obj( obj_lose );
      }
   }

   hpch = UMAX( 10, ch->hit );
   dam = number_range( hpch / 16 + 1, hpch / 8 );
   if( saves_breath( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_frost_breath		(int sn, int level, CHAR_DATA *ch, void *vo){						//FROST BREATH
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam;
   int hpch;

   if( chance( ch, 2 * level ) && !saves_breath( level, victim ) )
   {
      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         const char *msg;

         obj_next = obj_lose->next_content;
         if( number_bits( 2 ) != 0 )
            continue;

         switch ( obj_lose->item_type )
         {
            default:
               continue;
            case ITEM_CONTAINER:
            case ITEM_DRINK_CON:
            case ITEM_POTION:
               msg = "$p freezes and shatters!";
               break;
         }

         separate_obj( obj_lose );
         act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
         if( obj_lose->item_type == ITEM_CONTAINER )
         {
            act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM );
            act( AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR );
            empty_obj( obj_lose, NULL, victim->in_room );
         }
         extract_obj( obj_lose );
      }
   }

   hpch = UMAX( 10, ch->hit );
   dam = number_range( hpch / 16 + 1, hpch / 8 );
   if( saves_breath( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_gas_breath			(int sn, int level, CHAR_DATA *ch, void *vo){						//GAS BREATH
   CHAR_DATA *vch;
   TRV_DATA *loop_ctrl;
   int dam, hpch;
   bool ch_died = FALSE;

   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You fail to breathe."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   loop_ctrl = trvch_create( ch, TR_CHAR_ROOM_FORW );
   for( vch = ch->in_room->first_person; vch; vch = trvch_next( loop_ctrl ) )
   {
      if( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
         continue;

      if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
         continue;

      if( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
      {
         hpch = UMAX( 10, ch->hit );
         dam = number_range( hpch / 16 + 1, hpch / 8 );
         if( saves_breath( level, vch ) )
            dam /= 2;
         if( damage( ch, vch, dam, sn ) == rCHAR_DIED || char_died( ch ) )
            ch_died = TRUE;
      }
   }
   trv_dispose( &loop_ctrl );
   return ch_died ? rCHAR_DIED : rNONE;
}
ch_ret spell_lightning_breath	(int sn, int level, CHAR_DATA *ch, void *vo){						//LIGHTNING BREATH
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;
   int hpch;

   hpch = UMAX( 10, ch->hit );
   dam = number_range( hpch / 16 + 1, hpch / 8 );
   if( saves_breath( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_null				(int sn, int level, CHAR_DATA *ch, void *vo){						//SPELL NULL
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That's not a spell!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return rNONE;
}
ch_ret spell_notfound			(int sn, int level, CHAR_DATA *ch, void *vo){						//SPELL NOT FOUND
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That's not a spell!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   return rNONE;
}
ch_ret spell_transport			(int sn, int level, CHAR_DATA *ch, void *vo){						//TRANSPORT
   CHAR_DATA *victim;
   char arg3[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   SKILLTYPE *skill = get_skilltype( sn );

   target_name = one_argument( target_name, arg3 );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 15
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( victim->in_room == ch->in_room )
   {
      send_to_char( "They are right beside you!", ch );
      return rSPELL_FAILED;
   }

   if( ( obj = get_obj_carry( ch, arg3 ) ) == NULL
       || ( victim->carry_weight + get_obj_weight( obj ) ) > can_carry_w( victim )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   separate_obj( obj ); /* altrag shoots, haus alley-oops! */

   if( IS_OBJ_STAT( obj, ITEM_NODROP ) || IS_OBJ_STAT( obj, ITEM_PERMANENT ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't seem to let go of it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;   /* nice catch, caine */
   }

   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && get_trust( victim ) < LEVEL_IMMORTAL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That item is not for mortal hands to touch!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;   /* Thoric */
   }

   act( AT_MAGIC, "$p slowly dematerializes...", ch, obj, NULL, TO_CHAR );
   act( AT_MAGIC, "$p slowly dematerializes from $n's hands..", ch, obj, NULL, TO_ROOM );
   obj_from_char( obj );
   obj_to_char( obj, victim );
   act( AT_MAGIC, "$p from $n appears in your hands!", ch, obj, victim, TO_VICT );
   act( AT_MAGIC, "$p appears in $n's hands!", victim, obj, NULL, TO_ROOM );
   save_char_obj( ch );
   save_char_obj( victim );
   return rNONE;
}
ch_ret spell_close_portal		(int sn, int level, CHAR_DATA *ch, void *vo){						//CLOSE PORTAL

   OBJ_DATA *obj;
   OBJ_DATA *obj_next;

   for( obj = ch->in_room->first_content; obj; obj = obj_next )
   {
      obj_next = obj->next_content;

      if( obj->item_type == ITEM_PORTAL )
      {
         remove_portal( obj );
         obj->item_type = ITEM_TRASH;
         extract_obj( obj );
         act( AT_MAGIC, "As $n utters a lengthy incantation, the portal spins into nothingness.", ch, NULL, NULL, TO_ROOM );
         act( AT_MAGIC, "As you utter a lengthy incantation, the portal spins into nothingness.", ch, NULL, NULL, TO_CHAR );
      }
   }
   return rNONE;
}
ch_ret spell_portal				(int sn, int level, CHAR_DATA *ch, void *vo){						//PORTAL
   CHAR_DATA *victim;
   ROOM_INDEX_DATA *targetRoom, *fromRoom;
   int targetRoomVnum;
   OBJ_DATA *portalObj;
   EXIT_DATA *pexit;
   char buf[MAX_STRING_LENGTH];
   SKILLTYPE *skill = get_skilltype( sn );

   /*
    * No go if all kinds of things aren't just right, including the caster
    * and victim are not both pkill or both peaceful. -- Narn
    */
   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOPORTALOUT )
       || IS_SET( victim->in_room->area->flags, AFLAG_NOPORTALIN )
       || IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
       || victim->level >= level + 15
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || ( !IS_NPC( victim ) && CAN_PKILL( ch ) != CAN_PKILL( victim ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( victim->in_room == ch->in_room )
   {
      send_to_char( "They are right beside you!", ch );
      return rSPELL_FAILED;
   }

   targetRoomVnum = victim->in_room->vnum;
   fromRoom = ch->in_room;
   targetRoom = victim->in_room;

   /*
    * Check if there already is a portal in either room. 
    */
   for( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
   {
      if( IS_SET( pexit->exit_info, EX_PORTAL ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is already a portal in this room."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rSPELL_FAILED;
      }

      if( pexit->vdir == DIR_PORTAL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You may not create a portal in this room."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rSPELL_FAILED;
      }
   }

   for( pexit = targetRoom->first_exit; pexit; pexit = pexit->next )
      if( pexit->vdir == DIR_PORTAL )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

   pexit = make_exit( fromRoom, targetRoom, DIR_PORTAL );
   pexit->keyword = STRALLOC( "portal" );
   pexit->description = STRALLOC( "You gaze into the shimmering portal...\r\n" );
   pexit->key = -1;
   pexit->exit_info = EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
   pexit->vnum = targetRoomVnum;

   portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
   portalObj->timer = 3;

   snprintf( buf, MAX_STRING_LENGTH, "a portal created by %s", ch->name );
   STRFREE( portalObj->short_descr );
   portalObj->short_descr = STRALLOC( buf );

   /*
    * support for new casting messages 
    */
   if( !skill->hit_char || skill->hit_char[0] == '\0' )
   {
      set_char_color( AT_MAGIC, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You utter an incantation, and a portal forms in front of you!"); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
   }
   else
      act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
   if( !skill->hit_room || skill->hit_room[0] == '\0' )
      act( AT_MAGIC, "$n utters an incantation, and a portal forms in front of you!", ch, NULL, NULL, TO_ROOM );
   else
      act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
   if( !skill->hit_vict || skill->hit_vict[0] == '\0' )
      act( AT_MAGIC, "A shimmering portal forms in front of you!", victim, NULL, NULL, TO_ROOM );
   else
      act( AT_MAGIC, skill->hit_vict, victim, NULL, victim, TO_ROOM );
   portalObj = obj_to_room( portalObj, ch->in_room );

   pexit = make_exit( targetRoom, fromRoom, DIR_PORTAL );
   pexit->keyword = STRALLOC( "portal" );
   pexit->description = STRALLOC( "You gaze into the shimmering portal...\r\n" );
   pexit->key = -1;
   pexit->exit_info = EX_PORTAL | EX_xENTER | EX_HIDDEN;
   pexit->vnum = targetRoomVnum;

   portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
   portalObj->timer = 3;
   STRFREE( portalObj->short_descr );
   portalObj->short_descr = STRALLOC( buf );
   portalObj = obj_to_room( portalObj, targetRoom );

   return rNONE;
}
ch_ret spell_farsight			(int sn, int level, CHAR_DATA *ch, void *vo){						//FARSIGHT
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype( sn );

   if((   victim = get_char_world(ch, target_name)) == NULL
       || victim == ch
       || !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 15
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || ( !IS_NPC( victim ) && CAN_PKILL( victim ) && !CAN_PKILL( ch ) ) )
     {
     failed_casting( skill, ch, victim, NULL );
     return rSPELL_FAILED;
     }

     location = victim->in_room;

   if(!location)
     {
     failed_casting( skill, ch, victim, NULL );
     return rSPELL_FAILED;
     }

   successful_casting( skill, ch, victim, NULL );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   do_look( ch, "auto" );
   char_from_room( ch );
   char_to_room( ch, original );

   if( chance_attrib( victim, 20, get_curr_wis( victim ) ) && !IS_PKILL( ch ) )
      send_to_char( "You get an uneasy feeling that you are being watched.\r\n", victim );
   return rNONE;
}
ch_ret spell_recharge			(int sn, int level, CHAR_DATA *ch, void *vo){						//RECHARGE
   OBJ_DATA *obj = ( OBJ_DATA * ) vo;

   if( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
   {
      separate_obj( obj );
      if( obj->value[2] == obj->value[1] || obj->value[1] > ( obj->pIndexData->value[1] * 4 ) )
      {
         act( AT_FIRE, "$p bursts into flames, injuring you!", ch, obj, NULL, TO_CHAR );
         act( AT_FIRE, "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM );
         extract_obj( obj );
         if( damage( ch, ch, obj->level * 2, TYPE_UNDEFINED ) == rCHAR_DIED || char_died( ch ) )
            return rCHAR_DIED;
         else
            return rSPELL_FAILED;
      }

      if( chance( ch, 2 ) )
      {
         act( AT_YELLOW, "$p glows with a blinding magical luminescence.", ch, obj, NULL, TO_CHAR );
         obj->value[1] *= 2;
         obj->value[2] = obj->value[1];
         return rNONE;
      }
      else if( chance( ch, 5 ) )
      {
         act( AT_YELLOW, "$p glows brightly for a few seconds...", ch, obj, NULL, TO_CHAR );
         obj->value[2] = obj->value[1];
         return rNONE;
      }
      else if( chance( ch, 10 ) )
      {
         act( AT_WHITE, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR );
         act( AT_WHITE, "$n's attempt at recharging fails, and $p disintegrates.", ch, obj, NULL, TO_ROOM );
         extract_obj( obj );
         return rSPELL_FAILED;
      }
      else if( chance( ch, 50 - ( ch->level / 2 ) ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Nothing happens."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rSPELL_FAILED;
      }
      else
      {
         act( AT_MAGIC, "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR );
         --obj->value[1];
         obj->value[2] = obj->value[1];
         return rNONE;
      }
   }
   else
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't recharge that!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
}
ch_ret spell_plant_pass			(int sn, int level, CHAR_DATA *ch, void *vo){						//PLANT PASS
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype( sn );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
       || ( victim->in_room->sector_type != SECT_FOREST
            && victim->in_room->sector_type != SECT_FIELD )
       || ( ch->in_room->sector_type != SECT_FOREST
            && ch->in_room->sector_type != SECT_FIELD )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 15
       || ( CAN_PKILL( victim ) && !IS_NPC( ch ) && !IS_PKILL( ch ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || !in_hard_range( ch, victim->in_room->area )
       || ( IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( ch ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   if( ch->in_room->sector_type == SECT_FOREST )
      act( AT_MAGIC, "$n melds into a nearby tree!", ch, NULL, NULL, TO_ROOM );
   else
      act( AT_MAGIC, "$n melds into the grass!", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   if( ch->in_room->sector_type == SECT_FOREST )
      act( AT_MAGIC, "$n appears from behind a nearby tree!", ch, NULL, NULL, TO_ROOM );
   else
      act( AT_MAGIC, "$n grows up from the grass!", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return rNONE;
}
ch_ret spell_mist_walk			(int sn, int level, CHAR_DATA *ch, void *vo){						//MIST WALK
   CHAR_DATA *victim;
   bool allowday;
   SKILLTYPE *skill = get_skilltype( sn );

   set_char_color( AT_DGREEN, ch );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL || victim == ch )
   {
      failed_casting( skill, ch, victim, NULL );
      send_to_char( "You cannot sense your victim...", ch );
      return rSPELL_FAILED;
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
       || IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 15
       || ( CAN_PKILL( victim ) && !IS_NPC( ch ) && !IS_PKILL( ch ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || !in_hard_range( ch, victim->in_room->area )
       || ( IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( ch ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      send_to_char( "You cannot sense your victim...", ch );
      return rSPELL_FAILED;
   }

   /*
    * Subtract 22 extra bp for mist walk from 0500 to 2100 SB 
    */
   if( time_info.hour < 21 && time_info.hour > 5 && !IS_NPC( ch ) )
      gain_condition( ch, COND_BLOODTHIRST, -22 );

   act( AT_DGREEN, "$n dissolves into a cloud of glowing mist, then vanishes!", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   act( AT_DGREEN, "A cloud of glowing mist engulfs you, then withdraws to unveil $n!", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return rNONE;
}
ch_ret spell_solar_flight		(int sn, int level, CHAR_DATA *ch, void *vo){						//SOLAR FLIGHT
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype( sn );
   struct WeatherCell *cell = getWeatherCell( ch->in_room->area );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || ( time_info.hour > 18 || time_info.hour < 8 )
       || !victim->in_room
       || !IS_OUTSIDE( ch )
       || !IS_OUTSIDE( victim )
       || getCloudCover( cell ) >= 20
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
       || victim->level >= level + 15
       || ( CAN_PKILL( victim ) && !IS_NPC( ch ) && !IS_PKILL( ch ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || !in_hard_range( ch, victim->in_room->area )
       || ( IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( ch ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   act( AT_MAGIC, "$n disappears in a blinding flash of light!", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   act( AT_MAGIC, "$n appears in a blinding flash of light!", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return rNONE;
}
ch_ret spell_remove_invis		(int sn, int level, CHAR_DATA *ch, void *vo){						//REMOVE INVISIBILITY
   OBJ_DATA *obj;
   SKILLTYPE *skill = get_skilltype( sn );

   if( target_name[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What should the spell be cast upon?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   obj = get_obj_carry( ch, target_name );

   if( obj )
   {
      separate_obj( obj );

      if( !IS_OBJ_STAT( obj, ITEM_INVIS ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Its not invisible!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rSPELL_FAILED;
      }

      xREMOVE_BIT( obj->extra_flags, ITEM_INVIS );
      act( AT_MAGIC, "$p becomes visible again.", ch, obj, NULL, TO_CHAR );

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }
   else
   {
      CHAR_DATA *victim;

      victim = get_char_room( ch, target_name );

      if( victim )
      {
         if( !can_see( ch, victim ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You don't see %s!", target_name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rSPELL_FAILED;
         }

         if( !IS_AFFECTED( victim, AFF_INVISIBLE ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They are not invisible!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rSPELL_FAILED;
         }

         if( is_safe( ch, victim, TRUE ) )
         {
            failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
         }

         if( IS_SET( victim->immune, RIS_MAGIC ) )
         {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
         }
         if( !IS_NPC( victim ) )
         {
            if( chance( ch, 50 ) && ch->level + 10 < victim->level )
            {
               failed_casting( skill, ch, victim, NULL );
               return rSPELL_FAILED;
            }
            else
               check_illegal_pk( ch, victim );
         }
         else
         {
            if( chance( ch, 50 ) && ch->level + 15 < victim->level )
            {
               failed_casting( skill, ch, victim, NULL );
               return rSPELL_FAILED;
            }
         }

         affect_strip( victim, tes_effect_invisibility );
         affect_strip( victim, gsn_mass_invis );
         xREMOVE_BIT( victim->affected_by, AFF_INVISIBLE );
         successful_casting( skill, ch, victim, NULL );
         return rNONE;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You can't find %s!", target_name); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
}
ch_ret spell_animate_dead		(int sn, int level, CHAR_DATA *ch, void *vo){						//ANIMATE DEAD
   CHAR_DATA *mob;
   OBJ_DATA *corpse;
   OBJ_DATA *corpse_next;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   bool found;
   MOB_INDEX_DATA *pMobIndex;
   AFFECT_DATA af;
   char buf[MAX_STRING_LENGTH];
   SKILLTYPE *skill = get_skilltype( sn );

   found = FALSE;

   for( corpse = ch->in_room->first_content; corpse; corpse = corpse_next )
   {
      corpse_next = corpse->next_content;

      if( corpse->item_type == ITEM_CORPSE_NPC && corpse->cost != -5 )
      {
         found = TRUE;
         break;
      }
   }

   if( !found )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot find a suitable corpse here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( get_mob_index( MOB_VNUM_ANIMATED_CORPSE ) == NULL )
   {
      bug( "%s", "Vnum 5 not found for spell_animate_dead!" );
      return rNONE;
   }

   if( ( pMobIndex = get_mob_index( ( int )abs( corpse->cost ) ) ) == NULL )
   {
      bug( "%s", "Can not find mob for cost of corpse, spell_animate_dead" );
      return rSPELL_FAILED;
   }

   if( pMobIndex == get_mob_index( MOB_VNUM_DEITY ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't animate the corpse of a Deity's Avatar."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( !IS_NPC( ch ) )
   {
      if( IS_VAMPIRE( ch ) )
      {
         if( !IS_IMMORTAL( ch ) && ch->pcdata->condition[COND_BLOODTHIRST] - ( pMobIndex->level / 3 ) < 0 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have enough blood power to reanimate this" " corpse."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rSPELL_FAILED;
         }
         gain_condition( ch, COND_BLOODTHIRST, pMobIndex->level / 3 );
      }
      else if( ch->mana - ( pMobIndex->level * 4 ) < 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have enough mana to reanimate this " "corpse."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return rSPELL_FAILED;
      }
      else
         ch->mana -= ( pMobIndex->level * 4 );
   }



   if( IS_IMMORTAL( ch ) || ( chance( ch, 75 ) && pMobIndex->level - ch->level < 10 ) )
   {
      mob = create_mobile( get_mob_index( MOB_VNUM_ANIMATED_CORPSE ) );
      char_to_room( mob, ch->in_room );
      mob->level = UMIN( ch->level / 2, pMobIndex->level );
      mob->race = pMobIndex->race;  /* should be undead */

      /*
       * Fix so mobs wont have 0 hps and crash mud - Scryn 2/20/96 
       */
      if( !pMobIndex->hitnodice )
         mob->max_hit = pMobIndex->level * 8 + number_range( pMobIndex->level * pMobIndex->level / 4,
                                                             pMobIndex->level * pMobIndex->level );
      else
         mob->max_hit = dice( pMobIndex->hitnodice, pMobIndex->hitsizedice ) + pMobIndex->hitplus;
      mob->max_hit = UMAX( URANGE( mob->max_hit / 4,
                                   ( mob->max_hit * corpse->value[3] ) / 100, ch->level * dice( 20, 10 ) ), 1 );

      mob->hit = mob->max_hit;
      mob->damroll = ch->level / 8;
      mob->hitroll = ch->level / 6;
      mob->alignment = ch->alignment;

      act( AT_MAGIC, "$n makes $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_ROOM );
      act( AT_MAGIC, "You make $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_CHAR );

      snprintf( buf, MAX_STRING_LENGTH, "animated corpse %s", pMobIndex->player_name );
      STRFREE( mob->name );
      mob->name = STRALLOC( buf );

      snprintf( buf, MAX_STRING_LENGTH, "The animated corpse of %s", pMobIndex->short_descr );
      STRFREE( mob->short_descr );
      mob->short_descr = STRALLOC( buf );

      snprintf( buf, MAX_STRING_LENGTH, "An animated corpse of %s struggles with the horror of its undeath.\r\n",
                pMobIndex->short_descr );
      STRFREE( mob->long_descr );
      mob->long_descr = STRALLOC( buf );
      add_follower( mob, ch );
      af.type = sn;
      af.duration = ( int )( ( number_fuzzy( ( level + 1 ) / 4 ) + 1 ) * DUR_CONV );
      af.location = 0;
      af.modifier = 0;
      af.bitvector = meb( AFF_CHARM );
      affect_to_char( mob, &af );

      if( corpse->first_content )
         for( obj = corpse->first_content; obj; obj = obj_next )
         {
            obj_next = obj->next_content;
            obj_from_obj( obj );
            obj_to_room( obj, corpse->in_room );
         }

      separate_obj( corpse );
      extract_obj( corpse );
      return rNONE;
   }
   else
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }
}
ch_ret spell_possess			(int sn, int level, CHAR_DATA *ch, void *vo){						//POSSESS
   CHAR_DATA *victim;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype( sn );
   int ctry;

   if( !ch->desc || ch->desc->original )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are not in your original state."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( ( victim = get_char_room( ch, target_name ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( victim == ch )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't possess yourself!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( !IS_NPC( victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't possess another player!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( victim->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s is already possessed.", victim->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( IS_PACIFIST( victim ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rVICT_IMMUNE;
   }

   if( IS_SET( victim->immune, RIS_MAGIC ) || IS_SET( victim->immune, RIS_CHARM ) )
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   ctry = ris_save( victim, level, RIS_CHARM );

   if( IS_AFFECTED( victim, AFF_POSSESS )
       || IS_AFFECTED( ch, AFF_CHARM )
       || level < victim->level || victim->desc || saves_spell_staff( ctry, victim ) || !chance( ch, 25 ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   af.type = sn;
   af.duration = 20 + ( ch->level - victim->level ) / 2;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb( AFF_POSSESS );
   affect_to_char( victim, &af );

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have possessed %s!", victim->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;

   return rNONE;
}
ch_ret spell_knock				(int sn, int level, CHAR_DATA *ch, void *vo){						//KNOCK
   EXIT_DATA *pexit;
   SKILLTYPE *skill = get_skilltype( sn );

   set_char_color( AT_MAGIC, ch );
   /*
    * shouldn't know why it didn't work, and shouldn't work on pickproof
    * exits.  -Thoric
    */
   if( !( pexit = find_door( ch, target_name, FALSE ) )
       || !IS_SET( pexit->exit_info, EX_CLOSED )
       || !IS_SET( pexit->exit_info, EX_LOCKED ) || IS_SET( pexit->exit_info, EX_PICKPROOF ) )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }
   REMOVE_BIT( pexit->exit_info, EX_LOCKED );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "*Click*"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
   if( pexit->rexit && pexit->rexit->to_room == ch->in_room )
      REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
   check_room_for_traps( ch, TRAP_UNLOCK | trap_door[pexit->vdir] );
   return rNONE;
}
ch_ret spell_dream				(int sn, int level, CHAR_DATA *ch, void *vo){						//DREAM
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];

   target_name = one_argument( target_name, arg );
   set_char_color( AT_MAGIC, ch );
   if( !( victim = get_char_world( ch, arg ) ) || victim->in_room->area != ch->in_room->area )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   if( victim->position != POS_SLEEPING )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't asleep."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }
   if( !target_name )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "What do you want them to dream about?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   set_char_color( AT_TELL, victim );
   ch_printf( victim, "You have dreams about %s telling you '%s'.\r\n", PERS( ch, victim ), target_name );
   successful_casting( get_skilltype( sn ), ch, victim, NULL );
   return rNONE;
}
ch_ret spell_spiral_blast		(int sn, int level, CHAR_DATA *ch, void *vo){						//SPIRAL BLAST
   CHAR_DATA *vch;
   TRV_DATA *lc;
   int dam, hpch;
   bool ch_died;

   ch_died = FALSE;

   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You fail to breathe."); act(AT_MAGIC, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   lc = trvch_create( ch, TR_CHAR_ROOM_FORW );
   for( vch = ch->in_room->first_person; vch; vch = trvch_next( lc ) )
   {
      if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
         continue;

      if( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
      {
         act( AT_MAGIC, "Swirling colours radiate from $n, encompassing $N.", ch, ch, vch, TO_ROOM );
         act( AT_MAGIC, "Swirling colours radiate from you, encompassing $N", ch, ch, vch, TO_CHAR );

         hpch = UMAX( 10, ch->hit );
         dam = number_range( hpch / 14 + 1, hpch / 7 );
         if( saves_breath( level, vch ) )
            dam /= 2;
         if( damage( ch, vch, dam, sn ) == rCHAR_DIED || char_died( ch ) )
            ch_died = TRUE;
      }
   }
   trv_dispose( &lc );
   return ch_died ? rCHAR_DIED : rNONE;
}
ch_ret spell_scorching_surge	(int sn, int level, CHAR_DATA *ch, void *vo){						//SCORCHING SURGE
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   static const short dam_each[] = {
      0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
      60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
      92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
      112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
      132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
      152, 154, 156, 158, 160, 162, 164, 166, 168, 170
   };
   int dam;

   level = UMIN( ( unsigned int )level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
   level = UMAX( 0, level );
   dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   act( AT_MAGIC, "A fiery current lashes through $n's body!", ch, NULL, NULL, TO_ROOM );
   act( AT_MAGIC, "A fiery current lashes through your body!", ch, NULL, NULL, TO_CHAR );
   return damage( ch, victim, ( int )( dam * 1.4 ), sn );
}
ch_ret spell_helical_flow		(int sn, int level, CHAR_DATA *ch, void *vo){						//HELICAL FLOW
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype( sn );

   if( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || !victim->in_room
       || xIS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || xIS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
       || xIS_SET( victim->in_room->room_flags, ROOM_DEATH )
       || xIS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE )
       || xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
       || IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
       || IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
       || victim->level >= level + 15
       || ( CAN_PKILL( victim ) && !IS_NPC( ch ) && !IS_PKILL( ch ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
       || ( IS_NPC( victim ) && saves_spell_staff( level, victim ) )
       || !in_hard_range( ch, victim->in_room->area )
       || ( IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) && IS_PKILL( ch ) ) )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   act( AT_MAGIC, "$n coils into an ascending column of colour," " vanishing into thin air.", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   char_to_room( ch, victim->in_room );
   act( AT_MAGIC, "A coil of colours descends from above, " "revealing $n as it dissipates.", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   return rNONE;
}
bool check_save					(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim){				//CHECK SAVE
   SKILLTYPE *skill = get_skilltype( sn );
   bool saved = FALSE;

   if( SPELL_FLAG( skill, SF_PKSENSITIVE ) && !IS_NPC( ch ) && !IS_NPC( victim ) )
      level /= 2;

   if( skill->saves )
      switch ( skill->saves )
      {
         case SS_POISON_DEATH:
            saved = saves_poison_death( level, victim );
            break;
         case SS_ROD_WANDS:
            saved = saves_wands( level, victim );
            break;
         case SS_PARA_PETRI:
            saved = saves_para_petri( level, victim );
            break;
         case SS_BREATH:
            saved = saves_breath( level, victim );
            break;
         case SS_SPELL_STAFF:
            saved = saves_spell_staff( level, victim );
            break;
      }
   return saved;
}
//======================================================================================================================//
// PROCEDURE SPELLS										  																//
//======================================================================================================================//
ch_ret spell_attack				(int sn, int level, CHAR_DATA *ch, void *vo){						//SPELL ATTACK
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   SKILLTYPE *skill = get_skilltype( sn );
   bool saved = check_save( sn, level, ch, victim );
   int dam;
   ch_ret retcode = rNONE;

   if( saved && SPELL_SAVE( skill ) == SE_NEGATE )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( skill->dice )
      dam = UMAX( 0, dice_parse( ch, level, skill->dice ) );
   else
      dam = dice( 1, level / 2 );
   if( saved )
   {
      //Successful saving throws visibly soften some hostile magic.
      if( SPELL_SAVE( skill ) == SE_3QTRDAM || SPELL_SAVE( skill ) == SE_HALFDAM
          || SPELL_SAVE( skill ) == SE_QUARTERDAM || SPELL_SAVE( skill ) == SE_EIGHTHDAM )
         MAGIC_PARTIAL_SAVE( ch, victim );
      switch ( SPELL_SAVE( skill ) )
      {
         case SE_3QTRDAM:
            dam = ( dam * 3 ) / 4;
            break;
         case SE_HALFDAM:
            dam >>= 1;
            break;
         case SE_QUARTERDAM:
            dam >>= 2;
            break;
         case SE_EIGHTHDAM:
            dam >>= 3;
            break;

         case SE_ABSORB:  /* victim absorbs spell for hp's */
            act( AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, victim, TO_CHAR );
            act( AT_MAGIC, "You absorb $N's $t!", victim, skill->noun_damage, ch, TO_CHAR );
            act( AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, victim, TO_NOTVICT );
            victim->hit = URANGE( 0, victim->hit + dam, victim->max_hit );
            update_pos( victim );
            if( ( dam > 0 && ch->fighting && ch->fighting->who == victim )
                || ( dam > 0 && victim->fighting && victim->fighting->who == ch ) )
            {
               int xp = ch->fighting ? ch->fighting->xp : victim->fighting->xp;
               int xp_gain = ( int )( xp * dam * 2 ) / UMAX( 1, victim->max_hit );

               gain_exp( ch, 0 - xp_gain );
            }
            if( skill->first_affect )
               retcode = spell_affectchar( sn, level, ch, victim );
            return retcode;

         case SE_REFLECT: /* One reflection only: never recurse between two reflectors. */
            act( AT_MAGIC, "&C$N turns your spell back upon you!&D", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "&CYou turn $n's spell back upon $m!&D", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "&C$N deflects $n's spell back at $m!&D", ch, NULL, victim, TO_NOTVICT );
            return damage( victim, ch, dam, sn );
      }
   }
   retcode = damage( ch, victim, dam, sn );
   if( retcode == rNONE && skill->first_affect
       && !char_died( ch ) && !char_died( victim )
       && ( !is_affected( victim, sn ) || SPELL_FLAG( skill, SF_ACCUMULATIVE ) || SPELL_FLAG( skill, SF_RECASTABLE ) ) )
      retcode = spell_affectchar( sn, level, ch, victim );
   return retcode;
}
ch_ret spell_area_attack		(int sn, int level, CHAR_DATA *ch, void *vo){						//SPELL AREA ATTACK
   CHAR_DATA *vch, *vch_next;
   SKILLTYPE *skill = get_skilltype( sn );
   bool saved;
   bool affects;
   int dam;
   ch_ret retcode = rNONE;

   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }

   affects = ( skill->first_affect ? TRUE : FALSE );
   if( skill->hit_char && skill->hit_char[0] != '\0' )
      act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );

   for( vch = ch->in_room->first_person; vch; vch = vch_next )
   {
      vch_next = vch->next_in_room;

      if( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
         continue;

      if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
         continue;

      if( vch == ch )
         continue;

      if( is_safe( ch, vch, FALSE ) )
         continue;

      if( !IS_NPC( ch ) && !IS_NPC( vch ) && !in_arena( ch ) && ( !IS_PKILL( ch ) || !IS_PKILL( vch ) ) )
         continue;

      saved = check_save( sn, level, ch, vch );
      if( saved && SPELL_SAVE( skill ) == SE_NEGATE )
      {
         failed_casting( skill, ch, vch, NULL );
         continue;
      }
      else if( skill->dice )
         dam = UMAX( 0, dice_parse( ch, level, skill->dice ) );
      else
         dam = UMAX( 0, dice( 1, level / 2 ) );
      if( saved )
      {
         if( SPELL_SAVE( skill ) == SE_3QTRDAM || SPELL_SAVE( skill ) == SE_HALFDAM
             || SPELL_SAVE( skill ) == SE_QUARTERDAM || SPELL_SAVE( skill ) == SE_EIGHTHDAM )
            MAGIC_PARTIAL_SAVE( ch, vch );
         switch ( SPELL_SAVE( skill ) )
         {
            case SE_3QTRDAM:
               dam = ( dam * 3 ) / 4;
               break;
            case SE_HALFDAM:
               dam >>= 1;
               break;
            case SE_QUARTERDAM:
               dam >>= 2;
               break;
            case SE_EIGHTHDAM:
               dam >>= 3;
               break;

            case SE_ABSORB:  /* victim absorbs spell for hp's */
               act( AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, vch, TO_CHAR );
               act( AT_MAGIC, "You absorb $N's $t!", vch, skill->noun_damage, ch, TO_CHAR );
               act( AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, vch, TO_NOTVICT );
               vch->hit = URANGE( 0, vch->hit + dam, vch->max_hit );
               update_pos( vch );
               if( ( dam > 0 && ch->fighting && ch->fighting->who == vch )
                   || ( dam > 0 && vch->fighting && vch->fighting->who == ch ) )
               {
                  int xp = ch->fighting ? ch->fighting->xp : vch->fighting->xp;
                  int xp_gain = ( int )( xp * dam * 2 ) / UMAX( 1, vch->max_hit );

                  gain_exp( ch, 0 - xp_gain );
               }
               continue;

            case SE_REFLECT: /* Reflect directly, without allowing endless reflection chains. */
               act( AT_MAGIC, "&C$N turns your spell back upon you!&D", ch, NULL, vch, TO_CHAR );
               act( AT_MAGIC, "&CYou turn $n's spell back upon $m!&D", ch, NULL, vch, TO_VICT );
               retcode = damage( vch, ch, dam, sn );
               if( retcode == rVICT_DIED || retcode == rCHAR_DIED || char_died( ch ) )
                  break;
               continue;
         }
      }
      retcode = damage( ch, vch, dam, sn );
      if( retcode == rNONE && affects && !char_died( ch ) && !char_died( vch )
          && ( !is_affected( vch, sn ) || SPELL_FLAG( skill, SF_ACCUMULATIVE ) || SPELL_FLAG( skill, SF_RECASTABLE ) ) )
         retcode = spell_affectchar( sn, level, ch, vch );
      if( retcode == rCHAR_DIED || char_died( ch ) )
      {
         break;
      }
   }
   return retcode;
}
ch_ret spell_affectchar			(int sn, int level, CHAR_DATA *ch, void *vo){						//AFFECT CHARACTER
	AFFECT_DATA af;
	SMAUG_AFF *saf;
	SKILLTYPE *skill = get_skilltype(sn);
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int schance;
	bool affected = FALSE, first = TRUE;
	ch_ret retcode = rNONE;

	if(SPELL_FLAG(skill, SF_RECASTABLE)) {affect_strip(victim, sn);}
	
	for(saf = skill->first_affect; saf; saf = saf->next)
	{
		if(saf->location >= REVERSE_APPLY)
		{
			if(!SPELL_FLAG(skill, SF_ACCUMULATIVE))
			{
				if(first == TRUE)
				{
					if(SPELL_FLAG(skill, SF_RECASTABLE)) {affect_strip(ch, sn);	}
					if(is_affected(ch, sn))				 {affected = TRUE;		}
				}
				first = FALSE;
				if(affected == TRUE) continue;
			}
			victim = ch;
		}
		else victim = (CHAR_DATA *) vo;

		//Check if char has this bitvector already 
		af.bitvector = meb(saf->bitvector);
		if(saf->bitvector >= 0 && xIS_SET(victim->affected_by, saf->bitvector) && !SPELL_FLAG(skill, SF_ACCUMULATIVE))
        continue;

		//Necessary for affect_strip to work properly...
		switch (saf->bitvector)
		{
			default: 			af.type = sn; break;
			
			case AFF_BLIND:		af.type = gsn_blindness; 			break;
			case AFF_CURSE: 	af.type = gsn_curse;				break;
			case AFF_INVISIBLE:	af.type = tes_effect_invisibility;	break;
			case AFF_POSSESS:	af.type = gsn_possess;				break;
			
			case AFF_POISON:	
				af.type = gsn_poison; 
				schance = ris_save( victim, level, RIS_POISON);
				if(schance == 1000)
				{
					retcode = rVICT_IMMUNE;
					if(SPELL_FLAG(skill, SF_STOPONFAIL))
					return retcode;
					continue;
				}
				if(saves_poison_death(schance, victim))
				{
					if(SPELL_FLAG(skill, SF_STOPONFAIL))
					return retcode;
					continue;
				}
				victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
				break;
				
			case AFF_SLEEP:
				af.type = gsn_sleep;
				schance = ris_save(victim, level, RIS_SLEEP);
				if(schance == 1000)
				{
					retcode = rVICT_IMMUNE;
					if(SPELL_FLAG(skill, SF_STOPONFAIL))
					return retcode;
					continue;
				}
				break;

			case AFF_CHARM:
				af.type = gsn_charm_person;
				schance = ris_save(victim, level, RIS_CHARM);
				if(schance == 1000)
				{
					retcode = rVICT_IMMUNE;
					if(SPELL_FLAG(skill, SF_STOPONFAIL))
					return retcode;
					continue;
				}
				break;
		}
		af.duration = dice_parse(ch, level, saf->duration);
		af.modifier = dice_parse(ch, level, saf->modifier);
		af.location = saf->location % REVERSE_APPLY;

		if(af.duration == 0)
		{
			int xp_gain;
			switch (af.location)
			{
				case APPLY_HIT:
					victim->hit = URANGE(0, victim->hit + (!IS_NPC(ch) && IS_NPC(victim) ? af.modifier / 4 : af.modifier), victim->max_hit);
					update_pos( victim );
					
					if((af.modifier > 0 && ch->fighting && ch->fighting->who == victim) || (af.modifier > 0 && victim->fighting && victim->fighting->who == ch))
					{
						int xp = ch->fighting ? ch->fighting->xp : victim->fighting->xp;
						xp_gain = (int)(xp * af.modifier * 2) / UMAX(1, victim->max_hit);
						gain_exp(ch, 0 - xp_gain);
					}
					
					if(IS_NPC(victim) && victim->hit <= 0) {damage(ch, victim, 5, TYPE_UNDEFINED);}
					break;

				case APPLY_MANA:
					victim->mana = URANGE(0, victim->mana + af.modifier, victim->max_mana);
					update_pos( victim );
					break;
					
				case APPLY_MOVE:
					victim->move = URANGE( 0, victim->move + af.modifier, victim->max_move );
					update_pos( victim );
					break;
					
				default: 
					affect_modify( victim, &af, TRUE );
					break;
			}
		}
		else if(SPELL_FLAG(skill, SF_ACCUMULATIVE)) {affect_join(victim, &af);}
		else										{affect_to_char(victim, &af);}
	}
	update_pos(victim);
	return retcode;
}
ch_ret spell_affect				(int sn, int level, CHAR_DATA *ch, void *vo){						//AFFECT
   SMAUG_AFF *saf;
   SKILLTYPE *skill = get_skilltype( sn );
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   bool groupsp;
   bool areasp;
   bool hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
   ch_ret retcode;

   if( !skill->first_affect )
   {
      bug( "spell_affect has no affects sn %d", sn );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_GROUPSPELL ) )
      groupsp = TRUE;
   else
      groupsp = FALSE;

   if( SPELL_FLAG( skill, SF_AREA ) )
      areasp = TRUE;
   else
      areasp = FALSE;

   if( SPELL_FLAG( skill, SF_NOMOB ) && victim && IS_NPC( victim ) )
      return rSPELL_FAILED;

   if( !groupsp && !areasp )
   {
      /*
       * Can't find a victim 
       */
      if( !victim )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      if( SPELL_FLAG( skill, SF_NOMOB ) && IS_NPC( victim ) )
         return rSPELL_FAILED;

      if( ( skill->type != SKILL_HERB
            && IS_SET( victim->immune, RIS_MAGIC ) ) || is_immune( victim, SPELL_DAMAGE( skill ) ) )
      {
         immune_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      /*
       * Spell is already on this guy 
       */
      if( is_affected( victim, sn ) && !SPELL_FLAG( skill, SF_ACCUMULATIVE ) && !SPELL_FLAG( skill, SF_RECASTABLE ) )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      if( ( saf = skill->first_affect ) && !saf->next
          && saf->location == APPLY_STRIPSN && !is_affected( victim, dice_parse( ch, level, saf->modifier ) ) )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }

      if( check_save( sn, level, ch, victim ) )
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
   }
   else
   {
      if( skill->hit_char && skill->hit_char[0] != '\0' )
      {
         if( strstr( skill->hit_char, "$N" ) )
            hitchar = TRUE;
         else
            act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
      }
      if( skill->hit_room && skill->hit_room[0] != '\0' )
      {
         if( strstr( skill->hit_room, "$N" ) )
            hitroom = TRUE;
         else
            act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
      }
      if( skill->hit_vict && skill->hit_vict[0] != '\0' )
         hitvict = TRUE;
      if( victim )
         victim = victim->in_room->first_person;
      else
         victim = ch->in_room->first_person;
   }

   if( !victim )
   {
      bug( "spell_affect: could not find victim: sn %d", sn );
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }

   for( ; victim; victim = victim->next_in_room )
   {
      if( groupsp || areasp )
      {
         if( ( groupsp && !is_same_group( victim, ch ) )
             || IS_SET( victim->immune, RIS_MAGIC )
             || is_immune( victim, SPELL_DAMAGE( skill ) )
             || check_save( sn, level, ch, victim )
             || ( SPELL_FLAG( skill, SF_NOMOB ) && IS_NPC( victim ) )
             || ( !SPELL_FLAG( skill, SF_RECASTABLE ) && is_affected( victim, sn ) ) )
            continue;

         if( hitvict && ch != victim )
         {
            act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
            if( hitroom )
            {
               act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
               act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR );
            }
         }
         else if( hitroom )
            act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
         if( ch == victim )
         {
            if( hitvict )
               act( AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR );
            else if( hitchar )
               act( AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR );
         }
         else if( hitchar )
            act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
      }
      retcode = spell_affectchar( sn, level, ch, victim );
      if( !groupsp && !areasp )
      {
         if( retcode == rVICT_IMMUNE )
            immune_casting( skill, ch, victim, NULL );
         else
            successful_casting( skill, ch, victim, NULL );
         break;
      }
   }
   return rNONE;
}
ch_ret spell_obj_inv			(int sn, int level, CHAR_DATA *ch, void *vo){						//OBJECT IN INVENTORY
   OBJ_DATA *obj = ( OBJ_DATA * ) vo;
   SKILLTYPE *skill = get_skilltype( sn );
   struct WeatherCell *cell = getWeatherCell( ch->in_room->area );

   if( !obj )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }

   switch ( SPELL_ACTION( skill ) )
   {
      default:
      case SA_NONE:
         return rNONE;

      case SA_CREATE:
         if( SPELL_FLAG( skill, SF_WATER ) ) /* create water */
         {
            LIQ_TABLE *liq = get_liq( "water" );
            int water;

            if( obj->item_type != ITEM_DRINK_CON )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It is unable to hold water."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return rSPELL_FAILED;
            }

            if( ( !liq || liq->vnum != obj->value[2] ) && obj->value[1] != 0 )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It contains some other liquid."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               return rSPELL_FAILED;
            }

            water = UMIN( ( skill->dice ? dice_parse( ch, level, skill->dice ) : level )
                          * ( getPrecip( cell ) >= 0 ? 2 : 1 ), obj->value[0] - obj->value[1] );

            if( water > 0 )
            {
               separate_obj( obj );
               obj->value[2] = liq->vnum;
               obj->value[1] += water;
               if( !is_name( "water", obj->name ) )
               {
                  char buf[MAX_STRING_LENGTH];

                  snprintf( buf, MAX_STRING_LENGTH, "%s water", obj->name );
                  STRFREE( obj->name );
                  obj->name = STRALLOC( buf );
               }
            }
            successful_casting( skill, ch, NULL, obj );
            return rNONE;
         }
         if( SPELL_DAMAGE( skill ) == SD_FIRE ) /* burn object */
         {
            /*
             * return rNONE; 
             */
         }
         if( SPELL_DAMAGE( skill ) == SD_POISON /* poison object */
             || SPELL_CLASS( skill ) == SC_DEATH )
         {
            switch ( obj->item_type )
            {
               default:
                  failed_casting( skill, ch, NULL, obj );
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj( obj );
                  obj->value[3] = 1;
                  successful_casting( skill, ch, NULL, obj );
                  break;
            }
            return rNONE;
         }
         if( SPELL_CLASS( skill ) == SC_LIFE /* purify food/water */
             && ( obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_COOK ) )
         {
            switch ( obj->item_type )
            {
               default:
                  failed_casting( skill, ch, NULL, obj );
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj( obj );
                  obj->value[3] = 0;
                  successful_casting( skill, ch, NULL, obj );
                  break;
            }
            return rNONE;
         }

         if( SPELL_CLASS( skill ) != SC_NONE )
         {
            failed_casting( skill, ch, NULL, obj );
            return rNONE;
         }
         switch ( SPELL_POWER( skill ) )  /* clone object */
         {
               OBJ_DATA *clone;

            default:
            case SP_NONE:
               if( ch->level - obj->level < 10 || obj->cost > ch->level * get_curr_int( ch ) * get_curr_wis( ch ) )
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;

            case SP_MINOR:
               if( ch->level - obj->level < 20 || obj->cost > ch->level * get_curr_int( ch ) / 5 )
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;

            case SP_GREATER:
               if( ch->level - obj->level < 5 || obj->cost > ch->level * 10 * get_curr_int( ch ) * get_curr_wis( ch ) )
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;

            case SP_MAJOR:
               if( ch->level - obj->level < 0 || obj->cost > ch->level * 50 * get_curr_int( ch ) * get_curr_wis( ch ) )
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               clone = clone_object( obj );
               clone->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;
               obj_to_char( clone, ch );
               successful_casting( skill, ch, NULL, obj );
               break;
         }
         return rNONE;

      case SA_DESTROY:
      case SA_RESIST:
      case SA_SUSCEPT:
      case SA_DIVINATE:
         if( SPELL_DAMAGE( skill ) == SD_POISON )  /* detect poison */
         {
            if( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD || obj->item_type == ITEM_COOK )
            {
               if( obj->item_type == ITEM_COOK && obj->value[2] == 0 )
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It looks undercooked."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               else if( obj->value[3] != 0 )
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You smell poisonous fumes."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
               else
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It looks very delicious."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            }
            else
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It doesn't look poisoned."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rNONE;
         }
         return rNONE;
      case SA_OBSCURE: /* make obj invis */
         if( IS_OBJ_STAT( obj, ITEM_INVIS ) || chance( ch, skill->dice ? dice_parse( ch, level, skill->dice ) : 20 ) )
         {
            failed_casting( skill, ch, NULL, NULL );
            return rSPELL_FAILED;
         }
         successful_casting( skill, ch, NULL, obj );
         xSET_BIT( obj->extra_flags, ITEM_INVIS );
         return rNONE;

      case SA_CHANGE:
         return rNONE;
   }
}
ch_ret spell_create_obj			(int sn, int level, CHAR_DATA *ch, void *vo){						//CREATE OBJECT
   SKILLTYPE *skill = get_skilltype( sn );
   int lvl;
   int vnum = skill->value;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *oi;

   switch ( SPELL_POWER( skill ) )
   {
      default:
      case SP_NONE:
         lvl = 10;
         break;
      case SP_MINOR:
         lvl = 0;
         break;
      case SP_GREATER:
         lvl = level / 2;
         break;
      case SP_MAJOR:
         lvl = level;
         break;
   }

   /*
    * Add predetermined objects here
    */
   if( vnum == 0 )
   {
      if( !str_cmp( target_name, "sword" ) )
         vnum = OBJ_VNUM_SCHOOL_SWORD;
      if( !str_cmp( target_name, "shield" ) )
         vnum = OBJ_VNUM_SCHOOL_SHIELD;
   }

   if( ( oi = get_obj_index( vnum ) ) == NULL || ( obj = create_object( oi, lvl ) ) == NULL )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }
   obj->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;
   successful_casting( skill, ch, NULL, obj );
   if( CAN_WEAR( obj, ITEM_TAKE ) )
      obj_to_char( obj, ch );
   else
      obj_to_room( obj, ch->in_room );
   return rNONE;
}
ch_ret spell_create_mob			(int sn, int level, CHAR_DATA *ch, void *vo){						//CREATE MOB
   SKILLTYPE *skill = get_skilltype( sn );
   int lvl;
   int vnum = skill->value;
   CHAR_DATA *mob;
   MOB_INDEX_DATA *mi;
   AFFECT_DATA af;

   /*
    * set maximum mob level 
    */
   switch ( SPELL_POWER( skill ) )
   {
      default:
      case SP_NONE:
         lvl = 20;
         break;
      case SP_MINOR:
         lvl = 5;
         break;
      case SP_GREATER:
         lvl = level / 2;
         break;
      case SP_MAJOR:
         lvl = level;
         break;
   }

   /*
    * Add predetermined mobiles here
    */
   if( vnum == 0 )
   {
      if( !str_cmp( target_name, "cityguard" ) )
         vnum = MOB_VNUM_CITYGUARD;
      if( !str_cmp( target_name, "vampire" ) )
         vnum = MOB_VNUM_VAMPIRE;
   }

   if( ( mi = get_mob_index( vnum ) ) == NULL || ( mob = create_mobile( mi ) ) == NULL )
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }
   mob->level = UMIN( lvl, skill->dice ? dice_parse( ch, level, skill->dice ) : mob->level );
   mob->armor = interpolate( mob->level, 100, -100 );

   mob->max_hit = mob->level * 8 + number_range( mob->level * mob->level / 4, mob->level * mob->level );
   mob->hit = mob->max_hit;
   mob->gold = 0;
   successful_casting( skill, ch, mob, NULL );
   char_to_room( mob, ch->in_room );
   add_follower( mob, ch );
   af.type = sn;
   af.duration = ( int )( ( number_fuzzy( ( level + 1 ) / 3 ) + 1 ) * DUR_CONV );
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb( AFF_CHARM );
   affect_to_char( mob, &af );
   return rNONE;
}
ch_ret ranged_attack			(CHAR_DATA *, const char *, OBJ_DATA *, OBJ_DATA *, short, short);	//RANGED ATTACK
ch_ret spell_smaug				(int sn, int level, CHAR_DATA *ch, void *vo){						//SPELL SPAUG
   CHAR_DATA *victim;
   struct skill_type *skill = get_skilltype( sn );

   /*
    * Put this check in to prevent crashes from this getting a bad skill 
    */

   if( !skill )
   {
      bug( "spell_smaug: Called with a null skill for sn %d", sn );
      return rERROR;
   }

   switch ( skill->target )
   {
      case TAR_IGNORE:

         /*
          * offensive area spell 
          */
         if( SPELL_FLAG( skill, SF_AREA )
             && ( ( SPELL_ACTION( skill ) == SA_DESTROY
                    && SPELL_CLASS( skill ) == SC_LIFE )
                  || ( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) ) )
            return spell_area_attack( sn, level, ch, vo );

         if( SPELL_ACTION( skill ) == SA_CREATE )
         {
            if( SPELL_FLAG( skill, SF_OBJECT ) )   /* create object */
               return spell_create_obj( sn, level, ch, vo );
            if( SPELL_CLASS( skill ) == SC_LIFE )  /* create mob */
               return spell_create_mob( sn, level, ch, vo );
         }

         /*
          * affect a distant player 
          */
         if( SPELL_FLAG( skill, SF_DISTANT )
             && ( victim = get_char_world( ch, target_name ) )
             && !xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL ) && SPELL_FLAG( skill, SF_CHARACTER ) )
            return spell_affect( sn, level, ch, get_char_world( ch, target_name ) );

         /*
          * affect a player in this room (should have been TAR_CHAR_XXX) 
          */
         if( SPELL_FLAG( skill, SF_CHARACTER ) )
            return spell_affect( sn, level, ch, get_char_room( ch, target_name ) );

         if( skill->range > 0 && ( ( SPELL_ACTION( skill ) == SA_DESTROY
                                     && SPELL_CLASS( skill ) == SC_LIFE )
                                   || ( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) ) )
            return ranged_attack( ch, ranged_target_name, NULL, NULL, sn, skill->range );
         /*
          * will fail, or be an area/group affect 
          */
         return spell_affect( sn, level, ch, vo );

      case TAR_CHAR_OFFENSIVE:

         /*
          * a regular damage inflicting spell attack 
          */
         if( ( SPELL_ACTION( skill ) == SA_DESTROY
               && SPELL_CLASS( skill ) == SC_LIFE )
             || ( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) )
            return spell_attack( sn, level, ch, vo );

         /*
          * a nasty spell affect 
          */
         return spell_affect( sn, level, ch, vo );

      case TAR_CHAR_DEFENSIVE:
      case TAR_CHAR_SELF:
         if( SPELL_FLAG( skill, SF_NOFIGHT ) &&
             ( ch->position == POS_FIGHTING
               || ch->position == POS_EVASIVE
               || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't concentrate enough for that!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return rNONE;
         }

         if( vo && SPELL_ACTION( skill ) == SA_DESTROY )
         {
            victim = ( CHAR_DATA * ) vo;

            /*
             * cure poison 
             */
            if( SPELL_DAMAGE( skill ) == SD_POISON )
            {
               if( is_affected( victim, gsn_poison ) )
               {
                  affect_strip( victim, gsn_poison );
                  victim->mental_state = URANGE( -100, victim->mental_state, -10 );
                  successful_casting( skill, ch, victim, NULL );
                  return rNONE;
               }
               failed_casting( skill, ch, victim, NULL );
               return rSPELL_FAILED;
            }
            /*
             * cure blindness 
             */
            if( SPELL_CLASS( skill ) == SC_ILLUSION )
            {
               if( is_affected( victim, gsn_blindness ) )
               {
                  affect_strip( victim, gsn_blindness );
                  successful_casting( skill, ch, victim, NULL );
                  return rNONE;
               }
               failed_casting( skill, ch, victim, NULL );
               return rSPELL_FAILED;
            }
         }
         return spell_affect( sn, level, ch, vo );

      case TAR_OBJ_INV:
         return spell_obj_inv( sn, level, ch, vo );
   }
   return rNONE;
}
//======================================================================================================================//
// SMAUG SPELLS										  																	//
//======================================================================================================================//
ch_ret spell_ethereal_fist		(int sn, int level, CHAR_DATA *ch, void *vo){						//ETHEREAL FIST
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 35, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 6 ) ) - 31 );
   dam = UMAX( 0, dam );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;

   return damage( ch, victim, dam, sn );
}
ch_ret spell_spectral_furor		(int sn, int level, CHAR_DATA *ch, void *vo){						//SPECTRAL FUROR
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 16, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 7 ) + 7 ) );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_hand_of_chaos		(int sn, int level, CHAR_DATA *ch, void *vo){						//HAND OF CHAOS
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 18, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 7 ) + 9 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_disruption			(int sn, int level, CHAR_DATA *ch, void *vo){						//DISRUPPION
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 14, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 6 ) + 8 ) );

   if( saves_spell_staff( level, victim ) )
      dam = 0;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_sonic_resonance	(int sn, int level, CHAR_DATA *ch, void *vo){						//SONIC RESONANCE
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 23, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 8 ) ) );

   if( saves_spell_staff( level, victim ) )
      dam = dam * 3 / 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_mind_wrack			(int sn, int level, CHAR_DATA *ch, void *vo){						//MIND WRACK
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   /*
    * decrement mentalstate by up to 50 
    */

   level = UMAX( 0, level );
   dam = number_range( 0, 0 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_mind_wrench		(int sn, int level, CHAR_DATA *ch, void *vo){						//WRENCH
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   /*
    * increment mentalstate by up to 50 
    */

   level = UMAX( 0, level );
   dam = number_range( 0, 0 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_revive				(int sn, int level, CHAR_DATA *ch, void *vo){						//REVIVE
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   /*
    * set mentalstate to mentalstate/2 
    */
   level = UMAX( 0, level );
   dam = number_range( 0, 0 );
   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_sulfurous_spray	(int sn, int level, CHAR_DATA *ch, void *vo){						//SULFUROUS SPRAY
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 19, level );
   dam = ( int )( 1.3 * ( 2 * level * number_range( 1, 7 ) + 11 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_caustic_fount		(int sn, int level, CHAR_DATA *ch, void *vo){						//CAUSTIC FOUNT
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 42, level );
   dam = ( int )( 1.3 * ( 2 * level * number_range( 1, 6 ) ) - 31 );
   dam = UMAX( 0, dam );

   if( saves_spell_staff( level, victim ) )
      dam = dam * 1 / 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_acetum_primus		(int sn, int level, CHAR_DATA *ch, void *vo){						//ACETUM PRIMUS
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   dam = ( int )( 1.3 * ( 2 * level * number_range( 1, 4 ) + 7 ) );

   if( saves_spell_staff( level, victim ) )
      dam = ( int )( 3 * dam / 4 );
   return damage( ch, victim, dam, sn );
}
ch_ret spell_galvanic_whip		(int sn, int level, CHAR_DATA *ch, void *vo){						//GALVANIC WHIP
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 10, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 6 ) + 5 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_magnetic_thrust	(int sn, int level, CHAR_DATA *ch, void *vo){						//MAGNETIC THRUST
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 29, level );
   dam = ( int )( .65 * ( ( 5 * level * number_range( 1, 6 ) ) + 16 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 3;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_quantum_spike		(int sn, int level, CHAR_DATA *ch, void *vo){						//QUANTUM SPIKE
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam, l;

   level = UMAX( 0, level );
   l = 2 * ( level / 10 );
   dam = ( int )( 1.3 * ( l * number_range( 1, 40 ) + 145 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 2;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_black_hand			(int sn, int level, CHAR_DATA *ch, void *vo){						//BLACK HAND
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 5, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 6 ) + 3 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_black_fist			(int sn, int level, CHAR_DATA *ch, void *vo){						//BLACK FIST
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam;

   level = UMAX( 0, level );
   level = UMIN( 30, level );
   dam = ( int )( 1.3 * ( level * number_range( 1, 9 ) + 4 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_black_lightning	(int sn, int level, CHAR_DATA *ch, void *vo){						//BLACK LIGHTNING
   CHAR_DATA *victim = ( CHAR_DATA * ) vo;
   int dam, l;

   level = UMAX( 0, level );
   l = 2 * level / 10;
   dam = ( int )( 1.3 * ( l * number_range( 1, 50 ) + 135 ) );

   if( saves_spell_staff( level, victim ) )
      dam /= 4;
   return damage( ch, victim, dam, sn );
}
ch_ret spell_midas_touch		(int sn, int level, CHAR_DATA *ch, void *vo){						//MIDAS TOUCH
   int val;
   OBJ_DATA *obj = ( OBJ_DATA * ) vo;

   if( IS_OBJ_STAT( obj, ITEM_NODROP ) || IS_OBJ_STAT( obj, ITEM_PERMANENT ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't seem to let go of it."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;
   }

   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && get_trust( ch ) < LEVEL_IMMORTAL )  /* was victim instead of ch!  Thanks Nick Gammon */
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That item is not for mortal hands to touch!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rSPELL_FAILED;   /* Thoric */
   }

   if( !CAN_WEAR( obj, ITEM_TAKE ) || ( obj->item_type == ITEM_CORPSE_NPC ) || ( obj->item_type == ITEM_CORPSE_PC ) || ( obj->item_type == ITEM_KEY ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot seem to turn this item to gold!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return rNONE;
   }

   separate_obj( obj ); /* nice, alty :) */

   val = obj->cost / 2;
   val = UMAX( 0, val );

   ch->gold += val;

   if( obj_extracted( obj ) )
      return rNONE;
   if( cur_obj == obj->serial )
      global_objcode = rOBJ_SACCED;

   extract_obj( obj );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You transmogrify the item to gold!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

   return rNONE;
}


