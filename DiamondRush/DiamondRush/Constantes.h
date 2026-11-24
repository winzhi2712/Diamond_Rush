#pragma once

#include <string>

using std::string;

// Dimensiones para el juego
const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;
const int MARGEN_IZQ = 15;   // Espacios en blanco a la izquierda de todo lo que se dibuja
const int MARGEN_SUP = 2;   // Líneas en blanco arriba de las pantallas (título, pausa, victoria, game over)

// Requisitos mínimos de tamaño de terminal para evitar errores
const int MIN_TERMINAL_WIDTH = 105;
const int MIN_TERMINAL_HEIGHT = 30;

// Menú principal: todo se centra en esta columna (contada desde 1)
const int MENU_CENTRO = 50;
// Posición del top 5 en el menú principal
const int MENU_TOP_FILA = 17;
const int MENU_TOP_COL = MENU_CENTRO - 10;   // la lista mide 21 columnas

// Códigos ANSI para colores
const string ANSI_RESET = "\033[0m";
const string ANSI_WALL = "\033[48;5;137m\033[37m";
const string ANSI_DIAMOND = "\033[36m";      // Cian para diamante
const string ANSI_ROCK = "\033[33m";         // Amarillo/Marrón para roca
const string ANSI_PLAYER = "\033[32m";       // Verde para jugador
const string ANSI_EMPTY = "\033[0m";
const string ANSI_CAFE = "\033[38;5;137m";   // Marcos de piedra
const string ANSI_MORADO = "\033[95m";       // Amatista
const string ANSI_VERDE = "\033[92m";        // Musgo
const string ANSI_ORO = "\033[93m";          // Destacados
const string ANSI_LIANA = "\033[38;5;70m";   // Verde de liana para el título

// Símbolos del juego
const string CHAR_PLAYER = "☺";
const string CHAR_DOOR_CLOSED = "▓";
const string CHAR_DOOR_OPEN = "░";
const string CHAR_EXIT = "∏";
const string CHAR_DIAMOND = "♦";
const string CHAR_ROCK = "●";
const string CHAR_EMPTY = " ";
const string CHAR_HEART = "♥";

// Caracteres para las paredes
const string WALL_H = "═";
const string WALL_V = "║";
const string WALL_TL = "╔";
const string WALL_TR = "╗";
const string WALL_BL = "╚";
const string WALL_BR = "╝";
// Posición del recuadro de mejores puntajes dentro del menú principal (fila y columna, contadas desde 1)
