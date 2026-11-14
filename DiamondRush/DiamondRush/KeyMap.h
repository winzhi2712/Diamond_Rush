#pragma once
#include<unordered_map>
enum class Mov {
	VACIA,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	YES,
	PAUSE,
	EXIT
};


class KeyMap {
private:
	std::unordered_map<int, Mov> mapa;
public:
	KeyMap() {
		mapa['w'] = Mov::UP; //wasd
		mapa['W'] = Mov::UP;
		mapa['s'] = Mov::DOWN;
		mapa['S'] = Mov::DOWN;
		mapa['a'] = Mov::LEFT;
		mapa['A'] = Mov::LEFT;
		mapa['d'] = Mov::RIGHT;
		mapa['D'] = Mov::RIGHT;

		mapa[256 + 72] = Mov::UP; //flechitas
		mapa[256 + 80] = Mov::DOWN;
		mapa[256 + 75] = Mov::LEFT;
		mapa[256 + 77] = Mov::RIGHT;

		mapa[13] = Mov::YES;   //para estados
		mapa['p'] = Mov::PAUSE;
		mapa[27] = Mov::EXIT;
	}

	Mov getMov(int codigoTecla) { //convertir el codigo de la tecla a una accion del juego
		if (mapa.find(codigoTecla) != mapa.end()) {
			return mapa[codigoTecla];
		}
		return Mov::VACIA;
	}
};

