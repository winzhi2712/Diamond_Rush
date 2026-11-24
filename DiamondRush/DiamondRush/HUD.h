#pragma once
//Gestiona la interfaz de usuario (HUD)
//Muestra info de juego, como las vidas, tiempo, puntuacion y nivel actual
#include "Matrix.h"
#include "Tipos.h"
#include <string>

using std::string;
using std::to_string;

class HUD {
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

    //Escribe un símbolo completo (por ejemplo un corazón, que ocupa varios bytes) en una sola celda
    void writeGlyph(int row, int col, const string& glyph) {
        if (row < 0 || row >= hudBuffer.getRows()) {
            return;
        }
        if (col < 0 || col >= hudBuffer.getColumns()) {
            return;
        }
        hudBuffer.setValue(row, col, glyph);
    }

    //Convierte segundos a formato reloj m:ss (por ejemplo 272 -> "4:32", 65 -> "1:05")
    string formatearTiempo(int segundosTotales) {
        if (segundosTotales < 0) segundosTotales = 0;
        int minutos = segundosTotales / 60;
        int segundos = segundosTotales % 60;
        string relleno = (segundos < 10) ? "0" : "";   // el "0" que hace que 5 segundos se vea como :05
        return to_string(minutos) + ":" + relleno + to_string(segundos);
    }

public:
    //constructor
    HUD(Matrix<string>& buffer) : hudBuffer(buffer) {}

    //Dibuja la informacion del HUD
    void draw(const GameStats& stats) {
        //Limpia todo
        hudBuffer.setAll(" ");

        //Barra de vida: 10 corazones, cada uno vale 10 de vida ("~" marca un corazón perdido)
        writeText(0, 0, "VIDAS: ");
        int corazonesLlenos = (stats.lives + 9) / 10;
        if (corazonesLlenos < 0) corazonesLlenos = 0;
        if (corazonesLlenos > 10) corazonesLlenos = 10;
        for (int i = 0; i < 10; i++) {
            string corazon = (i < corazonesLlenos) ? "♥" : "~";
            writeGlyph(0, 7 + i, corazon);
        }

        writeText(0, 20, "TIEMPO: " + formatearTiempo(stats.timeRemaining));
        writeText(1, 0, "PUNTOS: " + to_string(stats.score));
        writeText(1, 20, "NIVEL: " + to_string(stats.currentLevel));
    }
};