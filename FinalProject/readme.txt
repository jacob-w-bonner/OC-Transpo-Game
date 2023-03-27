Changes

game.h
-Added a vector to contain enemy objects
-Added a variable to keep track of how many items a player has picked up
-Added a variable that holds whether or not the player is dead
-Added a variable to keep track of a time at which a new enemy spawns

game_object.h
-Defined member function to return rotation point
-Added variable to hold either "patroling" or "moving" state
-Added a vector to store the player location
-Added a variable to store whether or not a game object is hostile or not
-Added vector that stores point an object can rotate around

collectible_game_object.h
-Header file for item that player can pick up

enemy_game_object.h
-Header file for hostile enemies that can harm the player

game.cpp
-Changed window title to "Assignment 2"
-Added loop to delete enemy sprites in destructor
-Initialized new variables as defined in the game.h file (see above)
-Set new variable for whether a player is invulnerable or not to false
-Set new variable for time a player is invulnerable to 0
-Set a random number generator seed
-Added 5 collectible items to game object array
-Added new texture for items
-Added functionality to spawn new enemy every 7 seconds
-Added loop that checks, updates, and renders the enemies seperately from other objects
-Enemies rotate in a patrol when a player is not close, follow player to try and hit them when player gets close
-When player collides with item, player "picks up" item
-Picking up 5 items will make player invulnerable for 10 seconds, player sprite changes for duration

game_object.cpp
-Setting rotation point variable to a point close to the initial position upon initialization
-Setting the initial patrolling state to false upon initialization

collectible_game_object.cpp
-Subclass of game_object that a player can pick up
-Setting hostile variable to false upon initialization

enemy_game_object.cpp
-Subclass of game_object that can harm a player
-Setting hostile variable to true on initialization


Assets
-Player and enemy sprites taken from https://zintoki.itch.io/space-breaker under CC license
-Explosion sprite taken from https://weisinx7.itch.io/fireball-explosion-sprites under CC license
-Collectible item sprite taken from https://alexs-assets.itch.io/16x16-rpg-item-pack under CC license

Developed on Windows