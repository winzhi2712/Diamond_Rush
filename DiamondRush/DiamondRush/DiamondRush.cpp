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

using namespace std;
using namespace std::chrono;

const int TOTAL_NIVELES = 3;

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // Configurar consola en UTF-8 para Windows
#endif

    Terminal terminal;
    terminal.init();
    Game game;
    KeyMap keyMap;
    Puntajes puntajes;   // Top 5 de puntajes, guardado en puntajes.txt

    // Coordenadas de las gemas ya recolectadas en el nivel actual
    vector<pair<int, int>> gemasRecolectadasSession;

    if (!terminal.checkMinSize()) {
        cout << "Advertencia: La terminal es muy pequeña. Se recomienda al menos "
            << MIN_TERMINAL_WIDTH << "x" << MIN_TERMINAL_HEIGHT << "." << endl;
    }

    GameState estadoActual = GameState::MENU;

    GameMap nivelActual;
    Render* render = nullptr;
    Fisica* fisica = nullptr;

    bool ejecutandoJuego = true;
    int numeroNivel = 1;

    // Carga el nivel indicado por numeroNivel (nivel1.txt, nivel2.txt, nivel3.txt)
    auto cargarNivelActual = [&]() {
        string ruta = "nivel" + to_string(numeroNivel) + ".txt";
        nivelActual = MapLoader::cargarNivel(ruta);

        if (render != nullptr) delete render;
        render = new Render(MAP_HEIGHT, MAP_WIDTH);

        for (int r = 0; r < MAP_HEIGHT; r++) {
            for (int c = 0; c < MAP_WIDTH; c++) {
                render->getMapBuffer().setValue(r, c, nivelActual.grid[r][c]);
                render->getEntitiesBuffer().setValue(r, c, nivelActual.entities[r][c]);
                render->getWallGlyphBuffer().setValue(r, c, nivelActual.glyphs[r][c]);
            }
        }

        if (fisica != nullptr) delete fisica;
        fisica = new Fisica(render->getMapBuffer(), render->getEntitiesBuffer());
        // colocarJugador recibe (fila, columna)
        fisica->colocarJugador(nivelActual.playerY, nivelActual.playerX);

        for (const auto& ene : nivelActual.enemigos) {
            fisica->agregarEnemigo(ene.fila, ene.col, ene.dirFila, ene.dirCol, ene.ticks, ene.persigue, ene.tipoCelda);
        }

        terminal.clearTerminal();
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
                        game.reset();
                        numeroNivel = 1;
                        gemasRecolectadasSession.clear();
                        cargarNivelActual();

                        estadoActual = GameState::PLAYING;
                        enMenu = false;
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
            terminal.pollInputs();
            int tecla = terminal.obtenerSiguiente();
            terminal.limpiarBuffer();   // evita que se acumulen teclas y el personaje siga moviéndose

            Mov movimiento = Mov::VACIA;
            if (tecla != 0) {
                movimiento = keyMap.getMov(tecla);

                if (movimiento == Mov::PAUSE) {
                    game.pausar();
                    estadoActual = GameState::PAUSE;
                    continue;
                }
                else if (movimiento == Mov::EXIT) {
                    ejecutandoJuego = false;
                    continue;
                }
                // Reiniciar el nivel actual con la tecla R
                else if (movimiento == Mov::RESTART) {
                    cargarNivelActual();

                    // Borrar las gemas que el jugador ya había agarrado en este nivel
                    for (const auto& gema : gemasRecolectadasSession) {
                        render->getEntitiesBuffer().setValue(gema.first, gema.second, CellType::EMPTY);
                    }
                    continue;
                }
            }

            // 1. Motor de física
            ResultadoTick resultado = fisica->actualizar(movimiento);

            // 2. Puntos por gemas (se recuerda su posición para que no reaparezcan al reiniciar)
            if (resultado.puntosGanados > 0) {
                game.addScore(resultado.puntosGanados);
                gemasRecolectadasSession.push_back({ fisica->getFilaJugador(), fisica->getColJugador() });
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

            // 5. Nivel completado
            if (resultado.nivelGanado) {
                if (numeroNivel < TOTAL_NIVELES) {
                    numeroNivel++;
                    gemasRecolectadasSession.clear();   // las posiciones de las gemas son de otro nivel
                    cargarNivelActual();                // el reloj sigue corriendo: 5 minutos para los tres niveles
                }
                else {
                    game.addScore(game.timeLeft() * 5); // bono por el tiempo que sobró, solo al terminar todo
                    estadoActual = GameState::VICTORY;
                }
                continue;
            }

            // 6. HUD
            GameStats stats;
            stats.lives = game.getVide();
            stats.score = game.getScore();
            stats.timeRemaining = game.timeLeft();
            stats.currentLevel = numeroNivel;

            HUD hud(render->getHUDBuffer());
            hud.draw(stats);

            // 7. Renderizado diferencial
            render->combineBuffers();
            render->renderDifferential();

            // Ritmo del juego según los ticks por segundo de Fisica.h
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / ticksPorSegundo));
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
            terminal.gotoxy(0, 7);
            cout << "PUNTAJE FINAL: " << game.getScore() << endl;
            if (posicion > 0) cout << "NUEVO RECORD! Posicion " << posicion << endl;
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
            terminal.gotoxy(0, 7);
            cout << "PUNTAJE OBTENIDO: " << game.getScore() << endl;
            if (posicion > 0) cout << "NUEVO RECORD! Posicion " << posicion << endl;
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

    // Limpieza de memoria dinámica al salir
    if (render != nullptr) delete render;
    if (fisica != nullptr) delete fisica;

    return 0;
}