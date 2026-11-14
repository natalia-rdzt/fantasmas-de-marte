// =====================================================================
//  tests.cpp  -  Pruebas con casos limite (sin librerias externas)
//  Compilar:  g++ -std=c++17 -Wall -Wextra -o pruebas tests/tests.cpp
//  Ejecutar desde la carpeta del proyecto:  ./pruebas
// =====================================================================
#include <cstdio>
#include <string>
#include "../src/estructuras.hpp"
#include "../src/grafo.hpp"
#include "../src/catalogo.hpp"

static int g_verificaciones = 0, g_fallos = 0, g_pruebas = 0, g_pruebasFalladas = 0;
static int g_fallosAntes = 0;

#define VERIFICAR(cond) do { g_verificaciones++; if (!(cond)) { g_fallos++; \
    std::printf("      FALLO (linea %d): %s\n", __LINE__, #cond); } } while (0)

static void empezar(const char* nombre) {
    g_pruebas++;
    g_fallosAntes = g_fallos;
    std::printf("PRUEBA %2d: %s\n", g_pruebas, nombre);
}
static void terminar() {
    if (g_fallos == g_fallosAntes) std::printf("           OK\n");
    else g_pruebasFalladas++;
}

// Grafo pequeno de ayuda:  0-1 (1), 1-2 (2), 0-2 (10), nodo 3 aislado
static Grafo grafoAyuda() {
    Grafo g(4);
    g.agregarArista(0, 1, 1);
    g.agregarArista(1, 2, 2);
    g.agregarArista(0, 2, 10);
    return g;
}

int main() {
    std::printf("=== Pruebas de Fantasmas de Marte ===\n\n");

    // ---------------------------------------------------------------
    empezar("Pila: orden LIFO y pila vacia (caso limite)");
    {
        Pila<int> p;
        int x = -1;
        VERIFICAR(p.vacia());
        VERIFICAR(!p.desapilar(x));          // desapilar en pila vacia no se rompe
        VERIFICAR(!p.ver(x));
        p.apilar(1); p.apilar(2); p.apilar(3);
        VERIFICAR(p.tamano() == 3);
        VERIFICAR(p.ver(x) && x == 3);
        VERIFICAR(p.desapilar(x) && x == 3);
        VERIFICAR(p.desapilar(x) && x == 2);
        VERIFICAR(p.desapilar(x) && x == 1);
        VERIFICAR(p.vacia() && !p.desapilar(x));
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Cola: orden FIFO, cola vacia y reutilizacion");
    {
        Cola<int> c;
        int x = -1;
        VERIFICAR(!c.desencolar(x));
        c.encolar(10); c.encolar(20); c.encolar(30);
        VERIFICAR(c.desencolar(x) && x == 10);
        VERIFICAR(c.desencolar(x) && x == 20);
        c.encolar(40);                       // encolar despues de desencolar
        VERIFICAR(c.desencolar(x) && x == 30);
        VERIFICAR(c.desencolar(x) && x == 40);
        VERIFICAR(c.vacia() && !c.desencolar(x));
        c.encolar(7);                        // la cola vuelve a servir tras vaciarse
        VERIFICAR(c.desencolar(x) && x == 7);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("MinHeap: extrae en orden, con repetidos, y heap vacio");
    {
        MinHeap<ParDist> h;
        ParDist x;
        VERIFICAR(!h.extraerMin(x));
        double valores[] = { 5, 3, 8, 1, 9, 2, 3, 3, 0.5 };
        for (int i = 0; i < 9; i++) h.insertar(ParDist(valores[i], i));
        VERIFICAR(h.tamano() == 9);
        double anterior = -1;
        bool ordenado = true;
        while (h.extraerMin(x)) {
            if (x.d < anterior) ordenado = false;
            anterior = x.d;
        }
        VERIFICAR(ordenado);
        VERIFICAR(h.vacio());
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("TablaHash: insertar, buscar, actualizar, ausente y muchas claves");
    {
        TablaHash<int> t(2);                 // capacidad chica para forzar colisiones y redimensionar
        int v = -1;
        VERIFICAR(!t.buscar("nada", v));
        VERIFICAR(!t.buscar("", v));         // clave vacia no existe
        t.insertar("viking 1", 1);
        t.insertar("spirit", 4);
        VERIFICAR(t.buscar("viking 1", v) && v == 1);
        t.insertar("spirit", 44);            // misma clave: actualiza, no duplica
        VERIFICAR(t.buscar("spirit", v) && v == 44);
        VERIFICAR(t.tamano() == 2);
        bool todas = true;
        for (int i = 0; i < 500; i++) t.insertar("clave" + std::to_string(i), i);
        for (int i = 0; i < 500; i++)
            if (!t.buscar("clave" + std::to_string(i), v) || v != i) todas = false;
        VERIFICAR(todas);
        VERIFICAR(t.tamano() == 502);
        VERIFICAR(t.capacidad() > 2);        // se redimensiono
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Grafo: ruta normal (Dijkstra prefiere 1+2 sobre la arista directa de 10)");
    {
        Grafo g = grafoAyuda();
        ArregloDinamico<int> ruta;
        double d;
        VERIFICAR(g.rutaMasCorta(0, 2, ruta, d));
        VERIFICAR(d == 3);
        VERIFICAR(ruta.tamano() == 3 && ruta[0] == 0 && ruta[1] == 1 && ruta[2] == 2);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Grafo: ruta de un sitio a si mismo (distancia 0)");
    {
        Grafo g = grafoAyuda();
        ArregloDinamico<int> ruta;
        double d = -1;
        VERIFICAR(g.rutaMasCorta(1, 1, ruta, d));
        VERIFICAR(d == 0 && ruta.tamano() == 1 && ruta[0] == 1);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Grafo: nodo aislado y nodos invalidos (sin ruta, sin caerse)");
    {
        Grafo g = grafoAyuda();
        ArregloDinamico<int> ruta, saltos;
        double d;
        VERIFICAR(!g.rutaMasCorta(0, 3, ruta, d));    // 3 esta aislado
        VERIFICAR(ruta.vacio());
        VERIFICAR(!g.rutaMasCorta(-1, 2, ruta, d));
        VERIFICAR(!g.rutaMasCorta(0, 99, ruta, d));
        g.bfs(0, saltos);
        VERIFICAR(saltos[3] == -1);                   // BFS tampoco lo alcanza
        VERIFICAR(!g.esConexo());
        VERIFICAR(!g.agregarArista(0, 0, 5));         // lazo no permitido
        VERIFICAR(!g.agregarArista(0, 1, 5));         // duplicada
        VERIFICAR(!g.agregarArista(0, 2, -4));        // peso negativo
        VERIFICAR(!g.agregarArista(0, 50, 4));        // nodo inexistente
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Grafo: dos rutas con el mismo peso (empate, resultado determinista)");
    {
        Grafo g(4);                                   // A=0 B=1 C=2 D=3
        g.agregarArista(0, 1, 1);
        g.agregarArista(0, 2, 1);
        g.agregarArista(1, 3, 1);
        g.agregarArista(2, 3, 1);
        ArregloDinamico<int> r1, r2;
        double d1, d2;
        VERIFICAR(g.rutaMasCorta(0, 3, r1, d1) && g.rutaMasCorta(0, 3, r2, d2));
        VERIFICAR(d1 == 2 && d2 == 2 && r1.tamano() == 3);
        VERIFICAR(r1[1] == r2[1]);                    // siempre elige la misma
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Grafo: vacio y de un solo nodo");
    {
        Grafo vacio(0);
        ArregloDinamico<int> ruta, saltos;
        double d;
        VERIFICAR(vacio.numNodos() == 0 && vacio.esConexo());
        VERIFICAR(!vacio.rutaMasCorta(0, 0, ruta, d));
        vacio.bfs(0, saltos);
        VERIFICAR(saltos.vacio());
        Grafo uno(1);
        VERIFICAR(uno.esConexo());
        VERIFICAR(uno.rutaMasCorta(0, 0, ruta, d) && d == 0);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("BFS: saltos correctos (0-1-2 en linea, con atajo)");
    {
        Grafo g = grafoAyuda();
        ArregloDinamico<int> s;
        g.bfs(0, s);
        VERIFICAR(s[0] == 0 && s[1] == 1 && s[2] == 1);   // 2 esta a UN salto por la arista directa
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Catalogo: carga de datos, busqueda por nombre/alias/mayusculas");
    Catalogo cat;
    {
        bool cargo = cat.cargarAutomatico();
        if (!cargo) std::printf("      %s\n", cat.error.c_str());
        VERIFICAR(cargo);
        VERIFICAR(cat.sitios.tamano() == 8);
        VERIFICAR(cat.buscar("Viking 1") == cat.buscar("viking1"));   // nombre = id
        VERIFICAR(cat.buscar("Viking 1") != -1);
        VERIFICAR(cat.buscar("  SOJOURNER ") == cat.buscar("pathfinder"));   // alias, mayusculas, espacios
        VERIFICAR(cat.buscar("curiosity") == -1);                     // no esta en el catalogo
        VERIFICAR(cat.buscar("") == -1);                              // texto vacio
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Catalogo: el grafo es conexo y la ruta nunca es mas corta que la distancia directa");
    {
        VERIFICAR(cat.grafo.esConexo());
        VERIFICAR(cat.grafo.numNodos() == 8);
        bool desigualdad = true, simetria = true;
        for (int a = 0; a < 8; a++) {
            for (int b = 0; b < 8; b++) {
                ArregloDinamico<int> ruta, rutaInv;
                double d, dInv;
                if (!cat.grafo.rutaMasCorta(a, b, ruta, d)) { desigualdad = false; continue; }
                cat.grafo.rutaMasCorta(b, a, rutaInv, dInv);
                if (d + 1e-6 < cat.distanciaDirecta(a, b)) desigualdad = false;
                if (d - dInv > 1e-6 || dInv - d > 1e-6) simetria = false;
            }
        }
        VERIFICAR(desigualdad);
        VERIFICAR(simetria);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Haversine: puntos iguales = 0 y puntos opuestos = media circunferencia");
    {
        VERIFICAR(haversineKm(10, 20, 10, 20) < 1e-9);
        double mediaCirc = 3.14159265358979 * 3389.5;
        double d = haversineKm(0, 0, 0, 180);
        VERIFICAR(d > mediaCirc - 1 && d < mediaCirc + 1);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Rutas tematicas: paradas validas y respuesta correcta dentro de rango");
    {
        VERIFICAR(cat.rutas.tamano() >= 3);
        bool bien = true;
        for (int r = 0; r < cat.rutas.tamano(); r++) {
            const RutaTematica& rt = cat.rutas[r];
            if (rt.paradas.tamano() < 2) bien = false;
            if (rt.correcta < 0 || rt.correcta >= rt.opciones.tamano()) bien = false;
            for (int i = 0; i + 1 < rt.paradas.tamano(); i++) {
                ArregloDinamico<int> camino;
                double km;
                if (!cat.grafo.rutaMasCorta(rt.paradas[i], rt.paradas[i + 1], camino, km)) bien = false;
            }
        }
        VERIFICAR(bien);
    }
    terminar();

    // ---------------------------------------------------------------
    empezar("Catalogo: carpeta inexistente da error en vez de caerse");
    {
        Catalogo malo;
        VERIFICAR(!malo.cargarCarpeta("carpeta_que_no_existe/"));
        VERIFICAR(!malo.error.empty());
    }
    terminar();

    std::printf("\n=== Resultado: %d pruebas, %d verificaciones, %d fallos ===\n",
                g_pruebas, g_verificaciones, g_fallos);
    if (g_fallos == 0) std::printf("TODO OK\n");
    else std::printf("HAY %d PRUEBA(S) CON FALLOS\n", g_pruebasFalladas);
    return g_fallos == 0 ? 0 : 1;
}
