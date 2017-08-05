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

	double minPeriod = 1 / freq1;
	double maxPeriod = 1 / 20.0;

	double tolerance = 2;

	int testHarmonics = 10;
	double hostileTolerance = freq2 / fftSize;

	double tuning = 440.0;
	double notes = 12;

	list<double> factors1;
	map<double, double> frequencies; // <frequency, error>
	map<double, pair<double, double>> harmonics; // <harmonic, <frequency, factor>
	multimap<double, pair<double, double>> hostileHarmonics; // <harmonic, <original frequency, factor>

	cerr << "Hostile Harmonic Tolerance: " << hostileTolerance << " hz" << endl;

	for (int i = 0; i < fftSize / 2.0; i++) {
		double nyquistFrequency = 0.5 * freq1;
		double factor = i / (fftSize / 2.0) * nyquistFrequency;
		factors1.push_back(factor);
	}

	std::cout.precision(10);

	for (int i = 0; i < fftSize / 2.0; i++) {
		double nyquistFrequency = 0.5 * freq2;
		double factor = i / (fftSize / 2.0) * nyquistFrequency;

		for (double efactor : factors1) {
			if (fabs(efactor - factor) < tolerance) {
				double average = (factor + efactor) / 2;
				frequencies.emplace(average, fabs(efactor - factor));

				// Add harmonics
				for (int j = 2; j < testHarmonics + 2; j++) {
					harmonics.emplace(j * average, make_pair(average, j));
				}

				// Find hostile harmonics
				for (pair<double, pair<double, double> > harmonic : harmonics) {
					if (fabs(harmonic.first - average) < hostileTolerance) {
						hostileHarmonics.emplace(average, harmonic.second);
					}
				}
			}
		}
	}

	for (pair<double, double> freq : frequencies) {
		// Calculate note
		double note = notes * log2(freq.first / tuning);
		double noteFrequency = tuning*pow(2, round(note) / notes);

		cout << "Found factor " << fixed << setprecision(5) << setw(12)
			<< freq.first
			<< " hz for frequency "
			<< "(±" << setw(5) << setprecision(3) << freq.second << ")"
			<< " [" << setw(2) << hostileHarmonics.count(freq.first) << " hostile harmonics" << "]"
			<< " ♪" << setw(5) << setprecision(2) << note - round(note) << "±" << setw(7) << setprecision(3) << setfill('0') << fabs(freq.first - noteFrequency) << setfill(' ');
		if (fabs(freq.first - noteFrequency) < hostileTolerance) {
			// Announce when a note is too close
			cout << "!!!";
		}
		cout << endl;
	}

	cout << "\n\n" << "Hostile harmonics:\n";
	for (pair<double, pair<double, double> > harmonic : hostileHarmonics) {
		cout << setprecision(3) << setw(9) << harmonic.first << " Hz = "
			<< setprecision(0) << setw(2) << harmonic.second.second
			<< " * " << setprecision(3) << setw(9) << harmonic.second.first
			<< " (±" << setw(6) << setprecision(3)
			<< fabs(harmonic.first - harmonic.second.second * harmonic.second.first) << ")"
			<< endl;
	}
}
