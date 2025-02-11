import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import glob

# Encontrar todos os arquivos CSV que começam com "resultados_"
csv_files = glob.glob("resultados_*.csv")

# Carregar e concatenar os dados de todos os arquivos CSV
df_list = []
for file in csv_files:
    df_temp = pd.read_csv(file)
    heuristica = file.split('_')[1].split('.')[0]  # Extrair heurística do nome do arquivo
    df_temp['Heuristica'] = heuristica
    df_list.append(df_temp)

df = pd.concat(df_list, ignore_index=True)

# Definir um estilo visual elegante
sns.set_theme(style="whitegrid")

# Obter os valores únicos de N e heurísticas
unique_N = sorted(df["N"].unique())
unique_heuristics = df["Heuristica"].unique()

# Criar uma paleta de cores para diferenciar os gráficos
palette = sns.color_palette("husl", len(unique_N))

# Criar gráficos para cada valor de N
for i, N in enumerate(unique_N):
    subset = df[df["N"] == N]
    
    plt.figure(figsize=(9, 5), dpi=150)
    sns.lineplot(
        data=subset,
        x="Tempo",
        y="Valor",
        hue="Heuristica",
        marker="o",
        linewidth=3,  # Linha mais espessa para destaque
        markersize=9,  # Marcadores maiores
        palette="husl",  # Paleta de cores para heurísticas
    )
    
    # Melhorias no design
    plt.title(f"Valor por Tempo (N={N})", fontsize=16, fontweight="bold", color="darkblue")
    plt.xlabel("Tempo", fontsize=13, fontweight="bold", color="darkred")
    plt.ylabel("Valor", fontsize=13, fontweight="bold", color="darkred")
    plt.xticks(rotation=45, fontsize=11, fontweight="bold")
    plt.yticks(fontsize=11, fontweight="bold")
    
    # Adicionar grid leve para melhor visualização
    plt.grid(True, linestyle="--", alpha=0.6)

    # Ajustar margens para evitar cortes nos rótulos
    plt.tight_layout()

    # Salvar imagem com um nome descritivo
    plt.savefig(f"data/grafico_N{N}.png", dpi=150, bbox_inches="tight")
    
    # Mostrar o gráfico
    plt.show()
