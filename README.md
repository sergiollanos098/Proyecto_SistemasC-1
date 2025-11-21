# RISC-V Satellite Kernel 

Este repositorio contiene la implementación de un **Kernel Híbrido (C + RISC-V Assembly 32 bits)** diseñado para gestionar los sistemas críticos de un satélite en órbita LEO. El sistema simula la adquisición de temperatura, control de enfriamiento y transmisión de datos, operando en un entorno *Bare Metal* (sin sistema operativo) simulado sobre QEMU.

## Tabla de Contenidos
- [Descripción del Proyecto](#-descripción-del-proyecto)
- [Arquitectura del Sistema](#-arquitectura-del-sistema)
- [Requisitos Previos](#-requisitos-previos)
- [Instalación y Compilación](#-instalación-y-compilación)
- [Guía de Uso](#-guía-de-uso)
- [Escenarios de Prueba](#-escenarios-de-prueba)
- [Telemetría y Métricas](#-telemetría-y-métricas)

---

## Descripción del Proyecto

El objetivo es controlar tres procesos concurrentes en un satélite que orbita la tierra cada **100 minutos**:

1.  **Proceso 1 (Adquisición):** Lectura de sensores de temperatura.
2.  **Proceso 2 (Control):** Activación de escudos/enfriamiento si $T > 90^\circ C$, con desactivación a $T < 60^\circ C$.
3.  **Proceso 3 (Transmisión):** Envío de telemetría a tierra (UART).

El sistema debe ser capaz de detectar **anomalías críticas** ($T > 105^\circ C$) y reportar la dirección de memoria exacta (PC) donde ocurrió el evento.

---

## Arquitectura del Sistema

Se ha implementado una **arquitectura jerárquica de 3 capas** para maximizar la modularidad y eficiencia:

### 1. Capa de Kernel (C) - `scheduler.c`
* Actúa como el Planificador.
* Determina las zonas de operación: **Luminosa** (0-42 min) y **Oscura** (43-99 min).
* Orquesta la llamada a los procesos.

### 2. Capa de Procesos (Assembly Wrapper)
Son las interfaces que cumplen con los requisitos funcionales:
* **`proceso1.s`:** Intermediario para la adquisición de datos.
* **`proceso2.s`:** Implementa una máquina de estados para el control del enfriamiento usando un rango de activación y gestiona el estado en el stack.
* **`proceso3.s`:** Intermediario para la transmisión de datos.

### 3. Capa de Drivers / Hardware (Assembly Low-Level) - `io_driver.s`
Maneja la interacción directa con el hardware simulado:
* **Syscalls:** Lectura (`read`) y Escritura (`write`) usando las llamadas del sistema Linux (63 y 64).
* **Telemetría:** Lectura de ciclos de CPU (`rdcycle`) y captura del Program Counter (`auipc`).

---

## Requisitos Previos

El proyecto está diseñado para correr en un entorno Linux (nativo o WSL).

* **OS:** Ubuntu 20.04+ o WSL2 en Windows.
* **Compilador:** `riscv64-unknown-elf-gcc` (Toolchain RISC-V).
* **Simulador:** `qemu-riscv32` (QEMU User Mode).
* **Build Tool:** `make`.

Para instalar en Ubuntu/WSL:
```bash
sudo apt update
sudo apt install gcc-riscv64-unknown-elf qemu-user make
```

## Instalación y Compilación

### Clonar el repositorio:
```bash
git clone https://github.com/Sgarrido33/Proyecto_SistemasC.git
```
### Compilar: 
El proyecto utiliza un Makefile automatizado con banderas Bare Metal (`-nostdlib`, `-e main`). Puedes usar make clean antes para borrar el archivo `satelite`.
```bash
make
```

## Guía de Uso

El sistema no genera datos aleatorios internamente, sino que procesa archivos de entrada (`.txt`) que simulan diferentes perfiles de órbita.

### Ejecutar una simulación
```bash
./satelite < (nombre del archivo)
```
No ejecutar esto desde PowerShell. Usar la terminal de WSL (Ubuntu) para que la redirección de entrada (<) funcione correctamente.

## Escenarios de Prueba

Los archivos `.txt` representan diferentes condiciones orbitales (cada línea es una medición tomada cada 5 minutos):

| Archivo              | Descripción                               | Objetivo                                                                 |
|----------------------|---------------------------------------------|---------------------------------------------------------------------------|
| **inputs_base.txt**  | Órbita estándar con un pico de calor.       | Verificar activación normal (> 90°C) y apagado (< 60°C).                 |
| **inputs_frio.txt**  | Temperaturas bajas (< 80).                  | Asegurar que el sistema no se activa innecesariamente.                   |
| **inputs_stress.txt**| Fluctuaciones constantes 85–95.             | Probar la robustez de la lógica de histéresis.                           |
| **inputs_critico.txt** | Múltiples valores > 105.                 | Verificar la captura repetida del PC de Anomalía.                        |

---

## Telemetría y Métricas

El sistema imprime en tiempo real métricas de rendimiento para cada ciclo de ejecución:

- **Min: XX [Zona]**: Tiempo simulado de la órbita y fase actual (Luz/Sombra).  
- **[ON]/[OF]**: Indicadores visuales de cambio de estado del actuador de enfriamiento.  
- **!ANOMALIA PC: XXXXX**: Dirección de memoria hexadecimal donde ocurrió una violación crítica (>105°C).  
- **T_exe: XXXXX**: Ciclos de reloj exactos consumidos por la CPU para procesar la iteración completa (medido con `rdcycle`).  

---

## Consideraciones Técnicas

- **Gestión de Memoria**:  
  Se evitó el uso de `.data` global para variables mutables, previniendo *segmentation faults* en el entorno simulado.  
  El estado del enfriamiento se administra mediante punteros en el *stack* del `main`.

- **Input/Output**:  
  Implementación de funciones `itoa` (Integer to ASCII) en ensamblador, debido a la ausencia de la biblioteca estándar `stdio.h`.


