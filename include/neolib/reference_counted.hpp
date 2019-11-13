// reference_counted.hpp - v1.0
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <vector>
#include <functional>
#include "i_discoverable.hpp"

namespace neolib
{
    template <typename Base, bool DeallocateOnRelease = true>
    class reference_counted : public Base
    {
        typedef Base base_type;
    public:
        using typename base_type::release_during_destruction;
        using typename base_type::too_many_references;
        using typename base_type::destruction_watcher_already_subscribed;
        using typename base_type::destruction_watcher_not_found;
    public:
        using typename base_type::i_object_destruction_watcher;
    public:
        reference_counted() : iReferenceCount(0), iPinned(false), iDestroying(false)
        {
        }
        reference_counted(const reference_counted& aOther) : iReferenceCount(0), iPinned(aOther.iPinned), iDestroying(false)
        {
        }
        virtual ~reference_counted()
        {
            iDestroying = true;
            for (auto i = iDestructionWatchers.begin(); i != iDestructionWatchers.end(); ++i)
                if (*i != 0)
                    (*i)->object_being_destroyed(*this);
        }
        reference_counted& operator=(const reference_counted&)
        {
            // do nothing
            return *this;
        }
    public:
        virtual void add_ref() const
        {
            ++iReferenceCount;
        }
        virtual void release() const
        {
            if (--iReferenceCount <= 0 && !iPinned)
            {
                if (!iDestroying)
                    destroy();
                else
                    throw release_during_destruction();
            }
        }
        virtual const base_type* release_and_take_ownership() const
        {
            if (iReferenceCount != 1)
                throw too_many_references();
            iReferenceCount = 0;
            return this;
        }
        virtual base_type* release_and_take_ownership()
        {
            return const_cast<base_type*>(to_const(*this).release_and_take_ownership());
        }
        virtual void pin() const
        {
            iPinned = true;
        }
        virtual void unpin() const
        {
            iPinned = false;
            if (iReferenceCount <= 0)
                destroy();
        }
    public:
        virtual void subcribe_destruction_watcher(i_object_destruction_watcher& aWatcher) const
        {
            auto existingWatcher = std::find(iDestructionWatchers.begin(), iDestructionWatchers.end(), &aWatcher);
            if (existingWatcher != iDestructionWatchers.end())
                throw destruction_watcher_already_subscribed();
            iDestructionWatchers.push_back(&aWatcher);
        }
        virtual void unsubcribe_destruction_watcher(i_object_destruction_watcher& aWatcher) const
        {
            auto existingWatcher = std::find(iDestructionWatchers.begin(), iDestructionWatchers.end(), &aWatcher);
            if (existingWatcher == iDestructionWatchers.end())
                throw destruction_watcher_not_found();
            if (!iDestroying)
                iDestructionWatchers.erase(existingWatcher);
            else
                *existingWatcher = 0;
        }
    private:
        void destroy() const
        {
            if constexpr (DeallocateOnRelease)
                delete this;
            else
                (*this).~reference_counted();
        }
    private:
        mutable int32_t iReferenceCount;
        mutable bool iPinned;
        bool iDestroying;
        mutable std::vector<i_object_destruction_watcher*> iDestructionWatchers;
    };

    template <typename Interface>
    class ref_ptr : public i_ref_ptr<Interface>
    {
    public:
        typedef i_ref_ptr<Interface> abstract_type;
        typedef typename abstract_type::no_object no_object;
        typedef typename abstract_type::interface_not_found interface_not_found;
    public:
        ref_ptr(Interface* aObject = nullptr) :
            iObject(aObject), iReferenceCounted(true)
        {
            if (valid())
                iObject->add_ref();
        }
        ref_ptr(Interface& aObject) :
            iObject(&aObject), iReferenceCounted(false)
        {
        }
        ref_ptr(const ref_ptr& aOther) :
            iObject(aOther.ptr()), iReferenceCounted(aOther.reference_counted())
        {
            if (valid() && iReferenceCounted)
                iObject->add_ref();
        }
        ref_ptr(const abstract_type& aOther) :
            iObject(aOther.ptr()), iReferenceCounted(aOther.reference_counted())
        {
            if (valid() && iReferenceCounted)
                iObject->add_ref();
        }
        ref_ptr(i_discoverable& aDiscoverable) :
            iObject(nullptr), iReferenceCounted(true)
        {
            if (!aDiscoverable.discover(*this))
                throw interface_not_found();
        }
        template <typename Interface2>
        ref_ptr(const ref_ptr<Interface2>& aOther) :
            iObject(aOther.ptr()), iReferenceCounted(aOther.reference_counted())
        {
            if (valid() && iReferenceCounted)
                iObject->add_ref();
        }
        ~ref_ptr()
        {
            if (valid() && iReferenceCounted)
                iObject->release();
        }
        ref_ptr& operator=(const ref_ptr& aOther)
        {
            reset(aOther.ptr(), aOther.reference_counted());
            return *this;
        }
        ref_ptr& operator=(const abstract_type& aOther)
        {
            if (&aOther == this)
                return *this;
            reset(aOther.ptr(), aOther.reference_counted());
            return *this;
        }
        ref_ptr& operator=(nullptr_t)
        {
            reset();
            return *this;
        }
    public:
        bool reference_counted() const override
        {
            return iReferenceCounted;
        }
        void reset(Interface* aObject = nullptr, bool aReferenceCounted = true) override
        {
            ref_ptr copy(*this);
            if (valid() && iReferenceCounted)
                iObject->release();
            iObject = aObject;
            iReferenceCounted = aReferenceCounted;
            if (valid() && iReferenceCounted)
                iObject->add_ref();
        }
        Interface* release() override
        {
            if (iObject == nullptr)
                throw no_object();
            Interface* releasedObject = static_cast<Interface*>(iObject->release_and_take_ownership());
            iObject = nullptr;
            return releasedObject;
        }
        bool valid() const override
        {
            return iObject != nullptr;
        }
        Interface* ptr() const override
        {
            return iObject;
        }
        Interface* operator->() const override
        {
            if (iObject == nullptr)
                throw no_object();
            return iObject;
        }
        Interface& operator*() const override
        {
            if (iObject == nullptr)
                throw no_object();
            return *iObject;
        }
    private:
        Interface* iObject;
        bool iReferenceCounted;
    };

    template <typename Interface>
    class weak_ref_ptr : public i_weak_ref_ptr<Interface>
    {
    public:
        typedef i_weak_ref_ptr<Interface> base;
        typedef typename base::no_object no_object;
        typedef typename base::interface_not_found interface_not_found;
        typedef typename base::bad_release bad_release;
        typedef typename base::wrong_object wrong_object;
    public:
        weak_ref_ptr(Interface* aObject = nullptr) :
            iObject(aObject)
        {
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        weak_ref_ptr(Interface& aObject) :
            iObject(&aObject)
        {
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        weak_ref_ptr(const weak_ref_ptr& aOther) :
            iObject(aOther.ptr())
        {
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        weak_ref_ptr(const i_ref_ptr<Interface>& aOther) :
            iObject(aOther.ptr())
        {
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        weak_ref_ptr(i_discoverable& aDiscoverable) :
            iObject(nullptr)
        {
            if (!aDiscoverable.discover(*this))
                throw interface_not_found();
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        ~weak_ref_ptr()
        {
            if (valid())
                iObject->unsubcribe_destruction_watcher(*this);
        }
        weak_ref_ptr& operator=(const weak_ref_ptr& aOther)
        {
            reset(aOther.ptr());
            return *this;
        }
        weak_ref_ptr& operator=(const i_ref_ptr<Interface>& aOther)
        {
            reset(aOther.ptr());
            return *this;
        }
        weak_ref_ptr& operator=(nullptr_t)
        {
            reset();
            return *this;
        }
    public:
        virtual bool reference_counted() const
        {
            return false;
        }
        virtual void reset(Interface* aObject = nullptr, bool = false)
        {
            weak_ref_ptr copy(*this);
            iObject = aObject;
            if (valid())
                iObject->subcribe_destruction_watcher(*this);
        }
        virtual Interface* release()
        {
            if (iObject == nullptr)
                throw no_object();
            else
                throw bad_release();
        }
        virtual bool valid() const
        {
            return iObject != nullptr;
        }
        virtual Interface* ptr() const
        {
            return iObject;
        }
        virtual Interface* operator->() const
        {
            if (iObject == nullptr)
                throw no_object();
            return iObject;
        }
        virtual Interface& operator*() const
        {
            if (iObject == nullptr)
                throw no_object();
            return *iObject;
        }
    private:
        virtual void object_being_destroyed(i_reference_counted& aObject)
        {
            if (&aObject != iObject)
                throw wrong_object();
            iObject = nullptr;
        }
    private:
        Interface* iObject;
    };

    template <typename Interface>
    inline bool operator<(const ref_ptr<Interface>& lhs, const ref_ptr<Interface>& rhs)
    {
        if (lhs == rhs)
            return false;
        else if (lhs == nullptr)
            return false;
        else if (rhs == nullptr)
            return true;
        else
            return *lhs < *rhs;
    }

    template <typename ConcreteType, typename... Args>
    inline ref_ptr<ConcreteType> make_ref(Args&&... args)
    {
        return ref_ptr<ConcreteType>{ new ConcreteType{ std::forward<Args>(args)... } };
    }
}
