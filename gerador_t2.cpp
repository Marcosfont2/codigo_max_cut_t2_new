// Gerador de instancias do Trabalho 2 (mesmo formato do Trabalho 1: "n m" e depois "u v peso").
//
//   ./gerador_t2 aleatorio <n> <m> <semente> <pesos>     pesos = 1 (todos +1)  ou  pm1 (+1/-1 com prob. 1/2)
//   ./gerador_t2 toro <linhas> <colunas> <semente>       grade toroidal 2D com pesos +1/-1 e rotulos embaralhados
//
// O gerador e deterministico: mesmos argumentos geram o mesmo arquivo. A saida vai para a saida padrao.
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>
using namespace std;
using ll = long long;

int main(int argc, char **argv) {
    string tipo = argc > 1 ? argv[1] : "";
    if (tipo == "aleatorio" && argc == 6) {
        ll n = atoll(argv[2]), m = atoll(argv[3]);
        mt19937_64 rng(atoll(argv[4]));
        bool pm1 = string(argv[5]) == "pm1";
        if (n < 2 || m > n * (n - 1) / 2) {
            cerr << "m deve ser no maximo n(n-1)/2\n";
            return 1;
        }
        set<pair<ll, ll>> e;
        while ((ll)e.size() < m) {
            ll u = rng() % n + 1, v = rng() % n + 1;
            if (u == v) continue;
            if (u > v) swap(u, v);
            e.insert({u, v});
        }
        cout << n << ' ' << m << '\n';
        for (auto [u, v] : e) cout << u << ' ' << v << ' ' << (pm1 && (rng() & 1) ? -1 : 1) << '\n';
    } else if (tipo == "toro" && argc == 5) {
        int L = atoi(argv[2]), C = atoi(argv[3]);
        mt19937_64 rng(atoll(argv[4]));
        if (L < 3 || C < 3) {
            cerr << "linhas e colunas devem ser >= 3\n";
            return 1;
        }
        int n = L * C;
        vector<int> rot(n);  // rotulos embaralhados: a estrutura de grade nao aparece na numeracao
        iota(rot.begin(), rot.end(), 1);
        shuffle(rot.begin(), rot.end(), rng);
        cout << n << ' ' << 2 * n << '\n';
        for (int i = 0; i < L; i++)
            for (int j = 0; j < C; j++) {
                int a = i * C + j;
                int direita = i * C + (j + 1) % C, abaixo = ((i + 1) % L) * C + j;
                cout << rot[a] << ' ' << rot[direita] << ' ' << ((rng() & 1) ? -1 : 1) << '\n';
                cout << rot[a] << ' ' << rot[abaixo] << ' ' << ((rng() & 1) ? -1 : 1) << '\n';
            }
    } else {
        cerr << "Uso:\n  " << argv[0] << " aleatorio <n> <m> <semente> <1|pm1>\n  " << argv[0] << " toro <linhas> <colunas> <semente>\n";
        return 1;
    }
}
