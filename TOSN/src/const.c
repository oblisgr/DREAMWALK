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
 *                           Mud constants module                           *
 ****************************************************************************/

#include <stdio.h>
#include "mud.h"

const char *const npc_race			[MAX_NPC_RACE]							= {
	/*000-009*/ "human"	   	 , "elf"	   , "dwarf"   , "halfling" , "pixie"	   , "vampire"	 , "half-ogre"   , "half-orc" , "half-troll", "half-elf" , 
	/*010-019*/ "gith"	   	 , "drow"	   , "sea-elf" , "lizardman", "gnome"	   , "r5"		 , "r6"		     , "r7"	      , "r8"		, "r9"		 ,
	/*020-029*/ "troll"	     , "ant"	   , "ape"	   , "baboon"   , "bat"		   , "bear"	     , "bee"		 , "beetle"   , "boar"		, "bugbear"	 , 
	/*030-039*/ "cat"		 , "dog"	   , "dragon"  , "ferret"   , "fly"		   , "gargoyle"  , "gelatin"	 , "ghoul"	  , "gnoll"		, "goblin"	 , 
	/*040-049*/ "golem"	     , "gorgon"	   , "harpy"   , "hobgoblin", "kobold"	   , "locust"	 , "lycanthrope" , "minotaur" , "mold"		, "mule"	 , 
	/*050-059*/ "neanderthal", "ooze"	   , "orc"	   , "rat"	    , "rustmonster", "shadow"	 , "shapeshifter", "shrew"	  , "shrieker"	, "skeleton" , 
	/*060-069*/ "slime"	     , "snake"	   , "spider"  , "stirge"   , "thoul"	   , "troglodyte", "undead"	     , "wight"	  , "wolf"		, "worm"	 , 
	/*070-079*/ "zombie"	 , "bovine"	   , "canine"  , "feline"   , "porcine"	   , "mammal"	 , "rodent"	     , "avis"	  , "reptile"	, "amphibian", 
	/*080-089*/ "fish"	   	 , "crustacean", "insect"  , "spirit"   , "magical"	   , "horse"	 , "animal"	     , "humanoid" , "monster"	, "god"		 , 
	/*090-090*/ "???"		 ,

	/*091-099*/ "altmer"	 , "argonian"  , "bosmer"  , "breton"   , "dunmer"     , "imperial"  , "khajiit"     ,"nord"      , "redguard" ,
	/*100-106*/ "construct"  , "daedra"    , "beast"   , "undead"   , "atronach"   , "athropod"  , "corprus"	 ,

	/*107-113*/ "pandaren"   , "tauren"    , "bloodelf", "nightelf" , "worgen"     , "draenei"   ,"forsaken"
};
const char *const npc_class			[MAX_NPC_CLASS]							= {
	// SMAUG CLASSES (DONT TOUCH THEM) //
	//-----------------------------------------------------------------------------//
	/*00,01,02*/ "cleric"			, "vampire"				, "druid"				,
	/*03,04,05*/ "ranger"			, "augurer"				, "paladin"				,
	/*06,07,08*/ "nephandi"			, "savage"				, "pc11"				,
	/*09,10,11*/ "pc12"				, "pc13"				, "pc14"				,
	/*12,13,14*/ "pc15"				, "pc16"				, "pc17"				,
	/*15,16,17*/ "pc18"				, "pc19"				, "baker"				,
	/*18,19,20*/ "butcher"			, "blacksmith"			, "mayor"				,
	/*21,22,XX*/ "king"				, "queen"				, 
	
	/*23,24,25*/ "archer"			, "crusader"		   	, "knight"				,
	/*26,27,28*/ "rogue"			, "scout"			   	, "warrior"				,
	/*29,30,31*/ "champion"			, "barbarian"		   	, "smuggler" 			,
	/*32,33,34*/ "smith" 			, "miner" 			   	, "leatherworker"		,
	/*35,36,37*/ "woodcutter"		, "carpenter"		   	, "acrobat"				,
	/*38,39,40*/ "assassin"			, "bard"			  	, "agent"				,
	/*41,42,43*/ "monk"				, "pilgrim"			   	, "thief"				,
	/*44,45,46*/ "enforcer"			, "hunter"			   	, "farmer"				,
	/*47,48,49*/ "trader"			, "weaver"			   	, "publican"	    	,
	/*50,51,52*/ "artisan"			, "battlemage"		   	, "healer"				,
	/*53,54,55*/ "mage"				, "nightblade"		   	, "priest"				,
	/*56,57,58*/ "sorcerer"			, "spellsword"		   	, "warlock"				,
	/*59,60,61*/ "alchemist"		, "apothecary"		   	, "enchanter"			,
	/*62,63,64*/ "savant"			, "clothier"		   	, "scribe"				,
	
	/*65,66,67*/ "commoner"			, "noble"		   		, "slave"				,
	/*68,69,70*/ "witchhunter"		, "dreamer"				, "witch"				,
	/*71,72,73*/ "pauper" 			, "gladiator"			, "bandit"				,
	/*74,75,76*/ "guard"			, "herder"				, "marauder"			,
	/*77,78,79*/ "pirate" 			, "reserved1"			, "reserved2"
	/*80,81,82*/ "combat_expert" 	, "stealth_specialist" 	, "magic_user"
};

const struct str_app_type str_app	[26]									= {
   {-5,  -9,    0,    0},  /* 0  */
   {-5,  -8,   25,   10},  /* 1  */
   {-3,  -7,   50,   10},
   {-3,  -6,   75,   15},  /* 3  */
   {-2,  -5,  100,   20},
   {-2,  -4,  125,   25},  /* 5  */
   {-1,  -3,  150,   30},
   {-1,  -2,  175,   35},
   { 0,  -1,  200,   40},
   { 0,   0,  225,   45},
   { 0,   0,  250,   50},  /* 10  */
   { 0,   0,  275,   55},
   { 0,   0,  300,   60},
   { 1,   0,  325,   65},  /* 13  */
   { 1,   1,  350,   70},
   { 2,   2,  375,   75},  /* 15  */
   { 2,   3,  400,   80},
   { 3,   4,  425,   85},
   { 3,   5,  450,   90},  /* 18  */
   { 4,   6,  475,   95},
   { 4,   7,  500,  100},  /* 20  */
   { 5,   8,  525,  105},
   { 5,   9,  550,  110},
   { 6,  10,  575,  115},
   { 6,  11,  600,  120},
   { 7,  12,  625,  125}   /* 25   */
};
const struct int_app_type int_app	[26]				 					= {
   { 3},  /*  0 */
   { 5},  /*  1 */
   {10},
   {15},  /*  3 */
   {20},
   {25},  /*  5 */
   {30}, 
   {35},
   {40},
   {45},
   {50},  /* 10 */
   {55},
   {60},
   {65},
   {70},
   {75},  /* 15 */
   {80},
   {85},
   {90},  /* 18 */
   {95},
   {99},  /* 20 */
   {99},
   {99},
   {99},
   {99},
   {99}   /* 25 */
};
const struct wis_app_type wis_app	[26] 									= {
   {0},  /*  0 */
   {0},  /*  1 */
   {0},
   {0},  /*  3 */
   {0},
   {1},  /*  5 */
   {1},
   {1},
   {1},
   {2},
   {2},  /* 10 */
   {2},
   {2},
   {2},
   {2},
   {3},  /* 15 */
   {3},
   {4},
   {5},  /* 18 */
   {5},
   {5},  /* 20 */
   {6},
   {6},
   {6},
   {6},
   {7}   /* 25 */
};
const struct dex_app_type dex_app	[26] 									= {
   { 45},/* 0 */
   { 40},/* 1 */
   { 35},
   { 30},
   { 25},
   { 20},/* 5 */
   { 15},
   { 10},
   {  5},
   {  0},
   {  0},/* 10 */
   { -5},
   {-10},
   {-15},
   {-20},
   {-30},/* 15 */
   {-35},
   {-40},
   {-45},
   {-50},
   {-55},/* 20 */
   {-60},
   {-65},
   {-70},
   {-75},
   {-80} /* 25 */
};
const struct con_app_type con_app	[26] 									= {
   {-9, 20},/*  0 */
   {-8, 25},/*  1 */
   {-7, 30},
   {-6, 35},/*  3 */
   {-5, 40},
   {-4, 45},/*  5 */
   {-3, 50},
   {-2, 55},
   {-1, 60},
   { 0, 65},
   { 0, 70},/* 10 */
   { 1, 75},
   { 2, 80},
   { 3, 85},
   { 4, 88},
   { 5, 90},/* 15 */
   { 6, 95},
   { 7, 97},
   { 8, 99},/* 18 */
   { 9, 99},
   {10, 99},/* 20 */
   {11, 99},
   {12, 99},
   {13, 99},
   {14, 99},
   {15, 99} /* 25 */
};
const struct cha_app_type cha_app	[26] 									= {
   {-60},   /* 0 */
   {-50},   /* 1 */
   {-50},
   {-40},
   {-30},
   {-20},   /* 5 */
   {-10},
   {-5},
   {-1},
   {0},
   {0},  /* 10 */
   {0},
   {0},
   {0},
   {1},
   {5},  /* 15 */
   {10},
   {20},
   {30},
   {40},
   {50}, /* 20 */
   {60},
   {70},
   {80},
   {90},
   {99}  /* 25 */
};
const struct lck_app_type lck_app	[26] 									= {
   {60}, /* 0 */
   {50}, /* 1 */
   {50},
   {40},
   {30},
   {20}, /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0},  /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10},   /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50},   /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120}   /* 25 */
};
const char *const attack_table		[18] 									= {
   "hit",
   "slice", "stab", "slash", "whip", "claw",
   "blast", "pound", "crush", "grep", "bite",
   "pierce", "suction", "bolt", "arrow", "dart",
   "stone", "pea"
};
const char *s_blade_messages		[24] 									= {
   "miss", "barely scratch", "scratch", "nick", "cut", "hit", "tear",
   "rip", "gash", "lacerate", "hack", "maul", "rend", "decimate",
   "mangle", "devastate", "cleave", "butcher", "disembowel",
   "disfigure", "gut", "eviscerate", "slaughter", "annihilate"
};
const char *p_blade_messages		[24] 									= {
   "misses", "barely scratches", "scratches", "nicks", "cuts", "hits", 
   "tears", "rips", "gashes", "lacerates", "hacks", "mauls", "rends", 
   "decimates", "mangles", "devastates", "cleaves", "butchers", 
   "disembowels", "disfigures", "guts", "eviscerates", "slaughters", 
   "annihilates"
};
const char *s_blunt_messages		[24] 									= {
   "miss", "barely scratch", "scratch", "nick", "cut", "hit", "tear",
   "rip", "gash", "lacerate", "hack", "maul", "rend", "decimate",
   "mangle", "devastate", "cleave", "butcher", "disembowel",
   "disfigure", "gut", "eviscerate", "slaughter", "annihilate"
};
const char *p_blunt_messages		[24] 									= {
   "misses", "barely scratches", "scratches", "nicks", "cuts", "hits", 
   "tears", "rips", "gashes", "lacerates", "hacks", "mauls", "rends", 
   "decimates", "mangles", "devastates", "cleaves", "butchers", 
   "disembowels", "disfigures", "guts", "eviscerates", "slaughters", 
   "annihilates"
};
const char *s_generic_messages		[24] 									= {
   "miss", "barely scratch", "scratch", "nick", "cut", "hit", "tear",
   "rip", "gash", "lacerate", "hack", "maul", "rend", "decimate",
   "mangle", "devastate", "cleave", "butcher", "disembowel",
   "disfigure", "gut", "eviscerate", "slaughter", "annihilate"
};
const char *p_generic_messages		[24] 									= {
   "misses", "barely scratches", "scratches", "nicks", "cuts", "hits", 
   "tears", "rips", "gashes", "lacerates", "hacks", "mauls", "rends", 
   "decimates", "mangles", "devastates", "cleaves", "butchers", 
   "disembowels", "disfigures", "guts", "eviscerates", "slaughters", 
   "annihilates"
};
const char **const s_message_table	[18] 									= {
   s_generic_messages,  /* hit */
   s_blade_messages, /* slice */
   s_blade_messages, /* stab */
   s_blade_messages, /* slash */
   s_blunt_messages, /* whip */
   s_blade_messages, /* claw */
   s_generic_messages,  /* blast */
   s_blunt_messages, /* pound */
   s_blunt_messages, /* crush */
   s_generic_messages,  /* grep */
   s_blade_messages, /* bite */
   s_blade_messages, /* pierce */
   s_blunt_messages, /* suction */
   s_generic_messages,  /* bolt */
   s_generic_messages,  /* arrow */
   s_generic_messages,  /* dart */
   s_generic_messages,  /* stone */
   s_generic_messages   /* pea */
};
const char **const p_message_table	[18] 									= {
   p_generic_messages,  /* hit */
   p_blade_messages, /* slice */
   p_blade_messages, /* stab */
   p_blade_messages, /* slash */
   p_blunt_messages, /* whip */
   p_blade_messages, /* claw */
   p_generic_messages,  /* blast */
   p_blunt_messages, /* pound */
   p_blunt_messages, /* crush */
   p_generic_messages,  /* grep */
   p_blade_messages, /* bite */
   p_blade_messages, /* pierce */
   p_blunt_messages, /* suction */
   p_generic_messages,  /* bolt */
   p_generic_messages,  /* arrow */
   p_generic_messages,  /* dart */
   p_generic_messages,  /* stone */
   p_generic_messages   /* pea */
};
const char *const temp_settings		[MAX_CLIMATE] 							= {
   "cold",
   "cool",
   "normal",
   "warm",
   "hot",
};
const char *const precip_settings	[MAX_CLIMATE] 							= {
   "arid",
   "dry",
   "normal",
   "damp",
   "wet",
};
const char *const wind_settings		[MAX_CLIMATE] 							= {
   "still",
   "calm",
   "normal",
   "breezy",
   "windy",
};
const char *const preciptemp_msg	[6][6] 									= {
   /*
    * precip = 0 
    */
   {
    "Frigid temperatures settle over the land",
    "It is bitterly cold",
    "The weather is crisp and dry",
    "A comfortable warmth sets in",
    "A dry heat warms the land",
    "Seething heat bakes the land"},
   /*
    * precip = 1 
    */
   {
    "A few flurries drift from the high clouds",
    "Frozen drops of rain fall from the sky",
    "An occasional raindrop falls to the ground",
    "Mild drops of rain seep from the clouds",
    "It is very warm, and the sky is overcast",
    "High humidity intensifies the seering heat"},
   /*
    * precip = 2 
    */
   {
    "A brief snow squall dusts the earth",
    "A light flurry dusts the ground",
    "Light snow drifts down from the heavens",
    "A light drizzle mars an otherwise perfect day",
    "A few drops of rain fall to the warm ground",
    "A light rain falls through the sweltering sky"},
   /*
    * precip = 3 
    */
   {
    "Snowfall covers the frigid earth",
    "Light snow falls to the ground",
    "A brief shower moistens the crisp air",
    "A pleasant rain falls from the heavens",
    "The warm air is heavy with rain",
    "A refreshing shower eases the oppresive heat"},
   /*
    * precip = 4 
    */
   {
    "Sleet falls in sheets through the frosty air",
    "Snow falls quickly, piling upon the cold earth",
    "Rain pelts the ground on this crisp day",
    "Rain drums the ground rythmically",
    "A warm rain drums the ground loudly",
    "Tropical rain showers pelt the seering ground"},
   /*
    * precip = 5 
    */
   {
    "A downpour of frozen rain covers the land in ice",
    "A blizzard blankets everything in pristine white",
    "Torrents of rain fall from a cool sky",
    "A drenching downpour obscures the temperate day",
    "Warm rain pours from the sky",
    "A torrent of rain soaks the heated earth"}
};
const char *const windtemp_msg		[6][6] 									= {
    //Wind = 0 
	{
		"The frigid air is completely still",
		"A cold temperature hangs over the area",
		"The crisp air is eerily calm",
		"The warm air is still",
		"No wind makes the day uncomfortably warm",
		"The stagnant heat is sweltering"
	},

	//Wind = 1 
	{
		"A light breeze makes the frigid air seem colder",
		"A stirring of the air intensifies the cold",
		"A touch of wind makes the day cool",
		"It is a temperate day, with a slight breeze",
		"It is very warm, the air stirs slightly",
		"A faint breeze stirs the feverish air"
	},

	//Wind = 2 
	{
		"A breeze gives the frigid air bite",
		"A breeze swirls the cold air",
		"A lively breeze cools the area",
		"It is a temperate day, with a pleasant breeze",
		"Very warm breezes buffet the area",
		"A breeze ciculates the sweltering air"
	},

	//Wind = 3 
	{
		"Stiff gusts add cold to the frigid air",
		"The cold air is agitated by gusts of wind",
		"Wind blows in from the north, cooling the area",
		"Gusty winds mix the temperate air",
		"Brief gusts of wind punctuate the warm day",
		"Wind attempts to cut the sweltering heat"
	},

	//Wind = 4 
	{
		"The frigid air whirls in gusts of wind",
		"A strong, cold wind blows in from the north",
		"Strong wind makes the cool air nip",
		"It is a pleasant day, with gusty winds",
		"Warm, gusty winds move through the area",
		"Blustering winds punctuate the seering heat"
	},
	

	//Wind = 5 
	{
		"A frigid gale sets bones shivering",
		"Howling gusts of wind cut the cold air",
		"An angry wind whips the air into a frenzy",
		"Fierce winds tear through the tepid air",
		"Gale-like winds whip up the warm air",
		"Monsoon winds tear the feverish air"
	}
};
const char *const precip_msg		[3] 									= {
	"there is not a cloud in the sky",
	"pristine white clouds are in the sky",
	"thick, grey clouds mask the sun"
};
const char *const wind_msg			[6] 									= {
	"there is not a breath of wind in the air",
	"a slight breeze stirs the air",
	"a breeze wafts through the area",
	"brief gusts of wind punctuate the air",
	"angry gusts of wind blow",
	"howling winds whip the air into a frenzy"
};
