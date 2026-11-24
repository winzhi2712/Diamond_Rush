#pragma once
//Gestiona las diferentes pantallas del juego
//Incluye el menu principal, pausa, victoria y derrota
#include "Constantes.h"
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

// Estado de los diamantes que caen por toda la pantalla del menú
struct LluviaMenu {
    static const int N = 40;   // cantidad de diamantes
    int col[N];
    int fila[N];                // fila actual; negativa = todavía esperando arriba
    LluviaMenu() {
        for (int i = 0; i < N; i++) {
            col[i] = 2 + rand() % 97;       // columnas 2 a 98
            fila[i] = -(rand() % 26);       // arrancan escalonados, no todos juntos
        }
    }
};

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
    inline static const int TABLILLA_ANCHO = 40;   // ancho interior de la tablilla del nombre
    // Medidas del marco del menú principal
    inline static const int LLUVIA_FILAS = 26;   // hasta qué fila caen los diamantes

    // true si en esa celda hay texto del menú (los diamantes pasan "por detrás" y no se dibujan ahí)
    static bool zonaTextoMenu(int fila, int col) {
        if (fila >= 2 && fila <= 10 && col >= 14 && col <= 86) return true;     // título y subrayado
        if (fila >= 11 && fila <= 15 && col >= 43 && col <= 57) return true;    // opciones
        if (fila >= 16 && fila <= 24 && col >= 8 && col <= 92) return true;     // top 5 y muñequitos
        return false;
    }

    // Color de cada "pixel" del explorador
    static string colorPixel(char c) {
        switch (c) {
        case 'H': return "\033[38;5;94m";    // sombrero café
        case 'h': return "\033[38;5;130m";   // cinta del sombrero
        case 'S': return "\033[38;5;180m";   // piel
        case 'E': return "\033[38;5;232m";   // ojos
        case 'Y': return "\033[38;5;178m";   // camisa amarilla
        case 'y': return "\033[38;5;136m";   // manga en sombra
        case 'B': return "\033[38;5;94m";    // cinturón
        case 'K': return "\033[38;5;240m";   // botas
        }
        return "";
    }

    // Dibuja un explorador de bloques con su látigo. (fila, col) es la esquina superior izquierda.
    // derecha = false: látigo a la derecha. derecha = true: es el espejo.
    static void explorador(int fila, int col, bool derecha) {
        // Cuerpo: cada letra es un pixel de 2 columnas ('.' = vacío). 7 de ancho x 8 de alto.
        const string sprite[8] = {
            ".HHHHH.",   // copa del sombrero
            ".hhhhh.",   // cinta
            "HHHHHHH",   // ala
            ".SESES.",   // ojos
            ".SSSSS.",   // cara
            "yYYYYYS",   // brazos (la mano derecha sostiene el látigo)
            "YYBBBYY",   // cinturón
            ".KK.KK."    // botas
        };
        // Látigo con medios bloques: 4 filas (las filas 3 a 6 del sprite) x 10 columnas.
        // Sale de la mano (fila 5), baja un poco, sube en onda y termina en la punta arriba.
        const string latigo[4][10] = {
            { " ", " ", " ", " ", " ", " ", " ", " ", "▄", "▄" },
            { " ", " ", " ", " ", " ", " ", "▄", "▀", " ", " " },
            { "▄", "▄", " ", " ", "▄", "▀", " ", " ", " ", " " },
            { " ", " ", "▀", "▀", " ", " ", " ", " ", " ", " " }
        };
        const string COLOR_LATIGO = "\033[38;5;130m";
        const string COLOR_PUNTA = "\033[38;2;60;35;20m";   // café muy oscuro

        for (int r = 0; r < 8; r++) {
            // --- el cuerpo ---
            string cuerpo = "";
            for (int i = 0; i < 7; i++) {
                char c = sprite[r][derecha ? 6 - i : i];
                if (c == '.') cuerpo += "  ";
                else cuerpo += colorPixel(c) + "██" + ANSI_RESET;
            }
            // --- el látigo (solo en las filas 3 a 6) ---
            string whip = "";
            if (r >= 3 && r <= 6) {
                for (int i = 0; i < 10; i++) {
                    int idx = derecha ? 9 - i : i;
                    const string& cel = latigo[r - 3][idx];
                    if (cel == " ") whip += " ";
                    else {
                        bool punta = (r == 3 && idx >= 8);   // la punta es café muy oscuro
                        whip += (punta ? COLOR_PUNTA : COLOR_LATIGO) + cel + ANSI_RESET;
                    }
                }
            }
            else {
                whip = string(10, ' ');
            }
            cout << irA(fila + r, col) << (derecha ? whip + cuerpo : cuerpo + whip);
        }
    }

    // Explorador sin látigo que mira hacia la izquierda (hacia la tablilla del nombre).
    // (fila, col) es la esquina superior izquierda. "escala" agranda el dibujo: cada pixel pasa a medir
    // (2 * escala) columnas x escala filas. Con escala 1 mide 14 x 8; con escala 2, 28 x 16; con 3, 42 x 24.
    static void exploradorMirando(int fila, int col, int escala = 2) {
        // Cada letra es un pixel ('.' = vacío). 'a' y 'b' son medio pixel de piel y medio pixel de ojo negro.
        const string sprite[8] = {
            ".HHHHH.",   // copa del sombrero
            ".hhhhh.",   // cinta
            "HHHHHHH",   // ala
            ".ababS.",   // ojos negros corridos medio pixel a la izquierda (a = mitad derecha oscura, b = mitad izquierda oscura)
            ".SSSSS.",   // cara
            "SYYYYYy",   // brazos
            "YYBBBYY",   // cinturón
            ".KK.KK."    // botas
        };
        for (int r = 0; r < 8; r++) {
            string linea = "";
            for (int i = 0; i < 7; i++) {
                char c = sprite[r][i];
                if (c == '.') linea += string(2 * escala, ' ');
                else if (c == 'a' || c == 'b') {
                    // medio pixel = "escala" columnas: una mitad de piel y la otra de ojo negro
                    string piel = colorPixel('S'), ojo = colorPixel('E');
                    string mitad = "";
                    for (int k = 0; k < escala; k++) mitad += "█";
                    if (c == 'a') linea += piel + mitad + ojo + mitad + ANSI_RESET;   // piel | ojo
                    else          linea += ojo + mitad + piel + mitad + ANSI_RESET;   // ojo | piel
                }
                else {
                    linea += colorPixel(c);
                    for (int k = 0; k < escala; k++) linea += "██";
                    linea += ANSI_RESET;
                }
            }
            // la misma fila se repite "escala" veces hacia abajo
            for (int k = 0; k < escala; k++) {
                cout << irA(fila + r * escala + k, col) << linea;
            }
        }
    }

    // Pone una "|" a la derecha de cada bloque de █: reemplaza el primer espacio que le sigue,
    // así el ancho del texto no cambia. Se aplica al imprimir, por eso los textos quedan limpios.
    static string sombra(const string& s) {
        string r = "";
        bool trasBloque = false;
        size_t i = 0;
        while (i < s.size()) {
            unsigned char c = s[i];
            size_t n = 1;
            if (c >= 0xF0) n = 4;
            else if (c >= 0xE0) n = 3;
            else if (c >= 0xC0) n = 2;
            string ch = s.substr(i, n);
            if (ch == "█") { r += ch; trasBloque = true; }
            else if (ch == " " && trasBloque) { r += "|"; trasBloque = false; }
            else { r += ch; trasBloque = false; }
            i += n;
        }
        return r;
    }

    // Corta un texto UTF-8 por columnas de pantalla (no por bytes): cada "█" ocupa 3 bytes pero 1 columna.
    // "desde" empieza en 0.
    static string cortarColumnas(const string& s, int desde, int cuantas) {
        string salida = "";
        int col = 0;
        size_t i = 0;
        while (i < s.size() && col < desde + cuantas) {
            unsigned char c = s[i];
            size_t n = 1;
            if (c >= 0xF0) n = 4;
            else if (c >= 0xE0) n = 3;
            else if (c >= 0xC0) n = 2;
            if (col >= desde) salida += s.substr(i, n);
            i += n;
            col++;
        }
        return salida;
    }
    // Mueve el cursor a una fila y columna de la pantalla (empiezan en 1)
    static string irA(int fila, int col) {
        return "\033[" + to_string(fila) + ";" + to_string(col) + "H";
    }

    // Dibuja una fila de la tablilla: borde, contenido centrado y borde.
    // "anchoVisible" es cuántas columnas ocupa el contenido en pantalla (sin contar los códigos de color)
    static void lineaTablilla(int fila, const string& contenido, int anchoVisible) {
        int izq = (TABLILLA_ANCHO - anchoVisible) / 2;
        int der = TABLILLA_ANCHO - anchoVisible - izq;
        cout << irA(fila, MARGEN_IZQ + 1)
            << ANSI_CAFE << WALL_V << ANSI_RESET
            << string(izq, ' ') << contenido << ANSI_RESET << string(der, ' ')
            << ANSI_CAFE << WALL_V << ANSI_RESET;
    }
    // Una fila (0 a 4) de una letra grande de 5 columnas. Solo están las letras de NUEVO RECORD.
    static string glifo(char letra, int fila) {
        static const string gN[5] = { "█   █", "██  █", "█ █ █", "█  ██", "█   █" };
        static const string gU[5] = { "█   █", "█   █", "█   █", "█   █", " ███ " };
        static const string gE[5] = { "█████", "█    ", "████ ", "█    ", "█████" };
        static const string gV[5] = { "█   █", "█   █", "█   █", " █ █ ", "  █  " };
        static const string gO[5] = { " ███ ", "█   █", "█   █", "█   █", " ███ " };
        static const string gR[5] = { "████ ", "█   █", "████ ", "█  █ ", "█   █" };
        static const string gC[5] = { " ████", "█    ", "█    ", "█    ", " ████" };
        static const string gD[5] = { "████ ", "█   █", "█   █", "█   █", "████ " };

        switch (letra) {
        case 'N': return gN[fila];
        case 'U': return gU[fila];
        case 'E': return gE[fila];
        case 'V': return gV[fila];
        case 'O': return gO[fila];
        case 'R': return gR[fila];
        case 'C': return gC[fila];
        case 'D': return gD[fila];
        }
        return "     ";
    }

    // Una fila de una palabra completa en letras grandes (una columna de separación entre letras)
    static string filaPalabra(const string& palabra, int fila) {
        string salida = "";
        for (size_t i = 0; i < palabra.size(); i++) {
            if (i > 0) salida += " ";
            salida += glifo(palabra[i], fila);
        }
        return salida;
    }

public:

    static void gameOver() {
        cout << string(MARGEN_SUP, '\n');
        cout << "\033[31m"; // color rojo que no se habia definido en constantes
        const string* f[5] = { &gameOver1, &gameOver2, &gameOver3, &gameOver4, &gameOver5 };
        for (int i = 0; i < 5; i++) cout << pad() << sombra(*f[i]) << endl;
        cout << pad() << "PRESS ENTER TO RETRY" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void victory() {
        cout << string(MARGEN_SUP, '\n');
        cout << ANSI_PLAYER; //color
        const string* f[5] = { &victory1, &victory2, &victory3, &victory4, &victory5 };
        for (int i = 0; i < 5; i++) cout << pad() << sombra(*f[i]) << endl;
        cout << pad() << "PRESS ENTER TO RETRY" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }

    static void pause() {
        cout << string(MARGEN_SUP, '\n');
        cout << ANSI_ROCK; //nuevamente es un color nada mas (amarillo)
        const string* f[5] = { &pause1, &pause2, &pause3, &pause4, &pause5 };
        for (int i = 0; i < 5; i++) cout << pad() << sombra(*f[i]) << endl;
        cout << pad() << "PRESS ENTER TO RESUME" << endl;
        cout << pad() << "PRESS ESC TO EXIT";
        cout << ANSI_RESET;
    }
    // Dibuja la tablilla completa donde se escribe el nombre (se llama una sola vez)
    static void pedirNombre() {
        string horizontal;
        for (int i = 0; i < TABLILLA_ANCHO; i++) horizontal += WALL_H;

        cout << "\033[2J\033[H";
        cout << irA(MARGEN_SUP + 1, MARGEN_IZQ + 1) << ANSI_CAFE
            << WALL_TL << horizontal << WALL_TR << ANSI_RESET;

        for (int r = 2; r <= 8; r += 2) {
            lineaTablilla(MARGEN_SUP + r, "", 0);   // filas en blanco
        }

        const string titulo = "ESCRIBE TU NOMBRE, EXPLORADOR(A)";
        lineaTablilla(MARGEN_SUP + 3, ANSI_ORO + titulo, (int)titulo.size());

        cout << irA(MARGEN_SUP + 9, MARGEN_IZQ + 1) << ANSI_CAFE
            << WALL_BL << horizontal << WALL_BR << ANSI_RESET;

        cout << irA(MARGEN_SUP + 11, MARGEN_IZQ + 1) << ANSI_ORO << "ENTER      " << ANSI_RESET << "Confirmar";
        cout << irA(MARGEN_SUP + 12, MARGEN_IZQ + 1) << ANSI_ORO << "RETROCESO  " << ANSI_RESET << "Borrar letra";
        cout << irA(MARGEN_SUP + 13, MARGEN_IZQ + 1) << ANSI_ORO << "ESC        " << ANSI_RESET << "Volver";

        // Explorador grande (escala 2) a la derecha de la tablilla, que mide TABLILLA_ANCHO + 2 columnas con sus bordes.
        // Su parte de arriba queda a la altura del borde superior de la tablilla.
        exploradorMirando(MARGEN_SUP + 1, MARGEN_IZQ + 1 + TABLILLA_ANCHO + 2 + 5, 2);
        cout << flush;
    }

    // Repinta solo las ranuras del nombre y el contador (se llama cada vez que algo cambia)
    static void campoNombre(const string& nombre, bool cursorVisible) {
        const int MAX = 10;
        string ranuras = "";
        for (int i = 0; i < MAX; i++) {
            if (i > 0) ranuras += " ";
            if (i < (int)nombre.size()) {
                ranuras += ANSI_VERDE + string(1, nombre[i]) + ANSI_RESET;
            }
            else if (i == (int)nombre.size() && cursorVisible) {
                ranuras += ANSI_ORO + string("█") + ANSI_RESET;
            }
            else {
                ranuras += ANSI_MORADO + string("_") + ANSI_RESET;
            }
        }
        lineaTablilla(MARGEN_SUP + 5, ranuras, 2 * MAX - 1);

        string contador = to_string(nombre.size()) + "/" + to_string(MAX);
        lineaTablilla(MARGEN_SUP + 7, ANSI_CAFE + contador, (int)contador.size());
        cout << flush;
    }

    // Aviso en rojo cuando se intenta confirmar sin escribir nada (mostrar = false lo borra)
    static void avisoNombre(bool mostrar) {
        cout << irA(MARGEN_SUP + 15, MARGEN_IZQ + 1);
        if (mostrar) cout << "\033[91m" << "Escribe al menos una letra" << ANSI_RESET;
        else cout << string(26, ' ');
        cout << flush;
    }

    // Cartel gigante de NUEVO RECORD. "fase" va subiendo para que el brillo recorra las letras (animación).
    // Ocupa 5 filas empezando en "filaInicio".
    static void nuevoRecord(int fase, int filaInicio) {
        // Del oro profundo al blanco y de vuelta: el destello sube por las filas
        const string paleta[8] = {
            "\033[1m\033[38;5;178m",   // oro oscuro
            "\033[1m\033[38;5;220m",   // oro
            "\033[1m\033[38;5;221m",   // oro claro
            "\033[1m\033[38;5;228m",   // amarillo clarito
            "\033[1m\033[97m",         // blanco
            "\033[1m\033[38;5;228m",   // amarillo clarito
            "\033[1m\033[38;5;221m",   // oro claro
            "\033[1m\033[38;5;220m"    // oro
        };
        for (int r = 0; r < 5; r++) {
            cout << irA(filaInicio + r, MARGEN_IZQ + 1) << paleta[(fase + r) % 8]
                << filaPalabra("NUEVO", r) << "   " << filaPalabra("RECORD", r) << ANSI_RESET;
        }
        cout << flush;
    }

    // Cartel sobrio para los puestos 2 al 5 (una sola línea, sin animación)
    static void entrasAlTop(int posicion, int filaInicio) {
        cout << irA(filaInicio + 2, MARGEN_IZQ + 1)
            << ANSI_VERDE << "♦ ENTRAS AL TOP ♦" << ANSI_RESET
            << "  Puesto " << posicion << " de 5" << flush;
    }

    // Dibuja el título del menú de una vez: verde de arbusto, con un corazón rojo en lugar de la O,
    // un subrayado y los dos exploradores a los lados del top 5
    static void menuPrincipal() {
        cout << "\033[2J\033[H";

        // --- Título ---
        const string* filas[5] = { &title1, &title2, &title3, &title4, &title5 };
        // Corazón relleno de un solo rojo brillante, con su "|" a la derecha de cada fila. Cada fila mide 8 columnas.
        const string corazon[5] = {
            " ██|██| ",
            "███████|",
            "▀█████▀|",
            " ▀███▀| ",
            "  ▀█▀|  "
        };
        const string COLOR_CORAZON = "\033[91m";   // rojo brillante (el mismo de los corazones del juego)
        // letras antes del corazón + corazón + letras después (sin espacio entre el corazón y la N)
        const int ANCHO_TITULO = 22 + 8 + 39;
        int colTitulo = MENU_CENTRO - ANCHO_TITULO / 2;

        for (int r = 0; r < 5; r++) {
            // La O de DIAMOND ocupa las columnas 22 a 26 (contando desde 0): se reemplaza por el diamante
            string antes = sombra(cortarColumnas(*filas[r], 0, 22));
            string despues = sombra(cortarColumnas(*filas[r], 27, 40));
            // despues.substr(1) quita el espacio que quedaba entre el corazón y la N
            cout << irA(3 + r, colTitulo) << ANSI_VERDE << antes
                << COLOR_CORAZON << corazon[r]
                << ANSI_VERDE << despues.substr(1) << ANSI_RESET;
        }

        // --- Subrayado: línea doble con un diamante al centro y debajo una línea fina ---
        string tramo = "";
        for (int i = 0; i < 33; i++) tramo += WALL_H;
        cout << irA(9, colTitulo) << ANSI_CAFE << tramo
            << ANSI_MORADO << " ♦ "
            << ANSI_CAFE << tramo << ANSI_RESET;

        string fina = "";
        for (int i = 0; i < 45; i++) fina += "─";
        cout << irA(10, MENU_CENTRO - 22) << ANSI_VERDE << fina << ANSI_RESET;

        // --- Exploradores a los lados del top 5 (mismo sprite, uno espejado) ---
        explorador(MENU_TOP_FILA, MENU_CENTRO - 41, false);
        explorador(MENU_TOP_FILA, MENU_CENTRO + 18, true);
        cout << flush;
    }

    // Dibuja JUGAR y SALIR centrados. La elegida lleva "->" y parpadea entre dorado y amarillo clarito.
    // seleccion: 0 = JUGAR, 1 = SALIR
    static void menuOpciones(int seleccion, bool brillante) {
        const string opciones[2] = { "JUGAR", "SALIR" };
        for (int i = 0; i < 2; i++) {
            int fila = 12 + i * 2;
            if (i == seleccion) {
                string color = brillante ? "\033[1m\033[38;5;220m" : "\033[1m\033[38;5;229m";
                cout << irA(fila, MENU_CENTRO - 5) << color << "->" << ANSI_RESET;
                cout << irA(fila, MENU_CENTRO - 2) << color << opciones[i] << ANSI_RESET;
            }
            else {
                cout << irA(fila, MENU_CENTRO - 5) << "  ";
                cout << irA(fila, MENU_CENTRO - 2) << "\033[37m" << opciones[i] << ANSI_RESET;
            }
        }
        cout << flush;
    }
    // Hace bajar una fila a cada diamante. Al llegar abajo reaparece arriba en otra columna.
    static void menuLluvia(LluviaMenu& ll) {
        for (int i = 0; i < LluviaMenu::N; i++) {
            if (ll.fila[i] >= 1 && !zonaTextoMenu(ll.fila[i], ll.col[i])) {
                cout << irA(ll.fila[i], ll.col[i]) << " ";   // borra el rastro
            }
            ll.fila[i]++;
            if (ll.fila[i] > LLUVIA_FILAS) {
                ll.col[i] = 2 + rand() % 97;
                ll.fila[i] = -(rand() % 8);
            }
            if (ll.fila[i] >= 1 && !zonaTextoMenu(ll.fila[i], ll.col[i])) {
                string color = (i % 2 == 0) ? ANSI_MORADO : "\033[38;5;134m";
                cout << irA(ll.fila[i], ll.col[i]) << color << "♦" << ANSI_RESET;
            }
        }
        cout << flush;
    }
};