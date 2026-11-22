#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "Constantes.h"
#include "Tipos.h"

struct EnemigoInfo {
    int fila = 0;
    int col = 0;
    int dirFila = 0;
    int dirCol = 1;
    int ticks = 4;         // Velocidad fluida
    bool persigue = false;
    CellType tipoCelda = CellType::ENEMY_H;
};

struct GameMap {
    std::vector<std::vector<CellType>> grid;         // Capa fija (paredes, salida, botones, puertas, arbustos)
    std::vector<std::vector<CellType>> entities;     // Capa móvil (rocas, diamantes)
    std::vector<std::vector<std::string>> glyphs;    // Símbolo original de cada pared (.txt)
    int playerX = 0;   // columna
    int playerY = 0;   // fila
    std::vector<EnemigoInfo> enemigos;
};

class MapLoader {
public:
    inline static GameMap cargarNivel(const std::string& rutaArchivo) {
        GameMap mapaJuego;
        mapaJuego.grid = std::vector<std::vector<CellType>>(MAP_HEIGHT, std::vector<CellType>(MAP_WIDTH, CellType::EMPTY));
        mapaJuego.entities = std::vector<std::vector<CellType>>(MAP_HEIGHT, std::vector<CellType>(MAP_WIDTH, CellType::EMPTY));
        mapaJuego.glyphs = std::vector<std::vector<std::string>>(MAP_HEIGHT, std::vector<std::string>(MAP_WIDTH, WALL_H));

        std::ifstream archivo(rutaArchivo);
        if (!archivo.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo de mapa: " << rutaArchivo << std::endl;
            return mapaJuego;
        }

        std::string linea;
        int fila = 0;

        while (std::getline(archivo, linea) && fila < MAP_HEIGHT) {
            int col = 0;
            for (size_t i = 0; i < linea.size() && col < MAP_WIDTH; ) {
                unsigned char c = linea[i];
                std::string simbolo = "";

                // Cuántos bytes tiene el carácter UTF-8 actual
                size_t numBytes = 1;
                if (c >= 0xFC) numBytes = 6;
                else if (c >= 0xF8) numBytes = 5;
                else if (c >= 0xF0) numBytes = 4;
                else if (c >= 0xE0) numBytes = 3;
                else if (c >= 0xC0) numBytes = 2;

                if (i + numBytes <= linea.size()) {
                    simbolo = linea.substr(i, numBytes);
                }
                else {
                    simbolo = std::string(1, c);
                }

                // 1. Paredes (capa fija, guardando el símbolo original)
                if (simbolo == "╔" || simbolo == "╗" || simbolo == "╚" || simbolo == "╝" ||
                    simbolo == "═" || simbolo == "║" || simbolo == "╦" || simbolo == "╩" ||
                    simbolo == "╠" || simbolo == "╣" || simbolo == "╬" || simbolo == "▩" ||
                    simbolo == WALL_H || simbolo == WALL_V || simbolo == WALL_TL ||
                    simbolo == WALL_TR || simbolo == WALL_BL || simbolo == WALL_BR || simbolo == "#") {
                    mapaJuego.grid[fila][col] = CellType::WALL;
                    mapaJuego.glyphs[fila][col] = simbolo;
                }
                // 2. Jugador (solo se guarda su posición; lo coloca Fisica)
                else if (simbolo == "☺" || simbolo == "P" || simbolo == CHAR_PLAYER) {
                    mapaJuego.playerX = col;
                    mapaJuego.playerY = fila;
                }
                // 3. Salida
                else if (simbolo == "∏" || simbolo == "Π" || simbolo == CHAR_EXIT) {
                    mapaJuego.grid[fila][col] = CellType::EXIT;
                }
                // 4. Diamantes (capa móvil)
                else if (simbolo == "◆" || simbolo == CHAR_DIAMOND) {
                    mapaJuego.entities[fila][col] = CellType::DIAMOND;
                }
                // 5. Rocas (capa móvil, para que caigan y se empujen)
                else if (simbolo == "●" || simbolo == CHAR_ROCK) {
                    mapaJuego.entities[fila][col] = CellType::ROCK;
                }
                // 5b. Corazones (capa móvil): suben la vida al pisarlos
                else if (simbolo == "♥" || simbolo == "❤") {
                    mapaJuego.entities[fila][col] = CellType::HEART;
                }
                // 6. Arbustos destructibles (capa fija)
                else if (simbolo == "▒" || simbolo == "░" || simbolo == "❀") {
                    mapaJuego.grid[fila][col] = CellType::BUSH;
                }
                // 7. Botones (capa fija): r = rojo, g = verde, b = azul ('o' también es rojo)
                else if (simbolo == "r" || simbolo == "o") {
                    mapaJuego.grid[fila][col] = CellType::BUTTON;
                }
                else if (simbolo == "g") {
                    mapaJuego.grid[fila][col] = CellType::BUTTON_G;
                }
                else if (simbolo == "b") {
                    mapaJuego.grid[fila][col] = CellType::BUTTON_B;
                }
                // 8. Puertas (capa fija): R = rojo, G = verde, B = azul ('H' también es roja)
                else if (simbolo == "R" || simbolo == "H") {
                    mapaJuego.grid[fila][col] = CellType::DOOR;
                }
                else if (simbolo == "G") {
                    mapaJuego.grid[fila][col] = CellType::DOOR_G;
                }
                else if (simbolo == "B") {
                    mapaJuego.grid[fila][col] = CellType::DOOR_B;
                }
                // 9. Enemigo horizontal
                else if (simbolo == "ʃ") {
                    EnemigoInfo ene;
                    ene.fila = fila;
                    ene.col = col;
                    ene.dirFila = 0;
                    ene.dirCol = 1;
                    ene.ticks = 5;
                    ene.persigue = false;
                    ene.tipoCelda = CellType::ENEMY_H;
                    mapaJuego.enemigos.push_back(ene);
                }
                // 10. Enemigo vertical
                else if (simbolo == "௹") {
                    EnemigoInfo ene;
                    ene.fila = fila;
                    ene.col = col;
                    ene.dirFila = 1;
                    ene.dirCol = 0;
                    ene.ticks = 4;
                    ene.persigue = false;
                    ene.tipoCelda = CellType::ENEMY_V;
                    mapaJuego.enemigos.push_back(ene);
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