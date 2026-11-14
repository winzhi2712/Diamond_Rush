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

public:
    static void mainTitle() {
        cout << "\033[2J\033[H";    //limpia la pantalla
        cout << ANSI_DIAMOND;   //dice diamond pero es para utilizar el color 
        cout << title1 << endl;
        cout << title2 << endl;
        cout << title3 << endl;
        cout << title4 << endl;
        cout << title5 << endl;
        cout << "PRESS ENTER TO PLAY" << endl;
        cout << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void gameOver() {
        cout << "\033[31m"; // color rojo que no se habia definido en constantes
        cout << gameOver1 << endl;
        cout << gameOver2 << endl;
        cout << gameOver3 << endl;
        cout << gameOver4 << endl;
        cout << gameOver5 << endl;
        cout << "PRESS ENTER TO RETRY" << endl;
        cout << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void victory() {
        cout << ANSI_PLAYER; //color
        cout << victory1 << endl;
        cout << victory2 << endl;
        cout << victory3 << endl;
        cout << victory4 << endl;
        cout << victory5 << endl;
        cout << "PRESS ENTER TO RETRY" << endl;
        cout << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void pause() {
        cout << ANSI_ROCK; //nuevamente es un color nada mas (amarillo)
        cout << pause1 << endl;
        cout << pause2 << endl;
        cout << pause3 << endl;
        cout << pause4 << endl;
        cout << pause5 << endl;
        cout << "PRESS ENTER TO RESUME" << endl;
        cout << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }
};

