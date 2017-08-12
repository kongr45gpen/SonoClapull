#ifndef SONOCLAPULL_TONELOCATOR_H
#define SONOCLAPULL_TONELOCATOR_H

#include "MediaDecoder.h"

class ToneLocator {
    MediaDecoder decoder;

    static constexpr float CLOCK_FREQ1 = 1077.39258f;
    static constexpr float CLOCK_FREQ2 = 1593.60352f;
    static float  DATA_FREQ0;
    static float  DATA_FREQ1;
    static float  DATA_FREQ2;
    static float  DATA_FREQ3;
    static float  DATA_FREQ4;
    static float  DATA_FREQ5;
    static float  DATA_FREQ6;
    static float  DATA_FREQ7;

    unsigned int fftSize = 1024;

    std::shared_ptr<std::vector<float > > timeData;
    std::vector<float> frequencyData;

    /**
     * @todo This returns the mean. See if it's better to
     *       return the median.
     */
    float getFrequencyMedian();
    /**
     * The threshold calculated using the median for each
     * FFT frame
     */
    float threshold;

    /**
     * Fill the frequencyData array with the FFT amplitudes of timeData
     */
    inline void performTransform();

    bool containsFrequency(float hertz);

    /**
     * Get the next FFT frame
     */
    void nextFrame();
public:
    ToneLocator(const std::string &filename);
    void locateClock();
};


#endif //SONOCLAPULL_TONELOCATOR_H
