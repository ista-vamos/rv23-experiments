#ifndef OD_EVENTS_H_
#define OD_EVENTS_H_

#include <vamos-buffers/cpp/event.h>

using vamos::Event;

enum class Kind : vms_kind {
    End = Event::doneKind(),
    InputL = Event::firstValidKind(),
    OutputL,
    Dummy
};

inline const char *kindToStr(Kind k) {
    switch (k) {
        case Kind::InputL:
            return "InputL";
        case Kind::OutputL:
            return "OutputL";
        case Kind::Dummy:
            return "Dummy";
        case Kind::End:
            return "END";
    }
    abort();
}

struct TraceEvent : Event {
    union {
        struct {
            uint64_t value;
        } InputL;
        struct {
            uint64_t value;
        } OutputL;
        struct {
            uint64_t value;
        } Dummy;
    } data;

    TraceEvent() = default;
    TraceEvent(Kind k, vms_eventid id) : Event((vms_kind)k, id) {}
    TraceEvent(vms_kind k, vms_eventid id) : Event(k, id) {}

    bool operator==(const TraceEvent &rhs) const {
        return get_kind() == rhs.get_kind() &&
               (get_kind() == Event::doneKind() ||
                (data.Dummy.value == rhs.data.Dummy.value));
    }

    bool operator!=(const TraceEvent &rhs) const { return !operator==(rhs); }
};

struct Event_InputL : public TraceEvent {
    Event_InputL() = default;
    Event_InputL(vms_eventid id, uint64_t value)
        : TraceEvent(Kind::InputL, id) {
        data.InputL.value = value;
    }

    auto value() const { return data.InputL.value; }
};

struct Event_OutputL : public TraceEvent {
    Event_OutputL() = default;
    Event_OutputL(vms_eventid id, uint64_t value)
        : TraceEvent(Kind::OutputL, id) {
        data.OutputL.value = value;
    }

    auto value() const { return data.OutputL.value; }
};

struct Event_Dummy : public TraceEvent {
    Event_Dummy() = default;
    Event_Dummy(vms_eventid id, uint64_t value) : TraceEvent(Kind::Dummy, id) {
        data.Dummy.value = value;
    }

    auto value() const { return data.Dummy.value; }
};

#define DBG
#ifdef DBG
#include <iostream>
std::ostream &operator<<(std::ostream &s, const TraceEvent &ev);
#endif

#endif
