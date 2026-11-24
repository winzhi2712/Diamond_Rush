#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include "Constantes.h"

// Un puesto del ranking: quién lo logró y cuántos puntos hizo
struct Registro {
    std::string nombre;
    int puntos = 0;
};

// Guarda y muestra los 5 mejores puntajes con su nombre (se conservan en un archivo de texto)
// Formato del archivo: una línea por puesto, "NOMBRE PUNTOS" (los nombres no llevan espacios)
class Puntajes {
private:
    static const int MAX = 5;
    static const int MAX_NOMBRE = 10;
    std::vector<Registro> lista;   // siempre ordenada de mayor a menor
    std::string ruta;

public:
    Puntajes(const std::string& archivo = "puntajes.txt") : ruta(archivo) {
        cargar();
    }

    void cargar() {
        lista.clear();
        std::ifstream f(ruta);
        std::string linea;
        while (std::getline(f, linea)) {
            std::istringstream ss(linea);
            std::string a, b;
            ss >> a >> b;
            if (a.empty()) continue;   // línea vacía

            Registro r;
            try {
                if (b.empty()) {          // formato viejo: solo el número
                    r.nombre = "ANONIMO";
                    r.puntos = std::stoi(a);
                }
                else {                    // formato nuevo: nombre y número
                    r.nombre = a;
                    r.puntos = std::stoi(b);
                }
            }
            catch (...) {
                continue;                 // línea dañada: se ignora
            }
            lista.push_back(r);
        }
        std::stable_sort(lista.begin(), lista.end(),
            [](const Registro& x, const Registro& y) { return x.puntos > y.puntos; });
        if ((int)lista.size() > MAX) lista.erase(lista.begin() + MAX, lista.end());
    }

    void guardar() {
        std::ofstream f(ruta);
        for (const auto& r : lista) {
            f << r.nombre << " " << r.puntos << "\n";
        }
    }

    // Registra un puntaje. Devuelve su posición (1 a 5) si entró al top, o 0 si no.
    int agregar(std::string nombre, int puntaje) {
        if (puntaje < 0) return 0;
        if (nombre.empty()) nombre = "ANONIMO";
        if ((int)nombre.size() > MAX_NOMBRE) nombre = nombre.substr(0, MAX_NOMBRE);

        // Busca dónde va: después de todos los que tienen igual o más puntos
        int pos = 0;
        while (pos < (int)lista.size() && lista[pos].puntos >= puntaje) {
            pos++;
        }
        if (pos >= MAX) return 0;   // no alcanza para el top

        Registro nuevo;
        nuevo.nombre = nombre;
        nuevo.puntos = puntaje;
        lista.insert(lista.begin() + pos, nuevo);
        if ((int)lista.size() > MAX) lista.erase(lista.begin() + MAX, lista.end());

        guardar();
        return pos + 1;
    }

    // Imprime la lista. Si "resaltar" es 1..5, esa posición sale en amarillo.
    void imprimir(int resaltar = 0) const {
        std::string pad(MARGEN_IZQ, ' ');
        std::cout << pad << "MEJORES PUNTAJES\n";
        for (int i = 0; i < MAX; i++) {
            std::cout << pad << (i + 1) << ". ";
            if (i < (int)lista.size()) {
                if (i + 1 == resaltar) std::cout << "\033[93m";
                std::cout << std::left << std::setw(MAX_NOMBRE) << lista[i].nombre
                    << " " << std::right << std::setw(6) << lista[i].puntos
                    << "\033[0m";
            }
            else {
                std::cout << "---";
            }
            std::cout << "\n";
        }
    }
    // Dibuja el top 5 como texto simple en blanco, con el título en dorado, en una posición absoluta.
// "fila" y "col" (desde 1) apuntan a la primera línea (el título). La lista mide 21 columnas
// y empieza dos filas más abajo. Ocupa 7 filas en total.
    void dibujarTop(int fila, int col) const {
        const std::string blanco = "\033[97m";
        const std::string dorado = "\033[1m\033[38;5;220m";
        const std::string reset = "\033[0m";
        auto irA = [](int f, int c) {
            return "\033[" + std::to_string(f) + ";" + std::to_string(c) + "H";
            };

        // Título con letras separadas y en negrita (31 columnas), centrado sobre la lista (21 columnas)
        const std::string titulo = "M E J O R E S   P U N T A J E S";
        std::cout << irA(fila, col + 10 - 15) << dorado << titulo << reset;

        for (int i = 0; i < MAX; i++) {
            std::ostringstream linea;
            linea << (i + 1) << ". ";
            if (i < (int)lista.size()) {
                linea << std::left << std::setw(MAX_NOMBRE) << lista[i].nombre
                    << std::right << std::setw(8) << lista[i].puntos;
            }
            else {
                linea << "---";
            }
            std::cout << irA(fila + 2 + i, col) << blanco << linea.str() << reset;
        }
        std::cout << std::flush;
    }
};