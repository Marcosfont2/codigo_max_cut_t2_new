#!/bin/bash
# Gera as 12 instancias do experimento principal (instancias_longas/). Deterministico: sementes fixas.
# Uso (raiz do projeto): bash scripts/gerar_instancias_longas.sh
set -e
g++ -std=c++17 -O2 -o gerador_t2 gerador_t2.cpp
mkdir -p instancias_longas
D=instancias_longas
# (a) aleatorias esparsas pequenas: a DP acha o otimo; o B&B tenta provar
./gerador_t2 aleatorio 100 200 1 1   > $D/01_ALE_n100
./gerador_t2 aleatorio 120 240 1 1   > $D/02_ALE_n120
./gerador_t2 aleatorio 120 240 1 pm1 > $D/03_PM1_n120
# (b) grades toroidais de largura pequena: a DP acha o otimo mesmo com 100 mil vertices
./gerador_t2 toro 8 250 1     > $D/04_TORO_8x250
./gerador_t2 toro 10 200 1    > $D/05_TORO_10x200
./gerador_t2 toro 5 20000 1   > $D/06_TORO_5x20000
./gerador_t2 toro 7 14286 1   > $D/07_TORO_7x14286
# (c) instancias que a DP nao resolve (largura grande): so o B&B, que devolve a melhor solucao encontrada
./gerador_t2 aleatorio 200 400 1 1    > $D/08_ALE_n200
./gerador_t2 aleatorio 500 1000 1 pm1 > $D/09_PM1_n500
./gerador_t2 toro 20 50 1             > $D/10_TORO_20x50
# G32 e G57 vem do Trabalho 1; se instancias_t1/ nao existir, mantem as copias ja presentes
if [ -d instancias_t1 ]; then cp instancias_t1/G32 $D/11_G32; cp instancias_t1/G57 $D/12_G57; fi
ls $D
