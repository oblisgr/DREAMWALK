#ifndef DREAMWALK_H
#define DREAMWALK_H

struct char_data;
typedef struct char_data CHAR_DATA;
//==================================================================================================//
// LOGICAL HELPERS																					//
//==================================================================================================//
bool DO_GENERIC_CHECKS_C							(CHAR_DATA *ch);
bool DO_GENERIC_CHECKS_V							(CHAR_DATA *victim);
bool IS_CREATURE_MOB								(CHAR_DATA *ch);
bool IS_CLASSTYPE_COMBAT_EXPERT						(CHAR_DATA *ch);
bool IS_CLASSTYPE_STEALTH_SPECIALIST				(CHAR_DATA *ch);
bool IS_CLASSTYPE_MAGIC_USER						(CHAR_DATA *ch);
bool IS_CLASSTYPE_SOCIAL_WORKER						(CHAR_DATA *ch);
bool AUTOMATIC_ARMOR_CHECK							(CHAR_DATA *ch, CHAR_DATA *victim);
bool IS_TWO_HANDED_WEAPON							(CHAR_DATA *ch);
//==================================================================================================//
// FUNCTION HELPERS                                                             					//
//==================================================================================================//
void GET_MAPPER_PLAYER_STATS						(CHAR_DATA *ch, const char **EXPcolor, const char **ATKcolor, const char **DEFcolor, const char **DMGcolor, const char **char_repColor, const char **char_reputation, int *char_exp);
void GET_MAPPER_TIME								(char *pMSG, size_t size);
void GET_MAPPER_AREA_REPUTATION						(CHAR_DATA *ch, char *result, size_t size);
//--------------------------------------------------------------------------------------------------//
const char *GET_RELATION							(short score);
const char *LOWER_TEXT								(const char *src);
int ANSI_LENGTH										(const char *str);
void WRAP_TEXT										(const char *src, char *out, int max_width);
void CHAR_SHOW_EQUIPMENT							(CHAR_DATA *ch, CHAR_DATA *victim);
void CHAR_CUSTOM_PROMPT								(CHAR_DATA *ch);
void SHOW_OBJECT_CATEGORY							(CHAR_DATA *ch, char **prgpstrShow, int *prgnShow, int *pitShow, OBJ_DATA **ITEMS_IN_ROOM, int nShow, const char *title, int itemtype);
void OBJECT_RARITY_COLOR							(CHAR_DATA *ch, OBJ_DATA *ITEM);
void CHAR_FACTION_MEMBERSHIP						(CHAR_DATA *ch);
//--------------------------------------------------------------------------------------------------//
void CHAR_RECALCULATE_FULL							(CHAR_DATA *ch);
void CHAR_RECALCULATE_BIOGRAPHICS					(CHAR_DATA *ch);
void CHAR_RECALCULATE_NATTRIBUTES					(CHAR_DATA *ch);
void CHAR_RECALCULATE_VITALS						(CHAR_DATA *ch);
void CHAR_RECALCULATE_SMAUG							(CHAR_DATA *ch);
void CHAR_RECALCULATE_VATTRIBUTES					(CHAR_DATA *ch);
void CHAR_RECALCULATE_CATTRIBUTES					(CHAR_DATA *ch);
void CHAR_RECALCULATE_SKILLS						(CHAR_DATA *ch);
void CHAR_RECALCULATE_RESISTANCES					(CHAR_DATA *ch);
int GET_HIGHEST_FACTION_REF							(CHAR_DATA *ch);
void CHAR_RECALCULATE_FACTIONS						(CHAR_DATA *ch);
void CHAR_RECALCULATE_EQUIPMENT						(CHAR_DATA *ch);
void CHAR_RECALCULATE_BALANCE						(CHAR_DATA *ch);
void CHAR_RECALCULATE_PROMPT						(CHAR_DATA *ch);
void CHAR_RECALCULATE_CHALLENGE_RATING				(CHAR_DATA *ch);
//--------------------------------------------------------------------------------------------------//
int GET_FACTION_DISPOSITION_VALUE					(int faction_ref, int target_ref);
void CHAR_FACTION_KILL_ADJUSTMENT					(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
int GET_PERSONAL_DISPOSITION						(CHAR_DATA *ch, CHAR_DATA *victim);
const char *GET_PERSONAL_DISPOSITION_FEELING		(CHAR_DATA *ch);
void CHAR_RECALCULATE_PERSONAL_DISPOSITION			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
void CHAR_RECALCULATE_SETTLEMENT					(CHAR_DATA *ch);
//--------------------------------------------------------------------------------------------------//
void NPC_EQUIP_ITEM_UNARMORED						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_LIGHTARMOR						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_MEDIUMARMOR						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_HEAVYARMOR						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_APPAREL							(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_DEFAULT							(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_SHIELD							(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_AXEWEAPON						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_BLUNTWEAPON						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_SHORTBLADE						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_LONGBLADE						(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_POLEARM							(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_RANGEDWEAPON					(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_THROWNWEAPON					(CHAR_DATA *ch, CHAR_DATA *npc);
void NPC_EQUIP_ITEM_HANDWEAPON						(CHAR_DATA *ch, CHAR_DATA *npc);
//==================================================================================================//
// DREAMWALK HELPERS                                                            					//
//==================================================================================================//
void CREATURE_SHOW_DESCRIPTION					(CHAR_DATA *ch, CHAR_DATA *cre);
void NPC_EQUIP_ITEM_PROCEDURE					(CHAR_DATA *ch, CHAR_DATA *npc);
void DEBUG_THE_CODE								(CHAR_DATA *ch, CHAR_DATA *npc, CHAR_DATA *cre);
void CLEAN_DREAMWALK_MOBS						(ROOM_INDEX_DATA *room);
int	 GENERATE_RANDOM_AREA						(void);
bool TELEPORT_PLAYER_TO_ROOM					(CHAR_DATA *ch, int room_vnum);
void CALCULATE_CONSIDER_MESSAGE					(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
CHAR_DATA *SPAWN_RANDOM_NPC						(ROOM_INDEX_DATA *room);
CHAR_DATA *SPAWN_RANDOM_CREATURE				(ROOM_INDEX_DATA *room);
void END_DREAMWALK								(CHAR_DATA *ch);
//==================================================================================================//
// DREAMWALK EVENTS                                                          						//
//==================================================================================================//
void DREAMWALK_EVENT100_JUST_NOTHING			(CHAR_DATA *ch);
void DREAMWALK_EVENT200_TALK_IDLE				(CHAR_DATA *ch);
//--------------------------------------------------------------------------------------------------//
void DREAMWALK_EVENT300_NPC_REVEAL				(CHAR_DATA *ch, CHAR_DATA *npc);
void DREAMWALK_EVENT301_NPC_ATTACK				(CHAR_DATA *ch, CHAR_DATA *npc);
void DREAMWALK_EVENT302_NPC_SHOPKEEPER_TRADE	(CHAR_DATA *ch, CHAR_DATA *npc);
void DREAMWALK_EVENT303_NPC_SHOPKEEPER_REPAIR	(CHAR_DATA *ch, CHAR_DATA *npc);
void DREAMWALK_EVENT304_NPC_SHOPKEEPER_TRAIN	(CHAR_DATA *ch, CHAR_DATA *npc);
//--------------------------------------------------------------------------------------------------//
void DREAMWALK_EVENT400_CREATURE_REVEAL			(CHAR_DATA *ch, CHAR_DATA *cre);
void DREAMWALK_EVENT401_CREATURE_ATTACK			(CHAR_DATA *ch, CHAR_DATA *cre);
//==================================================================================================//
// DREAMWALK PROCEDURE                                                          					//
//==================================================================================================//
/*XXX*/ void DREAMWALK_PROCEDURE					(void);
//==================================================================================================//
// ACTIVE SKILLS																					//
//==================================================================================================//
void MANUAL_SKILL_CREATE_WEAPON				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_SHARPEN_WEAPON			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_REINFORCE_ARMOR			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_REPAIR_ITEM				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_INSPECT_ITEM				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_CHARGE_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_DISARM_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_PICK_POCKET				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_PICK_LOCK					(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_MOVE_SILENTLY				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_HIDE_IN_SHADOWS			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_SEARCH_SOMETHING			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_DETECT_TRAP				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_REMOVE_TRAP				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_PEEK_INVENTORY			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_THROW_POISON				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_BACKSTAB_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_CREATE_SCROLL				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_CREATE_POTION				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_IMBUE_WEAPON				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_ENCHANT_ARMOR				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_MEDITATE					(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_COOK_ITEM					(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_BANDAGE_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_TAUNT_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_BLUFF_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_FLIRT_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_INTIMIDATE_CREATURE		(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_JUDGE_CREATURE			(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_HIRE_CREATURE				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_DIG_TUNNEL				(CHAR_DATA *ch, const char *argument);
void MANUAL_SKILL_DREAMWALK					(CHAR_DATA *ch, const char *argument);
//==================================================================================================//
// ITEM USE																							//
//==================================================================================================//
bool AUTOMATIC_ACTION_POTION_USE			(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_SCROLL_USE			(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// COMBAT STYLES																					//
//==================================================================================================//
bool AUTOMATIC_ACTION_BERSERK_STYLE			(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_AGGRESSIVE_STYLE		(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_STANDARD_STYLE		(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_DEFENSIVE_STYLE		(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_EVASIVE_STYLE			(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// OFFENSIVE COMBAT ACTIONS (AUTOMATIC)																//
//==================================================================================================//
bool AUTOMATIC_ACTION_EXECUTE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_BANISH				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_INNERVATE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_BLINK					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_ENTANGLE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_ENRAGE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_GOUGE					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_SLAM					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_KICK					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_PUNCH					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_POUNCE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_BASH					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_GRAPPLE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_STUN					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_OVERPOWER				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_PUMMEL				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_FADE					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_DISENGAGE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_DISTRACT				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_CLEAVE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_WHIRLWIND				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_BLOCK					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_DODGE					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_TUMBLE				(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_PARRY					(CHAR_DATA *ch, CHAR_DATA *victim);
bool AUTOMATIC_ACTION_GRIP					(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// RACIAL SKILLS																					//
//==================================================================================================//
/*001*/ bool AUTOMATIC_RACIAL_BEAST_TONGUE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*002*/ bool AUTOMATIC_RACIAL_DRAGON_SKIN			(CHAR_DATA *ch, CHAR_DATA *victim);
/*003*/ bool AUTOMATIC_RACIAL_ANCESTOR_GUARDIAN		(CHAR_DATA *ch, CHAR_DATA *victim);
/*004*/ bool AUTOMATIC_RACIAL_STAR_OF_THE_WEST		(CHAR_DATA *ch, CHAR_DATA *victim);
/*005*/ bool AUTOMATIC_RACIAL_VOICE_OF_THE_EMPEROR	(CHAR_DATA *ch, CHAR_DATA *victim);
/*006*/ bool AUTOMATIC_RACIAL_EYE_OF_NIGHT			(CHAR_DATA *ch, CHAR_DATA *victim);
/*007*/ bool AUTOMATIC_RACIAL_EYE_OF_FEAR			(CHAR_DATA *ch, CHAR_DATA *victim);
/*008*/ bool AUTOMATIC_RACIAL_THUNDER_FIST			(CHAR_DATA *ch, CHAR_DATA *victim);
/*009*/ bool AUTOMATIC_RACIAL_WOAD					(CHAR_DATA *ch, CHAR_DATA *victim);
/*010*/ bool AUTOMATIC_RACIAL_ORC_BERSERK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*011*/ bool AUTOMATIC_RACIAL_ADRENALINE_RUSH		(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// MAGIC SPELLS																						//
//==================================================================================================//
/*001*/ bool AUTOMATIC_SPELL_DAMAGE_FIRE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*002*/ bool AUTOMATIC_SPELL_DAMAGE_FROST			(CHAR_DATA *ch, CHAR_DATA *victim);
/*003*/ bool AUTOMATIC_SPELL_DAMAGE_SHOCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*004*/ bool AUTOMATIC_SPELL_DAMAGE_POISON			(CHAR_DATA *ch, CHAR_DATA *victim);
/*005*/ bool AUTOMATIC_SPELL_DAMAGE_MAGIC			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*006*/ bool AUTOMATIC_SPELL_RESISTANCE_FIRE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*007*/ bool AUTOMATIC_SPELL_RESISTANCE_FROST		(CHAR_DATA *ch, CHAR_DATA *victim);
/*008*/ bool AUTOMATIC_SPELL_RESISTANCE_SHOCK		(CHAR_DATA *ch, CHAR_DATA *victim);
/*009*/ bool AUTOMATIC_SPELL_RESISTANCE_POISON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*010*/ bool AUTOMATIC_SPELL_RESISTANCE_MAGIC		(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*011*/ bool AUTOMATIC_SPELL_WEAKNESS_FIRE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*012*/ bool AUTOMATIC_SPELL_WEAKNESS_FROST			(CHAR_DATA *ch, CHAR_DATA *victim);
/*013*/ bool AUTOMATIC_SPELL_WEAKNESS_SHOCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*014*/ bool AUTOMATIC_SPELL_WEAKNESS_POISON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*015*/ bool AUTOMATIC_SPELL_WEAKNESS_MAGIC			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*016*/ bool AUTOMATIC_SPELL_SHIELD_FIRE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*017*/ bool AUTOMATIC_SPELL_SHIELD_FROST			(CHAR_DATA *ch, CHAR_DATA *victim);
/*018*/ bool AUTOMATIC_SPELL_SHIELD_SHOCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*019*/ bool AUTOMATIC_SPELL_SHIELD_POISON			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*020*/ bool AUTOMATIC_SPELL_LEVITATE				(CHAR_DATA *ch, CHAR_DATA *victim);
/*021*/ bool AUTOMATIC_SPELL_CHAMELEON				(CHAR_DATA *ch, CHAR_DATA *victim);
/*022*/ bool AUTOMATIC_SPELL_NIGHTEYE				(CHAR_DATA *ch, CHAR_DATA *victim);
/*023*/ bool AUTOMATIC_SPELL_SANCTUARY				(CHAR_DATA *ch, CHAR_DATA *victim);
/*024*/ bool AUTOMATIC_SPELL_INVISIBILITY			(CHAR_DATA *ch, CHAR_DATA *victim);
/*025*/ bool AUTOMATIC_SPELL_SILENCE				(CHAR_DATA *ch, CHAR_DATA *victim);
/*026*/ bool AUTOMATIC_SPELL_BLINDNESS				(CHAR_DATA *ch, CHAR_DATA *victim);
/*027*/ bool AUTOMATIC_SPELL_PARALYSIS				(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*028*/ bool AUTOMATIC_SPELL_BURDEN					(CHAR_DATA *ch, CHAR_DATA *victim);
/*029*/ bool AUTOMATIC_SPELL_FEATHER				(CHAR_DATA *ch, CHAR_DATA *victim);
/*030*/ bool AUTOMATIC_SPELL_SHIELD					(CHAR_DATA *ch, CHAR_DATA *victim);
/*031*/ bool AUTOMATIC_SPELL_SOUND					(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*032*/ bool AUTOMATIC_SPELL_DEMORALIZE_CREATURE	(CHAR_DATA *ch, CHAR_DATA *victim);
/*033*/ bool AUTOMATIC_SPELL_CALM_CREATURE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*034*/ bool AUTOMATIC_SPELL_RALLY_CREATURE			(CHAR_DATA *ch, CHAR_DATA *victim);
/*035*/ bool AUTOMATIC_SPELL_FRENZY_CREATURE		(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*036*/ bool AUTOMATIC_SPELL_DRAIN_STRENGTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*037*/ bool AUTOMATIC_SPELL_DRAIN_ENDURANCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*038*/ bool AUTOMATIC_SPELL_DRAIN_AGILITY			(CHAR_DATA *ch, CHAR_DATA *victim);
/*039*/ bool AUTOMATIC_SPELL_DRAIN_INTELLIGENCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*040*/ bool AUTOMATIC_SPELL_DRAIN_WILLPOWER		(CHAR_DATA *ch, CHAR_DATA *victim);
/*041*/ bool AUTOMATIC_SPELL_DRAIN_PERSONALITY		(CHAR_DATA *ch, CHAR_DATA *victim);
/*042*/ bool AUTOMATIC_SPELL_DRAIN_LUCK				(CHAR_DATA *ch, CHAR_DATA *victim);
/*043*/ bool AUTOMATIC_SPELL_DRAIN_HEALTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*044*/ bool AUTOMATIC_SPELL_DRAIN_MAGICKA			(CHAR_DATA *ch, CHAR_DATA *victim);
/*045*/ bool AUTOMATIC_SPELL_DRAIN_FATIGUE			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*046*/ bool AUTOMATIC_SPELL_DAMAGE_STRENGTH		(CHAR_DATA *ch, CHAR_DATA *victim);
/*047*/ bool AUTOMATIC_SPELL_DAMAGE_ENDURANCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*048*/ bool AUTOMATIC_SPELL_DAMAGE_AGILITY			(CHAR_DATA *ch, CHAR_DATA *victim);
/*049*/ bool AUTOMATIC_SPELL_DAMAGE_INTELLIGENCE	(CHAR_DATA *ch, CHAR_DATA *victim);
/*050*/ bool AUTOMATIC_SPELL_DAMAGE_WILLPOWER		(CHAR_DATA *ch, CHAR_DATA *victim);
/*051*/ bool AUTOMATIC_SPELL_DAMAGE_PERSONALITY		(CHAR_DATA *ch, CHAR_DATA *victim);
/*052*/ bool AUTOMATIC_SPELL_DAMAGE_LUCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*053*/ bool AUTOMATIC_SPELL_DAMAGE_HEALTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*054*/ bool AUTOMATIC_SPELL_DAMAGE_MAGICKA			(CHAR_DATA *ch, CHAR_DATA *victim);
/*055*/ bool AUTOMATIC_SPELL_DAMAGE_FATIGUE			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*056*/ bool AUTOMATIC_SPELL_FORTIFY_STRENGTH		(CHAR_DATA *ch, CHAR_DATA *victim);
/*057*/ bool AUTOMATIC_SPELL_FORTIFY_ENDURANCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*058*/ bool AUTOMATIC_SPELL_FORTIFY_AGILITY		(CHAR_DATA *ch, CHAR_DATA *victim);
/*059*/ bool AUTOMATIC_SPELL_FORTIFY_INTELLIGENCE	(CHAR_DATA *ch, CHAR_DATA *victim);
/*060*/ bool AUTOMATIC_SPELL_FORTIFY_WILLPOWER		(CHAR_DATA *ch, CHAR_DATA *victim);
/*061*/ bool AUTOMATIC_SPELL_FORTIFY_PERSONALITY	(CHAR_DATA *ch, CHAR_DATA *victim);
/*062*/ bool AUTOMATIC_SPELL_FORTIFY_LUCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*063*/ bool AUTOMATIC_SPELL_FORTIFY_HEALTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*064*/ bool AUTOMATIC_SPELL_FORTIFY_FATIGUE		(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*065*/ bool AUTOMATIC_SPELL_DAMAGE_WEAPON			(CHAR_DATA *ch, CHAR_DATA *victim);
/*066*/ bool AUTOMATIC_SPELL_DAMAGE_ARMOR			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*067*/ bool AUTOMATIC_SPELL_ABSORB_STRENGTH		(CHAR_DATA *ch, CHAR_DATA *victim);
/*068*/ bool AUTOMATIC_SPELL_ABSORB_ENDURANCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*069*/ bool AUTOMATIC_SPELL_ABSORB_AGILITY			(CHAR_DATA *ch, CHAR_DATA *victim);
/*070*/ bool AUTOMATIC_SPELL_ABSORB_INTELLIGENCE	(CHAR_DATA *ch, CHAR_DATA *victim);
/*071*/ bool AUTOMATIC_SPELL_ABSORB_WILLPOWER		(CHAR_DATA *ch, CHAR_DATA *victim);
/*072*/ bool AUTOMATIC_SPELL_ABSORB_PERSONALITY		(CHAR_DATA *ch, CHAR_DATA *victim);
/*073*/ bool AUTOMATIC_SPELL_ABSORB_LUCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*074*/ bool AUTOMATIC_SPELL_ABSORB_HEALTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*075*/ bool AUTOMATIC_SPELL_ABSORB_MAGICKA			(CHAR_DATA *ch, CHAR_DATA *victim);
/*076*/ bool AUTOMATIC_SPELL_ABSORB_FATIGUE			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*077*/ bool AUTOMATIC_SPELL_RESTORE_STRENGTH		(CHAR_DATA *ch, CHAR_DATA *victim);
/*078*/ bool AUTOMATIC_SPELL_RESTORE_ENDURANCE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*079*/ bool AUTOMATIC_SPELL_RESTORE_AGILITY		(CHAR_DATA *ch, CHAR_DATA *victim);
/*080*/ bool AUTOMATIC_SPELL_RESTORE_INTELLIGENCE	(CHAR_DATA *ch, CHAR_DATA *victim);
/*081*/ bool AUTOMATIC_SPELL_RESTORE_WILLPOWER		(CHAR_DATA *ch, CHAR_DATA *victim);
/*082*/ bool AUTOMATIC_SPELL_RESTORE_PERSONALITY	(CHAR_DATA *ch, CHAR_DATA *victim);
/*083*/ bool AUTOMATIC_SPELL_RESTORE_LUCK			(CHAR_DATA *ch, CHAR_DATA *victim);
/*084*/ bool AUTOMATIC_SPELL_RESTORE_HEALTH			(CHAR_DATA *ch, CHAR_DATA *victim);
/*085*/ bool AUTOMATIC_SPELL_RESTORE_FATIGUE		(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*086*/ bool AUTOMATIC_SPELL_BOUND_AXEWEAPON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*087*/ bool AUTOMATIC_SPELL_BOUND_BLUNTWEAPON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*088*/ bool AUTOMATIC_SPELL_BOUND_LONGBLADE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*089*/ bool AUTOMATIC_SPELL_BOUND_SHORTBLADE		(CHAR_DATA *ch, CHAR_DATA *victim);
/*090*/ bool AUTOMATIC_SPELL_BOUND_POLEARM			(CHAR_DATA *ch, CHAR_DATA *victim);
/*091*/ bool AUTOMATIC_SPELL_BOUND_HANDWEAPON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*092*/ bool AUTOMATIC_SPELL_BOUND_RANGEDWEAPON		(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*093*/ bool AUTOMATIC_SPELL_BOUND_HELM				(CHAR_DATA *ch, CHAR_DATA *victim);
/*094*/ bool AUTOMATIC_SPELL_BOUND_PAULDRONS		(CHAR_DATA *ch, CHAR_DATA *victim);
/*095*/ bool AUTOMATIC_SPELL_BOUND_CUIRASS			(CHAR_DATA *ch, CHAR_DATA *victim);
/*096*/ bool AUTOMATIC_SPELL_BOUND_GAUNTLETS		(CHAR_DATA *ch, CHAR_DATA *victim);
/*097*/ bool AUTOMATIC_SPELL_BOUND_GREAVES			(CHAR_DATA *ch, CHAR_DATA *victim);
/*098*/ bool AUTOMATIC_SPELL_BOUND_BOOTS			(CHAR_DATA *ch, CHAR_DATA *victim);
/*099*/ bool AUTOMATIC_SPELL_BOUND_SHIELD			(CHAR_DATA *ch, CHAR_DATA *victim);
//--------------------------------------------------------------------------------------------------//
/*100*/ bool AUTOMATIC_SPELL_SUMMON_DAEDRA			(CHAR_DATA *ch, CHAR_DATA *victim);
/*101*/ bool AUTOMATIC_SPELL_SUMMON_BEAST			(CHAR_DATA *ch, CHAR_DATA *victim);
/*102*/ bool AUTOMATIC_SPELL_SUMMON_AUTOMATON		(CHAR_DATA *ch, CHAR_DATA *victim);
/*103*/ bool AUTOMATIC_SPELL_SUMMON_ATRONACH		(CHAR_DATA *ch, CHAR_DATA *victim);
/*104*/ bool AUTOMATIC_SPELL_SUMMON_UNDEAD			(CHAR_DATA *ch, CHAR_DATA *victim);
//==================================================================================================//
// DREAMWALK SPELL PROCEDURES																		//
//==================================================================================================//
/*001*/ void RESET_EFFECT_VARIABLES					(CHAR_DATA *ch);
/*002*/ void EFFECT_TYPE_MODIFY_AREA				(int sn, CHAR_DATA *ch, void *vo, char *target_name);
/*003*/ void EFFECT_TYPE_ELEMENTAL_DAMAGE			(int sn, CHAR_DATA *ch, void *vo);
/*004*/ void EFFECT_TYPE_MODIFY_RESISTANCE			(int sn, CHAR_DATA *ch, void *vo);
/*005*/ void EFFECT_TYPE_APPLY_BEHAVIOUR			(int sn, CHAR_DATA *ch, void *vo);
/*006*/ void EFFECT_TYPE_VARIOUS_THINGS				(int sn, CHAR_DATA *ch, void *vo);
/*007*/ void EFFECT_TYPE_MODIFY_ATTRIBUTE			(int sn, CHAR_DATA *ch, void *vo);
/*008*/ void EFFECT_TYPE_CHANGE_ATTRIBUTE			(int sn, CHAR_DATA *ch, void *vo);
/*009*/ void EFFECT_TYPE_DAMAGE_EQUIPMENT			(int sn, CHAR_DATA *ch, void *vo);
/*010*/ void EFFECT_TYPE_SUMMON_ITEM				(int sn, CHAR_DATA *ch, void *vo);
/*011*/ void EFFECT_TYPE_SUMMON_CREATURE			(int sn, CHAR_DATA *ch, void *vo);
//==================================================================================================//
// ACTIVE SPELLS																					//
//==================================================================================================//
/*001*/ ch_ret MANUAL_SPELL_DAMAGE_FIRE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*002*/ ch_ret MANUAL_SPELL_DAMAGE_FROST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*003*/ ch_ret MANUAL_SPELL_DAMAGE_SHOCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*004*/ ch_ret MANUAL_SPELL_DAMAGE_POISON			(int sn, int level, CHAR_DATA * ch, void *vo);
/*005*/ ch_ret MANUAL_SPELL_DAMAGE_MAGIC			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*006*/ ch_ret MANUAL_SPELL_IMMUNITY_FIRE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*007*/ ch_ret MANUAL_SPELL_IMMUNITY_FROST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*008*/ ch_ret MANUAL_SPELL_IMMUNITY_SHOCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*009*/ ch_ret MANUAL_SPELL_IMMUNITY_POISON			(int sn, int level, CHAR_DATA * ch, void *vo);
/*010*/ ch_ret MANUAL_SPELL_IMMUNITY_MAGIC			(int sn, int level, CHAR_DATA * ch, void *vo);
/*011*/ ch_ret MANUAL_SPELL_IMMUNITY_DISEASE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*012*/ ch_ret MANUAL_SPELL_IMMUNITY_BLIGHT			(int sn, int level, CHAR_DATA * ch, void *vo);
/*013*/ ch_ret MANUAL_SPELL_IMMUNITY_CORPRUS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*014*/ ch_ret MANUAL_SPELL_IMMUNITY_PARALYSIS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*015*/ ch_ret MANUAL_SPELL_IMMUNITY_WEAPON			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*016*/ ch_ret MANUAL_SPELL_RESISTANCE_FIRE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*017*/ ch_ret MANUAL_SPELL_RESISTANCE_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*018*/ ch_ret MANUAL_SPELL_RESISTANCE_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*019*/ ch_ret MANUAL_SPELL_RESISTANCE_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*020*/ ch_ret MANUAL_SPELL_RESISTANCE_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
/*021*/ ch_ret MANUAL_SPELL_RESISTANCE_DISEASE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*022*/ ch_ret MANUAL_SPELL_RESISTANCE_BLIGHT		(int sn, int level, CHAR_DATA * ch, void *vo);
/*023*/ ch_ret MANUAL_SPELL_RESISTANCE_CORPRUS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*024*/ ch_ret MANUAL_SPELL_RESISTANCE_PARALYSIS	(int sn, int level, CHAR_DATA * ch, void *vo);
/*025*/ ch_ret MANUAL_SPELL_RESISTANCE_WEAPON		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*026*/ ch_ret MANUAL_SPELL_WEAKNESS_FIRE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*027*/ ch_ret MANUAL_SPELL_WEAKNESS_FROST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*028*/ ch_ret MANUAL_SPELL_WEAKNESS_SHOCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*029*/ ch_ret MANUAL_SPELL_WEAKNESS_POISON			(int sn, int level, CHAR_DATA * ch, void *vo);
/*030*/ ch_ret MANUAL_SPELL_WEAKNESS_MAGIC			(int sn, int level, CHAR_DATA * ch, void *vo);
/*031*/ ch_ret MANUAL_SPELL_WEAKNESS_DISEASE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*032*/ ch_ret MANUAL_SPELL_WEAKNESS_BLIGHT			(int sn, int level, CHAR_DATA * ch, void *vo);
/*033*/ ch_ret MANUAL_SPELL_WEAKNESS_CORPRUS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*034*/ ch_ret MANUAL_SPELL_WEAKNESS_PARALYSIS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*035*/ ch_ret MANUAL_SPELL_WEAKNESS_WEAPON			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*036*/ ch_ret MANUAL_SPELL_OPEN					(int sn, int level, CHAR_DATA * ch, void *vo);
/*037*/ ch_ret MANUAL_SPELL_LOCK					(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*038*/ ch_ret MANUAL_SPELL_SHIELD_FIRE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*039*/ ch_ret MANUAL_SPELL_SHIELD_FROST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*040*/ ch_ret MANUAL_SPELL_SHIELD_SHOCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*041*/ ch_ret MANUAL_SPELL_SHIELD_POISON			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*042*/ ch_ret MANUAL_SPELL_LEVITATE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*043*/ ch_ret MANUAL_SPELL_WATER_BREATHING			(int sn, int level, CHAR_DATA * ch, void *vo);
/*044*/ ch_ret MANUAL_SPELL_CHAMELEON				(int sn, int level, CHAR_DATA * ch, void *vo);
/*045*/ ch_ret MANUAL_SPELL_SANCTUARY				(int sn, int level, CHAR_DATA * ch, void *vo);
/*046*/ ch_ret MANUAL_SPELL_INVISIBILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*047*/ ch_ret MANUAL_SPELL_NIGHTEYE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*048*/ ch_ret MANUAL_SPELL_DETECT_LIFE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*049*/ ch_ret MANUAL_SPELL_DETECT_ENCHANTMENTS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*050*/ ch_ret MANUAL_SPELL_DETECT_KEYS				(int sn, int level, CHAR_DATA * ch, void *vo);
/*051*/ ch_ret MANUAL_SPELL_RECALL					(int sn, int level, CHAR_DATA * ch, void *vo);
/*052*/ ch_ret MANUAL_SPELL_SILENCE					(int sn, int level, CHAR_DATA * ch, void *vo);
/*053*/ ch_ret MANUAL_SPELL_BLINDNESS				(int sn, int level, CHAR_DATA * ch, void *vo);
/*054*/ ch_ret MANUAL_SPELL_CHARM					(int sn, int level, CHAR_DATA * ch, void *vo);
/*055*/ ch_ret MANUAL_SPELL_PARALYSIS				(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*056*/ ch_ret MANUAL_SPELL_BURDEN					(int sn, int level, CHAR_DATA * ch, void *vo);
/*057*/ ch_ret MANUAL_SPELL_FEATHER					(int sn, int level, CHAR_DATA * ch, void *vo);
/*058*/ ch_ret MANUAL_SPELL_SHIELD					(int sn, int level, CHAR_DATA * ch, void *vo);
/*059*/ ch_ret MANUAL_SPELL_SOUND					(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*060*/ ch_ret MANUAL_SPELL_DRAIN_STRENGTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*061*/ ch_ret MANUAL_SPELL_DRAIN_ENDURANCE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*062*/ ch_ret MANUAL_SPELL_DRAIN_AGILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*063*/ ch_ret MANUAL_SPELL_DRAIN_INTELLIGENCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*064*/ ch_ret MANUAL_SPELL_DRAIN_WILLPOWER			(int sn, int level, CHAR_DATA * ch, void *vo);
/*065*/ ch_ret MANUAL_SPELL_DRAIN_PERSONALITY		(int sn, int level, CHAR_DATA * ch, void *vo);
/*066*/ ch_ret MANUAL_SPELL_DRAIN_LUCK				(int sn, int level, CHAR_DATA * ch, void *vo);
/*067*/ ch_ret MANUAL_SPELL_DRAIN_ATTACK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*068*/ ch_ret MANUAL_SPELL_DRAIN_HEALTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*069*/ ch_ret MANUAL_SPELL_DRAIN_MAGICKA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*070*/ ch_ret MANUAL_SPELL_DRAIN_FATIGUE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*071*/ ch_ret MANUAL_SPELL_FORTIFY_STRENGTH		(int sn, int level, CHAR_DATA * ch, void *vo);
/*072*/ ch_ret MANUAL_SPELL_FORTIFY_ENDURANCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*073*/ ch_ret MANUAL_SPELL_FORTIFY_AGILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*074*/ ch_ret MANUAL_SPELL_FORTIFY_INTELLIGENCE	(int sn, int level, CHAR_DATA * ch, void *vo);
/*075*/ ch_ret MANUAL_SPELL_FORTIFY_WILLPOWER		(int sn, int level, CHAR_DATA * ch, void *vo);
/*076*/ ch_ret MANUAL_SPELL_FORTIFY_PERSONALITY		(int sn, int level, CHAR_DATA * ch, void *vo);
/*077*/ ch_ret MANUAL_SPELL_FORTIFY_LUCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*078*/ ch_ret MANUAL_SPELL_FORTIFY_ATTACK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*079*/ ch_ret MANUAL_SPELL_FORTIFY_HEALTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*080*/ ch_ret MANUAL_SPELL_FORTIFY_MAGICKA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*081*/ ch_ret MANUAL_SPELL_FORTIFY_FATIGUE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*082*/ ch_ret MANUAL_SPELL_ABSORB_STRENGTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*083*/ ch_ret MANUAL_SPELL_ABSORB_ENDURANCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*084*/ ch_ret MANUAL_SPELL_ABSORB_AGILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*085*/ ch_ret MANUAL_SPELL_ABSORB_INTELLIGENCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*086*/ ch_ret MANUAL_SPELL_ABSORB_WILLPOWER		(int sn, int level, CHAR_DATA * ch, void *vo);
/*087*/ ch_ret MANUAL_SPELL_ABSORB_PERSONALITY		(int sn, int level, CHAR_DATA * ch, void *vo);
/*088*/ ch_ret MANUAL_SPELL_ABSORB_LUCK				(int sn, int level, CHAR_DATA * ch, void *vo);
/*089*/ ch_ret MANUAL_SPELL_ABSORB_HEALTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*090*/ ch_ret MANUAL_SPELL_ABSORB_MAGICKA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*091*/ ch_ret MANUAL_SPELL_ABSORB_FATIGUE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*092*/ ch_ret MANUAL_SPELL_DEMORALIZE_CREATURE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*093*/ ch_ret MANUAL_SPELL_CALM_CREATURE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*094*/ ch_ret MANUAL_SPELL_RALLY_CREATURE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*095*/ ch_ret MANUAL_SPELL_FRENZY_CREATURE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*096*/ ch_ret MANUAL_SPELL_DAMAGE_STRENGTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*097*/ ch_ret MANUAL_SPELL_DAMAGE_ENDURANCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*098*/ ch_ret MANUAL_SPELL_DAMAGE_AGILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*099*/ ch_ret MANUAL_SPELL_DAMAGE_INTELLIGENCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*100*/ ch_ret MANUAL_SPELL_DAMAGE_WILLPOWER		(int sn, int level, CHAR_DATA * ch, void *vo);
/*101*/ ch_ret MANUAL_SPELL_DAMAGE_PERSONALITY		(int sn, int level, CHAR_DATA * ch, void *vo);
/*102*/ ch_ret MANUAL_SPELL_DAMAGE_LUCK				(int sn, int level, CHAR_DATA * ch, void *vo);
/*103*/ ch_ret MANUAL_SPELL_DAMAGE_HEALTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*104*/ ch_ret MANUAL_SPELL_DAMAGE_MAGICKA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*105*/ ch_ret MANUAL_SPELL_DAMAGE_FATIGUE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*106*/ ch_ret MANUAL_SPELL_RESTORE_STRENGTH		(int sn, int level, CHAR_DATA * ch, void *vo);
/*107*/ ch_ret MANUAL_SPELL_RESTORE_ENDURANCE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*108*/ ch_ret MANUAL_SPELL_RESTORE_AGILITY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*109*/ ch_ret MANUAL_SPELL_RESTORE_INTELLIGENCE	(int sn, int level, CHAR_DATA * ch, void *vo);
/*110*/ ch_ret MANUAL_SPELL_RESTORE_WILLPOWER		(int sn, int level, CHAR_DATA * ch, void *vo);
/*111*/ ch_ret MANUAL_SPELL_RESTORE_PERSONALITY		(int sn, int level, CHAR_DATA * ch, void *vo);
/*112*/ ch_ret MANUAL_SPELL_RESTORE_LUCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*113*/ ch_ret MANUAL_SPELL_RESTORE_HEALTH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*114*/ ch_ret MANUAL_SPELL_RESTORE_MAGICKA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*115*/ ch_ret MANUAL_SPELL_RESTORE_FATIGUE			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*116*/ ch_ret MANUAL_SPELL_DAMAGE_WEAPON			(int sn, int level, CHAR_DATA * ch, void *vo);
/*117*/ ch_ret MANUAL_SPELL_DAMAGE_ARMOR			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*118*/ ch_ret MANUAL_SPELL_CREATE_FOOD				(int sn, int level, CHAR_DATA * ch, void *vo);
/*119*/ ch_ret MANUAL_SPELL_CREATE_WATER			(int sn, int level, CHAR_DATA * ch, void *vo);
/*120*/ ch_ret MANUAL_SPELL_LIGHT					(int sn, int level, CHAR_DATA * ch, void *vo);
/*121*/ ch_ret MANUAL_SPELL_SOULTRAP				(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*122*/ ch_ret MANUAL_SPELL_BOUND_AXEWEAPON			(int sn, int level, CHAR_DATA * ch, void *vo);
/*123*/ ch_ret MANUAL_SPELL_BOUND_BLUNTWEAPON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*124*/ ch_ret MANUAL_SPELL_BOUND_LONGBLADE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*125*/ ch_ret MANUAL_SPELL_BOUND_SHORTBLADE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*126*/ ch_ret MANUAL_SPELL_BOUND_POLEARM			(int sn, int level, CHAR_DATA * ch, void *vo);
/*127*/ ch_ret MANUAL_SPELL_BOUND_RANGEDWEAPON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*128*/ ch_ret MANUAL_SPELL_BOUND_THROWNWEAPON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*129*/ ch_ret MANUAL_SPELL_BOUND_HANDWEAPON		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*130*/ ch_ret MANUAL_SPELL_BOUND_HELM				(int sn, int level, CHAR_DATA * ch, void *vo);
/*131*/ ch_ret MANUAL_SPELL_BOUND_PAULDRONS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*132*/ ch_ret MANUAL_SPELL_BOUND_CUIRASS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*133*/ ch_ret MANUAL_SPELL_BOUND_GAUNTLETS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*134*/ ch_ret MANUAL_SPELL_BOUND_GREAVES			(int sn, int level, CHAR_DATA * ch, void *vo);
/*135*/ ch_ret MANUAL_SPELL_BOUND_BOOTS				(int sn, int level, CHAR_DATA * ch, void *vo);
/*136*/ ch_ret MANUAL_SPELL_BOUND_SHIELD			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*137*/ ch_ret MANUAL_SPELL_SUMMON_DAEDRA			(int sn, int level, CHAR_DATA * ch, void *vo);
/*138*/ ch_ret MANUAL_SPELL_SUMMON_BEAST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*139*/ ch_ret MANUAL_SPELL_SUMMON_AUTOMATON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*140*/ ch_ret MANUAL_SPELL_SUMMON_ATRONACH			(int sn, int level, CHAR_DATA * ch, void *vo);
/*141*/ ch_ret MANUAL_SPELL_SUMMON_UNDEAD			(int sn, int level, CHAR_DATA * ch, void *vo);
//==================================================================================================//
// SPELLS: DISEASES																					//
//==================================================================================================//
/*001*/ ch_ret DISEASE_SPELL_SWAMP_FEVER			(int sn, int level, CHAR_DATA * ch, void *vo);
/*002*/ ch_ret DISEASE_SPELL_WITBANE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*003*/ ch_ret DISEASE_SPELL_ATAXIA					(int sn, int level, CHAR_DATA * ch, void *vo);
/*004*/ ch_ret DISEASE_SPELL_BROWN_ROT				(int sn, int level, CHAR_DATA * ch, void *vo);
/*005*/ ch_ret DISEASE_SPELL_HELLJOINT				(int sn, int level, CHAR_DATA * ch, void *vo);
/*006*/ ch_ret DISEASE_SPELL_YELLOW_TICK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*007*/ ch_ret DISEASE_SPELL_DROOPS					(int sn, int level, CHAR_DATA * ch, void *vo);
/*008*/ ch_ret DISEASE_SPELL_COLLYWOBBLES			(int sn, int level, CHAR_DATA * ch, void *vo);
/*009*/ ch_ret DISEASE_SPELL_BLACK_HEART			(int sn, int level, CHAR_DATA * ch, void *vo);
/*010*/ ch_ret DISEASE_SPELL_ASH_CHANCRE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*011*/ ch_ret DISEASE_SPELL_CHANTHRAX				(int sn, int level, CHAR_DATA * ch, void *vo);
/*012*/ ch_ret DISEASE_SPELL_ASH_WOE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*013*/ ch_ret DISEASE_SPELL_CORPRUS				(int sn, int level, CHAR_DATA * ch, void *vo);
/*014*/ ch_ret DISEASE_SPELL_VAMPIRISM				(int sn, int level, CHAR_DATA * ch, void *vo);
/*015*/ ch_ret DISEASE_SPELL_LYCANTHROPY			(int sn, int level, CHAR_DATA * ch, void *vo);
//==================================================================================================//
// SPELLS: EQUIP ARMOR AND WEAPONS																	//
//==================================================================================================//
/*001*/ ch_ret EQUIP_SPELL_UNARMORED_HEAD			(int sn, int level, CHAR_DATA * ch, void *vo);
/*002*/ ch_ret EQUIP_SPELL_UNARMORED_SHOULDERS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*003*/ ch_ret EQUIP_SPELL_UNARMORED_BODY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*004*/ ch_ret EQUIP_SPELL_UNARMORED_HANDS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*005*/ ch_ret EQUIP_SPELL_UNARMORED_WAIST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*006*/ ch_ret EQUIP_SPELL_UNARMORED_LEGS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*007*/ ch_ret EQUIP_SPELL_UNARMORED_FEET			(int sn, int level, CHAR_DATA * ch, void *vo);
/*008*/ ch_ret EQUIP_SPELL_UNARMORED_SHIELD			(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*009*/ ch_ret EQUIP_SPELL_LIGHTARMOR_HEAD			(int sn, int level, CHAR_DATA * ch, void *vo);
/*010*/ ch_ret EQUIP_SPELL_LIGHTARMOR_SHOULDERS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*011*/ ch_ret EQUIP_SPELL_LIGHTARMOR_BODY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*012*/ ch_ret EQUIP_SPELL_LIGHTARMOR_HANDS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*013*/ ch_ret EQUIP_SPELL_LIGHTARMOR_WAIST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*014*/ ch_ret EQUIP_SPELL_LIGHTARMOR_LEGS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*015*/ ch_ret EQUIP_SPELL_LIGHTARMOR_FEET			(int sn, int level, CHAR_DATA * ch, void *vo);
/*016*/ ch_ret EQUIP_SPELL_LIGHTARMOR_SHIELD		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*017*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_HEAD			(int sn, int level, CHAR_DATA * ch, void *vo);
/*018*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_SHOULDERS	(int sn, int level, CHAR_DATA * ch, void *vo);
/*019*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_BODY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*020*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_HANDS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*021*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_WAIST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*022*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_LEGS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*023*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_FEET			(int sn, int level, CHAR_DATA * ch, void *vo);
/*024*/ ch_ret EQUIP_SPELL_MEDIUMARMOR_SHIELD		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*025*/ ch_ret EQUIP_SPELL_HEAVYARMOR_HEAD			(int sn, int level, CHAR_DATA * ch, void *vo);
/*026*/ ch_ret EQUIP_SPELL_HEAVYARMOR_SHOULDERS		(int sn, int level, CHAR_DATA * ch, void *vo);
/*027*/ ch_ret EQUIP_SPELL_HEAVYARMOR_BODY			(int sn, int level, CHAR_DATA * ch, void *vo);
/*028*/ ch_ret EQUIP_SPELL_HEAVYARMOR_HANDS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*029*/ ch_ret EQUIP_SPELL_HEAVYARMOR_WAIST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*030*/ ch_ret EQUIP_SPELL_HEAVYARMOR_LEGS			(int sn, int level, CHAR_DATA * ch, void *vo);
/*031*/ ch_ret EQUIP_SPELL_HEAVYARMOR_FEET			(int sn, int level, CHAR_DATA * ch, void *vo);
/*032*/ ch_ret EQUIP_SPELL_HEAVYARMOR_SHIELD		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*033*/ ch_ret EQUIP_SPELL_AXEWEAPON				(int sn, int level, CHAR_DATA * ch, void *vo);
/*034*/ ch_ret EQUIP_SPELL_BLUNTWEAPON				(int sn, int level, CHAR_DATA * ch, void *vo);
/*035*/ ch_ret EQUIP_SPELL_SHORTBLADE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*036*/ ch_ret EQUIP_SPELL_LONGBLADE				(int sn, int level, CHAR_DATA * ch, void *vo);
/*037*/ ch_ret EQUIP_SPELL_POLEARM					(int sn, int level, CHAR_DATA * ch, void *vo);
/*038*/ ch_ret EQUIP_SPELL_RANGEDWEAPON				(int sn, int level, CHAR_DATA * ch, void *vo);
/*039*/ ch_ret EQUIP_SPELL_THROWNWEAPON				(int sn, int level, CHAR_DATA * ch, void *vo);
/*040*/ ch_ret EQUIP_SPELL_HANDWEAPON				(int sn, int level, CHAR_DATA * ch, void *vo);
//==================================================================================================//
// SPELLS: VARIOUS																					//
//==================================================================================================//
/*001*/ ch_ret MANUAL_SPELL_DREAMWALK				(int sn, int level, CHAR_DATA * ch, void *vo);
/*002*/ ch_ret MANUAL_SPELL_ARENA					(int sn, int level, CHAR_DATA * ch, void *vo);
/*003*/ ch_ret MANUAL_SPELL_RESTORE_HEALTH_FREE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*004*/ ch_ret MANUAL_SPELL_DAMAGE_SILVER			(int sn, int level, CHAR_DATA * ch, void *vo);
/*005*/ ch_ret MANUAL_SPELL_LOCK_PROCEDURE			(int sn, int level, CHAR_DATA * ch, void *vo);
//==================================================================================================//
// SPELLS: EQUIPMENT EFFECTS																		//
//==================================================================================================//
/*001*/ ch_ret PASSIVE_SPELL_ARMOR_DRAIN_HEALTH		(int sn, int level, CHAR_DATA * ch, void *vo);
/*002*/ ch_ret PASSIVE_SPELL_ARMOR_DRAIN_MAGICKA	(int sn, int level, CHAR_DATA * ch, void *vo);
/*003*/ ch_ret PASSIVE_SPELL_ARMOR_DRAIN_FATIGUE	(int sn, int level, CHAR_DATA * ch, void *vo);
/*004*/ ch_ret PASSIVE_SPELL_ARMOR_SHIELD_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*005*/ ch_ret PASSIVE_SPELL_ARMOR_SHIELD_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*006*/ ch_ret PASSIVE_SPELL_ARMOR_SHIELD_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*007*/ ch_ret PASSIVE_SPELL_ARMOR_SHIELD_POISON	(int sn, int level, CHAR_DATA * ch, void *vo);
/*008*/ ch_ret PASSIVE_SPELL_ARMOR_PARALYSIS		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*009*/ ch_ret MANUAL_SPELL_RETALIATE_FIRE			(int sn, int level, CHAR_DATA * ch, void *vo);
/*010*/ ch_ret MANUAL_SPELL_RETALIATE_FROST			(int sn, int level, CHAR_DATA * ch, void *vo);
/*011*/ ch_ret MANUAL_SPELL_RETALIATE_SHOCK			(int sn, int level, CHAR_DATA * ch, void *vo);
/*012*/ ch_ret MANUAL_SPELL_RETALIATE_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*013*/ ch_ret WEAPON_SPELL_DAMAGE_0307_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*014*/ ch_ret WEAPON_SPELL_DAMAGE_0307_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*015*/ ch_ret WEAPON_SPELL_DAMAGE_0307_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*016*/ ch_ret WEAPON_SPELL_DAMAGE_0307_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*017*/ ch_ret WEAPON_SPELL_DAMAGE_0307_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*018*/ ch_ret WEAPON_SPELL_DAMAGE_0104_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*019*/ ch_ret WEAPON_SPELL_DAMAGE_0104_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*020*/ ch_ret WEAPON_SPELL_DAMAGE_0104_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*021*/ ch_ret WEAPON_SPELL_DAMAGE_0104_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*022*/ ch_ret WEAPON_SPELL_DAMAGE_0104_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*023*/ ch_ret WEAPON_SPELL_DAMAGE_0305_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*024*/ ch_ret WEAPON_SPELL_DAMAGE_0305_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*025*/ ch_ret WEAPON_SPELL_DAMAGE_0305_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*026*/ ch_ret WEAPON_SPELL_DAMAGE_0305_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*027*/ ch_ret WEAPON_SPELL_DAMAGE_0305_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*028*/ ch_ret WEAPON_SPELL_DAMAGE_0007_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*029*/ ch_ret WEAPON_SPELL_DAMAGE_0007_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*030*/ ch_ret WEAPON_SPELL_DAMAGE_0007_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*031*/ ch_ret WEAPON_SPELL_DAMAGE_0007_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*032*/ ch_ret WEAPON_SPELL_DAMAGE_0007_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*033*/ ch_ret WEAPON_SPELL_DAMAGE_0407_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*034*/ ch_ret WEAPON_SPELL_DAMAGE_0407_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*035*/ ch_ret WEAPON_SPELL_DAMAGE_0407_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*036*/ ch_ret WEAPON_SPELL_DAMAGE_0407_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*037*/ ch_ret WEAPON_SPELL_DAMAGE_0407_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*038*/ ch_ret WEAPON_SPELL_DAMAGE_0509_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*039*/ ch_ret WEAPON_SPELL_DAMAGE_0509_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*040*/ ch_ret WEAPON_SPELL_DAMAGE_0509_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*041*/ ch_ret WEAPON_SPELL_DAMAGE_0509_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*042*/ ch_ret WEAPON_SPELL_DAMAGE_0509_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*043*/ ch_ret WEAPON_SPELL_DAMAGE_0110_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*044*/ ch_ret WEAPON_SPELL_DAMAGE_0110_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*045*/ ch_ret WEAPON_SPELL_DAMAGE_0110_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*046*/ ch_ret WEAPON_SPELL_DAMAGE_0110_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*047*/ ch_ret WEAPON_SPELL_DAMAGE_0110_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*048*/ ch_ret WEAPON_SPELL_DAMAGE_0125_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*049*/ ch_ret WEAPON_SPELL_DAMAGE_0125_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*050*/ ch_ret WEAPON_SPELL_DAMAGE_0125_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*051*/ ch_ret WEAPON_SPELL_DAMAGE_0125_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*052*/ ch_ret WEAPON_SPELL_DAMAGE_0125_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*053*/ ch_ret WEAPON_SPELL_DAMAGE_0511_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*054*/ ch_ret WEAPON_SPELL_DAMAGE_0511_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*055*/ ch_ret WEAPON_SPELL_DAMAGE_0511_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*056*/ ch_ret WEAPON_SPELL_DAMAGE_0511_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*057*/ ch_ret WEAPON_SPELL_DAMAGE_0511_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*058*/ ch_ret WEAPON_SPELL_DAMAGE_0120_FIRE		(int sn, int level, CHAR_DATA * ch, void *vo);
/*059*/ ch_ret WEAPON_SPELL_DAMAGE_0120_FROST		(int sn, int level, CHAR_DATA * ch, void *vo);
/*060*/ ch_ret WEAPON_SPELL_DAMAGE_0120_SHOCK		(int sn, int level, CHAR_DATA * ch, void *vo);
/*061*/ ch_ret WEAPON_SPELL_DAMAGE_0120_POISON		(int sn, int level, CHAR_DATA * ch, void *vo);
/*062*/ ch_ret WEAPON_SPELL_DAMAGE_0120_MAGIC		(int sn, int level, CHAR_DATA * ch, void *vo);
//--------------------------------------------------------------------------------------------------//
/*063*/ ch_ret MANUAL_SPELL_DAMAGE_0110_HEALTH		(int sn, int level, CHAR_DATA * ch, void *vo);
/*064*/ ch_ret MANUAL_SPELL_DAMAGE_0110_MAGICKA		(int sn, int level, CHAR_DATA * ch, void *vo);
/*065*/ ch_ret MANUAL_SPELL_DAMAGE_0110_FATIGUE		(int sn, int level, CHAR_DATA * ch, void *vo);
//==================================================================================================//
// DREAMWALK TABLES																					//
//==================================================================================================//	
/*001*/ extern const TES_RACE_INFO 					TES_INFO_TABLE_RACE_INFO[];
/*002*/ extern const TES_RACE_NATTRIBUTE 			TES_INFO_TABLE_RACE_NATTRIBUTE[];
/*004*/ extern const TES_RACE_VATTRIBUTE 			TES_INFO_TABLE_RACE_VATTRIBUTE[];
/*005*/ extern const TES_RACE_SKILL 				TES_INFO_TABLE_RACE_SKILL[];
/*006*/ extern const TES_RACE_RESISTANCE 			TES_INFO_TABLE_RACE_RESISTANCE[];
/*007*/ extern const TES_RACE_FACTION 				TES_INFO_TABLE_RACE_FACTION[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_BIRTHSIGN_INFO 			TES_INFO_TABLE_BIRTHSIGN_INFO[];
/*002*/ extern const TES_BIRTHSIGN_NATTRIBUTE 		TES_INFO_TABLE_BIRTHSIGN_NATTRIBUTE[];
/*003*/ extern const TES_BIRTHSIGN_VATTRIBUTE 		TES_INFO_TABLE_BIRTHSIGN_VATTRIBUTE[];
/*004*/ extern const TES_BIRTHSIGN_SKILL 			TES_INFO_TABLE_BIRTHSIGN_SKILL[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_ALIGNMENT_INFO 			TES_INFO_TABLE_ALIGNMENT_INFO[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_CLASS_INFO 				TES_INFO_TABLE_CLASS_INFO[];
/*002*/ extern const TES_CLASS_NATTRIBUTE 			TES_INFO_TABLE_CLASS_NATTRIBUTE[];
/*003*/ extern const TES_CLASS_VATTRIBUTE 			TES_INFO_TABLE_CLASS_VATTRIBUTE[]; 
/*004*/ extern const TES_CLASS_SKILL 				TES_INFO_TABLE_CLASS_SKILL[];
/*005*/ extern const TES_CLASS_FACTION 				TES_INFO_TABLE_CLASS_F[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_CLASSTYPE_INFO 			TES_INFO_TABLE_CLASSTYPE_INFO[];
/*002*/ extern const TES_CLASSTYPE_NATTRIBUTE 		TES_INFO_TABLE_CLASSTYPE_NATTRIBUTE[];
/*003*/ extern const TES_CLASSTYPE_VATTRIBUTE 		TES_INFO_TABLE_CLASSTYPE_VATTRIBUTE[]; 
/*004*/ extern const TES_CLASSTYPE_SKILL 			TES_INFO_TABLE_CLASSTYPE_SKILL[]; 	
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_CLASSSPEC_INFO 			TES_INFO_TABLE_CLASSSPEC_INFO[]; 
/*002*/ extern const TES_CLASSSPEC_NATTRIBUTE 		TES_INFO_TABLE_CLASSSPEC_NATTRIBUTE[];
/*003*/ extern const TES_CLASSSPEC_VATTRIBUTE 		TES_INFO_TABLE_CLASSSPEC_VATTRIBUTE[];
/*004*/ extern const TES_CLASSSPEC_SKILL 			TES_INFO_TABLE_CLASSSPEC_SKILL[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_FACTION_INFO 				TES_INFO_TABLE_FACTION_INFO[]; 
/*002*/ extern const TES_FACTION_NATTRIBUTE 		TES_INFO_TABLE_FACTION_NATTRIBUTE[];
/*003*/ extern const TES_FACTION_VATTRIBUTE 		TES_INFO_TABLE_FACTION_VATTRIBUTE[];
/*004*/ extern const TES_FACTION_SKILL 				TES_INFO_TABLE_FACTION_SKILL[];
/*005*/ extern const TES_FACTION_RANK 				TES_INFO_TABLE_FACTION_RANK[];
/*006*/ extern const TES_FACTION_DISPOSITION 		TES_INFO_TABLE_FACTION_DISPOSITION[];
/*007*/ extern const TES_FACTION_RELATION 			TES_INFO_TABLE_FACTION_RELATION[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_PLAYER_IDLE 				TES_INFO_TABLE_PLAYER_IDLE[];
/*002*/ extern const TES_CREATURE_INFO 				TES_INFO_TABLE_CREATURE_INFO[];
/*003*/ extern const TES_NPC_INFO 					TES_INFO_TABLE_NPC_INFO[]; 	
/*004*/ extern const TES_LOCATION_INFO 				TES_INFO_TABLE_LOCATION_INFO[];
/*005*/ extern const TES_CRAFTING_STATION_INFO 		TES_INFO_TABLE_CRAFTING_STATION_INFO[];
/*006*/ extern const  TES_ADVENTURE_INFO 			TES_INFO_TABLE_ADVENTURE_INFO[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_ARMOR_INFO 				TES_INFO_TABLE_ARMOR_INFO[];
/*002*/ extern const TES_WEAPON_INFO 				TES_INFO_TABLE_WEAPON_INFO[];
/*003*/ extern const TES_NECK_INFO 					TES_INFO_TABLE_NECK_INFO[];
/*004*/ extern const TES_FINGER_INFO 				TES_INFO_TABLE_FINGER_INFO[];
/*005*/ extern const TES_INGREDIENT_INFO 			TES_INFO_TABLE_INGREDIENT_INFO[];
/*006*/ extern const TES_POTION_INFO 				TES_INFO_TABLE_POTION_INFO[];
/*007*/ extern const TES_SCROLL_INFO 				TES_INFO_TABLE_SCROLL_INFO[];
/*008*/ extern const TES_CLUTTER_INFO 				TES_INFO_TABLE_CLUTTER_INFO[];
//--------------------------------------------------------------------------------------------------//
/*001*/ extern const TES_EFFECT_INFO 				TES_INFO_TABLE_EFFECT_INFO[];
/*002*/ extern const TES_ACTION_INFO 				TES_INFO_TABLE_ACTION_INFO[];
//--------------------------------------------------------------------------------------------------//

#endif