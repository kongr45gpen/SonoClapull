#ifndef SONOCLAPULL_TONELOCATOR_H
#define SONOCLAPULL_TONELOCATOR_H

#include "MediaDecoder.h"

class ToneLocator {
    MediaDecoder decoder;

    static constexpr float CLOCK_FREQ1 = 1077.39258f;
    static constexpr float CLOCK_FREQ2 = 1593.60352f;
    /**
     * These frequencies should be the same as the ones used
     * in the corresponding version of the SonoClapper app
     */
    static constexpr float DATA_FREQS[] = { 1077.39258f,
                                            3187.20703f,
                                            3703.41797f,
                                            5297.02148f,
                                            6890.62500f,
                                            8484.22852f,
                                           10077.83203f,
                                           13781.25000f};
    static constexpr float DATA_FREQ0 = 823; // not used

    unsigned int fftSize = 1024;

    std::shared_ptr<std::vector<float > > timeData;
    std::vector<float> frequencyData;

    MediaDecoder::time clock1time;
    MediaDecoder::time clock2time;
    MediaDecoder::time clockPeriod;

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
    std::vector<uint8_t> getData();
};


#endif //SONOCLAPULL_TONELOCATOR_H
