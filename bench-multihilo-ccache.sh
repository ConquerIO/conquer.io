#!/usr/bin/env bash
# bench-multihilo-ccache.sh - Mide la compilación multi-hilo y ccache en tu máquina.
#
# EJERCICIOS: COMPILACIÓN MULTI-HILO Y CCACHE
#
# Responde, de forma concisa, a estas preguntas:
#   1. Comprobad tiempos de ejecución de la compilación con diversos valores
#      para N en -jN. ¿A partir de qué valores de N ya no supone una mejora
#      sustancial el incremento en el número de trabajos en paralelo?
#   2. ¿Hay fallos de compilación con ejecuciones en paralelo? Si es así
#      tratad de ver por qué se producen y procurad solucionarlos.
#   3. Aplicad ccache en la compilación del proyecto y compara el tiempo de
#      ejecución de la compilación con y sin esta herramienta.
#   4. Borrad las estadísticas de ccache y limpiadla. Después, ejecutad dos
#      veces la compilación con esta herramienta y analizad las estadísticas
#      que proporciona la herramienta.
#
# Uso (desde cualquier carpeta):
#   ./bench-multihilo-ccache.sh            # pide confirmación
#   ./bench-multihilo-ccache.sh -y         # sin confirmación
#   ./bench-multihilo-ccache.sh clean      # borra los resultados-bench-*.md generados (pide confirmación)
#   ./bench-multihilo-ccache.sh clean -y   # idem, sin confirmación
#
# Variables opcionales:
#   REPS=2 ./bench-multihilo-ccache.sh     # repeticiones por N (por defecto 1)
#   JOBS="1 2 4 8" ./bench-multihilo-ccache.sh   # valores de N a probar
#                                          # (por defecto: 1 2 4 nproc/2 nproc 2·nproc)
#   RUNS_P2=10 ./bench-multihilo-ccache.sh # repeticiones de la pregunta 2 (por defecto 5)
#   UMBRAL=5 ./bench-multihilo-ccache.sh   # % mínimo de mejora para seguir subiendo N (por defecto 10)
#
# Duración aproximada: unos 3 minutos en una máquina de 10 núcleos.
#
# AVISO: ejecuta "make clean", vacía la caché de ccache (ccache -C) y pone a
# cero sus estadísticas (ccache -z). No modifica el Makefile ni el código.
#
# Resultado: se muestra por pantalla y se guarda en
#   resultados-bench-<máquina>-<fecha>.md

set -u

cd "$(dirname "$0")" || exit 1

# ---- Modo limpieza: borra los informes generados y sale ---------------------
if [ "${1:-}" = "clean" ] || [ "${1:-}" = "--clean" ]; then
  shopt -s nullglob
  files=(resultados-bench-*.md)
  if [ "${#files[@]}" -eq 0 ]; then
    echo "No hay resultados que borrar."
    exit 0
  fi
  printf 'Se borrarán %d fichero(s):\n' "${#files[@]}"
  printf '  %s\n' "${files[@]}"
  if [ "${2:-}" != "-y" ]; then
    read -r -p "¿Borrar? [s/N] " ans
    case "$ans" in s|S|si|sí|y|Y) ;; *) echo "Cancelado."; exit 0 ;; esac
  fi
  rm -f -- "${files[@]}" && echo "Borrados."
  exit 0
fi

# ---- Comprobaciones ---------------------------------------------------------
for cmd in make g++ awk; do
  command -v "$cmd" >/dev/null || { echo "Falta '$cmd'. Instálalo y vuelve a probar."; exit 1; }
done
[ -f Makefile ] || { echo "No se encuentra el Makefile en $(pwd)."; exit 1; }

HAS_CCACHE=1
command -v ccache >/dev/null || HAS_CCACHE=0

if [ "${1:-}" != "-y" ]; then
  echo "Este script ejecutará 'make clean' muchas veces y, si tienes ccache,"
  echo "vaciará su caché y sus estadísticas. Tarda varios minutos."
  read -r -p "¿Continuar? [s/N] " ans
  case "$ans" in s|S|si|sí|y|Y) ;; *) echo "Cancelado."; exit 0 ;; esac
fi

NPROC=$(nproc 2>/dev/null || echo 4)
REPS=${REPS:-1}
RUNS_P2=${RUNS_P2:-5}
UMBRAL=${UMBRAL:-10}   # % mínimo de mejora para considerar útil aumentar N
if [ -z "${JOBS:-}" ]; then
  JOBS=$(printf '%s\n' 1 2 4 $((NPROC / 2)) "$NPROC" $((NPROC * 2)) \
         | awk '$1 > 0' | sort -n | uniq | tr '\n' ' ')
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
OUT="resultados-bench-$(hostname -s 2>/dev/null || echo maquina)-$(date +%Y%m%d-%H%M).md"

# Escribe en pantalla y en el fichero de resultados
log() { printf '%s\n' "$*" | tee -a "$OUT"; }

# Ejecuta un comando y devuelve los segundos que tardó (con 2 decimales).
# La salida del comando queda en $TMP/last.out y el código en $TMP/last.rc.
timeit() {
  local s e
  s=$(date +%s%N)
  "$@" >"$TMP/last.out" 2>&1
  echo $? >"$TMP/last.rc"
  e=$(date +%s%N)
  awk -v d=$((e - s)) 'BEGIN { printf "%.2f", d / 1e9 }'
}

nsrc=$( { find src -name '*.cpp'; ls vendor/imgui/*.cpp vendor/rlImGui/*.cpp 2>/dev/null; } | wc -l)

: >"$OUT"
log "# Resultados: compilación multi-hilo y ccache"
log ""
log "- Fecha: $(date '+%Y-%m-%d %H:%M')"
log "- Máquina: $(uname -m), $NPROC núcleos (nproc)"
cpu=$(lscpu 2>/dev/null | awk -F: '/Model name/ && $2 !~ /^ *-$/ {gsub(/^ +/, "", $2); print $2; exit}')
[ -z "$cpu" ] && cpu=$(lscpu 2>/dev/null | awk -F: '/Vendor ID/ {gsub(/^ +/, "", $2); print $2; exit}')
log "- CPU: ${cpu:-desconocida}"
log "- $(make --version | head -1), $(g++ --version | head -1)"
log "- ccache: $([ $HAS_CCACHE = 1 ] && ccache --version | head -1 || echo 'no instalado')"
log "- Ficheros .cpp: $nsrc"
log "- Repeticiones por N: $REPS"
log ""

# ---- Pregunta 1 -------------------------------------------------------------
log "## 1. Tiempos con distintos valores de N en -jN (sin ccache)"
log ""
log "> Comprobad tiempos de ejecución de la compilación con diversos valores para N en -jN."
log "> ¿A partir de qué valores de N ya no supone una mejora sustancial el incremento en el número de trabajos en paralelo?"
log ""
log "| N | Media (s) | Aceleración vs -j1 | Mejora vs N anterior |"
log "|---|-----------|--------------------|----------------------|"
base=""
prev=""; prevj=""; sat=""
declare -A AVG   # media de cada N sin ccache; se reutiliza en la pregunta 3
for j in $JOBS; do
  total=0
  for _ in $(seq "$REPS"); do
    make clean >/dev/null 2>&1
    t=$(timeit make -j"$j" CCACHE=)
    if [ "$(cat "$TMP/last.rc")" != 0 ]; then
      log ""; log "ERROR: la compilación con -j$j falló. Salida:"; tail -20 "$TMP/last.out" | tee -a "$OUT"; exit 1
    fi
    total=$(awk -v a="$total" -v b="$t" 'BEGIN { print a + b }')
  done
  avg=$(awk -v a="$total" -v n="$REPS" 'BEGIN { printf "%.2f", a / n }')
  AVG[$j]=$avg
  [ -z "$base" ] && base=$avg
  speed=$(awk -v b="$base" -v a="$avg" 'BEGIN { printf "%.1fx", b / a }')
  if [ -n "$prev" ]; then
    gain=$(awk -v p="$prev" -v a="$avg" 'BEGIN { printf "%.1f", (p - a) / p * 100 }')
    gain_txt="${gain} %"
    if [ -z "$sat" ] && awk -v g="$gain" -v u="$UMBRAL" 'BEGIN { exit !(g < u) }'; then
      sat="$prevj"; sat_next="$j"; sat_gain="$gain"
    fi
  else
    gain_txt="—"
  fi
  log "| $j | $avg | $speed | $gain_txt |"
  prev=$avg; prevj=$j
done
log ""
if [ -n "$sat" ]; then
  log "**Conclusión:** pasar de -j$sat a -j$sat_next mejora solo un ${sat_gain} % (< ${UMBRAL} %): a partir de N=$sat el aumento de trabajos en paralelo ya no compensa."
else
  log "**Conclusión:** con los N probados, cada incremento mejora al menos un ${UMBRAL} %: no se observa saturación."
fi
log ""
log "Ficheros más lentos de compilar (sin ccache; se compilan $NPROC a la vez,"
log "así que los tiempos salen algo más altos que compilando de uno en uno):"
log ""
log '```'
export INCS="-I src $(for d in vendor/*/; do printf -- '-I %s ' "${d%/}"; done)"
{ find src -name '*.cpp'; ls vendor/imgui/*.cpp vendor/rlImGui/*.cpp 2>/dev/null; } \
  | xargs -P "$NPROC" -I{} bash -c '
      s=$(date +%s%N)
      g++ -std=c++17 -O3 -Wall -DNO_FONT_AWESOME $INCS -c "$1" -o /dev/null 2>/dev/null
      e=$(date +%s%N)
      awk -v d=$((e - s)) -v f="$1" "BEGIN { printf \"%.2f s  %s\n\", d / 1e9, f }"' _ {} \
  | sort -rn | head -5 | tee -a "$OUT"
log '```'
log ""

# ---- Pregunta 2 -------------------------------------------------------------
JMAX=$((NPROC * 2))
log "## 2. Fallos de compilación en paralelo"
log ""
log "> ¿Hay fallos de compilación con ejecuciones en paralelo? Si es así tratad de ver por qué se producen y procurad solucionarlos."
log ""
fails=0
for _ in $(seq "$RUNS_P2"); do
  make clean >/dev/null 2>&1
  make -j"$JMAX" CCACHE= >"$TMP/p2a.out" 2>&1 || fails=$((fails + 1))
done
log "- \`make clean && make -j$JMAX\`: **$fails fallos de $RUNS_P2**"

make -j"$NPROC" CCACHE= >/dev/null 2>&1   # build previo para reproducir la carrera
fails=0
: >"$TMP/p2b.err"
for _ in $(seq "$RUNS_P2"); do
  if ! make -j"$NPROC" clean all CCACHE= >"$TMP/p2b.out" 2>&1; then
    fails=$((fails + 1))
    grep -E 'error|Error|cannot find|No such' "$TMP/p2b.out" >>"$TMP/p2b.err"
  fi
done
log "- \`make -j$NPROC clean all\` (clean y compilación en la misma llamada): **$fails fallos de $RUNS_P2**"
if [ "$fails" -gt 0 ]; then
  log ""
  log "Errores más frecuentes:"
  log ""
  log '```'
  sort "$TMP/p2b.err" | uniq -c | sort -rn | head -6 | tee -a "$OUT"
  log '```'
fi
log ""
log "**Causa:** \`clean\` y \`all\` son objetivos independientes (ninguno depende del otro),"
log "así que con \`-j\` make los lanza a la vez: \`clean\` hace \`rm -rf build game\` mientras"
log "\`all\` compila y enlaza: el enlazador falla con \`cannot find build/.../X.o: No such file or directory\`"
log "(o el compilador no encuentra el directorio de salida)."
log ""
log "**Solución:** no mezclar \`clean\` con otros objetivos en una llamada paralela; ejecutarlos en"
log "llamadas separadas (\`make clean && make -jN\`). Comprobación:"
log ""
fails=0
for _ in $(seq "$RUNS_P2"); do
  make clean >/dev/null 2>&1
  make -j"$NPROC" CCACHE= >"$TMP/p2c.out" 2>&1 || fails=$((fails + 1))
done
log "- \`make clean && make -j$NPROC\` (llamadas separadas): **$fails fallos de $RUNS_P2**"
log ""
log "Alternativa en el Makefile (no aplicada): añadir"
log '```make'
log 'ifneq ($(filter clean,$(MAKECMDGOALS)),)'
log '.NOTPARALLEL:'
log 'endif'
log '```'
log "para que make serialice los objetivos cuando se pide \`clean\`."
log ""

# ---- Preguntas 3 y 4 ----------------------------------------------------------
if [ $HAS_CCACHE = 0 ]; then
  log "## 3 y 4. ccache"
  log ""
  log "ccache no está instalado: se omiten las preguntas 3 y 4."
  log "(Instálalo, por ejemplo con 'sudo apt install ccache', y vuelve a ejecutar el script.)"
  echo; echo "Resultados guardados en $OUT"
  exit 0
fi

log "## 3. Compilación con y sin ccache"
log ""
log "> Aplicad ccache en la compilación del proyecto y compara el tiempo de ejecución de la compilación con y sin esta herramienta."
log ""
log "| Escenario | -j1 (s) | -j$NPROC (s) |"
log "|-----------|---------|-------------|"
declare -A R
# "Sin ccache" es la misma medición que la pregunta 1: se reutiliza si existe
for j in 1 "$NPROC"; do
  if [ -n "${AVG[$j]:-}" ]; then
    R[sin,$j]=${AVG[$j]}
  else
    make clean >/dev/null 2>&1; R[sin,$j]=$(timeit make -j"$j" CCACHE=)
  fi
done
# En frío ccache compila todo igual: basta con medirlo con -j$NPROC
ccache -C >/dev/null; make clean >/dev/null 2>&1; R[frio,$NPROC]=$(timeit make -j"$NPROC")
for j in 1 "$NPROC"; do
  make clean >/dev/null 2>&1; R[cal,$j]=$(timeit make -j"$j")
done
log "| Sin ccache | ${R[sin,1]} | ${R[sin,$NPROC]} |"
log "| Con ccache, caché vacía (frío) | — | ${R[frio,$NPROC]} |"
log "| Con ccache, caché llena (caliente) | ${R[cal,1]} | ${R[cal,$NPROC]} |"
log ""

log "## 4. Estadísticas tras limpiar ccache y compilar dos veces"
log ""
log "> Borrad las estadísticas de ccache y limpiadla. Después, ejecutad dos veces la compilación con esta herramienta y analizad las estadísticas que proporciona la herramienta."
log ""
ccache -z >/dev/null
ccache -C >/dev/null
stats() { ccache -sv 2>/dev/null || ccache -s; }
# Resumen hits/misses acumulados (ccache >= 4: --print-stats, formato clave<TAB>valor)
PH=0; PM=0; PU=0   # hits/misses/no cacheables acumulados en la medición anterior
hitrate() {
  local cur h m u
  # No cacheables = todos los contadores de llamadas que ccache no puede cachear
  # (enlazado, sin fichero de entrada, argumentos no soportados, etc.)
  cur=$(ccache --print-stats 2>/dev/null | awk -F'\t' '
    $1 == "direct_cache_hit" || $1 == "preprocessed_cache_hit" { h += $2 }
    $1 == "cache_miss" { m += $2 }
    $1 == "called_for_link" || $1 == "called_for_preprocessing" || $1 == "called_for_cache_dir" \
      || $1 == "unsupported_compiler_option" || $1 == "unsupported_source_language" \
      || $1 == "unsupported_code_directive" || $1 == "no_input_file" || $1 == "multiple_source_files" \
      || $1 == "output_to_stdout" || $1 == "bad_output_file" || $1 == "bad_input_file" \
      || $1 == "autoconf_test" || $1 == "compile_failed" || $1 == "preprocessor_error" \
      || $1 == "bad_compiler_arguments" || $1 == "could_not_use_modules" \
      || $1 == "could_not_use_precompiled_header" { u += $2 }
    END { print h + 0, m + 0, u + 0 }')
  read -r h m u <<<"$cur"
  awk -v h="$h" -v m="$m" -v u="$u" -v ph="$PH" -v pm="$PM" -v pu="$PU" 'BEGIN {
    dh = h - ph; dm = m - pm; du = u - pu; t = dh + dm
    printf "**Resumen de esta compilación:** %d %s, %d %s, hit rate %.1f %% (%d %s, p. ej. el enlazado; no cuentan en el hit rate)\n", dh, (dh == 1 ? "hit" : "hits"), dm, (dm == 1 ? "miss" : "misses"), (t ? dh / t * 100 : 0), du, (du == 1 ? "llamada no cacheable" : "llamadas no cacheables")
  }'
  PH=$h; PM=$m; PU=$u
}
for n in 1 2; do
  make clean >/dev/null 2>&1
  t=$(timeit make -j"$NPROC")
  log "### Compilación $n ($t s)"
  log ""
  log '```'
  stats | tee -a "$OUT"
  log '```'
  log ""
  hitrate >"$TMP/hr"   # sin subshell, para conservar PH/PM
  log "$(cat "$TMP/hr")"
  log ""
done

dt=$(grep -lE '__DATE__|__TIME__|__TIMESTAMP__' $(find src -name '*.cpp' -o -name '*.hpp' -o -name '*.h') 2>/dev/null)
if [ -n "$dt" ]; then
  log "Ficheros que usan \`__DATE__\`/\`__TIME__\` (ccache no puede reutilizarlos, siempre dan miss):"
  log ""
  for f in $dt; do log "- \`$f\`"; done
  log ""
fi

echo
echo "Resultados guardados en $OUT"
