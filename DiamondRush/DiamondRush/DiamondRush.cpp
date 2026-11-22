#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
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
#include "NombreConsola.h" 


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
    Game game;
    KeyMap keyMap;
    Puntajes puntajes;   // Top 5 de puntajes, guardado en puntajes.txt

    if (!terminal.checkMinSize()) {
        cout << "Advertencia: La terminal es muy pequeña. Se recomienda al menos "
            << MIN_TERMINAL_WIDTH << "x" << MIN_TERMINAL_HEIGHT << "." << endl;
    }

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
    int popupTicks = 0;    // cuánto falta para que desaparezca el "+10"
    int popupValor = 0;
    const bool MODO_BENCHMARK = false; //Poner en true para activar el modo benchmark
    const int FRAMES_BENCHAMRK = 500;

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
            terminal.clearTerminal();
            Pantallas::mainTitle();
            cout << "\n\n";
            puntajes.imprimir();
            cout << flush;

            bool enMenu = true;
            while (enMenu && ejecutandoJuego) {
                terminal.pollInputs();
                int tecla = terminal.obtenerSiguiente();
                if (tecla != 0) {
                    Mov mov = keyMap.getMov(tecla);
                    if (mov == Mov::YES) {
                        string nombre = pedirNombre(terminal);
                        if (nombre.empty()) {
                            // ESC: vuelve a dibujar el menú
                            enMenu = false;
                        }
                        else {
                            puntajes.setNombre(nombre);
                            game.reset();
                            liberarNiveles();
                            construirNivel(1);
                            cambiarANivel(1);
                            popupTicks = 0;
                            estadoActual = GameState::PLAYING;
                            enMenu = false;
                        }
                    }
                    else if (mov == Mov::EXIT) {
                        ejecutandoJuego = false;
                        enMenu = false;
                    }
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

            // 2. Puntos (gemas, enemigos eliminados, corazón con vida llena) y curación
            int ganado = resultado.puntosGanados + resultado.puntosExtra;
            if (resultado.curacion > 0) {
                if (game.getVide() >= 100) ganado += puntosCorazonLleno;
                else game.curar(resultado.curacion);
            }
            if (ganado > 0) {
                game.addScore(ganado);
                if (popupTicks > 0) popupValor += ganado; else popupValor = ganado;
                popupTicks = 20;   // unos 0,8 segundos
            }
            // Se recuerda la posición para que gemas y corazones no reaparezcan al reiniciar con R
            if (resultado.puntosGanados > 0 || resultado.curacion > 0) {
                gemasPorNivel[numeroNivel].push_back({ fisica->getFilaJugador(), fisica->getColJugador() });
            }

            // 3. Daño recibido
            if (resultado.dano > 0) {
                game.perderVida(resultado.dano);
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

            // 6. HUD
            GameStats stats;
            stats.lives = game.getVide();
            stats.score = game.getScore();
            stats.timeRemaining = game.timeLeft();
            stats.currentLevel = numeroNivel;
            stats.bonus = (popupTicks > 0) ? popupValor : 0;
            if (popupTicks > 0) popupTicks--;

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
        // ESTADO: VICTORIA (se completaron los 3 niveles)
        // ==========================================
        else if (estadoActual == GameState::VICTORY) {
            terminal.clearTerminal();
            Pantallas::victory();

            int posicion = puntajes.agregar(game.getScore());
            terminal.gotoxy(MARGEN_IZQ, 7 + MARGEN_SUP);
            cout << "PUNTAJE FINAL: " << game.getScore() << endl;
            if (posicion > 0) cout << string(MARGEN_IZQ, ' ') << "NUEVO RECORD! Posicion " << posicion << endl;
            cout << endl;
            puntajes.imprimir(posicion);
            cout << flush;

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
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        // ==========================================
        // ESTADO: GAME OVER
        // ==========================================
        else if (estadoActual == GameState::GAMEOVER) {
            terminal.clearTerminal();
            Pantallas::gameOver();

            int posicion = puntajes.agregar(game.getScore());
            terminal.gotoxy(MARGEN_IZQ, 7 + MARGEN_SUP);
            cout << "PUNTAJE OBTENIDO: " << game.getScore() << endl;
            if (posicion > 0) cout << string(MARGEN_IZQ, ' ') << "NUEVO RECORD! Posicion " << posicion << endl;
            cout << endl;
            puntajes.imprimir(posicion);
            cout << flush;

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