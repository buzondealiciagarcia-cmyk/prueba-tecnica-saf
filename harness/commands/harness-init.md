---
description: Inicializa el proyecto con el harness (configuración, estado de fases, docs, hooks git, rama develop)
argument-hint: "[nombre del proyecto]"
---
# Inicializar el proyecto

`H="@@HARNESS@@/bin/h"`

1. Ejecuta `$H doctor`. Si faltan dependencias necesarias, muéstraselas al usuario con el comando de instalación y pregunta si quiere continuar igualmente.
2. Si no existe `harness.config.json`, pregunta con `AskUserQuestion` (una sola ronda): nombre del proyecto (usa `$ARGUMENTS` si viene), código corto (p.ej. `SAF`), autor que firmará los documentos, cliente (si se conoce) y prefijo del módulo C (recomendado `ogrid`).
3. Ejecuta `$H init --name "<nombre>" --code <código> --author "<autor>" --client "<cliente>" --module <módulo>`. El script crea la estructura, instala los hooks de `.githooks/`, crea `develop` y commitea en `harness/project-setup` integrándola con `--no-ff`.
4. Ejecuta `$H template build-base` si no existe la plantilla base y recuerda al usuario que puede registrar su plantilla Word con `/plantilla-registrar <ruta.docx>`.
5. Muestra `$H gate status` y el siguiente paso: `/req-iniciar <fichero o texto con los requisitos>`.
