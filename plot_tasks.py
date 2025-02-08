import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Carregar os dados do CSV
df = pd.read_csv("tasks_searchLocalFirstImprovement_m10_r1.500000_exec1.csv")

# Agrupar tarefas por máquina
grouped = df.groupby("Machine")["TaskProcessingTime"].apply(list)
task_counts = df.groupby("Machine")["TaskProcessingTime"].count()

# Criar o gráfico de barras empilhadas
fig, ax = plt.subplots(figsize=(12, 7), dpi=150)

# Gerar uma paleta de cores suaves
np.random.seed(42)
colors = plt.colormaps.get_cmap("Set2")

# Variável para rastrear a posição acumulada dentro da barra
bottoms = {m: 0 for m in grouped.index}

bar_width = 0.6  # Ajustando a largura para espaçamento entre máquinas

for index, row in df.iterrows():
    machine = row["Machine"]
    task_time = row["TaskProcessingTime"]

    # Adicionando a barra da tarefa dentro da máquina
    rects = ax.bar(machine, task_time, 
                   color=colors(index % 8), 
                   edgecolor="black",  # Adiciona borda preta para melhor separação
                   bottom=bottoms[machine], width=bar_width)

    # Adicionando texto dentro das barras
    for rect in rects:
        height = rect.get_height()
        if height > 10:  # Evita sobreposição em tarefas muito pequenas
            ax.text(rect.get_x() + rect.get_width() / 2, 
                    rect.get_y() + height / 2, 
                    f"{int(height)}", 
                    ha='center', va='center', fontsize=9, color='black', fontweight='bold')

    # Atualiza a posição acumulada
    bottoms[machine] += task_time

# Adicionar makespan no topo das barras
for machine, total_time in bottoms.items():
    ax.text(machine, total_time + 5, f"Makespan: {int(total_time)}", 
            ha='center', fontsize=11, fontweight='bold', 
            bbox=dict(facecolor='white', edgecolor='black', boxstyle='round,pad=0.3'))

# Criar rótulos no eixo X com o número de tarefas
xticks_labels = [f"{m}\n({task_counts[m]} tarefas)" for m in grouped.index]
ax.set_xticks(grouped.index)
ax.set_xticklabels(xticks_labels, fontsize=11)

# Configuração do gráfico
ax.set_xlabel("Máquinas (SearchLocalBestImprovement)", fontsize=13, fontweight="bold")
ax.set_ylabel("Tempo de Processamento", fontsize=13, fontweight="bold")
ax.set_title("Distribuição das Tarefas por Máquina", fontsize=15, fontweight='bold')

# Ajustar limites do eixo Y
ax.set_ylim([0, max(bottoms.values()) * 1.15])

# Ajuste do grid
plt.grid(axis="y", linestyle="--", alpha=0.6)

# Ajustar layout e salvar
plt.tight_layout()
plt.savefig("task_distribution.png", dpi=150)
plt.show()
