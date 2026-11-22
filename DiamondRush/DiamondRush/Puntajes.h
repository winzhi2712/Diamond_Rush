#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iostream>
#include "Constantes.h"

struct Registro {
    std::string nombre;
    int puntaje;
};

// Guarda y muestra los 5 mejores puntajes con nombre ("nombre puntaje" por linea)
class Puntajes {
private:
    static const int MAX = 5;
    std::vector<Registro> lista;   // siempre ordenada de mayor a menor
    std::string ruta;
    std::string nombreActual = "JUGADOR";

    static bool mayor(const Registro& a, const Registro& b) { return a.puntaje > b.puntaje; }

public:
    Puntajes(const std::string& archivo = "puntajes.txt") : ruta(archivo) {
        cargar();
    }

    void setNombre(const std::string& n) { nombreActual = n.empty() ? "JUGADOR" : n; }
    const std::string& getNombre() const { return nombreActual; }
    const std::vector<Registro>& getLista() const { return lista; }

    void cargar() {
        lista.clear();
        std::ifstream f(ruta);
        std::string linea;
        while (std::getline(f, linea)) {
            std::istringstream ss(linea);
            std::string a, b;
            if (!(ss >> a)) continue;
            Registro r;
            try {
                if (ss >> b) { r.nombre = a; r.puntaje = std::stoi(b); }
                else { r.nombre = "JUGADOR"; r.puntaje = std::stoi(a); }   // formato viejo: solo numero
            }
            catch (...) { continue; }
            lista.push_back(r);
        }
        std::stable_sort(lista.begin(), lista.end(), mayor);
        if ((int)lista.size() > MAX) lista.resize(MAX);
    }

    void guardar() {
        std::ofstream f(ruta);
        for (const auto& r : lista) {
            f << r.nombre << " " << r.puntaje << "\n";
        }
    }

    // Registra un puntaje. Devuelve su posicion (1 a 5) si entro al top, o 0 si no.
    // Si no se da nombre, usa el que se puso con setNombre().
    int agregar(int puntaje, const std::string& nombre = "") {
        if (puntaje <= 0) return 0;
        std::string n = nombre.empty() ? nombreActual : nombre;

        lista.push_back(Registro{ n, puntaje });
        std::stable_sort(lista.begin(), lista.end(), mayor);

        int pos = 0;
        for (size_t i = 0; i < lista.size(); i++) {
            if (lista[i].puntaje == puntaje && lista[i].nombre == n) {
                pos = (int)i + 1;
                break;
            }
        }

        if ((int)lista.size() > MAX) lista.resize(MAX);
        if (pos > MAX) pos = 0;

        guardar();
        return pos;
    }

    // Imprime la lista. Si "resaltar" es 1..5, esa posicion sale en amarillo.
    void imprimir(int resaltar = 0) const {
        std::string pad(MARGEN_IZQ, ' ');
        std::cout << pad << "MEJORES PUNTAJES\n";
        for (int i = 0; i < MAX; i++) {
            std::cout << pad << (i + 1) << ". ";
            if (i < (int)lista.size()) {
                if (i + 1 == resaltar) std::cout << "\033[93m";
                std::cout << lista[i].nombre << "  " << lista[i].puntaje << "\033[0m";
            }
            else {
                std::cout << "---";
            }
            std::cout << "\n";
        }
    }
};