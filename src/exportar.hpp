// =====================================================================
//  exportar.hpp  -  Escribe web/datos.js con TODO lo que calculo el C++:
//    - sitios y aristas del grafo
//    - ruta mas corta (Dijkstra) entre cada par de sitios
//    - saltos (BFS) entre cada par de sitios
//    - rutas tematicas con la distancia real de cada tramo
//  La pagina web (web/index.html) solo MUESTRA estos resultados.
// =====================================================================
#ifndef EXPORTAR_HPP
#define EXPORTAR_HPP

#include <cstdio>
#include <string>
#include "catalogo.hpp"

inline void jsonTexto(FILE* f, const std::string& s) {
    fputc('"', f);
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"')       fputs("\\\"", f);
        else if (c == '\\') fputs("\\\\", f);
        else if (c == '\n') fputs("\\n", f);
        else if (c == '\r') {}
        else if (c < 0x20)  fputc(' ', f);
        else                fputc(c, f);          // UTF-8 se escribe tal cual
    }
    fputc('"', f);
}

inline void jsonCamino(FILE* f, const Catalogo& cat, const ArregloDinamico<int>& ruta) {
    fputc('[', f);
    for (int i = 0; i < ruta.tamano(); i++) {
        if (i) fputc(',', f);
        jsonTexto(f, cat.sitios[ruta[i]].id);
    }
    fputc(']', f);
}

inline bool exportarDatosJS(const Catalogo& cat, const std::string& rutaArchivo) {
    FILE* f = fopen(rutaArchivo.c_str(), "wb");
    if (!f) return false;
    int n = cat.sitios.tamano();

    fputs("// Archivo generado por el programa en C++ (./fantasmas --exportar). No editar a mano.\n", f);
    fputs("window.DATOS = {\n", f);

    // --- sitios ---
    fputs("\"sitios\": [\n", f);
    for (int i = 0; i < n; i++) {
        const Sitio& s = cat.sitios[i];
        fputs("  {", f);
        fputs("\"id\":", f);             jsonTexto(f, s.id);
        fputs(",\"nombre\":", f);        jsonTexto(f, s.nombre);
        fputs(",\"tipo\":", f);          jsonTexto(f, s.tipo);
        fprintf(f, ",\"lat\":%.4f,\"lon\":%.4f", s.lat, s.lon);
        fputs(",\"region\":", f);        jsonTexto(f, s.region);
        fputs(",\"inicio\":", f);        jsonTexto(f, s.inicio);
        fputs(",\"fin\":", f);           jsonTexto(f, s.fin);
        fputs(",\"gancho\":", f);        jsonTexto(f, s.gancho);
        fputs(",\"comoFuncionaba\":", f);jsonTexto(f, s.comoFuncionaba);
        fputs(",\"descubrimiento\":", f);jsonTexto(f, s.descubrimiento);
        fputs(",\"desenlace\":", f);     jsonTexto(f, s.desenlace);
        fputs(",\"herencia\":", f);      jsonTexto(f, s.herencia);
        fputs(i + 1 < n ? "},\n" : "}\n", f);
    }
    fputs("],\n", f);

    // --- aristas (cada una una sola vez) ---
    fputs("\"aristas\": [\n", f);
    bool primera = true;
    for (int u = 0; u < n; u++) {
        const ArregloDinamico<Arista>& vec = cat.grafo.vecinos(u);
        for (int k = 0; k < vec.tamano(); k++) {
            int v = vec[k].destino;
            if (v < u) continue;
            if (!primera) fputs(",\n", f);
            primera = false;
            fputs("  {\"a\":", f); jsonTexto(f, cat.sitios[u].id);
            fputs(",\"b\":", f);   jsonTexto(f, cat.sitios[v].id);
            fprintf(f, ",\"km\":%.1f}", vec[k].peso);
        }
    }
    fputs("\n],\n", f);

    // --- distancias: Dijkstra entre todos los pares ---
    fputs("\"distancias\": {\n", f);
    for (int a = 0; a < n; a++) {
        fputs("  ", f); jsonTexto(f, cat.sitios[a].id); fputs(": {", f);
        bool pr = true;
        for (int b = 0; b < n; b++) {
            ArregloDinamico<int> ruta;
            double km;
            if (!cat.grafo.rutaMasCorta(a, b, ruta, km)) continue;
            if (!pr) fputc(',', f);
            pr = false;
            jsonTexto(f, cat.sitios[b].id);
            // km = distancia por la red de sitios (Dijkstra); directo = linea recta sobre Marte
            fprintf(f, ":{\"km\":%.1f,\"directo\":%.1f,\"camino\":", km, cat.distanciaDirecta(a, b));
            jsonCamino(f, cat, ruta);
            fputc('}', f);
        }
        fputs(a + 1 < n ? "},\n" : "}\n", f);
    }
    fputs("},\n", f);

    // --- saltos: BFS desde cada sitio ---
    fputs("\"saltos\": {\n", f);
    for (int a = 0; a < n; a++) {
        ArregloDinamico<int> saltos;
        cat.grafo.bfs(a, saltos);
        fputs("  ", f); jsonTexto(f, cat.sitios[a].id); fputs(": {", f);
        for (int b = 0; b < n; b++) {
            if (b) fputc(',', f);
            jsonTexto(f, cat.sitios[b].id);
            fprintf(f, ":%d", saltos[b]);
        }
        fputs(a + 1 < n ? "},\n" : "}\n", f);
    }
    fputs("},\n", f);

    // --- rutas tematicas ---
    fputs("\"rutas\": [\n", f);
    int nr = cat.rutas.tamano();
    for (int r = 0; r < nr; r++) {
        const RutaTematica& rt = cat.rutas[r];
        fputs("  {\"id\":", f);          jsonTexto(f, rt.id);
        fputs(",\"titulo\":", f);        jsonTexto(f, rt.titulo);
        fputs(",\"descripcion\":", f);   jsonTexto(f, rt.descripcion);
        fputs(",\"paradas\":[", f);
        for (int i = 0; i < rt.paradas.tamano(); i++) {
            if (i) fputc(',', f);
            jsonTexto(f, cat.sitios[rt.paradas[i]].id);
        }
        fputs("],\"tramos\":[", f);
        double total = 0;
        for (int i = 0; i + 1 < rt.paradas.tamano(); i++) {
            // En las rutas tematicas el tramo es la distancia directa entre paradas.
            double km = cat.distanciaDirecta(rt.paradas[i], rt.paradas[i + 1]);
            total += km;
            if (i) fputc(',', f);
            fputs("{\"de\":", f);  jsonTexto(f, cat.sitios[rt.paradas[i]].id);
            fputs(",\"a\":", f);   jsonTexto(f, cat.sitios[rt.paradas[i + 1]].id);
            fprintf(f, ",\"km\":%.1f}", km);
        }
        fprintf(f, "],\"totalKm\":%.1f", total);
        fputs(",\"pregunta\":", f);      jsonTexto(f, rt.pregunta);
        fputs(",\"opciones\":[", f);
        for (int i = 0; i < rt.opciones.tamano(); i++) {
            if (i) fputc(',', f);
            jsonTexto(f, rt.opciones[i]);
        }
        fprintf(f, "],\"correcta\":%d", rt.correcta);
        fputs(",\"explicacion\":", f);   jsonTexto(f, rt.explicacion);
        fputs(r + 1 < nr ? "},\n" : "}\n", f);
    }
    fputs("]\n};\n", f);

    fclose(f);
    return true;
}

#endif
