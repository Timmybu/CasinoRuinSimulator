#include <iostream>
#include <random>       // For modern C++ random number generation
#include <vector>       // To store the bankrolls we want to test
#include <iomanip>      // For formatting the output (setw, setprecision)
#include <chrono>       // For seeding the random number generator

/**
 * @brief Simulates a single run (e.g., one casino's lifetime) of many bets.
 * @param initialHouseBankroll The starting capital for the house.
 * @param betAmount The fixed amount of each bet.
 * @param numBets The total number of bets to simulate in this run.
 * @param houseWinProb The probability (0.0 to 1.0) that the house wins a single bet.
 * @param runIndex A unique index for this run, used to ensure a different random seed.
 * @return true if the house was ruined (bankroll < betAmount), false otherwise.
 */
bool simulateSingleRun(double initialHouseBankroll, double betAmount, long long numBets, double houseWinProb, int runIndex) {

    // Seed the random number generator. 
    // We use a combination of the current time and the runIndex to ensure
    // that even runs starting at the same millisecond get different random sequences.
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count() + runIndex;
    std::mt19937 generator(seed);

    // We use a uniform real distribution. If the number is < houseWinProb, the house wins.
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    double currentBankroll = initialHouseBankroll;

    for (long long i = 0; i < numBets; ++i) {
        // Simulate one coin flip
        if (distribution(generator) < houseWinProb) {
            // House wins
            currentBankroll += betAmount;
        }
        else {
            // Player wins
            currentBankroll -= betAmount;
        }

        // Check for ruin
        if (currentBankroll < betAmount) {
            // The house doesn't have enough money to cover the next player's win.
            // They are ruined.
            return true;
        }
    }

    // If the loop finishes, the house survived this run.
    return false;
}

int main() {
    // --- Configuration Parameters ---
    const double HOUSE_WIN_PROB = 5.0 / 9.0; // Approx 0.555...
    const double BET_AMOUNT = 25.0;

    // Simulate 1 million bets per run. This represents one "scenario".
    const long long BETS_PER_RUN = 1000000;

    // Run 10,000 scenarios to get a good statistical average.
    const int TOTAL_RUNS = 10000;

    // A list of different starting bankrolls to test.
    // Feel free to change these values!
    std::vector<double> bankrollsToTest = { 500, 1000, 2500, 5000, 7500, 10000, 15000, 20000 };

    // --- Simulation Start ---
    std::cout << "--- Casino Ruin Simulation ---" << std::endl;
    std::cout << "House Win Probability: " << (HOUSE_WIN_PROB * 100.0) << "%" << std::endl;
    std::cout << "Bet Amount: $" << BET_AMOUNT << std::endl;
    std::cout << "Simulating " << TOTAL_RUNS << " runs of "
        << BETS_PER_RUN << " bets each..." << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << std::fixed << std::setprecision(5);
    std::cout << std::setw(18) << "House Bankroll" << " | "
        << std::setw(12) << "Ruin Count" << " | "
        << "Ruin Prob (%)" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    // Loop over each bankroll we want to test
    for (double startBankroll : bankrollsToTest) {
        int ruinCount = 0;

        // Run the main simulation loop
        // Note: For more speed, this inner loop could be parallelized.
        for (int i = 0; i < TOTAL_RUNS; ++i) {
            if (simulateSingleRun(startBankroll, BET_AMOUNT, BETS_PER_RUN, HOUSE_WIN_PROB, i)) {
                ruinCount++;
            }
        }

        // Calculate and print the result for this bankroll
        double ruinProbability = static_cast<double>(ruinCount) / TOTAL_RUNS;

        std::cout << "$" << std::setw(17) << startBankroll << " | "
            << std::setw(12) << ruinCount << " | "
            << std::setw(12) << (ruinProbability * 100.0)
            << std::endl;
    }

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "Simulation complete." << std::endl;

    return 0;
}
