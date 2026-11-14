// =====================================================================
//  estructuras.hpp  -  Estructuras de datos PROPIAS del equipo MING
//  (sin usar std::vector, std::queue, std::priority_queue ni std::unordered_map)
//
//  Contenido:
//    ArregloDinamico<T>  arreglo que crece solo           (lineal)
//    Pila<T>             LIFO con nodos enlazados          (lineal)
//    Cola<T>             FIFO con nodos enlazados          (lineal)
//    MinHeap<T>          monticulo minimo                  (arbol, no lineal)
//    TablaHash<V>        tabla hash con encadenamiento     (lineal + listas)
// =====================================================================
#ifndef ESTRUCTURAS_HPP
#define ESTRUCTURAS_HPP

#include <string>
#include <cassert>

// ---------------------------------------------------------------------
//  ArregloDinamico<T>
//  agregar: O(1) amortizado | acceso []: O(1) | espacio: O(n)
// ---------------------------------------------------------------------
template <typename T>
class ArregloDinamico {
public:
    ArregloDinamico() : datos_(new T[4]), tam_(0), cap_(4) {}

    ArregloDinamico(const ArregloDinamico& o)
        : datos_(new T[o.cap_]), tam_(o.tam_), cap_(o.cap_) {
        for (int i = 0; i < tam_; i++) datos_[i] = o.datos_[i];
    }

    ArregloDinamico& operator=(const ArregloDinamico& o) {
        if (this != &o) {
            T* nuevo = new T[o.cap_];
            for (int i = 0; i < o.tam_; i++) nuevo[i] = o.datos_[i];
            delete[] datos_;
            datos_ = nuevo;
            tam_ = o.tam_;
            cap_ = o.cap_;
        }
        return *this;
    }

    ~ArregloDinamico() { delete[] datos_; }

    void agregar(const T& x) {
        if (tam_ == cap_) crecer();
        datos_[tam_++] = x;
    }

    T& operator[](int i) {
        assert(i >= 0 && i < tam_);
        return datos_[i];
    }
    const T& operator[](int i) const {
        assert(i >= 0 && i < tam_);
        return datos_[i];
    }

    int  tamano() const { return tam_; }
    bool vacio()  const { return tam_ == 0; }
    void limpiar()      { tam_ = 0; }

    // Quita el ultimo elemento: O(1). Devuelve false si esta vacio.
    bool quitarUltimo() {
        if (tam_ == 0) return false;
        tam_--;
        return true;
    }

    // Invierte el orden de los elementos: O(n)
    void invertir() {
        for (int i = 0, j = tam_ - 1; i < j; i++, j--) {
            T t = datos_[i];
            datos_[i] = datos_[j];
            datos_[j] = t;
        }
    }

private:
    void crecer() {
        int nueva = cap_ * 2;
        T* nuevo = new T[nueva];
        for (int i = 0; i < tam_; i++) nuevo[i] = datos_[i];
        delete[] datos_;
        datos_ = nuevo;
        cap_ = nueva;
    }

    T*  datos_;
    int tam_;
    int cap_;
};

// ---------------------------------------------------------------------
//  Pila<T>  (LIFO)  -  apilar / desapilar / ver: O(1)
//  Uso en el proyecto: historial de sitios visitados ("regresar").
// ---------------------------------------------------------------------
template <typename T>
class Pila {
public:
    Pila() : tope_(nullptr), tam_(0) {}
    ~Pila() { limpiar(); }
    Pila(const Pila&) = delete;
    Pila& operator=(const Pila&) = delete;

    void apilar(const T& x) {
        tope_ = new Nodo(x, tope_);
        tam_++;
    }

    // Devuelve false si la pila esta vacia (sin lanzar excepciones).
    bool desapilar(T& salida) {
        if (tope_ == nullptr) return false;
        Nodo* n = tope_;
        salida = n->dato;
        tope_ = n->sig;
        delete n;
        tam_--;
        return true;
    }

    bool ver(T& salida) const {
        if (tope_ == nullptr) return false;
        salida = tope_->dato;
        return true;
    }

    int  tamano() const { return tam_; }
    bool vacia()  const { return tam_ == 0; }

    void limpiar() {
        T basura;
        while (desapilar(basura)) {}
    }

private:
    struct Nodo {
        T dato;
        Nodo* sig;
        Nodo(const T& d, Nodo* s) : dato(d), sig(s) {}
    };
    Nodo* tope_;
    int   tam_;
};

// ---------------------------------------------------------------------
//  Cola<T>  (FIFO)  -  encolar / desencolar: O(1)
//  Uso en el proyecto: recorrido BFS.
// ---------------------------------------------------------------------
template <typename T>
class Cola {
public:
    Cola() : frente_(nullptr), final_(nullptr), tam_(0) {}
    ~Cola() { limpiar(); }
    Cola(const Cola&) = delete;
    Cola& operator=(const Cola&) = delete;

    void encolar(const T& x) {
        Nodo* n = new Nodo(x);
        if (final_ == nullptr) frente_ = final_ = n;
        else { final_->sig = n; final_ = n; }
        tam_++;
    }

    // Devuelve false si la cola esta vacia.
    bool desencolar(T& salida) {
        if (frente_ == nullptr) return false;
        Nodo* n = frente_;
        salida = n->dato;
        frente_ = n->sig;
        if (frente_ == nullptr) final_ = nullptr;
        delete n;
        tam_--;
        return true;
    }

    int  tamano() const { return tam_; }
    bool vacia()  const { return tam_ == 0; }

    void limpiar() {
        T basura;
        while (desencolar(basura)) {}
    }

private:
    struct Nodo {
        T dato;
        Nodo* sig;
        explicit Nodo(const T& d) : dato(d), sig(nullptr) {}
    };
    Nodo* frente_;
    Nodo* final_;
    int   tam_;
};

// ---------------------------------------------------------------------
//  MinHeap<T>  (monticulo binario minimo, guardado en un arreglo)
//  insertar: O(log n) | extraerMin: O(log n) | verMin: O(1)
//  T debe tener operator<.
//  Uso en el proyecto: cola de prioridad de Dijkstra.
// ---------------------------------------------------------------------
template <typename T>
class MinHeap {
public:
    void insertar(const T& x) {
        a_.agregar(x);
        subir(a_.tamano() - 1);
    }

    // Devuelve false si el heap esta vacio.
    bool extraerMin(T& salida) {
        if (a_.vacio()) return false;
        salida = a_[0];
        int ultimo = a_.tamano() - 1;
        a_[0] = a_[ultimo];
        a_.quitarUltimo();
        if (!a_.vacio()) bajar(0);
        return true;
    }

    bool verMin(T& salida) const {
        if (a_.vacio()) return false;
        salida = a_[0];
        return true;
    }

    int  tamano() const { return a_.tamano(); }
    bool vacio()  const { return a_.vacio(); }

private:
    void intercambiar(int i, int j) {
        T t = a_[i];
        a_[i] = a_[j];
        a_[j] = t;
    }

    void subir(int i) {
        while (i > 0) {
            int padre = (i - 1) / 2;
            if (a_[i] < a_[padre]) { intercambiar(i, padre); i = padre; }
            else break;
        }
    }

    void bajar(int i) {
        int n = a_.tamano();
        while (true) {
            int izq = 2 * i + 1, der = izq + 1, menor = i;
            if (izq < n && a_[izq] < a_[menor]) menor = izq;
            if (der < n && a_[der] < a_[menor]) menor = der;
            if (menor == i) break;
            intercambiar(i, menor);
            i = menor;
        }
    }

    ArregloDinamico<T> a_;
};

// ---------------------------------------------------------------------
//  TablaHash<V>  (clave std::string -> valor V, encadenamiento)
//  insertar / buscar: O(1) promedio, O(n) peor caso | espacio: O(n)
//  Se redimensiona (x2) cuando el factor de carga pasa de 0.75.
//  Uso en el proyecto: buscar un sitio por nombre, id o alias.
// ---------------------------------------------------------------------
template <typename V>
class TablaHash {
public:
    explicit TablaHash(int capacidadInicial = 16)
        : cap_(capacidadInicial < 1 ? 1 : capacidadInicial), tam_(0) {
        cubetas_ = new Nodo*[cap_];
        for (int i = 0; i < cap_; i++) cubetas_[i] = nullptr;
    }

    ~TablaHash() { liberar(); delete[] cubetas_; }
    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;

    // Si la clave ya existe, actualiza su valor.
    void insertar(const std::string& clave, const V& valor) {
        int idx = indice(clave, cap_);
        for (Nodo* n = cubetas_[idx]; n != nullptr; n = n->sig) {
            if (n->clave == clave) { n->valor = valor; return; }
        }
        cubetas_[idx] = new Nodo(clave, valor, cubetas_[idx]);
        tam_++;
        if (tam_ * 4 > cap_ * 3) redimensionar();
    }

    bool buscar(const std::string& clave, V& salida) const {
        int idx = indice(clave, cap_);
        for (Nodo* n = cubetas_[idx]; n != nullptr; n = n->sig) {
            if (n->clave == clave) { salida = n->valor; return true; }
        }
        return false;
    }

    bool contiene(const std::string& clave) const {
        V basura;
        return buscar(clave, basura);
    }

    int  tamano() const { return tam_; }
    int  capacidad() const { return cap_; }

    void limpiar() {
        liberar();
        tam_ = 0;
    }

private:
    struct Nodo {
        std::string clave;
        V valor;
        Nodo* sig;
        Nodo(const std::string& c, const V& v, Nodo* s) : clave(c), valor(v), sig(s) {}
    };

    // Funcion hash djb2
    static unsigned long hash(const std::string& s) {
        unsigned long h = 5381;
        for (size_t i = 0; i < s.size(); i++) h = h * 33 + (unsigned char)s[i];
        return h;
    }
    static int indice(const std::string& clave, int cap) {
        return (int)(hash(clave) % (unsigned long)cap);
    }

    void liberar() {
        for (int i = 0; i < cap_; i++) {
            Nodo* n = cubetas_[i];
            while (n != nullptr) { Nodo* sig = n->sig; delete n; n = sig; }
            cubetas_[i] = nullptr;
        }
    }

    void redimensionar() {
        int nuevaCap = cap_ * 2;
        Nodo** nuevas = new Nodo*[nuevaCap];
        for (int i = 0; i < nuevaCap; i++) nuevas[i] = nullptr;
        for (int i = 0; i < cap_; i++) {
            Nodo* n = cubetas_[i];
            while (n != nullptr) {
                Nodo* sig = n->sig;
                int idx = indice(n->clave, nuevaCap);
                n->sig = nuevas[idx];
                nuevas[idx] = n;
                n = sig;
            }
        }
        delete[] cubetas_;
        cubetas_ = nuevas;
        cap_ = nuevaCap;
    }

    Nodo** cubetas_;
    int cap_;
    int tam_;
};

#endif
