// Heuristica do Trabalho 1 (gulosa por grau + busca local 1-flip).
// Usada como limite inferior inicial do Branch-and-Bound e como referencia de comparacao.
#pragma once
#include "grafo.hpp"

vector<int> ordemPorGrau(const Grafo &g) {
    vector<int> ordem(g.n);
    iota(ordem.begin(), ordem.end(), 0);
    sort(ordem.begin(), ordem.end(), [&](int a, int b) { return g.adj[a].size() > g.adj[b].size(); });
    return ordem;
}

vector<int> construcaoGulosa(const Grafo &g, const vector<int> &ordem) {
    vector<int> lado(g.n, -1);
    for (int v : ordem) {
        ll peso0 = 0, peso1 = 0;
        for (auto [u, w] : g.adj[v]) {
            if (lado[u] == 0) peso0 += w;
            else if (lado[u] == 1) peso1 += w;
        }
        lado[v] = (peso0 >= peso1) ? 1 : 0;
    }
    return lado;
}

ll ganhoDeTroca(const Grafo &g, const vector<int> &lado, int v) {
    ll ganho = 0;
    for (auto [u, w] : g.adj[v]) ganho += (lado[u] == lado[v]) ? w : -w;
    return ganho;
}

void buscaLocal(const Grafo &g, vector<int> &lado, const vector<int> &ordem) {
    deque<int> fila(ordem.begin(), ordem.end());
    vector<char> naFila(g.n, 1);
    while (!fila.empty()) {
        int v = fila.front();
        fila.pop_front();
        naFila[v] = 0;
        if (ganhoDeTroca(g, lado, v) <= 0) continue;
        lado[v] ^= 1;
        for (auto [u, w] : g.adj[v])
            if (!naFila[u]) {
                naFila[u] = 1;
                fila.push_back(u);
            }
    }
}

vector<int> heuristicaT1(const Grafo &g) {
    vector<int> ordem = ordemPorGrau(g);
    vector<int> lado = construcaoGulosa(g, ordem);
    buscaLocal(g, lado, ordem);
    return lado;
}

