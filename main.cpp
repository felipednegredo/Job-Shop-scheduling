#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>

using std::vector;

struct Task {
    int id;
    int processingTime;
};

struct Machine {
    vector<Task> tasks;
    int totalProcessingTime = 0;
};

struct WorkStation {
    vector<Machine> machines;
};

template<typename T>
void print(T first) {
    std::cout << first << '\n';
}

template<typename T, typename ... Args>
void print(T first, Args ... args) {
    std::cout << first << ' ';
    print(args...);
}

Task generateTask(int i) {
    return {i, rand() % 100 + 1}; // Tempo aleatório entre 1 e 100
}

vector<Machine> initialize(int m, double r) {
    int numTasks = static_cast<int>(pow(m, r)); // Calcula n = m^r
    vector<Machine> machines(m);

    for (int i = 0; i < numTasks; ++i) {
        Task task = generateTask(i);
        machines[0].tasks.push_back(task);
        machines[0].totalProcessingTime += task.processingTime;
    }

    return machines;
}

void runSimulations(const vector<int>& m_values, const vector<double>& r_values, int numExecutions) {
    std::ofstream outputFile("resultados.csv");
    outputFile << "Heuristica,N,M,Replicação,Tempo,Iterações,Valor,Parametro\n";

    for (int m : m_values) {
        for (double r : r_values) {
            for (int exec = 1; exec <= numExecutions; ++exec) {
                vector<Machine> machines = initialize(m, r);
                
                // Calcula o makespan (tempo da máquina mais carregada)
                int makespan = 0;
                for (const auto& machine : machines) {
                    if (machine.totalProcessingTime > makespan) {
                        makespan = machine.totalProcessingTime;
                    }
                }

                // Grava no arquivo
                outputFile << "Heuristica" << "," << static_cast<int>(pow(m, r)) << "," << m << "," << exec << "," << makespan << "," << numExecutions << "," << "Valor" << "," << r << "\n";

                // Exibe no console
                print("Execução:", exec, "| m:", m, "| r:", r, "| Makespan:", makespan, "num_tasks:", machines[0].tasks.size());
            }
        }
    }

    outputFile.close();
}

void printMachine(const Machine& machine) {
    print("====================================");
    print("Máquina:");
    print("Tarefas:");
    for (const auto& task : machine.tasks) {
        print("Tarefa", task.id, "| Tempo:", task.processingTime);
    }
    print("====================================");
    print("Total de tarefas:", machine.tasks.size(), "| Makespan:", machine.totalProcessingTime);

}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Inicializa aleatoriedade

    vector<int> m_values = {10, 20, 50};
    vector<double> r_values = {1.5, 2.0};
    int numExecutions = 1;

    print("Iniciando simulações...");

    //Machine machine = initialize(m_values[0], r_values[0])[0];

    //printMachine(machine);

    runSimulations(m_values, r_values, numExecutions);

    return 0;
}
