#pragma once
#include <iostream>
#include <string>
#include <chrono>

#ifdef _WIN32 // En caso de compilar en Windows
#include <conio.h>
#include <thread>
#include <windows.h>
#else // En caso de ser Linux
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <cstdlib>
#include <poll.h>
#endif

#include "Constantes.h"

using namespace std;

class Terminal {
private:
#ifndef _WIN32 // Si no es Windows
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

    static const int BUFFER_SIZE = 50;  // Crear un buffer para las teclas con un arreglo
    int bufferTeclas[BUFFER_SIZE];
    int head = 0;
    int tail = 0;
    int size = 0;

    void pushBuffer(int tecla) {
        if (size < BUFFER_SIZE) { // Hacer que el push sea circular
            bufferTeclas[tail] = tecla;
            tail = (tail + 1) % BUFFER_SIZE;
            size++;
        }
    }

    int popBuffer() {
        if (size > 0) {
            int tecla = bufferTeclas[head];
            head = (head + 1) % BUFFER_SIZE;
            size--;
            return tecla;
        }
        return 0;
    }

public:
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
        // Restaura la terminal al salir
        atexit(restoreAtExit);
    }

    void restore() {
#ifdef _WIN32 
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

    void pollInputs() {
#ifdef _WIN32
        while (_kbhit()) {
            int tecla = _getch();
            if (tecla == 224 || tecla == 0) {
                tecla = _getch() + 256;
            }
            pushBuffer(tecla);
        }
#else
        char c;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            bool esFlecha = false;
            if (c == '\033') {
                char seq[2];
                struct pollfd pfd;
                pfd.fd = STDIN_FILENO;
                pfd.events = POLLIN;

                if (poll(&pfd, 1, 5) > 0) { // Tiempo de espera de 5 milisegundos
                    if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0) {
                        if (seq[0] == '[') {
                            int teclaFlecha = 0;
                            if (seq[1] == 'A')
                                teclaFlecha = 256 + 72; // ARRIBA
                            else if (seq[1] == 'B')
                                teclaFlecha = 256 + 80; // ABAJO
                            else if (seq[1] == 'C')
                                teclaFlecha = 256 + 77; // DERECHA
                            else if (seq[1] == 'D')
                                teclaFlecha = 256 + 75; // IZQUIERDA

                            if (teclaFlecha != 0) {
                                pushBuffer(teclaFlecha);
                                esFlecha = true;
                            }
                        }
                    }
                }
            }
            if (!esFlecha) {
                pushBuffer((unsigned char)c);
            }
        }
#endif
    }

    int obtenerSiguiente() {
        return popBuffer();
    }

    void limpiarBuffer() {
        head = tail = size = 0;
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

    bool checkMinSize() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
        int cols = info.srWindow.Right - info.srWindow.Left + 1;
        int rows = info.srWindow.Bottom - info.srWindow.Top + 1;
        return (rows >= MIN_TERMINAL_HEIGHT && cols >= MIN_TERMINAL_WIDTH);
#else
        struct winsize size;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
            return false;
        }
        return (size.ws_row >= MIN_TERMINAL_HEIGHT && size.ws_col >= MIN_TERMINAL_WIDTH);
#endif
    }
};

// Inicialización del puntero estático utilizada dentro de la clase
Terminal* Terminal::activeTerminal = nullptr;