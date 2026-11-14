// =====================================================================
//  catalogo.hpp  -  Carga los datos (data/sitios.txt, data/rutas.txt),
//  construye la tabla hash de busqueda y el grafo de sitios.
// =====================================================================
#ifndef CATALOGO_HPP
#define CATALOGO_HPP

#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "estructuras.hpp"
#include "grafo.hpp"

// ---------- utilidades de texto ----------

inline std::string recortar(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
    while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) j--;
    return s.substr(i, j - i);
}

// Minusculas (solo ASCII) y sin espacios sobrantes: "  Viking 1 " -> "viking 1"
inline std::string normalizar(const std::string& s) {
    std::string r = recortar(s);
    for (size_t i = 0; i < r.size(); i++)
        if (r[i] >= 'A' && r[i] <= 'Z') r[i] = (char)(r[i] - 'A' + 'a');
    return r;
}

inline ArregloDinamico<std::string> separar(const std::string& s, char sep) {
    ArregloDinamico<std::string> partes;
    std::string actual;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == sep) { partes.agregar(actual); actual.clear(); }
        else actual += s[i];
    }
    partes.agregar(actual);
    return partes;
}

inline bool esEntero(const std::string& s) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

// Distancia sobre la superficie de Marte (circulo maximo), en km.
inline double haversineKm(double lat1, double lon1, double lat2, double lon2) {
    const double RADIO_MARTE_KM = 3389.5;
    const double RAD = 3.14159265358979323846 / 180.0;
    double dlat = (lat2 - lat1) * RAD;
    double dlon = (lon2 - lon1) * RAD;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1 * RAD) * std::cos(lat2 * RAD) *
               std::sin(dlon / 2) * std::sin(dlon / 2);
    if (a > 1) a = 1;
    if (a < 0) a = 0;
    return RADIO_MARTE_KM * 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
}

// ---------- datos ----------

struct Sitio {
    std::string id, nombre, alias, tipo, region, inicio, fin;
    std::string gancho, comoFuncionaba, descubrimiento, desenlace, herencia;
    double lat, lon;
    Sitio() : lat(0), lon(0) {}
};

struct RutaTematica {
    std::string id, titulo, descripcion;
    ArregloDinamico<int> paradas;                 // indices de sitios
    std::string pregunta;
    ArregloDinamico<std::string> opciones;
    int correcta;
    std::string explicacion;
    RutaTematica() : correcta(0) {}
};

class Catalogo {
public:
    ArregloDinamico<Sitio> sitios;
    ArregloDinamico<RutaTematica> rutas;
    TablaHash<int> indice;        // nombre / id / alias normalizado -> posicion del sitio
    Grafo grafo;                  // nodo i = sitios[i]
    std::string error;

    // Busca en las carpetas habituales: data/, ../data/
    bool cargarAutomatico() {
        const char* candidatas[] = { "data/", "../data/", "./" };
        for (int i = 0; i < 3; i++) {
            std::ifstream prueba((std::string(candidatas[i]) + "sitios.txt").c_str());
            if (prueba) return cargarCarpeta(candidatas[i]);
        }
        error = "No encontre data/sitios.txt. Ejecuta el programa desde la carpeta del proyecto.";
        return false;
    }

    // carpeta debe terminar en '/'
    bool cargarCarpeta(const std::string& carpeta) {
        sitios.limpiar();
        rutas.limpiar();
        indice.limpiar();
        error.clear();
        if (!cargarSitios(carpeta + "sitios.txt")) return false;
        construirGrafo(3);
        if (!cargarRutas(carpeta + "rutas.txt")) return false;
        return true;
    }

    // Devuelve el indice del sitio, o -1 si no existe.
    int buscar(const std::string& texto) const {
        int pos;
        if (indice.buscar(normalizar(texto), pos)) return pos;
        return -1;
    }

    double distanciaDirecta(int a, int b) const {
        return haversineKm(sitios[a].lat, sitios[a].lon, sitios[b].lat, sitios[b].lon);
    }

    // Conecta cada sitio con sus k vecinos mas cercanos (distancia real en km)
    // y despues garantiza que el grafo sea conexo uniendo componentes con la
    // arista mas corta posible.
    void construirGrafo(int k) {
        int n = sitios.tamano();
        grafo.inicializar(n);
        for (int i = 0; i < n; i++) {
            ArregloDinamico<char> usado;
            for (int j = 0; j < n; j++) usado.agregar(0);
            usado[i] = 1;
            for (int paso = 0; paso < k && paso < n - 1; paso++) {
                int mejor = -1;
                double mejorD = Grafo::INF;
                for (int j = 0; j < n; j++) {
                    if (usado[j]) continue;
                    double d = distanciaDirecta(i, j);
                    if (d < mejorD) { mejorD = d; mejor = j; }
                }
                if (mejor == -1) break;
                usado[mejor] = 1;
                grafo.agregarArista(i, mejor, mejorD);
            }
        }
        // Unir componentes
        while (!grafo.esConexo()) {
            ArregloDinamico<int> saltos;
            grafo.bfs(0, saltos);
            int bi = -1, bj = -1;
            double mejorD = Grafo::INF;
            for (int i = 0; i < n; i++) {
                if (saltos[i] == -1) continue;
                for (int j = 0; j < n; j++) {
                    if (saltos[j] != -1) continue;
                    double d = distanciaDirecta(i, j);
                    if (d < mejorD) { mejorD = d; bi = i; bj = j; }
                }
            }
            if (bi == -1) break;
            grafo.agregarArista(bi, bj, mejorD);
        }
    }

private:
    static bool leerLineas(const std::string& ruta, ArregloDinamico<std::string>& lineas) {
        std::ifstream f(ruta.c_str());
        if (!f) return false;
        std::string l;
        bool primera = true;
        while (std::getline(f, l)) {
            if (primera && l.size() >= 3 && (unsigned char)l[0] == 0xEF &&
                (unsigned char)l[1] == 0xBB && (unsigned char)l[2] == 0xBF)
                l = l.substr(3);                   // quitar BOM de UTF-8
            primera = false;
            l = recortar(l);
            if (l.empty() || l[0] == '#') continue;
            lineas.agregar(l);
        }
        return true;
    }

    bool registrarClave(const std::string& clave, int pos) {
        std::string k = normalizar(clave);
        if (k.empty()) return true;
        int existente;
        if (indice.buscar(k, existente) && existente != pos) {
            error = "Clave repetida entre dos sitios: " + k;
            return false;
        }
        indice.insertar(k, pos);
        return true;
    }

    bool cargarSitios(const std::string& ruta) {
        ArregloDinamico<std::string> lineas;
        if (!leerLineas(ruta, lineas)) { error = "No pude abrir " + ruta; return false; }
        for (int i = 0; i < lineas.tamano(); i++) {
            ArregloDinamico<std::string> c = separar(lineas[i], '|');
            if (c.tamano() != 14) {
                error = ruta + ": la linea " + std::to_string(i + 1) +
                        " debe tener 14 campos y tiene " + std::to_string(c.tamano());
                return false;
            }
            Sitio s;
            s.id = normalizar(c[0]);   s.nombre = recortar(c[1]); s.alias = recortar(c[2]);
            s.tipo = recortar(c[3]);
            s.lat = std::atof(c[4].c_str());   s.lon = std::atof(c[5].c_str());
            s.region = recortar(c[6]); s.inicio = recortar(c[7]); s.fin = recortar(c[8]);
            s.gancho = recortar(c[9]); s.comoFuncionaba = recortar(c[10]);
            s.descubrimiento = recortar(c[11]); s.desenlace = recortar(c[12]);
            s.herencia = recortar(c[13]);
            if (s.id.empty() || s.nombre.empty()) { error = ruta + ": sitio sin id o nombre"; return false; }
            if (s.lat < -90 || s.lat > 90 || s.lon < 0 || s.lon > 360) {
                error = ruta + ": coordenadas fuera de rango en " + s.nombre;
                return false;
            }
            int pos = sitios.tamano();
            sitios.agregar(s);
            if (!registrarClave(s.id, pos) || !registrarClave(s.nombre, pos)) return false;
            ArregloDinamico<std::string> al = separar(s.alias, ';');
            for (int j = 0; j < al.tamano(); j++)
                if (!registrarClave(al[j], pos)) return false;
        }
        if (sitios.tamano() == 0) { error = ruta + ": no hay sitios"; return false; }
        return true;
    }

    bool cargarRutas(const std::string& ruta) {
        ArregloDinamico<std::string> lineas;
        if (!leerLineas(ruta, lineas)) { error = "No pude abrir " + ruta; return false; }
        for (int i = 0; i < lineas.tamano(); i++) {
            ArregloDinamico<std::string> c = separar(lineas[i], '|');
            if (c.tamano() != 8) {
                error = ruta + ": la linea " + std::to_string(i + 1) +
                        " debe tener 8 campos y tiene " + std::to_string(c.tamano());
                return false;
            }
            RutaTematica r;
            r.id = normalizar(c[0]); r.titulo = recortar(c[1]); r.descripcion = recortar(c[2]);
            ArregloDinamico<std::string> ps = separar(c[3], ';');
            for (int j = 0; j < ps.tamano(); j++) {
                int pos = buscar(ps[j]);
                if (pos == -1) { error = ruta + ": parada desconocida '" + ps[j] + "'"; return false; }
                r.paradas.agregar(pos);
            }
            r.pregunta = recortar(c[4]);
            ArregloDinamico<std::string> ops = separar(c[5], ';');
            for (int j = 0; j < ops.tamano(); j++) r.opciones.agregar(recortar(ops[j]));
            r.correcta = std::atoi(c[6].c_str());
            if (r.correcta < 0 || r.correcta >= r.opciones.tamano()) {
                error = ruta + ": respuesta correcta fuera de rango en " + r.titulo;
                return false;
            }
            r.explicacion = recortar(c[7]);
            rutas.agregar(r);
        }
        return true;
    }
};

#endif
