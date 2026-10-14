# GPS STT — Registro de ejecución de Codex

> Plantilla reiniciada para la próxima ejecución. No hay resultados del intento anterior que puedan atribuirse a una nueva implementación. Completar solo con evidencia de esa ejecución.

## Datos iniciales
- Fecha y hora:
- Rama (`git branch --show-current`):
- Estado inicial de Git:
- Versión/entorno de Ceedling:
- Ruta verificada de FSM GPS (destino obligatorio: ingSw/Services/gps/gps_parser.c/.h):
- Ruta verificada del ring buffer:
- Ruta verificada del archivo de pruebas:

## Plan por hitos
- [ ] Inspección y plan
- [ ] FSM implementada
- [ ] Build / regresión de FSM
- [ ] Tests STT incorporados
- [ ] Tests ejecutados y fallos analizados
- [ ] Integración con decoder real y verificación de llamadas con mocks/spies
- [ ] Dispatcher extensible sin nuevos parsers
- [ ] .clang-format y línea vacía entre funciones
- [ ] Revisión y entrega final

## Registro de actividades
| Hito | Archivos cambiados | Comando/acción | Resultado real | Commit local o pendiente |
| --- | --- | --- | --- | --- |
| | | | | |

## Resultados de Ceedling
- Comandos ejecutados:
- Casos aprobados:
- Casos fallidos:
- Casos no ejecutados y motivo:

## Decisiones de implementación (completar antes de editar)
- API mínima de recepción y ownership:
- Entrada esperada por decoder RMC y adapter:
- Dispatcher y mecanismo para agregar tipos:
- Estrategia mocks/spies para conteo de llamadas y estrategia de integración:
- Comandos Ceedling para cada modo de pruebas:
- Ring buffer: almacenamiento 128 / capacidad útil 127; estrategia para lotes mayores:

## Bloqueos y decisiones pendientes
-

## Entrega
- Commits locales realizados:
- Commits propuestos pero no realizados:
- Cambios sin commitear:
- `git status` final:
- Revisión de requisitos cumplidos / pendientes:

## Trazabilidad por caso (completar con evidencia; no pre-marcar como PASS)

| ID | Test / justificación | Estado (PASS/FAIL/NO EJECUTADO/NO ALCANZABLE) | Comando o evidencia |
| --- | --- | --- | --- |
| STT-01 | | NO EJECUTADO | |
| STT-02 | | NO EJECUTADO | |
| STT-03 | | NO EJECUTADO | |
| STT-04 | | NO EJECUTADO | |
| STT-05 | | NO EJECUTADO | |
| STT-06 | | NO EJECUTADO | |
| STT-07 | | NO EJECUTADO | |
| STT-08 | | NO EJECUTADO | |
| STT-09 | | NO EJECUTADO | |
| IL-01 | | NO EJECUTADO | |
| IL-02 | | NO EJECUTADO | |
| IL-03 | | NO EJECUTADO | |
| IL-04 | | NO EJECUTADO | |
| IL-05 | | NO EJECUTADO | |
| IL-06 | | NO EJECUTADO | |
| B-01 | | NO EJECUTADO | |
| B-02 | | NO EJECUTADO | |
| B-03 | | NO EJECUTADO | |
| B-04 | | NO EJECUTADO | |
| B-05 | | NO EJECUTADO | |
| B-06 | | NO EJECUTADO | |
| B-07 | | NO EJECUTADO | |
| B-08 | | NO EJECUTADO | |
| B-09 | | NO EJECUTADO | |


### Cobertura de transiciones

| Transición | Casos que efectivamente la recorrieron | Evidencia real |
| --- | --- | --- |
| T1 | | |
| T2 | | |
| T3 | | |
| T4 | | |
| T5 | | |
| T6 | | |
| T7 | | |
