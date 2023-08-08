#ifndef OD_WORKBAG_H
#define OD_WORKBAG_H

#include <vector>

#include "cfgset.h"
// #include <vamos-buffers/cpp/vector.h>

class Workbag {
    std::vector<ConfigurationsSet<3>> _queue;

   public:
    auto size() -> auto{ return _queue.size(); }
    auto empty() -> auto{ return _queue.empty(); }
    auto clear() -> auto{ return _queue.clear(); }
    auto swap(Workbag &rhs) -> auto{ return _queue.swap(rhs._queue); }

    auto push(ConfigurationsSet<3> &&C) -> auto{
        // return _queue.push(std::move(C));
        return _queue.push_back(std::move(C));
    }

    auto begin() -> auto{ return _queue.begin(); }
    auto end() -> auto{ return _queue.end(); }
    auto begin() const -> auto{ return _queue.begin(); }
    auto end() const -> auto{ return _queue.end(); }
};

#endif  // OD_WORKBAG_H
