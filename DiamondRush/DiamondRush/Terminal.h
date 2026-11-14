#pragma once
#include <iostream>
#include "Constantes.h"

#ifdef _WIN32 //en caso de compilar en windows

#include <conio.h>
#include <thread>
#include <windows.h>

#else //en caso de ser linux

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <cstdlib>

#endif

using namespace std;

class Terminal {
private:
#ifndef _WIN32 //si no es windows
    struct termios oldTerminal;
    bool terminalInitialized = false;
#endif

    static Terminal* activeTerminal;

    static void restoreAtExit() {
        if (activeTerminal != nullptr) {
            activeTerminal->restore();
        }
    }
#ifndef _WIN32
    static void handleSignal(int signal) {
        if (activeTerminal != nullptr) {
            activeTerminal->restore();
        }

        _Exit(128 + signal);
    }
#endif

public:
    //por el momento de desarrollo, todas las funciones
    //tendran un #ifdef _WIN32 para tener codigo que 
    //funciona en windows y linux
    //luego se puede borrar antes de entregar

    void init() {
        activeTerminal = this;
    #ifdef _WIN32
        ocultarCursor();
    #else
        if (tcgetattr(STDIN_FILENO, &oldTerminal) == -1) {
            return;
        }
        struct termios newTerminal = oldTerminal;
        newTerminal.c_lflag &= ~(ICANON | ECHO);
        
        newTerminal.c_cc[VMIN] = 0;
        newTerminal.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal) == -1) {
            return;
        }

        terminalInitialized = true;

        ocultarCursor();

        signal(SIGINT, handleSignal);
        signal(SIGTERM, handleSignal);
    #endif
        //restaura la terminal al salir
        atexit(restoreAtExit);
    }

    void restore() {
    #ifdef _WIN32 
        //codigo para restaurar cursor
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    #else 
        if (terminalInitialized) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminal);
            terminalInitialized = false;
        }    

        write(STDOUT_FILENO, "\033[?25h", 6);
    #endif
    }

    int inputs() {
    #ifdef _WIN32
        if (_kbhit()) {
            return _getch();
        }
        return 0;
    #else
        char tecla;

        if (read(STDIN_FILENO, &tecla, 1) > 0) {
            return tecla;
        }
        return 0;
    #endif
    }

    void gotoxy(int x, int y) {
    #ifdef _WIN32
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
        cout << "\033[" << y + 1 << ";" << x + 1 << "H";
    #endif
    }

    void ocultarCursor() {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    #else
        cout << "\033[?25l";
    #endif
    }

    void clearTerminal() {
    #ifdef _WIN32
        system("cls");
    #else
        cout << "\033[2J\033[H";
        cout.flush();
    #endif
    }

    //revisa tamaño de la terminal
    bool checkMinSize() {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO info;

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
        //saca columnas y filas
        int cols = info.srWindow.Right - info.srWindow.Left + 1;
        int rows = info.srWindow.Bottom - info.srWindow.Top + 1;

        if (rows >= MIN_TERMINAL_HEIGHT && cols >= MIN_TERMINAL_WIDTH) {
            return true;
        } else {
            return false;
        }
    #else
        struct winsize size;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
            return false;
        }
        if (size.ws_row >= MIN_TERMINAL_HEIGHT && size.ws_col >= MIN_TERMINAL_WIDTH) {
            return true;
        } else {
            return false;
        }
    #endif
    }
};

//inicializacion del puntero estatico utilizada dentro de la clase
inline Terminal* Terminal::activeTerminal = nullptr;