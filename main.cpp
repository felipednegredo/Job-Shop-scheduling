#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>


using std::vector;


struct Task {
    int id;
    int processingTime;
};

struct Machine {
    vector<Task> tasks;
    int totalProcessingTime = 0;
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


void generateTasks(vector<Task>& tasks, int numTasks) {
    for (int i = 0; i < numTasks; ++i) {
        tasks.push_back({i, rand() % 100 + 1});
    }
}

void initState(vector<Machine>& machines, const vector<Task>& tasks) {
    for (const auto& task : tasks) {
        machines[0].tasks.push_back(task);
    }
    machines[0].totalProcessingTime = std::accumulate(tasks.begin(), tasks.end(), 0, [](int sum, const Task& task) {
        return sum + task.processingTime;
    });
}

void distributeTasks(vector<Machine>& machines, const vector<Task>& tasks) {
    for (const auto& task : tasks) {
        auto minMachine = std::min_element(machines.begin(), machines.end(), [](const Machine& a, const Machine& b) {
            return a.totalProcessingTime < b.totalProcessingTime;
        });
        minMachine->tasks.push_back(task);
        minMachine->totalProcessingTime += task.processingTime;
    }
}

int calculateMakespan(const vector<Machine>& machines) {
    int makespan = 0;
    for (const auto& machine : machines) {
        makespan = std::max(makespan, machine.totalProcessingTime);
    }
    return makespan;
}

// void printMachinesAndTasks(const vector<Machine>& machines) {
//     for (size_t i = 0; i < machines.size(); ++i) {
//         print("Machine", i + 1, "Total Processing Time:", machines[i].totalProcessingTime);
//         for (const auto& task : machines[i].tasks) {
//             print("  Task ID:", task.id, "Processing Time:", task.processingTime);
//         }
//     }
// }


// void generateReportInCSV(const vector<Machine>& machines, const std::string& filename) {
//     std::ofstream file(filename);
//     if (file.is_open()) {
//         file << "Heuristica,N,M,Replicacao,Tempo,Iteracoes,Valor,Parametro\n";
//         for (size_t i = 0; i < machines.size(); ++i) {
//             for (const auto& task : machines[i].tasks) {
//                 file << i + 1 << "," << task.id << "," << task.processingTime << "\n";
//             }
//         }
//         file.close();
//     } else {
//         std::cerr << "Unable to open file for writing: " << filename << '\n';
//     }
// }
// 

int random_int(int lo, int hi){
    return lo + std::rand() % (hi - lo + 1);
}

int main() {
    std::srand(std::chrono::system_clock::now().time_since_epoch().count());
    for(int i = 0; i < 69; i++){
       print(random_int(0, 100));
    }
    // srand(static_cast<unsigned int>(time(0)));

    // vector<int> machineCounts = {10, 20, 50};
    // vector<double> taskMultipliers = {1.5, 2.0};

    // initState(machines, tasks);
    // printMachinesAndTasks(machines);

    return 0;
}