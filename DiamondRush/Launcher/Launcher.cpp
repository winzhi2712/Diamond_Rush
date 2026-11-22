// =====================================================================
//  Launcher.cpp  -  Pantalla de inicio de Diamond Rush
//
//  (El archivo es solo ASCII: los simbolos van como bytes UTF-8 en \xNN,
//   para que compile igual aunque Visual Studio lo guarde con otra codificacion.)
//
//  Es lo primero que se ve al ejecutar el juego: pregunta si se quiere
//  jugar en MODO CONSOLA o en MODO GUI y ejecuta la version elegida.
//
//  NO modifica nada del juego de consola: solo lo ejecuta como programa
//  aparte (DiamondRush / DiamondRush.exe). Lo unico que reutiliza, sin
//  tocarlo, es Terminal.h (lectura de teclas) y Constantes.h (colores).
// =====================================================================
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <system_error>
#include <filesystem>

#include "Terminal.h"
#include "Constantes.h"

namespace fs = std::filesystem;

// Nombres de los ejecutables que se lanzan (sin ".exe"; se agrega solo en Windows)
static const string EXE_CONSOLA = "DiamondRush";
static const string EXE_GUI = "DiamondRushGUI";

// Archivo que se usa para comprobar que la carpeta de trabajo es la correcta
static const string ARCHIVO_NIVEL = "nivel1.txt";

// ---------------------------------------------------------------------
//  Utilidades de texto (los simbolos UTF-8 ocupan varios bytes pero 1 columna)
// ---------------------------------------------------------------------
static int anchoCols(const string& s) {
    int n = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) n++;   // no cuenta los bytes de continuacion
    }
    return n;
}

static string relleno(const string& s, int ancho) {
    int n = anchoCols(s);
    return s + string(n < ancho ? ancho - n : 0, ' ');
}

static string repetir(const string& s, int veces) {
    string r;
    for (int i = 0; i < veces; i++) r += s;
    return r;
}

// ---------------------------------------------------------------------
//  Busqueda y lanzamiento de ejecutables
// ---------------------------------------------------------------------
static bool existeArchivo(const fs::path& p) {
    std::error_code ec;
    return fs::exists(p, ec) && fs::is_regular_file(p, ec);
}

// Busca el ejecutable junto al lanzador y, si no, en la carpeta de trabajo.
// Devuelve una ruta vacia si no lo encuentra.
static fs::path buscarExe(const string& nombre, const fs::path& dirLanzador) {
#ifdef _WIN32
    const string archivo = nombre + ".exe";
#else
    const string archivo = nombre;
#endif
    fs::path candidatos[2] = { dirLanzador / archivo, fs::path(".") / archivo };
    for (const fs::path& c : candidatos) {
        if (existeArchivo(c)) return c;
    }
    return fs::path();
}

static string comando(const fs::path& exe) {
    string ruta = exe.string();
#ifdef _WIN32
    return "\"\"" + ruta + "\"\"";   // cmd.exe pide comillas dobles alrededor de todo
#else
    return "'" + ruta + "'";
#endif
}

// ---------------------------------------------------------------------
//  Dibujo de la pantalla
// ---------------------------------------------------------------------
struct Opcion {
    string numero;        // "1" o "2"
    string nombre;        // "MODO CONSOLA"
    string descripcion;   // texto que sale bajo las cajas cuando esta seleccionada
    bool disponible;
};

static const int ANCHO_CAJA = 36;                  // ancho interior de cada caja
static const int FILA_BRILLO = MARGEN_SUP + 7;     // fila (desde 1) de la linea de gemas animadas

static string pad() { return string(MARGEN_IZQ, ' '); }

// Linea de gemas que "brillan" en ola; solo se redibuja esta linea cada cierto tiempo
static string lineaBrillo(int fase) {
    static const char* colores[4] = { "\033[95m", "\033[38;5;135m", "\033[97m", "\033[38;5;54m" };
    string s = "\033[" + to_string(FILA_BRILLO) + ";1H" + pad() + "   ";
    for (int i = 0; i < 16; i++) {
        s += colores[(i + fase) % 4];
        s += "\xE2\x99\xA6";
        s += ANSI_RESET;
        if (i < 15) s += "   ";
    }
    return s;
}

static string caja(const Opcion& op, bool seleccionada) {
    string sangria = pad() + string(15, ' ');
    string borde, relleno_fondo, texto;

    if (!op.disponible) {
        borde = "\033[38;5;240m";
        relleno_fondo = "";
    }
    else if (seleccionada) {
        borde = "\033[1m\033[38;5;220m";                          // dorado
        relleno_fondo = "\033[48;5;22m\033[38;5;230m\033[1m";    // verde selva con texto crema
    }
    else {
        borde = "\033[38;5;137m";                                  // marron tierra
        relleno_fondo = "\033[38;5;180m";                          // arena
    }

    string interior = (seleccionada ? "  \xE2\x96\xBA  " : "     ") + op.numero + "   " + op.nombre;
    if (!op.disponible) interior += "  (no instalado)";

    string arriba = "\xE2\x95\x94" + repetir("\xE2\x95\x90", ANCHO_CAJA) + "\xE2\x95\x97";
    string medio = "\xE2\x95\x91" + relleno_fondo + relleno(interior, ANCHO_CAJA) + ANSI_RESET + borde + "\xE2\x95\x91";
    string abajo = "\xE2\x95\x9A" + repetir("\xE2\x95\x90", ANCHO_CAJA) + "\xE2\x95\x9D";

    string out;
    out += sangria + borde + arriba + ANSI_RESET + "\n";
    out += sangria + borde + medio + ANSI_RESET + "\n";
    out += sangria + borde + abajo + ANSI_RESET + "\n";
    return out;
}

static void dibujarPantalla(int sel, const Opcion ops[2], const string& aviso, int fase) {
    static const int degradado[5] = { 154, 118, 82, 40, 34 };   // verde selva: de claro a oscuro
    static const string titulo[5] = {
        "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88     \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 ",
        "\xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88    \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88     \xE2\x96\x88   \xE2\x96\x88 ",
        "\xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88    \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88 ",
        "\xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88    \xE2\x96\x88  \xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88     \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 ",
        "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88 \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88     \xE2\x96\x88   \xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88  \xE2\x96\x88   \xE2\x96\x88 "
    };

    string out = "\033[2J\033[H";
    out += string(MARGEN_SUP, '\n');
    for (int i = 0; i < 5; i++) {
        out += pad() + "\033[38;5;" + to_string(degradado[i]) + "m" + titulo[i] + ANSI_RESET + "\n";
    }
    out += "\n";
    out += lineaBrillo(fase) + "\n";   // fila FILA_BRILLO
    out += "\n";
    out += pad() + string(15, ' ') + "\033[1m\033[38;5;220mELIGE COMO QUIERES JUGAR" + ANSI_RESET + "\n";
    out += "\n";
    out += caja(ops[0], sel == 0);
    out += "\n";
    out += caja(ops[1], sel == 1);
    out += "\n";
    out += pad() + "\033[38;5;180m" + ops[sel].descripcion + ANSI_RESET + "\n";
    out += pad() + "\033[91m" + aviso + ANSI_RESET + "\n";
    out += pad() + "\033[38;5;208mW/S o flechas\033[38;5;180m elegir     \033[38;5;208mENTER\033[38;5;180m jugar     \033[38;5;208mESC\033[38;5;180m salir\033[0m";
    cout << out << flush;
}

// ---------------------------------------------------------------------
//  Programa principal
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);   // UTF-8, igual que el juego
#endif

    // static: Terminal registra un puntero a si misma para restaurar la consola al salir
    static Terminal terminal;

    // Carpeta donde esta el lanzador (para encontrar los otros ejecutables)
    fs::path dirLanzador = fs::path(".");
    if (argc > 0) {
        std::error_code ec;
        fs::path completa = fs::absolute(argv[0], ec);
        if (!ec) dirLanzador = completa.parent_path();
    }

    fs::path rutaConsola = buscarExe(EXE_CONSOLA, dirLanzador);
    fs::path rutaGui = buscarExe(EXE_GUI, dirLanzador);

    Opcion ops[2] = {
        { "1", "MODO CONSOLA", "Modo texto: Versión creada en Consola.", !rutaConsola.empty() },
        { "2", "MODO GRÁFICO",     "Modo gui: Versión con efectos, creada para Presentación.", !rutaGui.empty() }
    };

    string aviso = "";
    if (!ops[0].disponible) {
        aviso = "No encuentro " + EXE_CONSOLA + " junto al lanzador.";
    }

    terminal.init();

    int sel = 0;
    int fase = 0;
    bool redibujar = true;
    bool salir = false;
    auto ultimoBrillo = std::chrono::steady_clock::now();

    while (!salir) {
        if (redibujar) {
            dibujarPantalla(sel, ops, aviso, fase);
            redibujar = false;
        }

        terminal.pollInputs();
        int tecla;
        while (!salir && (tecla = terminal.obtenerSiguiente()) != 0) {
            // Teclas: W/S y flechas (arriba = 256+72, abajo = 256+80 en Windows, 256+88 en Linux)
            if (tecla == 'w' || tecla == 'W' || tecla == 'a' || tecla == 'A' || tecla == 256 + 72 || tecla == 256 + 75) {
                sel = (sel + 1) % 2;
                aviso = "";
                redibujar = true;
            }
            else if (tecla == 's' || tecla == 'S' || tecla == 'd' || tecla == 'D' ||
                tecla == 256 + 80 || tecla == 256 + 88 || tecla == 256 + 77) {
                sel = (sel + 1) % 2;
                aviso = "";
                redibujar = true;
            }
            else if (tecla == '1' || tecla == '2') {
                sel = tecla - '1';
                aviso = "";
                redibujar = true;
            }
            else if (tecla == 27) {   // ESC
                salir = true;
            }
            else if (tecla == 13 || tecla == 10) {   // ENTER (13 en Windows, 10 en Linux)
                if (!ops[sel].disponible) {
                    aviso = (sel == 1)
                        ? "El modo GUI aun no esta instalado (falta " + EXE_GUI + ")."
                        : "No encuentro " + EXE_CONSOLA + " junto al lanzador.";
                    redibujar = true;
                }
                else if (!existeArchivo(fs::path(ARCHIVO_NIVEL))) {
                    aviso = "Falta " + ARCHIVO_NIVEL + " en la carpeta de trabajo del lanzador.";
                    redibujar = true;
                }
                else {
                    // Se devuelve la terminal a su estado normal antes de lanzar el juego,
                    // para que el juego guarde y restaure un estado correcto
                    terminal.restore();
                    cout << ANSI_RESET << "\033[2J\033[H" << flush;

                    fs::path exe = (sel == 0) ? rutaConsola : rutaGui;
                    int resultado = std::system(comando(exe).c_str());
                    (void)resultado;

                    // Al cerrar el juego se vuelve a esta pantalla
                    terminal.init();
                    terminal.limpiarBuffer();
                    aviso = "";
                    redibujar = true;
                }
            }
        }

        // Animacion de las gemas (solo se repinta esa linea)
        auto ahora = std::chrono::steady_clock::now();
        if (!redibujar && std::chrono::duration_cast<std::chrono::milliseconds>(ahora - ultimoBrillo).count() >= 220) {
            ultimoBrillo = ahora;
            fase = (fase + 1) % 4;
            cout << lineaBrillo(fase) << flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    terminal.restore();
    cout << ANSI_RESET << "\033[2J\033[H" << flush;
    return 0;
}