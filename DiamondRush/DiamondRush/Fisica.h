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
const int ticksInvulnerable = ticksPorSegundo / 2; // Duración de la invulnerabilidad tras recibir daño
const int contactoDolor = 10; //Daño que resta tocar a un enemigo
const int danoRoca = 20;      // Daño que resta que te caiga una roca encima
const int puntosGema = 10;   // Puntos que se suman al recoger un diamante/gema
const int maxEnemigos = 16;               // Número máximo de enemigos permitidos en simultáneo
const int alcanceLatigo = 3;    // Casillas máximas que alcanza el látigo
const int alcanceLatigoVertical = 2; // Así en vertical es más corto
const int ticksLatigo = 4;      // Cuántos ticks se ve el látigo en pantalla
const int cooldownLatigo = 5;   // Ticks de espera entre un latigazo y otro
const int ticksPorPasoTira = 2;   // Ticks que tarda la piedra en avanzar una casilla al ser jalada
const int vidaCorazon = 10;  // Vida que recupera un corazón
const int puntosCorazonLleno = 30;  // Puntos si recoges un corazón con la vida al 100
const int puntosEnemigo = 30;       // Puntos por aplastar un enemigo con una roca


// ESTRUCTURA DE RESULTADOS
//Objeto que la fisica retorna al bucle prinipal después de procesar cada tick
struct ResultadoTick {
    bool volverAtras = false; // true si el jugador pisó la casilla de entrada (regresar al mapa anterior)
    int puntosGanados = 0;    // Acumulador de puntos obtenidos en este tick
    int dano = 0;// Daño acumulado que el jugador debe recibir en este tic
    bool nivelGanado = false; // Bandera booleana que indica si el jugador llegó a la salida
    int curacion = 0;         // Vida recuperada en este tick
    int puntosExtra = 0;      // Puntos por eliminar enemigos en este tick
    bool rocaAterrizo = false; // true si una roca terminó de caer en este tick
    bool latigo = false;       // true si se lanzó el látigo en este tick
    bool puertaAbrio = false;  // true si una puerta se desbloqueó en este tick
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
    bool puertaAbierta[3];   // Estado de las puertas por color: 0 = rojo, 1 = verde, 2 = azul
    Enemigo enemigos[maxEnemigos]; // Arreglo estricto para almacenar todos los enemigos
    int numEnemigos;                 // Contador actual de enemigos registrados
    int filaInicio;              // Casilla de entrada del nivel (sirve para regresar al mapa anterior)
    int colInicio;
    int mirandoF;                // Hacia dónde mira el jugador (para el látigo)
    int mirandoC;
    int latigoCooldown;
    int latigoTicks;
    int latigoLargo;
    int latigoF[alcanceLatigo] = {};
    int latigoC[alcanceLatigo] = {};
    bool tiraActiva;             // true mientras el látigo está jalando una piedra
    int tiraF, tiraC;            // Posición actual de la piedra que se está jalando
    int tiraDirF, tiraDirC;      // Dirección del látigo al lanzarlo
    int tiraJugF, tiraJugC;      // Dónde estaba el jugador al lanzarlo (si se mueve, se cancela)
    int tiraContador;

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

    // Color de un botón (0 rojo, 1 verde, 2 azul) o -1 si la celda no es un botón
    int colorBoton(CellType t) {
        if (t == CellType::BUTTON) return 0;
        if (t == CellType::BUTTON_G) return 1;
        if (t == CellType::BUTTON_B) return 2;
        return -1;
    }

    // Color de una puerta (0 rojo, 1 verde, 2 azul) o -1 si la celda no es una puerta
    int colorPuerta(CellType t) {
        if (t == CellType::DOOR) return 0;
        if (t == CellType::DOOR_G) return 1;
        if (t == CellType::DOOR_B) return 2;
        return -1;
    }

    // Determina si un elemento fijo del mapa bloquea el paso (Pared o Puerta cerrada de su color).
    bool bloqueaMapa(CellType t) {
        int p = colorPuerta(t);
        return t == CellType::WALL || (p >= 0 && !puertaAbierta[p]);
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
        mirandoF = df;   
        mirandoC = dc;   
        int nf = filaJugador + df;
        int nc = colJugador + dc;
        if (!dentro(nf, nc)) {
            if (filaJugador == filaInicio && colJugador == colInicio)
                res.volverAtras = true;
            if (mapaEn(filaJugador, colJugador) == CellType::EXIT)
                res.nivelGanado = true;
            return;
        }

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
        // Caso D: Choca con un corazón
        else if (e == CellType::HEART) {
            res.curacion += vidaCorazon;
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

        if (nf == filaInicio && nc == colInicio)  
            res.volverAtras = true;
    }

    // Quita a un enemigo del arreglo y de la capa móvil
    void eliminarEnemigoEn(int f, int c) {
        for (int i = 0; i < numEnemigos; i++) {
            if (enemigos[i].fila == f && enemigos[i].col == c) {
                for (int j = i; j < numEnemigos - 1; j++) {
                    enemigos[j] = enemigos[j + 1];
                }
                numEnemigos--;
                ponerEnt(f, c, CellType::EMPTY);
                return;
            }
        }
    }

    // Hace caer las rocas una celda por tick si hay espacio vacío debajo.
// Si una roca que venía cayendo aterriza justo encima del jugador, le hace daño.
// Si una roca que venía cayendo le cae encima a un enemigo, lo mata.
    void actualizarRocas(ResultadoTick& res) {
        for (int f = mapa.getRows() - 2; f >= 0; f--) {
            for (int c = 0; c < mapa.getColumns(); c++) {
                if (ents.getValue(f, c) != CellType::ROCK) continue;

                // Solo mata si la roca ya venía en caída (una roca quieta no mata)
                CellType debajo = entEn(f + 1, c);
                if (rocaCayendo.getValue(f, c) &&
                    (debajo == CellType::ENEMY_H || debajo == CellType::ENEMY_V || debajo == CellType::ENEMY)) {
                    eliminarEnemigoEn(f + 1, c);
                    res.puntosExtra += puntosEnemigo;
                }

                if (libreParaMover(f + 1, c)) {
                    // La roca cae una celda y queda marcada como "en caída"
                    ents.setValue(f, c, CellType::EMPTY);
                    ents.setValue(f + 1, c, CellType::ROCK);
                    rocaCayendo.setValue(f, c, false);
                    rocaCayendo.setValue(f + 1, c, true);
                }
                else {
                    if (rocaCayendo.getValue(f, c)) res.rocaAterrizo = true;
                    // Si venía cayendo y justo debajo está el jugador, lo golpea
                    if (rocaCayendo.getValue(f, c) && f + 1 == filaJugador && c == colJugador) {
                        recibirDano(res, danoRoca);
                    }
                    rocaCayendo.setValue(f, c, false);   // ya aterrizó
                }
            }
        }
    }

    // Una puerta se abre solo si TODOS los botones de su color tienen una roca o al jugador encima.
    bool actualizarBotones() {
        int total[3] = { 0, 0, 0 };
        int activos[3] = { 0, 0, 0 };
        for (int f = 0; f < mapa.getRows(); f++) {
            for (int c = 0; c < mapa.getColumns(); c++) {
                int col = colorBoton(mapa.getValue(f, c));
                if (col < 0) continue;
                total[col]++;
                CellType e = ents.getValue(f, c);
                if (e == CellType::ROCK || e == CellType::PLAYER) {
                    activos[col]++;
                }
            }
        }
        bool abrio = false;
        for (int i = 0; i < 3; i++) {
            bool nueva = (total[i] > 0 && activos[i] == total[i]);
            if (nueva && !puertaAbierta[i]) abrio = true;
            puertaAbierta[i] = nueva;
        }
        return abrio;
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
    

    // Borra el dibujo del látigo (solo donde todavía sea látigo)
    void limpiarLatigo() {
        for (int i = 0; i < latigoLargo; i++) {
            CellType e = entEn(latigoF[i], latigoC[i]);
            if (e == CellType::WHIP_H || e == CellType::WHIP_V) {
                ponerEnt(latigoF[i], latigoC[i], CellType::EMPTY);
            }
        }
        latigoLargo = 0;
        latigoTicks = 0;
    }
    // Dibuja el látigo desde la casilla "desde" hasta la casilla "hasta" (contadas desde el jugador)
    void dibujarLatigo(int desde, int hasta, int dirF, int dirC) {
        CellType tipo = (dirF != 0) ? CellType::WHIP_V : CellType::WHIP_H;
        for (int i = desde; i <= hasta; i++) {
            int f = filaJugador + dirF * i;
            int c = colJugador + dirC * i;
            if (entEn(f, c) == CellType::EMPTY && latigoLargo < alcanceLatigo) {
                ponerEnt(f, c, tipo);
                latigoF[latigoLargo] = f;
                latigoC[latigoLargo] = c;
                latigoLargo++;
            }
        }
    }

    // Látigo: se estira hasta la primera roca dentro del alcance y empieza a jalarla hacia el jugador.
    // Alcance: 3 casillas en horizontal y 2 en vertical.
    // Se detiene en paredes, puertas cerradas, arbustos, la salida y enemigos.
    void usarLatigo() {
        if (latigoCooldown > 0 || tiraActiva) return;
        latigoCooldown = cooldownLatigo;
        limpiarLatigo();

        int alcance = (mirandoF != 0) ? alcanceLatigoVertical : alcanceLatigo;
        int dibHasta = 0;   // hasta qué casilla se dibuja el látigo
        for (int k = 1; k <= alcance; k++) {
            int f = filaJugador + mirandoF * k;
            int c = colJugador + mirandoC * k;
            if (!dentro(f, c)) break;

            CellType m = mapaEn(f, c);
            if (bloqueaMapa(m) || m == CellType::BUSH || m == CellType::EXIT) break;

            CellType e = entEn(f, c);

            if (e == CellType::ROCK) {
                dibHasta = k - 1;   // el látigo llega hasta la piedra, sin atravesarla
                // Solo la jala si no está ya pegada y hay espacio libre hacia el jugador
                if (k >= 2 && libreParaMover(filaJugador + mirandoF * (k - 1), colJugador + mirandoC * (k - 1))) {
                    tiraActiva = true;
                    tiraF = f;
                    tiraC = c;
                    tiraDirF = mirandoF;
                    tiraDirC = mirandoC;
                    tiraJugF = filaJugador;
                    tiraJugC = colJugador;
                    tiraContador = ticksPorPasoTira;
                }
                break;
            }

            if (e == CellType::ENEMY_H || e == CellType::ENEMY_V || e == CellType::ENEMY) {
                dibHasta = k - 1;
                break;
            }

            dibHasta = k;
        }

        dibujarLatigo(1, dibHasta, mirandoF, mirandoC);
        latigoTicks = (latigoLargo > 0) ? ticksLatigo : 0;
    }

    // Avanza una casilla la piedra que se está jalando y acorta el látigo.
    // Se cancela si el jugador se mueve, si la piedra cae o si algo bloquea el camino.
    void avanzarTira() {
        if (!tiraActiva) return;

        if (filaJugador != tiraJugF || colJugador != tiraJugC || entEn(tiraF, tiraC) != CellType::ROCK) {
            tiraActiva = false;
            limpiarLatigo();
            return;
        }

        tiraContador--;
        if (tiraContador > 0) return;
        tiraContador = ticksPorPasoTira;

        limpiarLatigo();   // se quita el látigo para que no estorbe a la piedra

        int nf = tiraF - tiraDirF;
        int nc = tiraC - tiraDirC;
        if (!libreParaMover(nf, nc)) {
            tiraActiva = false;
            return;
        }

        ponerEnt(tiraF, tiraC, CellType::EMPTY);
        ponerEnt(nf, nc, CellType::ROCK);
        rocaCayendo.setValue(tiraF, tiraC, false);
        rocaCayendo.setValue(nf, nc, false);
        tiraF = nf;
        tiraC = nc;

        int dist = std::abs(tiraF - filaJugador) + std::abs(tiraC - colJugador);
        if (dist <= 1) {
            tiraActiva = false;   // la piedra ya quedó pegada al jugador
            return;
        }

        dibujarLatigo(1, dist - 1, tiraDirF, tiraDirC);
        latigoTicks = ticksLatigo;
    }

public:
    Fisica(Matrix<CellType>& mapaFijo, Matrix<CellType>& entidades)
        : mapa(mapaFijo), ents(entidades),
        rocaCayendo(mapaFijo.getRows(), mapaFijo.getColumns()) {
        rocaCayendo.setAll(false);
        filaJugador = 0;
        colJugador = 0;
        invulnerableTicks = 0;
        for (int i = 0; i < 3; i++) puertaAbierta[i] = false;
        numEnemigos = 0;
        filaInicio = -1;
        colInicio = -1;
        mirandoF = 0;
        mirandoC = 1;
        latigoCooldown = 0;
        latigoTicks = 0;
        latigoLargo = 0;
        tiraActiva = false;
        tiraF = 0;
        tiraC = 0;
        tiraDirF = 0;
        tiraDirC = 0;
        tiraJugF = 0;
        tiraJugC = 0;
        tiraContador = 0;
    }

    void reiniciarNivel() {
        numEnemigos = 0;
        invulnerableTicks = 0;
        for (int i = 0; i < 3; i++) puertaAbierta[i] = false;
        rocaCayendo.setAll(false);
    }

    void colocarJugador(int f, int c) {
        filaJugador = f;
        colJugador = c;
        filaInicio = f;   
        colInicio = c;  
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
        else if (accion == Mov::LATIGO) {
            int antes = latigoCooldown;
            usarLatigo();
            res.latigo = (latigoCooldown > antes);   // true solo si el látigo se lanzó de verdad
        }

        avanzarTira();
        actualizarRocas(res);
        if (actualizarBotones()) res.puertaAbrio = true;
        actualizarEnemigos(res);

        if (invulnerableTicks > 0) invulnerableTicks--;
        if (latigoCooldown > 0) latigoCooldown--;
        if (!tiraActiva && latigoTicks > 0) {
            latigoTicks--;
            if (latigoTicks == 0) limpiarLatigo();
        }

        return res;
    }

    int getFilaJugador() { return filaJugador; }
    int getColJugador() { return colJugador; }
    int getInvulnerableTicks() { return invulnerableTicks; }

    // Dirección actual del enemigo que está en (f, c). Devuelve false si no hay ninguno.
    bool direccionEnemigoEn(int f, int c, int& dF, int& dC) {
        for (int i = 0; i < numEnemigos; i++) {
            if (enemigos[i].fila == f && enemigos[i].col == c) {
                dF = enemigos[i].dirFila;
                dC = enemigos[i].dirCol;
                return true;
            }
        }
        return false;
    }
};