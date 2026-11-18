// Validacao dos algoritmos exatos contra forca bruta em milhares de grafos pequenos aleatorios.
// (A forca bruta so e usada aqui, como oraculo de teste; nao faz parte da entrega algoritmica.)
// Compilar: g++ -std=c++17 -O2 -o validacao validacao.cpp      Uso: ./validacao [numero_de_casos] [semente]
#include "branch_and_bound.hpp"
#include "dp_largura.hpp"
#include "grafo.hpp"
#include "heuristica_t1.hpp"

static ll forcaBruta(const Grafo &g) {
    int n = g.n;
    ll melhor = LLONG_MIN;
    vector<int> lado(n);
    for (ll mask = 0; mask < (1LL << (n - 1)); mask++) {  // vertice n-1 fixo no lado 0
        for (int v = 0; v < n; v++) lado[v] = (mask >> v) & 1;
        lado[n - 1] = 0;
        melhor = max(melhor, valorCorte(g, lado));
    }
    return melhor;
}

int main(int argc, char **argv) {
    int casos = argc > 1 ? atoi(argv[1]) : 2000;
    mt19937 rng(argc > 2 ? atoi(argv[2]) : 7);
    int falhas = 0;
    for (int c = 0; c < casos; c++) {
        Grafo g;
        g.n = 1 + rng() % 16;
        g.adj.assign(g.n, {});
        int tipoPeso = rng() % 4;  // 0: todos 1, 1: +-1, 2: pesos inteiros em [-4,6], 3: positivos em [1,9]
        double dens = (rng() % 100) / 100.0;
        for (int u = 0; u < g.n; u++)
            for (int v = u + 1; v < g.n; v++)
                if ((rng() % 1000) / 1000.0 < dens) {
                    int w = tipoPeso == 0 ? 1 : tipoPeso == 1 ? ((rng() & 1) ? 1 : -1) : tipoPeso == 2 ? (int)(rng() % 11) - 4 : 1 + (int)(rng() % 9);
                    if (w == 0) continue;
                    g.adj[u].push_back({v, w});
                    g.adj[v].push_back({u, w});
                    g.m++;
                    g.pesoTotal += w;
                    g.pesoAbs += abs(w);
                }
        ll ref = forcaBruta(g);
        ResultadoBB bb = branchAndBound(g, heuristicaT1(g), 1e9);
        OrdemFronteira o = melhorOrdemFronteira(g, 5, 30, 5);
        ResultadoDP dp = dpLargura(g, o.ordem, 30, 1e9, 4000);
        bool ok = bb.otimo && bb.valor == ref && valorCorte(g, bb.lado) == ref && dp.ok && dp.valor == ref && valorCorte(g, dp.lado) == ref;
        if (!ok) {
            falhas++;
            cerr << "FALHA caso " << c << ": n=" << g.n << " m=" << g.m << " forca_bruta=" << ref << " bb=" << bb.valor << " dp=" << dp.valor
                 << " dp_ok=" << dp.ok << " valorDP_recalc=" << (dp.ok ? valorCorte(g, dp.lado) : -1) << "\n";
        }
    }
    cout << casos << " casos, " << falhas << " falhas\n";
    return falhas != 0;
}
