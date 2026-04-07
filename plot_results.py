"""
plot_results.py — Vykresli grafy z benchmark vysledkov

Pouzitie:
    python3 plot_results.py

Predpoklada: results/summary.csv (vystup z benchmark)
Vystup:      graphs/*.png
"""

import csv
import os
import math
from collections import defaultdict

# --- skus nacitat matplotlib ---
try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker
    HAS_MPL = True
except ImportError:
    HAS_MPL = False
    print("matplotlib nie je k dispozicii, generujem len textove tabulky.")

# ============================================================
# Nacitanie dat
# ============================================================

DATA_FILE = "results/summary.csv"

def load_csv(path):
    rows = []
    with open(path, newline='') as f:
        for row in csv.DictReader(f):
            rows.append({
                'scenario':  row['scenario'],
                'structure': row['structure'],
                'n':         int(row['n']),
                'mean_ns':   float(row['time_per_op_ns']),
                'stddev_ns': float(row['stddev_ns']),
            })
    return rows

# ============================================================
# Pomocne funkcie
# ============================================================

DS_LABELS = {
    'hash_chain':  'Hash (chaining)',
    'hash_double': 'Hash (double)',
    'rbt':         'Red-Black Tree',
    't23':         '2-3 Tree',
}

DS_COLORS = {
    'hash_chain':  '#378ADD',
    'hash_double': '#1D9E75',
    'rbt':         '#D85A30',
    't23':         '#BA7517',
}

DS_MARKERS = {
    'hash_chain':  'o',
    'hash_double': 's',
    'rbt':         '^',
    't23':         'D',
}

SCENARIO_TITLES = {
    'S1_seq_insert':   'S1 — Sequential insert',
    'S2_rand_insert':  'S2 — Random insert',
    'S3_search_hit':   'S3 — Search hit (100 % existing)',
    'S4_search_miss':  'S4 — Search miss (0 % existing)',
    'S5_delete_all':   'S5 — Delete all',
    'S6_mixed':        'S6 — Mixed workload (70/20/10)',
    'S7_hash_worst':   'S7 — Hash worst-case (all collisions)',
    'S8_sorted_insert':'S8 — Sorted insert',
}

def group_by(rows, key):
    d = defaultdict(list)
    for r in rows:
        d[r[key]].append(r)
    return d

# ============================================================
# Textova tabulka (vzdy)
# ============================================================

def print_table(rows):
    scenarios = group_by(rows, 'scenario')
    for sc, sc_rows in sorted(scenarios.items()):
        title = SCENARIO_TITLES.get(sc, sc)
        print(f"\n{'='*60}")
        print(f"  {title}")
        print(f"{'='*60}")
        print(f"  {'Struktura':<18} {'n':>9} {'ns/op':>12} {'stddev':>10}")
        print(f"  {'-'*53}")
        by_ds = group_by(sc_rows, 'structure')
        for ds in ['hash_chain', 'hash_double', 'rbt', 't23']:
            if ds not in by_ds:
                continue
            for r in sorted(by_ds[ds], key=lambda x: x['n']):
                print(f"  {DS_LABELS[ds]:<18} {r['n']:>9,} {r['mean_ns']:>12.3f} {r['stddev_ns']:>10.3f}")
        print()

# ============================================================
# Grafy
# ============================================================

def make_scenario_plot(sc, sc_rows, out_dir):
    """Pre kazdy scenar: cas/op vs n (log-log)."""
    fig, ax = plt.subplots(figsize=(8, 5))

    by_ds = group_by(sc_rows, 'structure')
    for ds in ['hash_chain', 'hash_double', 'rbt', 't23']:
        if ds not in by_ds:
            continue
        pts = sorted(by_ds[ds], key=lambda x: x['n'])
        ns  = [p['n']       for p in pts]
        mn  = [p['mean_ns'] for p in pts]
        sd  = [p['stddev_ns'] for p in pts]
        ax.errorbar(ns, mn, yerr=sd,
                    label=DS_LABELS[ds],
                    color=DS_COLORS[ds],
                    marker=DS_MARKERS[ds],
                    linewidth=2, markersize=7,
                    capsize=4)

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('Pocet prvkov n', fontsize=12)
    ax.set_ylabel('Cas na operaciu [ns]', fontsize=12)
    title = SCENARIO_TITLES.get(sc, sc)
    ax.set_title(title, fontsize=13, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, which='both', linestyle='--', alpha=0.4)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(
        lambda x, _: f'{int(x):,}'))

    path = os.path.join(out_dir, sc + '.png')
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Ulozeny: {path}")


def make_comparison_plot(rows, operation, out_dir):
    """Porovnanie DS pre zvolenu operaciu pri n=1_000_000."""
    TARGET_N = 1_000_000
    target_rows = [r for r in rows
                   if r['n'] == TARGET_N and operation.lower() in r['scenario'].lower()]
    if not target_rows:
        return

    ds_list = ['hash_chain', 'hash_double', 'rbt', 't23']
    by_ds = group_by(target_rows, 'structure')

    # Priemerne hodnoty cez scenare pre kazdu DS
    means  = []
    labels = []
    colors = []
    for ds in ds_list:
        if ds not in by_ds:
            continue
        vals = [r['mean_ns'] for r in by_ds[ds]]
        means.append(sum(vals) / len(vals))
        labels.append(DS_LABELS[ds])
        colors.append(DS_COLORS[ds])

    fig, ax = plt.subplots(figsize=(7, 4))
    bars = ax.bar(labels, means, color=colors, width=0.5, edgecolor='white', linewidth=1.2)
    for bar, val in zip(bars, means):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + max(means) * 0.01,
                f'{val:.1f} ns', ha='center', va='bottom', fontsize=9)
    ax.set_ylabel('Priemerny cas [ns/op]', fontsize=11)
    ax.set_title(f'Porovnanie pri n=1 000 000', fontsize=12, fontweight='bold')
    ax.grid(axis='y', linestyle='--', alpha=0.4)
    fig.tight_layout()
    path = os.path.join(out_dir, f'compare_{operation}_n1M.png')
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Ulozeny: {path}")


def make_all_ops_bar(rows, out_dir):
    """Sumarny bar chart: priemerne ns/op cez VSETKY scenare, pre n=100_000."""
    TARGET_N = 100_000
    sub = [r for r in rows if r['n'] == TARGET_N]
    if not sub:
        return

    ds_list = ['hash_chain', 'hash_double', 'rbt', 't23']
    scenarios = sorted(set(r['scenario'] for r in sub))
    by_sc_ds = defaultdict(dict)
    for r in sub:
        by_sc_ds[r['scenario']][r['structure']] = r['mean_ns']

    x = range(len(scenarios))
    width = 0.2
    fig, ax = plt.subplots(figsize=(13, 5))
    for i, ds in enumerate(ds_list):
        vals = [by_sc_ds[sc].get(ds, 0) for sc in scenarios]
        offset = (i - 1.5) * width
        bars = ax.bar([xi + offset for xi in x], vals,
                      width=width, label=DS_LABELS[ds],
                      color=DS_COLORS[ds], edgecolor='white')

    ax.set_yscale('log')
    ax.set_xticks(list(x))
    ax.set_xticklabels([SCENARIO_TITLES.get(sc, sc).split(' — ')[-1]
                        for sc in scenarios], rotation=25, ha='right', fontsize=8)
    ax.set_ylabel('Cas na operaciu [ns] — log skala', fontsize=10)
    ax.set_title(f'Vsetky scenare pri n = {TARGET_N:,}', fontsize=12, fontweight='bold')
    ax.legend(fontsize=9)
    ax.grid(axis='y', linestyle='--', alpha=0.4)
    fig.tight_layout()
    path = os.path.join(out_dir, 'all_scenarios_n100k.png')
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Ulozeny: {path}")


# ============================================================
# Main
# ============================================================

def main():
    if not os.path.exists(DATA_FILE):
        print(f"Subor {DATA_FILE} neexistuje. Najprv spusti ./benchmark")
        return

    rows = load_csv(DATA_FILE)
    print(f"Nacitanych {len(rows)} zaznamov.")

    print_table(rows)

    if not HAS_MPL:
        print("\nNainštaluj matplotlib: pip install matplotlib")
        return

    out_dir = 'graphs'
    os.makedirs(out_dir, exist_ok=True)
    print(f"\nGenerujem grafy do {out_dir}/")

    # Graf pre kazdy scenar
    for sc, sc_rows in group_by(rows, 'scenario').items():
        make_scenario_plot(sc, sc_rows, out_dir)

    # Sumarny bar chart pre n=100k
    make_all_ops_bar(rows, out_dir)

    print(f"\nGrafy ulozene do {out_dir}/")
    print("Import do dokumentacie: pouzij PNG subory z adresara graphs/")

if __name__ == '__main__':
    main()
