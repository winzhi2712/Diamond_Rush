#pragma once
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
// Contiene la logica principal del juego
// Gestiona el estado, funcionamiento y flujo de ejecucion

class Game {
private:
	int vida;
	int score;

	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point pausaInicio;
	const int LIMITE_TIEMPO = 480;

public:
	Game() {
		reset();
	}
	void reset() {
		vida = 100; // esto es temporal, ya despues lo cambiamos por la vida que quieran poner
		score = 0;
		start = std::chrono::steady_clock::now();
		pausaInicio = start;
	}

	// Reinicia solo el reloj (para empezar un nivel nuevo sin perder puntaje ni vida)
	void reiniciarTiempo() {
		start = std::chrono::steady_clock::now();
		pausaInicio = start;
	}

	// El reloj no avanza mientras el juego esta en pausa
	void pausar() {
		pausaInicio = std::chrono::steady_clock::now();
	}
	void reanudar() {
		start += std::chrono::steady_clock::now() - pausaInicio;
	}

	void addScore(int puntos) {
		score += puntos;
	}
	void perderVida(int damage) {
		vida -= damage;
	}
	void curar(int cantidad) {
		vida += cantidad;
		if (vida > 100) vida = 100;
	}
	int getVide() {
		return vida;
	}
	int getScore() {
		return score;
	}
	int timeLeft() {
		auto ahora = std::chrono::steady_clock::now();
		auto transcurrido = std::chrono::duration_cast<std::chrono::seconds>(ahora - start).count();
		int restante = LIMITE_TIEMPO - static_cast<int>(transcurrido);
		return (restante > 0) ? restante : 0;
	}
	std::string formatTime() {
		int totalSegundos = timeLeft();
		int min = totalSegundos / 60;
		int segundos = totalSegundos % 60;

		std::stringstream ss;
		ss << std::setfill('0') << std::setw(1) << min << ":" << std::setfill('0') << std::setw(2) << segundos;
		return ss.str();
	}
	bool perder() {
		return timeLeft() <= 0 || vida < 0;
	}
};