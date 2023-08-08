#include <cassert>
#include <fstream>

#include "events.h"
#include "monitor.h"

int x;

bool InputStream::hasEvent() const { return !isDone(); }

bool InputStream::isDone() const {
    size_t sending_output = reinterpret_cast<size_t>(data[2]);
    return sending_output == 2;
}

Event *InputStream::getEvent() {
    assert(hasEvent() && "getEvent() when there is no event");

    std::ifstream *stream = reinterpret_cast<std::ifstream *>(data[0]);
    float lat, lng;
    static float llat, llng;

    static Event_InputL I(0, 0, 0);
    static Event_OutputL O(0, 0, 0);
    static Event_Other oth(0, 0, 0);

    size_t &pos = reinterpret_cast<size_t &>(data[1]);
    size_t &sending = reinterpret_cast<size_t &>(data[2]);

    assert(sending <= 1);
    assert(stream->good());

    if (!(*stream >> lat >> lng)) {
        assert(sending <= 1);
        sending = 2;
        O = Event_OutputL(++pos, llat, llng);
        return &O;
    }

    // std::cout << lat << " " << lng << "\n";

    if (sending == 0) {
        sending = 1;
        I = Event_InputL(++pos, lat, lng);
        return &I;
    }

    llat = lat;
    llng = lng;
    oth = Event_Other(++pos, lat, lng);
    return &oth;
}

Inputs::Inputs(char **files) {
    *reinterpret_cast<char ***>(&data[2]) = files;
    assert(reinterpret_cast<char **>(data[2]) == files);
}

InputStream *Inputs::getNewInputStream() {
    auto *files = reinterpret_cast<char **>(data[2]);
    if (*files == nullptr)
        return nullptr;

    // std::cout << "New stream: " << *files <<"\n";
    auto *fstream = new std::ifstream(*files);
    assert(fstream->good());
    *reinterpret_cast<char ***>(&data[2]) = files + 1;

    auto *stream = new InputStream(_streams.size());
    _streams.emplace_back(stream);
    *reinterpret_cast<std::ifstream **>(&stream->data[0]) = fstream;
    reinterpret_cast<size_t &>(stream->data[1]) = 0;
    *reinterpret_cast<size_t *>(&stream->data[2]) = 0;

    return stream;
}

bool Inputs::done() const {
    auto *files = reinterpret_cast<char **>(data[2]);
    return *files == nullptr;
}
