#pragma once
//Contiene las estructuras pincipales del juego
//Como las celdas del mapa, los estados y las acciones

//Los tipos de elementos que pueden existir en la celda de las matrices
enum class CellType {
    EMPTY = 0,  //Espacio vacio
    WALL = 1,   //Muro estatico
    DIAMOND = 2,   //Gema
    ROCK = 3,   //Roca
    PLAYER = 4  //Jugador
};

//Estados globales del juego
enum class GameState {
    MENU = 0,
    PLAYING = 1,
    PAUSE = 2,
    VICTORY = 3,
    GAMEOVER = 4
};

//estructura basica para las coordenadas
struct Position {
    int x;  //columna
    int y;  //fila
};

//estrucutra para datos de HUD
struct GameStats {
    int lives;
    int score;
    int timeRemaining;
    int currentLevel;
};