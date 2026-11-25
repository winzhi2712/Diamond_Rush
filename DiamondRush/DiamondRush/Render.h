#pragma once
#include <iostream>
#include <string>
#include "Matrix.h"
#include "Constantes.h"
#include "Tipos.h"
#include "Fisica.h"

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

    bool puertasAbiertas[3] = { false, false, false };   // 0 = rojo, 1 = verde, 2 = azul

    bool panelPendiente = true;   // true = hay que dibujar el panel de controles en el siguiente frame

    // Escribe una línea del panel a la derecha del mapa (fila 1 = primera fila de la pantalla de juego)
    string lineaPanel(int fila, const string& texto) {
        return "\033[" + to_string(fila + MARGEN_SUP) + ";" + to_string(MARGEN_IZQ + MAP_WIDTH + 4) + "H" + texto;
    }

    // Color de cada "pixel" del explorador del panel
    static string colorPixelPanel(char c) {
        switch (c) {
        case 'H': return "\033[38;5;94m";    // sombrero café
        case 'h': return "\033[38;5;130m";   // cinta del sombrero
        case 'S': return "\033[38;5;180m";   // piel
        case 'E': return "\033[38;5;232m";   // ojos
        case 'Y': return "\033[38;5;178m";   // camisa amarilla
        case 'B': return "\033[38;5;94m";    // cinturón
        case 'K': return "\033[38;5;240m";   // botas
        case 'R': return "\033[38;5;245m";   // piedra (gris, igual que las rocas del juego)
        case 'r': return "\033[38;5;240m";   // piedra en sombra
        }
        return "";
    }

    // Explorador de bloques sosteniendo una piedra enorme sobre su cabeza.
    // "fila" es la primera fila del panel donde se dibuja. Cada pixel mide 2 columnas; 9 de ancho x 14 de alto.
    string exploradorCargando(int fila) {
        const string sprite[14] = {
            "..rRRRr..",   // piedra (más grande que el explorador)
            ".rRRRRRr.",
            "rRRRRRRRr",
            "rRRRRRRRr",
            ".rRRRRRr.",
            ".SrrrrrS.",   // manos sujetando la piedra
            ".YHHHHHY.",   // copa del sombrero (brazos arriba a los lados)
            ".YhhhhhY.",   // cinta
            ".YHHHHHY.",   // ala
            ".YSESESY.",   // ojos
            ".YSSSSSY.",   // cara
            ".YYBBBYY.",   // cinturón
            "..YYYYY..",   // torso
            "..KK.KK.."    // botas
        };
        const int sangria = 6;   // espacios a la izquierda para centrarlo en el panel (31 columnas de ancho)
        string out = "";
        for (int r = 0; r < 14; r++) {
            string linea = string(sangria, ' ');
            for (int i = 0; i < 9; i++) {
                char c = sprite[r][i];
                if (c == '.') linea += "  ";
                else linea += colorPixelPanel(c) + "██" + ANSI_RESET;
            }
            out += lineaPanel(fila + r, linea);
        }
        return out;
    }

    // Panel lateral con los controles y el valor de las gemas
    string panelTexto() {
        const string titulo = "\033[1m\033[96m";
        const string tecla = "\033[93m";
        string out = "";
        int f = 1;

        out += lineaPanel(f++, titulo + "CONTROLES" + ANSI_RESET);
        out += lineaPanel(f++, tecla + "WASD / Flechas" + ANSI_RESET + "  Mover");
        out += lineaPanel(f++, tecla + "E" + ANSI_RESET + "               Latigo");
        out += lineaPanel(f++, tecla + "P" + ANSI_RESET + "               Pausa");
        out += lineaPanel(f++, tecla + "R" + ANSI_RESET + "               Reiniciar nivel");
        out += lineaPanel(f++, tecla + "ESC" + ANSI_RESET + "             Salir");
        f++;

        out += lineaPanel(f++, titulo + "PUNTOS" + ANSI_RESET);
        out += lineaPanel(f++, "\033[95m" + CHAR_DIAMOND + ANSI_RESET + " = +" + to_string(puntosGema) + " puntos");
        out += lineaPanel(f++, "Eliminar enemigo = +" + to_string(puntosEnemigo) + " puntos");
        out += lineaPanel(f++, "Tiempo sobrante = +5 pts/seg");
        f++;

        out += exploradorCargando(f);
        return out;
    }

    // Número "al azar" pero siempre igual para los mismos datos: así las manchas de las paredes
    // no parpadean ni cambian de lugar entre un frame y otro
    static unsigned int ruido(int a, int b, int semilla) {
        unsigned int h = (unsigned int)a * 73856093u ^ (unsigned int)b * 19349663u ^ (unsigned int)semilla * 83492791u;
        h ^= h >> 13;
        h *= 0x5bd1e995u;
        h ^= h >> 15;
        return h;
    }

    // Color de una pared: café, con manchas de moho verde repartidas por la casilla (fila, col)
    string colorPared(int fila, int col) {
        // Las manchas se forman por bloques de 2 filas x 4 columnas (se ven casi cuadrados en la terminal)
        bool bloqueConMoho = ruido(fila / 2, col / 4, 18) % 100 < 31;   // ~25% de las paredes con moho
        unsigned int fino = ruido(fila, col, 2) % 100;

        if ((bloqueConMoho && fino < 75) || fino < 3) {   // mancha, más unas pocas motas sueltas
            if (ruido(fila, col, 3) % 100 < 85)
                return "\033[48;5;58m\033[38;5;22m";    // verde pantanoso (el más común)
            return "\033[48;5;64m\033[38;5;22m";        // verde claro (poco)
        }
        return ANSI_WALL;   // pared café normal
    }

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
            return puertasAbiertas[0] ? CHAR_DOOR_OPEN : CHAR_DOOR_CLOSED;
        case CellType::DOOR_G:
            return puertasAbiertas[1] ? CHAR_DOOR_OPEN : CHAR_DOOR_CLOSED;
        case CellType::DOOR_B:
            return puertasAbiertas[2] ? CHAR_DOOR_OPEN : CHAR_DOOR_CLOSED;
            return "H";
        case CellType::BUSH:
            return "₪";
        case CellType::WHIP_H:
            return "─";
        case CellType::WHIP_V:
            return "│";
        case CellType::HEART:
            return CHAR_HEART;
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
            return "\033[38;5;245m";   // gris
        case CellType::PLAYER:
            return jugadorHerido ? "\033[91m" : "\033[38;5;223m";   // rojo brillante si está herido, beige normal
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
            return "\033[38;5;130m";   // café
        case CellType::HEART:
            return "\033[91m";   // rojo brillante
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
    void setPuertaAbierta(int color, bool abierta) {
        if (color >= 0 && color < 3) puertasAbiertas[color] = abierta;
    }

    // Obliga a redibujar todo en el siguiente frame (por ejemplo, al volver de la pausa)
    void forceFullRedraw() {
        previousFrame.setAll({ "\x01", "" });   // valor imposible: todo se considera distinto
        panelPendiente = true;                  // y se vuelve a dibujar el panel
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
                    cell.color = colorPared(r, c);   // café con manchas de moho
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
        if (panelPendiente) {
            output += panelTexto();
            panelPendiente = false;
        }
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