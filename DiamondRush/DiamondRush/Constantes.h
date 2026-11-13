#pragma once
//Aqui se definen las constantes globales del juego
//Incluye: dimensiones de la pantalla y del mapa, los colores, los limites del sistema
#include <string>

using std::string;

//dimensiones para el juego
const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;

//Requisitos minimo de tamaño de terminal para evitar errores
const int MIN_TERMINAL_WIDTH = 80;
const int MIN_TERMINAL_HEIGHT = 24;

//codigos ANSI para consola de raspberry
const string ANSI_RESET = "\033[0m";
const string ANSI_WALL = "\033[44m\033[37m"; //color azul para pared
const string ANSI_DIAMOND = "\033[36m";     // cian para dia
const string ANSI_BOULDER = "\033[33m";    // Amarillo/Marrón
const string ANSI_PLAYER = "\033[32m";    // Verde 
const string ANSI_EMPTY = "\033[0m";

// Caracteres visuales en modo texto para cada celda
const char CHAR_WALL = '#';
const char CHAR_DIAMOND = '$';
const char CHAR_BOULDER = 'O';
const char CHAR_PLAYER = '&';
const char CHAR_EMPTY = ' ';