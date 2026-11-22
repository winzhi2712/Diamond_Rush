#!/usr/bin/env bash
# =====================================================================
#  build_pi.sh  -  Compila Diamond Rush completo en la Raspberry Pi
#
#  Va en la carpeta de la SOLUCION (la que tiene DiamondRush.slnx):
#      ~/proyectos/Diamond_Rush/DiamondRush/build_pi.sh
#
#  Estructura que espera (cada proyecto en su carpeta, como en Visual Studio):
#      DiamondRush/      -> DiamondRush.cpp, los .h, nivel1-3.txt, puntajes.txt
#      DiamondRushGUI/   -> DiamondRushGUI.cpp
#      Launcher/         -> Launcher.cpp
#
#  Los tres ejecutables se dejan en DiamondRush/ (junto a los niveles).
#  Uso:
#      chmod +x build_pi.sh
#      ./build_pi.sh
#      ./DiamondRush/Launcher
# =====================================================================
cd "$(dirname "$0")" || exit 1

CONSOLA=DiamondRush
GUI=DiamondRushGUI
LANZ=Launcher
SALIDA="$CONSOLA"

for f in "$CONSOLA/DiamondRush.cpp" "$GUI/DiamondRushGUI.cpp" "$LANZ/Launcher.cpp" "$CONSOLA/nivel1.txt"; do
    if [ ! -f "$f" ]; then
        echo "ERROR: falta $f (estoy en $(pwd))"
        exit 1
    fi
done

# Linux distingue mayusculas de minusculas: si el codigo dice NombreConsola.h
# pero el archivo se llama nombreConsola.h, se crea un acceso con el otro nombre.
if [ -f "$CONSOLA/nombreConsola.h" ] && [ ! -e "$CONSOLA/NombreConsola.h" ]; then
    ln -s nombreConsola.h "$CONSOLA/NombreConsola.h"
fi

echo "[1/3] Juego de consola..."
g++ -std=c++17 -O2 "$CONSOLA/DiamondRush.cpp" -o "$SALIDA/DiamondRush" || { echo "ERROR compilando DiamondRush"; exit 1; }

echo "[2/3] Lanzador..."
g++ -std=c++17 -O2 -I"$CONSOLA" "$LANZ/Launcher.cpp" -o "$SALIDA/Launcher" || { echo "ERROR compilando Launcher"; exit 1; }

echo "[3/3] Modo GUI (raylib)..."
# Estas librerias extra hacen falta cuando raylib esta instalado como libreria estatica
EXTRA="-lGL -lm -lpthread -ldl -lrt -lX11 -latomic"
if pkg-config --exists raylib 2>/dev/null; then
    LIBS="$(pkg-config --cflags --libs raylib) $EXTRA"
else
    LIBS="-lraylib $EXTRA"
fi
if g++ -std=c++17 -O2 -I"$CONSOLA" "$GUI/DiamondRushGUI.cpp" -o "$SALIDA/DiamondRushGUI" $LIBS 2> build_gui.log; then
    echo "Modo GUI listo."
    rm -f build_gui.log
else
    echo "AVISO: no se pudo compilar el modo GUI. Ultimas lineas del error:"
    echo "------------------------------------------------------------"
    tail -n 15 build_gui.log
    echo "------------------------------------------------------------"
    echo "(el error completo esta en build_gui.log)"
    echo "El modo consola y el lanzador si quedaron listos."
fi

echo
echo "Listo. Para jugar:"
echo "    cd $SALIDA && ./Launcher"
