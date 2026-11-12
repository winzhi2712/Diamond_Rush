#pragma once

#include <stdexcept>
#include <iostream>

using std::cout;
using std::runtime_error;
using std::endl;

template <typename E>
class Matrix {
private:
	int rows;
	int columns;
	E** matrix;

public:
	int currentRow;
	int currentCol;
	Matrix(int rows, int columns) {
		if (rows <= 0 || columns <= 0)
			throw runtime_error("Rows and Columns must be greater than zero");
		this->rows = rows;
		this->columns = columns;
		this->currentRow = 0;
		this->currentCol = 0;
		matrix = new E * [rows];
		for (int i = 0; i < rows; i++) {
			matrix[i] = new E[columns];
		}
	}
	~Matrix() {
		for (int i = 0; i < rows; i++) {
			delete[] matrix[i];
		}
		delete[] matrix;
	}
	E getValue(int row, int column) {
		if (row < 0 || row >= rows)
			throw runtime_error("Out of bounds (row)");
		if (column < 0 || column >= columns)
			throw runtime_error("Out of bounds (column)");
		return matrix[row][column];
	}
	void setValue(int row, int column, E value) {
		if (row < 0 || row >= rows)
			throw runtime_error("Out of bounds (row)");
		if (column < 0 || column >= columns)
			throw runtime_error("Out of bounds (column)");
		matrix[row][column] = value;
	}
	int getRows() {
		return rows;
	}
	int getColumns() {
		return columns;
	}
	void setAll(E value) {
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns; j++) { //recorrrer la matriz
				matrix[i][j] = value; //darme el valor
			}
		}
	}
	void transpose() {
		int columnsT = rows; //intercambar valores
		int rowsT = columns;
		E** temp = new E * [rowsT];
		for (int i = 0; i < rowsT; i++) { //crear una nueva matriz con las dimanesiones transpuestas
			temp[i] = new E[columnsT];
		}

		for (int i = 0; i < rows; i++) { //llenar la nueva matriz
			for (int c = 0; c < columns; c++) {
				temp[c][i] = matrix[i][c];
			}
		}


		for (int i = 0; i < rows; i++) { //borrar la anterior
			delete[] matrix[i];
		}
		delete[] matrix;
		rows = rowsT; //remplazar los valores por los de la transpuesta
		columns = columnsT;
		matrix = temp;
	}
	void addRow(E value) {
		E** temp = new E * [rows + 1];
		for (int i = 0; i < rows + 1; i++) { //crear nueva matriz con las nuevas dimensiones
			temp[i] = new E[columns];
		}

		for (int i = 0; i < rows; i++) {
			for (int c = 0; c < columns; c++) { //llenar la matriz nueva
				temp[i][c] = matrix[i][c];
			}
		}
		for (int c = 0; c < columns; c++) //llenar la nueva fila
			temp[rows][c] = value;

		for (int i = 0; i < rows; i++) { //borrar y remplazar
			delete[] matrix[i];
		}
		delete[] matrix;
		matrix = temp;
		rows += 1;
	}
	void addColumn(E value) {
		E** temp = new E * [rows]; //crear nueva matriz con las nuevas dimensiones
		for (int i = 0; i < rows; i++) {
			temp[i] = new E[columns + 1];
		}

		for (int i = 0; i < rows; i++) { //llenar la matriz
			for (int c = 0; c < columns; c++) {
				temp[i][c] = matrix[i][c];
			}
		}

		for (int i = 0; i < rows; i++) //llenar la nueva columna
			temp[i][columns] = value;

		for (int i = 0; i < rows; i++) { //borrar y remplazar
			delete[] matrix[i];
		}
		delete[] matrix;
		matrix = temp;
		columns += 1;
	}
	void removeRow(int value) {
		if (value < 0 || value >= rows) //restricciones
			throw runtime_error("Value no valido");
		E** temp = new E * [rows - 1]; //crear nueva matriz con las nuevas dimensiones
		for (int i = 0; i < rows - 1; i++) {
			temp[i] = new E[columns];
		}

		int row = 0;
		for (int i = 0; i < rows; i++) { //llenar sin la fila a quitar
			if (i != value) {
				for (int c = 0; c < columns; c++) {
					temp[row][c] = matrix[i][c];
				}
				row++;
			}
		}
		for (int i = 0; i < rows; i++) { //borrar y remplazar
			delete[] matrix[i];
		}
		delete[] matrix;
		matrix = temp;
		rows -= 1;
	}
	void removeColumn(int value) {
		if (value < 0 || value >= columns) //restricciones
			throw runtime_error("Value no valido");
		E** temp = new E * [rows];
		for (int i = 0; i < rows; i++) {//crear nueva matriz con las nuevas dimensiones
			temp[i] = new E[columns - 1];
		}

		for (int i = 0; i < rows; i++) { //llenar sin la columna a borrar
			int column = 0;
			for (int c = 0; c < columns; c++) {
				if (c != value) {
					temp[i][column] = matrix[i][c];
					column += 1;
				}
			}
		}
		for (int i = 0; i < rows; i++) { //borrar y remplazar
			delete[] matrix[i];
		}
		delete[] matrix;
		matrix = temp;
		columns -= 1;
	}
	void print() {
		system("cls");
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) { //recorrer e imprimir
				cout << "[" << matrix[r][c] << "]" << '\t';
			}
			cout << endl;
		}
	}
	void setCurrent(int x, int y) {
		currentRow = x;
		currentCol = y;
	}
	E getCurrent() {
		return getValue(currentRow, currentCol);
	}
};

