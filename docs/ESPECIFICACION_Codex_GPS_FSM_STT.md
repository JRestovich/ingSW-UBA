# Especificación de implementación — GPS Parser FSM y pruebas STT

**Estado:** contrato de implementación para Codex, todavía no validado contra el repositorio ni ejecutado.
**Rama de trabajo:** `gps_stt` (solo local).
**Orden obligatorio:** Fase A (FSM e integración) → verificación de compilación → Fase B (tests STT) → ejecución, correcciones y revisión final.
**Fuente:** análisis STT acordado para el parser GPS en esta conversación; no sustituye la inspección de las APIs reales del repositorio.

## 1. Propósito y alcance

Implementar en el módulo GPS existente una máquina de estados para recolectar tramas del flujo UART/ring buffer, verificar su integridad y despachar las tramas RMC al parser existente. Después, incorporar las pruebas unitarias derivadas de State Transition Testing (STT) usando el proyecto Ceedling existente.

**Obligatorio**

- Reutilizar `ring_buffer.h` y `ring_buffer.c` **existentes** del repositorio. Inspeccionar sus firmas, capacidad, ownership y semántica; no crear otro ring buffer ni duplicar su implementación.
- El ring buffer de recepción está especificado con **164 bytes de capacidad**. Verificar cómo se configura/instancia en el código real, y adaptar la configuración existente de forma mínima; distinguir desbordamiento del ring buffer y desbordamiento de *trama* (máximo 82 bytes).
- FSM de **tres estados exclusivamente**: `IDLE`, `FI` (find/init frame) y `FE` (frame extraction).
- Integrar la validación XOR/checksum y el despacho de RMC respetando módulos, funciones e interfaces existentes; no reescribir el parser RMC sin justificación demostrable.
- Introducir todos los nuevos tests STT **en el archivo existente** `test/test_gps/test_gps_parser.c`. Confirmar ruta antes de editar; si difiere, documentarlo y modificar el equivalente existente, sin crear suite paralela.
- Mantener el alcance en GPS y las mínimas dependencias necesarias; no introducir timeouts ni un cuarto estado.
- Escribir commits **locales, pequeños y claros**, sin push, merge, PR o comandos destructivos. Respetar siempre los permisos vigentes.

**Aspectos a comprobar antes de codificar**

1. Nombres reales de módulos, APIs públicas, ring buffer, callbacks, funciones de checksum y parser RMC.
2. Si la recepción comienza automáticamente o con un evento explícito de inicialización (`RX_READY`).
3. Ruta y comando real de Ceedling en este repositorio (no asumir que `ceedling test:all` funciona hasta comprobarlo).
4. Cambios locales preexistentes y rama activa (`git branch --show-current` debe devolver `gps_stt`). No sobrescribir ni commitear trabajo ajeno.
5. **Marcador confirmado para GNSS NMEA 0183:** usar **`$` (ASCII 0x24)** como único delimitador de inicio para RMC y las demás sentencias GNSS en alcance. El marcador `!` no está habilitado en este proyecto; si aparece en código o fixtures heredados, corregirlos o aislarlos según su alcance y documentar el cambio. No solicitar aprobación por la antigua discrepancia: quedó resuelta por decisión del usuario.

## 2. Modelo FSM acordado

### Estados

| Estado | Significado | Comportamiento |
|---|---|---|
| `IDLE` | Antes de habilitar recepción | No se procesa flujo como trama hasta `RX_READY`. |
| `FI` | Búsqueda de inicio | Ignora datos/ruido hasta `$`. |
| `FE` | Captura de trama | Acumula bytes, reconoce otro `$`, detecta terminador `\r\n` y longitud máxima. |

### Eventos

| ID | Evento | Definición |
|---|---|---|
| `E1` | `RX_READY` | Habilitación/inicialización de la recepción. |
| `E2` | `START` | Byte `$`. |
| `E3` | `DATA` | Byte que no es `$` ni el `\n` que completa `\r\n` dentro de `FE`. Un `\r` aislado cuenta como dato. |
| `E4` | `END` | Se acaba de completar `\r\n` dentro de `FE`. |
| `E5` | `OVERFLOW` | Se intenta superar 82 bytes de la trama sin cierre válido. |

**No existe `END_PARTIAL`.** Un `\r` que no completa todavía `\r\n` es un byte de datos más. El evento `END` solo existe al recibir el `\n` que cierra la secuencia completa.

### Tabla única de estados/eventos (Tabla 1 STT)

| Evento | `IDLE` | `FI` | `FE` |
|---|---|---|---|
| `E1 RX_READY` | **T1 → FI**: iniciar recepción | — | — |
| `E2 START ($)` | — | **T2 → FE**: inicializar nueva trama | **T3 → FE**: descartar parcial y resincronizar |
| `E3 DATA` | — | **T4 → FI**: ignorar dato/ruido | **T5 → FE**: acumular byte |
| `E4 END (\r\n)` | — | — | **T6 → FI**: validar y despachar si corresponde |
| `E5 OVERFLOW` | — | — | **T7 → FI**: descartar trama excedida |

`—` significa evento no aplicable para la FSM modelada, no necesariamente una entrada byte a byte imposible de recibir por UART. En `FI`, `\r` y `\n` son ruido y se descartan mediante T4; no hay transición END allí.

### Resumen de transiciones

| Transición | Origen → Destino | Disparador | Acción observable/verificable |
|---|---|---|---|
| T1 | IDLE → FI | RX_READY | Se habilita búsqueda de inicio. |
| T2 | FI → FE | `$` | Se inicia nuevo buffer lógico de trama (incluido el inicio). |
| T3 | FE → FE | otro `$` | Se abandona trama parcial y comienza una nueva. |
| T4 | FI → FI | dato/ruido | No modifica captura ni invoca parser. |
| T5 | FE → FE | dato | Se acumula sin superar capacidad. |
| T6 | FE → FI | `\r\n` completo | Se termina la captura; validación y despacho condicional. |
| T7 | FE → FI | intento de >82 bytes | Se descarta trama; no se despacha. |

### Semántica exacta del límite (REQUISITO CRÍTICO)

- **La trama puede tener exactamente 82 bytes, incluidos `$` inicial y `\r\n` final**. Si el byte número 82 es `\n` y el byte 81 es `\r`, se ejecuta T6 y **se entrega la trama a validación**, aunque luego checksum o campos puedan invalidarla.
- **No declarar overflow al almacenar el byte 82**. Solo declarar T7 ante un **intento de incorporar el byte número 83** a una trama que sigue abierta, salvo que el byte sea `$`, caso en el que se aplica resincronización T3.
- Si los 82 bytes acumulados NO cierran `\r\n`, permanecer en FE hasta el siguiente byte; al intentar sumar un byte ordinario adicional, T7.
- Prioridad sugerida al procesar cada byte en FE: 1) `START` (`$`) → T3; 2) determinar si el byte completa `\r\n`; 3) comprobar si hay capacidad para incorporar el byte (si no, T7); 4) incorporarlo; 5) si completa `\r\n`, T6; en otro caso T5. La implementación puede optimizar el orden **solo** si conserva las mismas propiedades en los límites.
- Si el byte 82 no completa la secuencia y el byte 83 es `\n`, **no** aceptar la trama: excede el máximo.
- Después de T6 o T7, volver a `FI` y poder reconocer la siguiente trama sin volver a `IDLE`.
- Distinguir `ring buffer overflow` de `frame overflow` y no confundir las capacidades 164 / 82.

### Validación y despacho tras T6

1. Comprobar el formato de la sentencia recibido (estructura, delimitador, terminador y sintaxis de checksum) según la API real.
2. Validar checksum NMEA mediante XOR de los bytes de la carga entre marcador de inicio y `*` (sin incluir esos delimitadores), comparándolo con sus dos dígitos hexadecimales. Conservar comportamiento existente correcto.
3. Si el checksum es inválido, descartar **sin llamar al parser RMC**.
4. Si checksum válido pero tipo no soportado, descartar sin llamar al parser RMC.
5. Si es tipo RMC soportado, invocar al decodificador/despachador ya existente y propagar solo datos válidos conforme a la API existente.
6. La decisión checksum/RMC es una **acción asociada a T6**, no un estado nuevo de la FSM.

No inventar API pública nueva solo para satisfacer los tests. Usar mocks/spies de los colaboradores ya existentes, o helpers internos mínimos cuando resulten justificables.

## 3. Árbol de transiciones STT (Paso 2)

Raíz: `IDLE`. La expansión se detiene al reencontrar estados ya visitados; los ciclos pueden ejercitarse repetidamente al generar tests.

```text
IDLE
└─ T1 → FI
   ├─ T4 → FI  [ruido; estado ya visitado]
   └─ T2 → FE
      ├─ T3 → FE  [reinicio; ya visitado]
      ├─ T5 → FE  [acumulación; ya visitado]
      ├─ T6 → FI  [fin válido; ya visitado]
      └─ T7 → FI  [overflow; ya visitado]
```

Caminos base: `T1→T4`, `T1→T2→T3`, `T1→T2→T5`, `T1→T2→T6`, `T1→T2→T7`. Para T6, debe existir acumulación suficiente como para completar una sentencia; por eso los casos concretos suelen incluir T5.

## 4. Casos legales STT (Paso 3) — todos obligatorios

Todos comienzan desde el estado inicial `IDLE` (salvo API que de forma legítima inicialice antes; en ese caso documentar montaje), usando bytes concretos y checksum generado correctamente para las tramas positivas.

| ID | Escenario / estímulo | Ruta a cubrir | Oráculo / resultado esperado |
|---|---|---|---|
| **STT-01** | Activar recepción | T1 | `IDLE→FI`; no parseo ni despacho. |
| **STT-02** | Ruido previo (p. ej., `abc\r\n`) seguido de `$` | T1→T4→T2 | Ignora ruido y comienza captura al recibir inicio. |
| **STT-03** | Sentencia RMC completa, con checksum correcto y `\r\n` | T1→T2→T5→T6 | Valida, invoca RMC una sola vez y vuelve a FI. |
| **STT-04** | Trama parcial, nuevo `$`, luego RMC completa válida | T1→T2→T5→T3→T5→T6 | Descarta la parcial, procesa solo la segunda y queda en FI. |
| **STT-05** | 82 bytes acumulados SIN `\r\n`, luego un byte ordinario adicional | T1→T2→T5→T7 | Overflow **solo ante intento de byte 83**; descarta, no despacha, vuelve a FI. |
| **STT-06** | Dos tramas completas consecutivas | T1→T2→T5→T6→T2→T5→T6 | Dos despachos, orden correcto y ninguna vuelta a IDLE. |
| **STT-07** | Trama RMC completa con checksum incorrecto | T1→T2→T5→T6 | Se alcanza T6, se rechaza checksum y NO se invoca parser RMC. |
| **STT-08** | Trama completa no RMC con checksum correcto | T1→T2→T5→T6 | Se alcanza T6, se valida checksum y NO se invoca parser RMC. |
| **STT-09** | Trama RMC de **exactamente 82 bytes** que termina en `\r\n` y tiene checksum válido | T1→T2→T5→T6 | Acepta para validación y procesa si el contenido RMC es válido. No se dispara T7. |

En STT-09, construir una sentencia de 82 bytes **realmente compatible con el parser existente**, sin agregar padding arbitrario que haga fallar el formato. Separar el oráculo de **límite aceptado para validación** del resultado de **decodificación RMC**, cuando el parser exija restricciones adicionales.

### Vectores de entrada base (para tests concretos)

Los siguientes ejemplos utilizan el delimitador estándar **`$`** confirmado para GNSS NMEA. El checksum XOR se calcula únicamente sobre el cuerpo entre `$` y `*`; cambiar el delimitador no modifica el valor calculado.

```c
// 70 bytes en total; checksum XOR de la carga = 0x6A.
"$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n"

// Tipo diferente de RMC, checksum correcto (0x47):
"$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n"
```

- Para STT-07 modificar, por ejemplo, `*6A` por `*00` **sin alterar el resto de la carga** (checksum incorrecto asegurado).
- Para STT-04 comenzar con `$ABC` (parcial) seguido de la sentencia RMC válida completa; aserción: solo una validación/despacho de la RMC final.
- Para STT-05 enviar un `$` seguido de **81 bytes que no cierren CRLF**: longitud acumulada 82. Verificar que **aún no** ocurrió T7. Enviar **un byte adicional ordinario** para disparar T7.
- Para STT-09 construir/validar programáticamente longitud de 82 bytes, CRLF en las dos últimas posiciones y checksum del cuerpo correcto. Si el decodificador rechaza un campo legítimamente extendido, testear por separado **admisión a T6** y **resultado del decodificador**; no cambiar la guarda de 82.
- Para STT-06 concatenar dos copias del vector RMC positivo, sin nueva activación RX_READY; esperar exactamente dos despachos.


## 5. Casos de transiciones ilegales STT (Paso 4)

**Regla de implementación:** estos son casos de **robustez del modelo**, no necesariamente entradas alcanzables desde la API pública. Primero determinar testabilidad sin alterar artificialmente la arquitectura. Si una combinación requiere inyección directa de eventos o acceso al estado interno, documentar esta limitación y preferir pruebas a través de interfaces existentes. **No forzar exposición de internals solo para incrementar métricas.**

| ID | Estado + evento no aplicable | Esperado | Testabilidad |
|---|---|---|---|
| **IL-01** | IDLE + `START` previo a RX_READY | No comenzar captura ni despachar. | Depende de si puede ingresarse bytes antes de iniciar. |
| **IL-02** | IDLE + `DATA` previo a RX_READY | No procesar bytes. | Ídem. |
| **IL-03** | IDLE + `END` (`\r\n`) previo a RX_READY | No validar ni despachar. | Ídem; quizá modelar como dos bytes. |
| **IL-04** | IDLE + evento `OVERFLOW` forzado | No ejecutar recuperación FE ni despacho. | Normalmente exige inyección de eventos. |
| **IL-05** | FI + evento `OVERFLOW` forzado | No descartar captura inexistente ni despachar. | Normalmente exige inyección de eventos. |
| **IL-06** | Repetir `RX_READY` estando en FI y estando en FE | No reinicializar inesperadamente la captura. | Depende de API de inicio; comprobar ambos subescenarios si accesibles. |

Además, en `FI`, bytes `\r` y `\n` (incluso consecutivos) son ruido tratado por T4, **no un evento END ilegal despachable**. Registrar con claridad qué IL-xx se automatiza, cuáles son inalcanzables y por qué.

## 6. Pruebas complementarias de guardas y bordes (sugeridas)

Este grupo **no reemplaza** las 9 legales y 6 ilegales acordadas. Implementar las alcanzables con la API real, priorizando que los límites se demuestren mediante aserciones.

| ID | Objetivo | Oráculo |
|---|---|---|
| **B-01** | Trama completa de 81 bytes y checksum válido | Aceptación normal; no overflow. |
| **B-02** | Trama de 82 bytes con últimos dos `\r\n` | T6, jamás T7. (Puede integrarse con STT-09). |
| **B-03** | Tras 82 bytes sin terminador, recibir byte ordinario 83 | Exactamente un descarte por T7. (Puede integrarse con STT-05). |
| **B-04** | `\r` aislado dentro de FE, luego carácter distinto de `\n` | Continuar acumulación, no cierre prematuro. |
| **B-05** | 82 bytes sin cierre seguidos de `$` | T3 resincroniza, no se intenta almacenar byte 83. |
| **B-06** | Terminador `\r\n` dividido en dos lotes/lecturas del ring buffer | Una sola trama completa, sin cierre prematuro. |
| **B-07** | Trama truncada, seguida por nuevo `$` y una sentencia válida | Descartar parcial, procesar segunda. |
| **B-08** | Sentencia inválida seguida inmediatamente de válida | Recuperación y despacho de la segunda. |
| **B-09** | Ruido en FI con caracteres de fin `\r`/`\n` | Sin validación ni despacho. |

Para fixtures, usar la utilidad de checksum existente o helper local en `test_gps_parser.c`, pero **nunca fabricar un checksum fijo sin verificarlo**. Cualquier caso que no pueda ejecutarse se marca claramente como `NO EJECUTADO` y se explica su motivo.

## 7. Plan de trabajo que debe ejecutar Codex

### Fase A — FSM y procesamiento (primero)

1. Confirmar rama `gps_stt`, estado de Git y permisos. Registrar estado inicial y no sobrescribir modificaciones ajenas.
2. Localizar y leer código GPS, ring buffer y dispatcher RMC. Verificar que el delimitador de inicio en implementación, fixtures y tests sea `$` conforme al requisito confirmado; documentar y corregir referencias heredadas a `!` cuando sean parte del flujo GNSS.
3. Anotar la API existente y un plan breve en `docs/STT_AGENT_PROGRESS.md`.
4. Implementar los **tres** estados y las transiciones T1–T7 con la semántica de §2.
5. Reutilizar el ring buffer existente de 164 bytes y el buffer lógico de trama de hasta 82 bytes.
6. Compilar y ejecutar las pruebas de regresión existentes. Registrar comando, salida resumida y estado real; corregir problemas introducidos por la FSM.
7. Si los permisos lo permiten, commitear cambios de producción en unidades pequeñas, con nombres claros. Si no, documentar commits propuestos y dejar el árbol de trabajo intacto.
8. **Continuar automáticamente a Fase B** cuando el código compile/esté verificable y no exista una decisión de requisitos pendiente; no esperar aprobación humana por defecto.

### Fase B — Casos STT (después)

1. Modificar **únicamente el archivo de tests existente** `test/test_gps/test_gps_parser.c` para incorporar nuevos casos, preservando pruebas previas.
2. Implementar todos los STT-01...STT-09, usando estímulos byte a byte o lotes acordes con la API pública.
3. Implementar IL-01...IL-06 alcanzables; para los inalcanzables, mantener un registro trazable de justificación sin introducir hooks inseguros solo para tests.
4. Implementar los bordes B-01...B-09 alcanzables sin duplicar aserciones innecesariamente.
5. Ejecutar Ceedling primero sobre el módulo GPS y luego suite completa si existe y es viable. Corregir defectos de código; no debilitar expectativas para hacer pasar tests.
6. Si dispone de gcov/gcovr configurados, medir cobertura y adjuntar resumen; **no inventar porcentajes** cuando no se disponga de instrumentación.
7. Registrar cobertura de T1...T7 con referencia a los tests que efectivamente ejercitan cada transición y con resultados reales (PASS / FAIL / NO EJECUTADO / NO ALCANZABLE).
8. Hacer commits locales, pequeños, descriptivos, solo si los permisos lo permiten.

### Revisión final — entregable auditable

Entregar en el registro de progreso y en la respuesta de Codex:

- Rama final, `git status` final, lista de archivos editados y confirmación de que **no hubo push**.
- Lista de commits locales realmente realizados (`git log`); propuestos no realizados por permisos, por separado.
- Tabla con **STT-01...STT-09**, **IL-01...IL-06** y bordes implementados, con PASS/FAIL/NO EJECUTADO/NO ALCANZABLE según evidencia.
- Verificación explícita de límite inclusivo de 82, CRLF dividido, resync `$`, recuperación tras errores y parser RMC.
- Comandos de build/tests ejecutados y resultados exactos, no presunciones.
- Problemas reales o decisiones pendientes. No declarar tarea finalizada si quedan bloqueos críticos.

## 8. Criterios de aceptación

Se considera implementada la modificación **solo si se verifica** que:

- [ ] Se ejecuta o simula correctamente T1–T7 y hay **trazabilidad de cobertura de las 7 transiciones**.
- [ ] Solo existen los estados IDLE, FI, FE; `END_PARTIAL` no es un evento separado.
- [ ] `\r\n` funciona incluso si llega en diferentes lecturas.
- [ ] Una sentencia de 82 bytes con terminador completo llega a validación y no produce overflow.
- [ ] El overflow ocurre **solamente al intentar byte 83** en trama sin cierre; T3 puede resincronizar aun cuando haya 82 bytes pendientes.
- [ ] El ring buffer usa el módulo existente `ring_buffer.h/.c` y su capacidad prevista (164 bytes), sin duplicación.
- [ ] Checksum inválido y sentencia no RMC no invocan al parser RMC; sentencia RMC válida sí.
- [ ] Todos los casos legales implementables están dentro de `test_gps_parser.c`, con resultados evidenciados.
- [ ] IL-xx no alcanzables están documentados, no disimulados como tests aprobados.
- [ ] La suite ejecutable relevante pasa, o las fallas quedan diagnosticadas y registradas como pendientes, sin falsos positivos.
- [ ] Los cambios están en `gps_stt`, separados en commits locales revisables cuando fue posible y **nunca publicados**.

## 9. Política de permisos y seguridad para la ejecución nocturna

- Ejecutar desde VS Code/Codex local, con equipo encendido, sin suspensión y proyecto abierto.
- Usar permisos de escritura limitados al repositorio. No desactivar sandbox ni pedir acceso irrestricto para saltar aprobación.
- Si una operación sobre `.git`, build o herramientas requiere aprobación no disponible, registrar el bloqueo, continuar tareas independientes permitidas y conservar cambios sin sobrescribirlos.
- No generar ningún push ni transferir el código a servicios remotos por iniciativa propia.
- No borrar trabajo preexistente, reescribir historia ni modificar ramas ajenas.
- Al volver, el usuario debe poder inspeccionar `docs/STT_AGENT_PROGRESS.md`, `git status`, `git log` y ejecutar manualmente Ceedling.

## 10. Notas sobre diferencias con el modelo original

- Las decisiones de checksum y de tipo de mensaje **no** son nuevos estados/transiciones; forman parte del procesamiento de T6.
- Los eventos ilegales son una técnica de diseño de pruebas, no una exigencia de modificar producción para exponer estados internos.
- Una prueba correctamente redactada pero no ejecutada **no** puede figurar como `PASS`.
- Si un detalle de la especificación contradice un requerimiento externo o una interfaz real, informarlo y detener **solo esa parte** en lugar de inventar una solución.
