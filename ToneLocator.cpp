#include <iostream>
#include <numeric>
#include <kiss_fftr.h>
#include "ToneLocator.h"

ToneLocator::ToneLocator(const std::string & filename) : decoder(filename, 1024) {
    std::cout << "Hello!" << std::endl;
}

void ToneLocator::locateClock() {
    while (true) {
        nextFrame();
        if (containsFrequency(CLOCK_FREQ1)) {
            break;
        }
    }
    std::cout << "found frequency 1" << std::endl;

    while (true) {
        nextFrame();
        if (containsFrequency(CLOCK_FREQ2)) {
            break;
        }
    }
    std::cout << "found frequency 2" << std::endl;
}

float ToneLocator::getFrequencyMedian() {
    if (frequencyData.size() < 2) {
        return 0;
    }

    // sum/size
    // (Do not account for DC term)

    return std::accumulate(frequencyData.begin() + 1, frequencyData.end(), 0.0f)
           / (float) (frequencyData.size() - 1);
}

void ToneLocator::nextFrame() {
    timeData = decoder.getNextSamples();
    performTransform();
    threshold = 10.0f * getFrequencyMedian();
}

void ToneLocator::performTransform() {
    if (!timeData) {
        throw std::runtime_error("Asked to perform transform on non-existent data");
    }
    if (fftSize > timeData->size()) {
        throw std::runtime_error("Not enough time data to perform FFT");
    }

    kiss_fft_cpx fft[fftSize/2];

    // Perform the actual calculation
    kiss_fftr_cfg cfg = kiss_fftr_alloc(fftSize, 0, nullptr, nullptr);
    kiss_fftr(cfg, timeData->data(), fft);

    // Store output
    frequencyData.resize(fftSize/2); // No need to reallocate vector, just resize it
    for (int i = 0 ; i < 512 ; i++) {
        // Perform the calculation in double precision and store the result as float
        // to prevent very large numbers from showing up as infinity
        frequencyData[i] = static_cast<float>(sqrt(pow(fft[i].r, 2) + pow(fft[i].i, 2)));
        if (static_cast<bool>(isnanf(frequencyData[i]))) {
            frequencyData[i] = 0.0f;
        }
        // TODO: Throw a warning if an infinity is found
    }
}

bool ToneLocator::containsFrequency(float hertz) {
    static float nyquistFrequency = decoder.getSampleRate() / 2.0f;
    float total = frequencyData.size();

    // The non-integer representation of the frequency's index in the output array
    float decimalIndex = total * hertz / nyquistFrequency;

    if (decimalIndex > total) {
        throw std::runtime_error("Frequency too high");
    }

    // The index of the first frequency column
    float preciseIndex = floorf(decimalIndex);
    auto index = static_cast<int>(preciseIndex);

    // The factors for the first and the second columns
    float b = decimalIndex - preciseIndex;
    float a = 1 - b;

    // The calculated frequency amplitude is a weighted
    // average of the values of the two columns
    float frequencyAmplitude = a * frequencyData[index] + b * frequencyData[index + 1];

    std::cout << "  (for " << hertz << " Hz, NyQ=" << nyquistFrequency << ", fftSiz=" << total << ", dcmIdx#"
                                                                                             << decimalIndex
                                                                                             << ")\n";
    std::cout << "Freq " << a << "*" << "ff[" << index << "] + " << b << " * " << "ff[" << index+1 << "]";
    std::cout << " = " << frequencyAmplitude << " <> thres=" << threshold << "\n";

    return frequencyAmplitude >= threshold;
}
