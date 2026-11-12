#include <iostream>
#include <string>
#include <chrono>
#include <conio.h>
#include <thread>
#include <windows.h>
#include "Matrix.h"
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
int main()
{
    ocultarCursor();
    Matrix<string> test(4, 4);
    test.setValue(0, 0, "*");
    bool loop = true;

    system("cls");
    gotoxy(0, 0);
    test.print();


    while (loop == true) {
        int tecla = inputs();

        if (tecla != 0) {
            if (tecla == 'w') {
                test.setValue(test.currentRow, test.currentCol, "");
                test.setValue(test.currentRow - 1, test.currentCol, "*");
                test.setCurrent(test.currentRow - 1, test.currentCol);
            }

            else if (tecla == 's') {
                test.setValue(test.currentRow, test.currentCol, "");
                test.setValue(test.currentRow + 1, test.currentCol, "*");
                test.setCurrent(test.currentRow + 1, test.currentCol);
            }


            else if (tecla == 'a') {
                test.setValue(test.currentRow, test.currentCol, "");
                test.setValue(test.currentRow, test.currentCol - 1, "*");
                test.setCurrent(test.currentRow, test.currentCol - 1);
            }


            else if (tecla == 'd') {
                test.setValue(test.currentRow, test.currentCol, "");
                test.setValue(test.currentRow, test.currentCol + 1, "*");
                test.setCurrent(test.currentRow, test.currentCol + 1);
            }
            else if (tecla == 27) {
                loop = false;
            }
            gotoxy(0, 0);
            test.print();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void assets() {
    string title1 = "████  ███  ███  █   █  ███  █   █ ████     ████  █   █  ████ █   █ ";
    string title2 = "█   █  █  █   █ ██ ██ █   █ ██  █ █   █    █   █ █   █ █     █   █ ";
    string title3 = "█   █  █  █████ █ █ █ █   █ █ █ █ █   █    ████  █   █  ███  █████ ";
    string title4 = "█   █  █  █   █ █   █ █   █ █  ██ █   █    █  █  █   █     █ █   █ ";
    string title5 = "████  ███ █   █ █   █  ███  █   █ ████     █   █  ███  ████  █   █ ";


    string diamond1 = "\xE2\x99\xA6";

}
