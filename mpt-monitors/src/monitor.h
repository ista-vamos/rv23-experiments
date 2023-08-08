#ifndef MONITOR_OD_H
#define MONITOR_OD_H

#include <vamos-buffers/cpp/event.h>

#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <vector>

using vamos::Event;

class TraceBase {
    std::string _name;
    const size_t _id;
    bool _done{false};

   public:
    TraceBase(size_t id) : _id(id) {}
    void setDone() { _done = true; }

    bool done() const { return _done; }
    size_t id() const { return _id; }

    void set_name(const std::string &n) { _name = n; }
    const std::string &name() const { return _name; }
    std::string descr() const {
        if (_name.empty()) {
            return std::to_string(_id);
        }
        return std::to_string(_id) + ": " + _name;
    }
};

template <typename EventTy>
class Trace : public TraceBase {
    std::vector<EventTy> _events;
    // we cache size of the vector, it has acually quite nice
    // impact on the runtime as we query the size all the time
    // (in canProceed())
    size_t _size{0};

   public:
    Trace(size_t id) : TraceBase(id) {}

    void append(const EventTy *e) {
        ++_size;
        _events.push_back(*e);
    }
    void append(const EventTy &e) {
        ++_size;
        _events.push_back(e);
    };

    Event *get(size_t idx) { return &_events[idx]; }
    const Event *get(size_t idx) const { return &_events[idx]; }

    Event *operator[](size_t idx) { return get(idx); }
    const Event *operator[](size_t idx) const { return get(idx); }

    size_t size() const { return _size; }
};

class Inputs;

///
/// \brief The InputStream class
/// Stream of events that are input to the monitor.
/// This is an opaque object, basically an iterator over incoming events.
/// The incoming events are stored into the trace that is referenced in
/// InputStream.
class InputStream {
    const std::string _name{};

   protected:
    const size_t _id;
    TraceBase *_trace;
    // this is ugly but efficient, we'll reinterpret cast the data into what we
    // need
    void *data[4];

    friend class Inputs;

   public:
    InputStream(size_t id) : _id(id) {}
    InputStream(size_t id, const std::string &name) : _name(name), _id(id) {}

    size_t id() const { return _id; }
    void setTrace(TraceBase *t) {
        _trace = t;
        if (!_name.empty())
            _trace->set_name(_name);
    }

    TraceBase *trace() { return _trace; }
    const TraceBase *trace() const { return _trace; }

    Event *getEvent();
    const Event *getEvent() const;

    bool hasEvent() const;
    bool isDone() const;
};

class Inputs {
    std::vector<std::unique_ptr<InputStream>> _streams;

    // this is ugly but efficient, we'll reinterpret cast the data into what we
    // need
    void *data[4];

   public:
    Inputs();
    Inputs(char *files[], size_t);

    InputStream *getNewInputStream();
    bool done() const;
};

class MString {
    static const size_t ARRAY_SIZE = 1;

   public:
    struct Letter {
        static const size_t BOT = ~static_cast<size_t>(0);

        size_t start;
        size_t end;
        Letter() = default;
        Letter(size_t s, size_t e) : start(s), end(e) {}

        bool operator==(const Letter &rhs) const {
            return start == rhs.start && end == rhs.end;
        }
        bool operator!=(const Letter &rhs) const { return !(*this == rhs); }
    };

    void append(const MString::Letter &l);
    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }

    bool operator==(const MString &rhs) const {
        if (_size != rhs._size)
            return false;

        if (_size <= ARRAY_SIZE) {
            for (size_t i = 0; i < _size; ++i) {
                if (_data.arr[i] != rhs._data.arr[i]) {
                    return false;
                }
            }
            return true;
        }

        return _data.vec == rhs._data.vec;
    }
    bool operator!=(const MString &rhs) const { return !operator==(rhs); }

    Letter &operator[](size_t idx) {
        if (_size <= ARRAY_SIZE) {
            return _data.arr[idx];
        }
        return _data.vec[idx];
    }
    Letter operator[](size_t idx) const {
        if (_size <= ARRAY_SIZE) {
            return _data.arr[idx];
        }
        return _data.vec[idx];
    }

    /*
    auto begin() -> auto { return _data.begin(); }
    auto end() -> auto { return _data.end(); }
    auto begin() const -> auto { return _data.begin(); }
    auto end() const -> auto { return _data.end(); }
    */

    MString() : _size(0) {}
    ~MString() {
        if (_size > ARRAY_SIZE) {
            // destroy the vector
            _data.vec.~vector();
        }
    }
    MString(const MString &rhs) : _size(rhs._size) {
        if (_size <= ARRAY_SIZE) {
            memcpy(_data.arr, rhs._data.arr, _size * sizeof(Letter));
        } else {
            _data.vec = rhs._data.vec;
        }
    }

    /*
    MString &operator=(const MString &rhs) {
      _size = rhs._size;
      if (_size <= ARRAY_SIZE) {
        memcpy(_data.arr, rhs._data.arr, _size * sizeof(Letter));
      } else {
        _data.vec = rhs._data.vec;
      }

      return *this;
    }
    */

    MString &operator=(MString &&rhs) {
        _size = rhs._size;
        if (_size <= ARRAY_SIZE) {
            memcpy(_data.arr, rhs._data.arr, _size * sizeof(Letter));
        } else {
            _data.vec = std::move(rhs._data.vec);
        }

        return *this;
    }

    Letter &back() {
        if (_size <= ARRAY_SIZE) {
            return _data.arr[_size - 1];
        }
        return _data.vec.back();
    }

   private:
    size_t _size{0};

    static_assert(ARRAY_SIZE * sizeof(Letter) <= sizeof(std::vector<Letter>),
                  "Array is bigger than vec");
    union DataTy {
        Letter arr[ARRAY_SIZE];
        std::vector<Letter> vec;

        DataTy() {}
        ~DataTy() {}
    } _data;

    friend std::ostream &operator<<(std::ostream &s, const MString &ev);
};

template <size_t ARRAY_SIZE>
class FixedMString {
   public:
    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }

    void append(const MString::Letter &l) {
        if (_size == 0) {
            assert(l.start != MString::Letter::BOT);
            _data[0] = l;
            ++_size;
            return;
        }

        auto &last = back();
        if (last.end == MString::Letter::BOT) {
            assert(last.end != MString::Letter::BOT);
            assert(l.start != MString::Letter::BOT);
            last.start = l.start;
        } else {
            assert(last.start != MString::Letter::BOT);
            assert(l.start != MString::Letter::BOT);
            assert(_size < ARRAY_SIZE && "OOB write");
            _data[_size++] = l;
        }
    }

    bool operator==(const FixedMString &rhs) const {
        if (_size != rhs._size)
            return false;

        for (size_t i = 0; i < _size; ++i) {
            if (_data[i] != rhs._data[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const FixedMString &rhs) const { return !operator==(rhs); }

    MString::Letter &operator[](size_t idx) { return _data[idx]; }
    MString::Letter operator[](size_t idx) const { return _data[idx]; }

    FixedMString() {}
    FixedMString(const FixedMString &rhs) : _size(rhs._size) {
        memcpy(_data, rhs._data, _size * sizeof(MString::Letter));
    }

    FixedMString &operator=(FixedMString &&rhs) {
        _size = rhs._size;
        memcpy(_data, rhs._data, _size * sizeof(MString::Letter));
        return *this;
    }

    MString::Letter &back() {
        assert(_size > 0);
        return _data[_size - 1];
    }

   private:
    size_t _size{0};
    MString::Letter _data[ARRAY_SIZE];

    // friend std::ostream &operator<<(std::ostream &s, const MString &ev);
};

#define DBG
#ifdef DBG
#include <iostream>
std::ostream &operator<<(std::ostream &s, const MString &ev);
#endif

// MString in this prefix expression is just a single position
struct PrefixExpression {
#ifndef NDEBUG
    size_t state{0};
#endif
    size_t match_pos{0};
};

#endif  // MONITOR_OD_H
