// Corte Maximo (Max-Cut) - algoritmos EXATOS (Trabalho 2, DIM0605) - interface de linha de comando.
//
// Modulos (cabecalhos): grafo.hpp, heuristica_t1.hpp, branch_and_bound.hpp, dp_largura.hpp.
//
// Compilar:  g++ -std=c++17 -O2 -o exato exato.cpp
// Usar:      ./exato                      (menu com as instancias de instancias_t2/ e instancias_t1/)
//            ./exato <instancia> [saida] [opcoes]      (ver README.md)
#include <filesystem>
#include <iomanip>

#include "branch_and_bound.hpp"
#include "dp_largura.hpp"
#include "grafo.hpp"
#include "heuristica_t1.hpp"

namespace fs = std::filesystem;

static string particaoCompacta(const vector<int> &lado) {
    string s = "Particao (lado de cada vertice, 1..n):\n";
    for (size_t i = 0; i < lado.size(); i++) {
        s += char('0' + lado[i]);
        if ((i + 1) % 100 == 0 || i + 1 == lado.size()) s += '\n';
    }
    return s;
}

int main(int argc, char **argv) {
    string instancia, saida, algoritmo = "ambos", csv;
    double tempoMax = 60;
    int larguraMax = 26;
    double memMax = 2000;
    double progresso = 0;
    bool soLargura = false;
    vector<string> pos;
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        auto prox = [&]() -> string { return i + 1 < argc ? argv[++i] : ""; };
        if (a == "--algoritmo") algoritmo = prox();
        else if (a == "--tempo") tempoMax = atof(prox().c_str());
        else if (a == "--largura-max") larguraMax = atoi(prox().c_str());
        else if (a == "--memoria-max") memMax = atof(prox().c_str());
        else if (a == "--csv") csv = prox();
        else if (a == "--progresso") progresso = atof(prox().c_str());
        else if (a == "--so-largura") soLargura = true;
        else pos.push_back(a);
    }
    if (algoritmo != "bb" && algoritmo != "dp" && algoritmo != "ambos") {
        cerr << "--algoritmo deve ser bb, dp ou ambos\n";
        return 1;
    }
    if (!pos.empty()) instancia = pos[0];
    if (pos.size() > 1) saida = pos[1];

    if (instancia.empty()) {  // menu: escolhe o caso de teste dentre os arquivos de instancias_t2/
        vector<string> arquivos;
        for (const char *pasta : {"instancias_longas", "instancias_t2", "instancias_t1"})  // casos novos (T2) e instancias do Trabalho 1
            if (fs::exists(pasta))
                for (auto &e : fs::directory_iterator(pasta))
                    if (e.is_regular_file()) arquivos.push_back(e.path().string());
        sort(arquivos.begin(), arquivos.end());
        if (arquivos.empty()) {
            cerr << "Uso: " << argv[0] << " <instancia> [saida] [opcoes]   (pasta instancias_t2/ nao encontrada)\n";
            return 1;
        }
        cout << "Instancias disponiveis:\n";
        for (size_t i = 0; i < arquivos.size(); i++) cout << "  [" << i + 1 << "] " << arquivos[i] << "\n";
        cout << "Escolha o numero: ";
        size_t k;
        if (!(cin >> k) || k < 1 || k > arquivos.size()) {
            cerr << "Escolha invalida\n";
            return 1;
        }
        instancia = arquivos[k - 1];
    }

    Grafo g;
    if (!lerGrafo(instancia, g)) {
        cerr << "Nao foi possivel ler a instancia '" << instancia << "'\n";
        return 1;
    }
    string nome = fs::path(instancia).filename().string();

    if (soLargura) {  // apenas estima a largura da ordem (para saber se a DP e viavel)
        auto o = melhorOrdemFronteira(g, 5, larguraMax, min(tempoMax, 30.0));
        cout << nome << " n=" << g.n << " m=" << g.m << " largura_estimada=";
        if (o.largura == INT_MAX) cout << ">" << larguraMax;
        else cout << o.largura;
        cout << " (" << o.tempo << " s)\n";
        return 0;
    }

    ostringstream r;
    r << fixed << setprecision(4);
    r << "Instancia: " << instancia << "\nn=" << g.n << " m=" << g.m << " peso_total=" << g.pesoTotal << "\n\n";

    // ---- heuristica do Trabalho 1 ----
    auto t0 = Clock::now();
    vector<int> ladoH = heuristicaT1(g);
    double tempoH = segundos(t0, Clock::now());
    ll valH = valorCorte(g, ladoH);
    r << "[Heuristica T1] guloso+busca local: " << valH << " (" << tempoH << " s)\n";

    ll melhor = valH;
    vector<int> ladoMelhor = ladoH;
    bool provado = false;
    string quemProvou;

    // ---- Algoritmo 1: Branch-and-Bound ----
    ResultadoBB bb;
    bool rodouBB = false;
    if (algoritmo == "bb" || algoritmo == "ambos") {
        if (g.n > 5000) {
            r << "[Branch-and-Bound] nao executado (n > 5000)\n";
        } else {
            rodouBB = true;
            bb = branchAndBound(g, ladoH, tempoMax, progresso);
            r << "[Branch-and-Bound] valor=" << bb.valor << (bb.otimo ? " (OTIMO provado)" : " (limite de tempo: otimalidade NAO provada)")
              << " nos=" << bb.nos << " tempo=" << bb.tempo << " s\n";
            r << "    limitante superior na raiz: " << bb.ubRaiz << "; nos/s: " << (long long)(bb.nos / max(bb.tempo, 1e-9)) << "\n";
            r << "    evolucao do melhor valor (tempo s: valor):";
            for (size_t i = 0; i < bb.trajetoria.size(); i++) {
                if (bb.trajetoria.size() > 12 && i == 6) r << " ...";
                if (bb.trajetoria.size() > 12 && i >= 6 && i + 6 < bb.trajetoria.size()) continue;
                r << " " << setprecision(2) << bb.trajetoria[i].first << ": " << bb.trajetoria[i].second;
            }
            r << setprecision(4) << "\n";
            if (!bb.amostrasNos.empty()) {
                r << "    nos visitados ao longo do tempo (s: nos):";
                for (auto &[t, n] : bb.amostrasNos) r << " " << (long long)t << ": " << n;
                r << "\n";
            }
            if (bb.valor > melhor) melhor = bb.valor, ladoMelhor = bb.lado;
            if (bb.otimo) provado = true, quemProvou = "Branch-and-Bound";
        }
    }

    // ---- Algoritmo 2: DP sobre a largura de caminho ----
    ResultadoDP dp;
    bool rodouDP = false;
    if (algoritmo == "dp" || algoritmo == "ambos") {
        auto o = melhorOrdemFronteira(g, 30, larguraMax, min(tempoMax, 20.0));
        if (o.largura == INT_MAX) {
            r << "[DP largura] inviavel: nao foi encontrada ordem com largura <= " << larguraMax << " (busca: " << o.tempo << " s)\n";
        } else {
            rodouDP = true;
            dp = dpLargura(g, o.ordem, larguraMax, tempoMax, memMax);
            if (dp.ok) {
                r << "[DP largura] largura=" << dp.largura << " valor=" << dp.valor << " (OTIMO) tempo=" << dp.tempo << " s (ordem: "
                  << o.tempo << " s) memoria~" << (long long)dp.memMB << " MB\n";
                if (dp.valor >= melhor) melhor = dp.valor, ladoMelhor = dp.lado;
                provado = true;
                quemProvou = quemProvou.empty() ? "DP largura" : quemProvou + " e DP largura";
            } else {
                r << "[DP largura] largura=" << dp.largura << " nao executada: " << dp.motivo << "\n";
            }
        }
    }

    // ---- conferencia ----
    ll conferido = valorCorte(g, ladoMelhor);
    r << "\n";
    if (provado) {
        r << "Valor otimo: " << melhor << " (provado por " << quemProvou << ")\n";
        r << "Heuristica T1 / otimo: " << (melhor != 0 ? (double)valH / melhor : 0.0) << "\n";
    } else {
        r << "Melhor valor encontrado: " << melhor << " (otimalidade NAO provada)\n";
    }
    bool consistente = true;
    if (rodouBB && bb.otimo && rodouDP && dp.ok && bb.valor != dp.valor) consistente = false;
    if (rodouBB && rodouDP && dp.ok && bb.otimo && bb.valor != dp.valor) r << "ERRO: algoritmos discordam!\n";
    r << "Conferencia: valor recalculado da particao final = " << conferido << (conferido == melhor && consistente ? " (OK)\n" : " (FALHA)\n");
    r << "\n" << particaoCompacta(ladoMelhor);

    cout << r.str();
    if (saida.empty()) {
        fs::create_directories("resultados_t2");
        saida = "resultados_t2/" + nome + ".txt";
    }
    {
        ofstream f(saida);
        // o arquivo guarda o relatorio e a particao no formato "vertice lado" (igual ao do Trabalho 1)
        string rel = r.str();
        rel = rel.substr(0, rel.find("\nParticao (lado de cada vertice"));
        f << rel << "\nParticao (vertice: lado):\n";
        for (int v = 0; v < g.n; v++) f << v + 1 << ' ' << ladoMelhor[v] << '\n';
    }
    cout << "Relatorio e solucao salvos em " << saida << endl;

    if (!csv.empty()) {  // linha para a tabela de experimentos
        bool novo = !fs::exists(csv) || fs::file_size(csv) == 0;
        ofstream f(csv, ios::app);
        if (novo) f << "instancia,n,m,peso_total,heur,t_heur,bb_valor,bb_otimo,bb_nos,bb_tempo,dp_largura,dp_ok,dp_valor,dp_tempo,dp_mem_mb,otimo,ub_raiz,bb_melhoras\n";
        f << nome << ',' << g.n << ',' << g.m << ',' << g.pesoTotal << ',' << valH << ',' << tempoH << ',';
        if (rodouBB) f << bb.valor << ',' << bb.otimo << ',' << bb.nos << ',' << bb.tempo << ',';
        else f << ",,,,";
        if (rodouDP) f << dp.largura << ',' << dp.ok << ',' << (dp.ok ? dp.valor : 0) << ',' << dp.tempo << ',' << (long long)dp.memMB << ',';
        else f << ",,,,,";
        f << (provado ? to_string(melhor) : "") << ',';
        if (rodouBB) f << bb.ubRaiz << ',' << bb.trajetoria.size() - 1;
        else f << ',';
        f << '\n';
    }
    return 0;
}
