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
 *                            Chess Game Module                             *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "mud.h"
#include "chess.h"
#include "imc.h"

#define WHITE_BACKGROUND ""
#define BLACK_BACKGROUND ""
#define WHITE_FOREGROUND ""
#define BLACK_FOREGROUND ""

const char *big_pieces[MAX_PIECES][2] = {
   {"%s       ",
    "%s       "},
   {"%s  (-)  ",
    "%s  -|-  "},
   {"%s  ###  ",
    "%s  { }  "},
   {"%s  /-*- ",
    "%s / /   "},
   {"%s  () + ",
    "%s  {}-| "},
   {"%s   @   ",
    "%s  /+\\  "},
   {"%s  ^^^^^^  ",
    "%s  {@}  "},
   {"%s  [-]  ",
    "%s  -|-  "},
   {"%s  ###  ",
    "%s  [ ]  "},
   {"%s  /-*- ",
    "%s / /   "},
   {"%s  [] + ",
    "%s  {}-| "},
   {"%s   #   ",
    "%s  /+\\  "},
   {"%s  ^^^^^^  ",
    "%s  [#]  "}
};

const char small_pieces[MAX_PIECES + 1] = " prnbqkPRNBQK";

static char *print_big_board( CHAR_DATA * ch, GAME_BOARD_DATA * board )
{
   static char retbuf[MAX_STRING_LENGTH * 2];
   char square[64];
   int x, y, piece;
   const char *ground, *ink;

   /* Each cell is exactly four visible characters wide. Chess coordinates
    * remain A-H for rows and 1-8 for columns, as in the original command. */
   retbuf[0] = '\0';
   mudstrlcat(retbuf, "&Y+========================================+&D\r\n", sizeof(retbuf));
   mudstrlcat(retbuf, "&Y|       T A M R I E L   C H E S S        |&D\r\n", sizeof(retbuf));
   mudstrlcat(retbuf, "&Y+========================================+&D\r\n", sizeof(retbuf));
   mudstrlcat(retbuf, "&Y     1    2    3    4    5    6    7    8 &D\r\n", sizeof(retbuf));

   for(x = 0; x < 8; x++)
   {
      snprintf(square, sizeof(square), "&Y%c &D", 'A' + x);
      mudstrlcat(retbuf, square, sizeof(retbuf));
      for(y = 0; y < 8; y++)
      {
         piece = board->board[x][y];
         if(piece < NO_PIECE || piece >= MAX_PIECES) piece = NO_PIECE;
         ground = ((x + y) % 2 == 0) ? "^{24}" : "^{180}";
         ink = IS_WHITE(piece) ? "&{231}" : IS_BLACK(piece) ? "&{16}" : "&{245}";
         /* Dark background + black piece: use a bright contrasting tint. */
         if(IS_BLACK(piece) && ((x + y) % 2 == 0)) ink = "&{220}";
         snprintf(square, sizeof(square), "%s%s  %c  &d", ground, ink,
                  piece == NO_PIECE ? ' ' : small_pieces[piece]);
         mudstrlcat(retbuf, square, sizeof(retbuf));
      }
      mudstrlcat(retbuf, "\r\n", sizeof(retbuf));
   }
   mudstrlcat(retbuf, "&YWhite: lowercase  &W(p r n b q k)&Y   Black: uppercase &W(P R N B Q K)&D\r\n", sizeof(retbuf));
   return retbuf;
}

static void init_board( GAME_BOARD_DATA * board )
{
   int x, y;
   for( x = 0; x < 8; x++ )
      for( y = 0; y < 8; y++ )
         board->board[x][y] = 0;
   board->board[0][0] = WHITE_ROOK;
   board->board[0][1] = WHITE_KNIGHT;
   board->board[0][2] = WHITE_BISHOP;
   board->board[0][3] = WHITE_QUEEN;
   board->board[0][4] = WHITE_KING;
   board->board[0][5] = WHITE_BISHOP;
   board->board[0][6] = WHITE_KNIGHT;
   board->board[0][7] = WHITE_ROOK;
   for( x = 0; x < 8; x++ )
      board->board[1][x] = WHITE_PAWN;
   for( x = 0; x < 8; x++ )
      board->board[6][x] = BLACK_PAWN;
   board->board[7][0] = BLACK_ROOK;
   board->board[7][1] = BLACK_KNIGHT;
   board->board[7][2] = BLACK_BISHOP;
   board->board[7][3] = BLACK_QUEEN;
   board->board[7][4] = BLACK_KING;
   board->board[7][5] = BLACK_BISHOP;
   board->board[7][6] = BLACK_KNIGHT;
   board->board[7][7] = BLACK_ROOK;
   board->player1 = NULL;
   board->player2 = NULL;
   board->turn = 0;
   board->type = TYPE_LOCAL;
}

static bool find_piece( GAME_BOARD_DATA * board, int *x, int *y, int piece )
{
   int a, b;

   for(a = 0; a < 8; a++)
      for(b = 0; b < 8; b++)
         if(board->board[a][b] == piece)
         {
            *x = a;
            *y = b;
            return TRUE;
         }

   *x = -1;
   *y = -1;
   return FALSE;
}

#define SAME_COLOR(x1,y1,x2,y2)	\
    ((IS_WHITE(board->board[x1][y1]) && IS_WHITE(board->board[x2][y2])) || \
    (IS_BLACK(board->board[x1][y1]) && IS_BLACK(board->board[x2][y2])))

static bool king_in_check( GAME_BOARD_DATA * board, int piece )
{
   int x = 0, y = 0, l, m;

   if( piece != WHITE_KING && piece != BLACK_KING )
      return FALSE;

   if( !find_piece( board, &x, &y, piece ) )
      return FALSE;

   if( x < 0 || y < 0 || x > 7 || y > 7 )
      return FALSE;

   /*
    * pawns 
    */
   if( IS_WHITE( piece ) && x < 7 &&
       ( ( y > 0 && board->board[x + 1][y - 1] == BLACK_PAWN ) || ( y < 7 && board->board[x + 1][y + 1] == BLACK_PAWN ) ) )
      return TRUE;
   else if( IS_BLACK( piece ) && x > 0 &&
            ( ( y > 0 && board->board[x - 1][y - 1] == WHITE_PAWN ) || ( y < 7 && board->board[x - 1][y + 1] == WHITE_PAWN ) ) )
      return TRUE;
   /*
    * knights 
    */
   if( x - 2 >= 0 && y - 1 >= 0 &&
       ( ( board->board[x - 2][y - 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 2][y - 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x - 2 >= 0 && y + 1 < 8 &&
       ( ( board->board[x - 2][y + 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 2][y + 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x - 1 >= 0 && y - 2 >= 0 &&
       ( ( board->board[x - 1][y - 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 1][y - 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x - 1 >= 0 && y + 2 < 8 &&
       ( ( board->board[x - 1][y + 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 1][y + 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x + 1 < 8 && y - 2 >= 0 &&
       ( ( board->board[x + 1][y - 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 1][y - 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x + 1 < 8 && y + 2 < 8 &&
       ( ( board->board[x + 1][y + 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 1][y + 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x + 2 < 8 && y - 1 >= 0 &&
       ( ( board->board[x + 2][y - 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 2][y - 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x + 2 < 8 && y + 1 < 8 &&
       ( ( board->board[x + 2][y + 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 2][y + 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   /*
    * horizontal/vertical long distance 
    */
   for( l = x + 1; l < 8; l++ )
      if( board->board[l][y] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, y ) )
            break;
         if( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
             board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( l = x - 1; l >= 0; l-- )
      if( board->board[l][y] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, y ) )
            break;
         if( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
             board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( m = y + 1; m < 8; m++ )
      if( board->board[x][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, x, m ) )
            break;
         if( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
             board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( m = y - 1; m >= 0; m-- )
      if( board->board[x][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, x, m ) )
            break;
         if( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
             board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
            return TRUE;
         break;
      }
   /*
    * diagonal long distance 
    */
   for( l = x + 1, m = y + 1; l < 8 && m < 8; l++, m++ )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x - 1, m = y + 1; l >= 0 && m < 8; l--, m++ )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x + 1, m = y - 1; l < 8 && m >= 0; l++, m-- )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x - 1, m = y - 1; l >= 0 && m >= 0; l--, m-- )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   return FALSE;
}

static bool king_in_checkmate( GAME_BOARD_DATA * board, int piece )
{
   int x = 0, y = 0, dx, dy, sk = 0;

   if( piece != WHITE_KING && piece != BLACK_KING )
      return FALSE;

   if( !find_piece( board, &x, &y, piece ) )
      return FALSE;

   if( x < 0 || y < 0 || x > 7 || y > 7 )
      return FALSE;

   if( !king_in_check( board, board->board[x][y] ) )
      return FALSE;

   dx = x + 1;
   dy = y + 1;
   if( dx < 8 && dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y + 1;
   if( dx >= 0 && dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x + 1;
   dy = y - 1;
   if( dx < 8 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y - 1;
   if( dx >= 0 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x;
   dy = y + 1;
   if( dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x;
   dy = y - 1;
   if( dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x + 1;
   dy = y;
   if( dx < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y;
   if( dx >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   return TRUE;
}

static int is_valid_move( CHAR_DATA * ch, GAME_BOARD_DATA * board, int x, int y, int dx, int dy )
{
   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      return MOVE_INVALID;
   }

   if( !board )
   {
      bug( "%s: NULL board!", __func__ );
      return MOVE_INVALID;
   }

   if( dx < 0 || dy < 0 || dx > 7 || dy > 7 )
      return MOVE_OFFBOARD;

   if( board->board[x][y] == NO_PIECE )
      return MOVE_INVALID;

   if( x == dx && y == dy )
      return MOVE_INVALID;

   if( IS_WHITE( board->board[x][y] ) && !str_cmp( board->player1, ch->name ) )
      return MOVE_WRONGCOLOR;
   if( IS_BLACK( board->board[x][y] ) && ( !str_cmp( board->player2, ch->name ) || !ch ) )
      return MOVE_WRONGCOLOR;

   switch ( board->board[x][y] )
   {
      case WHITE_PAWN:
      case BLACK_PAWN:
         if( IS_WHITE( board->board[x][y] ) &&
             dx == x + 2 && x == 1 && dy == y && board->board[dx][dy] == NO_PIECE && board->board[x + 1][dy] == NO_PIECE )
            return MOVE_OK;
         else if( IS_BLACK( board->board[x][y] ) &&
                  dx == x - 2 && x == 6 && dy == y &&
                  board->board[dx][dy] == NO_PIECE && board->board[x - 1][dy] == NO_PIECE )
            return MOVE_OK;
         if( IS_WHITE( board->board[x][y] ) && dx != x + 1 )
            return MOVE_INVALID;
         else if( IS_BLACK( board->board[x][y] ) && dx != x - 1 )
            return MOVE_INVALID;
         if( dy != y && dy != y - 1 && dy != y + 1 )
            return MOVE_INVALID;
         if( dy == y )
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_OK;
            else if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            else
               return MOVE_BLOCKED;
         }
         else
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_INVALID;
            else if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            else if( board->board[dx][dy] != BLACK_KING && board->board[dx][dy] != WHITE_KING )
               return MOVE_TAKEN;
            else
               return MOVE_INVALID;
         }
         break;
      case WHITE_ROOK:
      case BLACK_ROOK:
      {
         int cnt;

         if( dx != x && dy != y )
            return MOVE_INVALID;

         if( dx == x )
         {
            for( cnt = y; cnt != dy; )
            {
               if( cnt != y && board->board[x][cnt] != NO_PIECE )
                  return MOVE_BLOCKED;
               if( dy > y )
                  cnt++;
               else
                  cnt--;
            }
         }
         else if( dy == y )
         {
            for( cnt = x; cnt != dx; )
            {
               if( cnt != x && board->board[cnt][y] != NO_PIECE )
                  return MOVE_BLOCKED;
               if( dx > x )
                  cnt++;
               else
                  cnt--;
            }
         }

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_KNIGHT:
      case BLACK_KNIGHT:
         if( ( dx == x - 2 && dy == y - 1 ) ||
             ( dx == x - 2 && dy == y + 1 ) ||
             ( dx == x - 1 && dy == y - 2 ) ||
             ( dx == x - 1 && dy == y + 2 ) ||
             ( dx == x + 1 && dy == y - 2 ) ||
             ( dx == x + 1 && dy == y + 2 ) || ( dx == x + 2 && dy == y - 1 ) || ( dx == x + 2 && dy == y + 1 ) )
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_OK;
            if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            return MOVE_TAKEN;
         }
         return MOVE_INVALID;
         break;
      case WHITE_BISHOP:
      case BLACK_BISHOP:
      {
         int l, m, blocked = FALSE;

         if( dx == x || dy == y )
            return MOVE_INVALID;

         l = x;
         m = y;

         while( 1 )
         {
            if( dx > x )
               l++;
            else
               l--;
            if( dy > y )
               m++;
            else
               m--;
            if( l > 7 || m > 7 || l < 0 || m < 0 )
               return MOVE_INVALID;
            if( l == dx && m == dy )
               break;
            if( board->board[l][m] != NO_PIECE )
               blocked = TRUE;
         }
         if( l != dx || m != dy )
            return MOVE_INVALID;

         if( blocked )
            return MOVE_BLOCKED;

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_QUEEN:
      case BLACK_QUEEN:
      {
         int l, m, blocked = FALSE;

         l = x;
         m = y;

         while( 1 )
         {
            if( dx > x )
               l++;
            else if( dx < x )
               l--;
            if( dy > y )
               m++;
            else if( dy < y )
               m--;
            if( l > 7 || m > 7 || l < 0 || m < 0 )
               return MOVE_INVALID;
            if( l == dx && m == dy )
               break;
            if( board->board[l][m] != NO_PIECE )
               blocked = TRUE;
         }
         if( l != dx || m != dy )
            return MOVE_INVALID;

         if( blocked )
            return MOVE_BLOCKED;

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_KING:
      case BLACK_KING:
      {
         int sp, sk;
         if( dx > x + 1 || dx < x - 1 || dy > y + 1 || dy < y - 1 )
            return MOVE_INVALID;
         sk = board->board[x][y];
         sp = board->board[dx][dy];
         board->board[x][y] = sp;
         board->board[dx][dy] = sk;
         if( king_in_check( board, sk ) )
         {
            board->board[x][y] = sk;
            board->board[dx][dy] = sp;
            return MOVE_CHECK;
         }
         board->board[x][y] = sk;
         board->board[dx][dy] = sp;
         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;
         if( SAME_COLOR( x, y, dx, dy ) )
            return MOVE_SAMECOLOR;
         return MOVE_TAKEN;
      }
         break;
      default:
         bug( "Invaild piece: %d", board->board[x][y] );
         return MOVE_INVALID;
   }

   if( ( IS_WHITE( board->board[x][y] ) && IS_WHITE( board->board[dx][dy] ) ) ||
       ( IS_BLACK( board->board[x][y] ) && IS_BLACK( board->board[dx][dy] ) ) )
      return MOVE_SAMECOLOR;

   return MOVE_OK;
}

#undef SAME_COLOR

#ifdef IMC
void imc_send_chess( const char *from, const char *to, const char *argument )
{
   IMC_PACKET *p;

   if( this_imcmud->state < IMC_ONLINE )
      return;

   p = imc_newpacket( from, "chess", to );
   imc_addtopacket( p, "text=%s", argument );
   imc_write_packet( p );

   /*
    * setdata(&out, getdata(ch));
    * 
    * imc_sncpy(out.to, to, IMC_NAME_LENGTH);
    * strcpy(out.type, "chess");
    * imc_addkey(&out.data, "text", argument);
    * 
    * imc_send(&out);
    * imc_freedata(&out.data); 
    */
}

PFUN( imc_recv_chess )
{
   CHAR_DATA *victim;
   char txt[LGST], buf[LGST];

   imc_getData( txt, "text", packet );

   /*
    * Chess packets must have a specific destination 
    */
   if( !str_cmp( q->to, "*" ) )
      return;

   if( !( victim = imc_find_user( imc_nameof( q->to ) ) ) )
   {
      if( !str_cmp( txt, "stop" ) )
         return;

      snprintf( buf, LGST, "%s is not here.", imc_nameof( q->to ) );
      imc_send_tell( "*", q->from, buf, 1 );
      return;
   }

   if( !victim->pcdata->game_board )
   {
      if( !str_cmp( txt, "stop" ) )
         return;

      snprintf( buf, LGST, "%s is not ready to be joined in a game.", imc_nameof( q->to ) );
      imc_send_tell( "*", q->from, buf, 1 );
      imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
      return;
   }

   if( !str_cmp( txt, "start" ) )
   {
      if( victim->pcdata->game_board->player2 != NULL )
      {
         snprintf( buf, LGST, "%s is already playing a game.", imc_nameof( q->to ) );
         imc_send_tell( "*", q->from, buf, 1 );
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
         return;
      }
      victim->pcdata->game_board->player2 = str_dup( q->from );
      victim->pcdata->game_board->turn = 0;
      victim->pcdata->game_board->type = TYPE_IMC;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s has joined your game.", q->from); act(AT_PLAIN, "$t", victim, cMSG, NULL, TO_CHAR);}
      imc_send_chess( victim->name, q->from, "accepted" );
      return;
   }

   if( !str_cmp( txt, "accepted" ) )
   {
      if( !victim->pcdata->game_board ||
          victim->pcdata->game_board->player2 == NULL ||
          victim->pcdata->game_board->type != TYPE_IMC || str_cmp( victim->pcdata->game_board->player2, q->from ) )
      {
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
         return;
      }
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "You have joined %s in a game.", q->from); act(AT_PLAIN, "$t", victim, cMSG, NULL, TO_CHAR);}
      if( victim->pcdata->game_board->player2 )
         DISPOSE( victim->pcdata->game_board->player2 );
      victim->pcdata->game_board->player2 = str_dup( q->from );
      victim->pcdata->game_board->turn = 1;
      return;
   }

   if( !str_cmp( txt, "stop" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s has stopped the game.", q->from); act(AT_PLAIN, "$t", victim, cMSG, NULL, TO_CHAR);}
      free_game( victim->pcdata->game_board );
      return;
   }

   if( !str_cmp( txt, "invalidmove" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You have issued an invalid move according to the other mud."); act(AT_PLAIN, "$t", victim, cMSG, NULL, TO_CHAR);}
      interpret( victim, "chess stop" );
      return;
   }

   if( !str_cmp( txt, "moveok" ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The other mud has accepted your move."); act(AT_PLAIN, "$t", victim, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_prefix( "move", txt ) )
   {
      char a, b;
      int x, y, dx, dy, ret;

      a = b = ' ';
      x = y = dx = dy = -1;

      if( sscanf( txt, "move %c%d %c%d", &a, &y, &b, &dy ) != 4 ||
          a < '0' || a > '7' || b < '0' || b > '7' || y < 0 || y > 7 || dy < 0 || dy > 7 )
      {
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from,
                         "invalidmove" );
         return;
      }

      x = a - '0';
      dx = b - '0';
      x = ( 7 - x );
      y = ( 7 - y );
      dx = ( 7 - dx );
      dy = ( 7 - dy );
      log_printf( "%d, %d -> %d, %d", x, y, dx, dy );
      ret = is_valid_move( NULL, victim->pcdata->game_board, x, y, dx, dy );
      if( ret == MOVE_OK || ret == MOVE_TAKEN )
      {
         GAME_BOARD_DATA *board;
         int piece, destpiece;

         board = victim->pcdata->game_board;
         piece = board->board[x][y];
         destpiece = board->board[dx][dy];
         board->board[dx][dy] = piece;
         board->board[x][y] = NO_PIECE;

         if( king_in_check( board, IS_WHITE( board->board[dx][dy] ) ? WHITE_KING : BLACK_KING ) &&
             ( board->board[dx][dy] != WHITE_KING && board->board[dx][dy] != BLACK_KING ) )
         {
            board->board[dx][dy] = destpiece;
            board->board[x][y] = piece;
         }
         else
         {
            board->turn++;
            imc_send_chess( board->player1 ? board->player1 : NULL, q->from, "moveok" );
            return;
         }
      }
      imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from,
                      "invalidmove" );
      return;
   }

   log_printf( "Unknown chess command from: %s, %s", q->from, txt );
}
#endif

void init_chess( void )
{
#ifdef IMC
   imc_register_packet_handler( "chess", imc_recv_chess );
#endif
}

void free_game( GAME_BOARD_DATA * board )
{
   if( !board )
      return;

#ifdef IMC
   if( board->type == TYPE_IMC )
   {
      imc_send_chess( board->player1 ? board->player1 : NULL, board->player2, "stop" );
      STRFREE( board->player2 );
   }
#endif

   if( board->player1 )
   {
      if( CHAR_DATA *ch = get_char_world( supermob, board->player1 ) ) // Added for bugfix - Findecano 23/11/07
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "The game has been stopped at %d total moves.", board->turn); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         ch->pcdata->game_board = NULL;
      }
   }

   if( board->player2 )
   {
      if( CHAR_DATA *ch = get_char_world( supermob, board->player2 ) ) // Added for bugfix - Findecano 23/11/07
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "The game has been stopped at %d total moves.", board->turn); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         ch->pcdata->game_board = NULL;
      }
   }
   STRFREE( board->player1 );
   STRFREE( board->player2 );
   DISPOSE( board );
}

void do_chess( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );

   if( IS_NPC( ch ) )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "NPC's can't be in chess games."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "begin" ) )
   {
      GAME_BOARD_DATA *board;

      if( ch->pcdata->game_board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are already in a chess match."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      CREATE( board, GAME_BOARD_DATA, 1 );
      init_board( board );
      ch->pcdata->game_board = board;
      ch->pcdata->game_board->player1 = QUICKLINK( ch->name );
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&{#C9A95A}You have started a game of chess.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "join" ) )
   {
      GAME_BOARD_DATA *board = NULL;
      CHAR_DATA *vch;
      char arg2[MAX_INPUT_LENGTH];

      if( ch->pcdata->game_board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are already in a game of chess."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Join whom in a chess match?"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

#ifdef IMC
      if( strstr( arg2, "@" ) )
      {
         if( !str_cmp( imc_mudof( arg2 ), this_imcmud->localname ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You cannot join IMC chess on the local mud!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( !str_cmp( imc_mudof( arg2 ), "*" ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "* is not a valid mud name."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         if( !str_cmp( imc_nameof( arg2 ), "*" ) )
         {
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "* is not a valid player name."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            return;
         }

         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Attempting to initiate IMC chess game..."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

         CREATE( board, GAME_BOARD_DATA, 1 );
         init_board( board );
         board->type = TYPE_IMC;
         board->player1 = QUICKLINK( ch->name );
         board->player2 = STRALLOC( arg2 );
         board->turn = -1;
         ch->pcdata->game_board = board;
         imc_send_chess( ch->name, arg2, "start" );
         return;
      }
#endif

      if( !( vch = get_char_world( ch, arg2 ) ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Cannot find that player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( IS_NPC( vch ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That player is an NPC, and cannot play games."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      board = vch->pcdata->game_board;
      if( !board )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That player is not playing a game."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( board->player2 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That game already has two players."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      board->player2 = QUICKLINK( ch->name );
      ch->pcdata->game_board = board;
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&{#8FD694}You have joined a game of chess.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      vch = get_char_world( ch, board->player1 );
      if(vch) {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "&{#8FD694}%s has joined your chess game.&D", ch->name); act(AT_PLAIN, "$t", vch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !ch->pcdata->game_board )
   {
      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: chess <begin|cease|status|board|move|join>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      return;
   }

   if( !str_cmp( arg, "cease" ) )
   {
      free_game( ch->pcdata->game_board );
      return;
   }

   if( !str_cmp( arg, "status" ) )
   {
      GAME_BOARD_DATA *board = ch->pcdata->game_board;

      if( !board->player1 )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no black player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else if( !str_cmp( board->player1, ch->name ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are black."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s is black.", board->player1); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( king_in_checkmate( board, BLACK_KING ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The black king is in checkmate!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else if( king_in_check( board, BLACK_KING ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The black king is in check."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( !board->player2 )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is no white player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else if( !str_cmp( board->player2, ch->name ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are white."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s is white.", board->player2); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( king_in_checkmate( board, WHITE_KING ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The white king is in checkmate!"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      else if( king_in_check( board, WHITE_KING ) )
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The white king is in check."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}

      if( !board->player2 || !board->player1 )
         return;

      {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%d turns.", board->turn); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
      if( board->turn % 2 == 1 && !str_cmp( board->player1, ch->name ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "It is %s's turn.", board->player2); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else if( board->turn % 2 == 0 && !str_cmp( board->player2, ch->name ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "It is %s's turn.", board->player1); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      else
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It is your turn."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }
      return;
   }

   if( !str_prefix( arg, "board" ) )
   {
      send_to_char( print_big_board( ch, ch->pcdata->game_board ), ch );
      return;
   }

   if( !str_prefix( arg, "move" ) )
   {
      CHAR_DATA *opp;
      char opp_name[MAX_INPUT_LENGTH];
      char a, b;
      int x, y, dx, dy, ret;

      if( !ch->pcdata->game_board->player1 || !ch->pcdata->game_board->player2 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "There is only 1 player."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->pcdata->game_board->turn < 0 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The game hasn't started yet."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( king_in_checkmate( ch->pcdata->game_board, BLACK_KING ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The black king has been checkmated, the game is over."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( king_in_checkmate( ch->pcdata->game_board, WHITE_KING ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "The white king has been checkmated, the game is over."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( !*argument )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: chess move [piece to move] [where to move]"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->pcdata->game_board->turn % 2 == 1 && !str_cmp( ch->pcdata->game_board->player1, ch->name ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It is not your turn."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( ch->pcdata->game_board->turn % 2 == 0 && !str_cmp( ch->pcdata->game_board->player2, ch->name ) )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "It is not your turn."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( sscanf( argument, "%c%d %c%d", &a, &y, &b, &dy ) != 4 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: chess move [dest] [source]"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      if( a < 'a' || a > 'h' || b < 'a' || b > 'h' || y < 1 || y > 8 || dy < 1 || dy > 8 )
      {
         {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid move, use a-h, 1-8."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
         return;
      }

      x = a - 'a';
      dx = b - 'a';
      --y;
      --dy;

      ret = is_valid_move( ch, ch->pcdata->game_board, x, y, dx, dy );
      if( ret == MOVE_OK || ret == MOVE_TAKEN )
      {
         GAME_BOARD_DATA *board;
         int piece, destpiece;

         board = ch->pcdata->game_board;
         piece = board->board[x][y];
         destpiece = board->board[dx][dy];
         board->board[dx][dy] = piece;
         board->board[x][y] = NO_PIECE;

         if( king_in_check( board, IS_WHITE( board->board[dx][dy] ) ? WHITE_KING : BLACK_KING ) )
         {
            board->board[dx][dy] = destpiece;
            board->board[x][y] = piece;
            ret = MOVE_INCHECK;
         }
         else
         {
            ++board->turn;
#ifdef IMC
            if( ch->pcdata->game_board->type == TYPE_IMC )
            {
               snprintf( arg, MAX_INPUT_LENGTH, "move %d%d %d%d", x, y, dx, dy );
               imc_send_chess( ch->pcdata->game_board->player1, ch->pcdata->game_board->player2, arg );
            }
#endif
         }
      }

      if( !str_cmp( ch->name, ch->pcdata->game_board->player1 ) )
      {
         opp = get_char_world( ch, ch->pcdata->game_board->player2 );
         if( !opp )
            mudstrlcpy( opp_name, ch->pcdata->game_board->player2, MAX_INPUT_LENGTH );
      }
      else
      {
         opp = get_char_world( ch, ch->pcdata->game_board->player1 );
         if( !opp )
            mudstrlcpy( opp_name, ch->pcdata->game_board->player1, MAX_INPUT_LENGTH );
      }

#ifdef IMC
#define SEND_TO_OPP(arg,opp) \
      if( opp ) \
      { \
         if( ch->pcdata->game_board->type == TYPE_LOCAL ) \
            ch_printf( (opp), "%s\r\n", (arg) ); \
      } \
      else \
      { \
         if( ch->pcdata->game_board->type == TYPE_IMC ) \
            imc_send_tell( ch->name, opp_name, (arg), 1 ); \
      }
#else
#define SEND_TO_OPP(arg,opp) \
      if( opp ) \
      { \
         if( ch->pcdata->game_board->type == TYPE_LOCAL ) \
            ch_printf( (opp), "%s\r\n", (arg) ); \
      }
#endif

      switch ( ret )
      {
         case MOVE_OK:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Ok."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            snprintf( arg, MAX_INPUT_LENGTH, "%s has moved.\r\n", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_INVALID:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Invalid move."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case MOVE_BLOCKED:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "You are blocked in that direction."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case MOVE_TAKEN:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&{#F0B35E}You capture an enemy piece.&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            snprintf( arg, MAX_INPUT_LENGTH, "%s has taken one of your pieces!", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_CHECKMATE:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That move would result in a checkmate."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            snprintf( arg, MAX_INPUT_LENGTH, "%s has attempted a move that would result in checkmate.", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_OFFBOARD:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That move would be off the board."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case MOVE_SAMECOLOR:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Your own piece blocks the way."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case MOVE_CHECK:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That move would result in a check."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            snprintf( arg, MAX_INPUT_LENGTH, "%s has made a move that would result in a check.", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_WRONGCOLOR:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "That is not your piece."); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         case MOVE_INCHECK:
            {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "&{#FF7070}Your king is in check. You must protect it!&D"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
            break;

         default:
            bug( "%s: Unknown return value", __func__ );
            break;
      }
#undef SEND_TO_OPP
      return;
   }
   {char cMSG[MAX_STRING_LENGTH]; sprintf(cMSG, "%s", "Usage: chess <begin|cease|status|board|move|join>"); act(AT_PLAIN, "$t", ch, cMSG, NULL, TO_CHAR);}
}
