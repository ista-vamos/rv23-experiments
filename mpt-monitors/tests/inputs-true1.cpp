#include <cassert>

#include "events.h"
#include "monitor.h"

int x;

#define NUM_STREAMS 5
#define NUM_EVS 6
TraceEvent streams[][NUM_EVS] = {{
                                     Event_InputL(1, 1),
                                     Event_InputL(2, 2),
                                     Event_InputL(3, 3),
                                     Event_OutputL(1, 1),
                                     Event_OutputL(2, 2),
                                     Event_OutputL(3, 3),
                                 },
                                 {
                                     Event_InputL(1, 1),
                                     Event_InputL(2, 2),
                                     Event_InputL(3, 3),
                                     Event_OutputL(1, 1),
                                     Event_OutputL(2, 2),
                                     Event_OutputL(3, 3),
                                 },
                                 {
                                     Event_InputL(1, 1),
                                     Event_InputL(3, 3),
                                     Event_InputL(2, 2),
                                     Event_OutputL(1, 1),
                                     Event_OutputL(3, 3),
                                     Event_OutputL(2, 2),
                                 },
                                 {
                                     Event_InputL(1, 1),
                                     Event_InputL(3, 3),
                                     Event_InputL(2, 2),
                                     Event_OutputL(1, 1),
                                     Event_OutputL(3, 3),
                                     Event_OutputL(2, 2),
                                 },
                                 {
                                     Event_InputL(1, 1),
                                     Event_InputL(3, 3),
                                     Event_InputL(2, 2),
                                     Event_OutputL(1, 1),
                                     Event_OutputL(3, 3),
                                     Event_OutputL(2, 2),
                                 }};

bool InputStream::hasEvent() const {
    const size_t &pos = reinterpret_cast<const size_t &>(data[1]);
    return pos < NUM_EVS;
}

bool InputStream::isDone() const {
    const size_t &pos = reinterpret_cast<const size_t &>(data[1]);
    return pos >= NUM_EVS;
}

Event *InputStream::getEvent() {
    assert(hasEvent() && "getEvent() when there is no event");

    TraceEvent *events = static_cast<TraceEvent *>(data[0]);
    size_t &pos = reinterpret_cast<size_t &>(data[1]);

    return &events[pos++];
}

Inputs::Inputs() {
    size_t &returned = reinterpret_cast<size_t &>(data[0]);
    returned = 0;
}

bool Inputs::done() const {
    const size_t &returned = reinterpret_cast<const size_t &>(data[0]);
    return returned >= NUM_STREAMS;
}

InputStream *Inputs::getNewInputStream() {
    size_t &returned = reinterpret_cast<size_t &>(data[0]);
    if (returned >= NUM_STREAMS)
        return nullptr;

    auto *stream = new InputStream(_streams.size());
    _streams.emplace_back(stream);

    *reinterpret_cast<TraceEvent **>(&stream->data[0]) = streams[returned];
    *reinterpret_cast<size_t *>(&stream->data[1]) = 0;

    ++returned;

    return stream;
}
