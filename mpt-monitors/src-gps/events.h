#ifndef OD_EVENTS_H_
#define OD_EVENTS_H_

#include <vamos-buffers/cpp/event.h>

using vamos::Event;

enum class Kind : vms_kind {
    End = Event::doneKind(),
    InputL = Event::firstValidKind(),
    OutputL,
    Other
};

inline const char *kindToStr(Kind k) {
    switch (k) {
        case Kind::InputL:
            return "InputL";
        case Kind::OutputL:
            return "OutputL";
        case Kind::Other:
            return "Other";
        case Kind::End:
            return "END";
    }
}

struct TraceEvent : Event {
    union {
        struct {
            float lat;
            float lng;
        } InputL;
        struct {
            float lat;
            float lng;
        } OutputL;
        struct {
            float lat;
            float lng;
        } Other;
    } data;

    TraceEvent() = default;
    TraceEvent(Kind k, vms_eventid id) : Event((vms_kind)k, id) {}
    TraceEvent(vms_kind k, vms_eventid id) : Event(k, id) {}

    bool operator==(const TraceEvent &rhs) const {
        return get_kind() == rhs.get_kind() &&
               (get_kind() == Event::doneKind() ||
                (data.InputL.lat == rhs.data.InputL.lat &&
                 data.InputL.lng == rhs.data.InputL.lng));
    }

    bool operator!=(const TraceEvent &rhs) const { return !operator==(rhs); }
};

struct Event_InputL : public TraceEvent {
    Event_InputL() = default;
    Event_InputL(vms_eventid id, float lat, float lng)
        : TraceEvent(Kind::InputL, id) {
        data.InputL.lat = lat;
        data.InputL.lng = lng;
    }
};

struct Event_OutputL : public TraceEvent {
    Event_OutputL() = default;
    Event_OutputL(vms_eventid id, float lat, float lng)
        : TraceEvent(Kind::OutputL, id) {
        data.OutputL.lat = lat;
        data.OutputL.lng = lng;
    }
};

struct Event_Other : public TraceEvent {
    Event_Other() = default;
    Event_Other(vms_eventid id, float lat, float lng)
        : TraceEvent(Kind::Other, id) {
        data.Other.lat = lat;
        data.Other.lng = lng;
    }
};

#define DBG
#ifdef DBG
#include <iostream>
std::ostream &operator<<(std::ostream &s, const TraceEvent &ev);
#endif

#endif
