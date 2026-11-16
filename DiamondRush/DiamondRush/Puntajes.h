#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iostream>

// Guarda y muestra los 3 mejores puntajes (se conservan en un archivo de texto)
class Puntajes {
private:
    static const int MAX = 5;
    std::vector<int> lista;   // siempre ordenada de mayor a menor
    std::string ruta;

public:
    Puntajes(const std::string& archivo = "puntajes.txt") : ruta(archivo) {
        cargar();
    }

    void cargar() {
        lista.clear();
        std::ifstream f(ruta);
        int p;
        while (f >> p) {
            lista.push_back(p);
        }
        std::sort(lista.begin(), lista.end(), std::greater<int>());
        if ((int)lista.size() > MAX) lista.resize(MAX);
    }

    void guardar() {
        std::ofstream f(ruta);
        for (int p : lista) {
            f << p << "\n";
        }
    }

    // Registra un puntaje. Devuelve su posición (1 a 3) si entró al top, o 0 si no.
    int agregar(int puntaje) {
        if (puntaje <= 0) return 0;

        lista.push_back(puntaje);
        std::sort(lista.begin(), lista.end(), std::greater<int>());

        int pos = 0;
        for (size_t i = 0; i < lista.size(); i++) {
            if (lista[i] == puntaje) {
                pos = (int)i + 1;
                break;
            }
        }

        if ((int)lista.size() > MAX) lista.resize(MAX);
        if (pos > MAX) pos = 0;

        guardar();
        return pos;
    }

    // Imprime la lista. Si "resaltar" es 1..3, esa posición sale en amarillo.
    void imprimir(int resaltar = 0) const {
        std::cout << "MEJORES PUNTAJES\n";
        for (int i = 0; i < MAX; i++) {
            std::cout << (i + 1) << ". ";
            if (i < (int)lista.size()) {
                if (i + 1 == resaltar) std::cout << "\033[93m";
                std::cout << lista[i] << "\033[0m";
            }
            else {
                std::cout << "---";
            }
            std::cout << "\n";
        }
    }
};