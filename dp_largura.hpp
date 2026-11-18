// Algoritmo exato 2: programacao dinamica sobre a fronteira de uma ordem dos vertices
// (largura de caminho; ver Secao 5.2 do relatorio).
#pragma once
#include "grafo.hpp"

// Ordem de processamento escolhida de forma gulosa para minimizar a fronteira F (vertices ja processados
// que ainda tem vizinho nao processado). A tabela da DP tem 2^K entradas, K = |F| + 1 no pior passo.
// Devolve K maximo (ou INT_MAX se passar de 'corte').
static int ordemFronteiraGulosa(const Grafo &g, int inicio, mt19937 &rng, int corte, vector<int> &ordem) {
    int n = g.n;
    vector<int> cnt(n);  // vizinhos ainda nao processados
    for (int v = 0; v < n; v++) cnt[v] = g.adj[v].size();
    vector<char> proc(n, 0), noF(n, 0), ehCand(n, 0);
    vector<int> cand;
    ordem.clear();
    int tamF = 0, largura = 0, proxLivre = 0;
    for (int t = 0; t < n; t++) {
        int v = -1;
        if (t == 0) {
            v = inicio;
        } else {
            size_t k = 0;
            for (int x : cand)
                if (!proc[x]) cand[k++] = x;
            cand.resize(k);
            if (cand.empty()) {  // nova componente conexa
                while (proc[proxLivre]) proxLivre++;
                v = proxLivre;
            } else {
                int melhorDelta = INT_MAX, melhorCnt = INT_MAX;
                uint32_t melhorSorteio = 0;
                for (int x : cand) {
                    int d = cnt[x] > 0 ? 1 : 0;
                    for (auto [u, w] : g.adj[x])
                        if (noF[u] && cnt[u] == 1) d--;
                    uint32_t sorteio = rng();
                    if (d < melhorDelta || (d == melhorDelta && (cnt[x] < melhorCnt || (cnt[x] == melhorCnt && sorteio < melhorSorteio)))) {
                        melhorDelta = d, melhorCnt = cnt[x], melhorSorteio = sorteio, v = x;
                    }
                }
            }
        }
        largura = max(largura, tamF + 1);
        if (largura > corte) return INT_MAX;
        proc[v] = 1;
        ordem.push_back(v);
        for (auto [u, w] : g.adj[v]) cnt[u]--;
        if (cnt[v] > 0) noF[v] = 1, tamF++;
        for (auto [u, w] : g.adj[v]) {
            if (noF[u] && cnt[u] == 0) noF[u] = 0, tamF--;
            if (!proc[u] && !ehCand[u]) ehCand[u] = 1, cand.push_back(u);
        }
    }
    return largura;
}

struct OrdemFronteira {
    vector<int> ordem;
    int largura = INT_MAX;  // INT_MAX = nenhuma ordem com largura <= corte foi encontrada
    double tempo = 0;
};

// Varias partidas (vertice inicial e desempates diferentes); fica com a de menor largura.
OrdemFronteira melhorOrdemFronteira(const Grafo &g, int tentativas, int corte, double tempoMax) {
    auto t0 = Clock::now();
    mt19937 rng(12345);
    OrdemFronteira melhor;
    vector<int> ordem;
    int inicioGrauMin = 0;
    for (int v = 0; v < g.n; v++)
        if (g.adj[v].size() < g.adj[inicioGrauMin].size()) inicioGrauMin = v;
    for (int i = 0; i < tentativas; i++) {
        int inicio = (i == 0) ? inicioGrauMin : (int)(rng() % g.n);
        int limite = min(corte, melhor.largura == INT_MAX ? corte : melhor.largura - 1);
        int w = ordemFronteiraGulosa(g, inicio, rng, limite, ordem);
        if (w != INT_MAX && w < melhor.largura) melhor.largura = w, melhor.ordem = ordem;
        if (segundos(t0, Clock::now()) > tempoMax) break;
    }
    melhor.tempo = segundos(t0, Clock::now());
    return melhor;
}

struct ResultadoDP {
    bool ok = false;
    string motivo;  // se !ok
    ll valor = 0;
    vector<int> lado;
    int largura = 0;
    double tempo = 0, memMB = 0;
};

ResultadoDP dpLargura(const Grafo &g, const vector<int> &ordem, int larguraMax, double limiteTempo, double memMaxMB) {
    ResultadoDP r;
    int n = g.n;
    if (g.pesoAbs > 1000000000LL) {
        r.motivo = "soma dos pesos grande demais para a tabela de 32 bits";
        return r;
    }
    // --- simulacao previa: largura e memoria necessaria para reconstruir a solucao ---
    {
        vector<int> cnt(n);
        for (int v = 0; v < n; v++) cnt[v] = g.adj[v].size();
        vector<char> proc(n, 0), noF(n, 0);
        int tamF = 0, K = 0;
        double bits = 0;
        for (int v : ordem) {
            K = max(K, tamF + 1);
            proc[v] = 1;
            for (auto [u, w] : g.adj[v]) cnt[u]--;
            vector<int> sai;
            if (cnt[v] > 0) noF[v] = 1;
            tamF++;
            if (cnt[v] == 0) sai.push_back(v);
            for (auto [u, w] : g.adj[v])
                if (noF[u] && cnt[u] == 0) sai.push_back(u), noF[u] = 0;
            int atual = tamF;
            for (size_t i = 0; i < sai.size(); i++) {
                bits += ldexp(1.0, atual - 1);
                atual--;
            }
            tamF = atual;
            if (K > larguraMax) break;
        }
        r.largura = K;
        if (K > larguraMax) {
            r.motivo = "largura " + to_string(K) + " > limite " + to_string(larguraMax);
            return r;
        }
        double mem = bits / 8.0 / 1048576.0 + 3.0 * ldexp(4.0, K) / 1048576.0;
        r.memMB = mem;
        if (mem > memMaxMB) {
            r.motivo = "memoria estimada " + to_string((long long)mem) + " MB > limite";
            return r;
        }
    }

    auto t0 = Clock::now();
    const int NEG = INT_MIN / 4;
    struct Esquecimento {
        int j, vertice;
        vector<uint64_t> bits;  // bit m = 1 se, no estado m (sem o vertice), o vertice estava no lado 1
    };
    vector<vector<Esquecimento>> registro(n);
    vector<int> kAntes(n);
    vector<int> cnt(n);
    for (int v = 0; v < n; v++) cnt[v] = g.adj[v].size();
    vector<char> proc(n, 0);
    vector<int> F, pos(n, -1);
    vector<int> f(1, 0), h;

    for (int t = 0; t < n; t++) {
        int v = ordem[t];
        int k = F.size();
        kAntes[t] = k;
        vector<pair<int, int>> viz;  // (bit na fronteira, peso) dos vizinhos ja processados
        for (auto [u, w] : g.adj[v])
            if (proc[u]) viz.push_back({pos[u], w});
        h.assign((size_t)1 << (k + 1), NEG);
        bool novaComp = (k == 0);  // fronteira vazia: v abre componente nova, simetria => lado 0
        for (size_t mask = 0; mask < ((size_t)1 << k); mask++) {
            ll g0 = 0, g1 = 0;
            for (auto [j, w] : viz) {
                if ((mask >> j) & 1) g0 += w;  // v no lado 0 corta arestas para vizinhos no lado 1
                else g1 += w;                  // v no lado 1 corta arestas para vizinhos no lado 0
            }
            h[mask] = f[mask] + (int)g0;
            if (!novaComp) h[mask | ((size_t)1 << k)] = f[mask] + (int)g1;
        }
        F.push_back(v);
        pos[v] = k;
        proc[v] = 1;
        for (auto [u, w] : g.adj[v]) cnt[u]--;
        // esquece (maximiza sobre) os vertices que nao tem mais vizinhos nao processados, do maior bit ao menor
        for (int j = (int)F.size() - 1; j >= 0; j--) {
            if (cnt[F[j]] != 0) continue;
            int K = F.size();
            size_t metade = (size_t)1 << (K - 1);
            f.assign(metade, 0);
            Esquecimento e;
            e.j = j;
            e.vertice = F[j];
            e.bits.assign((metade + 63) / 64, 0);
            size_t baixo = ((size_t)1 << j) - 1;
            for (size_t m2 = 0; m2 < metade; m2++) {
                size_t m0 = (m2 & baixo) | ((m2 >> j) << (j + 1));
                size_t m1 = m0 | ((size_t)1 << j);
                if (h[m1] > h[m0]) {
                    f[m2] = h[m1];
                    e.bits[m2 >> 6] |= (uint64_t)1 << (m2 & 63);
                } else {
                    f[m2] = h[m0];
                }
            }
            h.swap(f);  // h passa a ser a tabela sem o vertice; f e reutilizado
            F.erase(F.begin() + j);
            for (size_t i = j; i < F.size(); i++) pos[F[i]] = i;
            pos[e.vertice] = -1;
            registro[t].push_back(move(e));
        }
        f.swap(h);
        if (segundos(t0, Clock::now()) > limiteTempo) {
            r.motivo = "limite de tempo";
            r.tempo = segundos(t0, Clock::now());
            return r;
        }
    }
    r.valor = f[0];

    // --- reconstrucao da particao otima ---
    r.lado.assign(n, 0);
    uint64_t mask = 0;
    for (int t = n - 1; t >= 0; t--) {
        for (int i = (int)registro[t].size() - 1; i >= 0; i--) {
            auto &e = registro[t][i];
            int b = (e.bits[mask >> 6] >> (mask & 63)) & 1;
            r.lado[e.vertice] = b;
            uint64_t baixo = mask & (((uint64_t)1 << e.j) - 1), alto = mask >> e.j;
            mask = baixo | ((uint64_t)b << e.j) | (alto << (e.j + 1));
        }
        int k = kAntes[t];
        uint64_t baixo = mask & (((uint64_t)1 << k) - 1), alto = mask >> (k + 1);
        mask = baixo | (alto << k);
    }
    r.ok = true;
    r.tempo = segundos(t0, Clock::now());
    return r;
}

