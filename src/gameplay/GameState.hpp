#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

enum class GameState {
    PLAYING, // Active exploration and fuse hunting
    HIDING,  // Player safe inside closet
    CAUGHT,  // Entity made contact -> Game Over
    ESCAPED  // All 3 fuses collected and player reached exit
};

#endif // GAME_STATE_HPP
