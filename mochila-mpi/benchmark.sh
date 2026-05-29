INSTANCE=${1:-data/large_scale/knapPI_3_5000_1000_1}
POP_TOTAL=${2:-800}
GEN=${3:-1000}
MUT=0.02
TOUR=3
SEED=42
MIG=50

# Se estiver como root (container), libera o mpirun
export OMPI_ALLOW_RUN_AS_ROOT=1
export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

echo "Instancia : $INSTANCE"
echo "Pop total : $POP_TOTAL (dividida entre os processos)"
echo "Geracoes  : $GEN"
echo ""
printf "%-10s %-12s %-12s %-10s %-10s\n" "Procs" "Pop/ilha" "Tempo(s)" "Speedup" "Fitness"

T1=""
for P in 1 2 4 8; do
    POP=$((POP_TOTAL / P))
    OUT=$(mpirun --oversubscribe -np $P ./mochila_mpi "$INSTANCE" $POP $GEN $MUT $TOUR $SEED $MIG 2>/dev/null)
    T=$(echo "$OUT" | grep "Tempo" | awk '{print $5}')
    F=$(echo "$OUT" | grep "^Fitness" | awk '{print $3}')

    if [ "$P" -eq 1 ]; then T1=$T; SPEEDUP="1.00"; \
    else SPEEDUP=$(echo "scale=2; $T1/$T" | bc); fi

    printf "%-10s %-12s %-12s %-10s %-10s\n" "$P" "$POP" "$T" "$SPEEDUP" "$F"
done
