#include <iostream>
#include <string>
#include <chrono>
#include <conio.h>
#include <thread>
#include <windows.h>
#include "constantes.h"
#include "MapLoader.h"
#include "Matrix.h"
#include "Terminal.h"
#include "Pantallas.h"
#include "KeyMap.h"
#include "Game.h"

using namespace std;
using namespace std::chrono;

// inputs
int inputs() {
    if (_kbhit()) {
        int tecla = _getch();
        if (tecla == 224 || tecla == 0) {
            tecla = _getch() + 256;
        }
        return tecla;
    }
    return 0;
}

// funciones para impresion
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ocultarCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// main loop
int main() {    
    SetConsoleOutputCP(65001);

    string title1 = "████  ███  ███  █   █  ███  █   █ ████     ████  █   █  ████ █   █ ";
    string title2 = "█   █  █  █   █ ██ ██ █   █ ██  █ █   █     █   █ █   █ █     █   █ ";
    string title3 = "█   █  █  █████ █ █ █ █   █ █ █ █ █   █     ████  █   █  ███  █████ ";
    string title4 = "█   █  █  █   █ █   █ █   █ █  ██ █   █     █   █ █   █     █ █   █ ";
    string title5 = "████  ███ █   █ █   █  ███  █   █ ████      █   █  ███  ████  █   █ ";

    Terminal terminal;
    terminal.init();
    Game game;
    KeyMap mapa;

    bool menu = true;
    bool pause = false;
    ocultarCursor();

    // Bucle del menú (sin breaks)
    while (menu) {
        terminal.clearTerminal();
        Pantallas::mainTitle();
        terminal.pollInputs();
        int opcion = terminal.obtenerSiguiente();
        if (opcion != 0) {
            Mov movMenu = mapa.getMov(opcion);
            if (movMenu == Mov::YES) {
                menu = false;
            }
            else if (movMenu == Mov::EXIT) {
                return 0;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Cargamos el nivel real desde el archivo de texto
    terminal.clearTerminal();
    cout << "Cargando nivel 1..." << endl;
    
    GameMap nivelActual = MapLoader::cargarNivel("nivel1.txt");

    int playerX = nivelActual.playerX;
    int playerY = nivelActual.playerY;

    bool loop = true;

    terminal.clearTerminal();
    terminal.gotoxy(0, 0);

    // Dibujamos el mapa inicial usando puros ifs (cero switch)
    for (int fila = 0; fila < MAP_HEIGHT; ++fila) {
        for (int col = 0; col < MAP_WIDTH; ++col) {
            CellType celda = nivelActual.grid[fila][col];

            if (celda == CellType::WALL) {
                cout << ANSI_WALL << " " << ANSI_RESET;
            }
            else if (celda == CellType::PLAYER) {
                cout << ANSI_PLAYER << CHAR_PLAYER << ANSI_RESET;
            }
            else if (celda == CellType::EXIT) {
                cout << ANSI_DIAMOND << CHAR_EXIT << ANSI_RESET;
            }
            else if (celda == CellType::DIAMOND) {
                cout << ANSI_DIAMOND << CHAR_DIAMOND << ANSI_RESET;
            }
            else if (celda == CellType::ROCK) {
                cout << ANSI_ROCK << CHAR_ROCK << ANSI_RESET;
            }
            else {
                cout << CHAR_EMPTY;
            }
        }
        cout << endl;
    }

    // Bucle principal del juego (controlado por bandera y sin breaks)
    while (loop == true && !game.perder()) {
        
        terminal.gotoxy(0, MAP_HEIGHT + 1);
        cout << "TIEMPO: " << game.formatTime() << "   ";

        try {
            terminal.pollInputs();
            int tecla = terminal.obtenerSiguiente();

            if (tecla != 0) {
                Mov movimiento = mapa.getMov(tecla);
                int nuevoX = playerX;
                int nuevoY = playerY;

                if (movimiento == Mov::UP) {
                    nuevoY--;
                }
                else if (movimiento == Mov::DOWN) {
                    nuevoY++;
                }
                else if (movimiento == Mov::LEFT) {
                    nuevoX--;
                }
                else if (movimiento == Mov::RIGHT) {
                    nuevoX++;
                }
                else if (movimiento == Mov::PAUSE) {
                    pause = true;
                    terminal.clearTerminal();
                    Pantallas::pause();
                    
                    while (pause) {
                        terminal.pollInputs();
                        int teclaPause = terminal.obtenerSiguiente();

                        if (teclaPause != 0) {
                            Mov movPause = mapa.getMov(teclaPause);
                            if (movPause == Mov::YES) {
                                pause = false;
                                terminal.clearTerminal();
                                terminal.gotoxy(0, 0);
                                // Opcional: Redibujar mapa si lo requieres al salir de pausa
                            }
                            else if (movPause == Mov::EXIT) {
                                pause = false;
                                loop = false;
                            }
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    }
                }
                else if (movimiento == Mov::EXIT) {
                    loop = false;
                }

                // Lógica de movimiento y validación de colisiones con puros ifs
                if (nuevoX >= 0 && nuevoX < MAP_WIDTH && nuevoY >= 0 && nuevoY < MAP_HEIGHT) {
                    if (nivelActual.grid[nuevoY][nuevoX] != CellType::WALL) {
                        // Limpiamos la posición vieja en la matriz lógica y en pantalla
                        nivelActual.grid[playerY][playerX] = CellType::EMPTY;
                        terminal.gotoxy(playerX, playerY);
                        cout << CHAR_EMPTY;

                        // Actualizamos las coordenadas del jugador
                        playerX = nuevoX;
                        playerY = nuevoY;

                        // Colocamos al jugador en la nueva posición
                        nivelActual.grid[playerY][playerX] = CellType::PLAYER;
                        terminal.gotoxy(playerX, playerY);
                        cout << ANSI_PLAYER << CHAR_PLAYER << ANSI_RESET;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    return 0;
}