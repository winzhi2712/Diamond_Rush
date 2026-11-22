// =====================================================================
//  DiamondRushGUI.cpp  -  Modo GUI de Diamond Rush (raylib)
//
//  Es un programa aparte del juego de consola. NO modifica ningun
//  archivo de la consola: reutiliza, tal cual estan, la logica del juego
//  (Fisica, Game, MapLoader, Puntajes, Tipos, Constantes, Matrix) y solo
//  cambia la forma de dibujar y de leer el teclado.
//
//  No usa imagenes ni archivos externos: todo se dibuja por codigo.
//  (El archivo es solo ASCII, sin tildes, para compilar igual en cualquier
//   configuracion de Visual Studio.)
// =====================================================================
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>

#include "Constantes.h"
#include "Tipos.h"
#include "Matrix.h"
#include "MapLoader.h"
#include "Fisica.h"
#include "Game.h"
#include "Puntajes.h"

// raylib trae su propio tipo llamado "Matrix" que choca con la plantilla
// Matrix<T> del juego; se le cambia el nombre solo mientras se lee raylib.h
#define Matrix RaylibMatrix
#include "raylib.h"
#undef Matrix

using std::string;
using std::to_string;
using std::vector;

// ---------------------------------------------------------------------
//  Configuracion
// ---------------------------------------------------------------------
static const int TOTAL_NIVELES = 3;
static const bool MODO_DEV = true;   // true = las teclas 1, 2 y 3 saltan de nivel (como en la consola)

static const int VW = 1280;           // lienzo virtual: se escala a la ventana
static const int VH = 720;
static const int TILE = 32;           // lado de cada casilla en pixeles
static const int HUD_H = 80;          // alto de la barra superior (40 x 32 = 1280, 20 x 32 = 640, 640 + 80 = 720)

static const int RETARDO_REPETICION = 6;   // ticks que hay que mantener una tecla para que repita
static const int PASO_REPETICION = 3;      // cada cuantos ticks se repite el movimiento

// Pixel-art de los titulos (los mismos de las pantallas de la consola)
static const char* const ARTE_TITULO[5] = {
    "####  ###  ###  #   #  ###  #   # ####     ####  #   #  #### #   # ",
    "#   #  #  #   # ## ## #   # ##  # #   #    #   # #   # #     #   # ",
    "#   #  #  ##### # # # #   # # # # #   #    ####  #   #  ###  ##### ",
    "#   #  #  #   # #   # #   # #  ## #   #    #  #  #   #     # #   # ",
    "####  ### #   # #   #  ###  #   # ####     #   #  ###  ####  #   # "
};
static const char* const ARTE_GAMEOVER[5] = {
    " ###   ###  #   # #####     ###  #   # ##### ####  ",
    "#     #   # ## ## #        #   # #   # #     #   # ",
    "#  ## ##### # # # ####     #   # #   # ####  ####  ",
    "#   # #   # #   # #        #   #  # #  #     #  #  ",
    " ###  #   # #   # #####     ###    #   ##### #   # "
};
static const char* const ARTE_VICTORIA[5] = {
    "#   # ###  ###  #####  ###  ####  #   # ",
    "#   #  #  #       #   #   # #   #  # #  ",
    "#   #  #  #       #   #   # ####    #   ",
    " # #   #  #       #   #   # #  #    #   ",
    "  #   ###  ###    #    ###  #   #   #   "
};
static const char* const ARTE_PAUSA[5] = {
    "####   ###  #   #  #### ##### ",
    "#   # #   # #   # #     #     ",
    "####  ##### #   #  ###  ####  ",
    "#     #   # #   #     # #     ",
    "#     #   #  ###  ####  ##### "
};

// ---------------------------------------------------------------------
//  Colores (paleta de Diamond Rush: selva, tierra, gemas moradas, oro)
// ---------------------------------------------------------------------
static Color rgb(int r, int g, int b, int a = 255) {
    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };
}
static Color mezcla(Color a, Color b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return rgb((int)(a.r + (b.r - a.r) * t), (int)(a.g + (b.g - a.g) * t),
        (int)(a.b + (b.b - a.b) * t), (int)(a.a + (b.a - a.a) * t));
}

static const Color NEGRO = { 0, 0, 0, 255 };
static const Color ORO = { 230, 175, 60, 255 };
static const Color ORO_CLARO = { 250, 215, 110, 255 };
static const Color CREMA = { 245, 235, 205, 255 };
static const Color ARENA = { 205, 170, 125, 255 };
static const Color TIERRA_OSCURA = { 24, 16, 12, 255 };
static const Color TIERRA = { 52, 36, 26, 255 };
static const Color ROJO_VIDA = { 235, 60, 60, 255 };

// ---------------------------------------------------------------------
//  Utilidades de dibujo
// ---------------------------------------------------------------------
static unsigned hash2(int a, int b) {
    unsigned h = (unsigned)a * 73856093u ^ (unsigned)b * 19349663u;
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;
    return h;
}

// raylib solo dibuja triangulos con un orden de vertices; aqui se corrige solo
static void tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
    float cruz = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cruz > 0) std::swap(b, c);
    DrawTriangle(a, b, c, col);
}

static void textoCentrado(const string& s, int cx, int y, int tam, Color c) {
    DrawText(s.c_str(), cx - MeasureText(s.c_str(), tam) / 2, y, tam, c);
}

// Dibuja un titulo de pixel-art ('#' = bloque) con degradado vertical y sombra
static void dibujarArte(const char* const* lineas, int n, int cx, int y, int px, Color arriba, Color abajo) {
    int ancho = 0;
    for (int i = 0; i < n; i++) ancho = std::max(ancho, (int)std::strlen(lineas[i]));
    int x0 = cx - (ancho * px) / 2;
    int sombra = std::max(2, px / 4);
    for (int r = 0; r < n; r++) {
        for (int c = 0; lineas[r][c] != '\0'; c++) {
            if (lineas[r][c] == '#') DrawRectangle(x0 + c * px + sombra, y + r * px + sombra, px, px, rgb(0, 0, 0, 150));
        }
    }
    for (int r = 0; r < n; r++) {
        Color col = mezcla(arriba, abajo, n > 1 ? (float)r / (float)(n - 1) : 0.0f);
        for (int c = 0; lineas[r][c] != '\0'; c++) {
            if (lineas[r][c] == '#') {
                DrawRectangle(x0 + c * px, y + r * px, px, px, col);
                DrawRectangle(x0 + c * px, y + r * px, px, std::max(1, px / 6), mezcla(col, rgb(255, 255, 255), 0.35f));
            }
        }
    }
}

// Gema morada centrada en (cx, cy); "tam" equivale al lado de una casilla
static void dibujarGema(float cx, float cy, float tam, double t, unsigned semilla) {
    float k = tam / 32.0f;
    auto P = [&](float x, float y) { return Vector2{ cx - 16.0f * k + x * k, cy - 16.0f * k + y * k }; };
    Color claro = rgb(214, 150, 255), base = rgb(170, 70, 230), medio = rgb(140, 50, 200), oscuro = rgb(100, 30, 160);
    tri(P(10, 6), P(22, 6), P(27, 13), base);
    tri(P(10, 6), P(27, 13), P(5, 13), base);
    tri(P(10, 6), P(17, 6), P(12, 13), claro);
    tri(P(5, 13), P(16, 13), P(16, 29), medio);
    tri(P(16, 13), P(27, 13), P(16, 29), oscuro);
    DrawLineEx(P(5, 13), P(27, 13), std::max(1.0f, k), rgb(60, 10, 100, 150));
    float s = std::sin((float)t * 2.5f + (float)(semilla % 628) / 100.0f);
    if (s > 0.92f) {
        Vector2 c = P(12, 9);
        float l = 5.0f * k;
        DrawLineEx(Vector2{ c.x - l, c.y }, Vector2{ c.x + l, c.y }, std::max(1.0f, k), rgb(255, 255, 255));
        DrawLineEx(Vector2{ c.x, c.y - l }, Vector2{ c.x, c.y + l }, std::max(1.0f, k), rgb(255, 255, 255));
    }
}

static void dibujarCorazon(float cx, float cy, Color col) {
    DrawCircleV(Vector2{ cx - 5.0f, cy - 3.0f }, 5.5f, col);
    DrawCircleV(Vector2{ cx + 5.0f, cy - 3.0f }, 5.5f, col);
    tri(Vector2{ cx - 10.3f, cy - 1.0f }, Vector2{ cx + 10.3f, cy - 1.0f }, Vector2{ cx, cy + 10.0f }, col);
}
// ---------------------------------------------------------------------
//  Sonidos (se generan por codigo, no hay archivos)
// ---------------------------------------------------------------------
struct Sonidos {
    Sound gema, dano, roca, corazon, latigo, enemigo, nivel, perder, victoria, paso, puerta;
};
static Sound MUS_MENU, MUS_JUEGO;
static Sonidos SND;
static bool audioOk = false;

// dur = segundos, f0 -> f1 = tono inicial y final (Hz), ruido = 0..1 (mezcla de ruido),
// salto = multiplica la frecuencia a la mitad del sonido (para hacer dos notas)
static Sound crearSonido(float dur, float f0, float f1, float ruido, bool cuadrada, float vol, float salto = 1.0f) {
    const int sr = 44100;
    int n = (int)(dur * sr);
    std::vector<short> datos(n);
    float fase = 0.0f;
    unsigned semilla = 12345u;
    for (int i = 0; i < n; i++) {
        float k = (float)i / (float)n;
        float f = f0 + (f1 - f0) * k;
        if (k > 0.5f) f *= salto;
        fase += 2.0f * 3.14159265f * f / (float)sr;
        float tono = cuadrada ? (std::sin(fase) >= 0.0f ? 0.6f : -0.6f) : std::sin(fase);
        semilla = semilla * 1664525u + 1013904223u;
        float r = (float)((semilla >> 16) & 0x7FFF) / 16383.5f - 1.0f;
        float env = (1.0f - k) * (1.0f - k);          // se apaga poco a poco
        if (i < 120) env *= (float)i / 120.0f;        // entrada suave, sin "clic"
        float v = (tono * (1.0f - ruido) + r * ruido) * env * vol;
        datos[i] = (short)(std::max(-1.0f, std::min(1.0f, v)) * 32000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = sr;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = datos.data();
    return LoadSoundFromWave(w);   // copia los datos, por eso no hace falta guardarlos
}

static float frecNota(int midi) {
    return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
}

// mel y bajo: una nota MIDI por paso (0 = silencio). largoMel/largoBajo = cuantos pasos dura cada nota.
static Sound crearMusica(const int* mel, const int* bajo, int pasos, float segPorPaso,
    int largoMel, int largoBajo, float volMel, float volBajo) {
    const int sr = 22050;
    int porPaso = (int)(segPorPaso * sr);
    int n = pasos * porPaso;
    std::vector<float> mezclaBuf(n, 0.0f);
    for (int voz = 0; voz < 2; voz++) {
        const int* notas = (voz == 0) ? mel : bajo;
        int largo = (voz == 0) ? largoMel : largoBajo;
        float vol = (voz == 0) ? volMel : volBajo;
        for (int p = 0; p < pasos; p++) {
            if (notas[p] <= 0) continue;
            float f = frecNota(notas[p]);
            int dur = std::min(largo * porPaso, n - p * porPaso);
            for (int i = 0; i < dur; i++) {
                float k = (float)i / (float)dur;
                float ang = 2.0f * 3.14159265f * f * (float)i / (float)sr;
                float onda = std::sin(ang) + 0.3f * std::sin(2.0f * ang);
                float env = std::exp(-3.0f * k);
                if (i < 150) env *= (float)i / 150.0f;
                if (dur - i < 150) env *= (float)(dur - i) / 150.0f;
                mezclaBuf[p * porPaso + i] += onda * env * vol;
            }
        }
    }

    std::vector<short> datos(n);
    for (int i = 0; i < n; i++) {
        datos[i] = (short)(std::max(-1.0f, std::min(1.0f, mezclaBuf[i])) * 32000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = sr;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = datos.data();
    return LoadSoundFromWave(w);
}
static Sound crearMusicaJuego() {
    const int sr = 22050;
    const float seg = 0.22f;                 // segundos por paso (menos = mas rapida)
    const int porPaso = (int)(seg * sr);
    const int compases = 16, pasosC = 8;
    const int n = compases * pasosC * porPaso;
    std::vector<float> buf(n, 0.0f);
    unsigned sem = 777u;

    auto nota = [&](int inicio, int dur, int midi, float vol, float decay, bool cuadrada) {
        float f = frecNota(midi);
        for (int i = 0; i < dur && inicio + i < n; i++) {
            float k = (float)i / (float)dur;
            float ang = 2.0f * 3.14159265f * f * (float)i / (float)sr;
            float onda = cuadrada ? (std::sin(ang) >= 0.0f ? 0.5f : -0.5f)
                : std::sin(ang) + 0.3f * std::sin(2.0f * ang);
            float env = std::exp(-decay * k);
            if (i < 100) env *= (float)i / 100.0f;
            if (dur - i < 100) env *= (float)(dur - i) / 100.0f;
            buf[inicio + i] += onda * env * vol;
        }
        };
    auto ruido = [&](int inicio, int dur, float vol, float decay) {
        for (int i = 0; i < dur && inicio + i < n; i++) {
            sem = sem * 1664525u + 1013904223u;
            float r = (float)((sem >> 16) & 0x7FFF) / 16383.5f - 1.0f;
            buf[inicio + i] += r * std::exp(-decay * (float)i / (float)dur) * vol;
        }
        };
    auto bombo = [&](int inicio, int dur, float vol) {
        float fase = 0.0f;
        for (int i = 0; i < dur && inicio + i < n; i++) {
            float k = (float)i / (float)dur;
            fase += 2.0f * 3.14159265f * (110.0f - 70.0f * k) / (float)sr;
            buf[inicio + i] += std::sin(fase) * std::exp(-5.0f * k) * vol;
        }
        };

    // Acordes: {nota del bajo, tres notas del arpegio}
    const int ac[5][4] = { {50,62,65,69},    // 0 Dm
                           {46,62,65,70},    // 1 Bb
                           {45,61,64,69},    // 2 A
                           {43,62,67,70},    // 3 Gm
                           {41,60,65,69} };  // 4 F
    const int prog[16] = { 0,0,1,2,  0,0,3,2,  1,4,3,0,  1,4,2,2 };   // acorde de cada compas
    const int patron[16] = { 0,1,0,3,  0,1,2,3,  2,1,2,3,  2,1,2,0 };   // melodia de cada compas
    const int mel[4][8] = { { 2,-1,-1, 1, 2,-1, 1,-1 },
                            { 0,-1, 1,-1, 2,-1,-1,-1 },
                            { 2, 1, 0,-1, 0, 1, 2,-1 },
                            {-1, 1,-1, 0,-1, 2,-1,-1 } };

    for (int c = 0; c < compases; c++) {
        const int* a = ac[prog[c]];
        bool segunda = (c >= 8);
        for (int s = 0; s < pasosC; s++) {
            int ini = (c * pasosC + s) * porPaso;
            // bajo en los pasos pares
            if (s % 2 == 0) nota(ini, porPaso * 2, a[0], 0.22f, 2.0f, true);
            // arpegio suave de fondo
            nota(ini, porPaso, a[1 + (s % 3)], 0.05f, 3.0f, false);
            // melodia
            int d = mel[patron[c]][s];
            if (d >= 0) nota(ini, porPaso * 2, a[1 + d] + 12, 0.10f, 2.5f, false);
            // percusion
            if (s == 0 || s == 4) bombo(ini, porPaso, 0.35f);
            if (segunda && (s == 2 || s == 6)) ruido(ini, porPaso, 0.10f, 6.0f);   // caja
            if (s % 2 == 1) ruido(ini, porPaso / 3, 0.035f, 8.0f);                 // platillo
        }
    }

    std::vector<short> datos(n);
    for (int i = 0; i < n; i++) {
        datos[i] = (short)(std::max(-1.0f, std::min(1.0f, buf[i])) * 32000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = sr;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = datos.data();
    return LoadSoundFromWave(w);
}
static Sound crearLatigo() {
    const int sr = 44100;
    const float dur = 0.26f, tChasquido = 0.11f;
    int n = (int)(dur * sr);
    std::vector<short> datos(n);
    unsigned sem = 4242u;
    float previo = 0.0f;
    for (int i = 0; i < n; i++) {
        float t = (float)i / (float)sr;
        sem = sem * 1664525u + 1013904223u;
        float r = (float)((sem >> 16) & 0x7FFF) / 16383.5f - 1.0f;
        float agudo = r - previo;   // quita los graves: queda lo "cortante"
        previo = r;
        float v;
        if (t < tChasquido) {
            float k = t / tChasquido;
            v = agudo * 0.25f * k * k;                       // siseo que va creciendo
        }
        else {
            float d = t - tChasquido;
            v = agudo * 0.55f * std::exp(-d * 70.0f);        // el chasquido, muy corto
            v += std::sin(2.0f * 3.14159265f * 220.0f * d) * std::exp(-d * 45.0f) * 0.35f;   // toque seco
        }
        if (i < 40) v *= (float)i / 40.0f;
        datos[i] = (short)(std::max(-1.0f, std::min(1.0f, v)) * 32000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = sr;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = datos.data();
    return LoadSoundFromWave(w);
}

static Sound crearPuerta() {
    const int sr = 44100;
    const float dur = 0.75f;
    int n = (int)(dur * sr);
    std::vector<short> datos(n);
    unsigned sem = 9001u;
    float filtro = 0.0f;
    float fase = 0.0f;
    for (int i = 0; i < n; i++) {
        float k = (float)i / (float)n;
        sem = sem * 1664525u + 1013904223u;
        float r = (float)((sem >> 16) & 0x7FFF) / 16383.5f - 1.0f;
        filtro += (r - filtro) * 0.06f;                          // ruido grave: roce de madera sobre piedra
        float temblor = 0.6f + 0.4f * std::sin(2.0f * 3.14159265f * 18.0f * k * dur);   // vibracion del arrastre
        float roce = filtro * 3.2f * temblor;
        fase += 2.0f * 3.14159265f * (62.0f + 14.0f * std::sin(k * 9.0f)) / (float)sr;
        float madera = std::sin(fase) * 0.35f;                   // retumbo grave de la hoja
        float env = std::min(1.0f, k * 8.0f) * (1.0f - k * k);   // entra rapido y se apaga al final
        float v = (roce + madera) * env * 0.7f;
        datos[i] = (short)(std::max(-1.0f, std::min(1.0f, v)) * 32000.0f);
    }
    Wave w;
    w.frameCount = (unsigned int)n;
    w.sampleRate = sr;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = datos.data();
    return LoadSoundFromWave(w);
}
static void cargarSonidos() {
    InitAudioDevice();
    audioOk = IsAudioDeviceReady();
    if (!audioOk) return;
    SetMasterVolume(0.6f);
    SND.gema = crearSonido(0.16f, 880.0f, 880.0f, 0.0f, false, 0.45f, 1.5f);
    SND.dano = crearSonido(0.28f, 240.0f, 70.0f, 0.30f, true, 0.60f);
    SND.roca = crearSonido(0.28f, 180.0f, 70.0f, 0.15f, false, 0.90f);     // golpe sordo
    SND.corazon = crearSonido(0.35f, 520.0f, 1040.0f, 0.0f, false, 0.50f);
    SND.latigo = crearLatigo();
    SND.enemigo = crearSonido(0.22f, 320.0f, 60.0f, 0.40f, true, 0.60f);
    SND.nivel = crearSonido(0.55f, 440.0f, 440.0f, 0.0f, false, 0.50f, 1.5f);
    SND.perder = crearSonido(0.80f, 330.0f, 50.0f, 0.15f, true, 0.60f);
    SND.victoria = crearSonido(0.90f, 523.0f, 523.0f, 0.0f, false, 0.55f, 2.0f);
    SND.paso = crearSonido(0.06f, 180.0f, 90.0f, 0.25f, false, 0.12f);     // pisada suave
    SND.puerta = crearPuerta();

    // Menu: tranquila, en La menor
    const int melMenu[32] = { 69,0,72,0,76,0,72,0,  77,0,76,0,72,0,69,0,
                              67,0,71,0,74,0,71,0,  76,0,74,0,72,0,71,0 };
    const int bajoMenu[32] = { 45,0,0,0,45,0,0,0,  41,0,0,0,41,0,0,0,
                               43,0,0,0,43,0,0,0,  40,0,0,0,40,0,0,0 };
    MUS_MENU = crearMusica(melMenu, bajoMenu, 32, 0.28f, 2, 4, 0.12f, 0.20f);

    MUS_JUEGO = crearMusicaJuego();
    SetSoundVolume(MUS_JUEGO, 0.6f);
}

static void descargarSonidos() {
    if (!audioOk) return;
    UnloadSound(SND.gema);    UnloadSound(SND.dano);    UnloadSound(SND.roca);
    UnloadSound(SND.corazon); UnloadSound(SND.latigo);  UnloadSound(SND.enemigo);
    UnloadSound(SND.nivel);   UnloadSound(SND.perder);  UnloadSound(SND.victoria);
    UnloadSound(SND.paso);    UnloadSound(SND.puerta);
    UnloadSound(MUS_MENU);    UnloadSound(MUS_JUEGO);
    CloseAudioDevice();
}

// Pone la musica que corresponde al estado (menu o jugando) y calla la otra
static void actualizarMusica(GameState e) {
    if (!audioOk) return;
    Sound* quiere = nullptr;
    if (e == GameState::MENU) quiere = &MUS_MENU;
    else if (e == GameState::PLAYING) quiere = &MUS_JUEGO;
    if (quiere != &MUS_MENU && IsSoundPlaying(MUS_MENU)) StopSound(MUS_MENU);
    if (quiere != &MUS_JUEGO && IsSoundPlaying(MUS_JUEGO)) StopSound(MUS_JUEGO);
    if (quiere != nullptr && !IsSoundPlaying(*quiere)) PlaySound(*quiere);
}
static void sonar(const Sound& s) {
    if (audioOk) PlaySound(s);
}
// ---------------------------------------------------------------------
//  Estado del juego (reproduce lo que hace main() en DiamondRush.cpp)
// ---------------------------------------------------------------------
struct Nivel {
    Matrix<CellType> mapa;    // capa fija
    Matrix<CellType> ents;    // capa movil
    Fisica fisica;
    vector<std::pair<int, int>> gemas;   // gemas ya recogidas (para no reaparecer al reiniciar)
    Nivel() : mapa(MAP_HEIGHT, MAP_WIDTH), ents(MAP_HEIGHT, MAP_WIDTH), fisica(mapa, ents) {
        mapa.setAll(CellType::EMPTY);
        ents.setAll(CellType::EMPTY);
    }
};

struct Juego {
    Game game;
    Puntajes puntajes;
    std::unique_ptr<Nivel> niveles[TOTAL_NIVELES + 1];
    int numeroNivel = 1;
    GameState estado = GameState::MENU;
    bool salir = false;

    int posRecord = 0;
    int puntajeFinal = 0;
    string nombre = "";
    double bonusInicio = -10.0;   // momento en que se ganaron los últimos puntos
    int bonusValor = 0;
    bool escribiendoNombre = false;
    vector<Registro> top;

    // entrada
    Mov pendMov = Mov::VACIA;
    bool pendLatigo = false;
    Mov dirAct = Mov::VACIA;
    int repeticion = 0;
    int mirarF = 0, mirarC = 1;   // hacia donde mira el jugador (solo para dibujarlo)
    double acumulador = 0.0;
};

static vector<Registro> leerTop5() {
    Puntajes p;   // lee puntajes.txt
    return p.getLista();
}

static void construirNivel(Juego& j, int n) {
    GameMap datos = MapLoader::cargarNivel("nivel" + to_string(n) + ".txt");
    std::unique_ptr<Nivel> nv(new Nivel());
    for (int r = 0; r < MAP_HEIGHT; r++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            nv->mapa.setValue(r, c, datos.grid[r][c]);
            nv->ents.setValue(r, c, datos.entities[r][c]);
        }
    }
    nv->fisica.colocarJugador(datos.playerY, datos.playerX);   // (fila, columna)
    for (const auto& ene : datos.enemigos) {
        nv->fisica.agregarEnemigo(ene.fila, ene.col, ene.dirFila, ene.dirCol, ene.ticks, ene.persigue, ene.tipoCelda);
    }
    j.niveles[n] = std::move(nv);
}

// Construye el nivel n si todavia no existe (por ejemplo, al saltar de nivel con las teclas de desarrollo)
static void asegurarNivel(Juego& j, int n) {
    if (!j.niveles[n]) construirNivel(j, n);
}

static void limpiarEntrada(Juego& j) {
    j.pendMov = Mov::VACIA;
    j.pendLatigo = false;
    j.dirAct = Mov::VACIA;
    j.repeticion = 0;
    j.acumulador = 0.0;
}

static void cambiarANivel(Juego& j, int n) {
    j.numeroNivel = n;
    limpiarEntrada(j);
}

static void nuevaPartida(Juego& j) {
    j.game.reset();
    for (int i = 1; i <= TOTAL_NIVELES; i++) j.niveles[i].reset();
    construirNivel(j, 1);
    cambiarANivel(j, 1);
    j.mirarF = 0;
    j.mirarC = 1;
    j.estado = GameState::PLAYING;
}

static void irAlMenu(Juego& j) {
    j.top = leerTop5();
    j.estado = GameState::MENU;
}

static void terminarPartida(Juego& j, GameState resultado) {
    j.puntajeFinal = j.game.getScore();
    j.posRecord = j.puntajes.agregar(j.puntajeFinal, j.nombre);   // guarda en puntajes.txt, igual que la consola
    j.top = leerTop5();
    j.estado = resultado;
    sonar(resultado == GameState::VICTORY ? SND.victoria : SND.perder);
}

// ---------------------------------------------------------------------
//  Entrada de teclado
// ---------------------------------------------------------------------
static bool teclaDir(Mov m) {
    switch (m) {
    case Mov::UP:    return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
    case Mov::DOWN:  return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
    case Mov::LEFT:  return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    case Mov::RIGHT: return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    default: return false;
    }
}

static Mov direccionPresionada() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) return Mov::UP;
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) return Mov::DOWN;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) return Mov::LEFT;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) return Mov::RIGHT;
    return Mov::VACIA;
}

static void reiniciarNivelActual(Juego& j) {
    int n = j.numeroNivel;
    vector<std::pair<int, int>> gemas = j.niveles[n]->gemas;   // copia: construirNivel crea un nivel limpio
    construirNivel(j, n);
    j.niveles[n]->gemas = gemas;
    for (const auto& g : gemas) {
        j.niveles[n]->ents.setValue(g.first, g.second, CellType::EMPTY);   // las gemas ya recogidas no vuelven
    }
    cambiarANivel(j, n);
}

static void pausar(Juego& j) {
    j.game.pausar();
    j.estado = GameState::PAUSE;
}

static void reanudar(Juego& j) {
    j.game.reanudar();
    limpiarEntrada(j);
    j.estado = GameState::PLAYING;
}

// Lee el teclado una vez por fotograma
static void leerEntrada(Juego& j) {
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

    switch (j.estado) {
    case GameState::MENU:
        if (j.escribiendoNombre) {
            int k = GetCharPressed();
            while (k > 0) {
                if (k >= 33 && k <= 126 && j.nombre.size() < 10) j.nombre += (char)k;
                k = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !j.nombre.empty()) j.nombre.pop_back();
            if (IsKeyPressed(KEY_ENTER) && !j.nombre.empty()) {   // sin nombre no avanza
                j.escribiendoNombre = false;
                nuevaPartida(j);
            }
            else if (IsKeyPressed(KEY_ESCAPE)) j.escribiendoNombre = false;
        }
        else {
            if (IsKeyPressed(KEY_ENTER)) { j.nombre = ""; j.escribiendoNombre = true; }
            else if (IsKeyPressed(KEY_ESCAPE)) j.salir = true;
        }
        break;

    case GameState::PLAYING: {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) || !IsWindowFocused()) {
            pausar(j);
            break;
        }
        if (IsKeyPressed(KEY_R)) { reiniciarNivelActual(j); break; }
        if (MODO_DEV) {
            for (int n = 1; n <= TOTAL_NIVELES; n++) {
                if (IsKeyPressed(KEY_ONE + (n - 1))) {
                    asegurarNivel(j, n);
                    cambiarANivel(j, n);
                }
            }
        }
        if (IsKeyPressed(KEY_E)) j.pendLatigo = true;

        Mov p = direccionPresionada();
        if (p != Mov::VACIA) {
            j.pendMov = p;
            j.dirAct = p;
        }
        if (j.dirAct != Mov::VACIA && !teclaDir(j.dirAct)) {
            j.dirAct = Mov::VACIA;
            const Mov todas[4] = { Mov::UP, Mov::DOWN, Mov::LEFT, Mov::RIGHT };
            for (Mov m : todas) {
                if (teclaDir(m)) j.dirAct = m;
            }
        }
        break;
    }

    case GameState::PAUSE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_P)) reanudar(j);
        else if (IsKeyPressed(KEY_ESCAPE)) irAlMenu(j);
        break;

    case GameState::VICTORY:
    case GameState::GAMEOVER:
        if (IsKeyPressed(KEY_ENTER)) { irAlMenu(j); j.nombre = ""; j.escribiendoNombre = true; }
        else if (IsKeyPressed(KEY_ESCAPE)) irAlMenu(j);
        break;
    }
}

// Decide la accion de este tick: una pulsacion se ejecuta enseguida; mantenida, se repite
static Mov elegirAccion(Juego& j) {
    if (j.pendLatigo) {
        j.pendLatigo = false;
        return Mov::LATIGO;
    }
    if (j.pendMov != Mov::VACIA) {
        Mov m = j.pendMov;
        j.pendMov = Mov::VACIA;
        j.repeticion = 0;
        return m;
    }
    if (j.dirAct != Mov::VACIA) {
        j.repeticion++;
        if (j.repeticion >= RETARDO_REPETICION && (j.repeticion - RETARDO_REPETICION) % PASO_REPETICION == 0) {
            return j.dirAct;
        }
    }
    else {
        j.repeticion = 0;
    }
    return Mov::VACIA;
}

// ---------------------------------------------------------------------
//  Un tick de juego (misma logica y mismo orden que el bucle de la consola)
// ---------------------------------------------------------------------
static void tick(Juego& j) {
    Mov accion = elegirAccion(j);
    if (accion == Mov::UP) { j.mirarF = -1; j.mirarC = 0; }
    else if (accion == Mov::DOWN) { j.mirarF = 1; j.mirarC = 0; }
    else if (accion == Mov::LEFT) { j.mirarF = 0; j.mirarC = -1; }
    else if (accion == Mov::RIGHT) { j.mirarF = 0; j.mirarC = 1; }

    Nivel& nv = *j.niveles[j.numeroNivel];
    int f0 = nv.fisica.getFilaJugador(), c0 = nv.fisica.getColJugador();
    ResultadoTick res = nv.fisica.actualizar(accion);
    if (res.latigo)           sonar(SND.latigo);
    if (res.rocaAterrizo)     sonar(SND.roca);
    if (res.puntosGanados > 0) sonar(SND.gema);
    if (res.puntosExtra > 0)  sonar(SND.enemigo);
    if (res.curacion > 0)     sonar(SND.corazon);
    if (res.dano > 0)         sonar(SND.dano);
    if (res.puertaAbrio)      sonar(SND.puerta);
    if (audioOk && (nv.fisica.getFilaJugador() != f0 || nv.fisica.getColJugador() != c0)) {
        static bool alterna = false;
        alterna = !alterna;
        SetSoundPitch(SND.paso, alterna ? 0.9f : 1.1f);   // dos tonos alternos, como pasos
        sonar(SND.paso);
    }

    int ganado = res.puntosGanados + res.puntosExtra;
    if (res.curacion > 0) {
        if (j.game.getVide() >= 100) ganado += puntosCorazonLleno;
        else j.game.curar(res.curacion);
    }
    if (ganado > 0) {
        j.game.addScore(ganado);
        double ahora = GetTime();
        if (ahora - j.bonusInicio < 0.9) j.bonusValor += ganado; else j.bonusValor = ganado;
        j.bonusInicio = ahora;
    }
    if (res.puntosGanados > 0 || res.curacion > 0) {
        nv.gemas.push_back({ nv.fisica.getFilaJugador(), nv.fisica.getColJugador() });
    }
    if (res.dano > 0) {
        j.game.perderVida(res.dano);
    }
    if (j.game.perder() || j.game.getVide() <= 0) {
        terminarPartida(j, GameState::GAMEOVER);
        return;
    }
    if (res.nivelGanado) {
        if (j.numeroNivel < TOTAL_NIVELES) {
            sonar(SND.nivel);
            int siguiente = j.numeroNivel + 1;
            asegurarNivel(j, siguiente);
            cambiarANivel(j, siguiente);   // el reloj sigue: 5 minutos para los tres niveles
        }
        else {
            j.game.addScore(j.game.timeLeft() * 5);   // bono por el tiempo que sobro
            terminarPartida(j, GameState::VICTORY);
        }
        return;
    }
    if (res.volverAtras && j.numeroNivel > 1) {
        asegurarNivel(j, j.numeroNivel - 1);
        cambiarANivel(j, j.numeroNivel - 1);
    }
}

static void actualizar(Juego& j, float dt) {
    leerEntrada(j);
    if (j.estado == GameState::PLAYING) {
        const double paso = 1.0 / ticksPorSegundo;
        j.acumulador += std::min(dt, 0.1f);
        while (j.acumulador >= paso && j.estado == GameState::PLAYING) {
            j.acumulador -= paso;
            tick(j);
        }
    }
}

// ---------------------------------------------------------------------
//  Casillas
// ---------------------------------------------------------------------
static void dibujarVacio(int px, int py, int f, int c) {
    unsigned h = hash2(f, c);
    int v = (int)(h % 7);
    DrawRectangle(px, py, TILE, TILE, rgb(34 + v, 25 + v, 20 + v));
    if ((h >> 8) % 5 == 0) {
        DrawRectangle(px + (int)((h >> 12) % 24) + 2, py + (int)((h >> 16) % 24) + 2, 2, 2, rgb(62, 47, 37));
    }
}

static void dibujarPared(int px, int py, int f, int c) {
    unsigned h = hash2(f, c);
    Color mortero = rgb(96, 70, 48);
    DrawRectangle(px, py, TILE, TILE, mortero);
    for (int r = 0; r < 2; r++) {
        int y0 = py + r * 16;
        int x[4], w[4], n;
        if (((f + r) % 2) == 0) { n = 2; x[0] = 0; w[0] = 16; x[1] = 16; w[1] = 16; }
        else { n = 3; x[0] = 0; w[0] = 8; x[1] = 8; w[1] = 16; x[2] = 24; w[2] = 8; }
        for (int i = 0; i < n; i++) {
            int tono = (int)((h >> (i * 3 + r * 7)) % 18);
            Color base = rgb(175 - 9 + tono, 135 - 9 + tono, 95 - 9 + tono);
            DrawRectangle(px + x[i], y0, w[i] - 1, 15, base);
            DrawRectangle(px + x[i], y0, w[i] - 1, 2, rgb(208, 170, 128));
            DrawRectangle(px + x[i], y0 + 13, w[i] - 1, 2, rgb(138, 102, 70));
        }
    }
}

static void dibujarSalida(int px, int py, double t) {
    float p = 0.75f + 0.25f * std::sin((float)t * 3.0f + (float)px);   // luz que respira
    Color piedra = rgb(128, 102, 74), piedraClara = rgb(168, 138, 102), piedraOscura = rgb(78, 58, 40);
    Color vacio = rgb(8, 5, 4);

    // marco: arco de piedra
    DrawCircleV(Vector2{ px + 16.0f, py + 15.0f }, 15.0f, piedra);
    DrawRectangle(px + 1, py + 15, 30, 17, piedra);

    // interior oscuro
    DrawCircleV(Vector2{ px + 16.0f, py + 15.0f }, 10.0f, vacio);
    DrawRectangle(px + 6, py + 15, 20, 17, vacio);

    // luz calida que sube desde el fondo, como una antorcha al otro lado
    DrawRectangleGradientV(px + 6, py + 15, 20, 17, vacio, mezcla(vacio, rgb(255, 190, 90), 0.85f * p));

    // detalles de piedra: bloques, brillo y dovela central
    DrawRectangle(px + 1, py + 15, 2, 17, piedraClara);
    DrawRectangle(px + 29, py + 15, 2, 17, piedraOscura);
    DrawRectangle(px + 1, py + 22, 5, 1, piedraOscura);
    DrawRectangle(px + 1, py + 28, 5, 1, piedraOscura);
    DrawRectangle(px + 26, py + 19, 5, 1, piedraOscura);
    DrawRectangle(px + 26, py + 25, 5, 1, piedraOscura);
    DrawRectangle(px + 13, py + 0, 6, 5, piedraClara);
    DrawRectangle(px + 13, py + 4, 6, 1, piedraOscura);

    // una mota de polvo flotando en la luz
    float subida = std::fmod((float)t * 6.0f + (float)px, 12.0f);
    DrawCircleV(Vector2{ px + 16.0f + 4.0f * std::sin((float)t * 1.3f), py + 28.0f - subida }, 1.0f, rgb(255, 225, 160, 170));
}

static void dibujarArbusto(int px, int py, int f, int c) {
    unsigned h = hash2(f, c);
    DrawCircleV(Vector2{ px + 10.0f, py + 19.0f }, 9.0f, rgb(30, 130, 50));
    DrawCircleV(Vector2{ px + 22.0f, py + 19.0f }, 9.0f, rgb(36, 150, 56));
    DrawCircleV(Vector2{ px + 16.0f, py + 12.0f }, 10.0f, rgb(60, 190, 75));
    DrawCircleV(Vector2{ px + 13.0f, py + 9.0f + (float)(h % 3) }, 3.5f, rgb(130, 235, 120));
    DrawCircleV(Vector2{ px + 21.0f, py + 16.0f }, 2.5f, rgb(110, 220, 100));
}

static Color colorLlave(int color) {
    if (color == 0) return rgb(240, 70, 70);
    if (color == 1) return rgb(70, 235, 90);
    return rgb(90, 140, 255);
}

static void dibujarBoton(int px, int py, int color, bool presionado) {
    Color c = colorLlave(color);
    DrawCircleV(Vector2{ px + 16.0f, py + 17.0f }, 12.0f, rgb(30, 22, 18));
    DrawCircleV(Vector2{ px + 16.0f, py + 16.0f }, 11.0f, mezcla(c, rgb(0, 0, 0), 0.55f));
    if (presionado) DrawCircleV(Vector2{ px + 16.0f, py + 16.0f }, 8.0f, mezcla(c, rgb(255, 255, 255), 0.2f));
    else DrawCircleV(Vector2{ px + 16.0f, py + 15.0f }, 8.0f, c);
}

static void dibujarPuerta(int px, int py, int color, bool abierta) {
    Color c = colorLlave(color);
    if (abierta) {
        DrawRectangle(px + 2, py, 3, TILE, rgb(c.r, c.g, c.b, 90));
        DrawRectangle(px + 27, py, 3, TILE, rgb(c.r, c.g, c.b, 90));
        return;
    }
    DrawRectangle(px, py, TILE, TILE, rgb(30, 22, 18));
    for (int i = 0; i < 4; i++) {
        DrawRectangle(px + 3 + i * 7, py, 4, TILE, c);
        DrawRectangle(px + 3 + i * 7, py, 1, TILE, mezcla(c, rgb(255, 255, 255), 0.5f));
    }
    DrawRectangle(px, py + 5, TILE, 3, mezcla(c, rgb(0, 0, 0), 0.5f));
    DrawRectangle(px, py + 24, TILE, 3, mezcla(c, rgb(0, 0, 0), 0.5f));
}

// ---------------------------------------------------------------------
//  Entidades
// ---------------------------------------------------------------------
static void dibujarRoca(int px, int py) {
    DrawCircleV(Vector2{ px + 16.0f, py + 17.0f }, 14.0f, rgb(70, 70, 76));
    DrawCircleV(Vector2{ px + 15.0f, py + 16.0f }, 12.5f, rgb(112, 112, 120));
    DrawCircleV(Vector2{ px + 11.0f, py + 11.0f }, 5.5f, rgb(160, 160, 170));
    DrawLineEx(Vector2{ px + 19.0f, py + 14.0f }, Vector2{ px + 24.0f, py + 21.0f }, 1.5f, rgb(70, 70, 76));
}
static void dibujarCorazonMapa(int px, int py, double t, unsigned h) {
    float flota = 1.5f * std::sin((float)t * 4.0f + (float)(h % 7));
    float cx = px + 16.0f, cy = py + 14.0f + flota;
    DrawCircleV(Vector2{ cx, cy + 3.0f }, 14.0f, rgb(255, 80, 80, 40));   // resplandor
    dibujarCorazon(cx, cy, ROJO_VIDA);
    DrawCircleV(Vector2{ cx - 5.0f, cy - 5.0f }, 2.0f, rgb(255, 255, 255, 200));   // brillo
}

static void dibujarEnemigo(int px, int py, bool rojo, int dirF, int dirC, double t, unsigned h) {
    float fase = (float)t * 6.0f + (float)(h % 7);
    Color cuerpo = rojo ? rgb(200, 55, 45) : rgb(60, 190, 70);
    Color claro = mezcla(cuerpo, rgb(255, 240, 160), 0.45f);
    Color oscuro = mezcla(cuerpo, rgb(0, 0, 0), 0.5f);

    Vector2 d = Vector2{ (float)dirC, (float)dirF };   // hacia donde mira
    Vector2 q = Vector2{ -d.y, d.x };                  // de lado a lado
    float cx = px + 16.0f, cy = py + 16.0f;

    const int N = 7;
    Vector2 p[N];
    float r[N];
    for (int i = 0; i < N; i++) {
        float avance = -12.0f + i * 3.6f;                 // de la cola (-12) a la cabeza (+9.6)
        float onda = 4.0f * std::sin(fase + i * 0.9f);    // ondulacion
        p[i] = Vector2{ cx + d.x * avance + q.x * onda, cy + d.y * avance + q.y * onda };
        r[i] = (i == N - 1) ? 6.0f : 2.5f + 3.0f * i / (float)(N - 1);
    }
    for (int i = 0; i < N; i++) DrawCircleV(Vector2{ p[i].x + 1.0f, p[i].y + 1.5f }, r[i], oscuro);
    for (int i = 0; i < N; i++) DrawCircleV(p[i], r[i], cuerpo);
    for (int i = 1; i < N - 1; i += 2) DrawCircleV(p[i], r[i] * 0.45f, claro);

    Vector2 cab = p[N - 1];
    Color lengua = rgb(230, 40, 60);
    Vector2 base = Vector2{ cab.x + d.x * 4.0f, cab.y + d.y * 4.0f };
    Vector2 punta = Vector2{ cab.x + d.x * 7.0f, cab.y + d.y * 7.0f };
    DrawLineEx(base, punta, 1.5f, lengua);
    DrawLineEx(punta, Vector2{ punta.x + d.x * 2.0f + q.x * 2.0f, punta.y + d.y * 2.0f + q.y * 2.0f }, 1.2f, lengua);
    DrawLineEx(punta, Vector2{ punta.x + d.x * 2.0f - q.x * 2.0f, punta.y + d.y * 2.0f - q.y * 2.0f }, 1.2f, lengua);

    for (int s = -1; s <= 1; s += 2) {
        Vector2 ojo = Vector2{ cab.x + d.x * 2.0f + q.x * 3.0f * s, cab.y + d.y * 2.0f + q.y * 3.0f * s };
        DrawCircleV(ojo, 2.2f, rgb(255, 255, 255));
        DrawCircleV(Vector2{ ojo.x + d.x * 0.7f, ojo.y + d.y * 0.7f }, 1.0f, rgb(20, 20, 20));
    }
}
static void dibujarJugador(int px, int py, int mirarF, int mirarC, bool herido, double t) {
    if (herido && ((int)(t * 14.0)) % 2 == 0) return;   // parpadea mientras es invulnerable
    Color camisa = herido ? rgb(235, 80, 80) : rgb(215, 170, 70);
    DrawRectangle(px + 11, py + 24, 4, 7, rgb(80, 52, 30));   // piernas
    DrawRectangle(px + 17, py + 24, 4, 7, rgb(80, 52, 30));
    DrawRectangle(px + 9, py + 15, 14, 11, camisa);           // cuerpo
    DrawRectangle(px + 9, py + 15, 14, 2, mezcla(camisa, rgb(255, 255, 255), 0.35f));
    DrawCircleV(Vector2{ px + 16.0f, py + 12.0f }, 7.0f, rgb(240, 200, 160));   // cabeza
    DrawRectangle(px + 5, py + 7, 22, 3, rgb(120, 70, 30));   // sombrero: ala
    DrawRectangle(px + 10, py + 2, 12, 6, rgb(140, 84, 36));  // sombrero: copa
    DrawRectangle(px + 10, py + 6, 12, 2, rgb(200, 150, 60));
    float ex = 2.0f * mirarC, ey = 1.5f * mirarF;
    DrawCircleV(Vector2{ px + 13.0f + ex, py + 12.5f + ey }, 1.4f, rgb(30, 20, 15));
    DrawCircleV(Vector2{ px + 19.0f + ex, py + 12.5f + ey }, 1.4f, rgb(30, 20, 15));
}

static void dibujarLatigo(int px, int py, bool horizontal) {
    Color cuero = rgb(176, 120, 60), brillo = rgb(235, 200, 140);
    if (horizontal) {
        DrawRectangle(px, py + 14, TILE, 4, cuero);
        DrawRectangle(px, py + 14, TILE, 1, brillo);
    }
    else {
        DrawRectangle(px + 14, py, 4, TILE, cuero);
        DrawRectangle(px + 14, py, 1, TILE, brillo);
    }
}

// Misma regla que Fisica::actualizarBotones: una puerta esta abierta si TODOS sus botones estan pisados
static void estadoPuertas(Nivel& nv, bool abierta[3]) {
    int total[3] = { 0, 0, 0 }, activos[3] = { 0, 0, 0 };
    for (int f = 0; f < MAP_HEIGHT; f++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            CellType m = nv.mapa.getValue(f, c);
            int col = (m == CellType::BUTTON) ? 0 : (m == CellType::BUTTON_G) ? 1 : (m == CellType::BUTTON_B) ? 2 : -1;
            if (col < 0) continue;
            total[col]++;
            CellType e = nv.ents.getValue(f, c);
            if (e == CellType::ROCK || e == CellType::PLAYER) activos[col]++;
        }
    }
    for (int i = 0; i < 3; i++) abierta[i] = (total[i] > 0 && activos[i] == total[i]);
}

static void dibujarNivel(Juego& j, double t) {
    Nivel& nv = *j.niveles[j.numeroNivel];
    bool puertas[3];
    estadoPuertas(nv, puertas);
    bool herido = nv.fisica.getInvulnerableTicks() > 0;

    for (int f = 0; f < MAP_HEIGHT; f++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            int px = c * TILE, py = HUD_H + f * TILE;
            CellType m = nv.mapa.getValue(f, c);
            CellType e = nv.ents.getValue(f, c);
            unsigned h = hash2(f, c);

            dibujarVacio(px, py, f, c);
            switch (m) {
            case CellType::WALL:     dibujarPared(px, py, f, c); break;
            case CellType::EXIT:     dibujarSalida(px, py, t); break;
            case CellType::BUSH:     dibujarArbusto(px, py, f, c); break;
            case CellType::BUTTON:   dibujarBoton(px, py, 0, e == CellType::ROCK || e == CellType::PLAYER); break;
            case CellType::BUTTON_G: dibujarBoton(px, py, 1, e == CellType::ROCK || e == CellType::PLAYER); break;
            case CellType::BUTTON_B: dibujarBoton(px, py, 2, e == CellType::ROCK || e == CellType::PLAYER); break;
            case CellType::DOOR:     dibujarPuerta(px, py, 0, puertas[0]); break;
            case CellType::DOOR_G:   dibujarPuerta(px, py, 1, puertas[1]); break;
            case CellType::DOOR_B:   dibujarPuerta(px, py, 2, puertas[2]); break;
            default: break;
            }
            switch (e) {
            case CellType::ROCK:    dibujarRoca(px, py); break;
            case CellType::DIAMOND: dibujarGema(px + 16.0f, py + 16.0f, (float)TILE, t, h); break;
            case CellType::HEART:   dibujarCorazonMapa(px, py, t, h); break;
            case CellType::PLAYER:  dibujarJugador(px, py, j.mirarF, j.mirarC, herido, t); break;
            case CellType::ENEMY:
            case CellType::ENEMY_H:
            case CellType::ENEMY_V: {
                int dF = 0, dC = 1;
                nv.fisica.direccionEnemigoEn(f, c, dF, dC);
                if (dF == 0 && dC == 0) dC = 1;   // si esta quieta, mira a la derecha
                dibujarEnemigo(px, py, e == CellType::ENEMY_V, dF, dC, t, h);
                break;
            }
            case CellType::WHIP_H:  dibujarLatigo(px, py, true); break;
            case CellType::WHIP_V:  dibujarLatigo(px, py, false); break;
            default: break;
            }
        }
    }
}

// ---------------------------------------------------------------------
//  HUD y pantallas
// ---------------------------------------------------------------------
static void dibujarHUD(Juego& j, double t) {
    DrawRectangleGradientV(0, 0, VW, HUD_H, rgb(70, 48, 32), rgb(38, 26, 18));
    DrawRectangle(0, HUD_H - 3, VW, 3, ORO);

    DrawText("VIDAS", 24, 10, 20, ORO_CLARO);
    int llenos = (j.game.getVide() + 9) / 10;
    llenos = std::max(0, std::min(10, llenos));
    for (int i = 0; i < 10; i++) {
        dibujarCorazon(34.0f + i * 26.0f, 50.0f, i < llenos ? ROJO_VIDA : rgb(70, 60, 58));
    }

    int restante = j.game.timeLeft();
    bool urgente = restante <= 30 && ((int)(t * 2.0)) % 2 == 0;
    DrawText("TIEMPO", 470, 10, 20, ORO_CLARO);
    DrawText(j.game.formatTime().c_str(), 470, 36, 36, urgente ? ROJO_VIDA : CREMA);

    DrawText("PUNTOS", 760, 10, 20, ORO_CLARO);
    DrawText(to_string(j.game.getScore()).c_str(), 760, 36, 36, CREMA);
    double edad = t - j.bonusInicio;
    if (edad >= 0.0 && edad < 0.9 && j.bonusValor > 0) {
        float k = (float)(edad / 0.9);   // 0 = recién ganado, 1 = desaparece
        int x = 760 + MeasureText(to_string(j.game.getScore()).c_str(), 36) + 14;
        int y = 40 - (int)(16.0f * k);   // sube mientras se desvanece
        DrawText(("+" + to_string(j.bonusValor)).c_str(), x, y, 30, Fade(rgb(120, 255, 120), 1.0f - k * k));
    }

    DrawText("NIVEL", 1060, 10, 20, ORO_CLARO);
    DrawText((to_string(j.numeroNivel) + "/" + to_string(TOTAL_NIVELES)).c_str(), 1060, 36, 36, CREMA);
}

static void dibujarFondoGemas(double t, int cantidad, float alfaOscuro) {
    DrawRectangleGradientV(0, 0, VW, VH, TIERRA_OSCURA, TIERRA);
    for (int i = 0; i < cantidad; i++) {
        unsigned h = hash2(i, 7);
        float x = 30.0f + (float)(h % 1220);
        float vel = 25.0f + (float)((h >> 8) % 50);
        float tam = 14.0f + (float)((h >> 16) % 22);
        float y = std::fmod((float)t * vel + (float)((h >> 4) % 900), (float)VH + 80.0f) - 40.0f;
        dibujarGema(x, y, tam, t, h);
    }
    DrawRectangle(0, 0, VW, VH, rgb(24, 16, 12, (int)(alfaOscuro * 255)));
}

static void dibujarTop5(const vector<Registro>& top, int cx, int y, int resaltar) {
    DrawRectangle(cx - 220, y, 440, 238, rgb(20, 13, 9, 200));
    DrawRectangle(cx - 220, y, 440, 3, ORO);
    textoCentrado("MEJORES PUNTAJES", cx, y + 14, 24, ORO_CLARO);
    for (int i = 0; i < 5; i++) {
        Color col = (i + 1 == resaltar) ? rgb(255, 235, 90) : CREMA;
        int yy = y + 56 + i * 34;
        if (i < (int)top.size()) {
            DrawText((to_string(i + 1) + ".  " + top[i].nombre).c_str(), cx - 195, yy, 26, col);
            string pts = to_string(top[i].puntaje);
            DrawText(pts.c_str(), cx + 195 - MeasureText(pts.c_str(), 26), yy, 26, col);
        }
        else {
            DrawText((to_string(i + 1) + ".  ---").c_str(), cx - 195, yy, 26, col);
        }
    }
}
static void dibujarMenu(Juego& j, double t) {
    dibujarFondoGemas(t, 28, 0.45f);
    dibujarArte(ARTE_TITULO, 5, VW / 2, 80, 16, rgb(190, 255, 100), rgb(30, 150, 60));
    float pulso = 0.6f + 0.4f * std::sin((float)t * 3.0f);
    if (j.escribiendoNombre) {
        textoCentrado("ESCRIBE TU NOMBRE", VW / 2, 215, 34, Fade(ORO_CLARO, pulso));
        DrawRectangle(VW / 2 - 200, 280, 400, 60, rgb(20, 13, 9, 220));
        DrawRectangleLines(VW / 2 - 200, 280, 400, 60, ORO);
        string mostrado = j.nombre + ((((int)(t * 2.0)) % 2 == 0) ? "_" : " ");
        textoCentrado(mostrado, VW / 2, 295, 36, CREMA);
        textoCentrado("ENTER para empezar     ESC para volver", VW / 2, 370, 22, ARENA);
    }
    else {
        textoCentrado("PRESIONA ENTER PARA JUGAR", VW / 2, 215, 34, Fade(ORO_CLARO, pulso));
        textoCentrado("ESC para salir", VW / 2, 262, 22, ARENA);
        dibujarTop5(j.top, VW / 2, 320, 0);
    }
    textoCentrado("WASD / Flechas: mover     E: latigo     P: pausa     R: reiniciar nivel     F11: pantalla completa",
        VW / 2, 664, 20, ARENA);
}
static void dibujarPausa(Juego& j, double t) {
    dibujarHUD(j, t);
    dibujarNivel(j, t);
    DrawRectangle(0, 0, VW, VH, rgb(0, 0, 0, 170));
    dibujarArte(ARTE_PAUSA, 5, VW / 2, 190, 18, rgb(255, 215, 90), rgb(210, 140, 30));
    float pulso = 0.6f + 0.4f * std::sin((float)t * 3.0f);
    textoCentrado("ENTER para continuar", VW / 2, 360, 32, Fade(ORO_CLARO, pulso));
    textoCentrado("ESC para volver al menu", VW / 2, 408, 22, ARENA);
}

static void dibujarFin(Juego& j, double t, bool victoria) {
    dibujarHUD(j, t);
    dibujarNivel(j, t);
    DrawRectangle(0, 0, VW, VH, rgb(0, 0, 0, 185));
    if (victoria) dibujarArte(ARTE_VICTORIA, 5, VW / 2, 50, 20, rgb(170, 255, 120), rgb(40, 160, 70));
    else dibujarArte(ARTE_GAMEOVER, 5, VW / 2, 50, 20, rgb(255, 120, 90), rgb(150, 25, 25));

    textoCentrado((victoria ? "PUNTAJE FINAL: " : "PUNTAJE OBTENIDO: ") + to_string(j.puntajeFinal), VW / 2, 180, 34, CREMA);
    textoCentrado(j.nombre, VW / 2, 150, 26, ORO_CLARO);
    if (j.posRecord > 0) {
        float pulso = 0.6f + 0.4f * std::sin((float)t * 5.0f);
        textoCentrado("NUEVO RECORD!  Posicion " + to_string(j.posRecord), VW / 2, 226, 28, Fade(rgb(255, 235, 90), pulso));
    }
    dibujarTop5(j.top, VW / 2, 280, j.posRecord);
    textoCentrado("ENTER para jugar otra vez     ESC para volver al menu", VW / 2, 548, 24, ARENA);
}

static void dibujar(Juego& j, double t) {
    switch (j.estado) {
    case GameState::MENU:     dibujarMenu(j, t); break;
    case GameState::PLAYING:
        ClearBackground(TIERRA_OSCURA);
        dibujarNivel(j, t);
        dibujarHUD(j, t);
        break;
    case GameState::PAUSE:    dibujarPausa(j, t); break;
    case GameState::VICTORY:  dibujarFin(j, t, true); break;
    case GameState::GAMEOVER: dibujarFin(j, t, false); break;
    }
}

// ---------------------------------------------------------------------
//  Programa principal
// ---------------------------------------------------------------------
static bool existeArchivo(const char* ruta) {
    std::ifstream f(ruta);
    return f.good();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(VW, VH, "Diamond Rush");
    SetExitKey(KEY_NULL);          // ESC no cierra la ventana: se usa para pausar y volver al menu
    SetTargetFPS(60);
    cargarSonidos();

    RenderTexture2D lienzo = LoadRenderTexture(VW, VH);
    SetTextureFilter(lienzo.texture, TEXTURE_FILTER_BILINEAR);

    const bool nivelesOk = existeArchivo("nivel1.txt");

    Juego* juego = new Juego();
    juego->top = leerTop5();

    while (!WindowShouldClose() && !juego->salir) {
        float dt = GetFrameTime();
        double t = GetTime();

        if (nivelesOk) {
            actualizar(*juego, dt);
            actualizarMusica(juego->estado);
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            juego->salir = true;
        }

        // Todo se dibuja en un lienzo fijo de 1280x720 y despues se escala a la ventana
        BeginTextureMode(lienzo);
        ClearBackground(TIERRA_OSCURA);
        if (nivelesOk) {
            dibujar(*juego, t);
        }
        else {
            textoCentrado("No se encontro nivel1.txt", VW / 2, 280, 40, ROJO_VIDA);
            textoCentrado("Ejecuta el juego desde la carpeta que tiene los niveles (nivel1.txt, nivel2.txt, nivel3.txt).", VW / 2, 340, 22, CREMA);
            textoCentrado("ESC para salir", VW / 2, 390, 22, ARENA);
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(NEGRO);
        float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
        float escala = std::min(sw / VW, sh / VH);
        Rectangle origen = { 0.0f, 0.0f, (float)lienzo.texture.width, -(float)lienzo.texture.height };
        Rectangle destino = { (sw - VW * escala) / 2.0f, (sh - VH * escala) / 2.0f, VW * escala, VH * escala };
        DrawTexturePro(lienzo.texture, origen, destino, Vector2{ 0.0f, 0.0f }, 0.0f, Color{ 255, 255, 255, 255 });
        EndDrawing();
    }

    delete juego;
    descargarSonidos();
    UnloadRenderTexture(lienzo);
    CloseWindow();
    return 0;
}
