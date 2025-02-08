import pandas as pd
import itertools

def calculate_makespan(machine_tasks):
    """Calcula o makespan, ou seja, o tempo da máquina mais carregada."""
    return max(sum(tasks) for tasks in machine_tasks.values())

# Carregar os dados
file_path = "tasks_searchLocalBestImprovement_m10_r1.500000_exec1.csv"  # Substitua pelo caminho correto
df = pd.read_csv(file_path)

# Organizar os dados por máquina
machine_tasks = {}
for _, row in df.iterrows():
    machine = row['Machine']
    task_time = row['TaskProcessingTime']
    if machine not in machine_tasks:
        machine_tasks[machine] = []
    machine_tasks[machine].append(task_time)

# Calcula o makespan da solução fornecida
best_improvement_makespan = calculate_makespan(machine_tasks)
print(f"Makespan da solução do Local Best Improvement: {best_improvement_makespan}")

# Testar balanceamento uniforme
tasks = [row['TaskProcessingTime'] for _, row in df.iterrows()]
machines = list(machine_tasks.keys())
avg_tasks_per_machine = len(tasks) // len(machines)

evenly_distributed = {m: tasks[i::len(machines)] for i, m in enumerate(machines)}
uniform_makespan = calculate_makespan(evenly_distributed)
print(f"Makespan do balanceamento uniforme: {uniform_makespan}")

# Testar heurística gulosa (menor carga primeiro)
greedy_machines = {m: [] for m in machines}
tasks.sort(reverse=True)  # Ordenar as tarefas da maior para a menor
for task in tasks:
    min_machine = min(greedy_machines, key=lambda m: sum(greedy_machines[m]))
    greedy_machines[min_machine].append(task)

greedy_makespan = calculate_makespan(greedy_machines)
print(f"Makespan da heurística gulosa: {greedy_makespan}")

# Comparação final
best_makespan = min(best_improvement_makespan, uniform_makespan, greedy_makespan)
if best_improvement_makespan == best_makespan:
    print("✅ A solução encontrada pelo Local Best Improvement foi ótima!")
else:
    print("❌ Existe uma solução melhor que o Local Best Improvement!")