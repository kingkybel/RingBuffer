

#include "ring_buffer.h"

#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Debug method to print buffer contents (for debugging only)
 */
template <typename T, typename Allocator> inline std::string to_string(dkyb::ring_buffer<T, Allocator> const& rb)
{
    std::ostringstream oss;
    oss << "Buffer contents: ";
    for (typename ring_buffer<T, Allocator>::size_type i = 0; i < rb.capacity(); ++i)
    {
        oss << rb[i] << " ";
    }
    oss << " | head=" << rb.head() << " tail=" << rb.tail() << " count=" << rb.size() << " full=" << rb.full()
        << " just_overwrote=" << rb.just_overwrote();
    return oss.str();
}
