#include <iostream>
#include <string>
#include <chrono>
#include <conio.h>
#include <thread>
#include <windows.h>
#include "Matrix.h"
#include "Terminal.h"
#include "Pantallas.h"
#include "KeyMap.h"
#include "Game.h"
using namespace std;
using namespace std::chrono;

//inputs
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


//funciones para impresion
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


//main loop
int main() {   
    SetConsoleOutputCP(65001);
    string title1 = "████  ███  ███  █   █  ███  █   █ ████     ████  █   █  ████ █   █ ";
    string title2 = "█   █  █  █   █ ██ ██ █   █ ██  █ █   █    █   █ █   █ █     █   █ ";
    string title3 = "█   █  █  █████ █ █ █ █   █ █ █ █ █   █    ████  █   █  ███  █████ ";
    string title4 = "█   █  █  █   █ █   █ █   █ █  ██ █   █    █  █  █   █     █ █   █ ";
    string title5 = "████  ███ █   █ █   █  ███  █   █ ████     █   █  ███  ████  █   █ ";

    Terminal terminal;
    terminal.init();
    Game game;
    KeyMap mapa;

    bool menu = true;
    bool pause = false;
    ocultarCursor();

    while (menu) {
        terminal.clearTerminal();
        Pantallas::mainTitle();
        terminal.pollInputs();
        int opcion = terminal.obtenerSiguiente();
        if (opcion != 0) {
            Mov movMenu = mapa.getMov(opcion);
            if (movMenu == Mov::YES)
                menu = !menu;
            else if (movMenu == Mov::EXIT)
                return 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }


    Matrix<string> test(10, 10);
    test.setValue(0, 0, "*");
    bool loop = true;

    terminal.clearTerminal();
    terminal.gotoxy(0, 0);
    test.print();


    while (loop == true) {
        if (game.perder()) {
            loop = false;
            break; ///////BREAK TEMPORAL, ODIO LOS BREAKS
        }
        terminal.gotoxy(0, 11);
        cout << "TIEMPO: " << game.formatTime();
        try {
            terminal.pollInputs();
            int tecla = terminal.obtenerSiguiente();

            if (tecla != 0) {
                Mov movimiento = mapa.getMov(tecla);
                if (movimiento == Mov::UP) {
                    test.setValue(test.currentRow, test.currentCol, "");
                    test.setValue(test.currentRow - 1, test.currentCol, "*");
                    test.setCurrent(test.currentRow - 1, test.currentCol);
                }

                else if (movimiento == Mov::DOWN) {
                    test.setValue(test.currentRow, test.currentCol, "");
                    test.setValue(test.currentRow + 1, test.currentCol, "*");
                    test.setCurrent(test.currentRow + 1, test.currentCol);
                }


                else if (movimiento == Mov::LEFT) {
                    test.setValue(test.currentRow, test.currentCol, "");
                    test.setValue(test.currentRow, test.currentCol - 1, "*");
                    test.setCurrent(test.currentRow, test.currentCol - 1);
                }


                else if (movimiento == Mov::RIGHT) {
                    test.setValue(test.currentRow, test.currentCol, "");
                    test.setValue(test.currentRow, test.currentCol + 1, "*");
                    test.setCurrent(test.currentRow, test.currentCol + 1);
                }
                else if (movimiento == Mov::PAUSE) {
                    pause = !pause;
                    terminal.clearTerminal();
                    Pantallas::pause();
                    while (pause) {
                        terminal.pollInputs();
                        int teclaPause = terminal.obtenerSiguiente();

                        if (teclaPause != 0) {

                            Mov movPause = mapa.getMov(teclaPause);
                            if (movPause == Mov::YES) {
                                pause = !pause;
                                terminal.clearTerminal();
                                terminal.gotoxy(0, 0);
                                test.print();
                            }
                            else if (movPause == Mov::EXIT) {
                                pause = !pause;
                                loop = !loop;
                            }
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    }
                }

                else if (movimiento == Mov::EXIT) {
                    loop = false;
                }
                terminal.gotoxy(0, 0);
                test.print();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
}
