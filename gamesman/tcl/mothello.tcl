####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#set dotExpandAmount 10



#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game

    global kGameName
    set kGameName "Othello"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    #set gInitialPosition 593531
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    #these are set in GS_SetupRulesFrame
    global gRowsOption gColsOption

    global gMinRows gMinCols
    set gMinRows 3
    set gMinCols 3

    ### Set boardRows, boardCols, boardSize
    global boardRows boardCols boardSize
    set boardRows [expr $gRowsOption + $gMinRows]
    set boardCols [expr $gColsOption + $gMinCols]
    set boardSize [expr $boardRows * $boardCols]

    set gForceCapture 0

    ### Set the strings to be used in the Edit Rules

    #global kStandardString kMisereString
    #set kStandardString "To have the most pieces of your color on the board at the end of the game."
    #set kMisereString "To have the least pieces of your color on the board at the end of the game."

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    set gMisereGame 0
    if {!$gMisereGame} {
	SetToWinString "To Win: Have the most number of pieces of your color when the board is filled."
    } else {
	SetToWinString "To Win: Have the least number of pieces of your color when the board is filled."
    }
    SetToMoveString "To Move: Players take turns placing a piece on the board where a capture will result.  A capture is made when a piece is placed so that it and another piece of the same color sandwich one or more oppositely colored pieces.  All of the sandwiched pieces then switch to the player's color.  Captures can be made vertically, horizontally, and diagonally."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Michael Chen, Robert Liao"
    set kTclAuthors "Keaton Mowery, Victor Perez"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"


#     puts "After InitGameSpecific"
#     puts $boardRows
#     puts $boardCols
#     puts $gInitialPosition
}


#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {

    return [list black white]

}


#############################################################################
# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.
#############################################################################
proc GS_ColorOfPlayers {} {

    return [list black white]
    
}


#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    set captureRule \
	[list \
	     "Should capture moves be mandatory?" \
	     "Yes" \
	     "No" \
	    ]

    set boardColsRule \
	[list \
	     "Board Columns:" \
	     "3" \
	     "4" \
	    ]

    set boardRowsRule \
	[list \
	     "Board Rows:" \
	     "3" \
	     "4" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule $captureRule $boardColsRule $boardRowsRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gForcedCapture
    set gForcedCapture 0

    global boardCols boardRows

    global gRowsOption gColsOption
    set gRowsOption 1
    set gColsOption 1

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gForcedCapture" "gColsOption" "gRowsOption"]

    global kLabelFont
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
	    incr rulePartNum
	}
	incr ruleNum
    } 
}


#############################################################################
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
    global gMisereGame gForcedCapture
    global boardCols boardRows
    global gMinCols gMinRows
    global gColsOption gRowsOption

    set boardCols [expr $gMinCols + $gColsOption]
    set boardRows [expr $gMinRows + $gRowsOption]

#     puts "GetOption"
#     puts "cols:"
#     puts $boardCols
#     puts "rows:"
#     puts $boardRows

    set option [expr [expr $boardCols << 5] + [expr $boardRows << 1] + $gForcedCapture]

#     puts "tcl GetOption"
#     puts "rows: "
#     puts $boardRows
#     puts "cols: "
#     puts $boardCols

    return $option
}


#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {

    global gForcedCapture boardCols boardRows

#    puts "tcl SetOption\n"
    
    set gForcedCapture [expr $option & 0x1]
    set boardRows [expr [expr $option >> 0x01] & 0x0f]
    set boardCols [expr $option  >> 5]

#     puts "SetOption"
#     puts "rows: "
#     puts $boardRows
#     puts "cols: "
#     puts $boardCols

}


proc max { x y } {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
}

proc min { x y } {
    if {$x < $y} {
	return $x
    } else {
	return $y
    }
}

#############################################################################
##
## DrawCircle
##
## Here we draw a circle on (slotX,slotY) in window w with a tag of theTag
##
#############################################################################

proc DrawCircle { w slotSizeX slotSizeY slotX slotY theTag theColor } {

    set startCircle [expr [min $slotSizeX $slotSizeY] / 8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$slotSizeX]
    set cornerY     [expr $slotY*$slotSizeY]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline $theColor \
		       -fill $theColor \
		       -tag $theTag]

    $w move $theCircle $cornerX $cornerY

    $w addtag piece$slotX$slotY withtag $theCircle
    #$w addtag tagPieceCoord$slotX$slotY withtag $theCircle
    #$w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle

    return $theCircle
}

proc DrawMoveCircle { w slotSizeX slotSizeY slotX slotY theTag theColor } {
    
    set startCircle [expr [min $slotSizeX $slotSizeY] / 3.0]
    set endCircle   [expr $startCircle*2.0]
    set cornerX     [expr $slotX*$slotSizeX]
    set cornerY     [expr $slotY*$slotSizeY]
    set theCircle [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -fill $theColor \
		       -tag $theTag]

    $w move $theCircle $cornerX $cornerY

    $w addtag movedot$slotX$slotY withtag $theCircle

    return $theCircle
}


#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {
    global boardRows boardCols
    global gFrameWidth gFrameHeight

    #set mySize [min $gFrameWidth [expr $gFrameHeight * [expr 4.0/5]]]
    set mySize [min $gFrameWidth $gFrameHeight]

    set vertCellSize [expr $mySize / $boardRows]
    set horizCellSize [expr $mySize / $boardCols]
    
   for {set x 0} {$x < $boardCols} {incr x} {
	for {set y 0} {$y < $boardRows} {incr y} {
	    $c create rectangle [expr $x * $horizCellSize] [expr $y * $vertCellSize] [expr ($x + 1) * $horizCellSize] [expr ($y + 1) * $vertCellSize] -fill darkgreen -outline black -width 2 -tag base 

	}

    }

    for {set x 0} {$x < $boardCols} {incr x} {
	for {set y 0} {$y < $boardRows} {incr y} {

	    DrawCircle $c $horizCellSize $vertCellSize $x $y pieces [lindex [GS_ColorOfPlayers] 0]

	    DrawMoveCircle $c $horizCellSize $vertCellSize $x $y moves cyan

	    #$c bind movedot$x$y <Enter> "MouseOverExpand movedot$x$y $c"
	    #$c bind piece$x$y <Leave> "MouseOutContract piece$x$y $c"

	}
    }

    $c create text [expr $gFrameWidth / 2] [expr $gFrameHeight/2] -width [expr $gFrameWidth * .9] -font {Helvetica 32 bold} \
	-fill white -text "No moves available.  Click here to pass." -tag NoMovesText

    $c raise base

}    

#proc MovePiece { x1 y1 x2 y2 c} {
#    $c lower piece$x1$y1
#    $c itemconfig piece$x2$y2 -fill [$c itemcget piece$x1$y1 -fill]
#    $c raise piece$x2$y2
#}

#proc MouseOverExpand { dot c } {
#    global dotExpandAmount
#    set dotExpandAmount .1
#    $c itemconfig $dot -fill red
#    $c itemconfig $dot -expand dotExpandAmount
#    puts "mouseoverexpand"
#}

#proc MouseOutContract { dot c } {
#    $c itemconfig $dot -fill blue
#}


#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}


#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
    global boardCols boardRows boardSize

    $c lower pieces
    $c lower moves
    $c raise base

    set pieceString [string range [C_GenericUnhash $position $boardSize] 0 [expr $boardSize-1]]
    set pieceNumber 0

#     puts "DrawPosition"
#     puts $position
#     puts $pieceString

    for {set i 0} {$i < $boardRows} {set i [expr $i + 1]} {
	for {set j 0} {$j < $boardCols} {set j [expr $j + 1]} {

	    if {[string compare [string index $pieceString $pieceNumber] "W"] == 0} {
		#puts "white piece at $j, $i"
		$c itemconfig piece$j$i -fill white
		$c raise piece$j$i
	    } elseif {[string compare [string index $pieceString $pieceNumber] "B"] == 0} {
		#puts "black piece at $j, $i"
		$c itemconfig piece$j$i -fill black	
		$c raise piece$j$i
	    } else {}

	    set pieceNumber [expr $pieceNumber + 1]
	}
    }

#     for {set i 0} {$i < $boardRows} {set i [expr $i + 1]} {
# 	for {set j 0} {$j < $boardCols} {set j [expr $j + 1]} {

# 	    if {[string compare [string index $pieceString $pieceNumber] "W"] == 0} {
# 		puts "white piece at $i, $j"
# 		$c itemconfig piece$i$j -fill white
# 		$c raise piece$i$j
# 	    } elseif {[string compare [string index $pieceString $pieceNumber] "B"] == 0} {
# 		puts "black piece at $i, $j"
# 		$c itemconfig piece$i$j -fill black	
# 		$c raise piece$i$j
# 	    } else {}

# 	    set pieceNumber [expr $pieceNumber + 1]
# 	}
#     }

}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this function is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    global boardRows boardCols

#     puts "initialization started..."
#     puts $boardRows
#     puts $boardCols

    GS_Deinitialize $c
    GS_Initialize $c

#     puts "done with initialization"
#     puts $boardRows
#     puts $boardCols

    GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
    return ""    
}


#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {

    GS_DrawPosition $c $newPosition
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

    $c lower moves

    foreach item $moveList {
	set move [lindex $item 0]
	set value [lindex $item 1]
	set color cyan
	
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red
	    }
	}
	
	if { $move != -1 } {
	    #we get -1 when there are no moves available

	    set movetag movedot[GetXYFromMove $item]
	    
	    $c raise $movetag
	    $c itemconfig $movetag -fill $color
	    
	    $c bind $movetag <ButtonRelease-1> "ReturnFromHumanMove $move"
	    $c bind $movetag <Enter> "$c itemconfig movedot[GetXYFromMove $item] -fill black"
	    $c bind $movetag <Leave> "$c itemconfig movedot[GetXYFromMove $item] -fill $color"

	} else {
	    
	    $c raise NoMovesText

	    $c bind NoMovesText <ButtonRelease-1> "ReturnFromHumanMove $move"
	    $c bind NoMovesText <Enter> "$c itemconfig NoMovesText -fill black"
	    $c bind NoMovesText <Leave> "$c itemconfig NoMovesText -fill white"

	}
    }
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in
    
    $c lower moves
    $c lower NoMovesText

}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {

}


#############################################################################
# GS_GameOver is called the moment the game is finished (won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {

	### TODO if needed
	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}


## returns the x and y values
proc GetXYFromMove {theMove} {

    set pos [lindex $theMove 0]
    return [Column $pos][Row $pos]

}

proc Row { index } {
    global boardCols

    return [expr $index / $boardCols]

}

proc Column { index } {
    global boardCols

    return [expr $index % $boardCols]

}

proc Index { row col } {
    global boardCols

    return [expr [expr $row * $boardCols] + $col]

}
