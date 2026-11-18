#!/usr/bin/env python3
"""Gera tabelas e figuras a partir de resultados_longos/resultados.csv.

Saidas (em resultados_longos/):
  tabela.md          tabela completa em texto (Markdown), tambem impressa no terminal
  tabela.tex         tabela principal em LaTeX (valores, razao da heuristica, DP)
  tabela_bb.tex      tabela do Branch-and-Bound em LaTeX (nos, limitante da raiz, melhorias)
  fig_razao.pdf/png  heuristica do Trabalho 1 e B&B (15 min) em relacao ao otimo, instancias com otimo provado
  fig_tempos.pdf/png tempo do B&B e da DP nas instancias em que a DP termina (escala logaritmica)
As tabelas usam so a biblioteca padrao; as figuras exigem matplotlib (pip install matplotlib).
Uso (raiz do projeto): python3 scripts/gerar_tabelas_figuras.py
"""
import csv, sys

ARQ = "resultados_longos/resultados.csv"
try:
    linhas = list(csv.DictReader(open(ARQ)))
except FileNotFoundError:
    sys.exit(f"{ARQ} nao encontrado: rode antes scripts/rodar_experimentos_longos.sh")
linhas.sort(key=lambda r: r["instancia"])

def inteiro(x): return int(float(x)) if x not in ("", None) else None
def real(x): return float(x) if x not in ("", None) else None
def virg(s): return s.replace(".", "{,}")

dados = []
for r in linhas:
    d = dict(nome=r["instancia"], n=inteiro(r["n"]), m=inteiro(r["m"]), heur=inteiro(r["heur"]),
             bb_valor=inteiro(r["bb_valor"]), bb_otimo=r["bb_otimo"] == "1", bb_nos=inteiro(r["bb_nos"]),
             bb_tempo=real(r["bb_tempo"]), dp_w=inteiro(r["dp_largura"]), dp_ok=r["dp_ok"] == "1",
             dp_valor=inteiro(r["dp_valor"]) if r["dp_ok"] == "1" else None, dp_tempo=real(r["dp_tempo"]),
             dp_mem=inteiro(r["dp_mem_mb"]), ub_raiz=inteiro(r.get("ub_raiz", "")),
             melhoras=inteiro(r.get("bb_melhoras", "")), otimo=inteiro(r["otimo"]))
    cand = [v for v in (d["heur"], d["bb_valor"], d["dp_valor"]) if v is not None]
    d["melhor"] = max(cand)
    d["razao"] = d["heur"] / d["otimo"] if d["otimo"] else None
    d["razao_bb"] = d["bb_valor"] / d["otimo"] if (d["otimo"] and d["bb_valor"] is not None) else None
    d["rotulo"] = d["nome"].split("_", 1)[1] if "_" in d["nome"] else d["nome"]
    dados.append(d)

def fmt_t(t):
    if t is None: return "---"
    if t < 0.001: return "<0,001"
    if t < 10: return f"{t:.3f}".replace(".", ",")
    if t < 100: return f"{t:.1f}".replace(".", ",")
    return f"{t:.0f}"
def esc(s): return s.replace("_", "\\_")

# ---------- Markdown ----------
md = ["| Instância | n | m | Valor final | Situação | Heurística T1 | Razão | B&B (valor, situação, tempo) | DP (largura, tempo) |",
      "|---|---:|---:|---:|---|---:|---:|---|---|"]
for d in dados:
    raz = f"{d['razao']:.3f}".replace(".", ",") if d["razao"] else "---"
    bb = "não roda (n>5000)" if d["bb_valor"] is None else f"{d['bb_valor']} ({'ótimo' if d['bb_otimo'] else 'limite'}), {fmt_t(d['bb_tempo'])} s"
    dp = "inviável (w>26)" if d["dp_w"] is None else f"w={d['dp_w']}, {fmt_t(d['dp_tempo'])} s"
    md.append(f"| {d['nome']} | {d['n']} | {d['m']} | {d['melhor']} | {'ótimo' if d['otimo'] is not None else 'melhor encontrado'} | {d['heur']} | {raz} | {bb} | {dp} |")
open("resultados_longos/tabela.md", "w").write("\n".join(md) + "\n")
print("\n".join(md))

# ---------- LaTeX: tabela principal ----------
tex = ["\\begin{tabular}{lrrrrcrrr}\\hline",
       "Instância & $n$ & $m$ & Ótimo/melhor & Heur. & Razão & $w$ & DP (s) & Mem. (MB) \\\\\\hline"]
for d in dados:
    raz = virg(f"{d['razao']:.3f}") if d["razao"] else "---"
    final = f"{d['melhor']}" + ("" if d["otimo"] is not None else "$^*$")
    w = d["dp_w"] if d["dp_w"] is not None else "---"
    tdp = fmt_t(d["dp_tempo"]).replace("<", "$<$").replace(",", "{,}") if d["dp_ok"] else "---"
    mem = d["dp_mem"] if d["dp_ok"] else "---"
    tex.append(f"{esc(d['nome'])} & {d['n']:,} & {d['m']:,} & {final:s} & {d['heur']} & {raz} & {w} & {tdp} & {mem} \\\\".replace(",", "\\,") if False else
               f"{esc(d['nome'])} & {d['n']} & {d['m']} & {final} & {d['heur']} & {raz} & {w} & {tdp} & {mem} \\\\")
tex += ["\\hline\\end{tabular}"]
open("resultados_longos/tabela.tex", "w").write("\n".join(tex) + "\n")

# ---------- LaTeX: tabela do B&B ----------
tb = ["\\begin{tabular}{lrcrrrrrr}\\hline",
      "Instância & Valor B\\&B & Situação & Tempo (s) & Nós ($10^9$) & Nós/s ($10^6$) & UB raiz & Melhorias & UB/final \\\\\\hline"]
for d in dados:
    if d["bb_valor"] is None:
        tb.append(f"{esc(d['nome'])} & \\multicolumn{{8}}{{c}}{{não executado ($n>5000$)}} \\\\")
        continue
    sit = "ótimo" if d["bb_otimo"] else "limite"
    nos = virg(f"{d['bb_nos']/1e9:.1f}")
    nps = virg(f"{d['bb_nos']/d['bb_tempo']/1e6:.1f}")
    ub = f"{d['ub_raiz']}"
    rel = virg(f"{d['ub_raiz']/d['melhor']:.2f}")
    tb.append(f"{esc(d['nome'])} & {d['bb_valor']} & {sit} & {fmt_t(d['bb_tempo'])} & {nos} & {nps} & {ub} & {d['melhoras']} & {rel} \\\\")
tb += ["\\hline\\end{tabular}"]
open("resultados_longos/tabela_bb.tex", "w").write("\n".join(tb) + "\n")
print("\nresultados_longos/tabela.md, tabela.tex e tabela_bb.tex gerados")

# ---------- Figuras ----------
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    sys.exit("matplotlib nao instalado: tabelas geradas, figuras puladas (pip install matplotlib)")
AZUL, CINZA = "#1f5fa8", "#9a9a9a"

com_otimo = [d for d in dados if d["razao"]]
if com_otimo:
    fig, ax = plt.subplots(figsize=(6.4, 2.9))
    x = list(range(len(com_otimo))); larg = 0.38
    ax.bar([i - larg / 2 for i in x], [d["razao"] for d in com_otimo], larg, color=CINZA, label="Heurística do Trabalho 1")
    xb = [i + larg / 2 for i, d in zip(x, com_otimo) if d["razao_bb"] is not None]
    ax.bar(xb, [d["razao_bb"] for d in com_otimo if d["razao_bb"] is not None], larg, color=AZUL, label="Branch-and-Bound (15 min)")
    for i, d in zip(x, com_otimo):
        ax.text(i - larg / 2, d["razao"] + 0.01, f"{d['razao']:.2f}".replace(".", ","), ha="center", fontsize=6)
        if d["razao_bb"] is not None:
            ax.text(i + larg / 2, d["razao_bb"] + 0.01, f"{d['razao_bb']:.2f}".replace(".", ","), ha="center", fontsize=6)
    ax.set_xticks(x); ax.set_xticklabels([d["rotulo"] for d in com_otimo], rotation=25, ha="right", fontsize=7)
    ax.set_ylim(0, 1.12); ax.set_ylabel("valor / ótimo"); ax.grid(axis="y", alpha=0.25); ax.legend(fontsize=7, loc="lower right")
    plt.tight_layout(); plt.savefig("resultados_longos/fig_razao.pdf"); plt.savefig("resultados_longos/fig_razao.png", dpi=150); plt.close()

dp_ok = [d for d in dados if d["dp_ok"]]
if dp_ok:
    fig, ax = plt.subplots(figsize=(6.4, 3.0))
    x = list(range(len(dp_ok))); larg = 0.38
    ax.bar([i + larg / 2 for i in x], [max(d["dp_tempo"], 1e-3) for d in dp_ok], larg, color=CINZA, label="DP sobre a fronteira")
    xb = [i - larg / 2 for i, d in zip(x, dp_ok) if d["bb_tempo"] is not None]
    cores = [AZUL for d in dp_ok if d["bb_tempo"] is not None]
    barras = ax.bar(xb, [d["bb_tempo"] for d in dp_ok if d["bb_tempo"] is not None], larg, color=cores, label="Branch-and-Bound")
    for b, d in zip(barras, [d for d in dp_ok if d["bb_tempo"] is not None]):
        if not d["bb_otimo"]:
            b.set_hatch("///"); b.set_edgecolor("white")
    ax.set_yscale("log"); ax.set_ylabel("tempo (s)")
    ax.set_xticks(x); ax.set_xticklabels([d["rotulo"] for d in dp_ok], rotation=25, ha="right", fontsize=7)
    ax.set_ylim(top=4000); ax.grid(axis="y", alpha=0.25); ax.legend(fontsize=7, loc="upper right")
    plt.tight_layout(); plt.savefig("resultados_longos/fig_tempos.pdf"); plt.savefig("resultados_longos/fig_tempos.png", dpi=150); plt.close()
print("figuras geradas: resultados_longos/fig_razao.* e fig_tempos.*")
