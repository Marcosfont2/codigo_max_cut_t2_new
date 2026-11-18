#!/bin/bash
# Roda os dois algoritmos exatos nas 12 instancias de instancias_longas/, com limite de tempo POR ALGORITMO
# (padrao 900 s = 15 min). Retomavel: instancias ja presentes em resultados_longos/resultados.csv sao puladas.
# Uso (raiz do projeto): bash scripts/rodar_experimentos_longos.sh [tempo_limite_s]
set -e
T=${1:-900}
g++ -std=c++17 -O2 -o exato exato.cpp
mkdir -p resultados_longos
for f in instancias_longas/*; do
  nome=$(basename "$f")
  if [ -f resultados_longos/resultados.csv ] && grep -q "^$nome," resultados_longos/resultados.csv; then continue; fi
  echo ">>> $nome (limite $T s por algoritmo)"
  ./exato "$f" "resultados_longos/$nome.txt" --tempo $T --largura-max 26 --csv resultados_longos/resultados.csv \
      | grep -E "^\[|Valor otimo|Melhor valor|Conferencia"
done
echo "Concluido. Para gerar tabelas e figuras: python3 scripts/gerar_tabelas_figuras.py"
