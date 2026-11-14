CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

all: fantasmas pruebas

fantasmas: src/main.cpp src/estructuras.hpp src/grafo.hpp src/catalogo.hpp src/exportar.hpp
	$(CXX) $(CXXFLAGS) -o fantasmas src/main.cpp

pruebas: tests/tests.cpp src/estructuras.hpp src/grafo.hpp src/catalogo.hpp
	$(CXX) $(CXXFLAGS) -o pruebas tests/tests.cpp

test: pruebas
	./pruebas

web: fantasmas
	./fantasmas --exportar

clean:
	rm -f fantasmas pruebas
