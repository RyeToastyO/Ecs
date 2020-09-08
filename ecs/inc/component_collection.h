// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "component.h"

#include <vector>

namespace ecs {
namespace impl {

struct IComponentCollection {
    virtual ~IComponentCollection () {}

    template<typename T>
    T* Get (uint32_t index);

    virtual uint32_t Allocate () = 0;
    virtual void CopyTo (uint32_t from, uint32_t to) = 0;
    virtual void MoveTo (uint32_t fromIndex, IComponentCollection& to, uint32_t toIndex) = 0;
    virtual void Remove (uint32_t index) = 0;
    virtual void RemoveAll () = 0;

protected:
    virtual void* GetComponentAtIndex (uint32_t index) = 0;
    virtual ComponentId GetComponentId () const = 0;
};

template<typename T>
struct TComponentCollection : IComponentCollection {
    uint32_t Allocate () override;
    void CopyTo (uint32_t from, uint32_t to) override;
    void MoveTo (uint32_t fromIndex, IComponentCollection& to, uint32_t toIndex) override;
    void Remove (uint32_t index) override;
    void RemoveAll () override;

protected:
    void* GetComponentAtIndex (uint32_t index) override;
    ComponentId GetComponentId () const override;

private:
    std::vector<T> m_components;
};

} // namespace impl
} // namespace ecs
