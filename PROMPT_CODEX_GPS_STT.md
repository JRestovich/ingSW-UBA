# Tarea para pegar en Codex (VS Code, ejecución local)

Trabajá en el repositorio actualmente abierto en VS Code y exclusivamente en la rama local `gps_stt`. Leé y respetá `AGENTS.md` y `docs/ESPECIFICACION_Codex_GPS_FSM_STT.md`. No realices push, merge ni PR.

**Formato confirmado:** marcador NMEA GNSS `$` (no `!`), final de trama `\r\n`. Aplicar en implementación y todos los tests.

## Objetivo
Implementá la FSM GPS de tres estados y luego las pruebas STT especificadas. Reutilizá `ring_buffer.h/.c` y agregá todas las pruebas nuevas al `test/test_gps/test_gps_parser.c` existente.

## Secuencia autónoma
1. Verificá la rama, el estado inicial de Git y que existan la especificación, el parser, el ring buffer y Ceedling. No sobrescribas cambios previos.
2. Leé el código pertinente y elaborá un plan corto por hitos. Guardá el plan y el estado inicial en `docs/STT_AGENT_PROGRESS.md`.
3. Implementá primero la FSM. Compilá o ejecutá los tests de regresión pertinentes, corregí errores y registrá resultados verificables.
4. Agregá los casos STT legales e ilegales y los complementarios requeridos por la especificación al test existente. Ejecutá Ceedling, diagnosticá los fallos y corregí el código o las pruebas cuando corresponda, sin debilitar las aserciones.
5. Realizá una revisión final de límites de 82 bytes, `\r\n`, resincronización, checksum, dispatcher RMC y compatibilidad con la API actual.
6. Hacé commits locales pequeños y descriptivos solo cuando la política de permisos lo permita. Si `.git` está bloqueado, conservá los cambios y registrá en el progreso qué commits proponés. No busques eludir el sandbox ni solicites acceso total.
7. Al finalizar, indicá resultado de cada prueba, bloqueos, archivos modificados, `git status`, lista de commits locales y diferencias aún sin commitear.

## Comportamiento sin supervisión
Continuá automáticamente de un hito al siguiente cuando la especificación sea clara. Si una acción requiere mi aprobación y no está disponible, no te quedes reintentando indefinidamente: registrá el bloqueo, seguí con los pasos independientes que sí estén permitidos, y dejá una instrucción clara para continuar mañana. No declares terminado el trabajo si quedaron hitos sin ejecutar.
