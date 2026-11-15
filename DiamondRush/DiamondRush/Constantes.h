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
const string ANSI_ROCK = "\033[33m";    // Amarillo/Marrón
const string ANSI_PLAYER = "\033[32m";    // Verde 
const string ANSI_EMPTY = "\033[0m";

enum class CellType {
    EMPTY,
    WALL,
    PLAYER,
    EXIT,
    DIAMOND,
    ROCK
};


// Aqui Axel, voy hacer estos cambios de prueba no sé todavía si funcionarán, pero quiero probarlos.    

// Definimos los símbolos exactos que usaste en tus niveles:
const char CHAR_PLAYER = '☺';      // La carita feliz que elegiste
const char CHAR_EXIT = '∏';        // El símbolo matemático de salida
const char CHAR_DIAMOND = '◆';     // El rombo para futuras gemas
const char CHAR_ROCK = '●';        // La roca segura
const char CHAR_EMPTY = ' ';       // Espacio libre