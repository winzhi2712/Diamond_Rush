#pragma once
//Gestiona las diferentes pantallas del juego
//Incluye el menu principal, pausa, victoria y derrota
#include "Constantes.h"
#include <string>
#include <iostream>

using namespace std;
class Pantallas {
private:
    inline static const string title1 = "████  ███  ███  █   █  ███  █   █ ████     ████  █   █  ████ █   █ ";
    inline static const string title2 = "█   █  █  █   █ ██ ██ █   █ ██  █ █   █    █   █ █   █ █     █   █ ";
    inline static const string title3 = "█   █  █  █████ █ █ █ █   █ █ █ █ █   █    ████  █   █  ███  █████ ";
    inline static const string title4 = "█   █  █  █   █ █   █ █   █ █  ██ █   █    █  █  █   █     █ █   █ ";
    inline static const string title5 = "████  ███ █   █ █   █  ███  █   █ ████     █   █  ███  ████  █   █ ";

    inline static const string gameOver1 = " ███   ███  █   █ █████     ███  █   █ █████ ████";
    inline static const string gameOver2 = "█     █   █ ██ ██ █        █   █ █   █ █     █   █";
    inline static const string gameOver3 = "█  ██ █████ █ █ █ ████     █   █ █   █ ████  ████";
    inline static const string gameOver4 = "█   █ █   █ █   █ █        █   █  █ █  █     █  █ ";
    inline static const string gameOver5 = " ███  █   █ █   █ █████     ███    █   █████ █   █ ";

    inline static const string victory1 = "█   █ ███  ███  █████  ███  ████  █   █ ";
    inline static const string victory2 = "█   █  █  █       █   █   █ █   █  █ █  ";
    inline static const string victory3 = "█   █  █  █       █   █   █ ████    █";
    inline static const string victory4 = " █ █   █  █       █   █   █ █  █    █ ";
    inline static const string victory5 = "  █   ███  ███    █    ███  █   █   █ ";

    inline static const string pause1 = "████   ███  █   █  ████ █████ ";
    inline static const string pause2 = "█   █ █   █ █   █ █     █     ";
    inline static const string pause3 = "████  █████ █   █  ███  ████  ";
    inline static const string pause4 = "█     █   █ █   █     █ █     ";
    inline static const string pause5 = "█     █   █  ███  ████  █████ ";

    // Margen izquierdo de las pantallas
    static string pad() {
        return string(MARGEN_IZQ, ' ');
    }

public:
    static void mainTitle() {
        cout << "\033[2J\033[H";    //limpia la pantalla
        cout << string(MARGEN_SUP, '\n');
        cout << ANSI_DIAMOND;   //dice diamond pero es para utilizar el color 
        cout << pad() << title1 << endl;
        cout << pad() << title2 << endl;
        cout << pad() << title3 << endl;
        cout << pad() << title4 << endl;
        cout << pad() << title5 << endl;
        cout << pad() << "PRESS ENTER TO PLAY" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void gameOver() {
        cout << string(MARGEN_SUP, '\n');
        cout << "\033[31m"; // color rojo que no se habia definido en constantes
        cout << pad() << gameOver1 << endl;
        cout << pad() << gameOver2 << endl;
        cout << pad() << gameOver3 << endl;
        cout << pad() << gameOver4 << endl;
        cout << pad() << gameOver5 << endl;
        cout << pad() << "PRESS ENTER TO RETRY" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void victory() {
        cout << string(MARGEN_SUP, '\n');
        cout << ANSI_PLAYER; //color
        cout << pad() << victory1 << endl;
        cout << pad() << victory2 << endl;
        cout << pad() << victory3 << endl;
        cout << pad() << victory4 << endl;
        cout << pad() << victory5 << endl;
        cout << pad() << "PRESS ENTER TO RETRY" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void pause() {
        cout << string(MARGEN_SUP, '\n');
        cout << ANSI_ROCK; //nuevamente es un color nada mas (amarillo)
        cout << pad() << pause1 << endl;
        cout << pad() << pause2 << endl;
        cout << pad() << pause3 << endl;
        cout << pad() << pause4 << endl;
        cout << pad() << pause5 << endl;
        cout << pad() << "PRESS ENTER TO RESUME" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }
};