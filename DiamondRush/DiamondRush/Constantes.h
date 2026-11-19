#pragma once

#include <string>

using std::string;

// Dimensiones para el juego
const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;
const int MARGEN_IZQ = 15;   // Espacios en blanco a la izquierda de todo lo que se dibuja
const int MARGEN_SUP = 2;   // Líneas en blanco arriba de las pantallas (título, pausa, victoria, game over)

// Requisitos mínimos de tamaño de terminal para evitar errores
const int MIN_TERMINAL_WIDTH = 80;
const int MIN_TERMINAL_HEIGHT = 24;

// Códigos ANSI para colores
const string ANSI_RESET = "\033[0m";
const string ANSI_WALL = "\033[48;5;137m\033[37m";
const string ANSI_DIAMOND = "\033[36m";      // Cian para diamante
const string ANSI_ROCK = "\033[33m";         // Amarillo/Marrón para roca
const string ANSI_PLAYER = "\033[32m";       // Verde para jugador
const string ANSI_EMPTY = "\033[0m";

// Símbolos del juego
const string CHAR_PLAYER = "☺";
const string CHAR_EXIT = "∏";
const string CHAR_DIAMOND = "♦";
const string CHAR_ROCK = "●";
const string CHAR_EMPTY = " ";

// Caracteres para las paredes
const string WALL_H = "═";
const string WALL_V = "║";
const string WALL_TL = "╔";
const string WALL_TR = "╗";
const string WALL_BL = "╚";
const string WALL_BR = "╝";