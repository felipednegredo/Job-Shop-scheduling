#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>

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
    int totalProcessingTime = 0;
};

// Struct for WorkStation
struct WorkStation {
    // Vector of machines
    vector<Machine> machines;
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
        machines[0].totalProcessingTime += task.processingTime;
    }

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
                        int newTotalProcessingTime = otherMachine.totalProcessingTime + task.processingTime;

                        // If the new total processing time is less than the total processing time of the machine
                        if (newTotalProcessingTime < machine.totalProcessingTime) {
                            // Remove the task from the machine
                            machine.tasks.erase(machine.tasks.begin() + i);
                            // Update the total processing time of the machine
                            machine.totalProcessingTime -= task.processingTime;

                            // Add the task to the other machine
                            otherMachine.tasks.push_back(task);
                            // Update the total processing time of the other machine
                            otherMachine.totalProcessingTime += task.processingTime;

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

// Function to search for the best improvement
void searchLocalBestImprovement(WorkStation& workStation) {
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
                // Pointer to the best machine
                Machine* bestMachine = nullptr;
                // Best total processing time
                int bestTotalProcessingTime = machine.totalProcessingTime;

                // For each other machine
                for (auto& otherMachine : workStation.machines) {
                    // If the machine is different from the other machine
                    if (&machine != &otherMachine) {
                        // Calculate the new total processing time
                        int newTotalProcessingTime = otherMachine.totalProcessingTime + task.processingTime;

                        // If the new total processing time is less than the best total processing time
                        if (newTotalProcessingTime < bestTotalProcessingTime) {
                            // Update the best machine
                            bestMachine = &otherMachine;
                            // Update the best total processing time
                            bestTotalProcessingTime = newTotalProcessingTime;
                        }
                    }
                }

                // If there is a best machine
                if (bestMachine != nullptr) {
                    // Remove the task from the machine
                    machine.tasks.erase(machine.tasks.begin() + i);
                    // Update the total processing time of the machine
                    machine.totalProcessingTime -= task.processingTime;

                    // Add the task to the best machine
                    bestMachine->tasks.push_back(task);
                    // Update the total processing time of the best machine
                    bestMachine->totalProcessingTime += task.processingTime;

                    // Set the improvement flag to true
                    improvement = true;
                }
            }
        }
    }
}

// Function to run the simulations
void runSimulations(const vector<int>& m_values, const vector<double>& r_values, int numExecutions, WorkStation& workStation, string searchMethod) {
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
            print("--------------------------------------");
            for (int exec = 1; exec <= numExecutions; ++exec) {
                // Initialize the machines
                workStation.machines = initialize(m, r);
                
                // Start timing
                auto start = std::chrono::high_resolution_clock::now();

                // Search for the best improvement
                if (searchMethod == "searchLocalBestImprovement") {
                    // Search for the best improvement
                    searchLocalBestImprovement(workStation);
                } else if (searchMethod == "searchLocalFirstImprovement") {
                    // Search for the first improvement
                    searchLocalFirstImprovement(workStation);
                } else if (searchMethod == "local iterative") {
                    print("Local Iterative");
                }

                // Stop timing
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - start;

                // Calculate the makespan
                int makespan = 0;
                for (const auto& machine : workStation.machines) {
                    // Update the makespan
                    if (machine.totalProcessingTime > makespan) {
                        makespan = machine.totalProcessingTime;
                    }
                }
                
                // Write the results to the output file
                outputFile << searchMethod << "," << static_cast<int>(pow(m, r)) << "," << m << "," << exec << "," << elapsed.count() << "," << numExecutions << "," << makespan << "," << r << "\n";
                
                print("Method:", searchMethod, "| Execution:", exec, "| m:", m, "| r:", r, "| Makespan:", makespan, "| Number of tasks:", workStation.machines[0].tasks.size(), "| Time:", elapsed.count(), "s");
            }
            print("--------------------------------------");
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
    int numExecutions = 10;
    WorkStation workStation;
    bool useBestImprovement = true;

    print("Starting simulations...");

    runSimulations(m_values, r_values, numExecutions, workStation, useBestImprovement ? "searchLocalBestImprovement" : "searchLocalFirstImprovement");

    return 0;
}
