#pragma once

// Mikel estuvo aquí y en sus corazones.
// ¿Qué es esto? pues maneja colisiones, movimiento del jugador, 
// física de piedras, puertas, botones y enemigos por cada tick.

#include <cstdlib>
#include "Matrix.h"
#include "Tipos.h"
#include "KeyMap.h"

//CONSTANTES DE CONFIGURACIÓN
const int ticksPorSegundo = 25; // Velocidad del bucle del juego 
const int ticksInvulnerable = 1 * ticksPorSegundo; // Duración de la invulnerabilidad tras recibir daño
const int contactoDolor = 10; //Daño que resta tocar a un enemigo
const int danoRoca = 20;      // Daño que resta que te caiga una roca encima
const int puntosGema = 10;   // Puntos que se suman al recoger un diamante/gema
const int maxEnemigos = 16;               // Número máximo de enemigos permitidos en simultáneo

// ESTRUCTURA DE RESULTADOS
//Objeto que la fisica retorna al bucle prinipal después de procesar cada tick
struct ResultadoTick {
    int puntosGanados = 0;    // Acumulador de puntos obtenidos en este tick
    int dano = 0;// Daño acumulado que el jugador debe recibir en este tic
    bool nivelGanado = false; // Bandera booleana que indica si el jugador llegó a la salida
};

//ESTRUCTURA DE ENEMIGO
// Define el estado y comportamiento individual de cada enemigo en el mapa.
struct Enemigo {
    int fila = 0;
    int col = 0;
    int dirFila = 0;        // Dirección actual en filas (-1 arriba, 1 abajo, 0 quieto)
    int dirCol = 0;         // Dirección actual en columnas (-1 izq, 1 der, 0 quieto)
    int ticksEntreMov = 6;  // Frecuencia de movimiento fluido (ej. cada 6 ticks)
    int contador = 6;       // Temporizador interno que cuenta regresivamente hasta el siguiente movimiento
    bool persigue = false;  // false = patrulla en línea recta; true = persigue al jugador
    CellType tipo = CellType::ENEMY_H; // Tipo de enemigo (ENEMY_H horizontal, ENEMY_V vertical)
};

class Fisica {
private:
    // Referencias a las dos matrices principales del juego (Capa fija y Capa movil)
    Matrix<CellType>& mapa; // Capa fija: Contiene paredes, salidas, botones y puertas
    Matrix<CellType>& ents; // Capa movil: Contiene rocas, gemas, jugador y enemigos
    Matrix<bool> rocaCayendo; // true si esa roca se movió hacia abajo en el tick anterior
    int filaJugador;
    int colJugador;
    int invulnerableTicks;   // Contador regresivo de protección contra daño del jugador
    bool puertasAbiertas;    // Estado global de las puertas (true = abiertas, false = cerradas)
    Enemigo enemigos[maxEnemigos]; // Arreglo estricto para almacenar todos los enemigos
    int numEnemigos;                 // Contador actual de enemigos registrados

    // Verifica si unas coordenadas (f, c) se encuentran dentro de los límites válidos del mapa.
    bool dentro(int f, int c) {
        return f >= 0 && f < mapa.getRows() && c >= 0 && c < mapa.getColumns();
    }
    // Obtiene el tipo de celda fija de forma segura. Si está fuera, se comporta como pared.
    CellType mapaEn(int f, int c) {
        if (!dentro(f, c))
            return CellType::WALL;
        return mapa.getValue(f, c);
    }

    // Obtiene el tipo de entidad móvil de forma segura.
    CellType entEn(int f, int c) {
        if (!dentro(f, c))
            return CellType::WALL;
        return ents.getValue(f, c);
    }
    // Coloca una entidad en la capa móvil asegurándose de no salir del mapa.
    void ponerEnt(int f, int c, CellType t) {
        if (dentro(f, c))
            ents.setValue(f, c, t);
    }

    // Determina si un elemento fijo del mapa bloquea el paso (Pared o Puerta cerrada).
    bool bloqueaMapa(CellType t) {
        return t == CellType::WALL || (t == CellType::DOOR && !puertasAbiertas);
    }
    // Regla crítica: Comprueba si una celda está totalmente libre para que una roca o enemigo entren.
    bool libreParaMover(int f, int c) {
        if (!dentro(f, c))
            return false;
        CellType m = mapaEn(f, c);
        // Las rocas y enemigos NO pueden pasar si hay una pared, una puerta cerrada, un arbusto o una entidad
        return !bloqueaMapa(m) && m != CellType::BUSH && m != CellType::EXIT && entEn(f, c) == CellType::EMPTY;
    }
    // Aplica daño al jugador siempre y cuando su tiempo de invulnerabilidad haya expirado.
    void recibirDano(ResultadoTick& res, int cantidad = contactoDolor) {
        if (invulnerableTicks > 0)
            return; // Si aún está parpadeando protegido, ignora el daño
        res.dano += cantidad;
        invulnerableTicks = ticksInvulnerable; // Reinicia el temporizador de invulnerabilidad
    }

    // Procesa el intento de movimiento del jugador según la dirección recibida (df, dc).
    void moverJugador(int df, int dc, ResultadoTick& res) {
        int nf = filaJugador + df;
        int nc = colJugador + dc;
        if (!dentro(nf, nc))
            return;

        CellType m = mapaEn(nf, nc);
        if (bloqueaMapa(m))
            return; // Si es pared o puerta cerrada, no pasa

        CellType e = entEn(nf, nc);

        // Caso A: Choca con un enemigo
        if (e == CellType::ENEMY_H || e == CellType::ENEMY_V || e == CellType::ENEMY) {
            recibirDano(res);
            return;
        }

        // Caso B: Choca con una roca (intenta empujar)
        if (e == CellType::ROCK) {
            if (df != 0) return; // Solo horizontal
            if (!libreParaMover(nf, nc + dc))
                return;
            ponerEnt(nf, nc + dc, CellType::ROCK);
            rocaCayendo.setValue(nf, nc, false);   // la roca empujada ya no está en caída
        }
        // Caso C: Choca con una gema/diamante
        else if (e == CellType::DIAMOND) {
            res.puntosGanados += puntosGema;
            ponerEnt(nf, nc, CellType::EMPTY);
        }

        // SI EL JUGADOR CAMINA SOBRE UN ARBUSTO: Lo destruye (convierte en vacío en el mapa fijo)
        if (m == CellType::BUSH) {
            mapa.setValue(nf, nc, CellType::EMPTY);
        }

        // Actualiza las posiciones de entidades móviles
        ponerEnt(filaJugador, colJugador, CellType::EMPTY);
        ponerEnt(nf, nc, CellType::PLAYER);
        filaJugador = nf;
        colJugador = nc;

        if (m == CellType::EXIT)
            res.nivelGanado = true;
    }

    // Hace caer las rocas una celda por tick si hay espacio vacío debajo.
    // Si una roca que venía cayendo aterriza justo encima del jugador, le hace daño.
    void actualizarRocas(ResultadoTick& res) {
        for (int f = mapa.getRows() - 2; f >= 0; f--) {
            for (int c = 0; c < mapa.getColumns(); c++) {
                if (ents.getValue(f, c) != CellType::ROCK) continue;

                if (libreParaMover(f + 1, c)) {
                    // La roca cae una celda y queda marcada como "en caída"
                    ents.setValue(f, c, CellType::EMPTY);
                    ents.setValue(f + 1, c, CellType::ROCK);
                    rocaCayendo.setValue(f, c, false);
                    rocaCayendo.setValue(f + 1, c, true);
                }
                else {
                    // Si venía cayendo y justo debajo está el jugador, lo golpea
                    if (rocaCayendo.getValue(f, c) && f + 1 == filaJugador && c == colJugador) {
                        recibirDano(res, danoRoca);
                    }
                    rocaCayendo.setValue(f, c, false);   // ya aterrizó
                }
            }
        }
    }

    // Revisa si hay un objeto pesado (roca) o el jugador encima de un botón.
    void actualizarBotones() {
        bool presionado = false;
        for (int f = 0; f < mapa.getRows(); f++) {
            for (int c = 0; c < mapa.getColumns(); c++) {
                if (mapa.getValue(f, c) == CellType::BUTTON) {
                    CellType e = ents.getValue(f, c);
                    if (e == CellType::ROCK || e == CellType::PLAYER) {
                        presionado = true;
                    }
                }
            }
        }
        puertasAbiertas = presionado;
    }

    // Determina si una celda puede ser pisada por un enemigo.
    bool transitable0Jugador(int f, int c) {
        return (f == filaJugador && c == colJugador) || libreParaMover(f, c);
    }

    // Algoritmo para calcular el siguiente paso de un enemigo en modo persecución hacia el jugador.
    void elegirDireccionPersecucion(Enemigo& en) {
        int df = filaJugador - en.fila;
        int dc = colJugador - en.col;
        int pf = (df > 0) - (df < 0);
        int pc = (dc > 0) - (dc < 0);
        int opciones[2][2];

        if (std::abs(df) >= std::abs(dc)) {
            opciones[0][0] = pf; opciones[0][1] = 0;
            opciones[1][0] = 0;  opciones[1][1] = pc;
        }
        else {
            opciones[0][0] = 0;  opciones[0][1] = pc;
            opciones[1][0] = pf; opciones[1][1] = 0;
        }

        en.dirFila = 0;
        en.dirCol = 0;
        for (int k = 0; k < 2; k++) {
            int a = opciones[k][0];
            int b = opciones[k][1];
            if ((a != 0 || b != 0) && transitable0Jugador(en.fila + a, en.col + b)) {
                en.dirFila = a;
                en.dirCol = b;
                return;
            }
        }
    }

    // Actualiza el comportamiento de todos los enemigos en el mapa por cada tick.
    void actualizarEnemigos(ResultadoTick& res) {
        for (int i = 0; i < numEnemigos; i++) {
            Enemigo& en = enemigos[i];
            en.contador--;
            if (en.contador > 0) continue;

            en.contador = en.ticksEntreMov;
            if (en.persigue) elegirDireccionPersecucion(en);

            if (en.dirFila == 0 && en.dirCol == 0) continue;

            int nf = en.fila + en.dirFila;
            int nc = en.col + en.dirCol;

            // Si el destino del enemigo es la celda del jugador, le hace daño
            if (nf == filaJugador && nc == colJugador) {
                recibirDano(res);
            }
            // Si la celda está libre, se desplaza hacia ella manteniendo su tipo respectivo
            else if (libreParaMover(nf, nc)) {
                ponerEnt(en.fila, en.col, CellType::EMPTY);
                ponerEnt(nf, nc, en.tipo);
                en.fila = nf;
                en.col = nc;
            }
            // Si patrullaba y choca con un obstáculo, invierte su dirección
            else if (!en.persigue) {
                en.dirFila = -en.dirFila;
                en.dirCol = -en.dirCol;
            }
        }
    }

public:
    Fisica(Matrix<CellType>& mapaFijo, Matrix<CellType>& entidades)
        : mapa(mapaFijo), ents(entidades),
        rocaCayendo(mapaFijo.getRows(), mapaFijo.getColumns()) {
        rocaCayendo.setAll(false);
        filaJugador = 0;
        colJugador = 0;
        invulnerableTicks = 0;
        puertasAbiertas = false;
        numEnemigos = 0;
    }

    void reiniciarNivel() {
        numEnemigos = 0;
        invulnerableTicks = 0;
        puertasAbiertas = false;
        rocaCayendo.setAll(false);
    }

    void colocarJugador(int f, int c) {
        filaJugador = f;
        colJugador = c;
        ponerEnt(f, c, CellType::PLAYER);
    }

    // Registra un nuevo enemigo indicando su tipo (ENEMY_H o ENEMY_V)
    bool agregarEnemigo(int f, int c, int dirF, int dirC, int ticks, bool persigue, CellType tipo = CellType::ENEMY_H) {
        if (numEnemigos >= maxEnemigos)
            return false;
        Enemigo& en = enemigos[numEnemigos];
        en.fila = f;
        en.col = c;
        en.dirFila = dirF;
        en.dirCol = dirC;
        en.ticksEntreMov = ticks;
        en.contador = ticks;
        en.persigue = persigue;
        en.tipo = tipo;
        numEnemigos++;
        ponerEnt(f, c, tipo);
        return true;
    }

    ResultadoTick actualizar(Mov accion) {
        ResultadoTick res;

        if (accion == Mov::UP)
            moverJugador(-1, 0, res);
        else if (accion == Mov::DOWN)
            moverJugador(1, 0, res);
        else if (accion == Mov::LEFT)
            moverJugador(0, -1, res);
        else if (accion == Mov::RIGHT)
            moverJugador(0, 1, res);

        actualizarRocas(res);
        actualizarBotones();
        actualizarEnemigos(res);

        if (invulnerableTicks > 0) invulnerableTicks--;

        return res;
    }

    int getFilaJugador() { return filaJugador; }
    int getColJugador() { return colJugador; }
    bool getPuertasAbiertas() { return puertasAbiertas; }
    int getInvulnerableTicks() { return invulnerableTicks; }
};