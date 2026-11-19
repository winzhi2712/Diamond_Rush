#pragma once
#include <iostream>
#include <string>
#include "Matrix.h"
#include "Constantes.h"
#include "Tipos.h"

using std::string;
using std::cout;
using std::endl;
using std::to_string;
using std::flush;

struct RenderCell {
    string character;
    string color;
};

class Render {
private:
    Matrix<CellType> mapBuffer;
    Matrix<CellType> entitiesBuffer;
    Matrix<string> hudBuffer;
    Matrix<RenderCell> finalBuffer;
    Matrix<RenderCell> previousFrame;
    Matrix<string> wallGlyphBuffer;
    bool jugadorHerido = false;   // true = la carita se pinta roja

    string getCharacter(CellType type) {
        switch (type) {
        case CellType::WALL:
            return WALL_H;
        case CellType::DIAMOND:
            return CHAR_DIAMOND;
        case CellType::ROCK:
            return CHAR_ROCK;
        case CellType::PLAYER:
            return CHAR_PLAYER;
        case CellType::EXIT:
            return CHAR_EXIT;
        case CellType::ENEMY:
        case CellType::ENEMY_H:
        case CellType::ENEMY_V:
            return "§";
        case CellType::BUTTON:
        case CellType::BUTTON_G:
        case CellType::BUTTON_B:
            return "o";
        case CellType::DOOR:
        case CellType::DOOR_G:
        case CellType::DOOR_B:
            return "H";
        case CellType::BUSH:
            return "₪";
        case CellType::WHIP_H:
            return "─";
        case CellType::WHIP_V:
            return "│";
        default:
            return CHAR_EMPTY;
        }
    }

    string getColor(CellType type) {
        switch (type) {
        case CellType::WALL:
            return ANSI_WALL;
        case CellType::DIAMOND:
            return "\033[95m";
        case CellType::ROCK:
            return "\033[37m";   // gris
        case CellType::PLAYER:
            return jugadorHerido ? "\033[91m" : "\033[93m";   // rojo brillante si está herido, amarillo normal
        case CellType::EXIT:
            return ANSI_DIAMOND;
        case CellType::ENEMY:
        case CellType::ENEMY_H:
            return "\033[32m";   // verde
        case CellType::ENEMY_V:
            return "\033[31m";   // rojo
        case CellType::BUTTON:
        case CellType::DOOR:
            return "\033[91m";   // rojo
        case CellType::BUTTON_G:
        case CellType::DOOR_G:
            return "\033[92m";   // verde
        case CellType::BUTTON_B:
        case CellType::DOOR_B:
            return "\033[94m";   // azul
        case CellType::BUSH:
            return "\033[92m";   // verde brillante
        case CellType::WHIP_H:
        case CellType::WHIP_V:
            return "\033[97m";   // blanco brillante
        default:
            return ANSI_EMPTY;
        }
    }

public:
    Render(int rows, int cols)
        : mapBuffer(rows, cols),
        entitiesBuffer(rows, cols),
        hudBuffer(2, cols),
        finalBuffer(rows + 2, cols),
        previousFrame(rows + 2, cols),
        wallGlyphBuffer(rows, cols) {

        mapBuffer.setAll(CellType::EMPTY);
        entitiesBuffer.setAll(CellType::EMPTY);
        hudBuffer.setAll(" ");
        finalBuffer.setAll({ CHAR_EMPTY, ANSI_RESET });
        previousFrame.setAll({ CHAR_EMPTY, ANSI_RESET });
        wallGlyphBuffer.setAll(WALL_H);
    }

    Matrix<CellType>& getMapBuffer() { return mapBuffer; }
    Matrix<CellType>& getEntitiesBuffer() { return entitiesBuffer; }
    Matrix<string>& getHUDBuffer() { return hudBuffer; }
    Matrix<string>& getWallGlyphBuffer() { return wallGlyphBuffer; }
    void setJugadorHerido(bool herido) { jugadorHerido = herido; }

    // Obliga a redibujar todo en el siguiente frame (por ejemplo, al volver de la pausa)
    void forceFullRedraw() {
        previousFrame.setAll({ "\x01", "" });   // valor imposible: todo se considera distinto
    }

    void combineBuffers() {
        for (int r = 0; r < mapBuffer.getRows(); r++) {
            for (int c = 0; c < mapBuffer.getColumns(); c++) {
                CellType entity = entitiesBuffer.getValue(r, c);
                CellType map = mapBuffer.getValue(r, c);

                CellType selected = (entity != CellType::EMPTY) ? entity : map;

                RenderCell cell;
                cell.character = getCharacter(selected);
                cell.color = getColor(selected);

             

                if (selected == CellType::WALL) {
                    cell.character = wallGlyphBuffer.getValue(r, c);
                }

                finalBuffer.setValue(r + 2, c, cell);
            }
        }

        for (int r = 0; r < hudBuffer.getRows(); r++) {
            for (int c = 0; c < hudBuffer.getColumns(); c++) {
                string charActual = hudBuffer.getValue(r, c);
                RenderCell cell;
                cell.character = charActual.empty() ? CHAR_EMPTY : charActual;
                cell.color = ANSI_RESET;
                if (charActual == "♥") {
                    cell.color = "\033[91m";                // corazón lleno: rojo
                }
                else if (charActual == "~") {
                    cell.character = "♥";                   // corazón perdido: mismo símbolo, en gris oscuro
                    cell.color = "\033[38;5;240m";
                }
                finalBuffer.setValue(r, c, cell);
            }
        }
    }

    void renderDifferential() {
        string output = "";
        for (int r = 0; r < finalBuffer.getRows(); r++) {
            for (int c = 0; c < finalBuffer.getColumns(); c++) {
                RenderCell current = finalBuffer.getValue(r, c);
                RenderCell previous = previousFrame.getValue(r, c);

                if (current.character != previous.character || current.color != previous.color) {
                    output += "\033[" + to_string(r + 1 + MARGEN_SUP) + ";" + to_string(c + 1 + MARGEN_IZQ) + "H";
                    output += current.color;
                    output += current.character;
                    output += ANSI_RESET;

                    previousFrame.setValue(r, c, current);
                }
            }
        }
        cout << output << flush;
    }
};