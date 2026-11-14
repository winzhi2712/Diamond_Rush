#pragma once
//Gestiona la interfaz de usuario (HUD)
//Muestra info de juego, como las vidas, tiempo, puntuacion y nivel actual
#include "Matrix.h"
#include "Tipos.h"
#include <string>

using std::string;
using std::to_string;

class HUD{
private:
    //Referencia al hudBuffer que está dentro de Render.h
    Matrix<string>& hudBuffer;

    //Funcion auxiliar para escribir texto
    void writeText(int row, int col, string text) {

        //restriccion 
        if (row < 0 || row >= hudBuffer.getRows()) {
            return;
        }
        if (col < 0 || col >= hudBuffer.getColumns()) {
            return;
        }

        for (int i = 0; i < text.length(); i++) {
            //revisar si se escribe dentro de los limites de la matriz
            if (col + i >= hudBuffer.getColumns()) {
                break;
            } 

            hudBuffer.setValue(row, col + i, string(1, text[i]));
        }
    }

public:
    //constructor
    HUD(Matrix<string>& buffer) : hudBuffer(buffer) {}

    //Dibuja la informacion del HUD
    void draw(const GameStats& stats) {
        //Limpia todo
        hudBuffer.setAll(" ");

        //Escribe nueva info para todo la HUD
        writeText(0, 0, "VIDAS: " + to_string(stats.lives));
        writeText(0, 20, "TIEMPO: " + to_string(stats.timeRemaining));
        writeText(1, 0, "PUNTOS: " + to_string(stats.score));
        writeText(1, 20, "NIVEL: " + to_string(stats.currentLevel));
    }
};

