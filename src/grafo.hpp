// =====================================================================
//  grafo.hpp  -  Grafo ponderado NO dirigido (lista de adyacencia)
//
//  Nodos   = sitios de Marte (0 .. n-1)
//  Aristas = distancia en km entre sitios cercanos
//
//  Complejidades (V = nodos, E = aristas):
//    agregarArista      O(grado)   (revisa que no exista duplicada)
//    dijkstra           O((V + E) log V)   con MinHeap propio
//    rutaMasCorta       O((V + E) log V)
//    bfs                O(V + E)
//    esConexo           O(V + E)
//    espacio            O(V + E)
// =====================================================================
#ifndef GRAFO_HPP
#define GRAFO_HPP

#include "estructuras.hpp"

struct Arista {
    int    destino;
    double peso;
    Arista() : destino(0), peso(0) {}
    Arista(int d, double p) : destino(d), peso(p) {}
};

// Par (distancia, nodo) para el heap de Dijkstra.
// Si hay empate en distancia, se desempata por numero de nodo
// para que el resultado sea siempre el mismo (determinista).
struct ParDist {
    double d;
    int    v;
    ParDist() : d(0), v(0) {}
    ParDist(double dd, int vv) : d(dd), v(vv) {}
    bool operator<(const ParDist& o) const {
        if (d != o.d) return d < o.d;
        return v < o.v;
    }
};

class Grafo {
public:
    static constexpr double INF = 1e18;

    explicit Grafo(int n = 0) : n_(0) { inicializar(n); }

    void inicializar(int n) {
        ady_ = ArregloDinamico<ArregloDinamico<Arista> >();
        n_ = (n < 0) ? 0 : n;
        for (int i = 0; i < n_; i++) ady_.agregar(ArregloDinamico<Arista>());
    }

    int  numNodos() const { return n_; }
    bool nodoValido(int u) const { return u >= 0 && u < n_; }

    int numAristas() const {
        int total = 0;
        for (int i = 0; i < n_; i++) total += ady_[i].tamano();
        return total / 2;
    }

    const ArregloDinamico<Arista>& vecinos(int u) const { return ady_[u]; }

    bool existeArista(int u, int v) const {
        if (!nodoValido(u) || !nodoValido(v)) return false;
        for (int i = 0; i < ady_[u].tamano(); i++)
            if (ady_[u][i].destino == v) return true;
        return false;
    }

    // Devuelve false si los nodos no son validos, es un lazo (u == v),
    // el peso es negativo o la arista ya existe.
    bool agregarArista(int u, int v, double peso) {
        if (!nodoValido(u) || !nodoValido(v)) return false;
        if (u == v || peso < 0) return false;
        if (existeArista(u, v)) return false;
        ady_[u].agregar(Arista(v, peso));
        ady_[v].agregar(Arista(u, peso));
        return true;
    }

    // ---------- Dijkstra ----------
    // Llena dist[] (INF si no alcanzable) y previo[] (-1 si no hay).
    // Los pesos no pueden ser negativos (agregarArista lo impide).
    void dijkstra(int origen, ArregloDinamico<double>& dist,
                  ArregloDinamico<int>& previo) const {
        dist.limpiar();
        previo.limpiar();
        for (int i = 0; i < n_; i++) { dist.agregar(INF); previo.agregar(-1); }
        if (!nodoValido(origen)) return;

        dist[origen] = 0;
        MinHeap<ParDist> heap;
        heap.insertar(ParDist(0, origen));

        ParDist actual;
        while (heap.extraerMin(actual)) {
            if (actual.d > dist[actual.v]) continue;   // entrada vieja: se ignora
            const ArregloDinamico<Arista>& vec = ady_[actual.v];
            for (int i = 0; i < vec.tamano(); i++) {
                int    w  = vec[i].destino;
                double nd = actual.d + vec[i].peso;
                if (nd < dist[w]) {
                    dist[w] = nd;
                    previo[w] = actual.v;
                    heap.insertar(ParDist(nd, w));
                }
            }
        }
    }

    // Ruta mas corta de a a b. Devuelve false si algun nodo no es valido
    // o si no existe camino (nodo aislado / otra componente).
    bool rutaMasCorta(int a, int b, ArregloDinamico<int>& ruta, double& distancia) const {
        ruta.limpiar();
        distancia = 0;
        if (!nodoValido(a) || !nodoValido(b)) return false;

        ArregloDinamico<double> dist;
        ArregloDinamico<int> previo;
        dijkstra(a, dist, previo);
        if (dist[b] >= INF) return false;

        for (int v = b; v != -1; v = previo[v]) ruta.agregar(v);
        ruta.invertir();
        distancia = dist[b];
        return true;
    }

    // ---------- BFS ----------
    // saltos[v] = numero minimo de aristas desde origen (-1 si no alcanzable).
    void bfs(int origen, ArregloDinamico<int>& saltos) const {
        saltos.limpiar();
        for (int i = 0; i < n_; i++) saltos.agregar(-1);
        if (!nodoValido(origen)) return;

        Cola<int> cola;
        saltos[origen] = 0;
        cola.encolar(origen);
        int u;
        while (cola.desencolar(u)) {
            const ArregloDinamico<Arista>& vec = ady_[u];
            for (int i = 0; i < vec.tamano(); i++) {
                int v = vec[i].destino;
                if (saltos[v] == -1) {
                    saltos[v] = saltos[u] + 1;
                    cola.encolar(v);
                }
            }
        }
    }

    // Un grafo vacio o de un solo nodo se considera conexo.
    bool esConexo() const {
        if (n_ <= 1) return true;
        ArregloDinamico<int> saltos;
        bfs(0, saltos);
        for (int i = 0; i < n_; i++)
            if (saltos[i] == -1) return false;
        return true;
    }

private:
    ArregloDinamico<ArregloDinamico<Arista> > ady_;
    int n_;
};

#endif
