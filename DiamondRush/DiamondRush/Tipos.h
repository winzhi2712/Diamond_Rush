#pragma once

// Contiene las estructuras principales del juego
enum class CellType {
    EMPTY,
    WALL,
    DIAMOND,
    ROCK,
    PLAYER,
    EXIT,
    DOOR,       // Puerta roja
    BUTTON,     // Botón rojo
    ENEMY,
    ENEMY_H,    // Enemigo horizontal (movimiento de lado a lado)
    ENEMY_V,    // Enemigo vertical (movimiento de arriba a abajo)
    BUSH,
    WHIP_H,     // Látigo dibujado en horizontal
    WHIP_V,     // Látigo dibujado en vertical
    DOOR_G,     // Puerta verde
    DOOR_B,     // Puerta azul
    BUTTON_G,   // Botón verde
    BUTTON_B    // Botón azul
};

// Estados globales del juego
enum class GameState {
    MENU = 0,
    PLAYING = 1,
    PAUSE = 2,
    VICTORY = 3,
    GAMEOVER = 4
};

// Estructura basica para las coordenadas
struct Position {
    int x = 0;  // columna
    int y = 0;  // fila
};

// Estructura para datos de HUD
struct GameStats {
    int lives = 0;
    int score = 0;
    int timeRemaining = 0;
    int currentLevel = 1;
};