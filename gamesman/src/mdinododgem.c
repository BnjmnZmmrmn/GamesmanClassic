/************************************************************************
** NAME:        mdodgem.c
** DESCRIPTION: Dino Dodgem (Arbitrary Board Size)
** AUTHOR:      Can Chang, Desmond Cheung
** DATE:        2002-11-10
** UPDATE HIST: Version 1.0
**************************************************************************/

/*************************************************************************
** Everything below here must be in every game file
**************************************************************************/
#include <stdio.h>
#include "gamesman.h"
#include <math.h>

STRING   kAuthorName         = "Can Chang, Desmond Cheung";
STRING   kDBName             = "dinododgem";
STRING   kGameName           = "Dino Dodgem";
BOOLEAN  kPartizan           = TRUE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*	 gGameSpecificTclInit = NULL;

STRING   kHelpGraphicInterface =
"The LEFT button puts a small circle over your piece. This selects\n\
the FROM slot. The MIDDLE button then selects the TO slot. If you\n\
wish to remove a piece from the board, click the MIDDLE button on\n\
the same place as the FROM slot. The RIGHT button is the same as UNDO,\n\
in that it reverts back to your most recent position.";

STRING   kHelpTextInterface    =
"On your turn, use the LEGEND to determine which numbers to choose (between\n\
1 and 9, with 1 at the upper left and 9 at the lower right) to correspond\n\
to the location of your piece and the empty orthogonally-adjacent position\n\
you wish to move that piece to. If you wish to move a piece off of the board,\n\
choose 0 as your destination. Example: '2 0' moves the piece on location\n\
2 off of the board. '5 2' moves your piece from position 5 to position 2.";

STRING   kHelpOnYourTurn =
"The moves on your turn are different for different players. Here is a summary:\n\
\n\
O player:      The O player may move his pieces UP, DOWN and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
 ^             off of the board. You may only move one of your pieces to an\n\
 |             adjacent empty spot on your turn. Moving off of the board\n\
 O->           means moving one of your pieces past the RIGHT-HAND-side\n\
 |             of the board.\n\
 v\n\n\
X player:      The X player may move his pieces LEFT, UP, and to the RIGHT.\n\
               The objective is to be the first to move both of your pieces\n\
    ^          off of the board. You may only move one of your pieces to an\n\
    |          adjacent empty spot on your turn. Moving off of the board\n\
 <- X ->       means moving one of your pieces past the UPPER-side of the board.\n\n\
Note: The circle always goes first.";

STRING   kHelpStandardObjective =
"To be the FIRST player to move both your pieces off of the board OR prevent\n\
your opponent from moving.";

STRING   kHelpReverseObjective =
"To be the LAST player to move your pieces off of the board OR to be\n\
prevented from moving by your opponent's pieces.";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"";

STRING   kHelpExample =
"         ( 1 2 3 )           : O - -     PLAYER O's turn\n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
     Dan's move [(u)ndo/1-9] : 1 2                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - -                     \n\
         ( 7 8 9 )           : - X X                     \n\n\
Computer's move              : 9 6                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 2 3                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 8 7                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : O - X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 4 5                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : X - -                     \n\n\
Computer's move              : 7 8                       \n\n\
         ( 1 2 3 )           : - - O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - O X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
     Dan's move [(u)ndo/1-9] : 5 2                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - - X                     \n\
         ( 7 8 9 )           : - X -                     \n\n\
Computer's move              : 8 5                       \n\n\
         ( 1 2 3 )           : - O O     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
     Dan's move [(u)ndo/1-9] : 3 0                       \n\n\
         ( 1 2 3 )           : - O -     PLAYER X's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X X                     \n\
         ( 7 8 9 )           : - - -                     \n\n\
Computer's move              : 6 3                       \n\n\
         ( 1 2 3 )           : - O X     PLAYER O's turn \n\
LEGEND:  ( 4 5 6 )  TOTAL:   : - X -                     \n\
         ( 7 8 9 )           : - - -                     \n\n\n\
Computer wins. Nice try, dude.";

/*************************************************************************
** Everything above here must be in every game file
**************************************************************************/

/*************************************************************************
** Every variable declared here is only used in this file (game-specific)
**************************************************************************/
/* Default 3x3 board, changed in GameSpecificMenu. */
POSITION gNumberOfPositions;
POSITION gInitialPosition;
POSITION kBadPosition        = -1;    /* This can never be the rep. of a position */
int boardsize = 9;                    /* default boardsize */
int side = 3;                         /* get side length of board */
int offtheboard = 9;                  /* Removing that piece from the board */
#define BADSLOT         -2            /* You've moved off the board in a bad way */
POSITION* g3Array = NULL;             /* Powers of 3 */

/* Maximum length of a side (constrained by 32-bit machine) */
#define MAX_SIDE 5
 /* Minimum length of a side */
#define MIN_SIDE 3

typedef char BlankOX;
#define Blank 0
#define o 1
#define x 2

typedef int SLOT;     /* A slot is the place where a piece moves from or to */
char *gBlankOXString[] = { "-", "O", "X" };
BOOLEAN gToTrapIsToWin = FALSE;  /* Being stuck is when you can't move. */

BOOLEAN initialized = FALSE;

/* local function prototypes */
POSITION BlankOXToPosition(BlankOX*, BlankOX);
void PositionToBlankOX(POSITION, BlankOX*, BlankOX*);
BOOLEAN CantMove(POSITION);
POSITION DefaultInitialPosition();

/************************************************************************
** NAME:        InitializeGame
** DESCRIPTION: Initialize the gDatabase, a global variable.
************************************************************************/
void InitializeGame() {
  int i;
  int piecesArray[10];
  BlankOX board[boardsize];

  assert(boardsize==side*side);
  assert(offtheboard==boardsize);

  piecesArray[0] = Blank;
  piecesArray[1] = boardsize-2*(side-1); /* initial */
  piecesArray[2] = boardsize-1; /* all but 1 off board */
  piecesArray[3] = o;
  piecesArray[4] = 0;
  piecesArray[5] = side-1;
  piecesArray[6] = x;
  piecesArray[7] = 0;
  piecesArray[8] = side-1;
  piecesArray[9] = -1;

  

  gNumberOfPositions = generic_hash_init(boardsize, piecesArray, NULL);
  initialized = TRUE;

  /*
  if (g3Array != NULL) {
    SafeFree(g3Array);
  }
  g3Array = (POSITION*) SafeMalloc((boardsize+1) * sizeof(POSITION));
  g3Array[0] = 1;

  for (i=1; i<=boardsize; i++) {
    g3Array[i] = g3Array[i-1]*3;
  }

  gNumberOfPositions = (int) pow((double)3, (double)boardsize)*2;
  */
  gInitialPosition = DefaultInitialPosition();

  generic_unhash(gInitialPosition, board);
}

/************************************************************************
** NAME:        DebugMenu
** DESCRIPTION: Menu used to debug internal problems. Does nothing if
**              kDebugMenu == FALSE
************************************************************************/
void DebugMenu() { }

/************************************************************************
** NAME:        GameSpecificMenu
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
************************************************************************/
void GameSpecificMenu() {
  int c, n; // My char for board size input.
  
  do {
    printf("\n\t----- Game-specific options for %s -----\n\n", kGameName);
    
    printf("\tCurrent Initial Position:\n");
    PrintPosition(gInitialPosition, gPlayerName[kPlayerOneTurn], kHumansTurn);
    
    printf("\tI)\tChoose the (I)nitial position\n");
    printf("\tT)\t(T)rapping opponent toggle from %s to %s\n", 
	   gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)",
	   !gToTrapIsToWin ? "GOOD (WINNING)" : "BAD (LOSING)");
    printf("\ts)\tSet board (s)ize.\n");
    
    printf("\tb)\t(B)ack = Return to previous activity.\n");
  
    printf("\nSelect an option: ");
    
    switch(GetMyChar()) {
    case 'S': case 's': // Set board size game option.
      getchar();
      do {
	printf("\n\nYou have an n by n board. What do you wish n to be? (Between %d and %d, inclusive. Values >4 can cause memory errors)\n", MIN_SIDE, MAX_SIDE);
	c=getchar();
	c = c - '0';
	side = c;
	boardsize = side*side;
	offtheboard = boardsize;
      } while (side<MIN_SIDE || side>MAX_SIDE);
      InitializeGame();
      break;
    case 'Q': case 'q':
      ExitStageRight();
    case 'H': case 'h':
      HelpMenus();
      break;
    case 'I': case'i':
      gInitialPosition = GetInitialPosition();
      break;
    case 'T': case 't':
      gToTrapIsToWin = !gToTrapIsToWin;
      break;
    case 'b': case 'B':
      return;
    default:
      printf("\nSorry, I don't know that option. Try another.\n");
      HitAnyKeyToContinue();
      break;
    }
  } while(TRUE);
}
  
/************************************************************************
** NAME:        SetTclCGameSpecificOptions
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
************************************************************************/
void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
  gToTrapIsToWin = (BOOLEAN) theOptions[0];
}

/************************************************************************
** NAME:        DoMove
** DESCRIPTION: Apply the move to the position.
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
** OUTPUTS:     (POSITION) : The position that results after the move.
** CALLS:       MoveToSlots(MOVE,*SLOT,*SLOT)
** Note:        A move is an int with first 8 bits from right 
**              representing the TO position, and bits 8-16 from right 
**              representing the FROM position.
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove; {
  BlankOX theBlankOX[boardsize], whosTurn;
  int from, to;
  /* Unhash. */
  PositionToBlankOX(thePosition,theBlankOX,&whosTurn);
  /* Un-encrypt the move. */
  to = theMove & 255;
  from = (theMove >> 8) & 255;
  /* Do the move. */
  theBlankOX[from] = Blank;
  if (to != boardsize) {
    theBlankOX[to] = whosTurn;
  }
  /* Hash. */
  thePosition = BlankOXToPosition(theBlankOX, 3-whosTurn);
  /* Return hashed position. */
  return thePosition;
}

/************************************************************************
** NAME:        GetInitialPosition
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** OUTPUTS:     POSITION initialPosition : The position returned
************************************************************************/
POSITION GetInitialPosition()
{
  BlankOX theBlankOX[boardsize], whosTurn;
  signed char c;
  int i, goodInputs, row, col;
  int numX, numO;
  goodInputs = 0;
  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tYou board must have at least one x or o and no more than %d x's and %d o's\n", side-1, side-1);
  printf("\tNote that it should be in the following format:\n\n");
  /* Print example board. */
  printf("EXAMPLE of a %d by %d board:\n", side, side);
  row = 0;
  col = 0;
 
  while (row < side) {
    printf("\n\t");
    if (row != (side-1)) {
      printf("o");
      col=1;
      while (col < side) {
	printf (" -");
	col++;
      }
      col=0;
    }
    else {
      printf("-");
      while (col < side) {
	printf(" x");
	col++;
      }
    }
    col = 1;
    row++;
  }

  do {
    numX = numO = 0;
    printf("\nNow enter a new board:\n");
    
    /* Get inputted initial position. */
    i = 0;
    getchar();
    
    row = side - 1;
    col = 0;
    while (row >= 0) {
      while (col < side) {
	c = getchar();
	if(c == 'x' || c == 'X') {
	  numX++;
	  theBlankOX[((side*row)+col)] = x;
	  col++;
	}
	else if(c == 'o' || c == 'O' || c == '0') {
	  numO++;
	  theBlankOX[((side*row)+col)] = o;
	  col++;
	}
	else if(c == '-') {
	  theBlankOX[((side*row)+col)] = Blank;
	  col++;
	}
	else {
	}
      }
      row--;
      col = 0;
    }
    
    getchar();
    printf("\nNow, whose turn is it? [O/X] : ");
    scanf("%c",&c);
    if(c == 'x' || c == 'X')
      whosTurn = x;
    else
      whosTurn = o;

    if (numX>side-1 || numO>side-1 || (numX==0 && numO==0)) {
      printf("\nInvalid board configuration\n");
    }
  } while (numX>side-1 || numO>side-1 || (numX==0 && numO==0));

  return(BlankOXToPosition(theBlankOX,whosTurn));
}

/************************************************************************
** NAME:        PrintComputersMove
** DESCRIPTION: Nicely format the computers move.
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
************************************************************************/
void PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName; {
  int from, to;
  MOVE theMove = computersMove;
  /* Un-encrypt the move. */
  to = theMove & 255;
  from = (theMove >> 8) & 255;
  printf("%8s's move              : %d %d\n", computersName, from, to);
}

/************************************************************************
** NAME:        Primitive
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
** INPUTS:      POSITION position : The position to inspect.
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
** CALLS:       BOOLEAN ThreeInARow()
**              BOOLEAN AllFilledIn()
**              PositionToBlankOX()
**              BlankOX OnlyPlayerLeft(*BlankOX)
************************************************************************/
VALUE Primitive(position) 
     POSITION position;
{
  BOOLEAN ThreeInARow(), AllFilledIn();
  BlankOX theBlankOX[boardsize],whosTurn,OnlyPlayerLeft();

  PositionToBlankOX(position,theBlankOX,&whosTurn);
  
  if (OnlyPlayerLeft(theBlankOX) == whosTurn)
    return(gStandardGame ? lose : win);  /* cause you're the only one left */
  else if(CantMove(position)) /* the other player just won */
    return(gToTrapIsToWin ? lose : win);       /* !gStandardGame */
  else
    return(undecided);                    /* no one has won yet */
}

BOOLEAN CantMove(position)
     POSITION position;
{
  MOVELIST *ptr, *GenerateMoves();
  BOOLEAN cantMove;
  
  ptr = GenerateMoves(position);
  cantMove = (ptr == NULL);
  FreeMoveList(ptr);
  return(cantMove);
}


BlankOX OnlyPlayerLeft(theBlankOX)
     BlankOX *theBlankOX;
{
  int i;
  BOOLEAN sawO = FALSE, sawX = FALSE;
  for(i = 0 ; i < boardsize ; i ++) {
    sawO |= (theBlankOX[i] == o);
    sawX |= (theBlankOX[i] == x);
  }
  if(sawX && !sawO)
    return(x);
  else if (sawO && !sawX)
    return(o);
  else if (sawO && sawX)
    return(Blank);
  else {
    printf("Error in OnlyPlayerLeft! the board is blank!!!\n");
    return(Blank);
  }
}

/************************************************************************
** NAME:        PrintPosition
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
************************************************************************/
void PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
  int row = 0, col = 0;
  VALUE GetValueOfPosition();
  BlankOX theBlankOx[boardsize], whosTurn;

  PositionToBlankOX(position,theBlankOx,&whosTurn);
  for (row = side - 1; row >= 0; row--) {
    if (row == side-1)
      printf("\n\tBOARD:  ");
        else
	  printf("\n\t        ");
    for (col = 0; col < side; col++) {
      printf ("%s ", gBlankOXString[ (int) theBlankOx[((side*row)+col)] ]);
    }
    if (row == side-1)
      printf("\t LEGEND:  ");
        else
	  printf("\t          ");
    for (col = 0; col < side; col++){
      if (col+(row*side)>=10) {
	printf("%d ",  col+(row*side));
      }
      else {
	printf("%d  ",  col+(row*side));
      }
    }
  }
  printf("\n\n");

  if ( gBlankOXString[(int)whosTurn] == "O")
    printf("\n\
        ^     ** It is player O's turn to move\n\
        |     ** and this is the move you can\n\
        O->   ** perform on one of your pieces.\n\
        |\n\
        v \n\n");
      else
	printf("\n\
           ^       ** It is player X's turn to move\n\
           |       ** and this is the move you can\n\
        <- X ->    ** perform on one of your pieces.\n\n");
  printf("  Move format: [from to] \n\n");
  
  
//  PositionToBlankOX(position,theBlankOx,&whosTurn);
//  for (row = side - 1; row >= 0; row--) {
//    printf("\n\t");
//    for (col = 0; col < side; col++) {
//      printf ("%s ", gBlankOXString[ (int) theBlankOx[((side*row)+col)] ]);
//    }
//  }
//  printf("\n\n");

/*    if ( gBlankOXString[(int)whosTurn] == "O") */
/*      printf("  */
/*        ^     ** It is player O's turn to move */
/*        |     ** and this is the move you can   */
/*        O->   ** perform on one of your pieces. */
/*        |     */
/*        v \n\n"); */
/*    else */
/*      printf("             */
/*           ^       ** It is player X's turn to move */
/*           |       ** and this is the move you can */
/*        <- X ->    ** perform on one of your pieces.\n\n"); */
/*    printf("   **Enter your input in the following format: */
/*         <from slot #> < space > <to slot #>\n\n"); */
}

/************************************************************************
** NAME:        GenerateMoves
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
** INPUTS:      POSITION position : The position to branch off of.
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
**              in the linked list of moves that can be generated.
** CALLS:       GENERIC_PTR SafeMalloc(int)
************************************************************************/
MOVELIST *GenerateMoves(position)
     POSITION position;
{
  MOVELIST *head = NULL;
  MOVE theMove;
  MOVELIST *CreateMovelistNode();
  BlankOX theBlankOX[boardsize], whosTurn;
  int i,j;     /* Values for J: 0=left,1=straight,2=right */
  int left, right, up, down; /* Board position relative to current piece. */
  
  PositionToBlankOX(position,theBlankOX,&whosTurn);

  /* X */
  if (whosTurn == x) {
    for (i = 0; i < boardsize; i++) {
      if (theBlankOX[i] == x) {
	left = i - 1;
	right = i + 1;
	up = i + side;
        /* left */
	if ((i % side) != 0 && theBlankOX[left] == Blank) {
	  // add new move i to left
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += left;
	  head = CreateMovelistNode (theMove, head);
	}
        /* right */
	if ((i % side) != (side - 1) && theBlankOX[right] == Blank) {
	  // add new move i to right
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += right;
	  head = CreateMovelistNode (theMove, head);
	}
        /* up */
	if (up < boardsize && theBlankOX[up] == Blank) {
	  // add new move i to up
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += up;
	  head = CreateMovelistNode (theMove, head);
	}
	else if (up >= boardsize) {
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += boardsize;
	  head = CreateMovelistNode (theMove, head);
	}
      }
    }
  }
  /* O */
  else {
    for (i = 0; i < boardsize; i++) {
      if (theBlankOX[i] == o) {
	up = i + side;
	down = i - side;
	right = i + 1;
        /* up */
	if (up < boardsize && theBlankOX[up] == Blank) {
	  // add new move i to up
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += up;
	  head = CreateMovelistNode (theMove, head);
	}
        /* down */
	if (down >= 0 && theBlankOX[down] == Blank) {
	  // add new move i to down
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += down;
	  head = CreateMovelistNode (theMove, head);
	}
        /* right */
	if ((i % side) != (side - 1) && theBlankOX[right] == Blank) {
	  // add new move i to right
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += right;
	  head = CreateMovelistNode (theMove, head);
	}
	else if ((i % side) == (side - 1)) {
	  theMove = i;
	  theMove = theMove << 8;
	  theMove += boardsize;
	  head = CreateMovelistNode (theMove, head);
	}
      }
    }
  }
  return(head);
}

/************************************************************************
** NAME:        GetAndPrintPlayersMove
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
************************************************************************/
USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
{
  USERINPUT ret, HandleDefaultTextInput();
  int xpos, ypos;
  char input = '0', fromSlotChar, toSlotChar, HandleTextualInput();
  BOOLEAN done = FALSE, ValidMove();
  SLOT fromSlot = BADSLOT, toSlot;
  BlankOX theBlankOX[boardsize], whosTurn;

  PositionToBlankOX(thePosition,theBlankOX,&whosTurn);

  do {
    printf("%8s's move [(u)ndo/0-%d 0-%d] : ", playerName, boardsize-1, boardsize-1);
    
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);
  return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
** NAME:        ValidTextInput
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
** INPUTS:      STRING input : The string input the user typed.
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
  SLOT fromSlot, toSlot;
  int ret;

  ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
  return(ret == 2 &&
	 fromSlot <= boardsize && fromSlot >= 0 && toSlot <= boardsize && toSlot >= 0);
}

/************************************************************************
** NAME:        ConvertTextInputToMove
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
** INPUTS:      STRING input : The string input the user typed.
** OUTPUTS:     MOVE : The move corresponding to the user's input.
************************************************************************/

MOVE ConvertTextInputToMove(input) STRING input; {
  SLOT fromSlot, toSlot;
  MOVE theMove;
  int ret;

  ret = sscanf(input,"%d %d", &fromSlot, &toSlot);
  /* Encrypt from and to into a MOVE. */
  theMove = fromSlot;
  theMove = theMove << 8;
  theMove += toSlot;
  return(theMove);
}

/************************************************************************
** NAME:        PrintMove
** DESCRIPTION: Print the move in a nice format.
** INPUTS:      MOVE *theMove         : The move to print. 
************************************************************************/
void PrintMove(theMove)
     MOVE theMove;
{
  SLOT from, to;
  
  /* Un-encrypt the move. */
  to = theMove & 255;
  from = (theMove >> 8) & 255;
  printf("[ %d %d ] ", from, to);
}

/************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
************************************************************************/

/************************************************************************
** NAME:        DefaultInitialPosition
** DESCRIPTION: Determines the default initial position
** INPUTS:      None
** OUTPUTS:     The integer hashcode that embodies the default initial position
************************************************************************/
POSITION DefaultInitialPosition() {
  BlankOX blankOX[boardsize];
  int i, j;

  for (i=0; i<boardsize; i++) {
      blankOX[i] = Blank;
  }
  for (i=1; i<side; i++) {
    blankOX[i*side] = o;
    blankOX[i] = x;
  }

  return BlankOXToPosition(blankOX, x);
}

/************************************************************************
** NAME:        BlankOXToPosition
** DESCRIPTION: Hashes internal representation of current game situation.
** INPUTS:      The current position array and whos turn.
** OUTPUTS:     The integer hashcode that embodies current position array and who's turn.
************************************************************************/
POSITION BlankOXToPosition(theBlankOX,whosTurn) BlankOX *theBlankOX,whosTurn; {
  return generic_hash(theBlankOX, whosTurn);
  /*
  int i;
  int position = 0;
  
  for (i = 0; i < boardsize; i++) {
      position += (theBlankOX[i] * g3Array[i]);
  }
  position = position << 1;
  position += whosTurn - 1;
  return(position);
  */
}

/************************************************************************
** NAME:        PositionToBlankOX
** DESCRIPTION: Unhashes thePos to current game situation.
** INPUTS:      An integer hashcode,
**              A container for the position,
**              A container for who's turn.
************************************************************************/
void PositionToBlankOX(thePos,theBlankOX,whosTurn) POSITION thePos; BlankOX *theBlankOX, *whosTurn; { 
  generic_unhash(thePos, theBlankOX);
  *whosTurn = whoseMove(thePos);
  /*
  int i;

  *whosTurn = (thePos & 1) + 1;
  thePos = thePos >> 1;
  for(i = 0; i < boardsize; i++) {
    theBlankOX[i] = (thePos / g3Array[i]) % 3;
  }
  */
}

/************************************************************************
 **              Database Functions
 ************************************************************************/
void setOption (int option) {
  option--;
  gStandardGame = (option%2 == 0);
  gToTrapIsToWin = (option/2%2 == 1);

  side = (option/2/2)+MIN_SIDE;
  boardsize = side*side;
  offtheboard = boardsize;

}

int getOption () {
  int option = 1;
  option += (gStandardGame ? 0 : 1);
  option += 2*(gToTrapIsToWin ? 1 : 0);
  option += 2*2*(side-MIN_SIDE);
  return option;
}

int NumberOfOptions () {
  return 2*2*(MAX_SIDE-MIN_SIDE+1);
}
