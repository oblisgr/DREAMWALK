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
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mud.h"

extern int top_affect;
void do_mpmset		(CHAR_DATA *ch, const char *argument){
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char log_buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int value, v2;
	int minattr, maxattr;

	/*
    * A desc means switched.. too many loopholes if we allow that.. 
    */
	if( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	smash_tilde( argument );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
  
	//==============================================================================//
	// VITALS									                			    	//
	//==============================================================================//
	#define MSET_VITAL(arg_name, field_name) 		\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < 1) total = 1; 				\
			if (total > 999) total = 999; 			\
			victim->field_name = total; 			\
			return; 								\
		}
	
		MSET_VITAL("health"	   		, char_health		)
		MSET_VITAL("maximum health"	, char_health_max	)
		MSET_VITAL("fatigue"    	, char_fatigue		)
		MSET_VITAL("maximum fatigue", char_fatigue_max	)
		MSET_VITAL("magicka"		, char_magicka		)
		MSET_VITAL("maximum magicka", char_magicka_max	)

	#undef MSET_PATTRIBUTE
  
	//==============================================================================//
	// NPC ATTRIBUTES									                			//
	//==============================================================================//
	#define MSET_NATTRIBUTE(arg_name, field_name) 	\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < 1) total = 1; 				\
			if (total > 99) total = 99; 			\
			victim->field_name = total; 			\
			return; 								\
		}

		MSET_NATTRIBUTE("strength"	  , bonus_strength		)
		MSET_NATTRIBUTE("endurance"	  , bonus_endurance		)
		MSET_NATTRIBUTE("agility"	  , bonus_agility		)
		MSET_NATTRIBUTE("speed"		  , bonus_speed			)
		MSET_NATTRIBUTE("intelligence", bonus_intelligence	)
		MSET_NATTRIBUTE("willpower"   , bonus_willpower		)
		MSET_NATTRIBUTE("personality" , bonus_personality	)
		MSET_NATTRIBUTE("luck"        , bonus_luck			)
		MSET_NATTRIBUTE("perception"  , bonus_perception	)
		MSET_NATTRIBUTE("wisdom"      , bonus_wisdom		)
		
	#undef MSET_NATTRIBUTE
   
	//==============================================================================//
	// VILLAGE ATTRIBUTES									                		//
	//==============================================================================//
	#define MSET_VATTRIBUTE(arg_name, field_name) 	\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < 1)  total = 1; 				\
			if (total > 99) total = 99; 			\
			victim->field_name = total; 			\
			return; 								\
		}

		MSET_VATTRIBUTE("military"  , bonus_military	)
		MSET_VATTRIBUTE("hygiene"   , bonus_hygiene		)
		MSET_VATTRIBUTE("trade"     , bonus_trade		)
		MSET_VATTRIBUTE("production", bonus_production	)
		MSET_VATTRIBUTE("science"   , bonus_science		)
		MSET_VATTRIBUTE("religion"  , bonus_religion	)
		MSET_VATTRIBUTE("culture"   , bonus_culture		)
		MSET_VATTRIBUTE("wealth"    , bonus_wealth		)
		MSET_VATTRIBUTE("happiness" , bonus_happiness	)
		MSET_VATTRIBUTE("growth"    , bonus_growth		)		

	#undef MSET_VATTRIBUTE 
   
	//==============================================================================//
	// BATTLE ATTRIBUTES									                		//
	//==============================================================================//
	#define MSET_BATTRIBUTE(arg_name, field_name) 	\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < 1)  total = 1; 				\
			if (total > 99) total = 99; 			\
			victim->field_name = total; 			\
			return; 								\
		}

		MSET_BATTRIBUTE("combat attack"  , char_combat_attack	)
		MSET_BATTRIBUTE("combat defence" , char_combat_defence	)
		MSET_BATTRIBUTE("combat damage"  , char_combat_damage	)
		MSET_BATTRIBUTE("stealth attack" , char_stealth_attack	)
		MSET_BATTRIBUTE("stealth defence", char_stealth_defence	)
		MSET_BATTRIBUTE("stealth damage" , char_stealth_damage	)
		MSET_BATTRIBUTE("magic attack"   , char_magic_attack	)
		MSET_BATTRIBUTE("magic defence"  , char_magic_defence	)
		MSET_BATTRIBUTE("magic damage"   , char_magic_damage	)

	#undef MSET_BATTRIBUTE

	//==============================================================================//
	// SKILLS									                			        //
	//==============================================================================//
	#define MSET_SKILL(arg_name, field_name) 		\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < 1)  total = 1;				\
			if (total > 99) total = 99; 			\
			victim->field_name = total; 			\
			return; 								\
		}

		MSET_SKILL("smithing"		, bonus_smithing		) MSET_SKILL("axeweapon"	, bonus_axeweapon		) 
		MSET_SKILL("bluntweapon"	, bonus_bluntweapon		) MSET_SKILL("longblade"	, bonus_longblade		) 
		MSET_SKILL("acrobatics"		, bonus_acrobatics		) MSET_SKILL("farming"		, bonus_farming			) 
		MSET_SKILL("heavyarmor"		, bonus_heavyarmor	   	) MSET_SKILL("mediumarmor"	, bonus_mediumarmor		) 
		MSET_SKILL("mining"			, bonus_mining			) MSET_SKILL("polearm"		, bonus_polearm			) 
		MSET_SKILL("leatherworking"	, bonus_leatherworking 	) MSET_SKILL("logging"		, bonus_logging			) 
		MSET_SKILL("blocking"		, bonus_blocking		) MSET_SKILL("lightarmor"	, bonus_lightarmor		) 
		MSET_SKILL("marksman"		, bonus_marksman		) MSET_SKILL("sneaking"		, bonus_sneaking		) 
		MSET_SKILL("woodworking"	, bonus_woodworking		) MSET_SKILL("hunting"		, bonus_hunting			) 
		MSET_SKILL("athletics"		, bonus_athletics		) MSET_SKILL("handtohand"	, bonus_handtohand		) 
		MSET_SKILL("shortblade"		, bonus_shortblade		) MSET_SKILL("unarmored"	, bonus_unarmored		) 
		MSET_SKILL("throwing"		, bonus_throwing		) MSET_SKILL("dualweapon"	, bonus_dualweapon		) 
		MSET_SKILL("securiry"		, bonus_security		) MSET_SKILL("alchemy"		, bonus_alchemy			) 
		MSET_SKILL("conjuration"	, bonus_conjuration		) MSET_SKILL("enchanting"	, bonus_enchanting		) 
		MSET_SKILL("inscription"	, bonus_inscription		) MSET_SKILL("hearbalism"	, bonus_herbalism		) 
		MSET_SKILL("alteration"		, bonus_alteration		) MSET_SKILL("destruction"	, bonus_destruction		) 
		MSET_SKILL("mysticism"		, bonus_mysticism		) MSET_SKILL("restoration"	, bonus_restoration		) 
		MSET_SKILL("weaving"		, bonus_weaving			) MSET_SKILL("smuggling"	, bonus_smuggling		) 
		MSET_SKILL("mercantile"		, bonus_mercantile		) MSET_SKILL("speechcraft"	, bonus_speechcraft		) 
		MSET_SKILL("illusion"		, bonus_illusion		) MSET_SKILL("jewelcrafting", bonus_jewelcrafting	) 
		MSET_SKILL("provisioning"	, bonus_provisioning	) MSET_SKILL("tailoring"	, bonus_tailoring		)

	#undef MSET_SKILL
	
	//==============================================================================//
	// RESISTANCE								                			        //
	//==============================================================================//
	
	#define MSET_RESISTANCE(arg_name, field_name) 	\
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value; \
			if (total < -99) total = -99;           \
			if (total > 99)  total = 99;  			\
			victim->field_name = total; 			\
			return; 								\
		}
	
		MSET_RESISTANCE("fire resistance"	  , bonus_resistance_fire_percentage		)
		MSET_RESISTANCE("frost resistance"	  , bonus_resistance_frost_percentage		)
		MSET_RESISTANCE("shock resistance"	  , bonus_resistance_shock_percentage		)
		MSET_RESISTANCE("poison resistance"	  , bonus_resistance_poison_percentage		)
		MSET_RESISTANCE("magic resistance"	  , bonus_resistance_magic_percentage		)
		MSET_RESISTANCE("disease resistance"  , bonus_resistance_disease_percentage		)
		MSET_RESISTANCE("blight resistance"	  , bonus_resistance_blight_percentage		)
		MSET_RESISTANCE("corprus resistance"  , bonus_resistance_corprus_percentage		)
		MSET_RESISTANCE("paralysis resistance", bonus_resistance_paralysis_percentage	)
		MSET_RESISTANCE("weapon resistance"   , bonus_resistance_weapon_percentage		)

	#undef MSET_FACTION
	
	//==============================================================================//
	// FACTION REPUTATION								                			//
	//==============================================================================//
	#define MSET_FACTION_REPUTATION(arg_name, field_name) 	\
		if (!str_cmp(arg2, arg_name)) 						\
		{ 													\
			int total = ch->field_name + value;				\
			if (total < -999) total = -999; 				\
			if (total > 999)  total = 999; 					\
			ch->field_name = total; 						\
			return; 										\
		}
	
		MSET_FACTION_REPUTATION("Faction_Bonus_FGD"	, bonus_faction_FGD)
		MSET_FACTION_REPUTATION("Faction_Bonus_TGD"	, bonus_faction_TGD)
		MSET_FACTION_REPUTATION("Faction_Bonus_MGD"	, bonus_faction_MGD)
		MSET_FACTION_REPUTATION("Faction_Bonus_DBD"	, bonus_faction_DBD)
		MSET_FACTION_REPUTATION("Faction_Bonus_IML"	, bonus_faction_IML)
		MSET_FACTION_REPUTATION("Faction_Bonus_IMC"	, bonus_faction_IMC)
		MSET_FACTION_REPUTATION("Faction_Bonus_BLD"	, bonus_faction_BLD)
		MSET_FACTION_REPUTATION("Faction_Bonus_EEC"	, bonus_faction_EEC)
		MSET_FACTION_REPUTATION("Faction_Bonus_TRT"	, bonus_faction_TRT)
		MSET_FACTION_REPUTATION("Faction_Bonus_MTG"	, bonus_faction_MTG)
		MSET_FACTION_REPUTATION("Faction_Bonus_CTG"	, bonus_faction_CTG)
		MSET_FACTION_REPUTATION("Faction_Bonus_NVC"	, bonus_faction_NVC)
		MSET_FACTION_REPUTATION("Faction_Bonus_TNL"	, bonus_faction_TNL)
		MSET_FACTION_REPUTATION("Faction_Bonus_RDN"	, bonus_faction_RDN)
		MSET_FACTION_REPUTATION("Faction_Bonus_HLU"	, bonus_faction_HLU)
		MSET_FACTION_REPUTATION("Faction_Bonus_TLV"	, bonus_faction_TLV)
		MSET_FACTION_REPUTATION("Faction_Bonus_DRS"	, bonus_faction_DRS)
		MSET_FACTION_REPUTATION("Faction_Bonus_IND"	, bonus_faction_IND)
		MSET_FACTION_REPUTATION("Faction_Bonus_SIX"	, bonus_faction_SIX)
		MSET_FACTION_REPUTATION("Faction_Bonus_CAE"	, bonus_faction_CAE)
		MSET_FACTION_REPUTATION("Faction_Bonus_ASH"	, bonus_faction_ASH)
		MSET_FACTION_REPUTATION("Faction_Bonus_FCN"	, bonus_faction_FCN)

		MSET_FACTION_REPUTATION("Faction_Bonus_NBL"	, bonus_faction_NBL)
		MSET_FACTION_REPUTATION("Faction_Bonus_CMN"	, bonus_faction_CMN)
		MSET_FACTION_REPUTATION("Faction_Bonus_SLV"	, bonus_faction_SLV)
		
	#undef MSET_FACTION
	
	//==============================================================================//
	// FACTION RANK								                					//
	//==============================================================================//
	#define MSET_FACTION_RANK(arg_name, field_name) \
		if (!str_cmp(arg2, arg_name)) 				\
		{ 											\
			int total = victim->field_name + value;	\
			if (total < 1)   total = 1; 		    \
			if (total > 13)  total = 13; 			\
			victim->field_name = total; 			\
			return; 								\
		}
	
		MSET_FACTION_RANK("fighters guild rank"			, char_faction_FGD_rank)
		MSET_FACTION_RANK("thieves guild rank"			, char_faction_TGD_rank)
		MSET_FACTION_RANK("mages guild rank"			, char_faction_MGD_rank)
		MSET_FACTION_RANK("dark brotherhood rank"		, char_faction_DBD_rank)
		MSET_FACTION_RANK("imperial legion rank"		, char_faction_IML_rank)
		MSET_FACTION_RANK("imperial cult rank"			, char_faction_IMC_rank)
		MSET_FACTION_RANK("blades rank"					, char_faction_BLD_rank)
		MSET_FACTION_RANK("east empire company rank"	, char_faction_EEC_rank)
		MSET_FACTION_RANK("tribunal temple rank"		, char_faction_TRT_rank)
		MSET_FACTION_RANK("morag tong rank"				, char_faction_MTG_rank)
		MSET_FACTION_RANK("camonna tong rank"			, char_faction_CTG_rank)
		MSET_FACTION_RANK("verevarine cult rank"		, char_faction_NVC_rank)
		MSET_FACTION_RANK("twin lamps rank"				, char_faction_TNL_rank)
		MSET_FACTION_RANK("house redoran rank"			, char_faction_RDN_rank)
		MSET_FACTION_RANK("house hlaalu rank"			, char_faction_HLU_rank)
		MSET_FACTION_RANK("house telvanni rank"			, char_faction_TLV_rank)
		MSET_FACTION_RANK("house dres rank"				, char_faction_DRS_rank)
		MSET_FACTION_RANK("house indoril rank"			, char_faction_IND_rank)
		MSET_FACTION_RANK("sixth house rank"			, char_faction_SIX_rank)
		MSET_FACTION_RANK("census and excise rank"		, char_faction_CAE_rank)
		MSET_FACTION_RANK("ashlanders rank"				, char_faction_ASH_rank)
		MSET_FACTION_RANK("factionless rank"			, char_faction_FCN_rank)

		MSET_FACTION_RANK("nobles rank"		 			, char_faction_NBL_rank)
		MSET_FACTION_RANK("commoners rank"				, char_faction_CMN_rank)
		MSET_FACTION_RANK("slaves rank"		    		, char_faction_SLV_rank)		

	#undef MSET_FACTION
	
   if( !*arg1 )
   {
      progbug( "MpMset: no args", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      progbug( "MpMset: no victim", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
   {
      progbug( "MpMset: victim is proto", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      minattr = 1;
      maxattr = 25;
   }
   else
   {
      minattr = 3;
      maxattr = 18;
   }

   value = is_number( arg3 ) ? atoi( arg3 ) : -1;
   if( atoi( arg3 ) < -1 && value == -1 )
      value = atoi( arg3 );
  
   if( !str_cmp( arg2, "str" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid str", ch );
         return;
      }
      victim->perm_str = value;
      return;
   }

   if( !str_cmp( arg2, "int" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid int", ch );
         return;
      }
      victim->perm_int = value;
      return;
   }

   if( !str_cmp( arg2, "wis" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid wis", ch );
         return;
      }
      victim->perm_wis = value;
      return;
   }

   if( !str_cmp( arg2, "dex" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid dex", ch );
         return;
      }
      victim->perm_dex = value;
      return;
   }

   if( !str_cmp( arg2, "con" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid con", ch );
         return;
      }
      victim->perm_con = value;
      return;
   }

   if( !str_cmp( arg2, "cha" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid cha", ch );
         return;
      }
      victim->perm_cha = value;
      return;
   }

   if( !str_cmp( arg2, "lck" ) )
   {
      if( value < minattr || value > maxattr )
      {
         progbug( "MpMset: Invalid lck", ch );
         return;
      }
      victim->perm_lck = value;
      return;
   }

   if( !str_cmp( arg2, "sav1" ) )
   {
      if( value < -30 || value > 30 )
      {
         progbug( "MpMset: Invalid sav1", ch );
         return;
      }
      victim->saving_poison_death = value;
      return;
   }

   if( !str_cmp( arg2, "sav2" ) )
   {
      if( value < -30 || value > 30 )
      {
         progbug( "MpMset: Invalid sav2", ch );
         return;
      }
      victim->saving_wand = value;
      return;
   }

   if( !str_cmp( arg2, "sav3" ) )
   {
      if( value < -30 || value > 30 )
      {
         progbug( "MpMset: Invalid sav3", ch );
         return;
      }
      victim->saving_para_petri = value;
      return;
   }

   if( !str_cmp( arg2, "sav4" ) )
   {
      if( value < -30 || value > 30 )
      {
         progbug( "MpMset: Invalid sav4", ch );
         return;
      }
      victim->saving_breath = value;
      return;
   }

   if( !str_cmp( arg2, "sav5" ) )
   {
      if( value < -30 || value > 30 )
      {
         progbug( "MpMset: Invalid sav5", ch );
         return;
      }
      victim->saving_spell_staff = value;
      return;
   }

   if( !str_cmp( arg2, "sex" ) )
   {
      if( value < 0 || value > 2 )
      {
         progbug( "MpMset: Invalid sex", ch );
         return;
      }
      victim->sex = value;
      return;
   }

   if( !str_cmp( arg2, "class" ) )
   {
      if( IS_NPC( victim ) )  /* Broken by Haus... fixed by Thoric */
      {
         if( value >= MAX_NPC_CLASS || value < 0 )
         {
            progbug( "MpMset: Invalid npc class", ch );
            return;
         }
         victim->Class = value;
         return;
      }
      progbug( "MpMset: can't set pc class", ch );
   }

   if( !str_cmp( arg2, "race" ) )
   {
      value = get_npc_race( arg3 );
      if( value < 0 )
         value = atoi( arg3 );
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc race", ch );
         return;
      }
      if( value < 0 || value >= MAX_NPC_RACE )
      {
         progbug( "MpMset: Invalid npc race", ch );
         return;
      }
      victim->race = value;
      return;
   }
/***************************************************************************/
   if( !str_cmp( arg2, "armor" ) )
   {
      if( value < -300 || value > 300 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "AC range is -300 to 300."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->armor = value;
      return;
   }

   if( !str_cmp( arg2, "armor_adjust" ) )
   {
      if( value < -299 || value > 299 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "AC range is -300 to 300."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->armor = victim->armor + value;
      return;
   }

   if( victim->armor < -299 )
       victim->armor = -299;

   if( victim->armor > +299 )
       victim->armor = +299;

/***************************************************************************/
   if( !str_cmp( arg2, "level" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc level", ch );
         return;
      }

      if( value < 0 || value > LEVEL_AVATAR + 5 )
      {
         progbug( "MpMset: Invalid npc level", ch );
         return;
      }
      victim->level = value;
      return;
   }

   if( !str_cmp( arg2, "numattacks" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc numattacks", ch );
         return;
      }

      if( value < 0 || value > 20 )
      {
         progbug( "MpMset: Invalid npc numattacks", ch );
         return;
      }
      victim->numattacks = value;
      return;
   }
/***************************************************************************/
   if( !str_cmp( arg2, "gold" ) )
   {
      victim->gold = value;
      return;
   }

   if( !str_cmp( arg2, "gold_adjust" ) )
   {
      victim->gold = victim->gold + value;
      return;
   }
/***************************************************************************/
   if( !str_cmp( arg2, "hitroll" ) )
   {
      victim->hitroll = URANGE( 0, value, 85 );
      return;
   }

   if( !str_cmp( arg2, "damroll" ) )
   {
      victim->damroll = URANGE( 0, value, 65 );
      return;
   }
/***************************************************************************/
   if( !str_cmp( arg2, "hp" ) )
   {
      if( value < 1 || value > 32700 )
      {
         progbug( "MpMset: Invalid hp", ch );
         return;
      }
      victim->max_hit = value;
      return;
   }

   if( !str_cmp( arg2, "hp_adjust" ) )
   {
      if( value < -999 || value > 999 )
      {
         progbug( "MpMset: Invalid hp", ch );
         return;
      }
      victim->hit = victim->hit + value;
      return;
   }

   if( victim->move < 0 )
       victim->move = 0;

   if( victim->move > +32700 )
       victim->move = +32700;

/***************************************************************************/
   if( !str_cmp( arg2, "mana" ) )
   {
      if( value < 0 || value > 30000 )
      {
         progbug( "MpMset: Invalid mana", ch );
         return;
      }
      victim->max_mana = value;
      return;
   }

   if( !str_cmp( arg2, "mana_adjust" ) )
   {
      if( value < -999 || value > 999 )
      {
         progbug( "MpMset: Invalid mana", ch );
         return;
      }
      victim->mana = victim->mana + value;
      return;
   }

   if( victim->move < 0 )
       victim->move = 0;

   if( victim->move > +30000 )
       victim->move = +30000;

/***************************************************************************/
   if( !str_cmp( arg2, "move" ) )
   {
      if( value < 0 || value > 30000 )
      {
         progbug( "MpMset: Invalid move", ch );
         return;
      }
      victim->max_move = value;
      return;
   }

   if( !str_cmp( arg2, "move_adjust" ) )
   {
      if( value < -999 || value > 999 )
      {
         progbug( "MpMset: Invalid move", ch );
         return;
      }
      victim->move = victim->move + value;
      return;
   }

   if( victim->move < 0 )
       victim->move = 0;

   if( victim->move > +30000 )
       victim->move = +30000;

/***************************************************************************/
   if( !str_cmp( arg2, "practice" ) )
   {
      if( value < 0 || value > 100 )
      {
         progbug( "MpMset: Invalid practice", ch );
         return;
      }
      victim->practice = value;
      return;
   }
/***************************************************************************/
   if( !str_cmp( arg2, "align" ) )
   {
      if( value < -999 || value > 999 )
      {
         progbug( "MpMset: Invalid align", ch );
         return;
      }
      victim->alignment = value;
      return;
   }

   if( !str_cmp( arg2, "align_adjust" ) )
   {
      if( value < -999 || value > 999 )
      {
         progbug( "MpMset: Invalid align", ch );
         return;
      }
      victim->alignment = victim->alignment + value;
      return;
   }

   if( victim->alignment < -999 )
       victim->alignment = -999;

   if( victim->alignment > +999 )
       victim->alignment = +999;

/***************************************************************************/
/* non-functional for now -- Blod
    if ( !str_cmp( arg2, "quest" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc quest", ch);
	    return;
	}

	if ( value < 0 || value > 500 )
	{
	    progbug("MpMset: Invalid pc quest", ch);
	    return;
	}

	victim->pcdata->quest = value;
	return;
    }
*/

   if( !str_cmp( arg2, "questplus" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc qp", ch );
         return;
      }

      if( value < 0 || value > 5000 )
      {
         progbug( "MpMset: Invalid pc qp", ch );
         return;
      }
      snprintf( log_buf, MAX_STRING_LENGTH, "%s raising glory of %s by %d ...", ch->name, victim->name, value );
      log_string( log_buf );
      victim->pcdata->quest_curr += value;
      victim->pcdata->quest_accum += value;
      return;
   }

   if( !str_cmp( arg2, "favor" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc favor", ch );
         return;
      }

      if( value < -2500 || value > 2500 )
      {
         progbug( "MpMset: Invalid pc favor", ch );
         return;
      }

      victim->pcdata->favor = value;
      return;
   }

   if( !str_cmp( arg2, "mentalstate" ) )
   {
      if( value < -100 || value > 100 )
      {
         progbug( "MpMset: Invalid mentalstate", ch );
         return;
      }
      victim->mental_state = value;
      return;
   }

   if( !str_cmp( arg2, "emotion" ) )
   {
      if( value < -100 || value > 100 )
      {
         progbug( "MpMset: Invalid emotion", ch );
         return;
      }
      victim->emotional_state = value;
      return;
   }

   if( !str_cmp( arg2, "thirst" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc thirst", ch );
         return;
      }

      if( value < 0 || value > 100 )
      {
         progbug( "MpMset: Invalid pc thirst", ch );
         return;
      }

      victim->pcdata->condition[COND_THIRST] = value;
      return;
   }

   if( !str_cmp( arg2, "drunk" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc drunk", ch );
         return;
      }

      if( value < 0 || value > 100 )
      {
         progbug( "MpMset: Invalid pc drunk", ch );
         return;
      }

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
   }

   if( !str_cmp( arg2, "full" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc full", ch );
         return;
      }

      if( value < 0 || value > 100 )
      {
         progbug( "MpMset: Invalid pc full", ch );
         return;
      }

      victim->pcdata->condition[COND_FULL] = value;
      return;
   }

   if( !str_cmp( arg2, "blood" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc blood", ch );
         return;
      }

      if( value < 0 || value > MAX_LEVEL + 10 )
      {
         progbug( "MpMset: Invalid pc blood", ch );
         return;
      }

      victim->pcdata->condition[COND_BLOODTHIRST] = value;
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc name", ch );
         return;
      }

      STRFREE( victim->name );
      victim->name = STRALLOC( arg3 );
      return;
   }

   if( !str_cmp( arg2, "deity" ) )
   {
      DEITY_DATA *deity;

      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc deity", ch );
         return;
      }

      if( arg3[0] == '\0' )
      {
         STRFREE( victim->pcdata->deity_name );
         victim->pcdata->deity_name = STRALLOC( "" );
         victim->pcdata->deity = NULL;
         return;
      }

      deity = get_deity( arg3 );
      if( !deity )
      {
         progbug( "MpMset: Invalid deity", ch );
         return;
      }
      STRFREE( victim->pcdata->deity_name );
      victim->pcdata->deity_name = QUICKLINK( deity->name );
      victim->pcdata->deity = deity;
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      STRFREE( victim->short_descr );
      victim->short_descr = STRALLOC( arg3 );
      return;
   }

   if( !str_cmp( arg2, "long" ) )
   {
      STRFREE( victim->long_descr );
      mudstrlcpy( buf, arg3, MAX_STRING_LENGTH );
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
      victim->long_descr = STRALLOC( buf );
      return;
   }

   if( !str_cmp( arg2, "title" ) )
   {
      if( IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set npc title", ch );
         return;
      }

      set_title( victim, arg3 );
      return;
   }

   if( !str_cmp( arg2, "spec" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc spec", ch );
         return;
      }

      if( !str_cmp( arg3, "none" ) )
      {
         victim->spec_fun = NULL;
         return;
      }

      if( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
      {
         progbug( "MpMset: Invalid spec", ch );
         return;
      }
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc flags", ch );
         return;
      }

      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no flags", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_actflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            progbug( "MpMset: Invalid flag", ch );
         else
         {
            if( value == ACT_PROTOTYPE )
               progbug( "MpMset: can't set prototype flag", ch );
            else if( value == ACT_IS_NPC )
               progbug( "MpMset: can't remove npc flag", ch );
            else
               xTOGGLE_BIT( victim->act, value );
         }
      }
      return;
   }

   if( !str_cmp( arg2, "affected" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't modify pc affected", ch );
         return;
      }

      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no affected", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            progbug( "MpMset: Invalid affected", ch );
         else
            xTOGGLE_BIT( victim->affected_by, value );
      }
      return;
   }

   /*
    * save some more finger-leather for setting RIS stuff
    * Why there's can_modify checks here AND in the called function, Ill
    * never know, so I removed them.. -- Alty
    */
   if( !str_cmp( arg2, "r" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "i" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ri" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "is" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ris" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mpmset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "resistant" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc resistant", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no resistant", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            progbug( "MpMset: Invalid resistant", ch );
         else
            TOGGLE_BIT( victim->resistant, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg2, "immune" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc immune", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no immune", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            progbug( "MpMset: Invalid immune", ch );
         else
            TOGGLE_BIT( victim->immune, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg2, "susceptible" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc susceptible", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no susceptible", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            progbug( "MpMset: Invalid susceptible", ch );
         else
            TOGGLE_BIT( victim->susceptible, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg2, "part" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc part", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no part", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_partflag( arg3 );
         if( value < 0 || value > 31 )
            progbug( "MpMset: Invalid part", ch );
         else
            TOGGLE_BIT( victim->xflags, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg2, "attack" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc attack", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no attack", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_attackflag( arg3 );
         if( value < 0 )
            progbug( "MpMset: Invalid attack", ch );
         else
            xTOGGLE_BIT( victim->attacks, value );
      }
      return;
   }

   if( !str_cmp( arg2, "defense" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc defense", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no defense", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_defenseflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            progbug( "MpMset: Invalid defense", ch );
         else
            xTOGGLE_BIT( victim->defenses, value );
      }
      return;
   }

   if( !str_cmp( arg2, "pos" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc pos", ch );
         return;
      }
      if( value < 0 || value > POS_STANDING )
      {
         progbug( "MpMset: Invalid pos", ch );
         return;
      }
      victim->position = value;
      return;
   }

   if( !str_cmp( arg2, "defpos" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc defpos", ch );
         return;
      }
      if( value < 0 || value > POS_STANDING )
      {
         progbug( "MpMset: Invalid defpos", ch );
         return;
      }
      victim->defposition = value;
      return;
   }

   if( !str_cmp( arg2, "speaks" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no speaks", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_langflag( arg3 );
         v2 = get_langnum( arg3 );
         if( value == LANG_UNKNOWN )
            progbug( "MpMset: Invalid speaks", ch );
         else if( !IS_NPC( victim ) )
         {
            if( !( value &= VALID_LANGS ) )
            {
               progbug( "MpMset: Invalid player language", ch );
               continue;
            }
            if( v2 == -1 )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               TOGGLE_BIT( victim->speaks, 1 << v2 );
         }
         else
         {
            if( v2 == -1 )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               TOGGLE_BIT( victim->speaks, 1 << v2 );
         }
      }
      if( !IS_NPC( victim ) )
      {
         REMOVE_BIT( victim->speaks, race_table[victim->race]->language );
         if( !knows_language( victim, victim->speaking, victim ) )
            victim->speaking = race_table[victim->race]->language;
      }
      return;
   }

   if( !str_cmp( arg2, "speaking" ) )
   {
      if( !IS_NPC( victim ) )
      {
         progbug( "MpMset: can't set pc speaking", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpMset: no speaking", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_langflag( arg3 );
         if( value == LANG_UNKNOWN )
            progbug( "MpMset: Invalid speaking", ch );
         else
         {
            v2 = get_langnum( arg3 );
            if( v2 == -1 )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               TOGGLE_BIT( victim->speaks, 1 << v2 );
         }
      }
      return;
   }
   
	progbug( "MpMset: Invalid field", ch );
	return;
}
void do_mposet		(CHAR_DATA *ch, const char *argument){
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   char outbuf[MAX_STRING_LENGTH];
   int value, tmp;

   /*
    * A desc means switched.. too many loopholes if we allow that.. 
    */
   if( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Huh?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   smash_tilde( argument );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );

   if( !*arg1 )
   {
      progbug( "MpOset: no args", ch );
      return;
   }

   if( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
   {
      progbug( "MpOset: no object", ch );
      return;
   }

   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      progbug( "MpOset: can't set prototype items", ch );
      return;
   }
   separate_obj( obj );
   value = atoi( arg3 );

   if( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
   {
      obj->value[0] = value;
      return;
   }

   if( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
   {
      obj->value[1] = value;
      return;
   }

   if( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
   {
      obj->value[2] = value;
      return;
   }

   if( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
   {
      obj->value[3] = value;
      return;
   }

   if( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
   {
      obj->value[4] = value;
      return;
   }

   if( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
   {
      obj->value[5] = value;
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpOset: no type", ch );
         return;
      }
      value = get_otype( argument );
      if( value < 1 )
      {
         progbug( "MpOset: Invalid type", ch );
         return;
      }
      obj->item_type = ( short )value;
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpOset: no flags", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_oflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            progbug( "MpOset: Invalid flag", ch );
         else
         {
            if( value == ITEM_PROTOTYPE )
               progbug( "MpOset: can't set prototype flag", ch );
            else
               xTOGGLE_BIT( obj->extra_flags, value );
         }
      }
      return;
   }

   if( !str_cmp( arg2, "wear" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpOset: no wear", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_wflag( arg3 );
         if( value < 0 || value > 31 )
            progbug( "MpOset: Invalid wear", ch );
         else
            TOGGLE_BIT( obj->wear_flags, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      obj->level = value;
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      obj->weight = value;
      return;
   }

   if( !str_cmp( arg2, "cost" ) )
   {
      obj->cost = value;
      return;
   }

   if( !str_cmp( arg2, "timer" ) )
   {
      obj->timer = value;
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( obj->name );
      obj->name = STRALLOC( arg3 );
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( arg3 );

      if( obj == supermob_obj )
      {
         STRFREE( supermob->short_descr );
         supermob->short_descr = QUICKLINK( obj->short_descr );
      }

      /*
       * Feature added by Narn, Apr/96 
       * * If the item is not proto, add the word 'rename' to the keywords
       * * if it is not already there.
       */
      if( str_infix( "mprename", obj->name ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", obj->name, "mprename" );
         STRFREE( obj->name );
         obj->name = STRALLOC( buf );
      }
      return;
   }

   if( !str_cmp( arg2, "long" ) )
   {
      STRFREE( obj->description );
      mudstrlcpy( buf, arg3, MAX_STRING_LENGTH );
      obj->description = STRALLOC( buf );
      return;
   }

   if( !str_cmp( arg2, "actiondesc" ) )
   {
      if( strstr( arg3, "%n" ) || strstr( arg3, "%d" ) || strstr( arg3, "%l" ) )
      {
         progbug( "MpOset: Illegal actiondesc", ch );
         return;
      }
      STRFREE( obj->action_desc );
      obj->action_desc = STRALLOC( arg3 );
      return;
   }

   if( !str_cmp( arg2, "affect" ) )
   {
      AFFECT_DATA *paf;
      short loc;
      int bitv;

      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' || !argument || argument[0] == 0 )
      {
         progbug( "MpOset: Bad affect syntax", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> affect <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      loc = get_atype( arg2 );
      if( loc < 1 )
      {
         progbug( "MpOset: Invalid affect field", ch );
         return;
      }
      if( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
      {
         bitv = 0;
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg3 );
            if( loc == APPLY_AFFECT )
               value = get_aflag( arg3 );
            else
               value = get_risflag( arg3 );
            if( value < 0 || value > 31 )
               progbug( "MpOset: bad affect flag", ch );
            else
               SET_BIT( bitv, 1 << value );
         }
         if( !bitv )
            return;
         value = bitv;
      }
      else
      {
         argument = one_argument( argument, arg3 );
         value = atoi( arg3 );
      }
      CREATE( paf, AFFECT_DATA, 1 );
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS( paf->bitvector );
      paf->next = NULL;
      LINK( paf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
      return;
   }

   if( !str_cmp( arg2, "rmaffect" ) )
   {
      AFFECT_DATA *paf;
      short loc, count;

      if( !argument || argument[0] == '\0' )
      {
         progbug( "MpOset: no rmaffect", ch );
         return;
      }
      loc = atoi( argument );
      if( loc < 1 )
      {
         progbug( "MpOset: Invalid rmaffect", ch );
         return;
      }

      count = 0;

      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( ++count == loc )
         {
            UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            DISPOSE( paf );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            --top_affect;
            return;
         }
      }
      progbug( "MpOset: rmaffect not found", ch );
      return;
   }

   /*
    * save some finger-leather
    */
   if( !str_cmp( arg2, "ris" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "r" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "i" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "ri" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "is" ) )
   {
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_mposet( ch, outbuf );
      return;
   }

   /*
    * Make it easier to set special object values by name than number
    *                  -Thoric
    */
   tmp = -1;
   switch ( obj->item_type )
   {
      case ITEM_WEAPON:
         if( !str_cmp( arg2, "weapontype" ) )
         {
            unsigned int x;

            value = -1;
            for( x = 0; x < sizeof( attack_table ) / sizeof( attack_table[0] ); x++ )
               if( !str_cmp( arg3, attack_table[x] ) )
                  value = x;
            if( value < 0 )
            {
               progbug( "MpOset: Invalid weapon type", ch );
               return;
            }
            tmp = 3;
            break;
         }
         if( !str_cmp( arg2, "condition" ) )
            tmp = 0;
         break;
      case ITEM_ARMOR:
         if( !str_cmp( arg2, "condition" ) )
            tmp = 3;
         if( !str_cmp( arg2, "ac" ) )
            tmp = 1;
         break;
      case ITEM_SALVE:
         if( !str_cmp( arg2, "slevel" ) )
            tmp = 0;
         if( !str_cmp( arg2, "maxdoses" ) )
            tmp = 1;
         if( !str_cmp( arg2, "doses" ) )
            tmp = 2;
         if( !str_cmp( arg2, "delay" ) )
            tmp = 3;
         if( !str_cmp( arg2, "spell1" ) )
            tmp = 4;
         if( !str_cmp( arg2, "spell2" ) )
            tmp = 5;
         if( tmp >= 4 && tmp <= 5 )
            value = skill_lookup( arg3 );
         break;
      case ITEM_SCROLL:
      case ITEM_POTION:
      case ITEM_PILL:
         if( !str_cmp( arg2, "slevel" ) )
            tmp = 0;
         if( !str_cmp( arg2, "spell1" ) )
            tmp = 1;
         if( !str_cmp( arg2, "spell2" ) )
            tmp = 2;
         if( !str_cmp( arg2, "spell3" ) )
            tmp = 3;
         if( tmp >= 1 && tmp <= 3 )
            value = skill_lookup( arg3 );
         break;
      case ITEM_STAFF:
      case ITEM_WAND:
         if( !str_cmp( arg2, "slevel" ) )
            tmp = 0;
         if( !str_cmp( arg2, "spell" ) )
         {
            tmp = 3;
            value = skill_lookup( arg3 );
         }
         if( !str_cmp( arg2, "maxcharges" ) )
            tmp = 1;
         if( !str_cmp( arg2, "charges" ) )
            tmp = 2;
         break;
      case ITEM_CONTAINER:
         if( !str_cmp( arg2, "capacity" ) )
            tmp = 0;
         if( !str_cmp( arg2, "cflags" ) )
            tmp = 1;
         if( !str_cmp( arg2, "key" ) )
            tmp = 2;
         break;
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
         if( !str_cmp( arg2, "tflags" ) )
         {
            tmp = 0;
            value = get_trigflag( arg3 );
         }
         break;
   }
   if( tmp >= 0 && tmp <= 3 )
   {
      obj->value[tmp] = value;
      return;
   }

   progbug( "MpOset: Invalid field", ch );
   return;
}
