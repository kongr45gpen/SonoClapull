#include <iostream>
#include <numeric>
#include <kiss_fftr.h>
#include "ToneLocator.h"
#include "FileEndException.h"

constexpr float ToneLocator::DATA_FREQS[];

ToneLocator::ToneLocator(const std::string & filename) : decoder(filename, 1024) {
    std::cout << "Hello!" << std::endl;
}

void ToneLocator::locateClock() {
    fftSize = 1024; // use 1024 bit precision for locating the clock
    decoder.setSampleCount(fftSize);
    decoder.setOverlapSamples(10); // TODO: Set to a higher value if this is too slow
    // TODO: Perform a more precise calculation if and when needed

    try {
        while (true) {
            nextFrame();
            if (containsFrequency(CLOCK_FREQ1)) {
                break;
            }
        }

        // While the clock times may not represent the absolute starting point of
        // the tone, they are precise enough, since we only need to work relatively
        clock1time = decoder.getSampleTime(0);
        std::cout << "Clock1 found at " << decoder.getSampleTime(0) << std::endl;

        while (true) {
            nextFrame();
            if (containsFrequency(CLOCK_FREQ2)) {
                break;
            }
        }
        clock2time = decoder.getSampleTime(0);
        std::cout << "Clock2 found at " << decoder.getSampleTime(0) << std::endl;
    } catch (FileEndException &e) {
        throw FileProcessException("Could not find clock in file: " + e.what());
    }

    clockPeriod = clock2time - clock1time;
}

std::vector<uint8_t> ToneLocator::getData() {
    std::vector<uint8_t> data(4);

    decoder.setOverlapZero();

    auto clockSamples = static_cast<unsigned int>(floor(clockPeriod * decoder.getSampleRate()));

    // The number of samples that we can trust have sound in them
    // TODO: Use substraction instead of multiplication, based on SonoClapper app?
    auto usableSamples = static_cast<unsigned int>(clockSamples * 0.75);
    // Calculate FFT size for maximum precision
    // Use a power of 2 so that the FFT is faster
    fftSize = static_cast<unsigned int>(pow(2, floor(log2(usableSamples))));

    std::cout << "Clock samples: " << clockSamples << " -> " << fftSize << std::endl;

    // Skip the next clocks
    decoder.setSampleCount(clockSamples);
    decoder.getNextSamples();
    decoder.getNextSamples();

    for (int i = 0; i < 4; i++) {
        // TODO: Implement sample skipping as a separate function
        decoder.setSampleCount(static_cast<int>(fftSize));
        nextFrame();

        uint8_t datum = 0;
        for (int j = 0; j < 8; j++) {
            if (containsFrequency(ToneLocator::DATA_FREQS[j])) {
                datum += (1 << j);
            }
        }
        std::cout << "Found data for [" << i << "]: " << (int) datum << std::endl;
        data[i] = datum;

        if (i != 3) {
            // Skip
            decoder.setSampleCount(static_cast<int>(clockSamples-fftSize));
            decoder.getNextSamples();
        }
    }

    return data;
}

float ToneLocator::getFrequencyMedian() {
    if (frequencyData.size() < 2) {
        return 0;
    }

    // sum/size
    // (Do not account for DC term)
    // TODO: Get sum for frequencies up to 15 kHz

    return std::accumulate(frequencyData.begin() + 1, frequencyData.end(), 0.0f)
           / (float) (frequencyData.size() - 1);
}

void ToneLocator::nextFrame() {
    timeData = decoder.getNextSamples();
    performTransform();
    threshold = 15.0f * getFrequencyMedian();
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
    for (int i = 0 ; i < fftSize/2 ; i++) {
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

//    std::cout << "Freq " << a << "*" << "ff[" << index << "] + " << b << " * " << "ff[" << index+1 << "]";
//    std::cout << " = " << frequencyAmplitude << " <> thres=" << threshold << "\n";

    return frequencyAmplitude >= threshold;
}
