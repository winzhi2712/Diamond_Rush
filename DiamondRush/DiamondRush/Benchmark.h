#pragma once
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
struct Frames {
	int frame = 0;
	double inputT = 0.0;
	double fisicaT = 0.0;
	double hudT = 0.0;
	double renderT = 0.0;
	double totalT = 0.0;

};

class Benchmark {
private:
	bool on = false;
	bool modoSilencio = false; //quita los sleeps y deja de imprimir en pantalla
	vector<Frames> muestra;
	Frames actual;

public:
	void startBench(bool silencio = false) { //por defecto, el modo silencio es false, se puede cambiar en la entrada
		muestra.clear();
		on = true;
		modoSilencio = silencio;
	}
	void stopBench() {
		on = false;
	}
	bool estaOn() const {
		return on;
	}
	bool estaSilencio() const {
		return modoSilencio;
	}
	void startFrame(int numFrame) {
		if (!on) return;
		actual = Frames{};
		actual.frame = numFrame;
	}
	void registerInput(double t) {
		if (on)
			actual.inputT = t;
	}

	void registerFisica (double t) {
		if (on)
			actual.fisicaT = t;
	}

	void registerHud(double t) {
		if (on)
			actual.hudT = t;
	}

	void registerRender(double t) {
		if (on)
			actual.renderT = t;
	}
	void endFrame() {
		if (!on)
			return;
		actual.totalT = actual.fisicaT + actual.hudT + actual.inputT + actual.renderT;
		muestra.push_back(actual);
	}

	bool exportCVS(const string& ruta) const {
		ofstream archivo(ruta);
		if (!archivo.is_open())
			return false;
		archivo << "frame, inputT, fisicaT, hudT, renderT, totalT" << endl;
		for (const auto& m : muestra) {
			archivo << m.frame << ", " << m.inputT << ", " << m.fisicaT << ", " << m.hudT << ", " << m.renderT << ", " << m.totalT <<endl;
		}
		return true;
	}
};

