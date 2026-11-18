// Algoritmo exato 1: Branch-and-Bound com limitante superior incremental (ver Secao 5.1 do relatorio).
#pragma once
#include "grafo.hpp"

struct ResultadoBB {
    ll valor = 0;
    vector<int> lado;
    bool otimo = false;  // true se a busca terminou (otimalidade provada)
    ll nos = 0;
    double tempo = 0;
    ll ubRaiz = 0;                              // limitante superior no no raiz (antes de qualquer decisao)
    vector<pair<double, ll>> trajetoria;        // (tempo, melhor valor) a cada melhoria da solucao incumbente
    vector<pair<double, ll>> amostrasNos;       // (tempo, nos visitados) a cada intervalo de progresso
};

// Ordem de decisao dos vertices: o proximo e o que tem maior peso (|w|) para vertices ja ordenados,
// de modo que as arestas fiquem "decididas" cedo e o limitante se torne justo rapidamente.
// novaComp[i] indica que o i-esimo vertice abre uma nova componente conexa (simetria: pode fixar lado 0).
static vector<int> ordemDecisao(const Grafo &g, vector<char> &novaComp) {
    int n = g.n;
    vector<ll> ligado(n, 0), grauPond(n, 0);
    for (int v = 0; v < n; v++)
        for (auto [u, w] : g.adj[v]) grauPond[v] += abs(w);
    vector<char> usado(n, 0);
    vector<int> ordem;
    novaComp.assign(n, 0);
    for (int i = 0; i < n; i++) {
        int melhor = -1;
        for (int v = 0; v < n; v++) {
            if (usado[v]) continue;
            if (melhor < 0 || ligado[v] > ligado[melhor] || (ligado[v] == ligado[melhor] && grauPond[v] > grauPond[melhor]))
                melhor = v;
        }
        novaComp[i] = (ligado[melhor] == 0);
        usado[melhor] = 1;
        ordem.push_back(melhor);
        for (auto [u, w] : g.adj[melhor]) ligado[u] += abs(w);
    }
    return ordem;
}

struct BranchAndBound {
    const Grafo &g;
    int n;
    vector<int> ordem;
    vector<char> novaComp;
    vector<int> lado;           // -1 = nao decidido, 0 ou 1
    vector<array<ll, 2>> c;     // c[v][s] = peso das arestas de v para vizinhos ja decididos no lado s
    ll corte = 0;               // valor das arestas ja decididas (ambas as pontas decididas) que estao cortadas
    ll somaMax = 0;             // soma, sobre v nao decidido, de max(c[v][0], c[v][1])
    ll ubNN = 0;                // soma de max(w, 0) das arestas com as duas pontas nao decididas
    ll melhor = 0;
    vector<int> melhorLado;
    ll nos = 0;
    bool estourou = false;
    double limite;
    double progresso = 0, ultimoProg = 0;  // intervalo (s) entre mensagens de progresso; 0 = sem mensagens
    vector<pair<double, ll>> trajetoria, amostrasNos;
    Clock::time_point t0;

    BranchAndBound(const Grafo &grafo, double limiteTempo) : g(grafo), n(grafo.n), limite(limiteTempo) {
        ordem = ordemDecisao(g, novaComp);
        lado.assign(n, -1);
        c.assign(n, {0, 0});
        for (int v = 0; v < n; v++)
            for (auto [u, w] : g.adj[v])
                if (u > v) ubNN += max(w, 0);
    }

    // Decide v no lado s, atualizando de forma incremental o corte e as parcelas do limitante. O(grau(v)).
    void colocar(int v, int s) {
        corte += c[v][1 - s];  // arestas de v para decididos do lado oposto passam a ser cortadas
        somaMax -= max(c[v][0], c[v][1]);
        lado[v] = s;
        for (auto [u, w] : g.adj[v]) {
            if (lado[u] >= 0) continue;
            ubNN -= max(w, 0);
            somaMax -= max(c[u][0], c[u][1]);
            c[u][s] += w;
            somaMax += max(c[u][0], c[u][1]);
        }
    }
    void desfazer(int v, int s) {
        lado[v] = -1;
        for (auto [u, w] : g.adj[v]) {
            if (lado[u] >= 0) continue;
            somaMax -= max(c[u][0], c[u][1]);
            c[u][s] -= w;
            somaMax += max(c[u][0], c[u][1]);
            ubNN += max(w, 0);
        }
        somaMax += max(c[v][0], c[v][1]);
        corte -= c[v][1 - s];
    }

    void rec(int i) {
        if (estourou) return;
        if ((++nos & 4095) == 0) {
            double t = segundos(t0, Clock::now());
            if (t > limite) {
                estourou = true;
                return;
            }
            if (progresso > 0 && t - ultimoProg >= progresso) {
                ultimoProg = t;
                amostrasNos.push_back({t, nos});
                cerr << "  [B&B] t=" << (long long)t << " s  nos=" << nos << "  melhor=" << melhor << endl;
            }
        }
        // Limitante superior: nenhum completamento da solucao parcial passa deste valor.
        if (corte + somaMax + ubNN <= melhor) return;
        if (i == n) {  // aqui somaMax = ubNN = 0, logo corte > melhor
            melhor = corte;
            melhorLado = lado;
            trajetoria.push_back({segundos(t0, Clock::now()), melhor});
            return;
        }
        int v = ordem[i];
        int primeiro = (c[v][1] >= c[v][0]) ? 0 : 1;  // lado que corta mais peso primeiro
        for (int k = 0; k < 2; k++) {
            int s = k == 0 ? primeiro : 1 - primeiro;
            if (novaComp[i] && s == 1) continue;  // quebra de simetria por componente
            colocar(v, s);
            rec(i + 1);
            desfazer(v, s);
            if (estourou) return;
        }
    }
};

ResultadoBB branchAndBound(const Grafo &g, const vector<int> &ladoInicial, double limiteTempo, double progresso = 0) {
    BranchAndBound bb(g, limiteTempo);
    bb.t0 = Clock::now();
    bb.progresso = progresso;
    bb.melhorLado = ladoInicial;
    bb.melhor = valorCorte(g, ladoInicial);
    bb.trajetoria.push_back({0.0, bb.melhor});
    ll ubRaiz = bb.corte + bb.somaMax + bb.ubNN;
    bb.rec(0);
    ResultadoBB r;
    r.valor = bb.melhor;
    r.lado = bb.melhorLado;
    r.otimo = !bb.estourou;
    r.nos = bb.nos;
    r.ubRaiz = ubRaiz;
    r.trajetoria = bb.trajetoria;
    r.amostrasNos = bb.amostrasNos;
    r.tempo = segundos(bb.t0, Clock::now());
    return r;
}

