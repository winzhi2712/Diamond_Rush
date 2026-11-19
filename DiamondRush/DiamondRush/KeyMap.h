#pragma once
#include <unordered_map>

enum class Mov {
	VACIA,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	PAUSE,
	EXIT,
	YES,
	RESTART, // Movimiento para reiniciar el nivel con la tecla R
	LATIGO
};

class KeyMap {
private:
	std::unordered_map<int, Mov> mapa;

public:
	KeyMap() {
		mapa['w'] = Mov::UP; // WASD
		mapa['W'] = Mov::UP;
		mapa['s'] = Mov::DOWN;
		mapa['S'] = Mov::DOWN;
		mapa['a'] = Mov::LEFT;
		mapa['A'] = Mov::LEFT;
		mapa['d'] = Mov::RIGHT;
		mapa['D'] = Mov::RIGHT;

		mapa[256 + 72] = Mov::UP; // Flechitas
		mapa[256 + 80] = Mov::DOWN;
		mapa[256 + 75] = Mov::LEFT;
		mapa[256 + 77] = Mov::RIGHT;

		mapa[13] = Mov::YES;   // Para estados
		mapa['p'] = Mov::PAUSE;
		mapa['P'] = Mov::PAUSE; // Añadido por si acaso presionan P mayúscula
		mapa[27] = Mov::EXIT;  // ESC

		// NUEVO: Mapeo para reiniciar el nivel con la tecla R
		mapa['r'] = Mov::RESTART;
		mapa['R'] = Mov::RESTART;
		mapa['e'] = Mov::LATIGO;
		mapa['E'] = Mov::LATIGO; //para ponerlo, pon en el ciclo principal mov == Mov::LATIGO en el main del cpp

	}

	Mov getMov(int codigoTecla) { // Convertir el código de la tecla a una acción del juego
		if (mapa.find(codigoTecla) != mapa.end()) {
			return mapa[codigoTecla];
		}

		return Mov::VACIA;
	}
};