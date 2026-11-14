#pragma once
#include <conio.h>
#include <thread>
#include <windows.h>
using namespace std;

class Terminal {

    static Terminal* activeTerminal;

    static const int BUFFER_SIZE = 50;  //crear un buffer para las teclas con un arreglo
    int bufferTeclas[BUFFER_SIZE];
    int head = 0;
    int tail = 0;
    int size = 0;

    void pushBuffer(int tecla) {
        if (size < BUFFER_SIZE) { //hacer que el push sea circular
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
                if (c == '\033') {
                    char seq[2];
                    struct pollfd pfd;
                    pfd.fd = STDIN_FILEMO;
                    pfd.events = POLLIN;
                    bool esFlecha = false;

                    if (poll(&pfd, 1, 5) > 0) { //tiempo de espera de 5 milisegundos
                        if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0) {
                            if (seq[0] == '[') {
                                int teclaFlecha = 0;
                                if (seq[1] == 'A')
                                    teclaFlecha = 256 + 72; //ARRIBA
                                else if (seq[1] == 'B')
                                    teclaFelcha = 256 + 88; //ABAJO
                                else if (seq[1] == 'C')
                                    teclaFelcha = 256 + 77; //DERECHA
                                else if (seq[1] == 'D')
                                    teclaFelcha = 256 + 75; //IZQUIERDA
                            }
                            if (teclaFlecha != 0) {
                                pushBuffer(teclaFlecha);
                                esFlecha = true;
                            }
                        }
                    }
                }
                if (!esFlecha) {
                    pushBuffer(27);
                }
                else {
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

