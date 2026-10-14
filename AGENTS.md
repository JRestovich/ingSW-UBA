# AGENTS.md — Reglas de desarrollo del repositorio

## Alcance y principio general

- Respetá las instrucciones del proyecto y las interfaces públicas existentes. Priorizá cambios mínimos, mantenibles y verificables.
- Si la tarea trata sobre la FSM del GPS y sus tests STT, la fuente de verdad es `docs/ESPECIFICACION_Codex_GPS_FSM_STT.md`.
- Inspeccioná el código real antes de asumir nombres, rutas, interfaces o comandos de build.
- No modifiques archivos que sean ajenos al objetivo solicitado ni sobrescribas cambios ajenos preexistentes.

## Restricciones específicas para GPS STT

- Trabajá exclusivamente en la rama local `gps_stt`. Comprobá `git branch --show-current` antes de editar. Si no coincide, detenete: no cambies de rama ni crees otra sin confirmación.
- Reutilizá `ring_buffer.h` y `ring_buffer.c` existentes; no implementes un segundo ring buffer.
- Implementá la nueva FSM de tres estados (IDLE, FI, FE) de acuerdo con la especificación y conservá el procesamiento RMC existente salvo incompatibilidad demostrada.
- El marcador de inicio de sentencia GNSS NMEA es **`$` (ASCII 0x24)**, no `!`; `\r\n` es el terminador. Los casos STT y fixtures deben usar ese marcador.
- Agregá los casos derivados de STT al archivo existente `test/test_gps/test_gps_parser.c`; verificá la ruta real antes de editar. No crees una suite paralela.
- Primero implementá y compilá la FSM; después agregá tests STT legales, ilegales y complementarios. No introduzcas un cuarto estado ni timeouts fuera del alcance.
- No declares que una prueba pasa sin haberla ejecutado realmente. No debilites las aserciones para ocultar fallos.

## Validación y trazabilidad

- Identificá el comando de Ceedling que usa el repositorio, ejecutá las pruebas afectadas y, si procede, la suite completa.
- Registrá comandos, resultados y bloqueos en `docs/STT_AGENT_PROGRESS.md`.
- Mantené cambios y commits pequeños, atómicos y con mensajes claros, por ejemplo `refactor(gps): implement frame collection FSM` y `test(gps): cover STT legal transitions`.
- Realizá solo commits locales del trabajo propio y solo si los permisos efectivos lo permiten. No desactives medidas de seguridad para obtener permisos sobre `.git`. Si un commit requiere aprobación, dejá el cambio sin commitear, indicá qué archivos contiene y registrá el comando sugerido para revisión humana.
- No hagas push, fetch innecesario, merge, rebase, pull request ni publicación externa. No utilices comandos destructivos como `git reset --hard` o `git clean -fd`.
- Nunca agregues secretos, claves, archivos generados de build ni archivos personales a los commits.

## Autonomía

- Podés inspeccionar, diseñar, editar, compilar, testear, corregir y continuar entre fases sin aprobación adicional, siempre dentro del alcance y de los permisos concedidos.
- Si hay una ambigüedad que cambia los requisitos o exige modificar componentes ajenos, documentá la decisión pendiente y detené esa parte; no inventes requisitos.
- Al finalizar, entregá resumen del trabajo, pruebas efectivamente ejecutadas, casos pendientes, cambios sin commitear, últimos commits y `git status`.
