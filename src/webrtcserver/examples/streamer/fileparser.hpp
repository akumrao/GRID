
#ifndef fileparser_hpp
#define fileparser_hpp

#include <string>
#include <vector>
#include "stream.hpp"

class FileParser: public StreamSource {
    std::string directory;
    std::string extension;
    uint64_t sampleDuration_us;
    uint64_t sampleTime_us = 0;
    uint32_t counter = -1;
    bool loop;
    uint64_t loopTimestampOffset = 0;
protected:
    rtc::binary sample = {};
public:
    FileParser(std::string directory, std::string extension, uint32_t samplesPerSecond, bool loop);
    virtual ~FileParser();
    virtual void start() override;
    virtual void stop() override;
    virtual void loadNextSample() override;

    rtc::binary getSample() override;
    uint64_t getSampleTime_us() override;
    uint64_t getSampleDuration_us() override;
};

#endif /* fileparser_hpp */
