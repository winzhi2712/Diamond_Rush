#pragma once

// Mikel estuvo aquí y en sus corazones.
// ¿Qué es esto? pues maneja colisiones, movimiento del jugador, 
// física de piedras, puertas, botones y enemigos por cada tick.


#include <cstdlib>
#include "Matrix.h"
#include "Tipos.h"
#include "KeyMap.h"

//CONSTANTES DE CONFIGURACIÓN
const int ticksPorSegundo = 30; // Velocidad del bucle del juego 
const int ticksInvulnerable = 1 * ticksPorSegundo; // Duración de la invulnerabilidad tras recibir daño
const int contactoDolor = 10; //Daño que resta tocar a un enemigo
const int puntosGema = 10;   // Puntos que se suman al recoger un diamante/gema
const int maxEnemigos = 16;              // Número máximo de enemigos permitidos en simultáneo

// ESTRUCTURA DE RESULTADOS
//Objeto que la fisica retorna al bucle prinipal después de procesar cada tick
struct ResultadoTick {
    int puntosGanados = 0;    // Acumulador de puntos obtenidos en este tick
    int daño = 0;          // Daño acumulado que el jugador debe recibir en este tick
    bool nivelGanado = false; // Bandera booleana que indica si el jugador llegó a la salida
};

//ESTRUCTURA DE ENEMIGO
// Define el stado y comportamiento individual de cada enemigo en el mapa.
struct Enemigo {
    int fila = 0;
    int col = 0;
    int dirFila = 0;       // Dirección actual en filas (-1 arriba, 1 abajo, 0 quieto)
    int dirCol = 0;        // Dirección actual en columnas (-1 izq, 1 der, 0 quieto)
    int ticksEntreMov = 10; // Frecuencia de movimiento (ej. se mueve cada 10 ticks)
    int contador = 10;     // Temporizador interno que cuenta regresivamente hasta el siguiente movimiento
    bool persigue = false; // false = patrulla en línea recta; true = persigue al jugador
};

class Fisica {
private:
    // Referencias a las dos matrices principales del juego (Capa fija y Capa mobil)
    Matrix<CellType>& mapa; // Capa fija: Contiene paredes, salidas, botones y puertas
    Matrix<CellType>& ents; // Capa movil: Contiene rocas, gemas, jugador y enemigos
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





    // Obtiene el tipo de entidad mó}vil de forma segura.
    CellType entEn(int f, int c) {
        if (!dentro(f, c)) 
            return CellType::WALL;
        return ents.getValue(f, c);
    }
    //coloca una entidad en la caa móvil asegurándose de no salir del mapa.
    void ponerEnt(int f, int c, CellType t) {
        if (dentro(f, c)) 
            ents.setValue(f, c, t);
    }

    // Determina si un elemento fijo del mapa bloquea el paso (Pared o Puerta cerrada).
    bool bloqueaMapa(CellType t) {
        return t == CellType::WALL || (t == CellType::DOOR && !puertasAbiertas);
    }
    //Regla crítica: Comprueba si una celda está totalmente libre para que una roca o enemigo entren.
    bool libreParaMover(int f, int c) {
        if (!dentro(f, c)) 
            return false;
        CellType m = mapaEn(f, c);
        // Debe estar dentro, no bloquear el mapa, no ser la salida, y no tener otra entidad encima.
        return !bloqueaMapa(m) && m != CellType::EXIT && entEn(f, c) == CellType::EMPTY;
    }
    // Aplica daño al jugador siempre y cuando su tiempo de invulnerabilidad haya expirado.
    void recibirDaño(ResultadoTick& res) {
        if (invulnerableTicks > 0) 
            return; // Si aún está parpadeando protegido, ignora el daño
        res.daño += contactoDolor;
        invulnerableTicks = ticksInvulnerable; // Reinicia el temporizador de invulnerabilidad (1 segundo)
    }


    // Procesa el intento de movimiento del jugador según la dirección recibida (df, dc).
    void moverJugador(int df, int dc, ResultadoTick& res) {
        int nf = filaJugador + df;
        int nc = colJugador + dc;
        if (!dentro(nf, nc)) 
            return; // Evita salir de los límites de la matriz
        CellType m = mapaEn(nf, nc);
        if (bloqueaMapa(m)) 
            return; // Si choca con pared o puerta cerrada, se cancela el movimiento
        CellType e = entEn(nf, nc);
        // Caso A: Choca con un enemigo
        if (e == CellType::ENEMY) {
            recibirDaño(res); // Recibe daño por contacto pero no se mueve
            return;
        }
        
        // Caso B: Choca con una roca (intenta empujar)
        if (e == CellType::ROCK) {
            if (df != 0) return; // REGLA: Las rocas solo se pueden empujar horizontalmente
            if (!libreParaMover(nf, nc + dc)) 
                return; // Si detrás de la roca no hay espacio libre, no se mueve
            // Empuja la roca una celda más en la misma dirección
            ponerEnt(nf, nc + dc, CellType::ROCK); 
        } 
        // Caso C: Choca con una gema/diamante
        else if (e == CellType::DIAMOND) {
            res.puntosGanados += puntosGema; // Suma puntos a la cuenta
        }
        // Actualiza las posiciones en la matriz de entidades móviles
        ponerEnt(filaJugador, colJugador, CellType::EMPTY); // Libera la celda actual del jugador
        ponerEnt(nf, nc, CellType::PLAYER);             // Ocupa la nueva celda
        filaJugador = nf;
        colJugador = nc;
        
        // Caso D: Si pisa la celda de salida, marca el nivel como ganado
        if (m == CellType::EXIT) 
            res.nivelGanado = true;
    }
    // Hace caer las rocas una celda por tick si hay espacio vacío debajo.
    void actualizarRocas() {
        //IMPORTANTE: Se recorre de ABAJO hacia ARRIBA (rows-2 hasta 0). 
        //Si fuera de arriba hacia abajo, una roca podría caer varias veces en el mismo tick.
        for (int f = mapa.getRows() - 2; f >= 0; f--) {
            for (int c = 0; c < mapa.getColumns(); c++) {
                // Si hay una roca y la celda de abajo está libre para recibirla...
                if (ents.getValue(f, c) == CellType::ROCK && libreParaMover(f + 1, c)) {
                    ents.setValue(f, c, CellType::EMPTY);    // Vacía su posición actual
                    ents.setValue(f + 1, c, CellType::ROCK); // La mueve una celda hacia abajo
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
                    // Si el botón tiene una roca o al jugador encima, se activa
                    if (e == CellType::ROCK || e == CellType::PLAYER) {
                        presionado = true;
                    }
                }
            }
        }
        puertasAbiertas = presionado; // Si al menos un botón está presionado, todas las puertas del mapa se abren
    }


    // Determina si una celda puede ser pisada por un enemigo (incluyendo si está el jugador allí).
    bool transitable0Jugador(int f, int c) {
        return (f == filaJugador && c == colJugador) || libreParaMover(f, c);
    }

    // Algoritmo para calcular el siguiente paso de un enemigo en modo persecución hacia el jugador.
    void elegirDireccionPersecucion(Enemigo& en) {
        int df = filaJugador - en.fila;
        int dc = colJugador - en.col;
        int pf = (df > 0) - (df < 0); // Signo de distancia en filas (-1, 0 o 1)
        int pc = (dc > 0) - (dc < 0); // Signo de distancia en columnas (-1, 0 o 1)
        int opciones[2][2];
        


        // Prioriza el eje donde esté más lejos del jugador para acortar distancia de manera natural
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
        // Evalúa las opciones de movimiento disponibles
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
            if (en.contador > 0) continue; // Si aún no le toca moverse según su velocidad, salta este turno
            
            en.contador = en.ticksEntreMov; // Reinicia el temporizador de movimiento
            if (en.persigue) elegirDireccionPersecucion(en); // Calcula ruta si persigue
            
            if (en.dirFila == 0 && en.dirCol == 0) continue;
            
            int nf = en.fila + en.dirFila;
            int nc = en.col + en.dirCol;
            
            // Si el destino del enemigo es la celda del jugador, le hace daño
            if (nf == filaJugador && nc == colJugador) {
                recibirDaño(res);
            } 
            // Si la celda está libre, se desplaza hacia ella
            else if (libreParaMover(nf, nc)) {
                ponerEnt(en.fila, en.col, CellType::EMPTY);
                ponerEnt(nf, nc, CellType::ENEMY);
                en.fila = nf;
                en.col = nc;
            } 
            // Si patrullaba y choca con un obstáculo, invierte su dirección (da media vuelta)
            else if (!en.persigue) {
                en.dirFila = -en.dirFila;
                en.dirCol = -en.dirCol;
            }
        }
    }

public:
    // Constructor que enlaza las referencias de las matrices del juego con el módulo de física.
    Fisica(Matrix<CellType>& mapaFijo, Matrix<CellType>& entidades)
        : mapa(mapaFijo), ents(entidades) {
        filaJugador = 0;
        colJugador = 0;
        invulnerableTicks = 0;
        puertasAbiertas = false;
        numEnemigos = 0;
    }

    // Restablece contadores al cargar un nuevo nivel.
    void reiniciarNivel() {
        numEnemigos = 0;
        invulnerableTicks = 0;
        puertasAbiertas = false;
    }

    // Ubica al jugador en una posición inicial específica dentro del mapa.
    void colocarJugador(int f, int c) {
        filaJugador = f;
        colJugador = c;
        ponerEnt(f, c, CellType::PLAYER);
    }

    //Registra un nuevo enemigo en las coordenadas indicadas con sus parámetros de patrulla/persecución.
    bool agregarEnemigo(int f, int c, int dirF, int dirC, int ticks, bool persigue) {
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
        numEnemigos++;
        ponerEnt(f, c, CellType::ENEMY);
        return true;
    }

    //orden estricto de ejecución de todo el juego por cada tick.
    ResultadoTick actualizar(Mov accion) {
        ResultadoTick res;
        
        //Procesa la entrada del jugador y su movimiento (gemas, empujes, salidas, enemigos)
        if (accion == Mov::UP) 
            moverJugador(-1, 0, res);
        else if (accion == Mov::DOWN) 
            moverJugador(1, 0, res);
        else if (accion == Mov::LEFT) 
            moverJugador(0, -1, res);
        else if (accion == Mov::RIGHT) 
            moverJugador(0, 1, res);

        //Ejecuta la física de gravedad de las rocas (de abajo hacia arriba)
        actualizarRocas();

        //Actualiza los botones y el estado de las puertas con las posiciones finales de las rocas
        actualizarBotones();

        //Mueve a los enemigos y evalúa colisiones de daño con el jugador
        actualizarEnemigos(res);

        //Disminuye en 1 el contador de invulnerabilidad del jugador al finalizar el tick
        if (invulnerableTicks > 0) invulnerableTicks--;
        
        return res; // Retorna el paquete con puntaje, daño y estado de victoria al motor principal
    }

    //GETTERS
    int getFilaJugador() { 
        return filaJugador; 
    }
    int getColJugador() { 
        return colJugador; 
    }
    bool getPuertasAbiertas() { 
        return puertasAbiertas; 
    }
    int getInvulnerableTicks() { 
        return invulnerableTicks; 
    }
    //Bueno ya estuvo, nos vemos mi gente latino.
};