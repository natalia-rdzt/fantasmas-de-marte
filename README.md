# Fantasmas de Marte — equipo MING

NASA Space Apps 2026 · reto **"Abandoned but not Forgotten"** · proyecto final de Estructura de Datos.

Ocho máquinas que la NASA dejó en Marte (Viking 1 y 2, Pathfinder/Sojourner, Spirit, Opportunity, Phoenix, InSight, Ingenuity), contadas en primera persona por ellas mismas y guiadas por **Sojo**. El **motor** está en C++ (estructuras propias) y la **página web** muestra lo que el motor calcula.

```
fantasmas-de-marte/
├── src/
│   ├── estructuras.hpp   ArregloDinamico, Pila, Cola, MinHeap, TablaHash (propias)
│   ├── grafo.hpp         Grafo ponderado + Dijkstra + BFS
│   ├── catalogo.hpp      Carga de datos, tabla hash de búsqueda, construcción del grafo
│   ├── exportar.hpp      Escribe web/datos.js con los resultados del C++
│   └── main.cpp          Menú de consola
├── tests/tests.cpp       15 pruebas con casos límite
├── data/sitios.txt       Los 8 sitios (AQUÍ se editan los textos y datos)
├── data/rutas.txt        3 rutas temáticas + pregunta final
├── web/index.html        Página (mapa, rutas, conexiones)
├── web/datos.js          GENERADO por el C++ (no editar a mano)
└── Makefile
```

## Cómo correrlo

Requiere `g++` con C++17.

```bash
make              # compila ./fantasmas y ./pruebas
./pruebas         # corre las pruebas
./fantasmas       # menú de consola
./fantasmas --exportar   # regenera web/datos.js
```

Sin `make` (por ejemplo en Windows con MinGW):

```bash
g++ -std=c++17 -O2 -o fantasmas src/main.cpp
g++ -std=c++17 -O2 -o pruebas tests/tests.cpp
```

Ejecútenlo siempre **desde la carpeta del proyecto** (para que encuentre `data/`).
En la consola de Windows, si los acentos se ven raros: `chcp 65001`.

Para ver la página: abran `web/index.html` en el navegador. Funciona sin internet.

## Cómo se conectan el C++ y la página

1. El C++ lee `data/sitios.txt` y `data/rutas.txt`.
2. Construye el grafo (cada sitio con sus 3 vecinos más cercanos, distancias reales en km con la fórmula de Haversine, radio de Marte 3389.5 km) y lo hace conexo.
3. Corre **Dijkstra** entre todos los pares y **BFS** desde cada sitio.
4. Escribe todo en `web/datos.js`. La página solo lo muestra.

Al explicarlo, sean claros: la página **muestra resultados precalculados por el C++**, no ejecuta el C++ en vivo. Un siguiente paso sería un servidor local o compilar a WebAssembly.

## Estructuras y complejidad (V = sitios, E = aristas, n = elementos)

| Estructura | Dónde se usa | Operaciones | Tiempo | Espacio |
|---|---|---|---|---|
| ArregloDinamico | Listas de sitios, rutas, vecinos, base del heap | agregar / acceder | O(1) amortizado / O(1) | O(n) |
| Pila | Historial ("regresar") | apilar, desapilar | O(1) | O(n) |
| Cola | BFS | encolar, desencolar | O(1) | O(n) |
| MinHeap (árbol, no lineal) | Cola de prioridad de Dijkstra | insertar, extraerMin | O(log n) | O(n) |
| TablaHash (encadenamiento) | Buscar sitio por nombre, id o alias | insertar, buscar | O(1) promedio, O(n) peor caso | O(n) |
| **Grafo** (lista de adyacencia, no lineal) | Red de sitios | agregarArista | O(grado) | O(V + E) |
| Dijkstra | Ruta más corta por la red | — | O((V + E) log V) | O(V) |
| BFS | Sitios a N saltos | — | O(V + E) | O(V) |
| Construir el grafo | Al cargar los datos | k vecinos más cercanos | O(V² · k) | O(V + E) |

Estructuras no lineales: **grafo** y **heap**. Lineales: arreglo, pila, cola, tabla hash.

### Preguntas que les pueden hacer en el pitch: "¿qué pasaría si cambian la estructura?"

- **Tabla hash → arreglo con búsqueda lineal:** buscar un sitio pasaría de O(1) promedio a O(n). Con 8 sitios no se nota, con miles sí.
- **Heap en Dijkstra → buscar el mínimo recorriendo un arreglo:** Dijkstra pasaría de O((V + E) log V) a O(V²).
- **Lista de adyacencia → matriz de adyacencia:** el espacio pasaría de O(V + E) a O(V²), y recorrer los vecinos de un nodo costaría O(V) en vez de O(grado).
- **BFS con cola → con pila:** sería un DFS, que ya no garantiza el menor número de saltos.
- **Pila de historial → cola:** "regresar" iría al sitio más viejo y no al último visitado.

## Rúbrica

| Requisito | Dónde está |
|---|---|
| Al menos 2 estructuras, una no lineal | Grafo y MinHeap (no lineales) + arreglo, pila, cola, tabla hash |
| Implementación propia | `src/estructuras.hpp` y `src/grafo.hpp`; sin `std::vector`, `std::queue`, `std::priority_queue` ni `std::unordered_map` |
| Algoritmos | Dijkstra, BFS, búsqueda en tabla hash, recorrido guiado (vecino más cercano) |
| Big O | Tabla de arriba, y comentarios en el código |
| 5 pruebas con casos límite | `tests/tests.cpp`: 15 pruebas (pila/cola/heap vacíos, clave ausente, nodo aislado, grafo vacío, ruta a sí mismo, empate de pesos, nodos inválidos, carpeta inexistente...) |
| Video | **Pendiente.** Sugerencia: 1) `./pruebas`, 2) menú de consola (opciones 2, 3, 5), 3) la página web |
| PDF de diplomas | **Pendiente.** Juntar los diplomas de todo el equipo |
| Pitch final | **Pendiente.** Que cada integrante pueda explicar una estructura |

## Para el reto de la NASA

- Historia en **primera persona** de cada máquina, para 12-15 años.
- Cada ficha explica **cómo funcionaba**, qué descubrió, cómo terminó y qué dejó a las misiones siguientes.
- Tres rutas temáticas: *Sigue el agua*, *Las primeras veces*, *Cómo terminó su misión*, con una pregunta final cada una.
- **Licencia narrativa:** Sojourner nunca salió de Ares Vallis (no se alejó más de unos 12 m de su base). Por eso Sojo es un guía "inspirado" en él y la página lo aclara en el pie.

## Datos: lo verificado y lo que falta

Verificado en esta sesión contra fuentes en línea (NASA, Wikipedia, notas de prensa): fechas de aterrizaje y fin de misión, 2,245 soles de Viking 1, 83 días y ~100 m de Sojourner, 2,208 soles de Spirit, ~45 km de Opportunity, hielo de agua de Phoenix (31 jul 2008), 1,300+ sismos de InSight, 72 vuelos de Ingenuity, causas de fin de misión, y las coordenadas de Viking 2, Spirit, Opportunity, Phoenix e InSight.

**Por verificar antes del pitch** (una persona como "editora de datos", con 2 fuentes por dato):

1. Coordenadas de **Pathfinder** (19.13° N, 326.78° E) e **Ingenuity** (18.44° N, 77.45° E): no pude confirmarlas en línea en esta sesión.
2. Coordenadas de **Viking 1 y 2**: las fuentes difieren por menos de 1° según la convención (planetográfica o planetocéntrica). No cambia el mapa, pero conviene citar cuál usan.
3. Detalles de funcionamiento que son conocimiento general y no los confirmé en línea: brazo con pala de los Viking, seis ruedas y herramienta de raspado de Spirit y Opportunity, SEIS colocado sobre el suelo, y que Ingenuity usó a Perseverance como base de comunicación.
4. La fecha exacta de fin de Viking 2: las fuentes dicen 11 o 12 de abril de 1980 (el archivo dice "abr 1980").
5. Las frases de "lo que dejé para los demás" son interpretaciones educativas. Si las citan en el pitch como hecho, busquen la fuente.

Fuentes sugeridas: <https://science.nasa.gov> (página de cada misión) y <https://nssdc.gsfc.nasa.gov> (NSSDCA).

## Cómo agregar un sitio o una ruta

1. Agreguen una línea en `data/sitios.txt` (14 campos separados por `|`) o en `data/rutas.txt` (8 campos).
2. Corran `./fantasmas --exportar` y recarguen la página.

Si la línea tiene un error, el programa dice cuál (campos de más o de menos, parada desconocida, coordenadas fuera de rango).

## Mejoras si sobra tiempo

- Servidor local para que la página le pida rutas al C++ en vivo.
- Árbol AVL por fecha para una línea del tiempo con consultas por rango.
- Trie para autocompletar en un buscador.
- Más sitios: la Luna (Apolo, Surveyor, LRV) o el espacio profundo (Pioneer, Dawn, NEAR, Kepler).
