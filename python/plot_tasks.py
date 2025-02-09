import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob
import os
import argparse

def plot_task_distribution(search_type):
    # Caminho para os arquivos CSV
    csv_files = glob.glob(f"data/{search_type}/tasks_{search_type}_*.csv")

    for csv_file in csv_files:
        # Carregar os dados do CSV
        df = pd.read_csv(csv_file)

        # Verifica se o DataFrame está vazio
        if df.empty:
            print(f"Aviso: {csv_file} está vazio. Pulando...")
            continue

        # Agrupar tarefas por máquina
        grouped = df.groupby("Machine")
        task_counts = grouped["TaskProcessingTime"].count()
        makespans = grouped["TaskProcessingTime"].sum()

        # Número de máquinas
        num_machines = len(grouped)

        # Ajusta dinamicamente o tamanho do gráfico
        fig_width = max(12, num_machines * 0.5)  # Ajusta largura para legibilidade
        fig, ax = plt.subplots(figsize=(fig_width, 7), dpi=150)

        # Gerar uma paleta de cores suaves
        np.random.seed(42)
        colors = plt.get_cmap("Set2")

        # Variável para rastrear a posição acumulada dentro da barra
        bottoms = {m: 0 for m in grouped.groups.keys()}
        bar_width = 0.6

        # Criar gráfico de barras empilhadas
        for index, row in df.iterrows():
            machine = row["Machine"]
            task_time = row["TaskProcessingTime"]

            rects = ax.bar(machine, task_time, 
                           color=colors(index % 8),
                           edgecolor="black",
                           bottom=bottoms[machine],
                           width=bar_width)
            
            # Adicionando texto dentro das barras
            for rect in rects:
                height = rect.get_height()
                if height > 10:  # Evita sobreposição em tarefas pequenas
                    ax.text(rect.get_x() + rect.get_width() / 2, 
                            rect.get_y() + height / 2, 
                            f"{int(height)}", 
                            ha='center', va='center', fontsize=9, color='black', fontweight='bold')
            
            bottoms[machine] += task_time

        # Adicionar makespan no topo das barras
        for machine, total_time in bottoms.items():
            ax.text(machine, total_time + 5, f"Makespan: {int(total_time)}", 
                    ha='center', fontsize=11, fontweight='bold', 
                    bbox=dict(facecolor='white', edgecolor='black', boxstyle='round,pad=0.3'))

        # Criar rótulos no eixo X com número de tarefas
        xticks_labels = [f"{m}\n({task_counts[m]} tarefas)" for m in grouped.groups.keys()]
        ax.set_xticks(list(grouped.groups.keys()))
        ax.set_xticklabels(xticks_labels, fontsize=10, rotation=45, ha='right')

        # Configuração do gráfico
        ax.set_xlabel(f"Máquinas ({search_type})", fontsize=13, fontweight="bold")
        ax.set_ylabel("Tempo de Processamento", fontsize=13, fontweight="bold")
        ax.set_title("Distribuição das Tarefas por Máquina", fontsize=15, fontweight='bold')
        ax.set_ylim([0, max(bottoms.values()) * 1.15])
        
        # Ajuste do grid
        plt.grid(axis="y", linestyle="--", alpha=0.6)

        # Ajuste automático do layout para evitar sobreposição
        plt.tight_layout()
        
        # Salvar figura
        output_file = os.path.splitext(os.path.basename(csv_file))[0] + ".png"
        plt.savefig(f"data/{search_type}/plot/{output_file}", dpi=150)
        plt.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot task distribution for a given search type.")
    parser.add_argument("search_type", type=str, help="The search type to filter the CSV files.")
    args = parser.parse_args()

    plot_task_distribution(args.search_type)