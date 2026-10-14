# Tarea para pegar en Codex (VS Code, ejecución local)

Trabajá en el repositorio actualmente abierto en VS Code y exclusivamente en la rama local `gps_stt`. Leé y respetá `AGENTS.md` y `docs/ESPECIFICACION_Codex_GPS_FSM_STT.md`. No realices push, merge ni PR.

**Formato confirmado:** marcador NMEA GNSS `$` (no `!`), final de trama `\r\n`. Aplicar en implementación y todos los tests.

## Objetivo
Implementá la FSM GPS de tres estados y luego las pruebas STT especificadas. Reutilizá `ring_buffer.h/.c` y agregá todas las pruebas nuevas al `test/test_gps/test_gps_parser.c` existente.

## Contrato de arquitectura y estilo

- Implementar recepción, FSM IDLE/FI/FE, detección, validación de formato/checksum y dispatcher en **ingSw/Services/gps/gps_parser.c/.h**; crear si faltan.
- Mantener **gps_parser_rmc.c/.h exclusivamente como decodificador RMC**, sin mover allí la FSM ni cambiar su API.
- Reutilizar ring_buffer.h/.c con **128 bytes de almacenamiento, 127 útiles**. No modificar ese módulo. Consumir lotes mayores a 127 sin perder datos ni orden.
- Máximo de trama: **82 bytes incluyendo `$` y CRLF**. Overflow al intentar byte 83; un nuevo `$` tiene prioridad para resincronizar.
- NMEA utiliza **checksum XOR, no CRC**. Solo después de validar estructura/checksum identificar el tipo y despachar. Para GPRMC/GNRMC delegar en GPS_PARSER_RMC_processMsg, adaptando a su entrada sin `$`. Descartar otros tipos sin llamar al decoder.
- Preparar un dispatcher interno con entradas de tipo y handler, extensible sin cambiar FSM ni duplicar validación. Implementar solo RMC; sin APIs de registro dinámico ni parsers futuros.
- Si falta API de recepción, está autorizado diseñar la mínima API de producción: contexto, init en IDLE, RX_READY idempotente y consumo de lotes. Documentar ownership y callback. No se exige integración con hardware UART ni concurrencia ISR.
- Aplicar **.clang-format** y dejar **una línea vacía entre funciones**, también en tests, conservando secciones del proyecto.
- Todas las pruebas nuevas van en test/test_gps/test_gps_parser.c. Reutilizar fixtures existentes y comprobar sus checksums.
- Verificar con mocks/spies las llamadas al decoder: cero ante checksum/estructura/tipo inválidos; una por RMC válido y orden correcto para tramas consecutivas. Agregar integración con el decoder real. Conteo de callbacks o revisión de guardas no reemplaza el oráculo de llamadas.
- Se autoriza configuración mínima Ceedling para ring_buffer y builds de mocks/integración si hacen falta, sin crear suites paralelas.

## Secuencia autónoma
1. Verificá la rama, el estado inicial de Git y que existan la especificación, el parser, el ring buffer y Ceedling. No sobrescribas cambios previos.
2. Leé el código pertinente, verificá contratos y planificá la estrategia de mocks/integración antes de editar. Elaborá un plan corto por hitos. Guardá el plan y el estado inicial en `docs/STT_AGENT_PROGRESS.md`.
3. Implementá primero la FSM. Compilá o ejecutá los tests de regresión pertinentes, corregí errores y registrá resultados verificables.
4. Agregá los casos STT legales e ilegales y los complementarios requeridos por la especificación al test existente. Ejecutá Ceedling, diagnosticá los fallos y corregí el código o las pruebas cuando corresponda, sin debilitar las aserciones.
5. Verificá formato con clang-format --dry-run --Werror --style=file sobre C/H modificados y git diff --check. Realizá una revisión final de límites de 82 bytes, `\r\n`, resincronización, checksum, dispatcher RMC y compatibilidad con la API actual.
6. Hacé commits locales pequeños y descriptivos solo cuando la política de permisos lo permita. Si `.git` está bloqueado, conservá los cambios y registrá en el progreso qué commits proponés. No busques eludir el sandbox ni solicites acceso total.
7. Al finalizar, indicá resultado de cada prueba, bloqueos, archivos modificados, `git status`, lista de commits locales y diferencias aún sin commitear.

## Comportamiento sin supervisión
Continuá automáticamente de un hito al siguiente cuando la especificación sea clara. Si una acción requiere mi aprobación y no está disponible, no te quedes reintentando indefinidamente: registrá el bloqueo, seguí con los pasos independientes que sí estén permitidos, y dejá una instrucción clara para continuar mañana. No declares terminado el trabajo si quedaron hitos sin ejecutar.
