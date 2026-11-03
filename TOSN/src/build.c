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
 *                      Online Building and Editing Module                  *
 ****************************************************************************/
/*
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mud.h"
#include "sha256.h"

extern int top_affect;
extern int top_reset;
extern int top_ed;
extern bool fBootDb;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

REL_DATA *first_relation = NULL;
REL_DATA *last_relation = NULL;

bool check_area_conflict( AREA_DATA * carea, int low_range, int hi_range );
char *sprint_reset( RESET_DATA * pReset, short *num );
void fix_exits( void );
bool validate_spec_fun( const char *name );

/*
 * Exit Pull/push types
 * (water, air, earth, fire)
 */
const char *const ex_pmisc[] = { "undefined", "vortex", "vacuum", "slip", "ice", "mysterious" };

const char *const ex_pwater[] = { "current", "wave", "whirlpool", "geyser" };

const char *const ex_pair[] = { "wind", "storm", "coldwind", "breeze" };

const char *const ex_pearth[] = { "landslide", "sinkhole", "quicksand", "earthquake" };

const char *const ex_pfire[] = { "lava", "hotair" };

const char *const ex_flags[] = {
   "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
   "dig", "eatkey", "nopassdoor", "hidden", "passage", "portal", "r1", "r2",
   "can_climb", "can_enter", "can_leave", "auto", "noflee", "searchable",
   "bashed", "bashproof", "nomob", "window", "can_look", "isbolt", "bolted",
   "r29", "r30", "r31"
};

const char *const sec_flags[] = {
   "inside", "city", "field", "forest", "hills", "mountain", "water_swim",
   "water_noswim", "underwater", "air", "desert", "dunno", "oceanfloor",
   "underground", "lava", "swamp", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16"
};

const char *const r_flags[] = {
   "dark", "death", "nomob", "indoors", "house", "neutral", "chaotic",
   "nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
   "donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
   "nosummon", "noastral", "teleport", "teleshowdesc", "nofloor",
   "nosupplicate", "arena", "nomissile", "noyell", "noquit", "prototype", "dnd",
   "_track_", "nowhere", "notrack"
};

const char *const o_flags[] = {
   "glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
   "antigood", "antievil", "antineutral", "noremove", "inventory",
   "antimage", "antithief", "antiwarrior", "anticleric", "organic", "metal",
   "donation", "clanobject", "clancorpse", "antivampire", "antidruid",
   "hidden", "poisoned", "covering", "deathrot", "buried", "prototype",
   "nolocate", "groundrot", "lootable", "personal", "multi_invoke", "enchanted",
   "permanent", "nofill", "deathdrop", "skinned"
};

/* Compiler says these aren't in use, so for now they'll be blocked out.
char *const mag_flags[] = {
   "returning", "backstabber", "bane", "loyal", "haste", "drain",
   "lightning_blade"
};
*/

const char *const w_flags[] = {
   "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
   "shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
   "missile", "back", "face", "ankle", "r4", "r5", "r6",
   "r7", "r8", "r9", "r10", "r11", "r12", "r13"
};

const char *const item_w_flags[] = {
   "take", "finger", "finger", "neck", "neck", "body", "head", "legs", "feet",
   "hands", "arms", "shield", "about", "waist", "wrist", "wrist", "wield",
   "hold", "dual", "ears", "eyes", "missile", "back", "face", "ankle", "ankle",
   "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13"
};

const char *const area_flags[] = {
   "nopkill", "freekill", "noteleport", "spelllimit", "prototype", "hidden",
   "silence", "noportalin", "noportalout", "nomagic", "noastral",
   "nowhere", "r12", "r13", "r14", "r15", "r16", "r17",
   "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

const char *const o_types[] = {
   "none", "light", "scroll", "wand", "staff", "weapon", "_fireweapon", "_missile",
   "treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
   "container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
   "corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
   "scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
   "lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
   "map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
   "fuel", "puddle", "journal", "missileweapon", "projectile", "quiver", "shovel",
   "salve", "cook", "keyring", "odor", "chance", "piece", "housekey", "mix"
};

const char *const a_types[] = {
   "none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
   "sex", "class", "level", "age", "height", "weight", "mana", "hit", "move",
   "gold", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
   "save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
   "immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
   "steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
   "search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
   "grip", "scribe", "brew", "wearspell", "removespell", "emotion", "mentalstate",
   "stripsn", "remove", "dig", "full", "thirst", "drunk", "blood", "cook",
   "recurringspell", "contagious", "xaffected", "odor", "roomflag", "sectortype",
   "roomlight", "televnum", "teledelay"
};

const char *const a_flags[] = {
   "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
   "detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
   "_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
   "charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
   "scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
   "berserk", "aqua_breath", "recurringspell", "contagious", "acidmist",
   "venomshield", "grapple"
};

const char *const act_flags[] = {
   "npc", "sentinel", "scavenger", "stopscript", "nosteal", "aggressive", "stayarea",
   "wimpy", "pet", "train", "practice", "immortal", "deadly", "polyself",
   "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted",
   "scholar", "secretive", "hardhat", "mobinvis", "noassist", "autonomous",
   "pacifist", "noattack", "annoying", "statshield", "prototype"
};

const char *const pc_flags[] = {
   "r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
/* changed "r8" to "" so players on watch can't see it  -- Gorog */
   "nosummon", "pager", "notitled", "groupwho", "diagnose", "highgag", "",
   "nstart", "dnd", "idle", "hints", "beckon", "nobeckon", "nodesc", "nobio",
   "nohomepage", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

const char *const plr_flags[] = {
   "npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
   "outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
   "wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
   "deny", "freeze", "thief", "killer", "litterbug", "ansi", "rip", "nice",
   "flee", "autogold", "automap", "afk", "invisprompt", "compass"
};

const char *const trap_flags[] = {
   "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
   "unlock", "north", "south", "east", "west", "up", "down", "examine",
   "northeast", "northwest", "southeast", "southwest", "r6", "r7", "r8",
   "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

const char *const cmd_flags[] = {
   "possessed", "polymorphed", "watch", "retired", "noabort", "r5", "r6", "r7", "r8",
   "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
   "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
   "r31"
};

const char *const wear_locs[] = {
   "light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
   "feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
   "wield", "hold", "dual_wield", "ears", "eyes", "missile_wield", "back",
   "face", "ankle1", "ankle2"
};

const char *const ris_flags[] = {
   "fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
   "poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
   "plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "r1", "r2", "r3",
   "r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

const char *const trig_flags[] = {
   "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
   "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
   "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
   "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
   "showroomdesc", "autoreturn", "r2", "r3"
};

const char *const part_flags[] = {
   "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
   "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
   "tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
   "sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
   "shell", "r2"
};

const char *const attack_flags[] = {
   "bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
   "gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
   "acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
   "blindness", "causeserious", "earthquake", "causecritical", "curse",
   "flamestrike", "harm", "fireball", "colorspray", "weaken", "spiralblast"
};

const char *const defense_flags[] = {
   "parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
   "dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
   "shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
   "monsum4", "disarm", "iceshield", "grip", "truesight", "acidmist", "venomshield"
};

const char *const npc_position[] = {
   "dead", "mortal", "incapacitated", "stunned", "sleeping",
   "resting", "sitting", "berserk", "aggressive", "fighting", "defensive",
   "evasive", "standing", "mounted", "shove", "drag"
};

const char *const npc_sex[] = {
   "neuter", "male", "female"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 * 
 */
const char *const mprog_flags[] = {
   "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
   "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
   "look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
   "speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use",
   "sell", "tell", "command", "login", "void", "greetinfight", "imminfo",
   "load"
};

const char *flag_string( int bitvector, const char *const flagarray[] )
{
   static char buf[MAX_STRING_LENGTH];
   int x;

   buf[0] = '\0';
   for( x = 0; x < 32; ++x )
      if( IS_SET( bitvector, 1U << x ) )
      {
         mudstrlcat( buf, flagarray[x], MAX_STRING_LENGTH );
         /*
          * don't catenate a blank if the last char is blank  --Gorog 
          */
         if( buf[0] != '\0' && ' ' != buf[strlen( buf ) - 1] )
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );
      }
   if( ( x = strlen( buf ) ) > 0 )
      buf[--x] = '\0';

   return buf;
}

const char *ext_flag_string( EXT_BV * bitvector, const char *const flagarray[] )
{
   static char buf[MAX_STRING_LENGTH];
   int x, known = MAX_BITS;

   if( flagarray == r_flags )            known = sizeof(r_flags) / sizeof(r_flags[0]);
   else if( flagarray == o_flags )       known = sizeof(o_flags) / sizeof(o_flags[0]);
   else if( flagarray == act_flags )     known = sizeof(act_flags) / sizeof(act_flags[0]);
   else if( flagarray == a_flags )       known = sizeof(a_flags) / sizeof(a_flags[0]);
   else if( flagarray == attack_flags )  known = sizeof(attack_flags) / sizeof(attack_flags[0]);
   else if( flagarray == defense_flags ) known = sizeof(defense_flags) / sizeof(defense_flags[0]);

   buf[0] = '\0';
   for( x = 0; x < MAX_BITS; ++x )
      if( xIS_SET( *bitvector, x ) )
      {
         if( x < known )
         {
            mudstrlcat( buf, flagarray[x], MAX_STRING_LENGTH );
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );
         }
      }
   if( ( x = strlen( buf ) ) > 0 )
      buf[--x] = '\0';

   return buf;
}

bool can_rmodify( CHAR_DATA * ch, ROOM_INDEX_DATA * room )
{
   int vnum = room->vnum;
   AREA_DATA *pArea;

   if( IS_NPC( ch ) )
      return FALSE;
   if( get_trust( ch ) >= sysdata.level_modify_proto )
      return TRUE;
   if( !xIS_SET( room->room_flags, ROOM_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to modify this room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( vnum >= pArea->low_r_vnum && vnum <= pArea->hi_r_vnum )
      return TRUE;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return FALSE;
}

bool can_omodify( CHAR_DATA * ch, OBJ_DATA * obj )
{
   int vnum = obj->pIndexData->vnum;
   AREA_DATA *pArea;

   if( IS_NPC( ch ) )
      return FALSE;
   if( get_trust( ch ) >= sysdata.level_modify_proto )
      return TRUE;
   if( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to modify this object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum )
      return TRUE;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return FALSE;
}

bool can_oedit( CHAR_DATA * ch, OBJ_INDEX_DATA * obj )
{
   int vnum = obj->vnum;
   AREA_DATA *pArea;

   if( IS_NPC( ch ) )
      return FALSE;
   if( get_trust( ch ) >= LEVEL_GOD )
      return TRUE;
   if( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to modify this object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum )
      return TRUE;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return FALSE;
}

bool can_mmodify( CHAR_DATA * ch, CHAR_DATA * mob )
{
   int vnum;
   AREA_DATA *pArea;

   if( mob == ch )
      return TRUE;

   if( !IS_NPC( mob ) )
   {
      if( get_trust( ch ) >= sysdata.level_modify_proto && get_trust( ch ) > get_trust( mob ) )
         return TRUE;
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }

   vnum = mob->pIndexData->vnum;

   if( IS_NPC( ch ) )
      return FALSE;
   if( get_trust( ch ) >= sysdata.level_modify_proto )
      return TRUE;
   if( !xIS_SET( mob->act, ACT_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to modify this mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum )
      return TRUE;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mobile is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return FALSE;
}

bool can_medit( CHAR_DATA * ch, MOB_INDEX_DATA * mob )
{
   int vnum = mob->vnum;
   AREA_DATA *pArea;

   if( IS_NPC( ch ) )
      return FALSE;
   if( get_trust( ch ) >= LEVEL_GOD )
      return TRUE;
   if( !xIS_SET( mob->act, ACT_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify this mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to modify this mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return FALSE;
   }
   if( vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum )
      return TRUE;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mobile is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return FALSE;
}

int get_otype( const char *type )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( o_types ) / sizeof( o_types[0] ) ); x++ )
      if( !str_cmp( type, o_types[x] ) )
         return x;
   return -1;
}

int get_aflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( a_flags ) / sizeof( a_flags[0] ) ); x++ )
      if( !str_cmp( flag, a_flags[x] ) )
         return x;
   return -1;
}

int get_trapflag( const char *flag )
{
   size_t x;

   for( x = 0; x < ( sizeof( trap_flags ) / sizeof( trap_flags[0] ) ); x++ )
      if( !str_cmp( flag, trap_flags[x] ) )
         return x;
   return -1;
}

int get_atype( const char *type )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( a_types ) / sizeof( a_types[0] ) ); x++ )
      if( !str_cmp( type, a_types[x] ) )
         return x;
   return -1;
}

int get_npc_class( const char *Class )
{
   unsigned int x;

   for( x = 0; x < MAX_NPC_CLASS; x++ )
      if( !str_cmp( Class, npc_class[x] ) )
         return x;
   return -1;
}

int get_npc_race( const char *type )
{
   unsigned int x;

   for( x = 0; x < MAX_NPC_RACE; x++ )
      if( !str_cmp( type, npc_race[x] ) )
         return x;
   return -1;
}

int get_pc_class( const char *Class )
{
   int x;

   for( x = 0; x < MAX_PC_CLASS; x++ )
      if( !str_cmp( class_table[x]->who_name, Class ) )
         return x;
   return -1;
}

int get_pc_race( const char *type )
{
   int i;

   for( i = 0; i < MAX_PC_RACE; i++ )
      if( !str_cmp( type, race_table[i]->race_name ) )
         return i;
   return -1;
}

int get_wearloc( const char *type )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( wear_locs ) / sizeof( wear_locs[0] ) ); x++ )
      if( !str_cmp( type, wear_locs[x] ) )
         return x;
   return -1;
}

int get_secflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( sec_flags ) / sizeof( sec_flags[0] ) ); x++ )
      if( !str_cmp( flag, sec_flags[x] ) )
         return x;
   return -1;
}

int get_exflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( ex_flags ) / sizeof( ex_flags[0] ) ); x++ )
      if( !str_cmp( flag, ex_flags[x] ) )
         return x;
   return -1;
}

int get_pulltype( const char *type )
{
   unsigned int x;

   if( !str_cmp( type, "none" ) || !str_cmp( type, "clear" ) )
      return 0;

   for( x = 0; x < ( sizeof( ex_pmisc ) / sizeof( ex_pmisc[0] ) ); x++ )
      if( !str_cmp( type, ex_pmisc[x] ) )
         return x;

   for( x = 0; x < ( sizeof( ex_pwater ) / sizeof( ex_pwater[0] ) ); x++ )
      if( !str_cmp( type, ex_pwater[x] ) )
         return x + PT_WATER;
   for( x = 0; x < ( sizeof( ex_pair ) / sizeof( ex_pair[0] ) ); x++ )
      if( !str_cmp( type, ex_pair[x] ) )
         return x + PT_AIR;
   for( x = 0; x < ( sizeof( ex_pearth ) / sizeof( ex_pearth[0] ) ); x++ )
      if( !str_cmp( type, ex_pearth[x] ) )
         return x + PT_EARTH;
   for( x = 0; x < ( sizeof( ex_pfire ) / sizeof( ex_pfire[0] ) ); x++ )
      if( !str_cmp( type, ex_pfire[x] ) )
         return x + PT_FIRE;
   return -1;
}

int get_rflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( r_flags ) / sizeof( r_flags[0] ) ); x++ )
      if( !str_cmp( flag, r_flags[x] ) )
         return x;
   return -1;
}

int get_mpflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( mprog_flags ) / sizeof( mprog_flags[0] ) ); x++ )
      if( !str_cmp( flag, mprog_flags[x] ) )
         return x;
   return -1;
}

int get_oflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( o_flags ) / sizeof( o_flags[0] ) ); x++ )
      if( !str_cmp( flag, o_flags[x] ) )
         return x;
   return -1;
}

int get_areaflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( area_flags ) / sizeof( area_flags[0] ) ); x++ )
      if( !str_cmp( flag, area_flags[x] ) )
         return x;
   return -1;
}

int get_wflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( w_flags ) / sizeof( w_flags[0] ) ); x++ )
      if( !str_cmp( flag, w_flags[x] ) )
         return x;
   return -1;
}

int get_actflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( act_flags ) / sizeof( act_flags[0] ) ); x++ )
      if( !str_cmp( flag, act_flags[x] ) )
         return x;
   return -1;
}

int get_pcflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( pc_flags ) / sizeof( pc_flags[0] ) ); x++ )
      if( !str_cmp( flag, pc_flags[x] ) )
         return x;
   return -1;
}

int get_plrflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( plr_flags ) / sizeof( plr_flags[0] ) ); x++ )
      if( !str_cmp( flag, plr_flags[x] ) )
         return x;
   return -1;
}

int get_risflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( ris_flags ) / sizeof( ris_flags[0] ) ); x++ )
      if( !str_cmp( flag, ris_flags[x] ) )
         return x;
   return -1;
}

/*
 * For use with cedit --Shaddai
 */
int get_cmdflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( cmd_flags ) / sizeof( cmd_flags[0] ) ); x++ )
      if( !str_cmp( flag, cmd_flags[x] ) )
         return x;
   return -1;
}

int get_trigflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( trig_flags ) / sizeof( trig_flags[0] ) ); x++ )
      if( !str_cmp( flag, trig_flags[x] ) )
         return x;
   return -1;
}

int get_partflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( part_flags ) / sizeof( part_flags[0] ) ); x++ )
      if( !str_cmp( flag, part_flags[x] ) )
         return x;
   return -1;
}

int get_attackflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( attack_flags ) / sizeof( attack_flags[0] ) ); x++ )
      if( !str_cmp( flag, attack_flags[x] ) )
         return x;
   return -1;
}

int get_defenseflag( const char *flag )
{
   unsigned int x;

   for( x = 0; x < ( sizeof( defense_flags ) / sizeof( defense_flags[0] ) ); x++ )
      if( !str_cmp( flag, defense_flags[x] ) )
         return x;
   return -1;
}

int get_langflag( const char *flag )
{
   unsigned int x;

   for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      if( !str_cmp( flag, lang_names[x] ) )
         return lang_array[x];
   return LANG_UNKNOWN;
}

int get_langnum( const char *flag )
{
   unsigned int x;

   for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      if( !str_cmp( flag, lang_names[x] ) )
         return x;
   return -1;
}

int get_npc_position( const char *position )
{
   size_t x;

   for( x = 0; x <= POS_DRAG; x++ )
      if( !str_cmp( position, npc_position[x] ) )
         return x;
   return -1;
}

int get_npc_sex( const char *sex )
{
   size_t x;

   for( x = 0; x <= SEX_FEMALE; x++ )
      if( !str_cmp( sex, npc_sex[x] ) )
         return x;
   return -1;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr( const char *str )
{
   static char newstr[MAX_STRING_LENGTH];
   int i, j;

   if( !str || str[0] == '\0' )
   {
      newstr[0] = '\0';
      return newstr;
   }

   for( i = j = 0; str[i] != '\0'; i++ )
      if( str[i] != '\r' )
      {
         newstr[j++] = str[i];
      }
   newstr[j] = '\0';
   return newstr;
}

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
   int len;
   char last;
   char *strptr;

   strptr = str;

   len = strlen( str );
   if( len )
      last = strptr[len - 1];
   else
      last = '\0';

   for( ; *str != '\0'; str++ )
   {
      if( *str == '~' )
         *str = '-';
   }
   if( len )
      strptr[len - 1] = last;

   return;
}


void start_editing( CHAR_DATA * ch, const char *data )
{
   EDITOR_DATA *edit;
   short lines, size, lpos;
   char c;

   if( !ch->desc )
   {
      bug( "%s", "Fatal: start_editing: no desc" );
      return;
   }
   if( ch->substate == SUB_RESTRICTED )
      bug( "%s", "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED" );

   set_char_color( AT_GREEN, ch );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Begin entering your text now (/? = help /s = save /c = clear /l = list)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   send_to_char( "-----------------------------------------------------------------------\r\n> ", ch );
   if( ch->editor )
      stop_editing( ch );

   CREATE( edit, EDITOR_DATA, 1 );
   edit->numlines = 0;
   edit->on_line = 0;
   edit->size = 0;
   size = 0;
   lpos = 0;
   lines = 0;
   if( !data )
      bug( "%s", "editor: data is NULL!\r\n" );
   else
      for( ;; )
      {
         c = data[size++];
         if( c == '\0' )
         {
            edit->line[lines][lpos] = '\0';
            break;
         }
         else if( c == '\r' );
         else if( c == '\n' || lpos > 78 )
         {
            edit->line[lines][lpos] = '\0';
            ++lines;
            lpos = 0;
         }
         else
            edit->line[lines][lpos++] = c;
         if( lines >= 49 || size > 4096 )
         {
            edit->line[lines][lpos] = '\0';
            break;
         }
      }
   if( lpos > 0 && lpos < 78 && lines < 49 )
   {
      edit->line[lines][lpos] = '~';
      edit->line[lines][lpos + 1] = '\0';
      ++lines;
      lpos = 0;
   }
   edit->numlines = lines;
   edit->size = size;
   edit->on_line = lines;
   ch->editor = edit;
   ch->desc->connected = CON_EDITING;
}

char *copy_buffer_nohash( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char tmp[100];
   short x, len;

   if( !ch )
   {
      bug( "%s: null ch", __func__ );
      return str_dup( "" );
   }

   if( !ch->editor )
   {
      bug( "%s: null editor", __func__ );
      return str_dup( "" );
   }

   buf[0] = '\0';
   for( x = 0; x < ch->editor->numlines; x++ )
   {
      mudstrlcpy( tmp, ch->editor->line[x], 100 );
      len = strlen( tmp );
      if( tmp[len - 1] == '~' )
         tmp[len - 1] = '\0';
      else
         mudstrlcat( tmp, "\n", 100 );
      smash_tilde( tmp );
      mudstrlcat( buf, tmp, MAX_STRING_LENGTH );
   }
   return str_dup( buf );
}

const char *copy_buffer( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char tmp[100];
   short x, len;

   if( !ch )
   {
      bug( "%s: null ch", __func__ );
      return STRALLOC( "" );
   }

   if( !ch->editor )
   {
      bug( "%s: null editor", __func__ );
      return STRALLOC( "" );
   }

   buf[0] = '\0';
   for( x = 0; x < ch->editor->numlines; x++ )
   {
      mudstrlcpy( tmp, ch->editor->line[x], 100 );
      len = strlen( tmp );
      if( tmp[len - 1] == '~' )
         tmp[len - 1] = '\0';
      else
         mudstrlcat( tmp, "\n", 100 );
      smash_tilde( tmp );
      mudstrlcat( buf, tmp, MAX_STRING_LENGTH );
   }
   return STRALLOC( buf );
}

void stop_editing( CHAR_DATA * ch )
{
   set_char_color( AT_PLAIN, ch );
   DISPOSE( ch->editor );
   ch->editor = NULL;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   ch->dest_buf = NULL;
   ch->spare_ptr = NULL;
   ch->substate = SUB_NONE;
   if( !ch->desc )
   {
      bug( "%s", "Fatal: stop_editing: no desc" );
      return;
   }
   ch->desc->connected = CON_PLAYING;
}

void do_goto( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location, *in_room;
   CHAR_DATA *fch, *fch_next, *victim;
   AREA_DATA *pArea;
   int vnum;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Goto where?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !is_number( arg ) && ( fch = get_char_world( ch, arg ) ) )
   {
      if( !IS_NPC( fch ) && get_trust( ch ) < get_trust( fch ) && IS_SET( fch->pcdata->flags, PCFLAG_DND ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Sorry. %s does not wish to be disturbed.", fch->name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your DND flag just foiled %s's goto command.", ch->name); act(AT_WHITE, "$t", fch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      vnum = atoi( arg );
      if( vnum < 0 || get_room_index( vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot find that..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( get_trust( ch ) < LEVEL_CREATOR || vnum < 1 || IS_NPC( ch ) || !ch->pcdata->area )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such location."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( get_trust( ch ) < sysdata.level_modify_proto )
      {
         if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to create rooms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room is not within your assigned range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      location = make_room( vnum, ch->pcdata->area );
      if( !location )
      {
         bug( "%s", "Goto: make_room failed" );
         return;
      }
      set_char_color( AT_WHITE, ch );
      send_to_char( "Waving your hand, you form order from swirling chaos,\r\nand step into a new reality...\r\n", ch );
   }

   if( ( victim = room_is_dnd( ch, location ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "That room is \"do not disturb\" right now."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your DND flag just foiled %s's goto command.", ch->name); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < sysdata.level_override_private )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room is private right now."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Overriding private flag!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   }

   in_room = ch->in_room;
   if( ch->fighting )
      stop_fighting( ch, TRUE );

   if( !xIS_SET( ch->act, PLR_WIZINVIS ) )
      act( AT_WHITE, "$n $T", ch, NULL,
           ( ch->pcdata
             && ch->pcdata->bamfout[0] != '\0' ) ? ch->pcdata->bamfout : ( char * )"leaves in a swirling mist.", TO_ROOM );

   ch->regoto = ch->in_room->vnum;
   char_from_room( ch );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, location );
   }
   char_to_room( ch, location );

   if( !xIS_SET( ch->act, PLR_WIZINVIS ) )
      act( AT_WHITE, "$n $T", ch, NULL,
           ( ch->pcdata
             && ch->pcdata->bamfin[0] != '\0' ) ? ch->pcdata->bamfin : ( char * )"appears in a swirling mist.", TO_ROOM );

   do_look( ch, "auto" );

   if( ch->in_room == in_room )
      return;
   for( fch = in_room->first_person; fch; fch = fch_next )
   {
      fch_next = fch->next_in_room;
      if( fch->master == ch && IS_IMMORTAL( fch ) )
      {
         act( AT_WHITE, "You follow $N.", fch, NULL, ch, TO_CHAR );
         do_goto( fch, argument );
      }
/* Experimental change by Gorog so imm's personal mobs follow them */
      else if( IS_NPC( fch ) && fch->master == ch )
      {
         char_from_room( fch );
         char_to_room( fch, location );
      }
   }
   return;
}

void do_mset( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char outbuf[MAX_STRING_LENGTH];
   int num, size, plus;
   int v2;
   char char1, char2;
   CHAR_DATA *victim;
   int value;
   int minattr, maxattr;
   bool lockvictim;
   const char *origarg = argument;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't mset"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_MOB_DESC:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s", "do_mset: sub_mob_desc: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
         }
         victim = ( CHAR_DATA * ) ch->dest_buf;
         if( char_died( victim ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your victim died!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            stop_editing( ch );
            return;
         }
         STRFREE( victim->description );
         victim->description = copy_buffer( ch );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
         }
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
   }

   victim = NULL;
   lockvictim = FALSE;
   smash_tilde( argument );

   if( ch->substate == SUB_REPEATCMD )
   {
      victim = ( CHAR_DATA * ) ch->dest_buf;

      if( !victim )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your victim died!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         argument = "done";
      }

      if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
      {
         if( victim )
            do_mstat( ch, victim->name );
         else
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No victim selected.  Type '?' for help."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
      {
         if( ch->dest_buf )
            RelDestroy( relMSET_ON, ch, ch->dest_buf );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mset mode off."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         return;
      }
   }
   if( victim )
   {
      lockvictim = TRUE;
      mudstrlcpy( arg1, victim->name, MAX_INPUT_LENGTH );
      argument = one_argument( argument, arg2 );
      mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
   }
   else
   {
      lockvictim = FALSE;
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
   }

   if( !str_cmp( arg1, "on" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: mset <victim|vnum> on."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( arg1[0] == '\0' || ( arg2[0] == '\0' && ch->substate != SUB_REPEATCMD ) || !str_cmp( arg1, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
      {
         if( victim )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: <victim> <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: mset <victim> <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Field being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  str int wis dex con cha lck sex class"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  gold hp mana move practice align race"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  hitroll damroll armor affected level"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  thirst drunk full blood flags"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  pos defpos part (see BODYPARTS)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  resistant immune susceptible (see RIS)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  attack defense numattacks"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  speaking speaks (see LANGUAGES)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  name short long description title spec clan"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  council quest qp qpa favor deity"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For editing index/prototype mobiles:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  hitnumdie hitsizedie hitplus (hit points)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  damnumdie damsizedie damplus (damage roll)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "To toggle area flag: aloaded"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "To toggle pkill flag: pkill"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !victim && get_trust( ch ) < LEVEL_GOD )
   {
      if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !victim )
   {
      if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No one like that in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->dest_buf = NULL;
      return;
   }
   if( get_trust( ch ) < LEVEL_GREATER && IS_NPC( victim ) && xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->dest_buf = NULL;
      return;
   }
   if( lockvictim )
      ch->dest_buf = victim;

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

   if( !str_cmp( arg2, "on" ) )
   {
      CHECK_SUBRESTRICTED( ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Mset mode on. (Editing %s).", victim->name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = victim;
      if( ch->pcdata )
      {
         if( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
         if( IS_NPC( victim ) )
            snprintf( buf, MAX_STRING_LENGTH, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum );
         else
            snprintf( buf, MAX_STRING_LENGTH, "<&CMset &W%s&w> %%i", victim->name );
         ch->pcdata->subprompt = STRALLOC( buf );
      }
      RelCreate( relMSET_ON, ch, victim );
      return;
   }
   value = is_number( arg3 ) ? atoi( arg3 ) : -1;

   if( atoi( arg3 ) < -1 && value == -1 )
      value = atoi( arg3 );

   if( !str_cmp( arg2, "str" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Strength range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_str = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_str = value;
      return;
   }

   if( !str_cmp( arg2, "int" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Intelligence range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_int = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_int = value;
      return;
   }

   if( !str_cmp( arg2, "wis" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Wisdom range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_wis = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_wis = value;
      return;
   }

   if( !str_cmp( arg2, "dex" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Dexterity range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_dex = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_dex = value;
      return;
   }

   if( !str_cmp( arg2, "con" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Constitution range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_con = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_con = value;
      return;
   }

   if( !str_cmp( arg2, "cha" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Charisma range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_cha = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_cha = value;
      return;
   }

   if( !str_cmp( arg2, "lck" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < minattr || value > maxattr )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Luck range is %d to %d.", minattr, maxattr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->perm_lck = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_lck = value;
      return;
   }

   if( !str_cmp( arg2, "sav1" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -30 || value > 30 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving throw range is -30 to 30."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->saving_poison_death = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_poison_death = value;
      return;
   }

   if( !str_cmp( arg2, "sav2" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -30 || value > 30 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving throw range is -30 to 30."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->saving_wand = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_wand = value;
      return;
   }

   if( !str_cmp( arg2, "sav3" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -30 || value > 30 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving throw range is -30 to 30."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->saving_para_petri = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_para_petri = value;
      return;
   }

   if( !str_cmp( arg2, "sav4" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -30 || value > 30 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving throw range is -30 to 30."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->saving_breath = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_breath = value;
      return;
   }

   if( !str_cmp( arg2, "sav5" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -30 || value > 30 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving throw range is -30 to 30."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->saving_spell_staff = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_spell_staff = value;
      return;
   }

   if( !str_cmp( arg2, "sex" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 2 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Sex range is 0 to 2."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->sex = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->sex = value;
      return;
   }

   if( !str_cmp( arg2, "class" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;


      if( IS_NPC( victim ) )
      {
         if( value >= MAX_NPC_CLASS || value < 0 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "NPC Class range is 0 to %d.", MAX_NPC_CLASS - 1); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         victim->Class = value;
         if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->Class = value;
         return;
      }


      if( value < 0 || value >= MAX_CLASS )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Class range is 0 to %d.", MAX_CLASS); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->Class = value;
      return;
   }

   if( !str_cmp( arg2, "race" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( IS_NPC( victim ) )
         value = get_npc_race( arg3 );
      else
         value = get_pc_race( arg3 );

      if( value < 0 )
         value = atoi( arg3 );
      if( !IS_NPC( victim ) && ( value < 0 || value >= MAX_RACE ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Race range is 0 to %d.", MAX_RACE - 1); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) && ( value < 0 || value >= MAX_NPC_RACE ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Race range is 0 to %d.", MAX_NPC_RACE - 1); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->race = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->race = value;
      return;
   }

   if( !str_cmp( arg2, "armor" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -300 || value > 300 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "AC range is -300 to 300."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->armor = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->ac = value;
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on PC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > LEVEL_AVATAR + 5 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Level range is 0 to %d.", LEVEL_AVATAR + 5); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->level = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->level = value;
      return;
   }

   if( !str_cmp( arg2, "numattacks" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on PC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 20 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Attacks range is 0 to 20."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->numattacks = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->numattacks = value;
      return;
   }

   if( !str_cmp( arg2, "gold" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      victim->gold = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->gold = value;
      return;
   }

   if( !str_cmp( arg2, "hitroll" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      victim->hitroll = URANGE( 0, value, 85 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitroll = victim->hitroll;
      return;
   }

   if( !str_cmp( arg2, "damroll" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      victim->damroll = URANGE( 0, value, 65 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damroll = victim->damroll;
      return;
   }

   if( !str_cmp( arg2, "hp" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 1 || value > 32700 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Hp range is 1 to 32,700 hit points."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->max_hit = value;
      return;
   }

   if( !str_cmp( arg2, "mana" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 30000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mana range is 0 to 30,000 mana points."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->max_mana = value;
      return;
   }

   if( !str_cmp( arg2, "move" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 30000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Move range is 0 to 30,000 move points."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->max_move = value;
      return;
   }

   if( !str_cmp( arg2, "practice" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Practice range is 0 to 100 sessions."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->practice = value;
      return;
   }

   if( !str_cmp( arg2, "align" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < -1000 || value > 1000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Alignment range is -1000 to 1000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->alignment = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->alignment = value;
      return;
   }

   if( !str_cmp( arg2, "password" ) )
   {
      char *pwdnew;

      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobs don't have passwords."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( strlen( arg3 ) < 5 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "New password must be at least five characters long."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg3[0] == '!' )
      {
         send_to_char( "New password cannot begin with the '!' character.", ch );
         return;
      }

      pwdnew = sha256_crypt( arg3 );   /* SHA-256 Encryption */

      DISPOSE( victim->pcdata->pwd );
      victim->pcdata->pwd = str_dup( pwdnew );
      if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
         save_char_obj( victim );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Your password has been changed by %s.", ch->name); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "rank" ) )
   {
      if( get_trust( ch ) < LEVEL_GOD )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      smash_tilde( argument );
      DISPOSE( victim->pcdata->rank );
      if( !argument || argument[0] == '\0' || !str_cmp( argument, "none" ) )
         victim->pcdata->rank = str_dup( "" );
      else
         victim->pcdata->rank = str_dup( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "quest" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 500 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The current quest range is 0 to 500."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->quest_number = value;
      return;
   }

   if( !str_cmp( arg2, "qpa" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->quest_accum = value;
      return;
   }

   if( !str_cmp( arg2, "qp" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 5000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The current quest point range is 0 to 5000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->quest_curr = value;
      return;
   }

   if( !str_cmp( arg2, "favor" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < -2500 || value > 2500 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Range is from -2500 to 2500."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->favor = value;
      return;
   }

   if( !str_cmp( arg2, "mentalstate" ) )
   {
      if( value < -100 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value must be in range -100 to +100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->mental_state = value;
      return;
   }

   if( !str_cmp( arg2, "emotion" ) )
   {
      if( value < -100 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Value must be in range -100 to +100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->emotional_state = value;
      return;
   }

   if( !str_cmp( arg2, "thirst" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Thirst range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->condition[COND_THIRST] = value;
      return;
   }

   if( !str_cmp( arg2, "drunk" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Drunk range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
   }

   if( !str_cmp( arg2, "full" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Full range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->condition[COND_FULL] = value;
      return;
   }

   if( !str_cmp( arg2, "blood" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 0 || value > MAX_LEVEL + 10 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Blood range is 0 to %d.", MAX_LEVEL + 10); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->pcdata->condition[COND_BLOODTHIRST] = value;
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on PC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg3[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Names can not be set to an empty string."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      STRFREE( victim->name );
      victim->name = STRALLOC( arg3 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         STRFREE( victim->pIndexData->player_name );
         victim->pIndexData->player_name = QUICKLINK( victim->name );
      }
      return;
   }

   if( !str_cmp( arg2, "minsnoop" ) )
   {
      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( victim->pcdata )
      {
         victim->pcdata->min_snoop = value;
         return;
      }
   }

   if( !str_cmp( arg2, "clan" ) )
   {
      CLAN_DATA *clan;

      if( get_trust( ch ) < LEVEL_GOD )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg3[0] == '\0' )
      {
         /*
          * Crash bug fix, oops guess I should have caught this one :)
          * * But it was early in the morning :P --Shaddai 
          */
         if( victim->pcdata->clan == NULL )
            return;
         /*
          * Added a check on immortals so immortals don't take up
          * * any membership space. --Shaddai
          */
         if( !IS_IMMORTAL( victim ) )
         {
            --victim->pcdata->clan->members;
            if( victim->pcdata->clan->members < 0 )
               victim->pcdata->clan->members = 0;
            remove_roster( victim->pcdata->clan, victim->name );

            if( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
            {
               STRFREE( victim->pcdata->clan->leader );
               victim->pcdata->clan->leader = STRALLOC( "" );
            }
            if( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
            {
               STRFREE( victim->pcdata->clan->number1 );
               victim->pcdata->clan->number1 = STRALLOC( "" );
            }
            if( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
            {
               STRFREE( victim->pcdata->clan->number2 );
               victim->pcdata->clan->number2 = STRALLOC( "" );
            }
            if( !str_cmp( victim->name, victim->pcdata->clan->deity ) )
            {
               STRFREE( victim->pcdata->clan->deity );
               victim->pcdata->clan->deity = STRALLOC( "" );
            }
            save_clan( victim->pcdata->clan );
         }
         remove_roster( victim->pcdata->clan, victim->name );
         STRFREE( victim->pcdata->clan_name );
         victim->pcdata->clan_name = STRALLOC( "" );
         victim->pcdata->clan = NULL;
         save_char_obj( victim );
         return;
      }
      clan = get_clan( arg3 );
      if( !clan )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such clan."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( victim->pcdata->clan != NULL && !IS_IMMORTAL( victim ) )
      {
         --victim->pcdata->clan->members;
         if( victim->pcdata->clan->members < 0 )
            victim->pcdata->clan->members = 0;
         remove_roster( victim->pcdata->clan, victim->name );

         if( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
         {
            STRFREE( victim->pcdata->clan->leader );
            victim->pcdata->clan->leader = STRALLOC( "" );
         }
         if( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
         {
            STRFREE( victim->pcdata->clan->number1 );
            victim->pcdata->clan->number1 = STRALLOC( "" );
         }
         if( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
         {
            STRFREE( victim->pcdata->clan->number2 );
            victim->pcdata->clan->number2 = STRALLOC( "" );
         }
         if( !str_cmp( victim->name, victim->pcdata->clan->deity ) )
         {
            STRFREE( victim->pcdata->clan->deity );
            victim->pcdata->clan->deity = STRALLOC( "" );
         }
         save_clan( victim->pcdata->clan );
      }
      STRFREE( victim->pcdata->clan_name );
      victim->pcdata->clan_name = QUICKLINK( clan->name );
      victim->pcdata->clan = clan;
      if( !IS_IMMORTAL( victim ) )
      {
         add_roster( victim->pcdata->clan, victim->name, victim->Class, victim->level, victim->pcdata->mkills,
                     victim->pcdata->mdeaths );
         ++victim->pcdata->clan->members;
         save_clan( victim->pcdata->clan );
         save_char_obj( victim );
      }
      return;
   }

   if( !str_cmp( arg2, "deity" ) )
   {
      DEITY_DATA *deity;

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg3[0] == '\0' )
      {
         if( victim->pcdata->deity )
         {
            --victim->pcdata->deity->worshippers;
            if( victim->pcdata->deity->worshippers < 0 )
               victim->pcdata->deity->worshippers = 0;
            save_deity( victim->pcdata->deity );
         }
         STRFREE( victim->pcdata->deity_name );
         victim->pcdata->deity = NULL;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Deity removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      deity = get_deity( arg3 );
      if( !deity )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such deity."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( victim->pcdata->deity )
      {
         --victim->pcdata->deity->worshippers;
         if( victim->pcdata->deity->worshippers < 0 )
            victim->pcdata->deity->worshippers = 0;
         save_deity( victim->pcdata->deity );
      }
      STRFREE( victim->pcdata->deity_name );
      victim->pcdata->deity_name = QUICKLINK( deity->name );
      victim->pcdata->deity = deity;
      ++deity->worshippers;
      save_deity( deity );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "council" ) )
   {
      COUNCIL_DATA *council;

      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( arg3[0] == '\0' )
      {
         if( victim->pcdata->council )
         {
            --victim->pcdata->council->members;
            if( victim->pcdata->council->members < 0 )
               victim->pcdata->council->members = 0;

            if( !str_cmp( victim->name, victim->pcdata->council->head ) )
            {
               STRFREE( victim->pcdata->council->head );
               victim->pcdata->council->head = STRALLOC( "" );
            }
            if( victim->pcdata->council->head2 != NULL && !str_cmp( victim->name, victim->pcdata->council->head2 ) )
            {
               STRFREE( victim->pcdata->council->head2 );
               victim->pcdata->council->head2 = NULL;
            }
            save_council( victim->pcdata->council );
         }
         STRFREE( victim->pcdata->council_name );
         victim->pcdata->council_name = STRALLOC( "" );
         victim->pcdata->council = NULL;
         check_switch( victim, FALSE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Removed from council."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      council = get_council( arg3 );
      if( !council )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such council."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( victim->pcdata->council )
      {
         --victim->pcdata->council->members;
         if( victim->pcdata->council->members < 0 )
            victim->pcdata->council->members = 0;

         if( !str_cmp( victim->name, victim->pcdata->council->head ) )
         {
            STRFREE( victim->pcdata->council->head );
            victim->pcdata->council->head = STRALLOC( "" );
         }
         if( victim->pcdata->council->head2 != NULL && !str_cmp( victim->name, victim->pcdata->council->head2 ) )
         {
            STRFREE( victim->pcdata->council->head2 );
            victim->pcdata->council->head2 = NULL;
         }
         save_council( victim->pcdata->council );
      }

      STRFREE( victim->pcdata->council_name );
      victim->pcdata->council_name = QUICKLINK( council->name );
      victim->pcdata->council = council;
      ++council->members;
      save_council( council );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      STRFREE( victim->short_descr );
      victim->short_descr = STRALLOC( arg3 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         STRFREE( victim->pIndexData->short_descr );
         victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
      }
      return;
   }

   if( !str_cmp( arg2, "long" ) )
   {
      STRFREE( victim->long_descr );
      mudstrlcpy( buf, arg3, MAX_STRING_LENGTH );
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
      victim->long_descr = STRALLOC( buf );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         STRFREE( victim->pIndexData->long_descr );
         victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
      }
      return;
   }

   if( !str_cmp( arg2, "description" ) )
   {
      if( arg3[0] )
      {
         STRFREE( victim->description );
         victim->description = STRALLOC( arg3 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
         }
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_MOB_DESC;
      ch->dest_buf = victim;
      start_editing( ch, victim->description );
      return;
   }

   if( !str_cmp( arg2, "title" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on NPC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      set_title( victim, arg3 );
      return;
   }

   if( !str_cmp( arg2, "spec" ) || !str_cmp( arg2, "spec_fun" ) )
   {
      SPEC_FUN *specfun;

      if( !can_mmodify( ch, victim ) )
         return;

      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not on PC's."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !str_cmp( arg3, "none" ) )
      {
         victim->spec_fun = NULL;
         STRFREE( victim->spec_funname );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Special function removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            victim->pIndexData->spec_fun = NULL;
            STRFREE( victim->pIndexData->spec_funname );
         }
         return;
      }

      if( ( specfun = spec_lookup( arg3 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such function."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !validate_spec_fun( arg3 ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s is not a valid spec_fun for mobiles.", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      victim->spec_fun = specfun;
      STRFREE( victim->spec_funname );
      victim->spec_funname = STRALLOC( arg3 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         victim->pIndexData->spec_fun = victim->spec_fun;
         STRFREE( victim->pIndexData->spec_funname );
         victim->pIndexData->spec_funname = STRALLOC( arg3 );
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Victim special function set."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      bool protoflag = FALSE, ftoggle = FALSE;

      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's flags."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <vic> flags <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
         protoflag = TRUE;

      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );

         if( IS_NPC( victim ) )
         {
            value = get_actflag( arg3 );

            if( value < 0 || value >= MAX_BITS )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else if( value == ACT_PROTOTYPE && ch->level < sysdata.level_modify_proto )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot change the prototype flag."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else if( value == ACT_IS_NPC )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "If the NPC flag could be changed, it would cause many problems."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
            {
               ftoggle = TRUE;
               xTOGGLE_BIT( victim->act, value );
            }
         }
         else
         {
            value = get_plrflag( arg3 );

            if( value < 0 || value >= MAX_BITS )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else if( value == ACT_IS_NPC )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "If the NPC flag could be changed, it would cause many problems."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
            {
               ftoggle = TRUE;
               xTOGGLE_BIT( victim->act, value );
            }
         }
      }
      if( ftoggle )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Flags set."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( IS_NPC( victim ) && ( xIS_SET( victim->act, ACT_PROTOTYPE ) || ( value == ACT_PROTOTYPE && protoflag ) ) )
         victim->pIndexData->act = victim->act;
      return;
   }

   if( !str_cmp( arg2, "affected" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's flags."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> affected <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            xTOGGLE_BIT( victim->affected_by, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->affected_by = victim->affected_by;
      return;
   }

   /*
    * save some more finger-leather for setting RIS stuff
    */
   if( !str_cmp( arg2, "r" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "i" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;


      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ri" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "is" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ris" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's ris."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      snprintf( outbuf, MAX_STRING_LENGTH, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "resistant" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's resistancies."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> resistant <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( victim->resistant, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->resistant = victim->resistant;
      return;
   }

   if( !str_cmp( arg2, "immune" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's immunities."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> immune <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( victim->immune, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->immune = victim->immune;
      return;
   }

   if( !str_cmp( arg2, "susceptible" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's susceptibilities."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> susceptible <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( victim->susceptible, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->susceptible = victim->susceptible;
      return;
   }

   if( !str_cmp( arg2, "part" ) )
   {
      if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's parts."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> part <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_partflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( victim->xflags, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->xflags = victim->xflags;
      return;
   }

   if( !str_cmp( arg2, "attack" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's attacks."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> attack <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_attackflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            xTOGGLE_BIT( victim->attacks, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->attacks = victim->attacks;
      return;
   }

   if( !str_cmp( arg2, "defense" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only modify a mobile's defenses."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> defense <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_defenseflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            xTOGGLE_BIT( victim->defenses, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->defenses = victim->defenses;
      return;
   }

   if( !str_cmp( arg2, "pos" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > POS_STANDING )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Position range is 0 to %d.", POS_STANDING); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->position = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->position = victim->position;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "defpos" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > POS_STANDING )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Position range is 0 to %d.", POS_STANDING); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->defposition = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->defposition = victim->defposition;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * save some finger-leather
    */
   if( !str_cmp( arg2, "hitdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s hitnumdie %d", arg1, num );
      do_mset( ch, outbuf );

      snprintf( outbuf, MAX_STRING_LENGTH, "%s hitsizedie %d", arg1, size );
      do_mset( ch, outbuf );

      snprintf( outbuf, MAX_STRING_LENGTH, "%s hitplus %d", arg1, plus );
      do_mset( ch, outbuf );
      return;
   }

   /*
    * save some more finger-leather
    */
   if( !str_cmp( arg2, "damdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;

      sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s damnumdie %d", arg1, num );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s damsizedie %d", arg1, size );
      do_mset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s damplus %d", arg1, plus );
      do_mset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "hitnumdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 30000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Number of hitpoint dice range is 0 to 30000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitnodice = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hitsizedie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 30000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Hitpoint dice size range is 0 to 30000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitsizedice = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hitplus" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 30000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Hitpoint bonus range is 0 to 30000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitplus = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "damnumdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Number of damage dice range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->barenumdie = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damnodice = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "damsizedie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 100 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Damage dice size range is 0 to 100."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->baresizedie = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damsizedice = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "damplus" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( value < 0 || value > 1000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Damage bonus range is 0 to 1000."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      victim->damplus = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damplus = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;

   }

   if( !str_cmp( arg2, "aloaded" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Player Characters only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      /*
       * Make sure they have an area assigned -Druid 
       */
      if( !victim->pcdata->area )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Player does not have an area assigned to them."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_mmodify( ch, victim ) )
         return;

      if( !IS_SET( victim->pcdata->area->status, AREA_LOADED ) )
      {
         SET_BIT( victim->pcdata->area->status, AREA_LOADED );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your area set to LOADED!"); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
         if( ch != victim )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area set to LOADED!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your area set to NOT-LOADED!"); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
         if( ch != victim )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area set to NON-LOADED!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !str_cmp( arg2, "pkill" ) )
   {
      if( IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Player Characters only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !can_mmodify( ch, victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
      {
         REMOVE_BIT( victim->pcdata->flags, PCFLAG_DEADLY );
         xSET_BIT( victim->act, PLR_NICE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are now a NON-PKILL player."); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
         if( ch != victim )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That player is now non-pkill."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
      {
         SET_BIT( victim->pcdata->flags, PCFLAG_DEADLY );
         xREMOVE_BIT( victim->act, PLR_NICE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are now a PKILL player."); act(AT_WHITE, "$t", victim, cMSG, NULL, TO_CHAR);}
         if( ch != victim )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That player is now pkill."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }

      if( victim->pcdata->clan && !IS_IMMORTAL( victim ) )
      {
         remove_roster( victim->pcdata->clan, victim->name );
         if( victim->pcdata->clan->clan_type == CLAN_GUILD )
         {
            int sn;

            for( sn = 0; sn < num_skills; ++sn )
               if( skill_table[sn]->guild == victim->pcdata->clan->Class && skill_table[sn]->name != NULL )
                  victim->pcdata->learned[sn] = 0;
         }
         if( victim->speaking & LANG_CLAN )
            victim->speaking = LANG_COMMON;
         REMOVE_BIT( victim->speaks, LANG_CLAN );
         --victim->pcdata->clan->members;
         if( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
         {
            STRFREE( victim->pcdata->clan->leader );
            victim->pcdata->clan->leader = STRALLOC( "" );
         }
         if( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
         {
            STRFREE( victim->pcdata->clan->number1 );
            victim->pcdata->clan->number1 = STRALLOC( "" );
         }
         if( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
         {
            STRFREE( victim->pcdata->clan->number2 );
            victim->pcdata->clan->number2 = STRALLOC( "" );
         }
         save_clan( victim->pcdata->clan );
         STRFREE( victim->pcdata->clan_name );
         victim->pcdata->clan_name = STRALLOC( "" );
         victim->pcdata->clan = NULL;
      }
      save_char_obj( victim );
      return;
   }

   if( !str_cmp( arg2, "speaks" ) )
   {
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> speaks <language> [language] ..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_langflag( arg3 );
         if( value == LANG_UNKNOWN )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else if( !IS_NPC( victim ) )
         {
            if( !( value &= VALID_LANGS ) )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Players may not know %s.", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               continue;
            }
         }

         v2 = get_langnum( arg3 );
         if( v2 == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( victim->speaks, 1 << v2 );
      }
      if( !IS_NPC( victim ) )
      {
         REMOVE_BIT( victim->speaks, race_table[victim->race]->language );
         if( !knows_language( victim, victim->speaking, victim ) )
            victim->speaking = race_table[victim->race]->language;
      }
      else if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->speaks = victim->speaks;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "speaking" ) )
   {
      if( !IS_NPC( victim ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Players must choose the language they speak themselves."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_mmodify( ch, victim ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: mset <victim> speaking <language> [language]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_langflag( arg3 );
         if( value == LANG_UNKNOWN )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            v2 = get_langnum( arg3 );
            if( v2 == -1 )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown language: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               TOGGLE_BIT( victim->speaking, 1 << v2 );
         }
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->speaking = victim->speaking;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_mset;
   }
   else
      do_mset( ch, "" );
   return;
}

void do_oset( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH], outbuf[MAX_STRING_LENGTH];
   OBJ_DATA *obj, *tmpobj;
   EXTRA_DESCR_DATA *ed;
   bool lockobj;
   const char *origarg = argument;

   int value, tmp;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't oset"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;

      case SUB_OBJ_EXTRA:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s: sub_obj_extra: NULL ch->dest_buf", __func__ );
            ch->substate = SUB_NONE;
            return;
         }
         /*
          * hopefully the object didn't get extracted...
          * if you're REALLY paranoid, you could always go through
          * the object and index-object lists, searching through the
          * extra_descr lists for a matching pointer...
          */
         ed = ( EXTRA_DESCR_DATA * ) ch->dest_buf;
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         tmpobj = ( OBJ_DATA * ) ch->spare_ptr;
         stop_editing( ch );
         ch->dest_buf = tmpobj;
         ch->substate = ch->tempnum;
         return;

      case SUB_OBJ_LONG:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s: sub_obj_long: NULL ch->dest_buf", __func__ );
            ch->substate = SUB_NONE;
            return;
         }
         obj = ( OBJ_DATA * ) ch->dest_buf;
         if( obj && obj_extracted( obj ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your object was extracted!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            stop_editing( ch );
            return;
         }
         STRFREE( obj->description );
         obj->description = copy_buffer( ch );
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         {
            if( can_omodify( ch, obj ) )
            {
               STRFREE( obj->pIndexData->description );
               obj->pIndexData->description = QUICKLINK( obj->description );
            }
         }
         tmpobj = ( OBJ_DATA * ) ch->spare_ptr;
         stop_editing( ch );
         ch->substate = ch->tempnum;
         ch->dest_buf = tmpobj;
         return;
   }

   obj = NULL;
   smash_tilde( argument );

   if( ch->substate == SUB_REPEATCMD )
   {
      obj = ( OBJ_DATA * ) ch->dest_buf;
      if( !obj )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your object was extracted!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         argument = "done";
      }

      if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
      {
         if( obj )
            do_ostat( ch, obj->name );
         else
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No object selected.  Type '?' for help."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
      {

         if( ch->dest_buf )
            RelDestroy( relOSET_ON, ch, ch->dest_buf );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Oset mode off."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         return;
      }
   }

   if( obj )
   {
      lockobj = TRUE;
      mudstrlcpy( arg1, obj->name, MAX_INPUT_LENGTH );
      argument = one_argument( argument, arg2 );
      mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
   }
   else
   {
      lockobj = FALSE;
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
   }

   if( !str_cmp( arg1, "on" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: oset <object|vnum> on."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
      {
         if( obj )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: <object> <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: oset <object> <field>  <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Field being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  flags wear level weight cost rent timer"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  name short long ed rmed actiondesc"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  type value0 value1 value2 value3 value4 value5"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  affect rmaffect layers"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For weapons:             For armor:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  weapontype condition     ac condition"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For scrolls, potions and pills:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  slevel spell1 spell2 spell3"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For wands and staves:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  slevel spell maxcharges charges"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "For containers:          For levers and switches:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  cflags key capacity      tflags"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !obj && get_trust( ch ) < LEVEL_GOD )
   {
      if( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't find that here."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else if( !obj )
   {
      if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is nothing like that in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   if( lockobj )
      ch->dest_buf = obj;

   separate_obj( obj );
   value = atoi( arg3 );

   if( !str_cmp( arg2, "on" ) )
   {
      CHECK_SUBRESTRICTED( ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Oset mode on. (Editing '%s' vnum %d).", obj->name, obj->pIndexData->vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = obj;
      if( ch->pcdata )
      {
         if( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
         snprintf( buf, MAX_STRING_LENGTH, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
         ch->pcdata->subprompt = STRALLOC( buf );
      }
      RelCreate( relOSET_ON, ch, obj );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool proto = FALSE;

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         proto = TRUE;
      if( proto && !can_omodify( ch, obj ) )
         return;
      STRFREE( obj->name );
      obj->name = STRALLOC( arg3 );
      if( proto )
      {
         STRFREE( obj->pIndexData->name );
         obj->pIndexData->name = QUICKLINK( obj->name );
      }
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         if( !can_omodify( ch, obj ) )
            return;
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( arg3 );
         STRFREE( obj->pIndexData->short_descr );
         obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
      }
      else
         /*
          * Feature added by Narn, Apr/96 
          * * If the item is not proto, add the word 'rename' to the keywords
          * * if it is not already there.
          */
      {
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( arg3 );
         if( str_infix( "rename", obj->name ) )
         {
            snprintf( buf, MAX_STRING_LENGTH, "%s %s", obj->name, "rename" );
            STRFREE( obj->name );
            obj->name = STRALLOC( buf );
         }
      }
      return;
   }

   if( !str_cmp( arg2, "long" ) )
   {
      if( arg3[0] )
      {
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         {
            if( !can_omodify( ch, obj ) )
               return;
            STRFREE( obj->description );
            obj->description = STRALLOC( arg3 );
            STRFREE( obj->pIndexData->description );
            obj->pIndexData->description = QUICKLINK( obj->description );
            return;
         }
         STRFREE( obj->description );
         obj->description = STRALLOC( arg3 );
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if( lockobj )
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_LONG;
      ch->dest_buf = obj;
      start_editing( ch, obj->description );
      return;
   }

   if( !str_cmp( arg2, "ed" ) )
   {
      if( arg3[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: oset <object> ed <keywords>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( obj->timer )
      {
         send_to_char( "It's not safe to edit an extra description on an object with a timer.\r\nTurn it off first.\r\n",
                       ch );
         return;
      }
      if( obj->item_type == ITEM_PAPER && get_trust( ch ) < LEVEL_IMPLEMENTOR )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can not add an extra description to a note paper at the moment."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         ed = SetOExtraProto( obj->pIndexData, arg3 );
      else
         ed = SetOExtra( obj, arg3 );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if( lockobj )
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_EXTRA;
      ch->dest_buf = ed;
      start_editing( ch, ed->description );
      return;
   }

   if( !str_cmp( arg2, "rmed" ) )
   {
      if( arg3[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: oset <object> rmed <keywords>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         if( DelOExtraProto( obj->pIndexData, arg3 ) )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Deleted."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( DelOExtra( obj, arg3 ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Deleted."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only oset the name, short and long right now."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[0] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[0] = value;
      return;
   }

   if( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[1] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[1] = value;
      return;
   }

   if( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[2] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         obj->pIndexData->value[2] = value;
         if( obj->item_type == ITEM_WEAPON && value != 0 )
            obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
      }
      return;
   }

   if( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[3] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[3] = value;
      return;
   }

   if( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[4] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[4] = value;
      return;
   }

   if( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[5] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[5] = value;
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> type <type>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      value = get_otype( argument );
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown type: %s", argument); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      obj->item_type = ( short )value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->item_type = obj->item_type;
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> flags <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_oflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            if( value == ITEM_PROTOTYPE
                && get_trust( ch ) < LEVEL_GREATER && !is_name( "protoflag", ch->pcdata->bestowments ) )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot change the prototype flag."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
            {
               xTOGGLE_BIT( obj->extra_flags, value );
               if( value == ITEM_PROTOTYPE )
                  obj->pIndexData->extra_flags = obj->extra_flags;
            }
         }
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->extra_flags = obj->extra_flags;
      return;
   }

   if( !str_cmp( arg2, "wear" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> wear <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_wflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( obj->wear_flags, 1 << value );
      }

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->wear_flags = obj->wear_flags;
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->level = value;

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->level = value;
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->weight = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->weight = value;
      return;
   }

   if( !str_cmp( arg2, "cost" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->cost = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->cost = value;
      return;
   }

   if( !str_cmp( arg2, "rent" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->rent = value;
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Item must have prototype flag to set this value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "layers" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->layers = value;
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Item must have prototype flag to set this value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "timer" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->timer = value;
      return;
   }

   if( !str_cmp( arg2, "actiondesc" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      if( strstr( arg3, "%n" ) || strstr( arg3, "%d" ) || strstr( arg3, "%l" ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Illegal characters!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      STRFREE( obj->action_desc );
      obj->action_desc = STRALLOC( arg3 );
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         STRFREE( obj->pIndexData->action_desc );
         obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
      }
      return;
   }

   /*
    * Crash fix and name support by Shaddai 
    */
   if( !str_cmp( arg2, "affect" ) )
   {
      AFFECT_DATA *paf;
      short loc;
      int bitv;

      if( !can_omodify( ch, obj ) )
         return;
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' || !argument || argument[0] == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> affect <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      loc = get_atype( arg2 );
      if( loc < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown field: %s", arg2); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
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
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               SET_BIT( bitv, 1 << value );
         }
         if( !bitv )
            return;
         value = bitv;
      }
      else
      {
         one_argument( argument, arg3 );
         if( loc == APPLY_WEARSPELL && !is_number( arg3 ) )
         {
            // use NULL as the character because we don't care
            // if anybody knows it; we just want to find it
            value = find_spell( NULL, arg3, FALSE );
            if( value == -1 )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown spell name."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }
         else
            value = atoi( arg3 );
      }
      CREATE( paf, AFFECT_DATA, 1 );
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS( paf->bitvector );
      paf->next = NULL;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         if( loc != APPLY_WEARSPELL && loc != APPLY_REMOVESPELL && loc != APPLY_STRIPSN && loc != APPLY_WEAPONSPELL )
         {
            CHAR_DATA *vch;
            OBJ_DATA *eq;

            for( vch = first_char; vch; vch = vch->next )
            {
               for( eq = vch->first_carrying; eq; eq = eq->next_content )
               {
                  if( eq->pIndexData == obj->pIndexData && eq->wear_loc != WEAR_NONE )
                     affect_modify( vch, paf, TRUE );
               }
            }
         }
         LINK( paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
      }
      else
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "rmaffect" ) )
   {
      AFFECT_DATA *paf;
      short loc, count;

      if( !can_omodify( ch, obj ) )
         return;

      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: oset <object> rmaffect <affect#>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      loc = atoi( argument );
      if( loc < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid number."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      count = 0;

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         OBJ_INDEX_DATA *pObjIndex;

         pObjIndex = obj->pIndexData;
         for( paf = pObjIndex->first_affect; paf; paf = paf->next )
         {
            if( ++count == loc )
            {
               UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
               if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL && paf->location != APPLY_STRIPSN
                   && paf->location != APPLY_WEAPONSPELL )
               {
                  CHAR_DATA *vch;
                  OBJ_DATA *eq;

                  for( vch = first_char; vch; vch = vch->next )
                  {
                     for( eq = vch->first_carrying; eq; eq = eq->next_content )
                     {
                        if( eq->pIndexData == pObjIndex && eq->wear_loc != WEAR_NONE )
                           affect_modify( vch, paf, FALSE );
                     }
                  }
               }
               DISPOSE( paf );
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               --top_affect;
               return;
            }
         }
      }
      else
      {
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
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   /*
    * save some finger-leather
    */
   if( !str_cmp( arg2, "ris" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "r" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "i" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "ri" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "is" ) )
   {
      if( !can_omodify( ch, obj ) )
         return;
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
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
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown weapon type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
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

      case ITEM_TRAP:
         if( !str_cmp( arg2, "trapflags" ) )
         {
            while( argument[0] != '\0' )
            {
               argument = one_argument( argument, arg3 );
               tmp = get_trapflag( arg3 );
               if( tmp >= 0 && tmp < 32 )
                  TOGGLE_BIT( value, 1 << tmp );
               else
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Oset: TRAP: bad flag"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            }
            tmp = 3;
         }
         break;

      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
         if( !str_cmp( arg2, "tflags" ) )
         {
            int tmpval = 0;

            tmp = 0;
            argument = arg3;
            while( argument && argument[0] != '\0' )
            {
               argument = one_argument( argument, arg3 );
               value = get_trigflag( arg3 );
               if( value < 0 || value > 31 )
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Invalid tflag %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               else
                  tmpval += ( 1 << value );
            }
            value = tmpval;
         }
         break;
   }

   if( tmp >= 0 && tmp <= 3 )
   {
      if( !can_omodify( ch, obj ) )
         return;
      obj->value[tmp] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[tmp] = value;
      return;
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_oset;
   }
   else
      do_oset( ch, "" );
   return;
}

/* rdig command by Dracones */
void do_rdig( CHAR_DATA* ch, const char* argument)
{
   char arg[1000];
   char tmpcmd[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location, *ch_location;
   AREA_DATA *pArea;
   EXIT_DATA *xit;
   int vnum, edir;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
      return;

   if( !ch->pcdata->area )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no area file with which to work."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   ch_location = ch->in_room;

   if( !can_rmodify( ch, ch_location ) )
      return;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Dig out a new room or dig into an existing room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: rdig <dir>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   edir = get_dir( arg );

   xit = get_exit( ch_location, edir );

   if( !xit )
   {

      pArea = ch->pcdata->area;

      vnum = pArea->low_r_vnum;

      while( vnum <= pArea->hi_r_vnum && get_room_index( vnum ) != NULL )
      {
         vnum++;
      }

      if( vnum > pArea->hi_r_vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No empty upper rooms could be found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Digging out room %d to the %s.", vnum, arg); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      location = make_room( vnum, pArea );
      if( !location )
      {
         bug( "%s: make_room failed", __func__ );
         return;
      }
      location->area = ch->pcdata->area;

      STRFREE( location->name );
      STRFREE( location->description );
      location->name = STRALLOC( ch_location->name );
      location->description = STRALLOC( ch_location->description );
      location->sector_type = ch_location->sector_type;
      location->room_flags = ch_location->room_flags;

      snprintf( tmpcmd, MAX_INPUT_LENGTH, "bexit %s %d", arg, vnum );
      do_redit( ch, tmpcmd );

   }
   else
   {

      vnum = xit->vnum;
      location = get_room_index( vnum );
      if( !location )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That exit does not lead to a valid room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Digging into room %d to the %s.", vnum, arg); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   }

   /*
    * Below here you may add anything else you wish to be
    * copied into the rdug rooms.
    */

   /*
    * Move while rdigging -- Dracones 
    */
   snprintf( tmpcmd, MAX_INPUT_LENGTH, "%d", vnum );
   do_goto( ch, tmpcmd );

   return;
}

/* rgrid command by Dracones */
void do_rgrid( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   EXIT_DATA *xit;
   ROOM_INDEX_DATA *location, *ch_location, *tmp;
   AREA_DATA *pArea;
   int vnum, maxnum, x, y, z;
   int room_count;
   int room_hold[MAX_RGRID_ROOMS];

   if( IS_NPC( ch ) )
      return set_char_color( AT_PLAIN, ch );

   ch_location = ch->in_room;
   pArea = ch->pcdata ? ch->pcdata->area : NULL;

   if( !pArea )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no assigned area for rgrid."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Create a block of rooms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: rgrid <x> <y> <z>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   x = atoi( arg );
   y = atoi( arg2 );
   z = atoi( arg3 );

   if( x < 1 || y < 1 )
   {

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must specify an x and y of at least 1."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   if( z < 1 )
      z = 1;

   if( x > MAX_RGRID_ROOMS || y > MAX_RGRID_ROOMS / x || z > MAX_RGRID_ROOMS / (x * y) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "The maximum number of rooms this mud can create at once is %d.", MAX_RGRID_ROOMS); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   maxnum = x * y * z;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Attempting to create a block of %d rooms, %d x %d x %d.", maxnum, x, y, z); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

   if( maxnum > MAX_RGRID_ROOMS )
   {

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "The maximum number of rooms this mud can create at once is %d.", MAX_RGRID_ROOMS); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Please try to create a smaller block of rooms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   room_count = 0;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Checking for available rooms..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

   if( maxnum > pArea->hi_r_vnum - pArea->low_r_vnum + 1 )
   {

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't even have that many rooms assigned to you."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
   {

      if( get_room_index( vnum ) == NULL )
         room_count++;

      if( room_count >= maxnum )
         break;
   }

   if( room_count < maxnum )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There aren't enough free rooms in your assigned range!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Enough free rooms were found, creating the rooms..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

   room_count = 0;
   vnum = pArea->low_r_vnum;

   while( room_count < maxnum )
   {

      if( get_room_index( vnum ) == NULL )
      {

         room_hold[room_count++] = vnum;

         location = make_room( vnum, pArea );
         if( !location )
         {
            bug( "%s: make_room failed", __func__ );
            return;
         }

         location->area = ch->pcdata->area;

         location->name = ch_location->name;
         location->description = ch_location->description;
         location->sector_type = ch_location->sector_type;
         location->room_flags = ch_location->room_flags;

      }
      vnum++;
   }

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Rooms created, linking the exits..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

   for( room_count = 1; room_count <= maxnum; room_count++ )
   {

      vnum = room_hold[room_count - 1];

      // Check to see if we can make N exits
      if( room_count % x )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count] );

         xit = make_exit( location, tmp, 0 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;

      }

      // Check to see if we can make S exits
      if( ( room_count - 1 ) % x )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count - 2] );

         xit = make_exit( location, tmp, 2 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;

      }

      // Check to see if we can make E exits
      if( ( room_count - 1 ) % ( x * y ) < x * y - x )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count + x - 1] );

         xit = make_exit( location, tmp, 1 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;

      }

      // Check to see if we can make W exits
      if( ( room_count - 1 ) % ( x * y ) >= x )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count - x - 1] );

         xit = make_exit( location, tmp, 3 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;
      }

      // Check to see if we can make D exits
      if( room_count > x * y )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count - x * y - 1] );

         xit = make_exit( location, tmp, 5 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;
      }

      // Check to see if we can make U exits
      if( room_count <= maxnum - ( x * y ) )
      {

         location = get_room_index( vnum );
         tmp = get_room_index( room_hold[room_count + x * y - 1] );

         xit = make_exit( location, tmp, 4 );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;
      }
   }
   return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir( const char *txt )
{
   int edir;
   char c1, c2;

   if( !str_cmp( txt, "northeast" ) )
      return DIR_NORTHEAST;
   if( !str_cmp( txt, "northwest" ) )
      return DIR_NORTHWEST;
   if( !str_cmp( txt, "southeast" ) )
      return DIR_SOUTHEAST;
   if( !str_cmp( txt, "southwest" ) )
      return DIR_SOUTHWEST;
   if( !str_cmp( txt, "somewhere" ) )
      return 10;

   c1 = txt[0];
   if( c1 == '\0' )
      return 0;
   c2 = txt[1];
   edir = 0;
   switch ( c1 )
   {
      case 'n':
         switch ( c2 )
         {
            default:
               edir = 0;
               break;   /* north */
            case 'e':
               edir = 6;
               break;   /* ne   */
            case 'w':
               edir = 7;
               break;   /* nw   */
         }
         break;
      case '0':
         edir = 0;
         break;   /* north */
      case 'e':
      case '1':
         edir = 1;
         break;   /* east  */
      case 's':
         switch ( c2 )
         {
            default:
               edir = 2;
               break;   /* south */
            case 'e':
               edir = 8;
               break;   /* se   */
            case 'w':
               edir = 9;
               break;   /* sw   */
         }
         break;
      case '2':
         edir = 2;
         break;   /* south */
      case 'w':
      case '3':
         edir = 3;
         break;   /* west  */
      case 'u':
      case '4':
         edir = 4;
         break;   /* up    */
      case 'd':
      case '5':
         edir = 5;
         break;   /* down  */
      case '6':
         edir = 6;
         break;   /* ne   */
      case '7':
         edir = 7;
         break;   /* nw   */
      case '8':
         edir = 8;
         break;   /* se   */
      case '9':
         edir = 9;
         break;   /* sw   */
      case '?':
         edir = 10;
         break;   /* somewhere */
   }
   return edir;
}

/*
 * Crash fix and name support by Shaddai 
 */
void add_room_affect( ROOM_INDEX_DATA * location, CHAR_DATA * ch, bool indexaffect, const char *argument )
{
   AFFECT_DATA *paf;
   char arg2[MAX_INPUT_LENGTH];
   int value, bitv;
   short loc;
   bool found = FALSE;

   argument = one_argument( argument, arg2 );
   if( arg2[0] == '\0' || !argument || argument[0] == '\0' )
   {
      if( !indexaffect )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit affect <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit permaffect <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   loc = get_atype( arg2 );
   if( loc < 1 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown field: %s", arg2); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( loc == APPLY_AFFECT )
   {
      char arg3[MAX_INPUT_LENGTH];

      argument = one_argument( argument, arg3 );
      if( loc == APPLY_AFFECT )
      {
         value = get_aflag( arg3 );

         if( value < 0 || value >= MAX_AFFECTED_BY )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown affect: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            found = TRUE;
      }
   }
   else if( loc == APPLY_RESISTANT || loc == APPLY_IMMUNE || loc == APPLY_SUSCEPTIBLE )
   {
      const char *ris = argument;
      char flag[MAX_INPUT_LENGTH];

      bitv = 0;
      while( ris[0] != '\0' )
      {
         ris = one_argument( ris, flag );
         value = get_risflag( flag );

         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", flag); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            SET_BIT( bitv, 1 << value );
            found = TRUE;
         }
      }
      if( !bitv )
         return;
      value = bitv;
   }
   else if( loc == APPLY_WEAPONSPELL || loc == APPLY_WEARSPELL
            || loc == APPLY_REMOVESPELL || loc == APPLY_STRIPSN || loc == APPLY_RECURRINGSPELL )
   {
      value = skill_lookup( argument );

      if( !IS_VALID_SN( value ) )
         ch_printf( ch, "Invalid spell: %s", argument );
      else
         found = TRUE;
   }
   else
   {
      value = atoi( argument );
      found = TRUE;
   }
   if( !found )
      return;

   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = loc;
   paf->modifier = value;
   xCLEAR_BITS( paf->bitvector );

   if( !indexaffect )
      LINK( paf, location->first_affect, location->last_affect, next, prev );
   else
      LINK( paf, location->first_permaffect, location->last_permaffect, next, prev );
   ++top_affect;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Room affect added."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

   if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL && paf->location != APPLY_STRIPSN )
   {
      CHAR_DATA *vch;

      /*
       * Now actually APPLY the affect to people in the room,
       * otherwise they will be corrupted when they leave the room.
       */
      for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
         affect_modify( vch, paf, TRUE );
   }
   return;
}

void remove_room_affect( ROOM_INDEX_DATA * location, CHAR_DATA * ch, bool indexaffect, const char *argument )
{
   AFFECT_DATA *paf, *paf_next;
   short loc = atoi( argument );

   if( !argument || argument[0] == '\0' )
   {
      if( !indexaffect )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit rmaffect <affect#>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit rmpermaffect <affect#>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( loc < 1 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid number."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   short count = 0;
   if( !indexaffect )
   {
      for( paf = location->first_affect; paf; paf = paf_next )
      {
         paf_next = paf->next;

         if( ++count == loc )
         {
            if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL && paf->location != APPLY_STRIPSN )
            {
               CHAR_DATA *vch;

               /*
                * Now actually remove the affect from people in the room,
                * otherwise they'll be corrupted when they leave.
                */
               for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
                  affect_modify( vch, paf, FALSE );
            }
            UNLINK( paf, location->first_affect, location->last_affect, next, prev );
            DISPOSE( paf );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Room affect removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            --top_affect;
            return;
         }
      }
   }
   else
   {
      for( paf = location->first_permaffect; paf; paf = paf_next )
      {
         paf_next = paf->next;

         if( ++count == loc )
         {
            if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL && paf->location != APPLY_STRIPSN )
            {
               CHAR_DATA *vch;

               /*
                * Now actually remove the affect from people in the room,
                * otherwise they'll be corrupted when they leave.
                */
               for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
                  affect_modify( vch, paf, FALSE );
            }
            UNLINK( paf, location->first_permaffect, location->last_permaffect, next, prev );
            DISPOSE( paf );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Room index affect removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            --top_affect;
            return;
         }
      }
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Room affect not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_rloop( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   AREA_DATA *pArea;
   int Start, End, vnum;

   set_char_color( AT_WHITE, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: rloop <start> <end> redit <arguments>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   Start = atoi( arg1 );
   End = atoi( arg2 );
   if( Start < 1 || End < Start || Start > End || Start == End )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to rloop in."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( Start < pArea->low_r_vnum || Start > pArea->hi_r_vnum || End < pArea->low_r_vnum || End > pArea->hi_r_vnum )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only rloop within your assigned area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( strncmp( argument, "redit ", 6 ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Rloop is restricted to redit commands only."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: rloop <start> <end> redit <arguments>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   original = ch->in_room;
   for( vnum = Start; vnum <= End; vnum++ )
   {
      if( ( location = get_room_index( vnum ) ) == NULL )
         continue;
      char_from_room( ch );
      char_to_room( ch, location );
      interpret( ch, argument );
   }

   char_from_room( ch );
   char_to_room( ch, original );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_redit( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[500];
   char arg3[500];
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA *location, *tmp;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *xit, *texit;
   int value;
   int edir = 0, ekey, evnum;
   const char *origarg = argument;

   set_char_color( AT_PLAIN, ch );
   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_ROOM_DESC:
         location = ( ROOM_INDEX_DATA * ) ch->dest_buf;
         if( !location )
         {
            bug( "%s", "redit: sub_room_desc: NULL ch->dest_buf" );
            location = ch->in_room;
         }
         STRFREE( location->description );
         location->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
      case SUB_ROOM_EXTRA:
         ed = ( EXTRA_DESCR_DATA * ) ch->dest_buf;
         if( !ed )
         {
            bug( "%s", "redit: sub_room_extra: NULL ch->dest_buf" );
            stop_editing( ch );
            return;
         }
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
   }

   location = ch->in_room;

   smash_tilde( argument );
   argument = one_argument( argument, arg );
   if( ch->substate == SUB_REPEATCMD )
   {
      if( arg[0] == '\0' )
      {
         do_rstat( ch, "" );
         return;
      }
      if( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Redit mode off."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         ch->substate = SUB_NONE;
         return;
      }
   }
   if( arg[0] == '\0' || !str_cmp( arg, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: <field> value"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: redit <field> value"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Field being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  name desc ed rmed affect rmaffect permaffect rmpermaffect"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  exit bexit exdesc exflags exname exkey"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  flags sector teledelay televnum tunnel"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  rlist exdistance pulltype pull push"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_rmodify( ch, location ) )
      return;

   if( !str_cmp( arg, "on" ) )
   {
      CHECK_SUBRESTRICTED( ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Redit mode on."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      ch->substate = SUB_REPEATCMD;
      if( ch->pcdata )
      {
         if( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
         ch->pcdata->subprompt = STRALLOC( "<&CRedit &W#%r&w> %i" );
      }
      return;
   }

   if( !str_cmp( arg, "name" ) )
   {
      if( argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the room name.  A very brief single line room description."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit name <Room summary>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      STRFREE( location->name );
      location->name = STRALLOC( argument );
      return;
   }

   if( !str_cmp( arg, "desc" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_DESC;
      ch->dest_buf = location;
      start_editing( ch, location->description );
      return;
   }

   if( !str_cmp( arg, "maxweight" ) )
   {
      location->max_weight = atoi( argument );
      return;
   }

   if( !str_cmp( arg, "tunnel" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the maximum characters allowed in the room at one time. (0 = unlimited)."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit tunnel <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      location->tunnel = URANGE( 0, atoi( argument ), 1000 );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "permaffect" ) || !str_cmp( arg, "indexaffect" ) )
   {
      add_room_affect( location, ch, TRUE, argument );
      return;
   }

   if( !str_cmp( arg, "affect" ) )
   {
      add_room_affect( location, ch, FALSE, argument );
      return;
   }

   if( !str_cmp( arg, "rmpermaffect" ) || !str_cmp( arg, "rmindexaffect" ) )
   {
      remove_room_affect( location, ch, TRUE, argument );
      return;
   }

   if( !str_cmp( arg, "rmaffect" ) )
   {
      remove_room_affect( location, ch, FALSE, argument );
      return;
   }

   if( !str_cmp( arg, "ed" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Create an extra description."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must supply keyword(s)."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      ed = SetRExtra( location, argument );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_EXTRA;
      ch->dest_buf = ed;
      start_editing( ch, ed->description );
      return;
   }

   if( !str_cmp( arg, "rmed" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Remove an extra description."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must supply keyword(s)."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( DelRExtra( location, argument ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Deleted."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "rlist" ) )
   {
      RESET_DATA *pReset;
      char *rbuf;
      short num;

      if( !location->first_reset )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This room has no resets to list."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      num = 0;
      for( pReset = location->first_reset; pReset; pReset = pReset->next )
      {
         num++;
         if( !( rbuf = sprint_reset( pReset, &num ) ) )
            continue;
         send_to_char( rbuf, ch );
      }
      return;
   }

   if( !str_cmp( arg, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Toggle the room flags."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit flags <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_rflag( arg2 );
         if( value < 0 || value >= ROOM_MAX )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg2); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            if( value == ROOM_PROTOTYPE && get_trust( ch ) < LEVEL_GREATER )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot change the prototype flag."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            else
               xTOGGLE_BIT( location->room_flags, value );
         }
      }
      return;
   }

   if( !str_cmp( arg, "teledelay" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the delay of the teleport. (0 = off)."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit teledelay <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      location->tele_delay = atoi( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "televnum" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the vnum of the room to teleport to."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit televnum <vnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      location->tele_vnum = atoi( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "sector" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the sector type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit sector <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      location->sector_type = atoi( argument );
      if( location->sector_type < 0 || location->sector_type >= SECT_MAX )
      {
         location->sector_type = 1;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "exkey" ) )
   {
      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2[0] == '\0' || arg3[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exkey <dir> <key vnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      value = atoi( arg3 );
      if( !xit )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      xit->key = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "exname" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Change or clear exit keywords."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exname <dir> [keywords]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( !xit )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      STRFREE( xit->keyword );
      xit->keyword = STRALLOC( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "exflags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Toggle or display exit flags."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exflags <dir> <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( !xit )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( argument[0] == '\0' )
      {
         snprintf( buf, MAX_STRING_LENGTH, "Flags for exit direction: %d  Keywords: %s  Key: %d\r\n[ ", xit->vdir,
                   xit->keyword, xit->key );
         for( value = 0; value <= MAX_EXFLAG; value++ )
         {
            if( IS_SET( xit->exit_info, 1 << value ) )
            {
               mudstrlcat( buf, ex_flags[value], MAX_STRING_LENGTH );
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
            }
         }
         mudstrlcat( buf, "]\r\n", MAX_STRING_LENGTH );
         send_to_char( buf, ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_exflag( arg2 );
         if( value < 0 || value > MAX_EXFLAG )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg2); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
            TOGGLE_BIT( xit->exit_info, 1 << value );
      }
      return;
   }

   if( !str_cmp( arg, "exit" ) )
   {
      bool addexit, numnotdir;

      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Create, change or remove an exit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exit <dir> [room] [flags] [key] [keywords]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      addexit = numnotdir = FALSE;
      switch ( arg2[0] )
      {
         default:
            edir = get_dir( arg2 );
            break;
         case '+':
            edir = get_dir( arg2 + 1 );
            addexit = TRUE;
            break;
         case '#':
            edir = atoi( arg2 + 1 );
            numnotdir = TRUE;
            break;
      }
      if( arg3[0] == '\0' )
         evnum = 0;
      else
         evnum = atoi( arg3 );
      if( numnotdir )
      {
         if( ( xit = get_exit_num( location, edir ) ) != NULL )
            edir = xit->vdir;
      }
      else
         xit = get_exit( location, edir );
      if( !evnum )
      {
         if( xit )
         {
            extract_exit( location, xit );
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Exit removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( evnum < 1 || evnum > MAX_VNUM )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid room number."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( ( tmp = get_room_index( evnum ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Non-existant room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( addexit || !xit )
      {
         if( numnotdir )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Cannot add an exit by number, sorry."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         if( addexit && xit && get_exit_to( location, edir, tmp->vnum ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is already an exit in that direction leading to that location."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
         xit = make_exit( location, tmp, edir );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;
         act( AT_WHITE, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
      }
      else
         act( AT_WHITE, "Something is different...", ch, NULL, NULL, TO_ROOM );
      if( xit->to_room != tmp )
      {
         xit->to_room = tmp;
         xit->vnum = evnum;
         texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );
         if( texit )
         {
            texit->rexit = xit;
            xit->rexit = texit;
         }
      }
      argument = one_argument( argument, arg3 );
      if( arg3[0] != '\0' )
         xit->exit_info = atoi( arg3 );
      if( argument && argument[0] != '\0' )
      {
         one_argument( argument, arg3 );
         ekey = atoi( arg3 );
         if( ekey != 0 || arg3[0] == '0' )
         {
            argument = one_argument( argument, arg3 );
            xit->key = ekey;
         }
         if( argument && argument[0] != '\0' )
         {
            STRFREE( xit->keyword );
            xit->keyword = STRALLOC( argument );
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Twisted and evil, but works           -Thoric
    * Makes an exit, and the reverse in one shot.
    */
   if( !str_cmp( arg, "bexit" ) )
   {
      EXIT_DATA *nxit, *rxit;
      char tmpcmd[1000];
      ROOM_INDEX_DATA *tmploc;
      int vnum, exnum;
      char rvnum[1000];
      bool numnotdir;

      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Create, change or remove a two-way exit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit bexit <dir> [room] [flags] [key] [keywords]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      numnotdir = FALSE;
      switch ( arg2[0] )
      {
         default:
            edir = get_dir( arg2 );
            break;
         case '#':
            numnotdir = TRUE;
            edir = atoi( arg2 + 1 );
            break;
         case '+':
            edir = get_dir( arg2 + 1 );
            break;
      }
      tmploc = location;
      exnum = edir;
      if( numnotdir )
      {
         if( ( nxit = get_exit_num( tmploc, edir ) ) != NULL )
            edir = nxit->vdir;
      }
      else
         nxit = get_exit( tmploc, edir );
      rxit = NULL;
      vnum = 0;
      rvnum[0] = '\0';
      if( nxit )
      {
         vnum = nxit->vnum;
         if( arg3[0] != '\0' )
            snprintf( rvnum, MAX_INPUT_LENGTH, "%d", tmploc->vnum );
         if( nxit->to_room )
            rxit = get_exit( nxit->to_room, rev_dir[edir] );
         else
            rxit = NULL;
      }
      snprintf( tmpcmd, MAX_INPUT_LENGTH, "exit %s %s %s", arg2, arg3, argument );
      do_redit( ch, tmpcmd );
      if( numnotdir )
         nxit = get_exit_num( tmploc, exnum );
      else
         nxit = get_exit( tmploc, edir );
      if( !rxit && nxit )
      {
         vnum = nxit->vnum;
         if( arg3[0] != '\0' )
            snprintf( rvnum, MAX_INPUT_LENGTH, "%d", tmploc->vnum );
         if( nxit->to_room )
            rxit = get_exit( nxit->to_room, rev_dir[edir] );
         else
            rxit = NULL;
      }
      if( vnum )
      {
         snprintf( tmpcmd, MAX_INPUT_LENGTH, "%d redit exit %d %s %s", vnum, rev_dir[edir], rvnum, argument );
         do_at( ch, tmpcmd );
      }
      return;
   }

   if( !str_cmp( arg, "pulltype" ) || !str_cmp( arg, "pushtype" ) )
   {
      int pt;

      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Set the %s between this room, and the destination room.", arg); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Usage: redit %s <dir> <type>", arg); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         if( ( pt = get_pulltype( argument ) ) == -1 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown pulltype: %s.  (See help PULLTYPES)", argument); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            xit->pulltype = pt;
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "pull" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the 'pull' between this room, and the destination room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit pull <dir> <force (0 to 100)>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->pull = URANGE( -100, atoi( argument ), 100 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "push" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the 'push' away from the destination room in the opposite direction."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit push <dir> <force (0 to 100)>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->pull = URANGE( -100, -( atoi( argument ) ), 100 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "exdistance" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Set the distance (in rooms) between this room, and the destination room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exdistance <dir> [distance]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->distance = URANGE( 1, atoi( argument ), 50 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "exdesc" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Create or clear a description for an exit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: redit exdesc <dir> [description]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         STRFREE( xit->description );
         if( !argument || argument[0] == '\0' )
            xit->description = STRALLOC( "" );
         else
         {
            snprintf( buf, MAX_STRING_LENGTH, "%s\r\n", argument );
            xit->description = STRALLOC( buf );
         }
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No exit in that direction.  Use 'redit exit ...' first."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_redit;
   }
   else
      do_redit( ch, "" );
   return;
}

void do_ocreate( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, cvnum;

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles cannot create."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );

   vnum = is_number( arg ) ? atoi( arg ) : -1;

   if( vnum == -1 || !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage:  ocreate <vnum> [copy vnum] <item name>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( vnum < 1 || vnum > MAX_VNUM )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Vnum out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg2 );
   cvnum = atoi( arg2 );
   if( cvnum != 0 )
      argument = one_argument( argument, arg2 );
   if( cvnum < 1 )
      cvnum = 0;

   if( get_obj_index( vnum ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "An object with that number already exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_NPC( ch ) )
      return;
   if( get_trust( ch ) < LEVEL_LESSER )
   {
      AREA_DATA *pArea;

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to create objects."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That number is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   pObjIndex = make_object( vnum, cvnum, argument );
   if( !pObjIndex )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Error."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      log_string( "do_ocreate: make_object failed." );
      return;
   }
   obj = create_object( pObjIndex, get_trust( ch ) );
   obj_to_char( obj, ch );
   act( AT_WHITE, "$n makes arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
   ch_printf_color( ch,
                    "&YYou make arcane gestures, and open your hands to reveal %s!\r\nObjVnum:  &W%d   &YKeywords:  &W%s\r\n",
                    pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name );
}

void do_mcreate( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *mob;
   int vnum, cvnum;

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobiles cannot create."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   argument = one_argument( argument, arg );

   vnum = is_number( arg ) ? atoi( arg ) : -1;

   if( vnum == -1 || !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage:  mcreate <vnum> [cvnum] <mobile name>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( vnum < 1 || vnum > MAX_VNUM )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Vnum out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   one_argument( argument, arg2 );
   cvnum = atoi( arg2 );
   if( cvnum != 0 )
      argument = one_argument( argument, arg2 );
   if( cvnum < 1 )
      cvnum = 0;

   if( get_mob_index( vnum ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A mobile with that number already exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( IS_NPC( ch ) )
      return;
   if( get_trust( ch ) < LEVEL_LESSER )
   {
      AREA_DATA *pArea;

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to create mobiles."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That number is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   pMobIndex = make_mobile( vnum, cvnum, argument );
   if( !pMobIndex )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Error."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      log_string( "do_mcreate: make_mobile failed." );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, ch->in_room );
   act( AT_WHITE, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
   ch_printf_color( ch,
                    "&YYou wave your arms about, and %s appears at your command!\r\nMobVnum:  &W%d   &YKeywords:  &W%s\r\n",
                    pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name );
}


 //**********************************************************************************************// 
 void do_npc_mcreate(CHAR_DATA* ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *mob;
	int vnum, cvnum;

	argument = one_argument( argument, arg );
	vnum = is_number( arg ) ? atoi( arg ) : -1;

	if( vnum == -1 || !argument || argument[0] == '\0' )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage:  mcreate <vnum> [cvnum] <mobile name>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	if( vnum < 1 || vnum > MAX_VNUM )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Vnum out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	one_argument( argument, arg2 );
	cvnum = atoi( arg2 );
	if( cvnum != 0 )
		argument = one_argument( argument, arg2 );
	if( cvnum < 1 )
		cvnum = 0;

	if( get_mob_index( vnum ) )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A mobile with that number already exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	if( !IS_NPC(ch) && get_trust(ch) < LEVEL_LESSER )
	{
		AREA_DATA *pArea;

		if(!ch->pcdata || !(pArea = ch->pcdata->area))
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must have an assigned area to create mobiles."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
			return;
		}
		if( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That number is not in your allocated range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
			return;
		}
	}

	pMobIndex = make_mobile( vnum, cvnum, argument );
	if( !pMobIndex )
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Error."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		log_string( "do_mcreate: make_mobile failed." );
		return;
	}
	mob = create_mobile(pMobIndex);
	char_to_room(mob, ch->in_room);
}
 //**********************************************************************************************//
 /*
 * Simple but nice and handy line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   EDITOR_DATA *edit;
   char cmd[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   const int max_buf_lines = 60;
   short x, line;
   bool save;

   if( ( d = ch->desc ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( d->connected != CON_EDITING )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      bug( "%s: d->connected != CON_EDITING", __func__ );
      return;
   }

   if( ch->substate <= SUB_PAUSE )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      bug( "%s: illegal ch->substate (%d)", __func__, ch->substate );
      d->connected = CON_PLAYING;
      return;
   }

   if( !ch->editor )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      bug( "%s: null editor", __func__ );
      d->connected = CON_PLAYING;
      return;
   }

   edit = ch->editor;
   save = FALSE;

   if( argument[0] == '/' || argument[0] == '\\' )
   {
      one_argument( argument, cmd );

      if( !str_cmp( cmd + 1, "?" ) )
      {
         send_to_char( "Editing commands\r\n---------------------------------\r\n", ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/l              list buffer"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/c              clear buffer"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/d [line]       delete line"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/g <line>       goto line"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/i <line>       insert line"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/f <format>     format text in buffer"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/r <old> <new>  global replace"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/a              abort editing"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         if( get_trust( ch ) > LEVEL_IMMORTAL )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "/! <command>    execute command (do not use another editing command)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         send_to_char( "/s              save buffer\r\n\r\n> ", ch );
         return;
      }

      if( !str_cmp( cmd + 1, "c" ) )
      {
         memset( edit, '\0', sizeof( EDITOR_DATA ) );
         edit->numlines = 0;
         edit->on_line = 0;
         send_to_char( "Buffer cleared.\r\n> ", ch );
         return;
      }

      if( !str_cmp( cmd + 1, "r" ) )
      {
         char word1[MAX_INPUT_LENGTH];
         char word2[MAX_INPUT_LENGTH];
         const char *sptr;
         char *wptr, *lwptr;
         int count, wordln, word2ln, lineln;

         sptr = one_argument( argument, word1 );
         sptr = one_argument( sptr, word1 );
         sptr = one_argument( sptr, word2 );
         if( word1[0] == '\0' || word2[0] == '\0' )
         {
            send_to_char( "Need word to replace, and replacement.\r\n> ", ch );
            return;
         }
         if( strcmp( word1, word2 ) == 0 )
         {
            send_to_char( "Done.\r\n> ", ch );
            return;
         }
         count = 0;
         wordln = strlen( word1 );
         word2ln = strlen( word2 );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Replacing all occurrences of %s with %s...", word1, word2); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         for( x = 0; x < edit->numlines; x++ )
         {
            lwptr = edit->line[x];
            while( ( wptr = strstr( lwptr, word1 ) ) != NULL )
            {
               ++count;
               lineln = snprintf( buf, MAX_INPUT_LENGTH, "%s%s", word2, wptr + wordln );
               if( lineln + wptr - edit->line[x] > 79 )
                  buf[lineln] = '\0';
               mudstrlcpy( wptr, buf, MAX_STRING_LENGTH );
               lwptr = wptr + word2ln;
            }
         }
         ch_printf( ch, "Found and replaced %d occurrence(s).\r\n> ", count );
         return;
      }

      /*
       * added format command - shogar 
       *
       * This has been redone to be more efficient, and to make format
       * start at beginning of buffer, not whatever line you happened
       * to be on, at the time.   
       */
      if( !str_cmp( cmd + 1, "f" ) )
      {
         char temp_buf[MAX_STRING_LENGTH + max_buf_lines];
         int ep, old_p, end_mark;
         int p = 0;

         pager_printf( ch, "Reformating...\r\n" );

         for( x = 0; x < edit->numlines; x++ )
         {
            strncpy( temp_buf + p, edit->line[x], MAX_STRING_LENGTH + max_buf_lines - p );
            p += strlen( edit->line[x] );
            temp_buf[p] = ' ';
            p++;
         }

         temp_buf[p] = '\0';
         end_mark = p;
         p = 75;
         old_p = 0;
         edit->on_line = 0;
         edit->numlines = 0;

         while( old_p < end_mark )
         {
            while( temp_buf[p] != ' ' && p > old_p )
               p--;

            if( p == old_p )
               p += 75;

            if( p > end_mark )
               p = end_mark;

            ep = 0;
            for( x = old_p; x < p; x++ )
            {
               edit->line[edit->on_line][ep] = temp_buf[x];
               ep++;
            }
            edit->line[edit->on_line][ep] = '\0';

            edit->on_line++;
            edit->numlines++;

            old_p = p + 1;
            p += 75;

         }
         pager_printf( ch, "Reformating done.\r\n> " );
         return;
      }

      if( !str_cmp( cmd + 1, "i" ) )
      {
         if( edit->numlines >= max_buf_lines )
            send_to_char( "Buffer is full.\r\n> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
               line = edit->on_line;
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\r\n> ", ch );
            else
            {
               for( x = ++edit->numlines; x > line; x-- )
                  mudstrlcpy( edit->line[x], edit->line[x - 1], MAX_STRING_LENGTH );
               mudstrlcpy( edit->line[line], "", MAX_STRING_LENGTH );
               send_to_char( "Line inserted.\r\n> ", ch );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "d" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\r\n> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
               line = edit->on_line;
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\r\n> ", ch );
            else
            {
               if( line == 0 && edit->numlines == 1 )
               {
                  memset( edit, '\0', sizeof( EDITOR_DATA ) );
                  edit->numlines = 0;
                  edit->on_line = 0;
                  send_to_char( "Line deleted.\r\n> ", ch );
                  return;
               }
               for( x = line; x < ( edit->numlines - 1 ); x++ )
                  mudstrlcpy( edit->line[x], edit->line[x + 1], MAX_STRING_LENGTH );
               mudstrlcpy( edit->line[edit->numlines--], "", MAX_STRING_LENGTH );
               if( edit->on_line > edit->numlines )
                  edit->on_line = edit->numlines;
               send_to_char( "Line deleted.\r\n> ", ch );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "g" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\r\n> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
            {
               send_to_char( "Goto what line?\r\n> ", ch );
               return;
            }
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\r\n> ", ch );
            else
            {
               edit->on_line = line;
               ch_printf( ch, "(On line %d)\r\n> ", line + 1 );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "l" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\r\n> ", ch );
         else
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "------------------"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            for( x = 0; x < edit->numlines; x++ )
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%2d> %s", x + 1, edit->line[x]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            send_to_char( "------------------\r\n> ", ch );
         }
         return;
      }
      if( !str_cmp( cmd + 1, "a" ) )
      {
         send_to_char( "\r\nAborting... ", ch );
         stop_editing( ch );
         return;
      }
      if( get_trust( ch ) > LEVEL_IMMORTAL && !str_cmp( cmd + 1, "!" ) )
      {
         DO_FUN *last_cmd;
         int substate = ch->substate;

         last_cmd = ch->last_cmd;
         ch->substate = SUB_RESTRICTED;
         interpret( ch, argument + 3 );
         ch->substate = substate;
         ch->last_cmd = last_cmd;
         set_char_color( AT_GREEN, ch );
         send_to_char( "\r\n> ", ch );
         return;
      }
      if( !str_cmp( cmd + 1, "s" ) )
      {
         d->connected = CON_PLAYING;
         if( !ch->last_cmd )
            return;
         ( *ch->last_cmd ) ( ch, "" );
         return;
      }
   }

   if( edit->size + strlen( argument ) + 1 >= MAX_STRING_LENGTH - 1 )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You buffer is full."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   else
   {
      if( strlen( argument ) > 79 )
      {
         strncpy( buf, argument, 79 );
         buf[79] = '\0';
         send_to_char( "(Long line trimmed)\r\n> ", ch );
      }
      else
         mudstrlcpy( buf, argument, MAX_INPUT_LENGTH );
      mudstrlcpy( edit->line[edit->on_line++], buf, MAX_STRING_LENGTH );
      if( edit->on_line > edit->numlines )
         edit->numlines++;
      if( edit->numlines >= max_buf_lines )
      {
         edit->numlines = max_buf_lines;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Buffer full."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         save = TRUE;
      }
   }

   if( save )
   {
      d->connected = CON_PLAYING;
      if( !ch->last_cmd )
         return;
      ( *ch->last_cmd ) ( ch, "" );
      return;
   }
   send_to_char( "> ", ch );
}

void assign_area( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char taf[1024];
   AREA_DATA *tarea, *tmp;
   bool created = FALSE;

   if( IS_NPC( ch ) )
      return;
   if( get_trust( ch ) > LEVEL_IMMORTAL && ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
   {
      tarea = ch->pcdata->area;
      snprintf( taf, 1024, "%s.are", capitalize( ch->name ) );
      if( !tarea )
      {
         for( tmp = first_build; tmp; tmp = tmp->next )
            if( !str_cmp( taf, tmp->filename ) )
            {
               tarea = tmp;
               break;
            }
      }
      if( !tarea )
      {
         log_printf_plus( LOG_NORMAL, ch->level, "Creating area entry for %s", ch->name );
         CREATE( tarea, AREA_DATA, 1 );
         LINK( tarea, first_build, last_build, next, prev );
         tarea->first_room = tarea->last_room = NULL;
         snprintf( buf, MAX_STRING_LENGTH, "{PROTO} %s's area in progress", ch->name );
         tarea->name = str_dup( buf );
         tarea->filename = str_dup( taf );
         snprintf( buf2, MAX_STRING_LENGTH, "%s", ch->name );
         tarea->author = STRALLOC( buf2 );
         tarea->age = 0;
         tarea->nplayer = 0;

         created = TRUE;
      }
      else
      {
         log_printf_plus( LOG_NORMAL, ch->level, "Updating area entry for %s", ch->name );
      }
      tarea->low_r_vnum = ch->pcdata->r_range_lo;
      tarea->low_o_vnum = ch->pcdata->o_range_lo;
      tarea->low_m_vnum = ch->pcdata->m_range_lo;
      tarea->hi_r_vnum = ch->pcdata->r_range_hi;
      tarea->hi_o_vnum = ch->pcdata->o_range_hi;
      tarea->hi_m_vnum = ch->pcdata->m_range_hi;
      ch->pcdata->area = tarea;
      if( created )
         sort_area( tarea, TRUE );
   }
}

void do_aassign( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   AREA_DATA *tarea, *tmp;

   set_char_color( AT_WHITE, ch );

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: aassign <filename.are>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( "none", argument ) || !str_cmp( "null", argument ) || !str_cmp( "clear", argument ) )
   {
      ch->pcdata->area = NULL;
      assign_area( ch );
      if( !ch->pcdata->area )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area pointer cleared."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Originally assigned area restored."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mudstrlcpy( buf, argument, MAX_STRING_LENGTH );
   tarea = NULL;

/*	if ( get_trust(ch) >= sysdata.level_modify_proto )   */

   if( get_trust( ch ) >= LEVEL_GREATER
       || ( is_name( buf, ch->pcdata->bestowments )
            && get_trust( ch ) >= sysdata.level_modify_proto )
       || ( ch->pcdata->council && is_name( buf, ch->pcdata->council->powers ) ) )
      for( tmp = first_area; tmp; tmp = tmp->next )
         if( !str_cmp( buf, tmp->filename ) )
         {
            tarea = tmp;
            break;
         }

   if( !tarea )
      for( tmp = first_build; tmp; tmp = tmp->next )
         if( !str_cmp( buf, tmp->filename ) )
         {
/*		if ( get_trust(ch) >= sysdata.level_modify_proto  */
            if( get_trust( ch ) >= LEVEL_GREATER
                || is_name( tmp->filename, ch->pcdata->bestowments )
                || ( ch->pcdata->council
                     && ( is_name( buf, ch->pcdata->council->powers )
                          || is_name( "aassign", ch->pcdata->council->powers ) ) ) )
            {
               tarea = tmp;
               break;
            }
            else
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You do not have permission to use that area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               return;
            }
         }

   if( !tarea )
   {
      if( get_trust( ch ) >= sysdata.level_modify_proto )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such area.  Use 'zones'."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such area.  Use 'newzones'."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   ch->pcdata->area = tarea;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Assigning you: %s", tarea->name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA * room, const char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = room->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelRExtra( ROOM_INDEX_DATA * room, const char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = room->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA * obj, const char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = obj->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelOExtra( OBJ_DATA * obj, const char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA * obj, const char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = obj->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA * obj, const char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

void fwrite_fuss_exdesc( FILE * fpout, EXTRA_DESCR_DATA * ed )
{
   fprintf( fpout, "%s", "#EXDESC\n" );
   fprintf( fpout, "ExDescKey    %s~\n", ed->keyword );
   if( ed->description && ed->description[0] != '\0' )
      fprintf( fpout, "ExDesc       %s~\n", strip_cr( ed->description ) );
   fprintf( fpout, "%s", "#ENDEXDESC\n\n" );
}

void fwrite_fuss_exit( FILE * fpout, EXIT_DATA * pexit )
{
   fprintf( fpout, "%s", "#EXIT\n" );
   fprintf( fpout, "Direction %s~\n", strip_cr( dir_name[pexit->vdir] ) );
   fprintf( fpout, "ToRoom    %d\n", pexit->vnum );
   if( pexit->key != -1 && pexit->key > 0 )
      fprintf( fpout, "Key       %d\n", pexit->key );
   if( pexit->distance > 1 )
      fprintf( fpout, "Distance  %d\n", pexit->distance );
   if( pexit->pull )
      fprintf( fpout, "Pull      %d %d\n", pexit->pulltype, pexit->pull );
   if( pexit->description && pexit->description[0] != '\0' )
      fprintf( fpout, "Desc      %s~\n", strip_cr( pexit->description ) );
   if( pexit->keyword && pexit->keyword[0] != '\0' )
      fprintf( fpout, "Keywords  %s~\n", strip_cr( pexit->keyword ) );
   if( pexit->exit_info )
      fprintf( fpout, "Flags     %s~\n", flag_string( pexit->exit_info, ex_flags ) );
   fprintf( fpout, "%s", "#ENDEXIT\n\n" );
}

void fwrite_fuss_affect( FILE * fp, AFFECT_DATA * paf )
{
   if( paf->type < 0 || paf->type >= num_skills )
   {
      fprintf( fp, "Affect       %d %d %d %d %s\n",
               paf->type,
               paf->duration,
               ( ( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_REMOVESPELL
                   || paf->location == APPLY_STRIPSN
                   || paf->location == APPLY_RECURRINGSPELL )
                 && IS_VALID_SN( paf->modifier ) )
               ? skill_table[paf->modifier]->slot : paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }
   else
   {
      fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
               skill_table[paf->type]->name,
               paf->duration,
               ( ( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_REMOVESPELL
                   || paf->location == APPLY_STRIPSN
                   || paf->location == APPLY_RECURRINGSPELL )
                 && IS_VALID_SN( paf->modifier ) )
               ? skill_table[paf->modifier]->slot : paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }
}

// Write a prog
bool mprog_write_prog( FILE * fpout, MPROG_DATA * mprog )
{
   if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
   {
      fprintf( fpout, "%s", "#MUDPROG\n" );
      fprintf( fpout, "Progtype  %s~\n", mprog_type_to_name( mprog->type ) );
      fprintf( fpout, "Arglist   %s~\n", mprog->arglist );

      if( mprog->comlist && mprog->comlist[0] != '\0' && !mprog->fileprog )
         fprintf( fpout, "Comlist   %s~\n", strip_cr( mprog->comlist ) );

      fprintf( fpout, "%s", "#ENDPROG\n\n" );
      return true;
   }
   return false;
}

void save_reset_level( FILE * fpout, RESET_DATA * start_reset, const int level )
{
   int spaces = level * 2;

   RESET_DATA *reset;

   for( reset = start_reset; reset; )
   {
      switch ( UPPER( reset->command ) )  /* extra arg1 arg2 arg3 */
      {
         case '*':
            break;

         default:
            fprintf( fpout, "%*.sReset %c %d %d %d %d\n", spaces, "",
                     UPPER( reset->command ), reset->extra, reset->arg1, reset->arg2, reset->arg3 );
            break;

         case 'G':
         case 'R':
            fprintf( fpout, "%*.sReset %c %d %d %d\n", spaces, "",
                     UPPER( reset->command ), reset->extra, reset->arg1, reset->arg2 );
            break;
      }  /* end of switch on command */

      /*
       * recurse to save nested resets 
       */
      save_reset_level( fpout, reset->first_reset, level + 1 );

      /*
       * where we go next depends on if this is a top-level reset or not - for some reason 
       */
      if( level == 0 )
         reset = reset->next;
      else
         reset = reset->next_reset;
   }  /* end of looping through resets */
}  /* end of save_reset_level */

void fwrite_fuss_room( FILE * fpout, ROOM_INDEX_DATA * room, bool install )
{
   EXIT_DATA *xit;
   AFFECT_DATA *paf;
   EXTRA_DESCR_DATA *ed;
   MPROG_DATA *mprog;

   if( install )
   {
      CHAR_DATA *victim, *vnext;
      OBJ_DATA *obj, *obj_next;

      // remove prototype flag from room 
      xREMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );

      // purge room of (prototyped) mobiles 
      for( victim = room->first_person; victim; victim = vnext )
      {
         vnext = victim->next_in_room;
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            extract_char( victim, TRUE );
      }

      // purge room of (prototyped) objects 
      for( obj = room->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         if( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) )
            extract_obj( obj );
      }
   }

   // Get rid of the track markers before saving.
   xREMOVE_BIT( room->room_flags, ROOM_TRACK );

   fprintf( fpout, "%s", "#ROOM\n" );
   fprintf( fpout, "Vnum     %d\n", room->vnum );
   fprintf( fpout, "Name     %s~\n", room->name );
   fprintf( fpout, "Sector   %s~\n", strip_cr( sec_flags[room->sector_type] ) );
   if( !xIS_EMPTY( room->room_flags ) )
      fprintf( fpout, "Flags    %s~\n", ext_flag_string( &room->room_flags, r_flags ) );
   if( room->tele_delay > 0 || room->tele_vnum > 0 || room->tunnel > 0 )
      fprintf( fpout, "Stats    %d %d %d %d\n", room->tele_delay, room->tele_vnum, room->tunnel, room->max_weight );
   if( room->description && room->description[0] != '\0' )
      fprintf( fpout, "Desc     %s~\n", strip_cr( room->description ) );

   for( xit = room->first_exit; xit; xit = xit->next )
   {
      if( IS_SET( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
         continue;

      fwrite_fuss_exit( fpout, xit );
   }

   save_reset_level( fpout, room->first_reset, 0 );

   for( paf = room->first_permaffect; paf; paf = paf->next )
      fwrite_fuss_affect( fpout, paf );

   for( ed = room->first_extradesc; ed; ed = ed->next )
      fwrite_fuss_exdesc( fpout, ed );

   if( room->mudprogs )
   {
      for( mprog = room->mudprogs; mprog; mprog = mprog->next )
         mprog_write_prog( fpout, mprog );
   }
   fprintf( fpout, "%s", "#ENDROOM\n\n" );
}

void fwrite_fuss_object( FILE * fpout, OBJ_INDEX_DATA * pObjIndex, bool install )
{
   AFFECT_DATA *paf;
   EXTRA_DESCR_DATA *ed;
   MPROG_DATA *mprog;
   int val0, val1, val2, val3, val4, val5;

   if( install )
      xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );

   fprintf( fpout, "%s", "#OBJECT\n" );
   fprintf( fpout, "Vnum     %d\n", pObjIndex->vnum );
   fprintf( fpout, "Keywords %s~\n", pObjIndex->name );
   fprintf( fpout, "Type     %s~\n", o_types[pObjIndex->item_type] );
   fprintf( fpout, "Short    %s~\n", pObjIndex->short_descr );
   if( pObjIndex->description && pObjIndex->description[0] != '\0' )
      fprintf( fpout, "Long     %s~\n", pObjIndex->description );
   if( pObjIndex->action_desc && pObjIndex->action_desc[0] != '\0' )
      fprintf( fpout, "Action   %s~\n", pObjIndex->action_desc );
   if( !xIS_EMPTY( pObjIndex->extra_flags ) )
      fprintf( fpout, "Flags    %s~\n", ext_flag_string( &pObjIndex->extra_flags, o_flags ) );
   if( pObjIndex->wear_flags )
      fprintf( fpout, "WFlags   %s~\n", flag_string( pObjIndex->wear_flags, w_flags ) );

   val0 = pObjIndex->value[0];
   val1 = pObjIndex->value[1];
   val2 = pObjIndex->value[2];
   val3 = pObjIndex->value[3];
   val4 = pObjIndex->value[4];
   val5 = pObjIndex->value[5];

   switch ( pObjIndex->item_type )
   {
      case ITEM_PILL:
      case ITEM_POTION:
      case ITEM_SCROLL:
         if( IS_VALID_SN( val1 ) )
         {
            val1 = HAS_SPELL_INDEX;
         }
         if( IS_VALID_SN( val2 ) )
         {
            val2 = HAS_SPELL_INDEX;
         }
         if( IS_VALID_SN( val3 ) )
         {
            val3 = HAS_SPELL_INDEX;
         }
         break;

      case ITEM_STAFF:
      case ITEM_WAND:
         if( IS_VALID_SN( val3 ) )
         {
            val3 = HAS_SPELL_INDEX;
         }
         break;
      case ITEM_SALVE:
         if( IS_VALID_SN( val4 ) )
         {
            val4 = HAS_SPELL_INDEX;
         }
         if( IS_VALID_SN( val5 ) )
         {
            val5 = HAS_SPELL_INDEX;
         }
         break;
   }
   fprintf( fpout, "Values   %d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
   fprintf( fpout, "Stats    %d %d %d %d %d\n", pObjIndex->weight,
            pObjIndex->cost, pObjIndex->rent ? pObjIndex->rent : ( int )( pObjIndex->cost / 10 ),
            pObjIndex->level, pObjIndex->layers );

   for( paf = pObjIndex->first_affect; paf; paf = paf->next )
      fwrite_fuss_affect( fpout, paf );

   switch ( pObjIndex->item_type )
   {
      case ITEM_PILL:
      case ITEM_POTION:
      case ITEM_SCROLL:
         fprintf( fpout, "Spells   '%s' '%s' '%s'\n",
                  IS_VALID_SN( pObjIndex->value[1] ) ?
                  skill_table[pObjIndex->value[1]]->name : "NONE",
                  IS_VALID_SN( pObjIndex->value[2] ) ?
                  skill_table[pObjIndex->value[2]]->name : "NONE",
                  IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
         break;
      case ITEM_STAFF:
      case ITEM_WAND:
         fprintf( fpout, "Spells   '%s'\n",
                  IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );

         break;
      case ITEM_SALVE:
         fprintf( fpout, "Spells   '%s' '%s'\n",
                  IS_VALID_SN( pObjIndex->value[4] ) ?
                  skill_table[pObjIndex->value[4]]->name : "NONE",
                  IS_VALID_SN( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
         break;
   }

   for( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
      fwrite_fuss_exdesc( fpout, ed );

   if( pObjIndex->mudprogs )
   {
      for( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
         mprog_write_prog( fpout, mprog );
   }

   fprintf( fpout, "%s", "#ENDOBJECT\n\n" );
}

void fwrite_fuss_mobile( FILE * fpout, MOB_INDEX_DATA * pMobIndex, bool install )
{
   SHOP_DATA *pShop;
   REPAIR_DATA *pRepair;
   MPROG_DATA *mprog;

   if( install )
      xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );

   fprintf( fpout, "%s", "#MOBILE\n" );

   fprintf( fpout, "Vnum       %d\n", pMobIndex->vnum );
   fprintf( fpout, "Keywords   %s~\n", pMobIndex->player_name );
   fprintf( fpout, "Short      %s~\n", pMobIndex->short_descr );
   if( pMobIndex->long_descr && pMobIndex->long_descr[0] != '\0' )
      fprintf( fpout, "Long       %s~\n", strip_cr( pMobIndex->long_descr ) );
   if( pMobIndex->description && pMobIndex->description[0] != '\0' )
      fprintf( fpout, "Desc       %s~\n", strip_cr( pMobIndex->description ) );
   fprintf( fpout, "Race       %s~\n", npc_race[pMobIndex->race] );
   fprintf( fpout, "Class      %s~\n", npc_class[pMobIndex->Class] );
   fprintf( fpout, "Position   %s~\n", npc_position[pMobIndex->position] );
   fprintf( fpout, "DefPos     %s~\n", npc_position[pMobIndex->defposition] );
   if( pMobIndex->spec_fun && pMobIndex->spec_funname && pMobIndex->spec_funname[0] != '\0' )
      fprintf( fpout, "Specfun    %s~\n", pMobIndex->spec_funname );
   fprintf( fpout, "Gender     %s~\n", npc_sex[pMobIndex->sex] );
   fprintf( fpout, "Actflags   %s~\n", ext_flag_string( &pMobIndex->act, act_flags ) );
   if( !xIS_EMPTY( pMobIndex->affected_by ) )
      fprintf( fpout, "Affected   %s~\n", ext_flag_string( &pMobIndex->affected_by, a_flags ) );
   fprintf( fpout, "Stats1     %d %d %d %d %d %d\n", pMobIndex->alignment, pMobIndex->level, pMobIndex->mobthac0,
            pMobIndex->ac, pMobIndex->gold, pMobIndex->exp );
   fprintf( fpout, "Stats2     %d %d %d\n", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
   fprintf( fpout, "Stats3     %d %d %d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
   fprintf( fpout, "Stats4     %d %d %d %d %d\n",
            pMobIndex->height, pMobIndex->weight, pMobIndex->numattacks, pMobIndex->hitroll, pMobIndex->damroll );
   fprintf( fpout, "Attribs    %d %d %d %d %d %d %d\n",
            pMobIndex->perm_str,
            pMobIndex->perm_int,
            pMobIndex->perm_wis, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_cha, pMobIndex->perm_lck );
   fprintf( fpout, "Saves      %d %d %d %d %d\n",
            pMobIndex->saving_poison_death,
            pMobIndex->saving_wand, pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff );
   if( pMobIndex->speaks )
      fprintf( fpout, "Speaks     %s~\n", flag_string( pMobIndex->speaks, lang_names ) );
   if( pMobIndex->speaking )
      fprintf( fpout, "Speaking   %s~\n", flag_string( pMobIndex->speaking, lang_names ) );
   if( pMobIndex->xflags )
      fprintf( fpout, "Bodyparts  %s~\n", flag_string( pMobIndex->xflags, part_flags ) );
   if( pMobIndex->resistant )
      fprintf( fpout, "Resist     %s~\n", flag_string( pMobIndex->resistant, ris_flags ) );
   if( pMobIndex->immune )
      fprintf( fpout, "Immune     %s~\n", flag_string( pMobIndex->immune, ris_flags ) );
   if( pMobIndex->susceptible )
      fprintf( fpout, "Suscept    %s~\n", flag_string( pMobIndex->susceptible, ris_flags ) );
   if( !xIS_EMPTY( pMobIndex->attacks ) )
      fprintf( fpout, "Attacks    %s~\n", ext_flag_string( &pMobIndex->attacks, attack_flags ) );
   if( !xIS_EMPTY( pMobIndex->defenses ) )
      fprintf( fpout, "Defenses   %s~\n", ext_flag_string( &pMobIndex->defenses, defense_flags ) );

   // Mob has a shop? Add that data to the mob index.
   if( ( pShop = pMobIndex->pShop ) != NULL )
   {
      fprintf( fpout, "ShopData   %d %d %d %d %d %d %d %d %d\n",
               pShop->buy_type[0], pShop->buy_type[1], pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4],
               pShop->profit_buy, pShop->profit_sell, pShop->open_hour, pShop->close_hour );
   }

   // Mob is a repair shop? Add that data to the mob index.
   if( ( pRepair = pMobIndex->rShop ) != NULL )
   {
      fprintf( fpout, "RepairData %d %d %d %d %d %d %d\n",
               pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type,
               pRepair->open_hour, pRepair->close_hour );
   }

   if( pMobIndex->mudprogs )
   {
      for( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
         mprog_write_prog( fpout, mprog );
   }
   fprintf( fpout, "%s", "#ENDMOBILE\n\n" );
}

void fwrite_area_header( FILE * fpout, AREA_DATA * tarea, bool install )
{
   if( install )
      REMOVE_BIT( tarea->flags, AFLAG_PROTOTYPE );

   fprintf( fpout, "%s", "#AREADATA\n" );
   fprintf( fpout, "Version      %d\n", tarea->version );
   fprintf( fpout, "Name         %s~\n", tarea->name );
   fprintf( fpout, "Author       %s~\n", tarea->author );
   fprintf( fpout, "WeatherX     %d\n", tarea->weatherx );
   fprintf( fpout, "WeatherY     %d\n", tarea->weathery );
   if( tarea->credits && tarea->credits[0] != '\0' )
      fprintf( fpout, "Credits      %s~\n", tarea->credits );
   fprintf( fpout, "Ranges       %d %d %d %d\n",
            tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
   if( tarea->spelllimit )
      fprintf( fpout, "SpellLimit   %d\n", tarea->spelllimit );
   if( tarea->high_economy || tarea->low_economy )
      fprintf( fpout, "Economy      %d %d\n", tarea->high_economy, tarea->low_economy );
   if( tarea->resetmsg )   /* Rennard */
      fprintf( fpout, "ResetMsg     %s~\n", tarea->resetmsg );
   if( tarea->reset_frequency )
      fprintf( fpout, "ResetFreq    %d\n", tarea->reset_frequency );

   if( tarea->flags )
      fprintf( fpout, "Flags        %s~\n", flag_string( tarea->flags, area_flags ) );

   fprintf( fpout, "%s", "#ENDAREADATA\n\n" );
}

void fold_area( AREA_DATA * tarea, const char *fname, bool install )
{
   char buf[256];
   FILE *fpout;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   ROOM_INDEX_DATA *pRoomIndex;
   int vnum;

   snprintf( buf, 256, "%s.bak", fname );
   rename( fname, buf );
   if( !( fpout = fopen( fname, "w" ) ) )
   {
      bug( "%s: fopen", __func__ );
      perror( fname );
      return;
   }

   tarea->version = AREA_VERSION_WRITE;

   fprintf( fpout, "%s", "#FUSSAREA\n" );

   fwrite_area_header( fpout, tarea, install );

   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; ++vnum )
   {
      if( !( pMobIndex = get_mob_index( vnum ) ) )
         continue;
      fwrite_fuss_mobile( fpout, pMobIndex, install );
   }

   for( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; ++vnum )
   {
      if( !( pObjIndex = get_obj_index( vnum ) ) )
         continue;
      fwrite_fuss_object( fpout, pObjIndex, install );
   }

   for( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; ++vnum )
   {
      if( !( pRoomIndex = get_room_index( vnum ) ) )
         continue;
      fwrite_fuss_room( fpout, pRoomIndex, install );
   }

   fprintf( fpout, "%s", "#ENDAREA\n" );
   fclose( fpout );
   fpout = NULL;
   return;
}

void old_fold_area( AREA_DATA * tarea, char *filename, bool install )
{
   RESET_DATA *pReset, *tReset, *gReset;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   MPROG_DATA *mprog;
   EXIT_DATA *xit;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   SHOP_DATA *pShop;
   REPAIR_DATA *pRepair;
   char buf[MAX_STRING_LENGTH];
   FILE *fpout;
   int vnum;
   int val0, val1, val2, val3, val4, val5;
   bool complexmob;

   log_printf_plus( LOG_NORMAL, LEVEL_GREATER, "Saving %s...", tarea->filename );

   snprintf( buf, MAX_STRING_LENGTH, "%s.bak", filename );
   rename( filename, buf );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      bug( "fold_area: cant open %s", filename );
      perror( filename );
      return;
   }

   if( install )
      REMOVE_BIT( tarea->flags, AFLAG_PROTOTYPE );
   fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );
   fprintf( fpout, "#VERSION %d\n", AREA_VERSION_WRITE );
   fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
   fprintf( fpout, "#CREDITS %s~\n\n", tarea->credits );
   fprintf( fpout, "#RANGES\n" );
   fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range,
            tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
   fprintf( fpout, "$\n\n" );
   if( tarea->resetmsg )   /* Rennard */
      fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );
   if( tarea->reset_frequency )
      fprintf( fpout, "#FLAGS\n%d %d\n\n", tarea->flags, tarea->reset_frequency );
   else
      fprintf( fpout, "#FLAGS\n%d\n\n", tarea->flags );

   fprintf( fpout, "#ECONOMY %d %d\n\n", tarea->high_economy, tarea->low_economy );

   /*
    * save mobiles 
    */
   fprintf( fpout, "#MOBILES\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
         continue;
      if( install )
         xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );
      if( pMobIndex->perm_str != 13 || pMobIndex->perm_int != 13
          || pMobIndex->perm_wis != 13 || pMobIndex->perm_dex != 13
          || pMobIndex->perm_con != 13 || pMobIndex->perm_cha != 13
          || pMobIndex->perm_lck != 13
          || pMobIndex->hitroll != 0 || pMobIndex->damroll != 0
          || pMobIndex->race != 0 || pMobIndex->Class != 3
          || !xIS_EMPTY( pMobIndex->attacks )
          || !xIS_EMPTY( pMobIndex->defenses )
          || pMobIndex->height != 0 || pMobIndex->weight != 0
          || pMobIndex->speaks != 0 || pMobIndex->speaking != 0 || pMobIndex->xflags != 0 || pMobIndex->numattacks != 0 )
         complexmob = TRUE;
      else
         complexmob = FALSE;
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", pMobIndex->player_name );
      fprintf( fpout, "%s~\n", pMobIndex->short_descr );
      fprintf( fpout, "%s~\n", strip_cr( pMobIndex->long_descr ) );
      fprintf( fpout, "%s~\n", strip_cr( pMobIndex->description ) );
      fprintf( fpout, "%s ", print_bitvector( &pMobIndex->act ) );
      fprintf( fpout, "%s %d %c\n", print_bitvector( &pMobIndex->affected_by ),
               pMobIndex->alignment, complexmob ? 'C' : 'S' );

      fprintf( fpout, "%d %d %d ", pMobIndex->level, pMobIndex->mobthac0, pMobIndex->ac );
      fprintf( fpout, "%dd%d+%d ", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
      fprintf( fpout, "%dd%d+%d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
      fprintf( fpout, "%d %d\n", pMobIndex->gold, pMobIndex->exp );
      /*
       * Need to convert to new positions correctly on loadup sigh -Shaddai 
       */
      fprintf( fpout, "%d %d %d\n", pMobIndex->position + 100, pMobIndex->defposition + 100, pMobIndex->sex );
      if( complexmob )
      {
         fprintf( fpout, "%d %d %d %d %d %d %d\n",
                  pMobIndex->perm_str,
                  pMobIndex->perm_int,
                  pMobIndex->perm_wis, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_cha, pMobIndex->perm_lck );
         fprintf( fpout, "%d %d %d %d %d\n",
                  pMobIndex->saving_poison_death,
                  pMobIndex->saving_wand,
                  pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff );
         fprintf( fpout, "%d %d %d %d %d %d %d\n",
                  pMobIndex->race,
                  pMobIndex->Class,
                  pMobIndex->height, pMobIndex->weight, pMobIndex->speaks, pMobIndex->speaking, pMobIndex->numattacks );
         fprintf( fpout, "%d %d %d %d %d %d %s ",
                  pMobIndex->hitroll,
                  pMobIndex->damroll,
                  pMobIndex->xflags,
                  pMobIndex->resistant, pMobIndex->immune, pMobIndex->susceptible, print_bitvector( &pMobIndex->attacks ) );
         fprintf( fpout, "%s\n", print_bitvector( &pMobIndex->defenses ) );
      }
      if( pMobIndex->mudprogs )
      {
         int count = 0;
         for( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
         {
            if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
            {
               if( mprog->type == IN_FILE_PROG )
               {
                  fprintf( fpout, "> %s %s~\n", mprog_type_to_name( mprog->type ), mprog->arglist );
                  count++;
               }
               /*
                * Don't let it save progs which came from files. That would be silly. 
                */
               else if( mprog->comlist && mprog->comlist[0] != '\0' && !mprog->fileprog )
               {
                  fprintf( fpout, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ),
                           mprog->arglist, strip_cr( mprog->comlist ) );
                  count++;
               }
            }
         }
         if( count > 0 )
            fprintf( fpout, "%s", "|\n" );
      }
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_m_vnum )
      tarea->hi_m_vnum = vnum - 1;

   /*
    * save objects 
    */
   fprintf( fpout, "#OBJECTS\n" );
   for( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
   {
      if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
         continue;
      if( install )
         xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", pObjIndex->name );
      fprintf( fpout, "%s~\n", pObjIndex->short_descr );
      fprintf( fpout, "%s~\n", pObjIndex->description );
      fprintf( fpout, "%s~\n", pObjIndex->action_desc );
      fprintf( fpout, "%d %s %d %d %d\n", pObjIndex->item_type,
               print_bitvector( &pObjIndex->extra_flags ), pObjIndex->wear_flags, pObjIndex->layers, pObjIndex->level );

      val0 = pObjIndex->value[0];
      val1 = pObjIndex->value[1];
      val2 = pObjIndex->value[2];
      val3 = pObjIndex->value[3];
      val4 = pObjIndex->value[4];
      val5 = pObjIndex->value[5];
      switch ( pObjIndex->item_type )
      {
         case ITEM_PILL:
         case ITEM_POTION:
         case ITEM_SCROLL:
            if( IS_VALID_SN( val1 ) )
            {
               val1 = HAS_SPELL_INDEX;
            }
            if( IS_VALID_SN( val2 ) )
            {
               val2 = HAS_SPELL_INDEX;
            }
            if( IS_VALID_SN( val3 ) )
            {
               val3 = HAS_SPELL_INDEX;
            }
            break;

         case ITEM_STAFF:
         case ITEM_WAND:
            if( IS_VALID_SN( val3 ) )
            {
               val3 = HAS_SPELL_INDEX;
            }
            break;
         case ITEM_SALVE:
            if( IS_VALID_SN( val4 ) )
            {
               val4 = HAS_SPELL_INDEX;
            }
            if( IS_VALID_SN( val5 ) )
            {
               val5 = HAS_SPELL_INDEX;
            }
            break;
      }
      if( val4 || val5 )
         fprintf( fpout, "%d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
      else
         fprintf( fpout, "%d %d %d %d\n", val0, val1, val2, val3 );

      fprintf( fpout, "%d %d %d\n", pObjIndex->weight,
               pObjIndex->cost, pObjIndex->rent ? pObjIndex->rent : ( int )( pObjIndex->cost / 10 ) );

      switch ( pObjIndex->item_type )
      {
         case ITEM_PILL:
         case ITEM_POTION:
         case ITEM_SCROLL:
            fprintf( fpout, "'%s' '%s' '%s'\n",
                     IS_VALID_SN( pObjIndex->value[1] ) ?
                     skill_table[pObjIndex->value[1]]->name : "NONE",
                     IS_VALID_SN( pObjIndex->value[2] ) ?
                     skill_table[pObjIndex->value[2]]->name : "NONE",
                     IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
            break;
         case ITEM_STAFF:
         case ITEM_WAND:
            fprintf( fpout, "'%s'\n", IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );

            break;
         case ITEM_SALVE:
            fprintf( fpout, "'%s' '%s'\n",
                     IS_VALID_SN( pObjIndex->value[4] ) ?
                     skill_table[pObjIndex->value[4]]->name : "NONE",
                     IS_VALID_SN( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
            break;
      }

      for( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
         fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

      for( paf = pObjIndex->first_affect; paf; paf = paf->next )
         fprintf( fpout, "A\n%d %d\n", paf->location,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) ) ? skill_table[paf->modifier]->slot : paf->modifier );

      if( pObjIndex->mudprogs )
      {
         int count = 0;
         for( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
         {
            if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
            {
               if( mprog->type == IN_FILE_PROG )
               {
                  fprintf( fpout, "> %s %s~\n", mprog_type_to_name( mprog->type ), mprog->arglist );
                  count++;
               }
               /*
                * Don't let it save progs which came from files. That would be silly. 
                */
               else if( mprog->comlist && mprog->comlist[0] != '\0' && !mprog->fileprog )
               {
                  fprintf( fpout, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ),
                           mprog->arglist, strip_cr( mprog->comlist ) );
                  count++;
               }
            }
         }
         if( count > 0 )
            fprintf( fpout, "%s", "|\n" );
      }
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_o_vnum )
      tarea->hi_o_vnum = vnum - 1;

   /*
    * save rooms   
    */
   fprintf( fpout, "#ROOMS\n" );
   for( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      if( install )
      {
         CHAR_DATA *victim, *vnext;
         OBJ_DATA *obj, *obj_next;

         /*
          * remove prototype flag from room 
          */
         xREMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );
         /*
          * purge room of (prototyped) mobiles 
          */
         for( victim = room->first_person; victim; victim = vnext )
         {
            vnext = victim->next_in_room;
            if( IS_NPC( victim ) )
               extract_char( victim, TRUE );
         }
         /*
          * purge room of (prototyped) objects 
          */
         for( obj = room->first_content; obj; obj = obj_next )
         {
            obj_next = obj->next_content;
            extract_obj( obj );
         }
      }
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", room->name );
      fprintf( fpout, "%s~\n", strip_cr( room->description ) );
      if( ( room->tele_delay > 0 && room->tele_vnum > 0 ) || room->tunnel > 0 || room->max_weight > 0 )
         fprintf( fpout, "0 %s %d %d %d %d %d\n", print_bitvector( &room->room_flags ),
                  room->sector_type, room->tele_delay, room->tele_vnum, room->tunnel, room->max_weight );
      else
         fprintf( fpout, "0 %s %d\n", print_bitvector( &room->room_flags ), room->sector_type );
      for( xit = room->first_exit; xit; xit = xit->next )
      {
         if( IS_SET( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
            continue;
         fprintf( fpout, "D%d\n", xit->vdir );
         fprintf( fpout, "%s~\n", strip_cr( xit->description ) );
         fprintf( fpout, "%s~\n", strip_cr( xit->keyword ) );
         if( xit->distance > 1 || xit->pull )
            fprintf( fpout, "%d %d %d %d %d %d\n",
                     xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance, xit->pulltype, xit->pull );
         else
            fprintf( fpout, "%d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum );
      }

      for( pReset = room->first_reset; pReset; pReset = pReset->next )
      {
         switch ( pReset->command ) /* extra arg1 arg2 arg3 */
         {
            default:
            case '*':
               break;
            case 'm':
            case 'M':
            case 'o':
            case 'O':
               fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
                        pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );

               for( tReset = pReset->first_reset; tReset; tReset = tReset->next_reset )
               {
                  switch ( tReset->command )
                  {
                     case 'p':
                     case 'P':
                     case 'e':
                     case 'E':
                        fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
                        if( tReset->first_reset )
                        {
                           for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
                           {
                              if( gReset->command != 'p' && gReset->command != 'P' )
                                 continue;
                              fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
                                       gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
                           }
                        }
                        break;

                     case 'g':
                     case 'G':
                        fprintf( fpout, "  R %c %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2 );
                        if( tReset->first_reset )
                        {
                           for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
                           {
                              if( gReset->command != 'p' && gReset->command != 'P' )
                                 continue;
                              fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
                                       gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
                           }
                        }
                        break;

                     case 't':
                     case 'T':
                     case 'h':
                     case 'H':
                        fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
                        break;
                  }
               }
               break;

            case 'd':
            case 'D':
            case 't':
            case 'T':
            case 'h':
            case 'H':
               fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
                        pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
               break;

            case 'r':
            case 'R':
               fprintf( fpout, "R %c %d %d %d\n", UPPER( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2 );
               break;
         }
      }

      for( ed = room->first_extradesc; ed; ed = ed->next )
         fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

      if( room->mudprogs )
      {
         int count = 0;
         for( mprog = room->mudprogs; mprog; mprog = mprog->next )
         {
            if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
            {
               if( mprog->type == IN_FILE_PROG )
               {
                  fprintf( fpout, "> %s %s~\n", mprog_type_to_name( mprog->type ), mprog->arglist );
                  count++;
               }
               /*
                * Don't let it save progs which came from files. That would be silly. 
                */
               else if( mprog->comlist && mprog->comlist[0] != '\0' && !mprog->fileprog )
               {
                  fprintf( fpout, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ),
                           mprog->arglist, strip_cr( mprog->comlist ) );
                  count++;
               }
            }
         }
         if( count > 0 )
            fprintf( fpout, "%s", "|\n" );
      }
      fprintf( fpout, "S\n" );
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_r_vnum )
      tarea->hi_r_vnum = vnum - 1;

   /*
    * save shops 
    */
   fprintf( fpout, "#SHOPS\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
         continue;
      if( ( pShop = pMobIndex->pShop ) == NULL )
         continue;
      fprintf( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
               pShop->keeper,
               pShop->buy_type[0],
               pShop->buy_type[1],
               pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4], pShop->profit_buy, pShop->profit_sell );
      fprintf( fpout, "        %2d %2d    ; %s\n", pShop->open_hour, pShop->close_hour, pMobIndex->short_descr );
   }
   fprintf( fpout, "0\n\n\n" );

   /*
    * save repair shops 
    */
   fprintf( fpout, "#REPAIRS\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
         continue;
      if( ( pRepair = pMobIndex->rShop ) == NULL )
         continue;
      fprintf( fpout, " %d   %2d %2d %2d         %3d %3d",
               pRepair->keeper,
               pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type );
      fprintf( fpout, "        %2d %2d    ; %s\n", pRepair->open_hour, pRepair->close_hour, pMobIndex->short_descr );
   }
   fprintf( fpout, "0\n\n\n" );

   /*
    * save specials 
    */
   fprintf( fpout, "%s", "#SPECIALS\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
         if( pMobIndex->spec_fun )
            fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, pMobIndex->spec_funname );
   }
   fprintf( fpout, "S\n\n\n" );

   /*
    * END 
    */
   fprintf( fpout, "#$\n" );
   fclose( fpout );
   return;
}

void do_savearea( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;
   char filename[256];

   set_char_color( AT_WHITE, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't have an assigned area to save."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( argument[0] == '\0' )
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if( get_trust( ch ) < LEVEL_GOD )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only save your own area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      for( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            break;
         }
      if( !found )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !tarea )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No area to save."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Ensure not wiping out their area with save before load - Scryn 8/11 
    */
   if( !IS_SET( tarea->status, AREA_LOADED ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your area is not loaded!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_SET( tarea->flags, AFLAG_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Cannot savearea %s, use foldarea instead.", tarea->filename); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   snprintf( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving area..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   fold_area( tarea, filename, FALSE );
   set_char_color( AT_WHITE, ch );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
}

void do_loadarea( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;
   char filename[256];
   int tmp;

   set_char_color( AT_WHITE, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You don't have an assigned area to load."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( argument[0] == '\0' )
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if( get_trust( ch ) < LEVEL_GOD )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can only load your own area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      for( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            break;
         }
      if( !found )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !tarea )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No area to load."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Stops char from loading when already loaded - Scryn 8/11 
    */
   if( IS_SET( tarea->status, AREA_LOADED ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your area is already loaded."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   snprintf( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Loading..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   load_area_file( tarea, filename );
   /*
    * Couldnt load the area? 
    */
   if( !IS_SET( tarea->status, AREA_LOADED ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your area couldn't be loaded."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Planing..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   check_planes( NULL );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Linking exits..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   fix_area_exits( tarea );
   if( tarea->first_room )
   {
      tmp = tarea->nplayer;
      tarea->nplayer = 0;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Resetting area..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      reset_area( tarea );
      tarea->nplayer = tmp;
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea( CHAR_DATA* ch, const char* argument)
{

   set_char_color( AT_WHITE, ch );

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unfold what?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   fBootDb = TRUE;
   load_area_file( last_area, argument );
   fBootDb = FALSE;
   check_planes( NULL );
   return;
}


void do_foldarea( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;

   set_char_color( AT_WHITE, ch );

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fold what?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( argument, "all" ) )
   {
      for( tarea = first_area; tarea; tarea = tarea->next )
         fold_area( tarea, tarea->filename, FALSE );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "All installed areas have been folded."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      if( !str_cmp( tarea->filename, argument ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Folding area..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         fold_area( tarea, tarea->filename, FALSE );
         set_char_color( AT_WHITE, ch );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such area exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
}

extern int top_area;

void write_area_list( void )
{
   AREA_DATA *tarea;
   FILE *fpout;

   fpout = fopen( AREA_LIST, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open %s for writing!", AREA_LIST );
      return;
   }
   fprintf( fpout, "help.are\n" );
   for( tarea = first_area; tarea; tarea = tarea->next )
      fprintf( fpout, "%s\n", tarea->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int num;
   DESCRIPTOR_DATA *d;

   set_char_color( AT_WHITE, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: installarea <filename> [Area title]"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( tarea = first_build; tarea; tarea = tarea->next )
   {
      if( !str_cmp( tarea->filename, arg ) )
      {
         if( argument && argument[0] != '\0' )
         {
            DISPOSE( tarea->name );
            tarea->name = str_dup( argument );
         }

         /*
          * Fold area with install flag -- auto-removes prototype flags 
          */
         REMOVE_BIT( tarea->flags, AFLAG_PROTOTYPE );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Saving and installing file..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         fold_area( tarea, tarea->filename, TRUE );

         /*
          * Remove from prototype area list 
          */
         UNLINK( tarea, first_build, last_build, next, prev );

         /*
          * Add to real area list 
          */
         LINK( tarea, first_area, last_area, next, prev );

         /*
          * Remove it from the prototype sort list. BUGFIX: Samson 4-15-03 
          */
         UNLINK( tarea, first_bsort, last_bsort, next_sort, prev_sort );

         /*
          * Sort the area into it's proper sort list. BUGFIX: Samson 4-15-03 
          */
         sort_area( tarea, FALSE );
         sort_area_by_name( tarea );

         /*
          * Fix up author if online 
          */
         for( d = first_descriptor; d; d = d->next )
            if( d->character && d->character->pcdata && d->character->pcdata->area == tarea )
            {
               /*
                * remove area from author 
                */
               d->character->pcdata->area = NULL;
               /*
                * clear out author vnums  
                */
               d->character->pcdata->r_range_lo = 0;
               d->character->pcdata->r_range_hi = 0;
               d->character->pcdata->o_range_lo = 0;
               d->character->pcdata->o_range_hi = 0;
               d->character->pcdata->m_range_lo = 0;
               d->character->pcdata->m_range_hi = 0;
               save_char_obj( d->character );
            }

         top_area++;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Writing area.lst..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         write_area_list(  );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Resetting new area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         num = tarea->nplayer;
         tarea->nplayer = 0;
         reset_area( tarea );
         tarea->nplayer = num;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Renaming author's building file."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         snprintf( buf, MAX_STRING_LENGTH, "%s%s.installed", BUILD_DIR, tarea->filename );
         snprintf( arg, MAX_INPUT_LENGTH, "%s%s", BUILD_DIR, tarea->filename );
         rename( arg, buf );
         snprintf( buf, MAX_STRING_LENGTH, "%s%s.bak", BUILD_DIR, tarea->filename );
         unlink( buf );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such area exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_astat( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;
   bool proto, found;
   int pdeaths = 0, pkills = 0, mdeaths = 0, mkills = 0;
   found = FALSE;
   proto = FALSE;

   set_char_color( AT_PLAIN, ch );

   if( !str_cmp( "summary", argument ) )
   {
      for( tarea = first_area; tarea; tarea = tarea->next )
      {
         pdeaths += tarea->pdeaths;
         mdeaths += tarea->mdeaths;
         pkills += tarea->pkills;
         mkills += tarea->mkills;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WTotal pdeaths:      &w%d", pdeaths); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WTotal pkills:       &w%d", pkills); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WTotal mdeaths:      &w%d", mdeaths); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WTotal mkills:       &w%d", mkills); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( tarea->filename, argument ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
      for( tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if( !found )
   {
      if( argument && argument[0] != '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area not found.  Check 'zones'."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         tarea = ch->in_room->area;
      }
   }

   ch_printf_color( ch, "\r\n&wName:     &W%s\r\n&wFilename: &W%-20s  &wPrototype: &W%s\r\n&wAuthor:   &W%s\r\n",
                    tarea->name, tarea->filename, proto ? "yes" : "no", tarea->author );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wAge: &W%-3d  &wCurrent number of players: &W%-3d  &wMax players: &W%d", tarea->age, tarea->nplayer, tarea->max_players); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wWeather: X Coord: &W%-3d  &w Y Coord: &W%-3d", tarea->weatherx, tarea->weathery); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   if( !proto )
   {
      if( tarea->high_economy )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wArea economy: &W%d &wbillion and &W%d gold coins.", tarea->high_economy, tarea->low_economy); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wArea economy: &W%d &wgold coins.", tarea->low_economy); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wGold Looted:  &W%d", tarea->gold_looted); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wMdeaths: &W%d   &wMkills: &W%d   &wPdeaths: &W%d   &wPkills: &W%d   &wIllegalPK: &W%d", tarea->mdeaths, tarea->mkills, tarea->pdeaths, tarea->pkills, tarea->illegal_pk); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wlow_room: &W%5d    &whi_room: &W%5d", tarea->low_r_vnum, tarea->hi_r_vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wlow_obj : &W%5d    &whi_obj : &W%5d", tarea->low_o_vnum, tarea->hi_o_vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wlow_mob : &W%5d    &whi_mob : &W%5d", tarea->low_m_vnum, tarea->hi_m_vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wsoft range: &W%d - %d    &whard range: &W%d - %d", tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wArea flags: &W%s", flag_string( tarea->flags, area_flags )); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wResetmsg: &W%s", tarea->resetmsg ? tarea->resetmsg : "(default)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);} /* Rennard */
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wReset frequency: &W%d &wminutes.", tarea->reset_frequency ? tarea->reset_frequency : 15); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
}

/* check other areas for a conflict while ignoring the current area */
bool check_for_area_conflicts( AREA_DATA * carea, int lo, int hi )
{
   AREA_DATA *area;

   for( area = first_area; area; area = area->next )
      if( area != carea && check_area_conflict( area, lo, hi ) )
         return TRUE;
   for( area = first_build; area; area = area->next )
      if( area != carea && check_area_conflict( area, lo, hi ) )
         return TRUE;

   return FALSE;
}

void do_aset( CHAR_DATA* ch, const char* argument)
{
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   bool proto, found;
   int vnum, value;

   set_char_color( AT_WHITE, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   vnum = atoi( argument );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: aset <area filename> <field> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\nField being one of:\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  low_room hi_room low_obj hi_obj low_mob hi_mob"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  name filename low_soft hi_soft low_hard hi_hard"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  author credits resetmsg resetfreq flags"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( get_trust( ch ) >= LEVEL_ASCENDANT )
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  weatherx weathery"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   found = FALSE;
   proto = FALSE;
   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( tarea->filename, arg1 ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
      for( tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, arg1 ) )
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if( !found )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Area not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      AREA_DATA *uarea;

      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't set an area's name to nothing."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      for( uarea = first_area; uarea; uarea = uarea->next )
      {
         if( !str_cmp( uarea->name, argument ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is already an installed area with that name."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }

      for( uarea = first_build; uarea; uarea = uarea->next )
      {
         if( !str_cmp( uarea->name, argument ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is already a prototype area with that name."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      DISPOSE( tarea->name );
      tarea->name = str_dup( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      char filename[256];

      if( proto )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You should only change the filename of installed areas."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !is_valid_filename( ch, "", argument ) )
         return;

      mudstrlcpy( filename, tarea->filename, 256 );
      DISPOSE( tarea->filename );
      tarea->filename = str_dup( argument );
      rename( filename, tarea->filename );
      write_area_list(  );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "low_economy" ) )
   {
      tarea->low_economy = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "high_economy" ) )
   {
      tarea->high_economy = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "weatherx" ) )
   {
      if( get_trust( ch ) < LEVEL_ASCENDANT )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "I don't recognize that field. Please double check."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
      {
         tarea->weatherx = vnum;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg2, "weathery" ) )
   {
      if( get_trust( ch ) < LEVEL_ASCENDANT )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "I don't recognize that field. Please double check."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
      {
         tarea->weathery = vnum;
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg2, "low_room" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->low_r_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->hi_r_vnum < vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set low_vnum higher than the hi_vnum."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->low_r_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hi_room" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->hi_r_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->low_r_vnum > vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set low_vnum lower than the low_vnum."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->hi_r_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "low_obj" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->low_o_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->hi_o_vnum < vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set low_obj higher than the hi_obj."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->low_o_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hi_obj" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->hi_o_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->low_o_vnum > vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set hi_obj lower than the low_obj."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->hi_o_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "low_mob" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->low_m_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->hi_m_vnum < vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set low_mob higher than the hi_mob."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->low_m_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hi_mob" ) )
   {
      if( check_for_area_conflicts( tarea, tarea->hi_m_vnum, vnum ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That would conflict with another area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( tarea->low_m_vnum > vnum )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set hi_mob lower than the low_mob."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      tarea->hi_m_vnum = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "low_soft" ) )
   {
      if( vnum < 0 || vnum > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not an acceptable value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      tarea->low_soft_range = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hi_soft" ) )
   {
      if( vnum < 0 || vnum > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not an acceptable value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      tarea->hi_soft_range = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "low_hard" ) )
   {
      if( vnum < 0 || vnum > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not an acceptable value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      tarea->low_hard_range = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "hi_hard" ) )
   {
      if( vnum < 0 || vnum > MAX_LEVEL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not an acceptable value."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      tarea->hi_hard_range = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "author" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't set an area's author to nothing."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      STRFREE( tarea->author );
      tarea->author = STRALLOC( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "credits" ) )
   {
      if( tarea->credits );
      STRFREE( tarea->credits );
      tarea->credits = STRALLOC( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "resetmsg" ) )
   {
      if( tarea->resetmsg )
         DISPOSE( tarea->resetmsg );
      if( str_cmp( argument, "clear" ) && argument && argument[0] != '\0' )
         tarea->resetmsg = str_dup( argument );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }  /* Rennard */

   if( !str_cmp( arg2, "resetfreq" ) )
   {
      tarea->reset_frequency = vnum;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: aset <filename> flags <flag> [flag]..."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_areaflag( arg3 );
         if( value < 0 || value > 31 )
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Unknown flag: %s", arg3); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         else
         {
            if( IS_SET( tarea->flags, 1 << value ) )
               REMOVE_BIT( tarea->flags, 1 << value );
            else
               SET_BIT( tarea->flags, 1 << value );
         }
      }
      return;
   }

   do_aset( ch, "" );
   return;
}


void do_rlist( CHAR_DATA* ch, const char* argument)
{
   ROOM_INDEX_DATA *room;
   int vnum;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   AREA_DATA *tarea;
   int lrange;
   int trange;

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
       || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou don't have an assigned area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] != '\0' && !is_number( arg1 ) )
      return;
   if( arg2[0] != '\0' && !is_number( arg2 ) )
      return;

/* I know the above code could be combined into 3 lines, but we write code
   once and then read it many times. It is so much easier to read without
   trying to figure out what opening parenthesis belongs with what closing
   one.  -- Gorog
*/

/* Bah! The following code uses atoi(arg1) and atoi(arg2) without even
   checking they are numeric or even exist. I put a fix for this above.
   It also uses is_number(arg1) without checking if arg1 may be null.
   Caused a crash when the command was uses with a single alpha arg.
   -- Gorog
*/

   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_r_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_r_vnum;
      else
         trange = atoi( arg2 );

      if( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum ) && get_trust( ch ) < LEVEL_GREATER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat is out of your vnum range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %s\r\n", vnum, room->name );
   }
   return;
}

void do_olist( CHAR_DATA* ch, const char* argument)
{
   OBJ_INDEX_DATA *obj;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lrange;
   int trange;

   /*
    * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
    */

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
       || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou don't have an assigned area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_o_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_o_vnum;
      else
         trange = atoi( arg2 );

      if( ( lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum ) && get_trust( ch ) < LEVEL_GREATER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat is out of your vnum range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( obj = get_obj_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %-20s (%s)\r\n", vnum, obj->name, obj->short_descr );
   }
   return;
}

void do_mlist( CHAR_DATA* ch, const char* argument)
{
   MOB_INDEX_DATA *mob;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lrange;
   int trange;

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
       || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou don't have an assigned area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_m_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_m_vnum;
      else
         trange = atoi( arg2 );

      if( ( lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum ) && get_trust( ch ) < LEVEL_GREATER )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat is out of your vnum range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( mob = get_mob_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %-20s '%s'\r\n", vnum, mob->player_name, mob->short_descr );
   }
}

void mpedit( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, const char *argument )
{
   if( mptype != -1 )
   {
      mprg->type = mptype;
      if( mprg->arglist )
         STRFREE( mprg->arglist );
      mprg->arglist = STRALLOC( argument );
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if( !mprg->comlist )
      mprg->comlist = STRALLOC( "" );
   start_editing( ch, mprg->comlist );
   return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't mpedit"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_RESTRICTED:
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't use this command from within another command."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s", "do_mpedit: sub_mprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ( MPROG_DATA * ) ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   value = atoi( arg3 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: mpedit <victim> <command> [number] <program> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Command being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  add delete insert edit list"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand fight hitprcnt greet allgreet"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  entry give bribe death time hour script"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  greetinfight"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "They aren't here."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No one like that in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( get_trust( ch ) < victim->level || !IS_NPC( victim ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < LEVEL_GREATER && IS_NPC( victim ) && xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      set_pager_color( AT_WHITE, ch );
      send_to_pager( "Their godly glow prevents you from getting close enough.\r\n", ch );
      return;
   }

   if( !can_mmodify( ch, victim ) )
      return;

   if( !xIS_SET( victim->act, ACT_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "A mobile must have a prototype flag to be mpset."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mprog = victim->pIndexData->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg2, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No programs on mobile:  %s - #%d", victim->name, victim->pIndexData->vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 1 )
      {
         if( strcmp( "full", arg3 ) )
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d>%s %s", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            }

            return;
         }
         else
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            }

            return;
         }
      }

      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            ch_printf( ch, "%d>%s %s\r\n%s\r\n", cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            break;
         }
      }

      if( !mprg )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      return;
   }

   if( !str_cmp( arg2, "edit" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No programs on mobile:  %s - #%d", victim->name, victim->pIndexData->vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      if( arg4[0] != '\0' )
      {
         mptype = get_mpflag( arg4 );
         if( mptype == -1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( victim->pIndexData->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( victim->pIndexData->progtypes, mprg->type );
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No programs on mobile:  %s - #%d", victim->name, victim->pIndexData->vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = victim->pIndexData->mudprogs;
         victim->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( victim->pIndexData->progtypes, mptype );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No programs on mobile:  %s - #%d", victim->name, victim->pIndexData->vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      mptype = get_mpflag( arg4 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( victim->pIndexData->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         victim->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( victim->pIndexData->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "add" ) )
   {
      mptype = get_mpflag( arg3 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( mprog != NULL )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         victim->pIndexData->mudprogs = mprg;
      xSET_BIT( victim->pIndexData->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_mpedit( ch, "" );
}

void do_opedit( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't opedit"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_RESTRICTED:
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't use this command from within another command."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s", "do_opedit: sub_oprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ( MPROG_DATA * ) ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   value = atoi( arg3 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: opedit <object> <command> [number] <program> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Command being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  add delete insert edit list"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand wear remove sac zap get"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  drop damage repair greet exa use"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  pull push (for levers,pullchains,buttons)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Object should be in your inventory to edit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't carrying that."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Nothing like that in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !can_omodify( ch, obj ) )
      return;

   if( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "An object must have a prototype flag to be opset."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mprog = obj->pIndexData->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg2, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object has no obj programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 1 )
      {
         if( strcmp( "full", arg3 ) )
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d>%s %s", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            }

            return;
         }
         else
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            }

            return;
         }
      }

      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            ch_printf( ch, "%d>%s %s\r\n%s\r\n", cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            break;
         }
      }

      if( !mprg )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      return;
   }

   if( !str_cmp( arg2, "edit" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object has no obj programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      if( arg4[0] != '\0' )
      {
         mptype = get_mpflag( arg4 );
         if( mptype == -1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( obj->pIndexData->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( obj->pIndexData->progtypes, mprg->type );
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object has no obj programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = obj->pIndexData->mudprogs;
         obj->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( obj->pIndexData->progtypes, mptype );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That object has no obj programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg4 );
      mptype = get_mpflag( arg4 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( obj->pIndexData->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         obj->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( obj->pIndexData->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "add" ) )
   {
      mptype = get_mpflag( arg3 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( mprog != NULL )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         obj->pIndexData->mudprogs = mprg;
      xSET_BIT( obj->pIndexData->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_opedit( ch, "" );
}

/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument )
{
   if( mptype != -1 )
   {
      mprg->type = mptype;
      if( mprg->arglist )
         STRFREE( mprg->arglist );
      mprg->arglist = STRALLOC( argument );
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if( !mprg->comlist )
      mprg->comlist = STRALLOC( "" );
   start_editing( ch, mprg->comlist );
   return;
}

void do_rpedit( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't rpedit"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_RESTRICTED:
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't use this command from within another command."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Fatal error: report to smaugmuds.afkmods.com"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            bug( "%s", "do_opedit: sub_oprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ( MPROG_DATA * ) ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   value = atoi( arg2 );
   /*
    * argument = one_argument( argument, arg3 ); 
    */

   if( arg1[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: rpedit <command> [number] <program> <value>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Command being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  add delete insert edit list"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand sleep rest rfight entry"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  leave death"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You should be standing in the room you wish to edit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_rmodify( ch, ch->in_room ) )
      return;

   mprog = ch->in_room->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg1, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This room has no room programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( value < 1 )
      {
         if( strcmp( "full", arg2 ) )
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d>%s %s", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            }

            return;
         }
         else
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            }

            return;
         }
      }

      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            ch_printf( ch, "%d>%s %s\r\n%s\r\n", cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            break;
         }
      }

      if( !mprg )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}

      return;
   }

   if( !str_cmp( arg1, "edit" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This room has no room programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg3 );
      if( arg3[0] != '\0' )
      {
         mptype = get_mpflag( arg3 );
         if( mptype == -1 )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( ch->in_room->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( ch->in_room->progtypes, mprg->type );
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg1, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room has no room programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg3 );
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = ch->in_room->mudprogs;
         ch->in_room->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( ch->in_room->progtypes, mptype );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program removed."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That room has no room programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      argument = one_argument( argument, arg3 );
      mptype = get_mpflag( arg2 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value < 1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( ch->in_room->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         ch->in_room->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( ch->in_room->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg1, "add" ) )
   {
      mptype = get_mpflag( arg2 );
      if( mptype == -1 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Unknown program type."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( mprog )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         ch->in_room->mudprogs = mprg;
      xSET_BIT( ch->in_room->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_rpedit( ch, "" );
}

void do_rdelete( CHAR_DATA* ch, const char* argument)
{
   ROOM_INDEX_DATA *location;

   if( ch->substate == SUB_RESTRICTED )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that while in a subprompt."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Delete which room?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Find the room. 
    */
   if( !( location = find_location( ch, argument ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such location."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Does the player have the right to delete this room? 
    */
   if( !can_rmodify( ch, location ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't delete this room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   delete_room( location );
   fix_exits(  ); /* Need to call this to solve a crash */
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Room %s has been deleted.", argument); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_odelete( CHAR_DATA* ch, const char* argument )
{
   OBJ_INDEX_DATA *obj;
   int vnum;

   if( ch->substate == SUB_RESTRICTED )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that while in a subprompt."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Delete which object?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !is_number( argument ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must specify the object's vnum to delete it."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( argument );

   /*
    * Find the obj. 
    */
   if( !( obj = get_obj_index( vnum ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Does the player have the right to delete this object?
    */
   if( get_trust( ch ) < sysdata.level_modify_proto
       && ( obj->vnum < ch->pcdata->area->low_o_vnum || obj->vnum > ch->pcdata->area->hi_o_vnum ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That obj is not in your assigned range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   delete_obj( obj );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Object %d has been deleted.", vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

void do_mdelete( CHAR_DATA* ch, const char* argument)
{
   MOB_INDEX_DATA *mob;
   int vnum;

   if( ch->substate == SUB_RESTRICTED )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that while in a subprompt."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Delete which mob?"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !is_number( argument ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You must specify the mob's vnum to delete it."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( argument );

   /*
    * Find the mob. 
    */
   if( !( mob = get_mob_index( vnum ) ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such mob."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   /*
    * Does the player have the right to delete this mob? 
    */
   if( get_trust( ch ) < sysdata.level_modify_proto
       && ( mob->vnum < ch->pcdata->area->low_m_vnum || mob->vnum > ch->pcdata->area->hi_m_vnum ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mob is not in your assigned range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   delete_mob( mob );
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Mob %d has been deleted.", vnum); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}

/*  
 *  Mobile and Object Program Copying 
 *  Last modified Feb. 24 1999
 *  Mystaric
 */
void mpcopy( MPROG_DATA * source, MPROG_DATA * destination )
{
   destination->type = source->type;
   destination->triggered = source->triggered;
   destination->resetdelay = source->resetdelay;
   destination->arglist = STRALLOC( source->arglist );
   destination->comlist = STRALLOC( source->comlist );
   destination->next = NULL;
}

void do_opcopy( CHAR_DATA* ch, const char* argument)
{
   char sobj[MAX_INPUT_LENGTH];
   char prog[MAX_INPUT_LENGTH];
   char num[MAX_INPUT_LENGTH];
   char dobj[MAX_INPUT_LENGTH];
   OBJ_DATA *source = NULL, *destination = NULL;
   MPROG_DATA *source_oprog = NULL, *dest_oprog = NULL, *source_oprg = NULL, *dest_oprg = NULL;
   int value = -1, optype = -1, cnt = 0;
   bool COPY = FALSE;

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't opcopy"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, sobj );
   argument = one_argument( argument, prog );

   if( sobj[0] == '\0' || prog[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: opcopy <source object> <program> [number] <destination object>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "        opcopy <source object> all <destination object>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "        opcopy <source object> all <destination object> <program>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand wear remove sac zap get"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  drop damage repair greet exa use"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  pull push (for levers,pullchains,buttons)"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Object should be in your inventory to edit."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !strcmp( prog, "all" ) )
   {
      argument = one_argument( argument, dobj );
      argument = one_argument( argument, prog );
      optype = get_mpflag( prog );
      COPY = TRUE;
   }
   else
   {
      argument = one_argument( argument, num );
      argument = one_argument( argument, dobj );
      value = atoi( num );
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      if( ( source = get_obj_carry( ch, sobj ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't carrying source object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( destination = get_obj_carry( ch, dobj ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You aren't carrying destination object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      if( ( source = get_obj_world( ch, sobj ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't find source object in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( destination = get_obj_world( ch, dobj ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't find destination object in all the realms."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( source == destination )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source and destination objects cannot be the same"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }


   if( !can_omodify( ch, destination ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify destination object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !IS_OBJ_STAT( destination, ITEM_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Destination object must have prototype flag."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   set_char_color( AT_PLAIN, ch );

   source_oprog = source->pIndexData->mudprogs;
   dest_oprog = destination->pIndexData->mudprogs;



   set_char_color( AT_GREEN, ch );

   if( !source_oprog )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source object has no mob programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( COPY )
   {
      for( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
      {
         if( optype == source_oprg->type || optype == -1 )
         {
            if( dest_oprog != NULL )
               for( ; dest_oprog->next; dest_oprog = dest_oprog->next );
            CREATE( dest_oprg, MPROG_DATA, 1 );
            if( dest_oprog )
               dest_oprog->next = dest_oprg;
            else
            {
               destination->pIndexData->mudprogs = dest_oprg;
               dest_oprog = dest_oprg;
            }
            mpcopy( source_oprg, dest_oprg );
            xSET_BIT( destination->pIndexData->progtypes, dest_oprg->type );
            cnt++;
         }
      }

      if( cnt == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No such program in source object"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d programs successfully copied from %s to %s.", cnt, sobj, dobj); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( value < 1 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   optype = get_mpflag( prog );

   for( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
   {
      if( ++cnt == value && source_oprg->type == optype )
      {
         if( dest_oprog != NULL )
            for( ; dest_oprog->next; dest_oprog = dest_oprog->next );
         CREATE( dest_oprg, MPROG_DATA, 1 );
         if( dest_oprog )
            dest_oprog->next = dest_oprg;
         else
            destination->pIndexData->mudprogs = dest_oprg;
         mpcopy( source_oprg, dest_oprg );
         xSET_BIT( destination->pIndexData->progtypes, dest_oprg->type );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s program %d from %s successfully copied to %s.", prog, value, sobj, dobj); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   if( !source_oprg )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source object."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   do_opcopy( ch, "" );
}

void do_mpcopy( CHAR_DATA* ch, const char* argument)
{
   char smob[MAX_INPUT_LENGTH];
   char prog[MAX_INPUT_LENGTH];
   char num[MAX_INPUT_LENGTH];
   char dmob[MAX_INPUT_LENGTH];
   CHAR_DATA *source = NULL, *destination = NULL;
   MPROG_DATA *source_mprog = NULL, *dest_mprog = NULL, *source_mprg = NULL, *dest_mprg = NULL;
   int value = -1, mptype = -1, cnt = 0;
   bool COPY = FALSE;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't opcop"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, smob );
   argument = one_argument( argument, prog );

   if( smob[0] == '\0' || prog[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: mpcopy <source mobile> <program> [number] <destination mobile>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "        mpcopy <source mobile> all <destination mobile>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "        mpcopy <source mobile> all <destination mobile> <program>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand fight hitprcnt greet allgreet"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  entry give bribe death time hour script"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  greetinfight"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !strcmp( prog, "all" ) )
   {
      argument = one_argument( argument, dmob );
      argument = one_argument( argument, prog );
      mptype = get_mpflag( prog );
      COPY = TRUE;
   }
   else
   {
      argument = one_argument( argument, num );
      argument = one_argument( argument, dmob );
      value = atoi( num );
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      if( ( source = get_char_room( ch, smob ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source mobile is not present."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( destination = get_char_room( ch, dmob ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Destination mobile is not present."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   else
   {
      if( ( source = get_char_world( ch, smob ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't find source mobile"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ( destination = get_char_world( ch, dmob ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Can't find destination mobile"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }
   if( source == destination )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source and destination mobiles cannot be the same"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( get_trust( ch ) < source->level || !IS_NPC( source ) ||
       get_trust( ch ) < destination->level || !IS_NPC( destination ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You can't do that!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_mmodify( ch, destination ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify destination mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !xIS_SET( destination->act, ACT_PROTOTYPE ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Destination mobile must have a prototype flag to mpcopy."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   source_mprog = source->pIndexData->mudprogs;
   dest_mprog = destination->pIndexData->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !source_mprog )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source mobile has no mob programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }


   if( COPY )
   {
      for( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
      {
         if( mptype == source_mprg->type || mptype == -1 )
         {
            if( dest_mprog != NULL )
               for( ; dest_mprog->next; dest_mprog = dest_mprog->next );
            CREATE( dest_mprg, MPROG_DATA, 1 );

            if( dest_mprog )
               dest_mprog->next = dest_mprg;
            else
            {
               destination->pIndexData->mudprogs = dest_mprg;
               dest_mprog = dest_mprg;
            }
            mpcopy( source_mprg, dest_mprg );
            xSET_BIT( destination->pIndexData->progtypes, dest_mprg->type );
            cnt++;
         }
      }

      if( cnt == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "No such program in source mobile"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d programs successfully copied from %s to %s.", cnt, smob, dmob); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( value < 1 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mptype = get_mpflag( prog );

   for( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
   {
      if( ++cnt == value && source_mprg->type == mptype )
      {
         if( dest_mprog != NULL )
            for( ; dest_mprog->next; dest_mprog = dest_mprog->next );
         CREATE( dest_mprg, MPROG_DATA, 1 );
         if( dest_mprog )
            dest_mprog->next = dest_mprg;
         else
            destination->pIndexData->mudprogs = dest_mprg;
         mpcopy( source_mprg, dest_mprg );
         xSET_BIT( destination->pIndexData->progtypes, dest_mprg->type );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s program %d from %s successfully copied to %s.", prog, value, smob, dmob); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !source_mprg )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source mobile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   do_mpcopy( ch, "" );
}

void do_rpcopy( CHAR_DATA* ch, const char* argument)
{
   char sroom[MAX_INPUT_LENGTH];
   char prog[MAX_INPUT_LENGTH];
   char num[MAX_INPUT_LENGTH];
   char droom[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *source = NULL, *destination = NULL;
   MPROG_DATA *source_rprog = NULL, *dest_rprog = NULL, *source_rprg = NULL, *dest_rprg = NULL;
   int value = -1, mptype = -1, cnt = 0;
   bool COPY = FALSE;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mob's can't rpcopy."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->desc )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have no descriptor!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, sroom );
   argument = one_argument( argument, prog );

   if( !is_number( sroom ) || prog[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Syntax: rpcopy <source rvnum> <program> [number] <destination rvnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  rpcopy <source rvnum> all <destination rvnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  rpcopy <source rvnum> all <destination rvnum> <program>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Program being one of:"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  act speech rand fight hitprcnt greet allgreet"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  entry give bribe death time hour script"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( prog, "all" ) )
   {
      argument = one_argument( argument, droom );
      argument = one_argument( argument, prog );
      mptype = get_mpflag( prog );
      COPY = TRUE;
   }
   else
   {
      argument = one_argument( argument, num );
      argument = one_argument( argument, droom );
      value = atoi( num );
   }

   if( ( source = get_room_index( atoi( sroom ) ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source room does not exist."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !is_number( droom ) || ( destination = get_room_index( atoi( droom ) ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Destination room does not exist."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( source == destination )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source and destination rooms cannot be the same."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_rmodify( ch, destination ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot modify destination room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   source_rprog = source->mudprogs;
   dest_rprog = destination->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !source_rprog )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Source room has no programs."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( COPY )
   {
      for( source_rprg = source_rprog; source_rprg; source_rprg = source_rprg->next )
      {
         if( mptype == source_rprg->type || mptype == -1 )
         {
            if( dest_rprog != NULL )
               for( ; dest_rprog->next; dest_rprog = dest_rprog->next );
            CREATE( dest_rprg, MPROG_DATA, 1 );

            if( dest_rprog )
               dest_rprog->next = dest_rprg;
            else
            {
               destination->mudprogs = dest_rprg;
               dest_rprog = dest_rprg;
            }

            mpcopy( source_rprg, dest_rprg );
            xSET_BIT( destination->progtypes, dest_rprg->type );
            cnt++;
         }
      }

      if( cnt == 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d programs successfully copied from %s to %s.", cnt, sroom, droom); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( value < 1 )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   mptype = get_mpflag( prog );

   for( source_rprg = source_rprog; source_rprg; source_rprg = source_rprg->next )
   {
      if( ++cnt == value && source_rprg->type == mptype )
      {
         if( dest_rprog != NULL )
            for( ; dest_rprog->next; dest_rprog = dest_rprog->next );
         CREATE( dest_rprg, MPROG_DATA, 1 );
         if( dest_rprog )
            dest_rprog->next = dest_rprg;
         else
            destination->mudprogs = dest_rprg;
         mpcopy( source_rprg, dest_rprg );
         xSET_BIT( destination->progtypes, dest_rprg->type );
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s program %d from %s successfully copied to %s.", prog, value, sroom, droom); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
   }

   if( !source_rprg )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "No such program in source room."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   do_rpcopy( ch, "" );
}

/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelCreate( relation_type tp, void *actor, void *subject )
{
   REL_DATA *tmp;

   if( tp < relMSET_ON || tp > relOSET_ON )
   {
      bug( "%s: invalid type (%d)", __func__, tp );
      return;
   }

   if( !actor )
   {
      bug( "%s: NULL actor", __func__ );
      return;
   }

   if( !subject )
   {
      bug( "%s: NULL subject", __func__ );
      return;
   }

   for( tmp = first_relation; tmp; tmp = tmp->next )
      if( tmp->Type == tp && tmp->Actor == actor && tmp->Subject == subject )
      {
         bug( "%s: duplicated relation", __func__ );
         return;
      }

   CREATE( tmp, REL_DATA, 1 );
   tmp->Type = tp;
   tmp->Actor = actor;
   tmp->Subject = subject;
   LINK( tmp, first_relation, last_relation, next, prev );
}

/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelDestroy( relation_type tp, void *actor, void *subject )
{
   REL_DATA *rq;

   if( tp < relMSET_ON || tp > relOSET_ON )
   {
      bug( "%s: invalid type (%d)", __func__, tp );
      return;
   }

   if( !actor )
   {
      bug( "%s: NULL actor", __func__ );
      return;
   }

   if( !subject )
   {
      bug( "%s: NULL subject", __func__ );
      return;
   }

   for( rq = first_relation; rq; rq = rq->next )
      if( rq->Type == tp && rq->Actor == actor && rq->Subject == subject )
      {
         UNLINK( rq, first_relation, last_relation, next, prev );
         /*
          * Dispose will also set to NULL the passed parameter 
          */
         DISPOSE( rq );
         break;
      }
}

void do_findexit( CHAR_DATA* ch, const char* argument)
{
   ROOM_INDEX_DATA *room;
   EXIT_DATA *pexit;
   int exvnum, cou, h_range, l_range;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   AREA_DATA *tarea;

   static const char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
       || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YYou don't have an assigned area."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg1[0] != '\0' && !is_number( arg1 ) )
      return;

   if( ( arg2[0] != '\0' ) && ( str_cmp( arg2, "mud" ) && !is_number( arg2 ) ) )
      return;
   else if( arg3[0] != '\0' && !is_number( arg3 ) )
      return;

   exvnum = atoi( arg1 );
   if( !str_cmp( arg2, "mud" ) && get_trust( ch ) >= LEVEL_GREATER )
   {
      l_range = 1;
      h_range = MAX_VNUM;
   }
   else
   {
      if( tarea )
      {
         if( arg2[0] == '\0' )
            l_range = tarea->low_r_vnum;
         else
            l_range = atoi( arg2 );

         if( arg3[0] == '\0' )
            h_range = tarea->hi_r_vnum;
         else
            h_range = atoi( arg3 );

         if( ( l_range < tarea->low_r_vnum || h_range > tarea->hi_r_vnum ) && get_trust( ch ) < LEVEL_GREATER )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThat is out of your vnum range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }
      }
      else
      {
         l_range = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
         h_range = ( is_number( arg3 ) ? atoi( arg3 ) : 1 );
      }
   }

   for( cou = 0; cou < MAX_KEY_HASH; cou++ )
   {
      if( room_index_hash[cou] )
         for( room = room_index_hash[cou]; room; room = room->next )
         {
            if( room->vnum >= l_range && room->vnum <= h_range && room->first_exit )
            {
               for( pexit = room->first_exit; pexit; pexit = pexit->next )
               {
                  if( pexit->to_room && pexit->to_room->vnum == exvnum )
                     pager_printf( ch, "%5d) Direction: %-2s.  Key: %d Flags: %d Keywords: %s.\n\r",
                                   room->vnum,
                                   dir_text[pexit->vdir],
                                   pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
               }
            }
         }
   }
   return;
}

/* Lists all exits that links the specified area with others.
 * Idea for it after a talk with Iliana - Luc 11/2000
 * Modified to include a "reverse" option similar to Smaug 1.8, plus provided
 * a colorized version.  Slightly reworked the logic too. - Luc 06/2007
 */
void do_alinks( CHAR_DATA* ch, const char* argument)
{
   static const char *dirs[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "--" };
   char buf[MAX_INPUT_LENGTH];
   AREA_DATA *area;
   AREA_DATA *target;
   ROOM_INDEX_DATA *room;
   EXIT_DATA *rexit;
   EXT_BV fmask;
   EXT_BV dummy;
   int rnum, dnum, count, detail;
   bool rev = FALSE;
   bool ex_found;
   bool rm_found;

   argument = one_argument( argument, buf );
   if( !buf[0] )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&wPlease see the related helpfile."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   for( area = first_area_name, target = NULL; area; area = area->next_sort_name )
   {
      if( IS_SET( area->status, AREA_DELETED ) )
         continue;
      if( !str_cmp( area->filename, buf ) )
      {
         target = area;
         break;
      }
   }

   if( !target )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&wNo areas found with the supplied name."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   count = detail = 0;
   if( *argument != '\0' )
   {
      if( !str_prefix( argument, "detailed" ) )
         detail = 1;
      else if( !str_prefix( argument, "names" ) )
         detail = 2;
      else if( !str_prefix( argument, "reverse" ) )
         rev = TRUE, detail = 1;
   }
   ch_printf( ch, "&wListing all links from &W%s &wto &W%s&w...\r\n\r\n",
              rev ? "&wthe world" : area->name, rev ? area->name : "&wthe world" );

   xCLEAR_BITS( fmask );
   xSET_BIT( fmask, ROOM_NO_MOB );
   xSET_BIT( fmask, ROOM_PRIVATE );
   xSET_BIT( fmask, ROOM_SOLITARY );
   xSET_BIT( fmask, ROOM_NOFLOOR );
   xSET_BIT( fmask, ROOM_NOMISSILE );
   xSET_BIT( fmask, ROOM_DND );
   xSET_BIT( fmask, ROOM_PROTOTYPE );

   for( area = first_area_name; area; area = area->next_sort_name )
   {
      if( IS_SET( area->status, AREA_DELETED ) )
         continue;
      if( rev ? area == target : area != target )
         continue;
      rm_found = FALSE;
      for( rnum = area->low_r_vnum; rnum <= area->hi_r_vnum; rnum++ )
      {
         room = get_room_index( rnum );
         if( !room )
            continue;
         ex_found = FALSE;
         for( rexit = room->first_exit; rexit; rexit = rexit->next )
         {
            if( rexit->to_room && (rev ? rexit->to_room->area == target : rexit->to_room->area != target) )
            {
               /*
                * exit is an inter-area link involving the target area 
                */
               count++;
               if( !ex_found )
               {
                  /*
                   * print room's data only once, the same with area names 
                   */
                  if( !rm_found && rev )
                     ch_printf( ch, "&W%s &c- &W%s\r\n\r\n", area->filename, area->name );
                  ch_printf( ch, "  &G%d &c- &w%s", room->vnum, room->name );
                  if( xHAS_BITS( room->room_flags, fmask ) )
                  {
                     int i;
                     for( i = 0; i < XBI; i++ )
                        dummy.bits[i] = room->room_flags.bits[i] & fmask.bits[i];
                     ch_printf( ch, " &c- &O%s", ext_flag_string( &dummy, r_flags ) );
                  }
                  send_to_char( "\r\n", ch );
               }
               dnum = UMIN( rexit->vdir, 10 );
               if( detail == 0 )
                  ch_printf( ch, "%s&C%s:&g%d&O%s%s%s", ex_found ? " " : "    ", dirs[dnum], rexit->vnum,
                             xIS_EMPTY( rexit->to_room->progtypes ) ? "" : "p", rexit->key <= 0 ? "" : "k",
                             rexit->exit_info ? "f" : "" );
               else if( detail == 1 )
               {
                  ch_printf( ch, "    &C%-2s&c:&g%d&O%s &c- %s%s", dirs[dnum], rexit->vnum,
                             xIS_EMPTY( rexit->to_room->progtypes ) ? "" : "p", rev ? "&w" : "&W",
                             rev ? rexit->to_room->name : rexit->to_room->area->name );
                  if( rexit->exit_info )
                     ch_printf( ch, " &c- &O%s", flag_string( rexit->exit_info, ex_flags ) );
                  if( rexit->key > 0 )
                     ch_printf( ch, " &ckey:&W%d", rexit->key );
                  send_to_char( "\r\n", ch );
               }
               else
                  {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "    &C%-2s&c:&g%d&O%s%s%s &c- &W%s &c- &w%s", dirs[dnum], rexit->vnum, xIS_EMPTY( rexit->to_room->progtypes ) ? "" : "p", rexit->key <= 0 ? "" : "k", rexit->exit_info ? "f" : "", rexit->to_room->area->name, rexit->to_room->name); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
               ex_found = TRUE;
            }
         }

         if( ex_found )
         {
            if( detail == 0 )
               send_to_char( "\r\n", ch );
            rm_found = TRUE;
         }
      }
      if( rm_found )
         send_to_char( "\r\n", ch );
   }
   if( count )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&wFound &C%d&w link(s).", count); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   else
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&wNo links exists."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
}
