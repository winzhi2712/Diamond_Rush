#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cctype>
#include <vector>
#include <ctime>
#include <utility>
#include "Constantes.h"

#include "Tipos.h"
#include "MapLoader.h"
#include "Matrix.h"
#include "Terminal.h"
#include "Render.h"
#include "Pantallas.h"
#include "KeyMap.h"
#include "Game.h"
#include "Fisica.h"
#include "HUD.h"
#include "Puntajes.h"
#include "Benchmark.h"

using namespace std;
using namespace std::chrono;

const int TOTAL_NIVELES = 3;
const bool MODO_DEV = true;   // true = teclas 1, 2 y 3 saltan de nivel; PONERLO en false antes de entregar

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // Configurar consola en UTF-8 para Windows
#endif

    Terminal terminal;
    terminal.init();
    srand(static_cast<unsigned>(time(nullptr)));
    Game game;
    KeyMap keyMap;
    Puntajes puntajes;   // Top 5 de puntajes, guardado en puntajes.txt
    string nombreJugador = "JUGADOR";   // provisorio: en el Paso 7 se pide al jugador antes de empezar
    const int FILA_CARTEL = 12;   // fila donde van NUEVO RECORD / ENTRAS AL TOP

    // Pantalla de resultado (victoria y game over): registra el puntaje, muestra el cartel y el top 5.
    // Devuelve la posición lograda (0 si no entró al top).
    auto mostrarResultado = [&](const string& etiquetaPuntaje) -> int {
        int posicion = puntajes.agregar(nombreJugador, game.getScore());

        terminal.gotoxy(0, 9);   // fila 10 de la pantalla
        cout << string(MARGEN_IZQ, ' ') << nombreJugador << " - " << etiquetaPuntaje << game.getScore();

        if (posicion == 1) {
            Pantallas::nuevoRecord(0, FILA_CARTEL);
        }
        else if (posicion > 1) {
            Pantallas::entrasAlTop(posicion, FILA_CARTEL);
        }

        terminal.gotoxy(0, FILA_CARTEL + 5);   // el top 5 va debajo del cartel
        puntajes.imprimir(posicion);
        cout << flush;
        return posicion;
        };
 

    GameState estadoActual = GameState::MENU;

    // Cada nivel conserva su propio estado para poder ir y volver entre mapas
    Render* renders[TOTAL_NIVELES + 1] = { nullptr };
    Fisica* fisicas[TOTAL_NIVELES + 1] = { nullptr };
    vector<pair<int, int>> gemasPorNivel[TOTAL_NIVELES + 1];   // gemas ya recolectadas en cada nivel
    Render* render = nullptr;   // nivel que se está jugando ahora
    Fisica* fisica = nullptr;

    bool ejecutandoJuego = true;
    int numeroNivel = 1;

    // Construye (o reconstruye desde cero) el nivel n leyendo nivelN.txt
    auto construirNivel = [&](int n) {
        GameMap datos = MapLoader::cargarNivel("nivel" + to_string(n) + ".txt");

        if (fisicas[n] != nullptr) { delete fisicas[n]; fisicas[n] = nullptr; }
        if (renders[n] != nullptr) { delete renders[n]; renders[n] = nullptr; }

        renders[n] = new Render(MAP_HEIGHT, MAP_WIDTH);
        for (int r = 0; r < MAP_HEIGHT; r++) {
            for (int c = 0; c < MAP_WIDTH; c++) {
                renders[n]->getMapBuffer().setValue(r, c, datos.grid[r][c]);
                renders[n]->getEntitiesBuffer().setValue(r, c, datos.entities[r][c]);
                renders[n]->getWallGlyphBuffer().setValue(r, c, datos.glyphs[r][c]);
            }
        }

        fisicas[n] = new Fisica(renders[n]->getMapBuffer(), renders[n]->getEntitiesBuffer());
        // colocarJugador recibe (fila, columna)
        fisicas[n]->colocarJugador(datos.playerY, datos.playerX);

        for (const auto& ene : datos.enemigos) {
            fisicas[n]->agregarEnemigo(ene.fila, ene.col, ene.dirFila, ene.dirCol, ene.ticks, ene.persigue, ene.tipoCelda);
        }

        gemasPorNivel[n].clear();
        };

    // Cambia el nivel que se está jugando (el nivel ya debe estar construido)
    auto cambiarANivel = [&](int n) {
        numeroNivel = n;
        render = renders[n];
        fisica = fisicas[n];
        terminal.clearTerminal();
        render->forceFullRedraw();
        };

    // Libera todos los niveles (al empezar una partida nueva o al salir)
    auto liberarNiveles = [&]() {
        for (int i = 1; i <= TOTAL_NIVELES; i++) {
            if (fisicas[i] != nullptr) { delete fisicas[i]; fisicas[i] = nullptr; }
            if (renders[i] != nullptr) { delete renders[i]; renders[i] = nullptr; }
            gemasPorNivel[i].clear();
        }
        render = nullptr;
        fisica = nullptr;
        };

    Benchmark benchmark;
    int frameActual = 0;
    const bool MODO_BENCHMARK = false; //Poner en true para activar el modo benchmark
    const int FRAMES_BENCHAMRK = 500;

    // No se arranca hasta que la terminal sea lo bastante grande (en benchmark se omite)
    if (!MODO_BENCHMARK) {
        while (ejecutandoJuego && !terminal.checkMinSize()) {
            terminal.clearTerminal();
            cout << "La terminal es muy pequena." << endl;
            cout << "Se necesita al menos " << MIN_TERMINAL_WIDTH << " columnas x "
                << MIN_TERMINAL_HEIGHT << " filas." << endl;
            cout << "Agrandala y el juego continuara solo. (ESC para salir)" << endl;

            for (int i = 0; i < 10 && ejecutandoJuego; i++) {   // espera ~0.5 s revisando ESC
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                if (tecla != 0 && keyMap.getMov(tecla) == Mov::EXIT) {
                    ejecutandoJuego = false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }

    if (MODO_BENCHMARK) {
        benchmark.startBench(false);
        game.reset();
        liberarNiveles();
        construirNivel(1);
        cambiarANivel(1);
        estadoActual = GameState::PLAYING;
    }
    auto medirT = [](auto&& fn) {
        auto start = std::chrono::steady_clock::now();
        fn();
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::micro>(end - start).count();
    };
    while (ejecutandoJuego) {

        // ==========================================
        // ESTADO: MENÚ PRINCIPAL
        // ==========================================
        if (estadoActual == GameState::MENU) {
            Pantallas::menuPrincipal();
            puntajes.dibujarTop(MENU_TOP_FILA, MENU_TOP_COL);

            int seleccion = 0;        // 0 = JUGAR, 1 = SALIR
            bool brillante = true;
            Pantallas::menuOpciones(seleccion, brillante);

            LluviaMenu lluvia;
            int cuentaLluvia = 0;
            int cuentaParpadeo = 0;

            bool enMenu = true;
            while (enMenu && ejecutandoJuego) {
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                while (tecla != 0 && enMenu) {
                    Mov mov = keyMap.getMov(tecla);

                    if (mov == Mov::UP || mov == Mov::DOWN) {
                        seleccion = (mov == Mov::UP) ? 0 : 1;
                        brillante = true;          // al moverte, la opción se ve brillante de inmediato
                        cuentaParpadeo = 0;
                        Pantallas::menuOpciones(seleccion, brillante);
                    }
                    else if (mov == Mov::YES) {
                        if (seleccion == 0) {
                            estadoActual = GameState::NAME_INPUT;
                        }
                        else {
                            ejecutandoJuego = false;
                        }
                        enMenu = false;
                    }
                    else if (mov == Mov::EXIT) {
                        ejecutandoJuego = false;
                        enMenu = false;
                    }
                    tecla = terminal.obtenerSiguiente();
                }

                if (enMenu) {
                    cuentaLluvia++;
                    if (cuentaLluvia >= 5) {        // ~0.15 s: cada diamante baja una fila
                        cuentaLluvia = 0;
                        Pantallas::menuLluvia(lluvia);
                    }
                    cuentaParpadeo++;
                    if (cuentaParpadeo >= 16) {     // ~0.5 s: la opción elegida cambia de brillo
                        cuentaParpadeo = 0;
                        brillante = !brillante;
                        Pantallas::menuOpciones(seleccion, brillante);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        // ==========================================
            // ESTADO: INGRESO DE NOMBRE
        // ==========================================
        else if (estadoActual == GameState::NAME_INPUT) {
            const int MAX_NOMBRE = 10;
            string nombre = "";
            bool cursorVisible = true;
            int ticksParpadeo = 0;

            Pantallas::pedirNombre();
            Pantallas::campoNombre(nombre, cursorVisible);
            terminal.limpiarBuffer();   // descarta el Enter con el que se llegó aquí

            bool enNombre = true;
            while (enNombre && ejecutandoJuego) {
                terminal.pollInputs();
                bool redibujar = false;

                int tecla = terminal.obtenerSiguiente();
                while (tecla != 0 && enNombre) {
                    Mov mov = keyMap.getMov(tecla);

                    if (mov == Mov::YES) {
                        if (nombre.empty()) {
                            Pantallas::avisoNombre(true);
                        }
                        else {
                            nombreJugador = nombre;
                            game.reset();
                            liberarNiveles();
                            construirNivel(1);
                            cambiarANivel(1);
                            estadoActual = GameState::PLAYING;
                            enNombre = false;
                        }
                    }
                    else if (mov == Mov::EXIT) {
                        estadoActual = GameState::MENU;
                        enNombre = false;
                    }
                    else if (mov == Mov::BORRAR) {
                        if (!nombre.empty()) {
                            nombre.pop_back();
                            redibujar = true;
                        }
                    }
                    else if (tecla < 128 && isalnum(tecla) && (int)nombre.size() < MAX_NOMBRE) {
                        nombre += (char)toupper(tecla);
                        redibujar = true;
                        Pantallas::avisoNombre(false);
                    }
                    tecla = terminal.obtenerSiguiente();
                }

                if (enNombre) {
                    if (redibujar) {
                        cursorVisible = true;     // al escribir, el cursor se ve de inmediato
                        ticksParpadeo = 0;
                    }
                    else {
                        ticksParpadeo++;
                        if (ticksParpadeo >= 16) {   // ~0.5 s a 30 ms por vuelta
                            ticksParpadeo = 0;
                            cursorVisible = !cursorVisible;
                            redibujar = true;
                        }
                    }
                    if (redibujar) Pantallas::campoNombre(nombre, cursorVisible);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        // ==========================================
        // ESTADO: JUGANDO
        // ==========================================
        else if (estadoActual == GameState::PLAYING) {
            benchmark.startFrame(frameActual);
            Mov movimiento = Mov::VACIA;
            double tInput = medirT([&]() {
                if (benchmark.estaSilencio()) {
                    static const Mov secuencia[] = {
                        Mov::RIGHT, Mov::VACIA, Mov::DOWN, Mov::VACIA, Mov::LEFT, Mov::VACIA
                    };
                    movimiento = secuencia[frameActual % 6];
                }
                else {
                    terminal.pollInputs();
                    int tecla = terminal.obtenerSiguiente();
                    terminal.limpiarBuffer();   // evita que se acumulen teclas y el personaje siga moviéndose

                    if (tecla != 0) {
                        // Atajo de desarrollo: las teclas 1, 2 y 3 saltan directo a ese nivel
                        if (MODO_DEV && tecla >= '1' && tecla <= '0' + TOTAL_NIVELES) {
                            int destino = tecla - '0';
                            if (fisicas[destino] == nullptr) construirNivel(destino);
                            cambiarANivel(destino);
                            return;
                        }
                        movimiento = keyMap.getMov(tecla);

                        if (movimiento == Mov::PAUSE) {
                            game.pausar();
                            estadoActual = GameState::PAUSE;
                            movimiento = Mov::VACIA;
                            return;
                        }
                        else if (movimiento == Mov::EXIT) {
                            ejecutandoJuego = false;
                            movimiento = Mov::VACIA;
                            return;
                        }
                        // Reiniciar el nivel actual con la tecla R
                        else if (movimiento == Mov::RESTART) {
                            vector<pair<int, int>> gemas = gemasPorNivel[numeroNivel];   // copia, porque construirNivel limpia la lista
                            construirNivel(numeroNivel);
                            gemasPorNivel[numeroNivel] = gemas;

                            // Borrar las gemas que el jugador ya había agarrado en este nivel
                            for (const auto& gema : gemas) {
                                renders[numeroNivel]->getEntitiesBuffer().setValue(gema.first, gema.second, CellType::EMPTY);
                            }
                            cambiarANivel(numeroNivel);
                            movimiento = Mov::VACIA;
                            return;
                        }
                    }
                }
            });
            benchmark.registerInput(tInput);
            if (!ejecutandoJuego || estadoActual != GameState::PLAYING) {
                benchmark.endFrame();
                continue;
            }
            // 1. Motor de física
            ResultadoTick resultado;
            double tFisica = medirT([&]() {
                resultado = fisica->actualizar(movimiento);
                });
            benchmark.registerFisica(tFisica);

            // 2. Puntos por gemas (se recuerda su posición para que no reaparezcan al reiniciar)
            if (resultado.puntosGanados > 0) {
                game.addScore(resultado.puntosGanados);
                gemasPorNivel[numeroNivel].push_back({ fisica->getFilaJugador(), fisica->getColJugador() });
            }

            // 3. Daño recibido
            if (resultado.dano > 0) {
                game.perderVida(resultado.dano);
            }
            // 3b. Corazones: curan, o dan puntos si la vida ya está llena
            if (resultado.corazones > 0) {
                if (game.vidaLlena()) {
                    game.addScore(puntosCorazonLleno);
                }
                else {
                    game.curar(curacionCorazon);
                }
                // Se guarda la posición para que no reaparezca al reiniciar con R
                gemasPorNivel[numeroNivel].push_back({ fisica->getFilaJugador(), fisica->getColJugador() });
            }
            // 3c. Enemigos aplastados por rocas: +30 cada uno
            if (resultado.enemigosMuertos > 0) {
                game.addScore(resultado.enemigosMuertos * puntosEnemigo);
            }

            // 4. Derrota
            if (game.perder() || game.getVide() <= 0) {
                estadoActual = GameState::GAMEOVER;
                continue;
            }

            // 5. Cambio de nivel (ir adelante o regresar al mapa anterior)
            if (resultado.nivelGanado) {
                if (numeroNivel < TOTAL_NIVELES) {
                    int siguiente = numeroNivel + 1;
                    if (fisicas[siguiente] == nullptr) construirNivel(siguiente);
                    cambiarANivel(siguiente);   // el reloj sigue corriendo: 5 minutos para los tres niveles
                }
                else {
                    game.addScore(game.timeLeft() * 5); // bono por el tiempo que sobró, solo al terminar todo
                    estadoActual = GameState::VICTORY;
                }
                continue;
            }
            if (resultado.volverAtras && numeroNivel > 1) {
                cambiarANivel(numeroNivel - 1);
                continue;
            }

            // La carita se pone roja mientras dura la invulnerabilidad tras recibir daño
            render->setJugadorHerido(fisica->getInvulnerableTicks() > 0);
            // Las puertas cambian de símbolo según estén abiertas o cerradas
            for (int color = 0; color < 3; color++) {
                render->setPuertaAbierta(color, fisica->getPuertaAbierta(color));
            }
            // 6. HUD
            GameStats stats;
            stats.lives = game.getVide();
            stats.score = game.getScore();
            stats.timeRemaining = game.timeLeft();
            stats.currentLevel = numeroNivel;

            HUD hud(render->getHUDBuffer());
            double tHud = medirT([&]() {
                hud.draw(stats);
                });
            benchmark.registerHud(tHud);

            // 7. Renderizado diferencial
            double tRender = medirT([&]() {
                render->combineBuffers();
                if (!benchmark.estaSilencio()) {
                    render->renderDifferential();
                }
                });
            benchmark.registerRender(tRender);
            benchmark.endFrame();
            frameActual++;

            if (benchmark.estaOn() && frameActual >= FRAMES_BENCHAMRK) {
                benchmark.stopBench();
                ejecutandoJuego = false;
                continue;
            }
            if (!benchmark.estaSilencio()) {
                // Ritmo del juego según los ticks por segundo de Fisica.h
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / ticksPorSegundo));
            }
        }

        // ==========================================
        // ESTADO: PAUSA
        // ==========================================
        else if (estadoActual == GameState::PAUSE) {
            terminal.clearTerminal();
            Pantallas::pause();
            cout << flush;

            bool enPausa = true;
            while (enPausa && ejecutandoJuego) {
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                if (tecla != 0) {
                    Mov mov = keyMap.getMov(tecla);
                    if (mov == Mov::YES) {
                        game.reanudar();
                        render->forceFullRedraw();
                        estadoActual = GameState::PLAYING;
                        enPausa = false;
                        terminal.clearTerminal();
                    }
                    else if (mov == Mov::EXIT) {
                        estadoActual = GameState::MENU;
                        enPausa = false;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
        // ==========================================
        // ESTADO: VICTORIA 
        // ==========================================
        else if (estadoActual == GameState::VICTORY) {
            terminal.clearTerminal();
            Pantallas::victory();
            int posicion = mostrarResultado("PUNTAJE FINAL: ");

            int faseColor = 0;
            int cuentaColor = 0;
            bool enVictoria = true;
            while (enVictoria && ejecutandoJuego) {
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                if (tecla != 0) {
                    Mov mov = keyMap.getMov(tecla);
                    if (mov == Mov::YES || mov == Mov::EXIT) {
                        estadoActual = GameState::MENU;
                        enVictoria = false;
                    }
                }

                // Animación de NUEVO RECORD: los colores se desplazan cada ~0.25 s
                if (enVictoria && posicion == 1) {
                    cuentaColor++;
                    if (cuentaColor >= 8) {
                        cuentaColor = 0;
                        faseColor++;
                        Pantallas::nuevoRecord(faseColor, FILA_CARTEL);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        // ==========================================
// ESTADO: GAME OVER
// ==========================================
        else if (estadoActual == GameState::GAMEOVER) {
            terminal.clearTerminal();
            Pantallas::gameOver();
            int posicion = mostrarResultado("PUNTAJE OBTENIDO: ");

            int faseColor = 0;
            int cuentaColor = 0;
            bool enGameOver = true;
            while (enGameOver && ejecutandoJuego) {
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                if (tecla != 0) {
                    Mov mov = keyMap.getMov(tecla);
                    if (mov == Mov::YES || mov == Mov::EXIT) {
                        estadoActual = GameState::MENU;
                        enGameOver = false;
                    }
                }

                if (enGameOver && posicion == 1) {
                    cuentaColor++;
                    if (cuentaColor >= 8) {
                        cuentaColor = 0;
                        faseColor++;
                        Pantallas::nuevoRecord(faseColor, FILA_CARTEL);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
    }

    if (MODO_BENCHMARK) {
        benchmark.exportCVS("benchmark.csv");
    }
    // Limpieza de memoria dinámica al salir
    liberarNiveles();

    return 0;
}