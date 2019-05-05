/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../composition.h"

namespace ecs {
namespace impl {

inline size_t Composition::GetHash () const {
    return m_flags.GetHash();
}

inline bool Composition::operator== (const Composition & rhs) const {
    if (m_shared.size() != rhs.m_shared.size())
        return false;
    if (!(m_flags == rhs.m_flags))
        return false;

    auto iterLhs = m_shared.begin();
    auto iterRhs = rhs.m_shared.begin();
    while (iterLhs != m_shared.end()) {
        if (!(iterLhs->second == iterRhs->second))
            return false;
        ++iterLhs;
        ++iterRhs;
    }

    return true;
}

} // namespace impl
} // namespace ecs
