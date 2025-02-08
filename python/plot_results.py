import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Carregar os dados
df = pd.read_csv("resultados.csv")  # Substitua pelo nome correto do arquivo

# Definir um estilo visual elegante
sns.set_theme(style="whitegrid")

# Obter os valores únicos de N
unique_N = sorted(df["N"].unique())  # Ordena para manter consistência

# Criar uma paleta de cores para diferenciar os gráficos
palette = sns.color_palette("husl", len(unique_N))

# Criar gráficos para cada valor de N
for i, N in enumerate(unique_N):
    subset = df[df["N"] == N]
    
    plt.figure(figsize=(9, 5), dpi=150)
    sns.lineplot(
        data=subset,
        x="Replicação",
        y="Iterações",
        marker="o",
        linewidth=3,  # Linha mais espessa para destaque
        markersize=9,  # Marcadores maiores
        color=palette[i],  # Cor diferente para cada N
    )
    
    # Melhorias no design
    plt.title(f"Iterações por Replicação (N={N})", fontsize=16, fontweight="bold", color="darkblue")
    plt.xlabel("Replicação", fontsize=13, fontweight="bold", color="darkred")
    plt.ylabel("Iterações", fontsize=13, fontweight="bold", color="darkred")
    plt.xticks(subset["Replicação"], rotation=45, fontsize=11, fontweight="bold")
    plt.yticks(fontsize=11, fontweight="bold")
    
    # Adicionar grid leve para melhor visualização
    plt.grid(True, linestyle="--", alpha=0.6)

    # Ajustar margens para evitar cortes nos rótulos
    plt.tight_layout()

    # Salvar imagem com um nome descritivo
    plt.savefig(f"data/grafico_N{N}.png", dpi=150, bbox_inches="tight")
    
    # Mostrar o gráfico
    plt.show()
