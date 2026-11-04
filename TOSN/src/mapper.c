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
 *                           Room Mapper Module                             *
 ****************************************************************************/
 
//Add a way of displaying up and down directions effectively
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "mapper.h"

//====================================================================//
//4-4 for exits mode. 9-12 in mapper.h
//3-3 for adjustent rooms mode. 6-8 in mapper.h
//====================================================================//
#define MAP_LAYOUT 		1	// 1 = MAP LEFT, 2 = FULL MAP
#define MAP_SIDE_COLS 6 // 18 map characters; leaves 78 text columns in 99 total
#define MAP_SIDE_ROWS 	4	// MAP ROWS IN LAYOUT 1 (4)
#define MAP_DESC_WIDTH  78	// DESCRIPTION WIDTH IN LAYOUT 1
//====================================================================//

#if MAP_LAYOUT != 1 && MAP_LAYOUT != 2
#error MAP_LAYOUT must be 1 or 2
#endif

#define DREAMWALK_EXTRA_X          22
#define DREAMWALK_EXTRA_DEPTH      22
#define DREAMWALK_MAPX             (MAPX + DREAMWALK_EXTRA_X)
#define DREAMWALK_MAXDEPTH         (MAXDEPTH + DREAMWALK_EXTRA_DEPTH)

static MAP_TYPE dream_dmap[DREAMWALK_MAPX + 1][MAPY + 1];
static bool dream_map_active = FALSE;

typedef struct
{
	int vnum;
	int depth;
} DREAM_MAP_VISIT;

static DREAM_MAP_VISIT dream_visited[DREAMWALK_MAPX + 1][MAPY + 1];

#define ACTIVE_MAPX                (dream_map_active ? DREAMWALK_MAPX : MAPX)
#define ACTIVE_MAXDEPTH            (dream_map_active ? DREAMWALK_MAXDEPTH : MAXDEPTH)
#define ACTIVE_BOUNDARY(x,y)       (dream_map_active ? ((x) < 0 || (y) < 0 || (x) > DREAMWALK_MAPX || (y) > MAPY) : BOUNDARY(x,y))
#define MAP_CELL(x,y)              (*(dream_map_active ? &dream_dmap[(x)][(y)] : &dmap[(x)][(y)]))

bool check_blind			(CHAR_DATA *ch);
MAP_TYPE dmap				[MAPX + 1][MAPY + 1];												// THE MAP ITSELF.
void get_exit_dir			(int dir, int *x, int *y, int xorig, int yorig){
    //Get the next coord based on direction 
	switch (dir)
	{
		case DIR_NORTH:			*x = xorig;		*y = yorig - 1;	break;
		case DIR_EAST:   		*x = xorig + 1;	*y = yorig;		break;
		case DIR_SOUTH:  		*x = xorig;		*y = yorig + 1;	break;
		case DIR_WEST:   		*x = xorig - 1;	*y = yorig;		break;
		case DIR_UP:			*x = xorig + 1;	*y = yorig - 1;	break;
		case DIR_DOWN:			*x = xorig - 1;	*y = yorig + 1; break;
		case DIR_NORTHEAST:		*x = xorig + 1;	*y = yorig - 1;	break;
		case DIR_NORTHWEST:		*x = xorig - 1;	*y = yorig - 1;	break;
		case DIR_SOUTHEAST: 	*x = xorig + 1;	*y = yorig + 1;	break;
		case DIR_SOUTHWEST: 	*x = xorig - 1;	*y = yorig + 1;	break;
		default:				*x = -1;		*y = -1;		break;
	}
}
char *get_exits				(CHAR_DATA *ch){
	static char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found = FALSE;

	buf[0] = '\0';

	if(!check_blind(ch)) return buf;
	set_char_color(AT_EXITS, ch);

	for(pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
	{
		//Immortals see all exits, even secret ones.
		if(IS_IMMORTAL(ch))
		{
			if(pexit->to_room)
			{
				found = TRUE;

				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), "%-5s:"	, capitalize(dir_name[pexit->vdir]));
				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " &p%s&D", pexit->to_room->name );

				//New code added to display closed, or otherwise invisible exits to immortals. 
				if(IS_SET(pexit->exit_info, EX_CLOSED))				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Closed)");
				if(IS_SET(pexit->exit_info, EX_DIG))				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Dig)");
				if(IS_SET(pexit->exit_info, EX_WINDOW))				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Window)");
				if(IS_SET(pexit->exit_info, EX_HIDDEN))				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Hidden)");
				if(xIS_SET(pexit->to_room->room_flags, ROOM_DEATH))	snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Trap)");
				mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
			}
		}
		else
		{
			if(pexit->to_room && !IS_SET(pexit->exit_info, EX_SECRET) && (!IS_SET( pexit->exit_info, EX_WINDOW ) 
			|| IS_SET(pexit->exit_info, EX_ISDOOR)) && !IS_SET(pexit->exit_info, EX_HIDDEN))
			{
				found = TRUE;
				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), "%-5s:", capitalize(dir_name[pexit->vdir]));
				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " &p%s&D", pexit->to_room->name );

				if(IS_SET(pexit->exit_info, EX_CLOSED))	
				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Closed)");

				if(IS_AFFECTED(ch, AFF_DETECTTRAPS) && xIS_SET(pexit->to_room->room_flags, ROOM_DEATH))
				snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), " (Trap)");

				mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
			}
		}
	}

	if(!found)
	{
		mudstrlcat( buf, "There are no exits", MAX_STRING_LENGTH );
		mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
	}

	return buf;
}
void clear_coord			(int x, int y){														// CLEAR ONE MAP COORD.
	MAP_CELL(x,y).tegn   = ' ';
	MAP_CELL(x,y).vnum   =  0;
	MAP_CELL(x,y).depth  =  0;
	MAP_CELL(x,y).sector = -1;
	
	//xCLEAR_BITS( MAP_CELL(x,y).info );
	MAP_CELL(x,y).can_see = TRUE;
}
void clear_room				(int x, int y){														// CLEAR ALL EXITS FOR ONE ROOM.
	int dir, exitx, exity;

	//Cycle through the four directions. 
	for(dir = 0; dir < 4; ++dir)
	{
		//Find next coord in this direction.
		get_exit_dir(dir, &exitx, &exity, x, y);

		//If coord is valid, clear it .
		if(!ACTIVE_BOUNDARY(exitx, exity)) {clear_coord(exitx, exity);}
	}
}
void map_exits				(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int x, int y, int depth ){	// THIS FUNCTION IS RECUSIVE (IT CALLS ITSELF).
	static char map_chars[12] = "|-|-udb/\\\\/";
	int door;
	int exitx = 0, exity = 0;
	int roomx = 0, roomy = 0;
	EXIT_DATA *pExit;

	if (dream_map_active)
	{
		if (dream_visited[x][y].vnum == pRoom->vnum && dream_visited[x][y].depth <= depth)
			return;
		dream_visited[x][y].vnum = pRoom->vnum;
		dream_visited[x][y].depth = depth;
	}

	//Setup this coord as a room - Change any symbols that can't be displayed here 
	
	MAP_CELL(x,y).sector = pRoom->sector_type;
	
	switch (pRoom->sector_type)
	{
		case SECT_INSIDE		: 	MAP_CELL(x,y).tegn = 'B'; 	break;
		case SECT_CITY			:	MAP_CELL(x,y).tegn = 'C';	break;
		case SECT_UNDERGROUND	:	MAP_CELL(x,y).tegn = 'U';	break;
		case SECT_FIELD			:	MAP_CELL(x,y).tegn = 'V';	break;
		case SECT_FOREST		:	MAP_CELL(x,y).tegn = 'F';	break;
		case SECT_LAVA			:	MAP_CELL(x,y).tegn = 'L';	break;
		case SECT_SWAMP			:	MAP_CELL(x,y).tegn = 'S';	break;
		case SECT_DESERT		:	MAP_CELL(x,y).tegn = 'D';	break;
		case SECT_ICE			:	MAP_CELL(x,y).tegn = 'I';	break;
		case SECT_HILLS			:	MAP_CELL(x,y).tegn = 'H';	break;
		case SECT_MOUNTAIN		: 	MAP_CELL(x,y).tegn = 'M';	break;
		case SECT_WATER_SWIM	:	MAP_CELL(x,y).tegn = 'W';	break;
		case SECT_WATER_NOSWIM	:	MAP_CELL(x,y).tegn = 'O';	break;
		case SECT_UNDERWATER	:	MAP_CELL(x,y).tegn = 'Q';	break;
		case SECT_AIR			:	MAP_CELL(x,y).tegn = 'A';	break;
		default					:	MAP_CELL(x,y).tegn = 'X';	break;
	}

	CHAR_DATA *victim;
/*	
	//Shows players, npcs, and creatures on map.
	if (pRoom != ch->in_room)
	{
		for (victim = pRoom->first_person; victim; victim = victim->next_in_room)
		{
			if (!can_see(ch, victim))
			continue;

			if (!IS_NPC(victim))
			{
				MAP_CELL(x,y).tegn = '%'; //Fellow player
				break;
			}

			if (!xIS_SET(victim->act, ACT_PET))	MAP_CELL(x,y).tegn = '$'; //Npc
			else								MAP_CELL(x,y).tegn = '@'; //Creature
		}
	}
*/
	MAP_CELL(x,y).vnum    = pRoom->vnum;
	MAP_CELL(x,y).depth   = depth;
    MAP_CELL(x,y).can_see = room_is_dark( pRoom );
	
	//Player is OUTSIDE: show first indoor/underground room, then stop.
	if (ch->in_room->sector_type != SECT_INSIDE
		&& ch->in_room->sector_type != SECT_UNDERGROUND
			&& (pRoom->sector_type == SECT_INSIDE
				|| pRoom->sector_type == SECT_UNDERGROUND))
	{
		return;
	}
	
	//Player is INSIDE: show first outdoor room, then stop.
	if ((ch->in_room->sector_type == SECT_INSIDE
		|| ch->in_room->sector_type == SECT_UNDERGROUND)
			&& pRoom->sector_type != SECT_INSIDE
				&& pRoom->sector_type != SECT_UNDERGROUND)
	{
		return;
	}
	
	//Limit recursion 
	if(depth > ACTIVE_MAXDEPTH) return;

	//This room is done, deal with it's exits 
	for(door = 0; door < 10; ++door)
	{
		//Skip if there is no exit in this direction 
		if(!(pExit = get_exit(pRoom, door))) 
		continue;

		//Get the coords for the next exit and room in this direction 
		get_exit_dir(door, &exitx, &exity, x, y);
		get_exit_dir(door, &roomx, &roomy, exitx, exity);

		//Skip if coords fall outside map 
		if(ACTIVE_BOUNDARY(exitx, exity) || ACTIVE_BOUNDARY(roomx, roomy))
		continue;

		//Skip if there is no room beyond this exit 
		if(!pExit->to_room)
        continue;

		//Ensure there are no clashes with previously defined rooms 
		if((MAP_CELL(roomx,roomy).vnum != 0) && (MAP_CELL(roomx,roomy).vnum != pExit->to_room->vnum))
		{
			//Use the new room if the depth is higher 
			if(MAP_CELL(roomx,roomy).depth <= depth)
            continue;

			//It is so clear the old room 
			clear_room(roomx, roomy);
		}

		//No exits at MAXDEPTH 
		if(depth == ACTIVE_MAXDEPTH)
        continue;

		//No need for exits that are already mapped 
		if(MAP_CELL(exitx,exity).depth > 0 && MAP_CELL(roomx,roomy).vnum != 0)
		continue;


		//Fill in exit 
		MAP_CELL(exitx,exity).depth = depth;
		MAP_CELL(exitx,exity).vnum = pExit->to_room->vnum;
	  //MAP_CELL(exitx,exity).info = pExit->exit_info;
		MAP_CELL(exitx,exity).tegn = map_chars[door];
		MAP_CELL(exitx,exity).sector = UMIN(pRoom->sector_type, pExit->to_room->sector_type);

		//More to do? If so we recurse
		if(depth < ACTIVE_MAXDEPTH && ((MAP_CELL(roomx,roomy).vnum == pExit->to_room->vnum) || (MAP_CELL(roomx,roomy).vnum == 0)))
		{
			//Depth increases by one each time 
			map_exits(ch, pExit->to_room, roomx, roomy, depth + 1);
		}
	}
}
void reformat_desc			(char *desc){														// REFORMAT ROOM DESCRIPTIONS TO EXCLUDE UNDESIRABLE CHARACTERS.

	//Index variables to keep track of array/pointer elements 
	unsigned int i = 0;
	int j 		   = 0;
	char buf[MAX_STRING_LENGTH], *p;

	buf[0] = '\0';

	if(!desc) return;

	//Replace all "\n" and "\r" with spaces 
	size_t len = strlen(desc);
	for (i = 0; i < len; ++i)
	{
		if((desc[i] == '\r') || (desc[i] == '\n'))
        desc[i] = ' ';
	}

	//Remove multiple spaces 
	for(p = desc; *p != '\0'; ++p)
	{
		buf[j] = *p;
		++j;

		//Two or more consecutive spaces? 
		if((*p == ' ') && (*(p + 1) == ' '))
		{
			do
			{
				++p;
			}
			while(*(p + 1) == ' ');
		}
	}

	buf[j] = '\0';

	//Copy to desc 
	mudstrlcpy(desc, buf, MAX_STRING_LENGTH);
}
int get_line				(char *desc, size_t max_len){
	size_t i, j = 0;

	//Return if it's short enough for one line 
	if(strlen(desc) <= max_len)
    return 0;

	//Calculate end point in string without color 
	for(i = 0; i <= strlen(desc); ++i)
	{
		char dst[20];
		int vislen;

		switch (desc[i])
		{
			case '&':  											// NORMAL, Foreground colour
			case '^':  											// BACKGROUND colour
			case '}':  											// BLINK Foreground colour
            *dst = '\0';
            vislen = 0;
            i += colorcode( &desc[i], dst, NULL, 20, &vislen ); // Skip input token
            j += vislen; 										// Count output token length
            break;   											// this was missing - if you have issues, remove it

			default:   											// No conversion, just count
            ++j;
            break;
		}
      
		if(j > max_len)
		break;
	}

	//End point is now in i, find the nearest space 
	for(j = i; j > 0; --j)
	{
		if(desc[j] == ' ')
        break;
	}

	//There could be a problem if there are no spaces on the line 
	return j + 1;
}
char *whatColor				(char *str, char *pos){
	static char col[2];

	col[0] = '\0';
	
	while(str != pos)
	{
		if(*str == '&' || *str == '^' || *str == '}')
		{
			col[0] = *str;
			++str;
			
			if(!str)
			{
				col[1] = '\0';
				break;
			}
			col[1] = *str;
		}
		++str;
	}
	return col;
}
static void mapper_description_line(char **text, char *out, size_t size)
{
	char *start = *text, *end = start, *space = NULL;
	int visible = 0;
	while (*end && visible < MAP_DESC_WIDTH)
	{
		if ((*end == '&' || *end == '^' || *end == '}') && end[1])
		{
			end += 2;
			continue;
		}
		if (*end == ' ') space = end;
		++end;
		++visible;
	}
	if (*end && *end != ' ' && space) end = space;
	snprintf(out, size, "&W%.*s&D", (int)(end - start), start);
	while (*end == ' ' || *end == '\r' || *end == '\n') ++end;
	*text = end;
}

void show_map				(CHAR_DATA *ch, char *text){
	char buf[MAX_STRING_LENGTH * 2];
	char line[MAX_STRING_LENGTH];
	char stats[2][MAX_STRING_LENGTH];
	char pMSG[256];
	int x, y;
	char *description = text ? text : (char *)"";
	int description_rows = 0;
	int display_rows;
	char *count_description = description;
	int map_start = 0, map_end = ACTIVE_MAPX;
	int map_top = 0, map_bottom = MAPY;	
	int map_cols = ACTIVE_MAPX;
	int map_width = MAP_SIDE_COLS * 3;
	bool dreamwalk = is_affected(ch, tes_effect_dreamwalk);
	const char *suf;
	short day;
	int first_map_row;
	char area_reputation[256];

	//====================================================================//
	// INITIALIZING														  //
	//====================================================================//

	buf[0] = '\0';


	//====================================================================//
	// GATHERING THE PLAYER'S STATS										  //
	//====================================================================//
	const char *EXPcolor, *ATKcolor, *DEFcolor, *DMGcolor;
	const char *char_repColor, *char_reputation;
	int char_exp;

	while (*count_description)
	{
		mapper_description_line(&count_description, line, sizeof(line));
		++description_rows;
	}
	/* Time, reputation, description, blank row and Hp precede Atk. */
	display_rows = description_rows + 4;

	GET_MAPPER_PLAYER_STATS(ch, &EXPcolor, &ATKcolor, &DEFcolor, &DMGcolor, &char_repColor, &char_reputation, &char_exp);
	GET_MAPPER_TIME(pMSG, sizeof(pMSG));
	GET_MAPPER_AREA_REPUTATION(ch, area_reputation, sizeof(area_reputation));
	snprintf(stats[0], sizeof(stats[0]), "&wHp:%s%d%% &wMp:%s%d%% &wFp:%s%d%% &wEnc:%s%d%% &wNut:%s%d%% &wHyd:%s%d%% &wVit:%s%+d&D",
		ch->HLTcolor, ch->char_health_percentage,
		ch->MGKcolor, ch->char_magicka_percentage,
		ch->FTGcolor, ch->char_fatigue_percentage,
		ch->ECBcolor, ch->char_encumbrance_percentage,
		ch->NUTcolor, ch->char_nutrition_percentage,
		ch->HYDcolor, ch->char_hydration_percentage,
		ch->VITcolor, ch->char_vitality_percentage);
	snprintf(stats[1], sizeof(stats[1]), "&wAtk:%s%+d &wDef:%s%+d &wDmg:%s%+d &wLevel:&C%d &wExp:%s%d%% &wAlign:%s%+d &wRep:%s%+d&D",
		ATKcolor, ch->atk_bonus, DEFcolor, ch->def_bonus, DMGcolor, ch->dmg_bonus,
		ch->char_level, EXPcolor, char_exp, ch->ALGcolor, ch->alignment,
		char_repColor, ch->char_honor);



	//====================================================================//
	// ROOM NAME + TIME (FULL MAP MODE)									  //
	//====================================================================//

#if MAP_LAYOUT == 2
	if (!dreamwalk)
	{
		snprintf(line, sizeof(line), "&Y You are at: &R%s&D\r\n", ch->in_room->name); //Add remove space between map and desc here
		mudstrlcat(buf, line, MAX_STRING_LENGTH * 2);
		snprintf(line, sizeof(line), "&C%s&D\r\n", pMSG);
		mudstrlcat(buf, line, MAX_STRING_LENGTH * 2);
	}
#endif

#if MAP_LAYOUT == 1
	map_cols = map_width / 3;
	map_start = (ACTIVE_MAPX - map_cols) / 2;
	map_end = map_start + map_cols;
	map_top = (MAPY - display_rows) / 2;
	if (map_top < 0) map_top = 0;
	map_bottom = map_top + display_rows;
#endif


	//====================================================================//
	// GATHERING THE AREA REPUTATION									  //
	//====================================================================//
	
	//====================================================================//
	// MAP TOP BORDER													  //
	//====================================================================//

	//buf[0] = '\0';

	mudstrlcat(buf, "&Y+", MAX_STRING_LENGTH);

	for (x = 0; x < map_width; ++x)
		mudstrlcat(buf, "-", MAX_STRING_LENGTH);

#if MAP_LAYOUT == 1
	if (MAP_LAYOUT == 1)
	{
		mudstrlcat(buf, "+&D", MAX_STRING_LENGTH);
		snprintf(line, sizeof(line), "&Y You are at: &R%s&D\r\n", ch->in_room->name); //Room name
		mudstrlcat(buf, line, MAX_STRING_LENGTH);
	}
	else mudstrlcat(buf, "+&D\r\n", MAX_STRING_LENGTH);
#else
	mudstrlcat(buf, "+&D\r\n", MAX_STRING_LENGTH);
#endif

	send_to_char(buf, ch);

	//====================================================================//
	// MAP
	//====================================================================//
	
	//********************************************************************//
	first_map_row = map_top;						//Exit paths
	//first_map_row = (MAPY / 2) % 2;				//Adjustent rooms.
	//********************************************************************//
	
	//********************************************************************//
	for (y = map_top; y < map_bottom; ++y)			//Exit paths
	//for (y = (MAPY / 2) % 2; y < MAPY; y += 2) 	//Adjustent rooms.
	//********************************************************************//	
	{
		buf[0] = '\0';

		//------------------------------------------------------------//
		// LEFT BORDER
		//------------------------------------------------------------//
		mudstrlcat(buf, "&Y|&D", MAX_STRING_LENGTH);

		//------------------------------------------------------------//
		// MAP CHARACTERS
		//------------------------------------------------------------//
	
		//********************************************************************//
		for (x = map_start; x < map_end; ++x)			//Exit paths	
		//for (x = (MAPX / 2) % 2; x < MAPX; x += 2)	//Adjustent rooms.
		//********************************************************************//	
		{
			const char *exit_color;

			if (x < 0 || x > ACTIVE_MAPX || y < 0 || y > MAPY)
			{
				mudstrlcat(buf, "   ", MAX_STRING_LENGTH);
				continue;
			}

		
			if (MAP_CELL(x,y).tegn == 'X')
				MAP_CELL(x,y).sector = ch->in_room->sector_type;

				 if (MAP_CELL(x,y).sector == SECT_INSIDE)			exit_color = "&R";
			else if (MAP_CELL(x,y).sector == SECT_CITY) 			exit_color = "&W";
			else if (MAP_CELL(x,y).sector == SECT_UNDERGROUND)		exit_color = "&w";		
			else if (MAP_CELL(x,y).sector == SECT_FIELD)			exit_color = "&g";
			else if (MAP_CELL(x,y).sector == SECT_FOREST)			exit_color = "&G";
			else if (MAP_CELL(x,y).sector == SECT_LAVA) 			exit_color = "&R";
			else if (MAP_CELL(x,y).sector == SECT_SWAMP) 			exit_color = "&p";
			else if (MAP_CELL(x,y).sector == SECT_DESERT) 			exit_color = "&Y";	
			else if (MAP_CELL(x,y).sector == SECT_ICE) 			exit_color = "&W"; //Snow?
			else if (MAP_CELL(x,y).sector == SECT_HILLS)			exit_color = "&w";
			else if (MAP_CELL(x,y).sector == SECT_MOUNTAIN)		exit_color = "&W";
			else if (MAP_CELL(x,y).sector == SECT_WATER_SWIM)		exit_color = "&B";
			else if (MAP_CELL(x,y).sector == SECT_WATER_NOSWIM) 	exit_color = "&b";
			else if (MAP_CELL(x,y).sector == SECT_UNDERWATER) 		exit_color = "&b";		
			else if (MAP_CELL(x,y).sector == SECT_AIR) 			exit_color = "&C";
			else												exit_color = "&W";

		
			switch (MAP_CELL(x,y).tegn)
			{
				case '-' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s---&D", exit_color	);	break;	//Road east/south
				case '|' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s | &D", exit_color	);	break; 	//Road north/south
				case '+' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s + &D", exit_color	);	break;	//Road interesection
				case '\\': snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y 🡑 &D", exit_color	);	break;	//Up core
				case '/' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y 🡓 &D", exit_color	);	break;	//Down core
				case 'u' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y 🡑 &D", exit_color	);	break;	//Up
				case 'd' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y 🡓 &D", exit_color	);	break;	//Down
				case 'b' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y 🡙 &D", exit_color	);	break;	//Both	
			
				case 'X' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s[&px%s]&D", exit_color, exit_color	);	break;	//You
				case '%' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s[&cx%s]&D", exit_color, exit_color	);	break;	//Player			
				case '$' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s[&ox%s]&D", exit_color, exit_color	);	break;	//Npc
				case '@' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s[&rx%s]&D", exit_color, exit_color	);	break;	//Mob		

				case ' ' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s", dreamwalk ? "   " : "&P   &D"); break;
				case 'B' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&R[ ]&D"	);	break; 	//Inside			
				case 'C' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&W[ ]&D"	);	break;	//City
				case 'U' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&w[ ]&D"	);	break;	//Underground
				case 'V' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&g[ ]&D"	);	break;	//Field			
				case 'F' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&G[ ]&D"	);	break;	//Forest
				case 'L' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&R[ ]&D"	);	break;	//Lava
				case 'S' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&g[ ]&D"	);	break;	//Swamp
				case 'D' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&Y[ ]&D"	);	break;	//Desert
				case 'I' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&C[ ]&D"	);	break;	//Ice
				case 'H' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&w[ ]&D"	);	break;	//Hills
				case 'M' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&W[ ]&D"	);	break;	//Mountain
				case 'W' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&B[ ]&D"	);	break;	//Water swim
				case 'O' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&b[ ]&D"	);	break;	//Water no swim		
				case 'A' : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&b[ ]&D"	);	break;	//Underwater
				default  : snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "   "  		);	break;				
			}
		}

		//------------------------------------------------------------//
		// RIGHT BORDER
		//------------------------------------------------------------//
#if MAP_LAYOUT == 1
		if (MAP_LAYOUT == 1)
		{
		mudstrlcat(buf, "&Y| &D", MAX_STRING_LENGTH); //Add remove space between map and desc here
		if (y == first_map_row)
		{
			mudstrlcat(buf, "&C", MAX_STRING_LENGTH);
			mudstrlcat(buf, pMSG, MAX_STRING_LENGTH); //Time
			mudstrlcat(buf, "&D", MAX_STRING_LENGTH);
		}
		else if (y == first_map_row + 1)
		{
			mudstrlcat(buf, "&G", MAX_STRING_LENGTH);
			mudstrlcat(buf, area_reputation, MAX_STRING_LENGTH); //Area reputation
			mudstrlcat(buf, "&D", MAX_STRING_LENGTH);
		}
		else if (*description)
		{
			mapper_description_line(&description, line, sizeof(line));
			mudstrlcat(buf, line, MAX_STRING_LENGTH);
		}
		else if (y - first_map_row == description_rows + 3)
			mudstrlcat(buf, stats[0], MAX_STRING_LENGTH);
		mudstrlcat(buf, "\r\n", MAX_STRING_LENGTH);
		}
		else mudstrlcat(buf, "&Y|&D\r\n", MAX_STRING_LENGTH);
#else
		mudstrlcat(buf, "&Y|&D\r\n", MAX_STRING_LENGTH);
#endif

		send_to_char(buf, ch);
	}

	//====================================================================//
	// MAP BOTTOM BORDER
	//====================================================================//

	buf[0] = '\0';
	
	mudstrlcat(buf, "&Y+", MAX_STRING_LENGTH * 2);

	for (x = 0; x < map_width; ++x)
		mudstrlcat(buf, "-", MAX_STRING_LENGTH * 2);

	mudstrlcat(buf, "+ &D", MAX_STRING_LENGTH * 2); //Add remove space between map and desc here
	
#if MAP_LAYOUT == 1
	mudstrlcat(buf, stats[1], MAX_STRING_LENGTH * 2);
#endif
	mudstrlcat(buf, "\r\n", MAX_STRING_LENGTH * 2);
	send_to_char(buf, ch);
	if (dreamwalk)
	{
		send_to_char("\r\n", ch);
		return;
	}

	ch_printf(ch, "\r\n", pMSG);
	
	//====================================================================//
	// SHOW EXITS
	//====================================================================//

	{
		EXIT_DATA *pexit;

		for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
		{
			if (pexit->to_room)
			{
				//ch_printf(ch, "&R%s &Yis to the &O%-5s&D\r\n", pexit->to_room->name, capitalize(dir_name[pexit->vdir]));
				ch_printf(ch, "&W[&O%-5.5s&W] &W[&Y%d&W] &R%s&D\r\n", capitalize(dir_name[pexit->vdir]), pexit->to_room->vnum, pexit->to_room->name);
			}
		}
	}

	//====================================================================//
	// SHOW PROMPT
	//====================================================================//
	
	//CHAR_CUSTOM_PROMPT(ch);
	
}
void draw_room_map			(CHAR_DATA *ch, const char *desc){									// CLEAR, GENERATE AND DISPLAY THE MAP.
	int x, y;
	static char buf[MAX_STRING_LENGTH];

	mudstrlcpy(buf, desc, MAX_STRING_LENGTH);
	dream_map_active = is_affected(ch, tes_effect_dreamwalk);
	if (dream_map_active) memset(dream_visited, 0, sizeof(dream_visited));

	//Remove undesirable characters 
	reformat_desc(buf);

	//Clear map 
	for(y = 0; y <= MAPY; ++y)
	{
		for(x = 0; x <= ACTIVE_MAPX; ++x)
		{
			clear_coord( x, y );
		}
	}

	//Start with players pos at centre of map 
	x = ACTIVE_MAPX / 2;
	y = MAPY / 2;

	MAP_CELL(x,y).vnum = ch->in_room->vnum;
	MAP_CELL(x,y).depth = 0;

	//Generate the map 
	map_exits(ch, ch->in_room, x, y, 0);

	//Current position should be a "X"
	MAP_CELL(x,y).tegn   = 'X';
	MAP_CELL(x,y).sector = -1;

	//Send the map 
	show_map(ch, buf);
}
