# STD-DEP · Construcción y despliegue

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Plataforma de desarrollo | macOS (simulación) |
| Objetivo | Linux embebido (por definir en diseño); la simulación debe parecerse lo máximo posible |
| Mecanismo | Docker Compose: broker + algoritmo + UI (+ reproductor de datos) |

## 1. Builds (CMake presets)

| Preset | Uso |
|---|---|
| `host-debug` | desarrollo en macOS, `-O0 -g` |
| `host-test` | tests: `-O0 -g --coverage -fsanitize=address,undefined` |
| `host-release` | `-O2`, mismas opciones de aviso; es el binario que se mide |
| `linux-container` | build dentro de la imagen Docker (Debian estable, GCC); es el que se despliega |
| `cross-aarch64` | toolchain `aarch64-linux-gnu-gcc` para comprobar portabilidad al objetivo |

## 2. Docker

| ID | Nivel | Regla |
|---|---|---|
| DEP-01 | OBL | `deploy/docker-compose.yml` con servicios: `broker` (eclipse-mosquitto:2, config en `deploy/mosquitto/`), `ogrid` (binario C, imagen multi-stage `deploy/ogrid.Dockerfile`), `ui` (puente + estáticos), `replay` (opcional, reproduce grabaciones de STD-DAT). |
| DEP-02 | OBL | Imágenes con etiqueta fija (nunca `latest`), usuario no root, sistema de ficheros de solo lectura salvo volúmenes de log y config. |
| DEP-03 | OBL | Redes de Compose que replican las VLAN e IPs del plan de STD-NET (`ipam` con subredes fijas). |
| DEP-04 | OBL | La configuración (STD-CFG) se monta como volumen de solo lectura; nada de parámetros en variables de entorno sueltas. |
| DEP-05 | OBL | `healthcheck` en cada servicio; `ogrid` declara `depends_on: broker: condition: service_healthy`. |
| DEP-06 | OBL | Límites de CPU y memoria aproximados al hardware objetivo (`cpus`, `mem_limit`) para que la simulación sea representativa. |
| DEP-07 | OBL | Arranque, parada y estado con `deploy/saf.sh {up|down|status|logs|replay <fichero>}` (cumple STD-SH). |

## 3. Versionado de artefactos

La versión del binario (`ogrid --version`) y de las imágenes coincide con el tag SemVer de `main` (`git describe`). Un despliegue desde una rama distinta de `main` se marca `-dev+<sha>`.
