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
    long int makespan = 0;
};

// Struct for WorkStation
struct WorkStation {
    // Vector of machines
    vector<Machine> machines;
    // Number of steps
    long long int steps = 0;
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
void exportTaskAllocation(const WorkStation& workStation, const string& filename = "data/tasks.csv") {
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
    long int numTasks = static_cast<int>(pow(m, r));
    // Create m machines
    vector<Machine> machines(m);

    // Generate n tasks and add them to the first machine
    for (long int i = 0; i < numTasks; ++i) {
        // Generate a task
        Task task = generateTask(i);
        // Add the task to the first machine
        machines[0].tasks.push_back(task);
        // Update the total processing time of the first machine
        machines[0].makespan += task.processingTime;
    }
    for (std::vector<Machine>::size_type i = 1; i < machines.size(); ++i){
        machines[i].makespan = 0;
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
                        long int newmakespan = otherMachine.makespan + task.processingTime;

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

// Function to compute the standard deviation of the makespan
void searchLocalBestImprovement(WorkStation& workStation) {
    bool improvement = true;

    while (improvement) {
        improvement = false;

        // Encontra a máquina com o maior makespan
        auto maxMachineIt = std::max_element(
            workStation.machines.begin(), workStation.machines.end(),
            [](const Machine& a, const Machine& b) { return a.makespan < b.makespan; });

        Machine* maxMachine = &(*maxMachineIt);
        double maxMakespanBefore = maxMachine->makespan;

        Task bestTask;
        Machine* targetMachine = nullptr;
        double bestReduction = 0.0;

        // Percorre cada tarefa da máquina com maior makespan
        for (size_t i = 0; i < maxMachine->tasks.size(); ++i) {
            Task task = maxMachine->tasks[i];

            // Tenta realocar para outra máquina
            for (auto& otherMachine : workStation.machines) {
                if (&otherMachine == maxMachine) continue;

                double newMaxMakespan = std::max(maxMachine->makespan - task.processingTime,
                                                 otherMachine.makespan + task.processingTime);

                double makespanReduction = maxMakespanBefore - newMaxMakespan;

                if (makespanReduction > bestReduction) {
                    bestReduction = makespanReduction;
                    bestTask = task;
                    targetMachine = &otherMachine;
                }
            }
        }

        // Se encontrou uma realocação que melhora o makespan máximo, aplica a mudança
        if (targetMachine) {
            auto it = std::find_if(maxMachine->tasks.begin(), maxMachine->tasks.end(),
                                   [&](const Task& t) { return t.id == bestTask.id; });

            if (it != maxMachine->tasks.end()) {
                maxMachine->tasks.erase(it);
                maxMachine->makespan -= bestTask.processingTime;
            }

            targetMachine->tasks.push_back(bestTask);
            targetMachine->makespan += bestTask.processingTime;

            workStation.steps++;
            improvement = true;
        }
    }
}


void searchLocalIterative(WorkStation& workStation, double per, int ILSmax = 1000) {
    WorkStation s0 = workStation;  // Initial solution
    WorkStation s = s0;            // Best solution found so far
    int iter = 0, melhorIter = 0;  // Iteration counters

    // Function to calculate the maximum makespan among all machines
    auto calculateMaxMakespan = [](const WorkStation& ws) {
        int maxMakespan = 0;
        for (const auto& machine : ws.machines) {
            if (machine.makespan > maxMakespan) {
                maxMakespan = machine.makespan;
            }
        }
        return maxMakespan;
    };

    // Local search function: Applies the best improvement heuristic
    auto applyLocalSearch = [](WorkStation& ws) {
        searchLocalBestImprovement(ws);
    };

    // Perturbation function: Moves tasks between machines to escape local optima
    auto applyPerturbation = [per](WorkStation& ws) {
        int numTasks = 0;
        for (const auto& machine : ws.machines) {
            numTasks += machine.tasks.size();
        }

        int numPerturbations = std::max(1, static_cast<int>(numTasks * per));

        for (long long int i = 0; i < numPerturbations; ++i) {
            int machineIndex = rand() % ws.machines.size();
            if (!ws.machines[machineIndex].tasks.empty()) {
                int taskIndex = rand() % ws.machines[machineIndex].tasks.size();
                Task task = ws.machines[machineIndex].tasks[taskIndex];

                ws.machines[machineIndex].tasks.erase(ws.machines[machineIndex].tasks.begin() + taskIndex);
                ws.machines[machineIndex].makespan -= task.processingTime;

                int newMachineIndex;
                do {
                    newMachineIndex = rand() % ws.machines.size();
                } while (newMachineIndex == machineIndex);

                ws.machines[newMachineIndex].tasks.push_back(task);
                ws.machines[newMachineIndex].makespan += task.processingTime;
                ws.steps++;
            }
        }
    };

    int bestMakespan = calculateMaxMakespan(s);

    while (iter - melhorIter < ILSmax) {
        iter++;  // Increment iteration count

        WorkStation s_prime = s;
        applyPerturbation(s_prime);

        //int makespanBefore = calculateMaxMakespan(s_prime);
        applyLocalSearch(s_prime);
        int makespanAfter = calculateMaxMakespan(s_prime);

        if (makespanAfter < bestMakespan) {
            s = s_prime;
            bestMakespan = makespanAfter;
            melhorIter = iter;  // Reset iteration counter since an improvement was found
        }
    }

    workStation = s;  // Update the original workstation with the best solution found
}

// Function to run the simulations
void runSimulations(const vector<int>& m_values, const vector<double>& r_values, int numExecutions, string searchMethod, double per) {
    // Delete the output file if it exists
    std::string outputFilename = "python/resultados_" + searchMethod + ".csv";
    std::remove(outputFilename.c_str());

    // Open the output file
    std::ofstream outputFile(outputFilename);
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
                WorkStation workStation;
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
                } else if (searchMethod == "searchLocalIterative") {
                    print("Search Local Iterative");
                    // Search for the iterative improvement
                    searchLocalIterative(workStation, per);
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

                // Find the maximum makespan among all machines
                auto maxMakespanIt = std::max_element(workStation.machines.begin(), workStation.machines.end(),
                                                      [](const Machine& a, const Machine& b) { return a.makespan < b.makespan; });
                long int maxMakespan = maxMakespanIt->makespan;

                // Write the results to the output file
                outputFile << searchMethod << "," << static_cast<int>(pow(m, r)) << "," << m << "," << exec << "," << elapsed.count() << "," << workStation.steps << "," << maxMakespan << "," << parametre << "\n";
                
                if (exec == 1) {
                    // Export the task allocation to a CSV file for each execution
                    exportTaskAllocation(workStation, "python/data/"+ searchMethod +"/tasks_" + searchMethod + "_M" + std::to_string(m) + "_tasks" + std::to_string(static_cast<int>(pow(m, r))) + "_exec" + std::to_string(exec) + ".csv");

                }

                print("Method:", searchMethod, "Machines: ", workStation.machines.size() ,"| Execution:", exec, "| m:", m, "| r:", r, "| Max Makespan:", maxMakespan, "| Number of tasks:", static_cast<int>(pow(m, r)), "| Time:", elapsed.count(), "s");
            }
        }
    }

    outputFile.close();
}

// Function to run the simulations
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

    vector<int> m_values = {2500};
    vector<double> r_values = {2.0};
    int numExecutions = 10;
    //double per = 0;
    double per = 0.1;
    string searchMethod = "searchLocalIterative";
    //string searchMethod = "searchLocalBestImprovement";
    //string searchMethod = "searchLocalFirstImprovement";

    print("Starting simulations...");

    runSimulations(m_values, r_values, numExecutions, searchMethod ,per);

    print("Simulations finished!");

    //print("First machine Debug:");

    //printWorkStation(workStation);

    return 0;
}