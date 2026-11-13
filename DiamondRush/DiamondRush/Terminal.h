#pragma once
#include <conio.h>
#include <thread>
#include <windows.h>
using namespace std;

class Terminal {

public:
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
    void clearTerminal() {
        system("cls");
    }
};

