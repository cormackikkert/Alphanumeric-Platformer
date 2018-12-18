# Alphanumeric Platformer
A Platformer designed to run on the ed1 board.
The player is a jello-like blob which has to make it to the end of a level which makes use of wall jump mechanics, moving platforms and hazards.

Cormac Kikkert programmed the platformer and rendering logic, while Timothy Horscroft programmed the music logic.

This board uses a 16x2 lcd alphanumeric display. To increase resolution custom characters were created each frame, 
which allowed each character to make use of the 5x8 pixels it controls, increasing the resolution to effectively 80x16.

As the lcd display can only render 8 custom characters at once the game uses a spotlight, only showing pixels around the player

As the game is running on a single core processor, the code computes the game logic and renders the display, then takes time to 
play the background music, keeping track of the time to ensure the game runs at a consitent frame rate.
