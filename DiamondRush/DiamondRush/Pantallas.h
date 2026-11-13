#pragma once
//Gestiona las diferentes pantallas del juego
//Incluye el menu principal, pausa, victoria y derrota

#include <string>
#include <iostream>

using namespace std;
class Pantallas {
private:
    string title1 = "████  ███  ███  █   █  ███  █   █ ████     ████  █   █  ████ █   █ ";
    string title2 = "█   █  █  █   █ ██ ██ █   █ ██  █ █   █    █   █ █   █ █     █   █ ";
    string title3 = "█   █  █  █████ █ █ █ █   █ █ █ █ █   █    ████  █   █  ███  █████ ";
    string title4 = "█   █  █  █   █ █   █ █   █ █  ██ █   █    █  █  █   █     █ █   █ ";
    string title5 = "████  ███ █   █ █   █  ███  █   █ ████     █   █  ███  ████  █   █ ";
    
    string gameOver1 = " ███   ███  █   █ █████     ███  █   █ █████ ████";
    string gameOver2 = "█     █   █ ██ ██ █        █   █ █   █ █     █   █";
    string gameOver3 = "█  ██ █████ █ █ █ ████     █   █ █   █ ████  ████";
    string gameOver4 = "█   █ █   █ █   █ █        █   █  █ █  █     █  █ ";
    string gameOver5 = " ███  █   █ █   █ █████     ███    █   █████ █   █ ";

    string victory1 = "█   █ ███  ███  █████  ███  ████  █   █ ";
    string victory2 = "█   █  █  █       █   █   █ █   █  █ █  ";
    string victory3 = "█   █  █  █       █   █   █ ████    █";
    string victory4 = " █ █   █  █       █   █   █ █  █    █ ";
    string victory5 = "  █   ███  ███    █    ███  █   █   █ ";

    string pause1 = "████   ███  █   █  ████ █████ ";
    string pause2 = "█   █ █   █ █   █ █     █     ";
    string pause3 = "████  █████ █   █  ███  ████  ";
    string pause4 = "█     █   █ █   █     █ █     ";
    string pause5 = "█     █   █  ███  ████  █████ ";

public:
    void mainTitle() {
        cout << title1 << endl;
        cout << title2 << endl;
        cout << title3 << endl;
        cout << title4 << endl;
        cout << title5 << endl;
        cout << "PRESS ENTER TO PLAY" << endl;
        cout << "PRESS ESC TO EXIT";
    }

    void gameOver() {
        cout << gameOver1 << endl;
        cout << gameOver2 << endl;
        cout << gameOver3 << endl;
        cout << gameOver4 << endl;
        cout << gameOver5 << endl;
        cout << "PRESS ENTER TO RETRY" << endl;
        cout << "PRESS ESC TO EXIT";
    }

    void victory() {
        cout << victory1 << endl;
        cout << victory2 << endl;
        cout << victory3 << endl;
        cout << victory4 << endl;
        cout << victory5 << endl;
        cout << "PRESS ENTER TO RETRY" << endl;
        cout << "PRESS ESC TO EXIT";
    }

    void pause() {
        cout << pause1 << endl;
        cout << pause2 << endl;
        cout << pause3 << endl;
        cout << pause4 << endl;
        cout << pause5 << endl;
        cout << "PRESS ENTER TO RESUME" << endl;
        cout << "PRESS ESC TO EXIT";
    }
};

