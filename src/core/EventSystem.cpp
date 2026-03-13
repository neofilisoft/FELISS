#include "EventSystem.h"
#include <algorithm>

namespace Feliss {

void EventSystem::unsubscribe(ListenerID lid) {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (auto& [idx, vec] : m_listeners) {
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [lid](const Entry& e){ return e.lid == lid; }), vec.end());
    }
}

void EventSystem::flush() {
    std::vector<Queued> batch;
    {
        std::lock_guard<std::mutex> lk(m_qMutex);
        while (!m_queue.empty()) {
            batch.push_back(std::move(m_queue.front()));
            m_queue.pop();
        }
    }
    for (auto& q : batch) {
        std::vector<Entry> copy;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_listeners.find(q.idx);
            if (it == m_listeners.end()) continue;
            copy = it->second;
        }
        for (auto& e : copy) e.fn(q.ev);
    }
}

void EventSystem::clear() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_listeners.clear();
}

size_t EventSystem::listenerCount() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    size_t n = 0;
    for (auto& [k, v] : m_listeners) n += v.size();
    return n;
}

} // namespace Feliss
