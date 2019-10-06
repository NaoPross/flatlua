#pragma once
#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <cstddef>
#include <string>

namespace build {

class resource {
public:
    resource(const char *start, const char *end): m_data(start),
                                                  m_size(end - start)
    {}

    const char * const &data() const { return m_data; }
    const size_t &size() const { return m_size; }

    const char *begin() const { return m_data; }
    const char *end() const { return m_data + m_size; }

    std::string str() const {
        return std::string(m_data, m_size);
    }

private:
    const char *m_data;
    size_t m_size;
};

}

#define LOAD_RESOURCE(x) ([]() {                                            \
        extern const char _binary_##x##_start, _binary_##x##_end;           \
        return build::resource(&_binary_##x##_start, &_binary_##x##_end);          \
    })()

#endif
