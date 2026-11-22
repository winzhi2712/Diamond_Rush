#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "Terminal.h"
#include "Constantes.h"

// Pide el nombre (1 a 10 caracteres). Devuelve "" si el jugador presiona ESC.
inline std::string pedirNombre(Terminal& term) {
    std::string nombre = "";
    term.clearTerminal();
    term.limpiarBuffer();
    bool redibujar = true;
    while (true) {
        term.pollInputs();
        int k;
        while ((k = term.obtenerSiguiente()) != 0) {
            if (k == 13 || k == 10) {
                if (!nombre.empty()) return nombre;   // sin nombre no avanza
            }
            else if (k == 27) {
                return "";
            }
            else if ((k == 8 || k == 127) && !nombre.empty()) {
                nombre.pop_back();
                redibujar = true;
            }
            else if (k >= 33 && k <= 126 && nombre.size() < 10) {
                nombre += (char)k;
                redibujar = true;
            }
        }
        if (redibujar) {
            std::cout << "\033[" << (MARGEN_SUP + 2) << ";" << (MARGEN_IZQ + 1) << "H"
                << "ESCRIBE TU NOMBRE: " << nombre << "_            "
                << "\033[" << (MARGEN_SUP + 4) << ";" << (MARGEN_IZQ + 1) << "H"
                << "ENTER para empezar     ESC para volver" << std::flush;
            redibujar = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}