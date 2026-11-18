// Grafo ponderado, leitura de instancias e valor de um corte. Tipos e utilitarios comuns a todos os modulos.
#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
using ll = long long;
using Clock = chrono::steady_clock;

struct Grafo {
    int n = 0;
    ll m = 0;          // numero de arestas (apos fundir arestas paralelas)
    ll pesoTotal = 0;  // soma dos pesos (com sinal)
    ll pesoAbs = 0;    // soma dos |pesos|
    vector<vector<pair<int, int>>> adj;  // (vizinho, peso)
};

static double segundos(Clock::time_point a, Clock::time_point b) { return chrono::duration<double>(b - a).count(); }

// Formato: "n m" e depois m linhas "u v peso" (vertices de 1 a n). Ignora lacos e funde arestas paralelas.
bool lerGrafo(const string &caminho, Grafo &g) {
    ifstream in(caminho);
    if (!in) return false;
    ll m;
    if (!(in >> g.n >> m) || g.n <= 0) return false;
    g.adj.assign(g.n, {});
    for (ll i = 0; i < m; i++) {
        int u, v, w;
        if (!(in >> u >> v >> w)) return false;
        u--, v--;
        if (u < 0 || v < 0 || u >= g.n || v >= g.n) return false;
        if (u == v) continue;
        g.adj[u].push_back({v, w});
        g.adj[v].push_back({u, w});
    }
    g.m = 0;
    for (auto &lista : g.adj) {  // funde paralelas: ordena por vizinho e soma os pesos
        sort(lista.begin(), lista.end());
        vector<pair<int, int>> fund;
        for (auto &[u, w] : lista) {
            if (!fund.empty() && fund.back().first == u) fund.back().second += w;
            else fund.push_back({u, w});
        }
        lista = fund;
        g.m += lista.size();
    }
    g.m /= 2;
    for (int v = 0; v < g.n; v++)
        for (auto [u, w] : g.adj[v])
            if (u > v) {
                g.pesoTotal += w;
                g.pesoAbs += abs(w);
            }
    return true;
}

// Valor do corte: soma dos pesos das arestas cujas pontas estao em lados diferentes.
ll valorCorte(const Grafo &g, const vector<int> &lado) {
    ll corte = 0;
    for (int v = 0; v < g.n; v++)
        for (auto [u, w] : g.adj[v])
            if (u > v && lado[u] != lado[v]) corte += w;
    return corte;
}

