#pragma once
// Contiene las estructuras principales del juego

enum class CellType {
    EMPTY = 0,  
    WALL,       // Pared fija (╔ ═ ║ ╬ etc.), no se puede atravesar
    DIAMOND,    // Diamante/gema : el jugador lo recoge y suma puntos
    ROCK,       // Roca : cae por gravedad y se puede empujar
    PLAYER,     // Jugador 
    EXIT,       // Salida del nivel 

    ENEMY,      // Enemigo genérico 
    ENEMY_H,    // Enemigo que patrulla en horizontal 
    ENEMY_V,    // Enemigo que patrulla en vertical 
    BUTTON,     // Botón rojo (r u o en el .txt)
    BUTTON_G,   // Botón verde (g)
    BUTTON_B,   // Botón azul (b)

    DOOR,       // Puerta roja (R o H en el .txt); la abre el botón rojo
    DOOR_G,     // Puerta verde (G); la abre el botón verde
    DOOR_B,     // Puerta azul (B); la abre el botón azul
    BUSH,       // Arbusto destructible 
    WHIP_H,     // Látigo horizontal (─), efecto visual al presionar E
    WHIP_V      // Látigo vertical (│), efecto visual al presionar E
};

enum class GameState {
    MENU = 0,
    PLAYING,
    PAUSE,
    VICTORY,
    GAMEOVER
};

struct Position {
    int x;  // columna
    int y;  // fila
};

struct GameStats {
    int lives;
    int score;
    int timeRemaining;
    int currentLevel;
};