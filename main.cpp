#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <climits>

using std::vector;
using std::string;

// Struct for Task
struct Task {
    // Task id
    int id;
    // Task processing time
    int processingTime;
};

// Struct for Machine
struct Machine {
    // Vector of tasks
    vector<Task> tasks;
    // Total processing time for the machine
    int makespan = 0;
};

// Struct for WorkStation
struct WorkStation {
    // Vector of machines
    vector<Machine> machines;
    // Number of steps
    int steps = 0;
};

// Function to print a single value
template<typename T>
void print(T first) {
    std::cout << first << '\n';
}

// Function to print multiple values
template<typename T, typename ... Args>
void print(T first, Args ... args) {
    std::cout << first << ' ';
    print(args...);
}

// Function to generate a task
Task generateTask(int i) {
    // Return a task with id i and random processing time between 1 and 100
    return {i, rand() % 100 + 1}; // Tempo aleatório entre 1 e 100
}

// Function to export the task allocation to a CSV file
void exportTaskAllocation(const WorkStation& workStation, const string& filename = "tasks.csv") {
    // Open the file
    std::ofstream file(filename);
    // Write the header
    file << "Machine,TaskProcessingTime\n";

    // For each machine
    int machineIndex = 1;
    for (const auto& machine : workStation.machines) {
        // For each task in the machine
        for (const auto& task : machine.tasks) {
            // Write the machine index and the task processing time to the file
            file << machineIndex << "," << task.processingTime << "\n";
        }
        // Increment the machine index
        machineIndex++;
    }

    // Close the file
    file.close();
    // Print the message
    print("Task allocation saved to", filename);
}


// Function to initialize the machines
vector<Machine> initialize(int m, double r) {
    // Calculate n = m^r
    int numTasks = static_cast<int>(pow(m, r));
    // Create m machines
    vector<Machine> machines(m);

    // Generate n tasks and add them to the first machine
    for (int i = 0; i < numTasks; ++i) {
        // Generate a task
        Task task = generateTask(i);
        // Add the task to the first machine
        machines[0].tasks.push_back(task);
        // Update the total processing time of the first machine
        machines[0].makespan += task.processingTime;
    }

    // exportTaskAllocation({machines}, "tasks-inicial.csv");

    // Return the machines
    return machines;
}

// Function to search for the first improvement
void searchLocalFirstImprovement(WorkStation& workStation) {
    // Flag to indicate if there was an improvement
    bool improvement = true;

    // While there is an improvement
    while (improvement) {
        // Set the improvement flag to false
        improvement = false;

        // For each machine
        for (auto& machine : workStation.machines) {
            // For each task in the machine
            for (size_t i = 0; i < machine.tasks.size(); ++i) {
                // Get the task
                Task task = machine.tasks[i];

                // For each other machine
                for (auto& otherMachine : workStation.machines) {
                    // If the machine is different from the other machine
                    if (&machine != &otherMachine) {
                        // Calculate the new total processing time
                        int newmakespan = otherMachine.makespan + task.processingTime;

                        // If the new total processing time is less than the total processing time of the machine
                        if (newmakespan < machine.makespan) {
                            // Remove the task from the machine
                            machine.tasks.erase(machine.tasks.begin() + i);
                            // Update the total processing time of the machine
                            machine.makespan -= task.processingTime;

                            // Add the task to the other machine
                            otherMachine.tasks.push_back(task);
                            // Update the total processing time of the other machine
                            otherMachine.makespan += task.processingTime;

                            workStation.steps++;

                            // Set the improvement flag to true
                            improvement = true;
                            // Break the loop
                            break;
                        }
                    }
                }

                // Break the loop
                if (improvement) {
                    break;
                }
            }


            if (improvement) {
                break;
            }
        }
    }
}

// Função para calcular o desvio padrão dos makespans das máquinas
double computeMakespanStdDev(const std::vector<Machine>& machines) {
    double sum = 0, sumSq = 0;
    int n = machines.size();

    for (const auto& machine : machines) {
        sum += machine.makespan;
        sumSq += machine.makespan * machine.makespan;
    }

    double mean = sum / n;
    return std::sqrt((sumSq / n) - (mean * mean));  // Desvio padrão
}

// Função de busca local com melhor balanceamento
void searchLocalBestImprovement(WorkStation& workStation) {
    bool improvement = true;

    while (improvement) {
        improvement = false;

        // Estado inicial: calcular o desvio padrão antes da movimentação
        double currentStdDev = computeMakespanStdDev(workStation.machines);

        Task* bestTask = nullptr;
        Machine* sourceMachine = nullptr;
        Machine* targetMachine = nullptr;
        double bestStdDevReduction = 0.0;

        // Iterar sobre cada máquina
        for (auto& machine : workStation.machines) {
            if (machine.tasks.empty()) continue;  // Ignorar máquinas vazias

            // Iterar sobre cada tarefa da máquina
            for (size_t i = 0; i < machine.tasks.size(); ++i) {
                Task& task = machine.tasks[i];

                // Testar realocar a tarefa para outra máquina
                for (auto& otherMachine : workStation.machines) {
                    if (&machine == &otherMachine) continue;  // Não mover para a mesma máquina

                    // Simular a troca
                    machine.makespan -= task.processingTime;
                    otherMachine.makespan += task.processingTime;

                    // Calcular novo desvio padrão
                    double newStdDev = computeMakespanStdDev(workStation.machines);

                    // Se houve melhora significativa, armazenar a melhor escolha
                    double stdDevReduction = currentStdDev - newStdDev;
                    if (stdDevReduction > bestStdDevReduction) {
                        bestStdDevReduction = stdDevReduction;
                        bestTask = &task;
                        sourceMachine = &machine;
                        targetMachine = &otherMachine;
                    }

                    // Reverter a simulação
                    machine.makespan += task.processingTime;
                    otherMachine.makespan -= task.processingTime;
                }
            }
        }

        // Se encontramos a melhor tarefa para realocar, aplicamos a troca
        if (bestTask && sourceMachine && targetMachine) {
            auto it = std::find_if(sourceMachine->tasks.begin(), sourceMachine->tasks.end(),
                                   [&](const Task& t) { return t.id == bestTask->id; });

            if (it != sourceMachine->tasks.end()) {
                sourceMachine->tasks.erase(it);
                sourceMachine->makespan -= bestTask->processingTime;
            }

            targetMachine->tasks.push_back(*bestTask);
            targetMachine->makespan += bestTask->processingTime;

            workStation.steps++;
            improvement = true;
        }
    }
}


void searchLocalIterative(WorkStation& workStation, double per, int ILSmax=1000) {
    // Initial solution
    WorkStation s0 = workStation;
    // Current solution
    WorkStation s = s0;
    int iter = 0, melhorIter = 0;

    // Function to evaluate the makespan of a workstation
    auto f = [](const WorkStation& ws) {
        // Find the maximum makespan
        int maxMakespan = 0;
        // For each machine
        for (const auto& machine : ws.machines) {
            // If the makespan of the machine is greater than the maximum makespan
            if (machine.makespan > maxMakespan) {
                // Update the maximum makespan
                maxMakespan = machine.makespan;
            }
        }
        // Return the maximum makespan
        return maxMakespan;
    };

    // Local search function
    auto BuscaLocal = [](WorkStation& ws) {
        searchLocalBestImprovement(ws);
    };

    // Perturbation function
    auto perturbacao = [per](WorkStation& ws) {
        int numTasks = 0;
        for (const auto& machine : ws.machines) {
            numTasks += machine.tasks.size();
        }
        // Calculate the number of perturbations
        int numPerturbations = static_cast<int>(numTasks * per);

        // For each perturbation
        for (int i = 0; i < numPerturbations; ++i) {
            // Select a random machine
            int machineIndex = rand() % ws.machines.size();
            // If the machine is not empty
            if (!ws.machines[machineIndex].tasks.empty()) {
                // Select a random task
                int taskIndex = rand() % ws.machines[machineIndex].tasks.size();
                // Get the task
                Task task = ws.machines[machineIndex].tasks[taskIndex];
                // Remove the task from the machine
                ws.machines[machineIndex].tasks.erase(ws.machines[machineIndex].tasks.begin() + taskIndex);
                // Update the makespan of the machine
                ws.machines[machineIndex].makespan -= task.processingTime;
                // Select a random machine
                int newMachineIndex = rand() % ws.machines.size();
                // Add the task to the new machine
                ws.machines[newMachineIndex].tasks.push_back(task);
                // Update the makespan of the new machine
                ws.machines[newMachineIndex].makespan += task.processingTime;

                // Update the number of steps
                ws.steps++;
            }
        }
    };

    // Main iterative local search loop
    while (iter - melhorIter < ILSmax) {
        iter++;
        // Copy the current solution
        WorkStation s_prime = s;
        // Perturb the solution
        perturbacao(s_prime);
        // Local search
        BuscaLocal(s_prime);

        // If the new solution is better
        if (f(s_prime) < f(s)) {
            // Update the current solution
            s = s_prime;
            // Update the best iteration
            melhorIter = iter;
        }
    }

    // Update the original workstation
    workStation = s;
}



// Function to run the simulations
void runSimulations(const vector<int>& m_values, const vector<double>& r_values, int numExecutions, WorkStation& workStation, string searchMethod, double per) {
    // Delete the output file if it exists
    std::remove("resultados.csv");

    // Open the output file
    std::ofstream outputFile("resultados.csv");
    // Write the header
    outputFile << "Heuristica,N,M,Replicação,Tempo,Iterações,Valor,Parametro\n";

    // For each m value
    for (int m : m_values) {
        // For each r value
        for (double r : r_values) {
            // For each execution
            print("Executions for m =", m, "and r =", r);
            for (int exec = 1; exec <= numExecutions; ++exec) {
                // Initialize the machines
                workStation.machines = initialize(m, r);

                if (exec == 1) {
                    print("--------------------------------------");
                    print("Quantidade de máquinas:", workStation.machines.size(), "| Quantidade de tarefas:", static_cast<int>(pow(m, r)), "| Replicação:", r, "| Execução:", exec);
                    print("--------------------------------------");
                }
                
                // Start timing
                auto start = std::chrono::high_resolution_clock::now();

                // Search for the best improvement
                if (searchMethod == "searchLocalBestImprovement") {
                    print("Search Local Best Improvement");
                    // Search for the best improvement
                    searchLocalBestImprovement(workStation);
                } else if (searchMethod == "searchLocalFirstImprovement") {
                    print("Search Local First Improvement");
                    // Search for the first improvement
                    searchLocalFirstImprovement(workStation);
                } else if (searchMethod == "local iterative") {
                    print("Local Iterative");
                }

                // Stop timing
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - start;

                print("Finished in", elapsed.count(), "s");

                string parametre = "";

                if (per == 0) {
                    parametre = "N/A";
                }
                else {
                    parametre = std::to_string(per);
                }
                
                // Write the results to the output file
                outputFile << searchMethod << "," << static_cast<int>(pow(m, r)) << "," << m << "," << exec << "," << elapsed.count() << "," << workStation.steps << "," << "teste" << "," << parametre << "\n";
                
                if (m == 10 && r == 1.5 && exec == 1) {
                    exportTaskAllocation(workStation, "tasks_" + searchMethod + "_m" + std::to_string(m) + "_r" + std::to_string(r) + "_exec" + std::to_string(exec) + ".csv");
                }

                print("Method:", searchMethod, "| Execution:", exec, "| m:", m, "| r:", r, "|",workStation.machines[0].makespan, "| Number of tasks:", static_cast<int>(pow(m, r)), "| Time:", elapsed.count(), "s");
            }
        }
    }

    outputFile.close();
}

void printMachine(const Machine& machine, int i = 1) {
    print("====================================");
    print("Máquina", i);
    print("Tarefas:");
    for (const auto& task : machine.tasks) {
        print("Tarefa", task.id, "| Tempo:", task.processingTime);
    }
    print("====================================");
    print("Total de tarefas:", machine.tasks.size(), "| Makespan:", machine.makespan);

}

void printWorkStation(const WorkStation& workStation) {
    int i = 1;
    for (const Machine& machine : workStation.machines) {
        printMachine(machine,i);
        ++i;
    }
}


int main() {
    srand(static_cast<unsigned int>(time(0))); // Inicializa aleatoriedade

    vector<int> m_values = {10, 20, 50};
    vector<double> r_values = {1.5, 2.0};
    int numExecutions = 1;
    WorkStation workStation;
    string searchMethod = "searchLocalBestImprovement";
    //string searchMethod = "searchLocalFirstImprovement";

    print("Starting simulations...");

    runSimulations(m_values, r_values, numExecutions, workStation, searchMethod ,0);

    print("Simulations finished!");

    //print("First machine Debug:");

    //printWorkStation(workStation);

    return 0;
}
