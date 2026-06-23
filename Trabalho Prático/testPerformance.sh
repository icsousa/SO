#!/bin/bash

# Tamanhos de cache a testar
CACHE_SIZES=(1 2 10 50)
CONCURRENCIES=(1 2 5 10)

CATALOG="Gcatalog.tsv"
DOC_DIR="Gdataset/"

SERVER="./bin/dserver"
CLIENT="./bin/dclient"
FIFO="/tmp/dserver_fifo"

INDEX_SCRIPT="./addGdatasetMetadata.sh"

TOTAL_DOCS=$(( $(wc -l < "$CATALOG") - 1 ))  # ignora header

for CACHE in "${CACHE_SIZES[@]}"; do

    echo ""
    echo "==== cache_size → $CACHE ===="
    echo ""

    make
    echo ""

    $SERVER "$DOC_DIR" "$CACHE" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1

    $INDEX_SCRIPT "$CATALOG" > /dev/null 2>&1
    sleep 1

    for N in "${CONCURRENCIES[@]}"; do
        START=$(date +%s%3N)
        $CLIENT -s "and" "$N" > /dev/null 2>&1
        END=$(date +%s%3N)
        DIFF=$((END - START))
        echo "[CACHE=$CACHE] -s \"and\" com $N processos → ${DIFF}ms"
    done

    # Testar -l em todos os documentos
    START=$(date +%s%3N)
    for ((i=1; i<=TOTAL_DOCS; i++)); do
        $CLIENT -l "$i" "and" > /dev/null 2>&1
    done
    END=$(date +%s%3N)
    DIFF=$((END - START))
    echo "[CACHE=$CACHE] -l <todos> \"and\" → ${DIFF}ms"

    # Testar -c em todos os documentos
    START=$(date +%s%3N)
    for ((i=1; i<=TOTAL_DOCS; i++)); do
        $CLIENT -c "$i" > /dev/null 2>&1
    done
    END=$(date +%s%3N)
    DIFF=$((END - START))
    echo "[CACHE=$CACHE] -c <todos> → ${DIFF}ms"

    # Testar -d em todos os documentos
    START=$(date +%s%3N)
    for ((i=1; i<=TOTAL_DOCS; i++)); do
        $CLIENT -d "$i" > /dev/null 2>&1
    done
    END=$(date +%s%3N)
    DIFF=$((END - START))
    echo "[CACHE=$CACHE] -d <todos> → ${DIFF}ms"

    echo ""

    kill -9 $SERVER_PID
    rm -f $FIFO
    sleep 1

    make clean
done
