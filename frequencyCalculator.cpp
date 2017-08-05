#include <iostream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <list>
#include <map>

using namespace std;

int main() {
    double freq1 = 44100;
    double freq2 = 48000;
    double fftSize = 1024;

    double minPeriod = 1/freq1;
    double maxPeriod = 1/20.0;

    double tolerance = 2;

    list<double> factors1;
    multimap<double, double> frequencies; // <error, frequency>

    for (int i = 0; i < fftSize/2.0; i++) {
        double nyquistFrequency = 0.5 * freq1;
        double factor = i/(fftSize/2.0) * nyquistFrequency;
        factors1.push_back(factor);
    }

    std::cout.precision(10);

    for (int i = 0; i < fftSize/2.0; i++) {
        double nyquistFrequency = 0.5 * freq2;
        double factor = i/(fftSize/2.0) * nyquistFrequency;

        for (double efactor : factors1) {
            if (fabs(efactor - factor) < tolerance) {
                frequencies.emplace(fabs(efactor-factor), (factor+efactor)/2);
            }
        }
    }

    for (pair<double, double> freq : frequencies) {
        cout << "Found factor " << fixed << setprecision(5) << setw(12)
                     << freq.second
                     << " hz for frequency "
                     << "(±"  << setw(5) << setprecision(3) << freq.first << ")"
                     << endl;
    }
}
