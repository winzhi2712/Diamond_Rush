#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "Constantes.h"

struct GameMap {
    std::vector<std::vector<CellType>> grid;
    int playerX = 0;
    int playerY = 0;
};

class MapLoader {
public:
    inline static GameMap cargarNivel(const std::string& rutaArchivo) {
        GameMap mapaJuego;
        mapaJuego.grid = std::vector<std::vector<CellType>>(MAP_HEIGHT, std::vector<CellType>(MAP_WIDTH, CellType::EMPTY));

        std::ifstream archivo(rutaArchivo);
        if (!archivo.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo de mapa: " << rutaArchivo << std::endl;
            return mapaJuego;
        }

        std::string linea;
        int fila = 0;

        while (std::getline(archivo, linea) && fila < MAP_HEIGHT) {
            int col = 0;
            // Recorremos la línea manejando correctamente los caracteres UTF-8 (que pueden ocupar varios bytes)
            for (size_t i = 0; i < linea.size() && col < MAP_WIDTH; ) {
                unsigned char c = linea[i];
                std::string simbolo = "";

                // Determinamos cuántos bytes conforman el carácter actual en UTF-8
                size_t numBytes = 1;
                if (c >= 0xFC) numBytes = 6;
                else if (c >= 0xF8) numBytes = 5;
                else if (c >= 0xF0) numBytes = 4;
                else if (c >= 0xE0) numBytes = 3;
                else if (c >= 0xC0) numBytes = 2;

                // Extraemos el símbolo completo (sea de 1 o varios bytes)
                if (i + numBytes <= linea.size()) {
                    simbolo = linea.substr(i, numBytes);
                } else {
                    simbolo = std::string(1, c);
                }

                // 1. Mapeo de paredes (comparamos con la cadena UTF-8 completa)
                if (simbolo == "╔" || simbolo == "╗" || simbolo == "╚" || simbolo == "╝" || 
                    simbolo == "═" || simbolo == "║" || simbolo == "╦" || simbolo == "╩" || 
                    simbolo == "╠" || simbolo == "╣" || simbolo == "╬" || simbolo == "▩" || simbolo == string(1, CHAR_WALL)) {
                    mapaJuego.grid[fila][col] = CellType::WALL;
                }
                // 2. Mapeo del jugador
                else if (simbolo == "☺" || simbolo == "P" || simbolo == string(1, CHAR_PLAYER)) {
                    mapaJuego.grid[fila][col] = CellType::PLAYER;
                    mapaJuego.playerX = col;
                    mapaJuego.playerY = fila;
                }
                // 3. Mapeo de la salida
                else if (simbolo == "∏" || simbolo == string(1, CHAR_EXIT)) {
                    mapaJuego.grid[fila][col] = CellType::EXIT;
                }
                // 4. Mapeo de diamantes
                else if (simbolo == "◆" || simbolo == string(1, CHAR_DIAMOND)) {
                    mapaJuego.grid[fila][col] = CellType::DIAMOND;
                }
                // 5. Mapeo de rocas
                else if (simbolo == "●" || simbolo == string(1, CHAR_ROCK)) {
                    mapaJuego.grid[fila][col] = CellType::ROCK;
                }
                // 6. Espacios vacíos
                else {
                    mapaJuego.grid[fila][col] = CellType::EMPTY;
                }

                i += numBytes;
                col++;
            }
            fila++;
        }

        archivo.close();
        return mapaJuego;
    }
};