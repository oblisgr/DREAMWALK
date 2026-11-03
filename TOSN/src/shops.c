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
 *                       Shop and repair shop module                        *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "mud.h"

//--------------------------------------------------------------------------------------------------//
// Shop appearance and reliable opening hours. No changes to the septim economy.
//--------------------------------------------------------------------------------------------------//
static bool SHOP_IS_OPEN(int open_hour, int close_hour){
	if(open_hour <= close_hour)	return time_info.hour >= open_hour && time_info.hour <= close_hour;
	return time_info.hour >= open_hour || time_info.hour <= close_hour;
}
static const char *SHOP_RARITY_COLOR(OBJ_DATA *obj){
	if(!obj) return "&W";
	switch(obj->value[5])
	{
		case 6: return "&R";
		case 5: return "&Y";
		case 4: return "&P";
		case 3: return "&B";
		case 2: return "&G";
		default: return "&W";
	}
}
static const char *SHOP_ITEM_CATEGORY(OBJ_DATA *obj){
	if(!obj) return "Other";
	switch(obj->item_type)
	{
		case ITEM_WEAPON:	return "Weapon";
		case ITEM_ARMOR:	return "Armor";
		case ITEM_WAND:		return "Wand";
		case ITEM_STAFF:	return "Staff";
		case ITEM_POTION:	return "Potion";
		case ITEM_SCROLL:	return "Scroll";
		case ITEM_FOOD:		return "Food";
		case ITEM_CONTAINER:	return "Container";
		default:			return "Goods";
	}
}
static int SHOP_BULK_REPAIR_COST(int cost){
	long long amount = (long long)cost * 11 /10;
	return amount > INT_MAX ? INT_MAX : (int)amount;
}
static void SHOP_REACTION(CHAR_DATA *ch, CHAR_DATA *keeper, int action){
	// A short, occasional response; not an extra transaction or skill check.
	if(!ch || !keeper || number_percent() > 35) return;

	switch(action)
	{
		case 1: // Purchased an item
			switch(number_range(0, 2))
			{
				case 0: act(AT_TELL, "$n says, 'A fine choice. May it serve you well.'", keeper, NULL, ch, TO_VICT); break;
				case 1: act(AT_TELL, "$n says, 'Good doing business with you.'", keeper, NULL, ch, TO_VICT); break;
				case 2: act(AT_TELL, "$n says, 'You'll get good use out of that.'", keeper, NULL, ch, TO_VICT); break;
			} break;
		case 2: // Sold an item
			switch(number_range(0, 2))
			{
				case 0: act(AT_TELL, "$n says, 'I'll find a buyer for this.'", keeper, NULL, ch, TO_VICT); break;
				case 1: act(AT_TELL, "$n says, 'Fair trade, I'd say.'", keeper, NULL, ch, TO_VICT); break;
				case 2: act(AT_TELL, "$n says, 'Another useful piece for my shelves.'", keeper, NULL, ch, TO_VICT); break;
			} break;
		case 3: // Repaired an item
			switch(number_range(0, 2))
			{
				case 0: act(AT_TELL, "$n says, 'There. Good as new.'", keeper, NULL, ch, TO_VICT); break;
				case 1: act(AT_TELL, "$n says, 'Take better care of it next time.'", keeper, NULL, ch, TO_VICT); break;
				case 2: act(AT_TELL, "$n says, 'A proper repair takes a practiced hand.'", keeper, NULL, ch, TO_VICT); break;
			} break;
	}
}
CHAR_DATA *find_keeper		(CHAR_DATA * ch){ 												//Shopping commands.

	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *keeper, *whof;
	SHOP_DATA *pShop;
	int speakswell;
   
	if(!ch || !ch->in_room) return NULL;
	pShop = NULL;
	for(keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
		if(IS_NPC(keeper) && keeper->pIndexData && (pShop = keeper->pIndexData->pShop) != NULL)
			break;

	if(!pShop)
	{
		act(AT_YELLOW, "You can't do that here.", ch, NULL, NULL, TO_CHAR);
		return NULL;
	}

	//Undesirables.
	if(!IS_NPC(ch) && xIS_SET(ch->act, PLR_KILLER))
	{
		do_say(keeper, "Murderers are not welcome here!");
		snprintf(buf, MAX_STRING_LENGTH, "%s the KILLER is over here!\r\n", ch->name);
		do_yell(keeper, buf);
		return NULL;
	}

	if(!IS_NPC(ch) && xIS_SET(ch->act, PLR_THIEF))
	{
		do_say(keeper, "Thieves are not welcome here!");
		snprintf(buf, MAX_STRING_LENGTH, "%s the THIEF is over here!\r\n", ch->name);
		do_yell(keeper, buf);
		return NULL;
	}

	//Disallow sales during battle.
	if( (whof = who_fighting(keeper)) != NULL)
	{
		if(whof == ch) act(AT_YELLOW, "I don't think that's a good idea.", ch, NULL, NULL, TO_CHAR);
		else		   do_say(keeper, "I'm too busy for that!");
		return NULL;
	}

	if(who_fighting(ch))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&Y%s doesn't seem to want to get involved.&D", PERS(keeper, ch)); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return NULL;
	}

	//Opening hours support shops which close after midnight.
	if(!SHOP_IS_OPEN(pShop->open_hour, pShop->close_hour))
	{
		do_say(keeper, "The shop is closed. Come back during business hours.");
		return NULL;
	}

	if(keeper->position == POS_SLEEPING) {act(AT_YELLOW, "While they're asleep?", ch, NULL, NULL, TO_CHAR); return NULL;}
	if(keeper->position < POS_SLEEPING)  {act(AT_YELLOW, "I don't think they can hear you.", ch, NULL, NULL, TO_CHAR); return NULL;}

	//Invisible or hidden people.
	if(!can_see(keeper, ch))
	{
		do_say(keeper, "I don't trade with folks I can't see.");
		return NULL;
	}

	speakswell = UMIN(knows_language(keeper, ch->speaking, ch), knows_language(ch, ch->speaking, keeper));

	if((number_percent() % 65 ) > speakswell)
	{
		     if(speakswell > 60) snprintf(buf, MAX_STRING_LENGTH, "%s Could you repeat that? I didn't quite catch it.", ch->name);
		else if(speakswell > 50) snprintf(buf, MAX_STRING_LENGTH, "%s Could you say that a little more clearly please?", ch->name);
		else if(speakswell > 40) snprintf(buf, MAX_STRING_LENGTH, "%s Sorry... What was that you wanted?", ch->name);
		else					 snprintf(buf, MAX_STRING_LENGTH, "%s I can't understand you.", ch->name);
		do_tell(keeper, buf);
		return NULL;
	}

	return keeper;
}
CHAR_DATA *find_fixer		(CHAR_DATA * ch){ 												//Repair commands.

	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *keeper, *whof;
	REPAIR_DATA *rShop;
	int speakswell;

	if(!ch || !ch->in_room) return NULL;
	rShop = NULL;
	for(keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
		if(IS_NPC(keeper) && keeper->pIndexData && (rShop = keeper->pIndexData->rShop) != NULL)
			break;

	if( !rShop )
	{
		act(AT_YELLOW, "You can't do that here.", ch, NULL, NULL, TO_CHAR);
		return NULL;
	}

	//Undesirables.
	if(!IS_NPC(ch) && xIS_SET(ch->act, PLR_KILLER))
	{
		do_say(keeper, "Murderers are not welcome here!");
		snprintf(buf, MAX_STRING_LENGTH, "%s the KILLER is over here!\r\n", ch->name);
		do_yell(keeper, buf);
		return NULL;
	}

	if(!IS_NPC(ch) && xIS_SET(ch->act, PLR_THIEF))
	{
		do_say(keeper, "Thieves are not welcome here!");
		snprintf(buf, MAX_STRING_LENGTH, "%s the THIEF is over here!\r\n", ch->name);
		do_yell(keeper, buf);
		return NULL;
	}

	//Disallow sales during battle
	if((whof = who_fighting(keeper)) != NULL)
	{
		if(whof == ch) act(AT_YELLOW, "I don't think that's a good idea.", ch, NULL, NULL, TO_CHAR);
		else		   do_say(keeper, "I'm too busy for that!" );
		return NULL;
	}

	//According to rlog, this is the second time I've done this so mobiles can repair in combat.
	if(!IS_NPC(ch) && who_fighting(ch))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&Y%s doesn't seem to want to get involved.&D", PERS(keeper, ch)); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		return NULL;
	}

	//Opening hours support shops which close after midnight.
	if(!SHOP_IS_OPEN(rShop->open_hour, rShop->close_hour))
	{
		do_say(keeper, "The shop is closed. Come back during business hours.");
		return NULL;
	}

	if(keeper->position == POS_SLEEPING) {act(AT_YELLOW, "While they're asleep?", ch, NULL, NULL, TO_CHAR); return NULL;}
	if(keeper->position < POS_SLEEPING)  {act(AT_YELLOW, "I don't think they can hear you.", ch, NULL, NULL, TO_CHAR); return NULL;}

	//Invisible or hidden people.
	if(!can_see(keeper, ch))
	{
		do_say(keeper, "I don't trade with folks I can't see.");
		return NULL;
	}

	speakswell = UMIN( knows_language(keeper, ch->speaking, ch), knows_language(ch, ch->speaking, keeper));

	if((number_percent() % 65) > speakswell)
	{
		     if(speakswell > 60) snprintf(buf, MAX_STRING_LENGTH, "%s Could you repeat that?  I didn't quite catch it.", ch->name);
		else if(speakswell > 50) snprintf(buf, MAX_STRING_LENGTH, "%s Could you say that a little more clearly please?", ch->name);
		else if(speakswell > 40) snprintf(buf, MAX_STRING_LENGTH, "%s Sorry... What was that you wanted?", ch->name);
		else 					 snprintf(buf, MAX_STRING_LENGTH, "%s I can't understand you.", ch->name);
		do_tell( keeper, buf );
		return NULL;
	}

	return keeper;
}
int get_cost				(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy){
	SHOP_DATA *pShop;
	long long cost;
	bool richcustomer;
	int profitmod;

	if(!ch || !keeper || !keeper->pIndexData || !obj || obj->cost <= 0) return 0;
	if((pShop = keeper->pIndexData->pShop) == NULL) return 0;

	// Avoid overflow when a high-level player carries a large purse.
	richcustomer = (long long)ch->gold > (long long)ch->level * ch->level * 100000;

	if(fBuy)
	{
		profitmod = 13 - get_curr_cha(ch) + (richcustomer ? 15 : 0) + ((URANGE(5, ch->level, LEVEL_AVATAR) - 20) /2);
		cost = (long long)obj->cost * UMAX((pShop->profit_sell + 1), pShop->profit_buy + profitmod) /100;
		cost = cost * (80 + UMIN(ch->level, LEVEL_AVATAR)) /100;
	}
	else
	{
		OBJ_DATA *obj2;
		int itype;

		profitmod = get_curr_cha(ch) - 13 - (richcustomer ? 15 : 0);
		cost = 0;

		for(itype = 0; itype < MAX_TRADE; itype++)
		{
			if(obj->item_type == pShop->buy_type[itype])
			{
				cost = (long long)obj->cost * UMIN((pShop->profit_buy - 1), pShop->profit_sell + profitmod) /100;
				break;
			}
		}

		// Existing rule: a keeper does not buy duplicates of a stock item.
		for(obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content)
		{
			if(obj->pIndexData == obj2->pIndexData)
			{
				cost = 0;
				break;
			}
		}
	}

	if(obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
	{
		if(obj->value[1] <= 0 || obj->value[2] <= 0) return 0;
		if(cost > INT_MAX && obj->value[2] > obj->value[1]) cost = INT_MAX;
		else cost = cost * obj->value[2] / obj->value[1];
	}

	if(cost <= 0) return 0;
	return cost > INT_MAX ? INT_MAX : (int)cost;
}
int get_repaircost		(CHAR_DATA *keeper, OBJ_DATA *obj){
	REPAIR_DATA *rShop;
	long long cost, base;
	int itype, missing;
	bool found = FALSE;

	if(!keeper || !keeper->pIndexData || !obj || (rShop = keeper->pIndexData->rShop) == NULL) return -1;

	// Only things which can actually be restored are accepted.
	if(obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF) return -1;

	for(itype = 0; itype < MAX_FIX; itype++)
	{
		if(obj->item_type == rShop->fix_type[itype]) {found = TRUE; break;}
	}
	if(!found) return -1;

	base = obj->cost;
	if(base <= 0 && obj->pIndexData) base = (long long)obj->pIndexData->cost * 66 /100;
	cost = base * rShop->profit_fix /1000;
	if(cost < 1) cost = 1;

	switch(obj->item_type)
	{
		case ITEM_ARMOR:
			missing = obj->value[1] - obj->value[0];
			break;
		case ITEM_WEAPON:
			missing = INIT_WEAPON_CONDITION - obj->value[0];
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			missing = obj->value[1] - obj->value[2];
			break;
		default: return -1;
	}

	if(missing <= 0) return -2;
	cost *= missing;
	return cost > INT_MAX ? INT_MAX : (int)cost;
}
void do_buy					(CHAR_DATA *ch, const char *argument){
	char arg[MAX_INPUT_LENGTH];
	int maxgold;

	argument = one_argument(argument, arg);
	if(arg[0] == '\0') {act(AT_YELLOW, "Buy what? Try 'list' to see the goods.", ch, NULL, NULL, TO_CHAR); return;}

	if(xIS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
	{
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *pet;
		ROOM_INDEX_DATA *pRoomIndexNext, *in_room;
		long long petcost;

		if(IS_NPC(ch)) return;
		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
		if(!pRoomIndexNext)
		{
			bug("%s: bad pet shop at vnum %d.", __func__, ch->in_room->vnum);
			act(AT_YELLOW, "No pets are available here.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		in_room = ch->in_room;
		ch->in_room = pRoomIndexNext;
		pet = get_char_room(ch, arg);
		ch->in_room = in_room;

		if(!pet || !IS_NPC(pet) || !xIS_SET(pet->act, ACT_PET)) {act(AT_YELLOW, "That companion isn't for sale.", ch, NULL, NULL, TO_CHAR); return;}
		if(xIS_SET(ch->act, PLR_BOUGHT_PET)) {act(AT_YELLOW, "You've already adopted a companion this level.", ch, NULL, NULL, TO_CHAR); return;}
		petcost = 10LL * pet->level * pet->level;
		if(petcost <= 0 || petcost > INT_MAX || ch->gold < petcost) {act(AT_YELLOW, "You don't have enough septims.", ch, NULL, NULL, TO_CHAR); return;}
		if(ch->level < pet->level) {act(AT_YELLOW, "You aren't ready to care for that companion.", ch, NULL, NULL, TO_CHAR); return;}

		pet = create_mobile(pet->pIndexData);
		if(!pet) {act(AT_YELLOW, "That companion isn't available right now.", ch, NULL, NULL, TO_CHAR); return;}
		maxgold = (int)petcost;
		ch->gold -= maxgold;
		boost_economy(ch->in_room->area, maxgold);
		xSET_BIT(ch->act, PLR_BOUGHT_PET);
		xSET_BIT(pet->act, ACT_PET);
		xSET_BIT(pet->affected_by, AFF_CHARM);

		argument = one_argument(argument, arg);
		if(arg[0] != '\0')
		{
			snprintf(buf, sizeof(buf), "%s %s", pet->name, arg);
			STRFREE(pet->name);
			pet->name = STRALLOC(buf);
		}
		snprintf(buf, sizeof(buf), "&Y%s: A neck tag says 'I belong to %s'.&D\r\n", pet->description ? pet->description : "", ch->name);
		STRFREE(pet->description);
		pet->description = STRALLOC(buf);
		char_to_room(pet, ch->in_room);
		add_follower(pet, ch);
		act(AT_YELLOW, "Your new companion looks to you expectantly.", ch, NULL, NULL, TO_CHAR);
		act(AT_YELLOW, "$n welcomes $N as a companion.", ch, NULL, pet, TO_ROOM);
		return;
	}
	else
	{
		CHAR_DATA *keeper;
		OBJ_DATA *obj, *buy_obj = NULL, *bag = NULL;
		OBJ_INDEX_DATA *bag_index;
		int cost, unit_cost;
		int noi = 1;
		const int mnoi = 20;

		if((keeper = find_keeper(ch)) == NULL) return;
		maxgold = (long long)keeper->level * keeper->level * 50000 > INT_MAX ? INT_MAX : keeper->level * keeper->level * 50000;

		if(is_number(arg))
		{
			noi = atoi(arg);
			argument = one_argument(argument, arg);
			if(noi < 1 || noi > mnoi)
			{
				act(AT_TELL, "$n says, 'You may buy between one and twenty items at a time.'", keeper, NULL, ch, TO_VICT);
				return;
			}
		}
		if(arg[0] == '\0') {act(AT_YELLOW, "Buy which item?", ch, NULL, NULL, TO_CHAR); return;}

		obj = get_obj_carry(keeper, arg);
		if(!obj || obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj))
		{
			act(AT_TELL, "$n says, 'I don't have that for sale. Have a look at my list.'", keeper, NULL, ch, TO_VICT);
			return;
		}

		unit_cost = get_cost(ch, keeper, obj, TRUE);
		if(unit_cost <= 0 || (long long)unit_cost * noi > INT_MAX)
		{
			act(AT_TELL, "$n says, 'That isn't available for sale.'", keeper, NULL, ch, TO_VICT);
			return;
		}
		cost = unit_cost * noi;

		if(!IS_OBJ_STAT(obj, ITEM_INVENTORY) && noi > 1)
		{
			act(AT_TELL, "$n says, 'I only have one of those in stock.'", keeper, NULL, ch, TO_VICT);
			return;
		}
		if(IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && get_trust(ch) < LEVEL_IMMORTAL)
		{
			act(AT_TELL, "$n says, 'That item is not ready for sale.'", keeper, NULL, ch, TO_VICT);
			return;
		}
		if(ch->gold < cost)
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YYou need %d more septim%s to buy that.&D", cost - ch->gold, cost - ch->gold == 1 ? "" : "s"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			return;
		}
		if((long long)ch->carry_number + (long long)get_obj_number(obj) * noi + (noi > 1 ? 1 : 0) > can_carry_n(ch))
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot carry that many items."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			return;
		}
		if((long long)ch->carry_weight + (long long)get_obj_weight(obj) * noi + (noi > 1 ? 2 : 0) > can_carry_w(ch))
		{
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot carry that much weight."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			return;
		}

		// Prepare generated stock before collecting payment.
		if(IS_OBJ_STAT(obj, ITEM_INVENTORY))
		{
			bag_index = NULL;
			if(noi > 1 && (bag_index = get_obj_index(OBJ_VNUM_SHOPPING_BAG)) == NULL)
			{
				bug("do_buy: missing shopping bag object %d", OBJ_VNUM_SHOPPING_BAG);
				{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The shopkeeper cannot package your order."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
				return;
			}
			buy_obj = create_object(obj->pIndexData, obj->level);
			if(!buy_obj) return;
			if(noi > 1)
			{
				bag = create_object(bag_index, 1);
				if(!bag) {extract_obj(buy_obj); return;}
				xSET_BIT(bag->extra_flags, ITEM_GROUNDROT);
				bag->timer = 10;
				bag->value[0] = bag->weight + (buy_obj->weight * noi);
				buy_obj->count = noi;
				obj->pIndexData->count += (noi - 1);
				numobjsloaded += (noi - 1);
			}
		}

		if(noi == 1)
		{
			act(AT_ACTION, "$n purchases $p.", ch, obj, NULL, TO_ROOM);
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GYou buy &W%s &Gfor &Y%d &Gseptim%s.&D", obj->short_descr, cost, cost == 1 ? "" : "s"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		}
		else
		{
			act(AT_ACTION, "$n purchases several items from the merchant.", ch, NULL, NULL, TO_ROOM);
			{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GYou buy &W%d x %s &Gfor &Y%d &Gseptims.&D", noi, obj->short_descr, cost); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
			act(AT_ACTION, "$n packs your order into a bag.", keeper, NULL, ch, TO_VICT);
		}

		ch->gold -= cost;
		if((long long)keeper->gold + cost > INT_MAX) keeper->gold = INT_MAX;
		else keeper->gold += cost;
		if(keeper->gold > maxgold)
		{
			boost_economy(keeper->in_room->area, keeper->gold - maxgold /2);
			keeper->gold = maxgold /2;
		}

		if(buy_obj)
		{
			if(bag) {obj_to_obj(buy_obj, bag); obj_to_char(bag, ch);}
			else obj_to_char(buy_obj, ch);
		}
		else
		{
			obj_from_char(obj);
			obj_to_char(obj, ch);
		}
		SHOP_REACTION(ch, keeper, 1);
		mprog_sell_trigger(keeper, ch, obj);
		return;
	}
}
void do_list				(CHAR_DATA *ch, const char *argument){
	// list [level] [level] [item]: original level markers are retained.
	if(xIS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
	{
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA *pet;
		bool found = FALSE;

		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
		if(!pRoomIndexNext)
		{
			bug("do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
			act(AT_YELLOW, "No companions are available here.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		send_to_pager("&Y+------------------------------------------------------------------+&D\r\n", ch);
		send_to_pager("&Y| &WC O M P A N I O N S   F O R   A D O P T I O N                      &Y|&D\r\n", ch);
		send_to_pager("&Y+------------------------------------------------------------------+&D\r\n", ch);
		for(pet = pRoomIndexNext->first_person; pet; pet = pet->next_in_room)
		{
			if(IS_NPC(pet) && xIS_SET(pet->act, ACT_PET))
			{
				found = TRUE;
				pager_printf(ch, "&CLevel %-3d &Y%8lld septims &W%s&D\r\n", pet->level, 10LL * pet->level * pet->level, pet->short_descr);
			}
		}
		if(!found) send_to_pager("&wNo companions are available at present.&D\r\n", ch);
		send_to_pager("&Y+------------------------------------------------------------------+&D\r\n", ch);
		return;
	}
	else
	{
		char arg[MAX_INPUT_LENGTH];
		const char *rest;
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost, lower = -1, upper = -1;
		bool found = FALSE;

		rest = one_argument(argument, arg);
		if((keeper = find_keeper(ch)) == NULL) return;

		if(is_number(arg))
		{
			upper = atoi(arg);
			rest = one_argument(rest, arg);
			if(is_number(arg))
			{
				lower = atoi(arg);
				rest = one_argument(rest, arg);
				if(lower > upper) {int tmp = lower; lower = upper; upper = tmp;}
			}
		}

		pager_printf(ch, "&Y+------------------------------------------------------------------+&D\r\n");
		pager_printf(ch, "&Y| &W%s's wares&D\r\n", PERS(keeper, ch));
		pager_printf(ch, "&Y| &wOpen: %02d:00 - %02d:00    Your purse: &Y%d &wseptims&D\r\n", keeper->pIndexData->pShop->open_hour, keeper->pIndexData->pShop->close_hour, ch->gold);
		pager_printf(ch, "&Y+------------------------------------------------------------------+&D\r\n");
		pager_printf(ch, "&C Lvl  Type        Price   Goods&D\r\n");
		pager_printf(ch, "&Y--------------------------------------------------------------------&D\r\n");

		for(obj = keeper->first_carrying; obj; obj = obj->next_content)
		{
			if(obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj)) continue;
			cost = get_cost(ch, keeper, obj, TRUE);
			if(cost <= 0 || (arg[0] != '\0' && !nifty_is_name(arg, obj->name))) continue;
			if(IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && get_trust(ch) < LEVEL_IMMORTAL) continue;

			found = TRUE;
			if(upper >= 0 && obj->level <= upper)
			{
				pager_printf(ch, "&w------------------------- level %d -------------------------&D\r\n", upper);
				upper = -1;
			}
			if(lower >= 0 && obj->level <= lower)
			{
				pager_printf(ch, "&w------------------------- level %d -------------------------&D\r\n", lower);
				lower = -1;
			}
			pager_printf(ch, "&W%4d  &C%-10.10s &Y%7d  %s%s&D\r\n", obj->level, SHOP_ITEM_CATEGORY(obj), cost, SHOP_RARITY_COLOR(obj), obj->short_descr);
		}
		if(!found) send_to_pager("&wNo matching goods are currently on sale.&D\r\n", ch);
		pager_printf(ch, "&Y--------------------------------------------------------------------&D\r\n");
		pager_printf(ch, "&wTry: &Cbuy <item>  &W/ &Cbuy <amount> <item>  &W/ &Cvalue <item>&D\r\n");
	}
}
void do_sell				(CHAR_DATA *ch, const char *argument){
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;

	one_argument(argument, arg);
	if(arg[0] == '\0') {act(AT_YELLOW, "Sell what?", ch, NULL, NULL, TO_CHAR); return;}
	if((keeper = find_keeper(ch)) == NULL) return;

	if((obj = get_obj_carry(ch, arg)) == NULL) {act(AT_YELLOW, "You don't have that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(!can_see_obj(keeper, obj)) {act(AT_YELLOW, "The merchant cannot examine that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(!can_drop_obj(ch, obj)) {act(AT_YELLOW, "You cannot part with that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(obj->timer > 0) {act(AT_YELLOW, "The merchant refuses goods that won't last.", ch, NULL, NULL, TO_CHAR); return;}
	if((cost = get_cost(ch, keeper, obj, FALSE)) <= 0) {act(AT_YELLOW, "The merchant isn't interested in that.", ch, NULL, NULL, TO_CHAR); return;}
	if(cost > keeper->gold) {act(AT_YELLOW, "The merchant doesn't have enough septims.", ch, NULL, NULL, TO_CHAR); return;}
	if((long long)ch->gold + cost > INT_MAX) {act(AT_YELLOW, "Your purse cannot hold that much gold.", ch, NULL, NULL, TO_CHAR); return;}

	separate_obj(obj);
	act(AT_ACTION, "$n trades $p to the merchant.", ch, obj, NULL, TO_ROOM);
	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&GYou sell &W%s &Gfor &Y%d &Gseptim%s.&D", obj->short_descr, cost, cost == 1 ? "" : "s"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	ch->gold += cost;
	keeper->gold -= cost;

	if(obj->item_type == ITEM_TRASH) extract_obj(obj);
	else {obj_from_char(obj); obj_to_char(obj, keeper);}
	SHOP_REACTION(ch, keeper, 2);
}
void do_value				(CHAR_DATA *ch, const char *argument){
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;

	if(!argument || argument[0] == '\0') {act(AT_YELLOW, "Value what?", ch, NULL, NULL, TO_CHAR); return;}
	if((keeper = find_keeper(ch)) == NULL) return;
	if((obj = get_obj_carry(ch, argument)) == NULL) {act(AT_YELLOW, "You don't have that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(!can_drop_obj(ch, obj)) {act(AT_YELLOW, "You cannot part with that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(!can_see_obj(keeper, obj)) {act(AT_YELLOW, "The merchant cannot appraise that item.", ch, NULL, NULL, TO_CHAR); return;}
	if((cost = get_cost(ch, keeper, obj, FALSE)) <= 0) {act(AT_YELLOW, "The merchant isn't interested in that.", ch, NULL, NULL, TO_CHAR); return;}

	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WThe merchant offers &Y%d &Wseptim%s for &C%s&D.", cost, cost == 1 ? "" : "s", obj->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	if(cost > keeper->gold) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&YThe merchant cannot afford the offer at present.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	ch->reply = keeper;
}
void repair_one_obj			(CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, 
	const char *arg, int maxgold, const char *fixstr, const char *fixstr2){ 				//Repair a single object. Used when handling "repair all"

   char buf[MAX_STRING_LENGTH];
   int cost;

    if(!can_drop_obj(ch, obj))
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&YYou can't let go of %s.&D", obj->short_descr); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
	}
	else if((cost = get_repaircost(keeper, obj)) < 0)
	{
		if( cost != -2 ) {act(AT_YELLOW, "The item cannot be repaired.", ch, NULL, NULL, TO_CHAR);}
		else			 {act(AT_YELLOW, "The item is already in perfect condition."   , ch, NULL, NULL, TO_CHAR);}
	}

   //"repair all" gets a 10% surcharge - Gorog 
	else if((cost = strcmp("all", arg) ? cost : SHOP_BULK_REPAIR_COST(cost)) > ch->gold)
	{
		snprintf(buf, sizeof(buf), "It will cost %d septim%s to %s $p.&D", cost, cost == 1 ? "" : "s", fixstr);
		act(AT_YELLOW, buf, ch, obj, NULL, TO_CHAR);
		act(AT_YELLOW, "You can't afford this.&D", ch, NULL, NULL, TO_CHAR);
	}
	else
	{
		snprintf(buf, MAX_STRING_LENGTH, "$n gives $p to $N, who quickly %s it.", fixstr2);
		act( AT_YELLOW, buf, ch, obj, keeper, TO_ROOM );
		
		snprintf( buf, MAX_STRING_LENGTH, "$N charges you %d septim%s to %s $p.", cost, cost == 1 ? "" : "s", fixstr);
		act(AT_YELLOW, buf, ch, obj, keeper, TO_CHAR);
		
		ch->gold -= cost;
		if((long long)keeper->gold + cost > INT_MAX) keeper->gold = INT_MAX;
		else keeper->gold += cost;
		SHOP_REACTION(ch, keeper, 3);
		
		if(keeper->gold < 0)
		{
			keeper->gold = 0;
		}
		else if( keeper->gold > maxgold )
		{
			boost_economy(keeper->in_room->area, keeper->gold - maxgold /2);
			keeper->gold = maxgold /2;
			act(AT_YELLOW, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM);
		}

		switch (obj->item_type)
		{
			default: 			act(AT_YELLOW, "For some reason, you think you got ripped off.&D", ch, NULL, NULL, TO_CHAR); break;
			case ITEM_ARMOR: 	obj->value[0] = obj->value[1]; break;
			case ITEM_WEAPON: 	obj->value[0] = INIT_WEAPON_CONDITION; break;
			case ITEM_WAND:
			case ITEM_STAFF: 	obj->value[2] = obj->value[1]; break;
		}

		oprog_repair_trigger(ch, obj);
	}
}
void do_repair				(CHAR_DATA *ch, const char *argument){
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	const char *fixstr;
	const char *fixstr2;
	int maxgold;

	if( argument[0] == '\0' )
		{act(AT_YELLOW, "Repair what?", ch, NULL, NULL, TO_CHAR);return;}

	if((keeper = find_fixer(ch)) == NULL)
	return;

	maxgold = (long long)keeper->level * keeper->level * 100000 > INT_MAX ? INT_MAX : keeper->level * keeper->level * 100000;
	
	switch (keeper->pIndexData->rShop->shop_type)
	{
		default:
		case SHOP_FIX: 		fixstr = "repair"; 	 fixstr2 = "repairs";   break;
		case SHOP_RECHARGE: fixstr = "recharge"; fixstr2 = "recharges"; break;
	}

	if(!strcmp(argument, "all"))
	{
		OBJ_DATA *obj_next;
		for(obj = ch->first_carrying; obj; obj = obj_next)
		{
			obj_next = obj->next_content;
			if(obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && can_see_obj(keeper, obj) 
			&& (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF))
			{
				repair_one_obj(ch, keeper, obj, argument, maxgold, fixstr, fixstr2);
				if(char_died(ch) || char_died(keeper)) return;
			}
		}
		return;
	}

	if((obj = get_obj_carry(ch, argument)) == NULL)
		{act(AT_YELLOW, "You don't have that item.", ch, NULL, NULL, TO_CHAR); return;}

	repair_one_obj(ch, keeper, obj, argument, maxgold, fixstr, fixstr2);
}
void appraise_all			(CHAR_DATA *ch, CHAR_DATA *keeper, const char *fixstr){
	OBJ_DATA *obj;
	long long total = 0;
	int cost, quoted;
	bool found = FALSE;

	for(obj = ch->first_carrying; obj; obj = obj->next_content)
	{
		if(obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj) || !can_see_obj(keeper, obj)) continue;
		if(!can_drop_obj(ch, obj)) continue;
		cost = get_repaircost(keeper, obj);
		if(cost < 0) continue;
		quoted = SHOP_BULK_REPAIR_COST(cost);
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&C%-38.38s &Y%8d &wseptims&D", obj->short_descr, quoted); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
		total += quoted;
		found = TRUE;
	}

	if(!found) {{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&WNothing in your inventory needs service here.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);} return;}
	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&WTotal to %s everything: &Y%lld &Wseptims (10%% bulk surcharge).&D", fixstr, total); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
}
void do_appraise			(CHAR_DATA *ch, const char *argument){
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	const char *fixstr;

	one_argument(argument, arg);

	if(arg[0] == '\0')
	{
		act(AT_YELLOW, "Appraise what?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if((keeper = find_fixer(ch)) == NULL)
    return;

	switch (keeper->pIndexData->rShop->shop_type)
	{
		default:
		case SHOP_FIX: 		fixstr = "repair";   break;
		case SHOP_RECHARGE: fixstr = "recharge"; break;
	}

	if(!strcmp(arg, "all"))
	{
      appraise_all(ch, keeper, fixstr);
      return;
	}

	if((obj = get_obj_carry(ch, arg)) == NULL) {act(AT_YELLOW, "You don't have that item.", ch, NULL, NULL, TO_CHAR); return;}
	if(!can_drop_obj(ch, obj)) 				   {act(AT_YELLOW, "You can't let go of it."  , ch, NULL, NULL, TO_CHAR); return;}
	
	if((cost = get_repaircost(keeper, obj)) < 0)
	{
		if(cost != -2) {act( AT_YELLOW, "Shop owner cannot do anything with this.", ch, NULL, NULL, TO_CHAR);}
		else 		   {act( AT_YELLOW, "This item is just... fine."			  , ch, NULL, NULL, TO_CHAR);}
		return;
	}

	snprintf(buf, MAX_STRING_LENGTH, "It will cost %d septim%s to %s that.", cost, cost == 1 ? "" : "s", fixstr );
	act(AT_YELLOW, buf, ch, NULL, keeper, TO_CHAR);
	
	if(cost > ch->gold) 
		{act( AT_YELLOW, "You cannot afford this.", ch, NULL, NULL, TO_CHAR);}
	
   return;
}
void do_makeshop			(CHAR_DATA *ch, const char *argument){
   SHOP_DATA *shop;
   int vnum;
   MOB_INDEX_DATA *mob;

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: makeshop <mobvnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( mob->pShop )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile already has a shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   CREATE( shop, SHOP_DATA, 1 );

   LINK( shop, first_shop, last_shop, next, prev );
   shop->keeper = vnum;
   shop->profit_buy = 120;
   shop->profit_sell = 90;
   shop->open_hour = 0;
   shop->close_hour = 23;
   mob->pShop = shop;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_shopset 			(CHAR_DATA *ch, const char *argument){
   SHOP_DATA *shop;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int vnum, value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: shopset <mob vnum> <field> value"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\nField being one of:\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( arg1 );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( !mob->pShop )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile doesn't keep a shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   shop = mob->pShop;
   value = atoi( argument );

   if( !str_cmp( arg2, "buy0" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->buy_type[0] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "buy1" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->buy_type[1] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "buy2" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->buy_type[2] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "buy3" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->buy_type[3] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "buy4" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->buy_type[4] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "buy" ) )
   {
      if( value <= ( shop->profit_sell + 5 ) || value > 1000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->profit_buy = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "sell" ) )
   {
      if( value < 0 || value >= ( shop->profit_buy - 5 ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->profit_sell = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "open" ) )
   {
      if( value < 0 || value > 23 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->open_hour = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "close" ) )
   {
      if( value < 0 || value > 23 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      shop->close_hour = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "keeper" ) )
   {
      if( ( mob2 = get_mob_index( value ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_medit( ch, mob2 ) )
         return;
      if( mob2->pShop )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mobile already has a shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      mob->pShop = NULL;
      mob2->pShop = shop;
      shop->keeper = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_shopset( ch, "" );
   return;
}
void do_shopstat			(CHAR_DATA *ch, const char *argument){
	SHOP_DATA *shop;
	MOB_INDEX_DATA *mob;
	int vnum;

	if(argument[0] == '\0')
		{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: shopstat <keeper vnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);} return;}

	vnum = atoi(argument);

	if((mob = get_mob_index(vnum)) == NULL) 
		{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);} return;}
	
	if(!mob->pShop)							
		{{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile doesn't keep a shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);} return;}
	
	shop = mob->pShop;

	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Keeper: %d  %s", shop->keeper, mob->short_descr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
	{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]", o_types[shop->buy_type[0]], o_types[shop->buy_type[1]], o_types[shop->buy_type[2]], o_types[shop->buy_type[3]], o_types[shop->buy_type[4]]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Profit:  buy %3d%%  sell %3d%%", shop->profit_buy, shop->profit_sell); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hours:   open %2d  close %2d", shop->open_hour, shop->close_hour); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_shops				(CHAR_DATA *ch, const char *argument){
	SHOP_DATA *shop;

	if(!first_shop)
	{
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no shops."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
		return;
	}

	set_char_color(AT_NOTE, ch);
	
	for(shop = first_shop; shop; shop = shop->next)
		{char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Keeper: %5d Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d", shop->keeper, shop->profit_buy, shop->profit_sell, shop->open_hour, shop->close_hour, shop->buy_type[0], shop->buy_type[1], shop->buy_type[2], shop->buy_type[3], shop->buy_type[4]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
	return;
}
void do_makerepair			(CHAR_DATA *ch, const char *argument){
   REPAIR_DATA *repair;
   int vnum;
   MOB_INDEX_DATA *mob;

   if( !argument || argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: makerepair <mobvnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( mob->rShop )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile already has a repair shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   CREATE( repair, REPAIR_DATA, 1 );

   LINK( repair, first_repair, last_repair, next, prev );
   repair->keeper = vnum;
   repair->profit_fix = 100;
   repair->shop_type = SHOP_FIX;
   repair->open_hour = 0;
   repair->close_hour = 23;
   mob->rShop = repair;
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_repairset			(CHAR_DATA *ch, const char *argument){
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int vnum, value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: repairset <mob vnum> <field> value"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      send_to_char( "\r\nField being one of:\r\n", ch );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "  fix0 fix1 fix2 profit type open close keeper"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( arg1 );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( !mob->rShop )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile doesn't keep a repair shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   repair = mob->rShop;
   value = atoi( argument );

   if( !str_cmp( arg2, "fix0" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->fix_type[0] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "fix1" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->fix_type[1] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "fix2" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid item type!"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->fix_type[2] = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "profit" ) )
   {
      if( value < 1 || value > 1000 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->profit_fix = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( value < 1 || value > 2 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->shop_type = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "open" ) )
   {
      if( value < 0 || value > 23 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->open_hour = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "close" ) )
   {
      if( value < 0 || value > 23 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Out of range."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      repair->close_hour = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg2, "keeper" ) )
   {
      if( ( mob2 = get_mob_index( value ) ) == NULL )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      if( !can_medit( ch, mob2 ) )
         return;
      if( mob2->rShop )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That mobile already has a repair shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      mob->rShop = NULL;
      mob2->rShop = repair;
      repair->keeper = value;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Done."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   do_repairset( ch, "" );
   return;
}
void do_repairstat			(CHAR_DATA *ch, const char *argument){
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob;
   int vnum;

   if( argument[0] == '\0' )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: repairstat <keeper vnum>"); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Mobile not found."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !mob->rShop )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "This mobile doesn't keep a repair shop."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }
   repair = mob->rShop;

   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Keeper: %d  %s", repair->keeper, mob->short_descr); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "fix0 [%s]  fix1 [%s]  fix2 [%s]", o_types[repair->fix_type[0]], o_types[repair->fix_type[1]], o_types[repair->fix_type[2]]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Profit: %3d%%  Type: %d", repair->profit_fix, repair->shop_type); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Hours:   open %2d  close %2d", repair->open_hour, repair->close_hour); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
void do_repairshops			(CHAR_DATA *ch, const char *argument){
   REPAIR_DATA *repair;

   if( !first_repair )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There are no repair shops."); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   set_char_color( AT_NOTE, ch );
   for( repair = first_repair; repair; repair = repair->next )
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "Keeper: %5d Profit: %3d Type: %d Open: %2d Close: %2d Fix: %2d %2d %2d", repair->keeper, repair->profit_fix, repair->shop_type, repair->open_hour, repair->close_hour, repair->fix_type[0], repair->fix_type[1], repair->fix_type[2]); act(AT_WHITE, "$t", ch, cMSG, NULL, TO_CHAR);}
   return;
}
