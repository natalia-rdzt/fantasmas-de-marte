// =====================================================================
//  FANTASMAS DE MARTE  -  Equipo MING
//  NASA Space Apps 2026 - reto "Abandoned but not Forgotten"
//
//  Programa de consola (el "motor"). La pagina web (web/index.html) muestra
//  lo que este programa calcula, exportado en web/datos.js
//
//  Compilar:  g++ -std=c++17 -O2 -o fantasmas src/main.cpp
//  Ejecutar:  ./fantasmas            (menu)
//             ./fantasmas --exportar (solo genera web/datos.js)
// =====================================================================
#include <iostream>
#include <string>
#include <cstdio>
#include "catalogo.hpp"
#include "exportar.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

static bool g_fin = false;   // se pone en true si se acaba la entrada (Ctrl+D / Ctrl+Z)

// ---------- entrada / salida ----------

static bool leerLinea(const std::string& mensaje, std::string& salida) {
    std::cout << mensaje << std::flush;
    if (!std::getline(std::cin, salida)) { g_fin = true; salida.clear(); return false; }
    salida = recortar(salida);
    return true;
}

static void pausa() {
    std::string basura;
    leerLinea("\n(Enter para continuar) ", basura);
}

// Acepta numero de la lista (1..n) o nombre / id / alias. Devuelve indice o -1.
static int pedirSitio(const Catalogo& cat, const std::string& mensaje) {
    std::string t;
    if (!leerLinea(mensaje, t) || t.empty()) return -1;
    if (esEntero(t)) {
        int k = std::atoi(t.c_str());
        if (k >= 1 && k <= cat.sitios.tamano()) return k - 1;
        std::cout << "  Numero fuera de la lista.\n";
        return -1;
    }
    int pos = cat.buscar(t);
    if (pos == -1) std::cout << "  No encontre \"" << t << "\". Prueba con el numero de la lista.\n";
    return pos;
}

static std::string kmTexto(double km) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f km", km);
    return buf;
}

static std::string nombresRuta(const Catalogo& cat, const ArregloDinamico<int>& ruta) {
    std::string s;
    for (int i = 0; i < ruta.tamano(); i++) {
        if (i) s += "  ->  ";
        s += cat.sitios[ruta[i]].nombre;
    }
    return s;
}

// ---------- pantallas ----------

static void listarSitios(const Catalogo& cat) {
    std::cout << "\n--- Los " << cat.sitios.tamano() << " sitios de Marte ---\n";
    for (int i = 0; i < cat.sitios.tamano(); i++) {
        const Sitio& s = cat.sitios[i];
        std::cout << "  " << (i + 1) << ". " << s.nombre << "  (" << s.tipo << ", "
                  << s.inicio << " - " << s.fin << ")\n";
    }
}

static void mostrarFicha(const Sitio& s) {
    std::cout << "\n==================== " << s.nombre << " ====================\n";
    std::cout << s.tipo << " | " << s.region << " | " << s.inicio << " -> " << s.fin << "\n\n";
    std::cout << "\"" << s.gancho << "\"\n\n";
    std::cout << "COMO FUNCIONABA\n  " << s.comoFuncionaba << "\n\n";
    std::cout << "LO QUE DESCUBRI\n  " << s.descubrimiento << "\n\n";
    std::cout << "COMO TERMINO\n  " << s.desenlace << "\n\n";
    std::cout << "LO QUE DEJE PARA LOS DEMAS\n  " << s.herencia << "\n";
}

static void mostrarFichaCorta(const Sitio& s) {
    std::cout << "\n* " << s.nombre << " (" << s.region << ")\n";
    std::cout << "  \"" << s.gancho << "\"\n";
    std::cout << "  Descubrimiento: " << s.descubrimiento << "\n";
}

// 2. Buscar con tabla hash
static void opcionBuscar(const Catalogo& cat, Pila<int>& historial) {
    int i = pedirSitio(cat, "Nombre, alias o numero del sitio: ");
    if (i == -1) return;
    mostrarFicha(cat.sitios[i]);
    historial.apilar(i);
}

// 3. Dijkstra
static void opcionRuta(const Catalogo& cat, Pila<int>& historial) {
    listarSitios(cat);
    int a = pedirSitio(cat, "\nOrigen: ");
    if (a == -1) return;
    int b = pedirSitio(cat, "Destino: ");
    if (b == -1) return;

    ArregloDinamico<int> ruta;
    double total;
    if (!cat.grafo.rutaMasCorta(a, b, ruta, total)) {
        std::cout << "  No hay ruta entre esos sitios.\n";
        return;
    }
    std::cout << "\nRuta mas corta por la red de sitios (Dijkstra): " << kmTexto(total) << "\n  " << nombresRuta(cat, ruta) << "\n";
    std::cout << "  (En linea recta sobre Marte serian " << kmTexto(cat.distanciaDirecta(a, b)) << ")\n";
    for (int i = 0; i + 1 < ruta.tamano(); i++) {
        std::cout << "    " << cat.sitios[ruta[i]].nombre << " -> " << cat.sitios[ruta[i + 1]].nombre
                  << ": " << kmTexto(cat.distanciaDirecta(ruta[i], ruta[i + 1])) << "\n";
    }
    std::cout << "\nParadas del camino:\n";
    for (int i = 0; i < ruta.tamano(); i++) mostrarFichaCorta(cat.sitios[ruta[i]]);
    historial.apilar(b);
}

// 4. BFS
static void opcionVecinos(const Catalogo& cat) {
    listarSitios(cat);
    int a = pedirSitio(cat, "\nSitio de partida: ");
    if (a == -1) return;
    std::string t;
    leerLinea("Cuantos saltos? (Enter = 1): ", t);
    int maxSaltos = esEntero(t) ? std::atoi(t.c_str()) : 1;

    ArregloDinamico<int> saltos;
    cat.grafo.bfs(a, saltos);
    std::cout << "\nDesde " << cat.sitios[a].nombre << " (BFS, hasta " << maxSaltos << " salto(s)):\n";
    bool alguno = false;
    for (int nivel = 1; nivel <= maxSaltos; nivel++) {
        for (int i = 0; i < cat.sitios.tamano(); i++) {
            if (saltos[i] == nivel) {
                std::cout << "  a " << nivel << " salto(s): " << cat.sitios[i].nombre << "\n";
                alguno = true;
            }
        }
    }
    if (!alguno) std::cout << "  (ninguno)\n";
}

// 5. Rutas tematicas
static void opcionRutaTematica(const Catalogo& cat, Pila<int>& historial) {
    std::cout << "\n--- Rutas tematicas ---\n";
    for (int i = 0; i < cat.rutas.tamano(); i++)
        std::cout << "  " << (i + 1) << ". " << cat.rutas[i].titulo << " - " << cat.rutas[i].descripcion << "\n";
    std::string t;
    if (!leerLinea("\nElige una ruta (numero): ", t) || !esEntero(t)) return;
    int r = std::atoi(t.c_str()) - 1;
    if (r < 0 || r >= cat.rutas.tamano()) { std::cout << "  Ruta no valida.\n"; return; }
    const RutaTematica& rt = cat.rutas[r];

    std::cout << "\nSojo, tu guia, te lleva por: " << rt.titulo << "\n";
    double total = 0;
    for (int i = 0; i < rt.paradas.tamano(); i++) {
        int s = rt.paradas[i];
        if (i > 0) {
            double km = cat.distanciaDirecta(rt.paradas[i - 1], s);   // linea recta sobre Marte
            total += km;
            std::cout << "\n   >> Viajamos " << kmTexto(km) << " en linea recta, de "
                      << cat.sitios[rt.paradas[i - 1]].nombre << " a " << cat.sitios[s].nombre << "\n";
        }
        mostrarFicha(cat.sitios[s]);
        historial.apilar(s);
        if (i + 1 < rt.paradas.tamano()) { pausa(); if (g_fin) return; }
    }
    std::cout << "\nRecorrido total de la ruta: " << kmTexto(total) << "\n";

    // pregunta de cierre
    std::cout << "\nPREGUNTA FINAL: " << rt.pregunta << "\n";
    for (int i = 0; i < rt.opciones.tamano(); i++)
        std::cout << "  " << (i + 1) << ") " << rt.opciones[i] << "\n";
    if (!leerLinea("Tu respuesta: ", t)) return;
    int resp = esEntero(t) ? std::atoi(t.c_str()) - 1 : -1;
    if (resp == rt.correcta) std::cout << "\nCorrecto!\n";
    else std::cout << "\nNo exactamente. La respuesta era: " << rt.opciones[rt.correcta] << "\n";
    std::cout << rt.explicacion << "\n";
}

// 6. Recorrido guiado: siempre al sitio no visitado mas cercano (por Dijkstra)
static void opcionRecorrido(const Catalogo& cat) {
    listarSitios(cat);
    int actual = pedirSitio(cat, "\nEmpezar en: ");
    if (actual == -1) return;
    int n = cat.sitios.tamano();
    ArregloDinamico<char> visitado;
    for (int i = 0; i < n; i++) visitado.agregar(0);
    visitado[actual] = 1;
    double total = 0;
    std::cout << "\nRecorrido guiado (siempre al mas cercano):\n  1. " << cat.sitios[actual].nombre << "\n";
    for (int paso = 2; paso <= n; paso++) {
        ArregloDinamico<double> dist;
        ArregloDinamico<int> previo;
        cat.grafo.dijkstra(actual, dist, previo);
        int mejor = -1;
        for (int j = 0; j < n; j++)
            if (!visitado[j] && dist[j] < Grafo::INF && (mejor == -1 || dist[j] < dist[mejor])) mejor = j;
        if (mejor == -1) break;
        total += dist[mejor];
        std::cout << "  " << paso << ". " << cat.sitios[mejor].nombre << "  (+" << kmTexto(dist[mejor]) << ")\n";
        visitado[mejor] = 1;
        actual = mejor;
    }
    std::cout << "Total: " << kmTexto(total) << "\n";
}

// 7. Pila: regresar
static void opcionRegresar(const Catalogo& cat, Pila<int>& historial) {
    if (historial.tamano() < 2) {
        std::cout << "  No hay un sitio anterior en el historial.\n";
        return;
    }
    int actual = 0;
    historial.desapilar(actual);          // quitamos el actual
    int anterior = 0;
    historial.ver(anterior);              // el nuevo tope es el anterior
    std::cout << "\nRegresamos a: " << cat.sitios[anterior].nombre << "\n";
    mostrarFicha(cat.sitios[anterior]);
}

static void opcionExportar(const Catalogo& cat, const std::string& destino) {
    const char* candidatas[] = { "web/datos.js", "../web/datos.js" };
    std::string ruta = destino;
    bool ok = false;
    if (!ruta.empty()) ok = exportarDatosJS(cat, ruta);
    else {
        for (int i = 0; i < 2 && !ok; i++) { ruta = candidatas[i]; ok = exportarDatosJS(cat, ruta); }
    }
    if (ok) std::cout << "  Listo: " << ruta << " (abre web/index.html en el navegador)\n";
    else std::cout << "  No pude escribir el archivo. Ejecuta desde la carpeta del proyecto.\n";
}

static void menu() {
    std::cout << "\n=========== FANTASMAS DE MARTE (equipo MING) ===========\n"
              << " 1. Ver los sitios\n"
              << " 2. Buscar un sitio por nombre (tabla hash)\n"
              << " 3. Ruta mas corta entre dos sitios (Dijkstra)\n"
              << " 4. Sitios a N saltos (BFS)\n"
              << " 5. Rutas tematicas con pregunta final\n"
              << " 6. Recorrido guiado de todos los sitios\n"
              << " 7. Regresar al sitio anterior (pila)\n"
              << " 8. Exportar datos para la pagina web\n"
              << " 0. Salir\n";
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);     // para ver acentos en Windows
#endif
    bool soloExportar = false;
    std::string carpeta, destino;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--exportar") {
            soloExportar = true;
            if (i + 1 < argc && std::string(argv[i + 1]).substr(0, 2) != "--") destino = argv[++i];
        } else if (a == "--carpeta" && i + 1 < argc) {
            carpeta = argv[++i];
            if (!carpeta.empty() && carpeta[carpeta.size() - 1] != '/') carpeta += '/';
        }
    }

    Catalogo cat;
    bool ok = carpeta.empty() ? cat.cargarAutomatico() : cat.cargarCarpeta(carpeta);
    if (!ok) { std::cerr << "Error: " << cat.error << "\n"; return 1; }

    if (soloExportar) {
        opcionExportar(cat, destino);
        return 0;
    }

    Pila<int> historial;
    std::cout << "\nHola, soy Sojo, tu guia. Hay " << cat.sitios.tamano()
              << " maquinas que la NASA dejo en Marte, y todas tienen una historia.\n"
              << "(Nota: yo, Sojo, soy un narrador inspirado en Sojourner; los datos son de misiones reales.)\n";

    while (!g_fin) {
        menu();
        std::string op;
        if (!leerLinea("Elige una opcion: ", op)) break;
        if (op == "0") break;
        else if (op == "1") listarSitios(cat);
        else if (op == "2") opcionBuscar(cat, historial);
        else if (op == "3") opcionRuta(cat, historial);
        else if (op == "4") opcionVecinos(cat);
        else if (op == "5") opcionRutaTematica(cat, historial);
        else if (op == "6") opcionRecorrido(cat);
        else if (op == "7") opcionRegresar(cat, historial);
        else if (op == "8") opcionExportar(cat, "");
        else std::cout << "  Opcion no valida.\n";
    }
    std::cout << "\nHasta pronto. Ningun explorador esta olvidado mientras alguien cuente su historia.\n";
    return 0;
}
