# Max-Cut: algoritmos exatos (Trabalho 2)

Branch-and-Bound e programação dinâmica sobre a largura de caminho, mais a heurística do Trabalho 1 para comparação.
Só precisa de `g++` (C++17).

## Rodar as 12 instâncias

```
bash scripts/rodar_experimentos_longos.sh 900
```

- Compila sozinho e roda cada instância de `instancias_longas/` com limite de 900 s (15 min) por algoritmo.
- Duração: cerca de 2h15 no pior caso. Retomável: se interromper, rode o mesmo comando.
- Teste rápido: `bash scripts/rodar_experimentos_longos.sh 20` (depois apague `resultados_longos/`).
- Resultados em `resultados_longos/`: `resultados.csv` e um `.txt` por instância (relatório e partição).

## Tabelas e figuras

```
python3 scripts/gerar_tabelas_figuras.py
```

Lê `resultados_longos/resultados.csv` e grava em `resultados_longos/`: `tabela.md` (também impressa no terminal), `tabela.tex`, `fig_razao.pdf/png` e `fig_tempos.pdf/png`. As tabelas só usam Python 3; as figuras precisam de matplotlib (`pip install matplotlib`).

## Relatório

Fontes LaTeX e PDF em `relatorio/` (`relatorio.tex` é o arquivo principal; compile duas vezes: `pdflatex relatorio.tex` duas vezes seguidas, senão aparecem "??" nas referências). `relatorio/resultados.csv` são os resultados usados no texto (execução com 900 s por algoritmo).

## Rodar uma instância

```
g++ -std=c++17 -O2 -o exato exato.cpp
./exato instancias_longas/04_TORO_8x250
./exato instancias_longas/08_ALE_n200 --algoritmo bb --tempo 900
./exato                       # menu para escolher a instância
```

Opções: `--algoritmo bb|dp|ambos`, `--tempo S`, `--largura-max W` (padrão 26; use 24 se faltar memória), `--csv arquivo`.

## Arquivos

- `exato.cpp`: interface. Módulos: `grafo.hpp`, `heuristica_t1.hpp`, `branch_and_bound.hpp`, `dp_largura.hpp`.
- `scripts/gerar_tabelas_figuras.py`: tabelas e figuras a partir do CSV.
- `gerador_t2.cpp` e `scripts/gerar_instancias_longas.sh`: recriam as instâncias (determinístico).
- `validacao.cpp`: compara os algoritmos com força bruta (`g++ -std=c++17 -O2 -o validacao validacao.cpp && ./validacao 3000 7`).
