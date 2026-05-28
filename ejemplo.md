# md-editor

Un visor de **Markdown** escrito en *Qt6 / C++*.

## Características de esta versión

- Abre y **renderiza** archivos Markdown.
- Se adapta al redimensionar la ventana.
- Resuelve enlaces e imágenes relativas.

### Ejemplo de lista numerada

1. Primer elemento
2. Segundo elemento
3. Tercer elemento

### Bloque de código

```cpp
#include <string>

// Punto de entrada del programa
int main() {
    std::string saludo = "Hola, mundo";  /* comentario */
    int n = 42;
    return 0;
}
```

### Lista de tareas

- [x] Leer archivos
- [x] Editar con formato
- [ ] Exportar a PDF

### Lista anidada

- Nivel 1
    - Nivel 2
        - Nivel 3

### Cita

> Esto es una cita para comprobar el formato.

---

### Tabla

| Función        | Estado     |
|----------------|------------|
| Leer archivos  | ✅ Hecho   |
| Editar         | 🚧 Próximo |

Un [enlace de ejemplo](https://www.qt.io) y texto final.
