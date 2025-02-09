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

// Function to compute the standard deviation of the makespan
double computeMakespanStdDev(const std::vector<Machine>& machines) {
    // Calculate the mean and the sum of the squares of the makespan
    double sum = 0, sumSq = 0;
    // Get the number of machines
    int n = machines.size();

    // For each machine
    for (const auto& machine : machines) {
        // Update the sum and the sum of the squares
        sum += machine.makespan;
        // Update the sum of the squares
        sumSq += machine.makespan * machine.makespan;
    }

    // Calculate the mean
    double mean = sum / n;
    // Return the square root of the difference between the sum of the squares divided by n and the square of the mean
    return std::sqrt((sumSq / n) - (mean * mean));
}

// Function to search for the best improvement
void searchLocalBestImprovement(WorkStation& workStation) {
    bool improvement = true;

    while (improvement) {
        improvement = false;

        // Calculate the current standard deviation
        double currentStdDev = computeMakespanStdDev(workStation.machines);

        Task* bestTask = nullptr;
        Machine* sourceMachine = nullptr;
        Machine* targetMachine = nullptr;
        double bestStdDevReduction = 0.0;

        // For each machine
        for (auto& machine : workStation.machines) {
            // If the machine is empty, skip it
            if (machine.tasks.empty()) continue;

            // For each task in the machine
            for (size_t i = 0; i < machine.tasks.size(); ++i) {
                // Get the task
                Task& task = machine.tasks[i];

                // For each other machine
                for (auto& otherMachine : workStation.machines) {
                    // If the machine is the same as the other machine, skip it
                    if (&machine == &otherMachine) continue;

                    // Simulate the reallocation
                    machine.makespan -= task.processingTime;
                    otherMachine.makespan += task.processingTime;

                    // Calculate the new standard deviation
                    double newStdDev = computeMakespanStdDev(workStation.machines);

                    // Calculate the standard deviation reduction
                    double stdDevReduction = currentStdDev - newStdDev;
                    if (stdDevReduction > bestStdDevReduction) {
                        // Update the best improvement
                        bestStdDevReduction = stdDevReduction;
                        // Update the best task
                        bestTask = &task;
                        // Update the source machine
                        sourceMachine = &machine;
                        // Update the target machine
                        targetMachine = &otherMachine;
                    }

                    // Undo the reallocation
                    machine.makespan += task.processingTime;
                    otherMachine.makespan -= task.processingTime;
                }
            }
        }

        // If there is a best improvement
        if (bestTask && sourceMachine && targetMachine) {
            // Remove the best task from the source machine
            auto it = std::find_if(sourceMachine->tasks.begin(), sourceMachine->tasks.end(),
                                   [&](const Task& t) { return t.id == bestTask->id; });

            // If the task was found
            if (it != sourceMachine->tasks.end()) {
                // Remove the task from the source machine
                sourceMachine->tasks.erase(it);
                // Update the makespan of the source machine
                sourceMachine->makespan -= bestTask->processingTime;
            }

            // Add the best task to the target machine
            targetMachine->tasks.push_back(*bestTask);
            // Update the makespan of the target machine
            targetMachine->makespan += bestTask->processingTime;

            // Update the number of steps
            workStation.steps++;
            // Set the improvement flag to true
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
            maxMakespan = std::max(maxMakespan, machine.makespan);
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

        for (int i = 0; i < numPerturbations; ++i) {
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

        int makespanBefore = calculateMaxMakespan(s_prime);
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
void runSimulations(const vector<int>& m_values, const vector<double>& r_values, int numExecutions, WorkStation& workStation, string searchMethod, double per) {
    // Delete the output file if it exists
    std::remove("python/resultados.csv");

    // Open the output file
    std::ofstream outputFile("python/resultados.csv");
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
                
                // Write the results to the output file
                outputFile << searchMethod << "," << static_cast<int>(pow(m, r)) << "," << m << "," << exec << "," << elapsed.count() << "," << workStation.steps << "," << "teste" << "," << parametre << "\n";
                
                // Export the task allocation to a CSV file for each execution
                exportTaskAllocation(workStation, "python/data"+ searchMethod +"/tasks_" + searchMethod + "_M" + std::to_string(m) + "_tasks" + std::to_string(static_cast<int>(pow(m, r))) + "_exec" + std::to_string(exec) + ".csv");

                print("Method:", searchMethod, "| Execution:", exec, "| m:", m, "| r:", r, "|",workStation.machines[0].makespan, "| Number of tasks:", static_cast<int>(pow(m, r)), "| Time:", elapsed.count(), "s");
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

    vector<int> m_values = {10, 20, 50};
    vector<double> r_values = {1.5, 2.0};
    int numExecutions = 1;
    WorkStation workStation;
    double per = 0;
    //double per = 0.1;
    //string searchMethod = "searchLocalIterative";
    //string searchMethod = "searchLocalBestImprovement";
    string searchMethod = "searchLocalFirstImprovement";

    print("Starting simulations...");

    runSimulations(m_values, r_values, numExecutions, workStation, searchMethod ,per);

    print("Simulations finished!");

    //print("First machine Debug:");

    //printWorkStation(workStation);

    return 0;
}
