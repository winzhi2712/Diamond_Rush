#pragma once
//Se encarga del renderizado del juego y la actualizacion de celdas
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
    char character;
    string color;
};

class Render{
private:
    //Buffer individuales para los elementos del mapa
    Matrix<CellType> mapBuffer;
    Matrix<CellType> entitiesBuffer;

    //Buffer de HUD, tiene texto en lugar de tipo de celda
    Matrix<string> hudBuffer;

    //Buffer final para renderizar en conjunto
    Matrix<RenderCell> finalBuffer;

    //Buffer de frame anterior para comparar
    Matrix<RenderCell> previousFrame;

    //Funcion auxiliar que saca el tipo de char
    char getCharacter(CellType type) {
        switch (type) {
            case CellType::WALL:
                return CHAR_WALL;
            case CellType::DIAMOND:
                return CHAR_DIAMOND;
            case CellType::ROCK:
                return CHAR_ROCK;
            case CellType::PLAYER:
                return CHAR_PLAYER;
            default:
                return CHAR_EMPTY;
        }
    }

    //Funcion auxiliar para sacar el color
    string getColor(CellType type) {
        switch (type) {
            case CellType::WALL:
                return ANSI_WALL;
            case CellType::DIAMOND:
                return ANSI_DIAMOND;
            case CellType::ROCK:
                return ANSI_ROCK;
            case CellType::PLAYER:
                return ANSI_PLAYER;
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
        previousFrame(rows + 2, cols) {

            mapBuffer.setAll(CellType::EMPTY);
            entitiesBuffer.setAll(CellType::EMPTY);
            hudBuffer.setAll(" ");
            finalBuffer.setAll({' ', ANSI_RESET});
            previousFrame.setAll({' ', ANSI_RESET});
        }

    //Getters
    Matrix<CellType>& getMapBuffer() {
        return mapBuffer;
    }

    Matrix<CellType>& getEntitiesBuffer() {
        return entitiesBuffer;
    }

    Matrix<string>& getHUDBuffer() {
        return hudBuffer;
    }

    //Funciones de renderizado
    void combineBuffers() {
        //proceso para combinar buffer de mapa y entidades
        for (int r = 0; r < mapBuffer.getRows(); r++) {
            for (int c = 0; c < mapBuffer.getColumns(); c++) {
                //Sacar los valores que hay en esa celda
                CellType entity = entitiesBuffer.getValue(r, c);
                CellType map = mapBuffer.getValue(r, c);

                //Celda final que se va elegir
                CellType selected;

                //Siempre que exista algo por encima de una celda vacia se elige a esa cosa
                if (entity != CellType::EMPTY) {
                    selected = entity; //seleccion de cosa
                } else {
                    selected = map; //mapa entonces
                }

                //Convertir a estructura de rendercell
                RenderCell cell;
                cell.character = getCharacter(selected);
                cell.color = getColor(selected);

                //llenar el buffer final y +2 ya que el HUD utiliza las primeras dos filas
                finalBuffer.setValue(r+2, c, cell);
            }
        }

        //proceso para agregar hud al buffer final
        for (int r = 0; r < hudBuffer.getRows(); r++) {
            for (int c = 0; c < hudBuffer.getColumns(); c++) {
                string charActual = hudBuffer.getValue(r, c);
                
                //celda final
                RenderCell cell;
                if (charActual.empty()) {
                    cell.character = ' ';
                } else {
                    cell.character = charActual[0];
                }
                cell.color = ANSI_RESET;

                //llenar nuevamente para HUD
                finalBuffer.setValue(r, c, cell);
            }
        }
    }

    //funcion donde se imprime solo lo cambiado
    void renderDifferential() {
        string output = "";
        for (int r = 0; r < finalBuffer.getRows(); r++) {
            for (int c = 0; c < finalBuffer.getColumns(); c++) {
                //Sacar celda actual y anterior
                RenderCell current = finalBuffer.getValue(r, c);
                RenderCell previous = previousFrame.getValue(r, c);

                //revisa si cambia algo
                if (current.character != previous.character || current.color != previous.color) {
                    
                    //mueve el cursor a la ubicacion
                    output += "\033[" + to_string(r + 1) + ";" + to_string(c + 1) + "H"; 

                    //Se aplica el color nuevo
                    output += current.color;

                    //Imprime el caracter
                    output += current.character;

                    //Resetea el color de la terminal
                    output += ANSI_RESET;

                    //Se hace un update del frame anterior
                    previousFrame.setValue(r, c, current);
                }
            }
        }

        cout << output << flush;
    }
};

