#include <iostream>
#include <random>       // For modern C++ random number generation
#include <vector>       // To store the bankrolls we want to test
#include <iomanip>      // For formatting the output (setw, setprecision)
#include <chrono>       // For seeding the random number generator
#include <map>          // For histogram bins
#include <cmath>        // For floor
#include <limits>       // For min/max initialization

/**
 * @brief Simulates a single run (e.g., one casino's lifetime) of many bets.
 * @param initialHouseBankroll The starting capital for the house.
 * @param betAmount The fixed amount of each bet.
 * @param numBets The total number of bets to simulate in this run.
 * @param houseWinProb The probability (0.0 to 1.0) that the house wins a single bet.
 * @param runIndex A unique index for this run, used to ensure a different random seed.
 * @return The final bankroll of the house after the run.
 * If the house is ruined, this value will be < betAmount.
 */
double simulateSingleRun(double initialHouseBankroll, double betAmount, long long numBets, double houseWinProb, int runIndex) {

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
            // They are ruined. Return the current (ruined) bankroll.
            return currentBankroll;
        }
    }

    // If the loop finishes, the house survived. Return the final bankroll.
    return currentBankroll;
}

/**
 * @brief Analyzes and prints a histogram of final (surviving) bankrolls.
 * @param finalBankrolls A vector containing the final bankroll from every run.
 * @param betAmount The bet amount, used to identify ruined runs.
 * @param numBins The number of ranges to create for the histogram.
 * @param totalRuns The total number of simulations.
 */
void printBankrollHistogram(const std::vector<double>& finalBankrolls, double betAmount, int numBins, int totalRuns) {
    std::vector<double> survivingBankrolls;
    int ruinCount = 0;

    double minBankroll = std::numeric_limits<double>::max();
    double maxBankroll = std::numeric_limits<double>::lowest();

    for (double finalBankroll : finalBankrolls) {
        if (finalBankroll < betAmount) {
            ruinCount++;
        }
        else {
            survivingBankrolls.push_back(finalBankroll);
            if (finalBankroll < minBankroll) minBankroll = finalBankroll;
            if (finalBankroll > maxBankroll) maxBankroll = finalBankroll;
        }
    }

    int numSurvivors = survivingBankrolls.size();
    if (numSurvivors == 0) {
        std::cout << "    No surviving runs to chart." << std::endl;
        return;
    }

    // --- Create Bins ---
    // We use a map to store bins. The key = the lower bound of the bin range.
    std::map<double, int> bins;
    double binWidth = (maxBankroll - minBankroll) / numBins;

    // Handle the case where min == max (all survivors have the same bankroll)
    if (binWidth == 0) {
        // Avoid division by zero if all values are identical
        binWidth = 100.0;
    }

    // Initialize bins
    for (int i = 0; i < numBins; ++i) {
        bins[minBankroll + i * binWidth] = 0;
    }

    // Populate bins
    int maxBinCount = 0; // For scaling the chart
    for (double bankroll : survivingBankrolls) {
        double binKey;
        if (binWidth == 0) {
            binKey = minBankroll;
        }
        else {
            // Find the bin this bankroll belongs to
            binKey = minBankroll + std::floor((bankroll - minBankroll) / binWidth) * binWidth;
        }

        // Handle the max value, which might fall just outside the last bin due to precision
        if (bankroll == maxBankroll) {
            auto it = bins.rbegin(); // Get the last bin
            it->second++;
            if (it->second > maxBinCount) maxBinCount = it->second;
        }
        else {
            auto it = bins.find(binKey);
            if (it != bins.end()) {
                it->second++;
                if (it->second > maxBinCount) maxBinCount = it->second;
            }
        }
    }

    // --- Print Histogram ---
    std::cout << "\n    --- Final Bankroll Distribution (for " << numSurvivors << " surviving runs) ---" << std::endl;
    std::cout << "    Min Surviving Bankroll: $" << minBankroll << std::endl;
    std::cout << "    Max Surviving Bankroll: $" << maxBankroll << std::endl;
    std::cout << "    ------------------------------------------------------------------" << std::endl;

    const int MAX_BAR_WIDTH = 40; // Max characters for the bar

    std::cout << std::fixed << std::setprecision(2);
    // C++17: for (auto const& [rangeStart, count] : bins) {
    // C++11 compatible version:
    for (auto const& binPair : bins) {
        double rangeStart = binPair.first;
        int count = binPair.second;

        double rangeEnd = rangeStart + binWidth;
        std::cout << "    $" << std::setw(12) << rangeStart << " - $" << std::setw(12) << rangeEnd << " | ";

        int barWidth = 0;
        if (maxBinCount > 0) {
            // Scale the bar width relative to the most populated bin
            barWidth = static_cast<int>((static_cast<double>(count) / maxBinCount) * MAX_BAR_WIDTH);
        }

        for (int i = 0; i < barWidth; ++i) {
            std::cout << "#";
        }

        double percentage = 0.0;
        if (numSurvivors > 0) {
            percentage = (static_cast<double>(count) / numSurvivors) * 100.0;
        }
        std::cout << " (" << count << ", " << std::setprecision(1) << percentage << "%)" << std::endl;
    }
    std::cout << std::fixed << std::setprecision(5); // Reset precision for main loop
    std::cout << "    ------------------------------------------------------------------" << std::endl;
}


int main() {

    // --- Configuration Parameters ---
    // These are all the values you might want to change

    // The house's advantage on a single bet (5/9 = 0.555...)
    const double HOUSE_WIN_PROB = 5.0 / 9.0;

    // The fixed bet amount for every game
    const double BET_AMOUNT = 25.0;

    // The number of bets in a single "run" (e.g., one casino's lifetime)
    const long long BETS_PER_RUN = 100;

    // The total number of "runs" to simulate for each bankroll
    // More runs = more accurate probability, but slower simulation
    const int TOTAL_RUNS = 1000000;

    // The number of bars/ranges to display in the final histogram
    const int HISTOGRAM_BINS = 15;

    // A list of different starting bankrolls to test
    std::vector<double> bankrollsToTest = {500};
    // ----------------------------------


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
        std::vector<double> finalBankrolls; // Store all final bankrolls
        finalBankrolls.reserve(TOTAL_RUNS); // Pre-allocate memory

        // Run the main simulation loop
        // Note: For more speed, this inner loop could be parallelized.
        for (int i = 0; i < TOTAL_RUNS; ++i) {
            double finalBankroll = simulateSingleRun(startBankroll, BET_AMOUNT, BETS_PER_RUN, HOUSE_WIN_PROB, i);
            finalBankrolls.push_back(finalBankroll);
            if (finalBankroll < BET_AMOUNT) {
                ruinCount++;
            }
        }

        // Calculate and print the result for this bankroll
        double ruinProbability = static_cast<double>(ruinCount) / TOTAL_RUNS;

        std::cout << "$" << std::setw(17) << startBankroll << " | "
            << std::setw(12) << ruinCount << " | "
            << std::setw(12) << (ruinProbability * 100.0)
            << std::endl;

        // --- Print the new histogram ---
        printBankrollHistogram(finalBankrolls, BET_AMOUNT, HISTOGRAM_BINS, TOTAL_RUNS);
        std::cout << std::endl; // Add a blank line for readability
    }

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "Simulation complete." << std::endl;

    return 0;
}