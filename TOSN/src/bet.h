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
 *			Auction bid parsing routines			    *
 ****************************************************************************/
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>


/*
 * The following code was written by Erwin Andreasen for the automated
 * auction command.
 *
 * Completely cleaned up by Thoric
 */


/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.
*/

int advatoi( char *s )
{
   long long number = 0;
   int multiplier = 0;
   int digit;

   if( !s || !isdigit( (unsigned char)s[0] ) )
      return 0;

   while( isdigit( (unsigned char)s[0] ) )
   {
      digit = *s++ - '0';
      if( number > ( INT_MAX - digit ) / 10 )
         return 0;
      number = number * 10 + digit;
   }

   switch( UPPER( s[0] ) )
   {
      case 'K': multiplier = 1000;    ++s; break;
      case 'M': multiplier = 1000000; ++s; break;
      case '\0': break;
      default: return 0;
   }

   if( multiplier )
   {
      if( number > INT_MAX / multiplier )
         return 0;
      number *= multiplier;
   }

   while( isdigit( (unsigned char)s[0] ) && multiplier > 1 )
   {
      multiplier /= 10;
      digit = *s++ - '0';
      if( number > INT_MAX - digit * multiplier )
         return 0;
      number += digit * multiplier;
   }

   // Extra digits beyond the k/m precision are ignored, as before.
   if( s[0] != '\0' && !isdigit( (unsigned char)s[0] ) )
      return 0;

   return (int)number;
}

/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/
int parsebet( const int currentbet, char *s )
{
   long long result;
   long long amount = 0;
   int percent = 0;

   if( !s || !s[0] || currentbet < 0 )
      return 0;

   if( isdigit( (unsigned char)s[0] ) )
      return advatoi( s );

   if( s[0] != '+' && s[0] != '*' && s[0] != 'x' )
      return 0;

   if( s[1] == '\0' )
      amount = ( s[0] == '+' ) ? 25 : 2;
   else
   {
      int i;
      for( i = 1; s[i]; ++i )
      {
         if( !isdigit( (unsigned char)s[i] ) )
            return 0;
         if( amount > ( INT_MAX - ( s[i] - '0' ) ) / 10 )
            return 0;
         amount = amount * 10 + ( s[i] - '0' );
      }
   }

   if( s[0] == '+' )
   {
      percent = 100;
      result = (long long)currentbet * ( percent + amount ) / 100;
   }
   else
      result = (long long)currentbet * amount;

   if( result > INT_MAX )
      return 0;
   return (int)result;
}
