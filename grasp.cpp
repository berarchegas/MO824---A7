#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <ctime>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <list>

using namespace std;

vector<string> instances = {
    "vrp_instances/instance3.vrp",
};

bool intensification = false;
bool diversification = false;

int calculateRouteCost(vector<int>& route, vector<vector<int> >& distanceMatrix) {
    int cost = 0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        cost += distanceMatrix[route[i]][route[i + 1]];
    }
    return cost;
}

vector<vector<int>> generateInitialSolution(vector<vector<int>>& distanceMatrix, int dimension, int vehicleCapacity, vector<int>& demand, double alpha = 0.3) {
    
    vector<vector<int>> routes;
    vector<int> unvisitedCustomers;

    for (int i = 1; i < dimension; ++i) {
        unvisitedCustomers.push_back(i);
    }

    int vehicleIndex = 0;
    routes.push_back(vector<int>());
    routes[0].push_back(0); 

    random_device rd;
    mt19937 gen(rd());

    while (!unvisitedCustomers.empty()) {

        vector<vector<int> > customerWithIncrease; //primeiro elemento do par é o cliente, segundo é o aumento de custo
        int currentLoad = 0;
        for (int node : routes[vehicleIndex]) {
            currentLoad += demand[node];
        }

        // para todos os clientes não visitados, verifica se é possível adicionar o cliente sem exceder a capacidade
        int increase;
        bool returnToDepot;
        for (int customer : unvisitedCustomers) {
            returnToDepot = false;
            int lastNode = routes[vehicleIndex].back();
            if (currentLoad + demand[customer] <= vehicleCapacity) {
                increase = distanceMatrix[lastNode][customer] + distanceMatrix[customer][0] - distanceMatrix[lastNode][0];
                if (increase > distanceMatrix[0][customer] + distanceMatrix[customer][0]) {
                    increase = distanceMatrix[0][customer] + distanceMatrix[customer][0];
                    returnToDepot = true;
                }
                if (returnToDepot) {
                    customerWithIncrease.push_back({customer, increase, 1});
                }
                else {
                    customerWithIncrease.push_back({customer, increase, 0});
                }
            }
        }

        if (customerWithIncrease.empty()) { // Não há clientes restantes que possam ser adicionados sem exceder a capacidade
            
            routes[vehicleIndex].push_back(0); // Retorna ao depot
            vehicleIndex++;
            routes.push_back(vector<int>());
            routes[vehicleIndex].push_back(0); // Comeca nova rota no depot

        } else {

            sort(customerWithIncrease.begin(), customerWithIncrease.end(), [](vector<int> a, vector<int> b) {
                return a[1] < b[1];
            });

            vector<vector<int> > rcl;
            double threshold = customerWithIncrease[0][1] + alpha * (customerWithIncrease.back()[1] - customerWithIncrease[0][1]);
            for (auto customer : customerWithIncrease) {
                if (customer[1] <= threshold) {
                    rcl.push_back(customer);
                }
            }

            uniform_int_distribution<> dis(0, rcl.size() - 1);
            int randomIndex = dis(gen);

            int selectedCustomer = rcl[randomIndex][0];
            if (rcl[randomIndex][2] == 1) {
                routes[vehicleIndex].push_back(0);
                vehicleIndex++;
                routes.push_back(vector<int>());
                routes[vehicleIndex].push_back(0);
            }
            routes[vehicleIndex].push_back(selectedCustomer);
            unvisitedCustomers.erase(remove(unvisitedCustomers.begin(), unvisitedCustomers.end(), selectedCustomer), unvisitedCustomers.end());
        }
    }

    routes[vehicleIndex].push_back(0); // Return to depot

    return routes;
}

bool isTabu(const vector<int>& move, list<vector<int>>& tabuList) {
    return find(tabuList.begin(), tabuList.end(), move) != tabuList.end();
}

vector<vector<int>> localSearch(vector<vector<int>>& solution, vector<vector<int>>& distanceMatrix, vector<int>& demand, int vehicleCapacity, int bestGlobalCost, int maxTabuSize = 100) {
   
    list<vector<int>> tabuList;
    bool improved = true;
    int bestCost = 0;
    vector<vector<int>> bestSolution = solution;

    // Calcula o custo inicial da solução
    for (auto& route : bestSolution) {
        bestCost += calculateRouteCost(route, distanceMatrix);
    }

    random_device rd;
    mt19937 gen(rd());

    while (improved) {
        improved = false;

        uniform_int_distribution<> dis(0, bestSolution.size() - 1);
        vector<int> selected_routes;
        for (int i = 0; i < 10; ++i) {
            selected_routes.push_back(dis(gen));
        }

        // 2-opt para cada rota
        for (int i : selected_routes){
            vector<int> route = bestSolution[i];
            int routeSize = route.size();
            int currentRouteCost = calculateRouteCost(route, distanceMatrix);

            // Sorteia 100 pares de clientes para fazer o 2-opt
            uniform_int_distribution<> dis2(1, routeSize - 2);
            vector<pair<int, int>> pairs;
            for (int j = 0; j < 100; ++j) {
                int a = dis2(gen);
                int b = dis2(gen);
                if (a < b) {
                    pairs.emplace_back(a, b);
                } else {
                    pairs.emplace_back(b, a);
                }
            }

            for (pair<int, int> p : pairs) {
                int j = p.first;
                int k = p.second;
                vector<int> newRoute = route;
                reverse(newRoute.begin() + j, newRoute.begin() + k + 1);
                int newRouteCost = calculateRouteCost(newRoute, distanceMatrix);
                int newTotalCost = bestCost - currentRouteCost + newRouteCost;

                vector<int> move = {i, j, k};
                if (newTotalCost < bestCost && (!isTabu(move, tabuList) || newTotalCost < bestGlobalCost)) {
                    solution[i] = newRoute;
                    improved = true;
                    bestCost = newTotalCost;
                    bestSolution = solution;
                    tabuList.push_back(move);
                    if (tabuList.size() > maxTabuSize) {
                        tabuList.pop_front();
                    }
                }
            }
                
        }

        // Sorteia 10 pares de rotas para fazer o swap
        vector<int> routei;
        vector<int> routej;
        for (int i = 0; i < 10; ++i) {
            routei.push_back(dis(gen));
            routej.push_back(dis(gen));
        }

        // Swap inter-rotas
        for (size_t idx = 0; idx < routei.size(); ++idx) {
            int i = routei[idx];
            int j = routej[idx];
            if (i == j) continue;

            int costI = calculateRouteCost(bestSolution[i], distanceMatrix);
            int loadI = accumulate(bestSolution[i].begin(), bestSolution[i].end(), 0, [&](int sum, int node) { return sum + demand[node]; });

            int costJ = calculateRouteCost(bestSolution[j], distanceMatrix);
            int loadJ = accumulate(bestSolution[j].begin(), bestSolution[j].end(), 0, [&](int sum, int node) { return sum + demand[node]; });

            // Sorteia 100 pares de clientes para fazer a troca
            uniform_int_distribution<> dis2(1, min(bestSolution[i].size(), bestSolution[j].size()) - 2);
            vector<pair<int, int>> pairs;
            for (int a = 0; a < 100; ++a) {
                pairs.emplace_back(dis2(gen), dis2(gen));
            }

            for (pair<int, int> p : pairs) {
                int k = p.first;
                int l = p.second;

                if ((loadI - demand[bestSolution[i][k]] + demand[bestSolution[j][l]]) <= vehicleCapacity && (loadJ - demand[bestSolution[j][l]] + demand[bestSolution[i][k]]) <= vehicleCapacity) {

                    vector<int> newRouteI = bestSolution[i];
                    vector<int> newRouteJ = bestSolution[j];

                    swap(newRouteI[k], newRouteJ[l]);

                    int newCostI = calculateRouteCost(newRouteI, distanceMatrix);
                    int newCostJ = calculateRouteCost(newRouteJ, distanceMatrix);
                    int newTotalCost = bestCost - costI - costJ + newCostI + newCostJ;

                    vector<int> move = {static_cast<int>(i), static_cast<int>(j), k, l};
                    if (newTotalCost < bestCost && (!isTabu(move, tabuList) || newTotalCost < bestGlobalCost)) {
                        solution[i] = newRouteI;
                        solution[j] = newRouteJ;
                        improved = true;
                        bestCost = newTotalCost;
                        bestSolution = solution;
                        tabuList.push_back(move);
                        if (tabuList.size() > maxTabuSize) {
                            tabuList.pop_front();
                        }
                    }
                }
            }
        }
    }

    return bestSolution;
}

vector<vector<int>> perturbation(vector<vector<int>>& solution, const vector<vector<int>>& distanceMatrix, const vector<int>& demand, int vehicleCapacity) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, solution.size() - 1);

    for (int perturbation_count = 0; perturbation_count < 5; ++perturbation_count) {
        // Seleciona rotas aleatórias para perturbar
        int routeIndex1 = dis(gen);
        int routeIndex2 = dis(gen);

        // Garantir que as rotas sejam diferentes e tenham pelo menos dois clientes para perturbar
        if (routeIndex1 != routeIndex2 && solution[routeIndex1].size() > 2 && solution[routeIndex2].size() > 2) {
            uniform_int_distribution<> disRoute1(1, solution[routeIndex1].size() - 2);
            uniform_int_distribution<> disRoute2(1, solution[routeIndex2].size() - 2);

            int i = disRoute1(gen);
            int j = disRoute2(gen);

            int demand1 = accumulate(solution[routeIndex1].begin(), solution[routeIndex1].end(), 0, [&](int sum, int customer) { return sum + demand[customer]; });
            int demand2 = accumulate(solution[routeIndex2].begin(), solution[routeIndex2].end(), 0, [&](int sum, int customer) { return sum + demand[customer]; });

            // Verificar se a troca não viola a capacidade do veículo
            if (demand1 - demand[solution[routeIndex1][i]] + demand[solution[routeIndex2][j]] <= vehicleCapacity &&
                demand2 - demand[solution[routeIndex2][j]] + demand[solution[routeIndex1][i]] <= vehicleCapacity) {
                swap(solution[routeIndex1][i], solution[routeIndex2][j]);
            }
        }
    }

    return solution;
}

vector<vector<int> > grasp(int iterations, vector<vector<int> > distanceMatrix, int dimension, int capacity, vector<int> demand) {


    cout << "GRASP" << endl;
    cout << "diversification: " << diversification << endl;
    cout << "intensification: " << intensification << endl;

    vector<vector<int> > bestSolution;
    int bestCost = INT_MAX;
    int lastBestCost = INT_MAX;

    // parametros para o reactive grasp: 
    double possible_alphas[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    double averages_for_each_alpha[9] = {0}; // media de custo para as solucoes encontradas com cada alfa
    double counts_for_each_alpha[9] = {0}; // contagem de solucoes encontradas com cada alfa
    int alpha_index; // indice do alfa atual
    vector<double> probs = {1, 1, 1, 1, 1, 1, 1, 1, 1}; // probabilidades iniciais para cada alfa
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<int> currentdis(probs.begin(), probs.end());
    alpha_index = currentdis(gen);

    auto start = chrono::high_resolution_clock::now();
    int max_duration_seconds = 15 * 60;
    int i = 1;
    while (chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start).count() < max_duration_seconds) {
        
        vector<vector<int> > solution;
        if (diversification) {
            solution = generateInitialSolution(distanceMatrix, dimension, capacity, demand, possible_alphas[alpha_index]);
            solution = localSearch(solution, distanceMatrix, demand, capacity, bestCost, intensification ? 100 : 0);
            solution = perturbation(solution, distanceMatrix, demand, capacity);
        }
        else {
            solution = generateInitialSolution(distanceMatrix, dimension, capacity, demand);
        }
        solution = localSearch(solution, distanceMatrix, demand, capacity, bestCost, intensification ? 100 : 0);
        
        int cost = 0;
        for (int j = 0; j < solution.size(); ++j) {
            cost += calculateRouteCost(solution[j], distanceMatrix);
        }
        
        counts_for_each_alpha[alpha_index]++;
        averages_for_each_alpha[alpha_index] = (averages_for_each_alpha[alpha_index] * (counts_for_each_alpha[alpha_index] - 1) + cost) / counts_for_each_alpha[alpha_index];
        
        if (cost < bestCost) {
            bestSolution = solution;
            bestCost = cost;
        }

        if (i % 20 == 0) {
            cout << "Iteration " << i << endl;
            cout << "Best cost: " << bestCost << endl;
            cout << "Alpha: " << possible_alphas[alpha_index] << endl;

            // Recalcular probabilidades
            vector<double> probs(9, 0.0);
            double sum = 0;
            for (int j = 0; j < 9; ++j) {
                if (averages_for_each_alpha[j] > 0) {
                    probs[j] = bestCost / averages_for_each_alpha[j];
                    sum += probs[j];
                }
            }

            if (sum > 0) {
                for (int j = 0; j < 9; ++j) {
                    probs[j] /= sum;
                }

                discrete_distribution<int> dis(probs.begin(), probs.end());
                currentdis = dis;
                alpha_index = currentdis(gen);
            } else {
                alpha_index = currentdis(gen);
            }
        } else {
            alpha_index = currentdis(gen);
        }

        i++;
    }

    return bestSolution;
}

struct Node {
    int x;
    int y;
};

int main() {
    int dimension;
    int capacity;
    
    for (string instance : instances) {

        cout << "Instance: " << instance << endl;

        ifstream file(instance);
        string line;
        vector<Node> nodes;
        vector<int> demand;
        bool coordSection = false;
        bool demandSection = false;
        bool depotSection = false;
        bool first = true;

        while (getline(file, line)) {

            istringstream iss(line);
            string keyword;
            iss >> keyword;

            if (keyword == "DIMENSION") {
                iss.ignore(3); 
                iss >> dimension;
            } else if (keyword == "CAPACITY") {
                iss.ignore(3); 
                iss >> capacity;
            } else if (keyword == "NODE_COORD_SECTION") {
                coordSection = true;
                continue;   
            } else if (keyword == "DEMAND_SECTION") {
                coordSection = false;
                demandSection = true;
                continue;
            } else if (keyword == "DEPOT_SECTION") {
                depotSection = true;
                demandSection = false; 
                continue;
            }

            if (coordSection) {
                Node node;
                iss.ignore(1);
                iss >> node.x >> node.y;
                nodes.push_back(node);
            }
            else if (demandSection) {
                int newdemand;
                iss.ignore(1);
                iss >> newdemand;
                demand.push_back(newdemand);
            }
            else if (depotSection) {
                if (first) {
                    first = false;
                    Node depot;
                    depot.x = stoi(keyword);
                    nodes.insert(nodes.begin(), depot);
                }
                else {
                    nodes[0].y = stoi(keyword);
                    break;
                }
            }
        }

        vector<vector<int> > distanceMatrix(dimension, vector<int>(dimension, 0));
        for (int i = 0; i < dimension; ++i) {
            for (int j = 0; j < dimension; ++j) {
                distanceMatrix[i][j] = floor(
                    sqrt(
                        pow(nodes[i].x - nodes[j].x, 2) +
                        pow(nodes[i].y - nodes[j].y, 2)
                    ) + 0.5
                );
            }
        }

        int iterations = 200;
        vector<vector<int> > bestSolution = grasp(iterations, distanceMatrix, dimension, capacity, demand);

    }

    return 0;
}
