/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef BZR_ILIST_H
#define BZR_ILIST_H

#include "Noncopyable.h"

template<class Elem>
class ilist;

struct ilist_node : Noncopyable
{
    explicit ilist_node(size_t off) : _prev(nullptr), _next(nullptr), _offset(uint8_t(off))
    {
        assert(off <= numeric_limits<uint8_t>::max());
    }

    ~ilist_node()
    {
        if(_prev != nullptr)
        {
            _prev->_next = _next;
            _next->_prev = _prev;
        }
    }

    ilist_node* _prev;
    ilist_node* _next;
    const uint8_t _offset;
};

template<class Elem, int Num>
class ilist_hook
{
public:
    ilist_hook() : _node(offset())
    {}

    static size_t offset()
    {
        auto elem = reinterpret_cast<Elem*>(0);
        auto node = static_cast<ilist_hook<Elem, Num>*>(elem);
        return (uint8_t*)node - (uint8_t*)elem;       
    }

private:
    ilist_node _node;

    friend class ilist<Elem>;
};

template<class Elem, bool IsConst>
class ilist_iterator : public iterator<bidirectional_iterator_tag, Elem>
{
public:
    typedef typename conditional<IsConst, const ilist_node, ilist_node>::type node;
    typedef typename conditional<IsConst, const Elem, Elem>::type elem;
    typedef typename conditional<IsConst, const uint8_t, uint8_t>::type byte;

    explicit ilist_iterator(node* n) : _node(n)
    {}

    ilist_iterator& operator--()
    {
        _node = _node->_prev;
        return *this;
    }

    ilist_iterator operator--(int)
    {
        auto tmp = *this;
        _node = _node->_prev;
        return tmp;
    }

    ilist_iterator& operator++()
    {
        _node = _node->_next;
        return *this;
    }

    ilist_iterator operator++(int)
    {
        auto tmp = *this;
        _node = _node->_next;
        return tmp;
    }

    elem& operator*() const
    {
        return *reinterpret_cast<elem*>(reinterpret_cast<byte*>(_node) - _node->_offset);
    }

    elem* operator->() const
    {
        return reinterpret_cast<elem*>(reinterpret_cast<byte*>(_node) - _node->_offset);
    }

    template<int Num>
    bool on_hook() const
    {
        return _node->_offset == ilist_hook<Elem, Num>::offset();
    }

    bool in_list() const
    {
        return _node->_prev != nullptr;
    }

    bool operator==(const ilist_iterator& rhs) const
    {
        return _node == rhs._node;
    }

    bool operator!=(const ilist_iterator& rhs) const
    {
        return  _node != rhs._node;
    }

private:
    node* _node;

    friend class ilist<Elem>;
};

template<class Elem>
class ilist : Noncopyable
{
public:
    typedef ilist_iterator<Elem, false> iterator;
    typedef ilist_iterator<Elem, true> const_iterator;

    ilist() : _root(0)
    {
        _root._prev = &_root;
        _root._next = &_root;
    }

    iterator begin()
    {
        return iterator(_root._next);
    }

    const_iterator begin() const
    {
        return const_iterator(_root._next);
    }

    iterator end()
    {
        return iterator(&_root);
    }

    const_iterator end() const
    {
        return const_iterator(&_root);
    }

    void insert(iterator position, iterator it)
    {
        it._node->_prev = position._node->_prev;
        it._node->_next = position._node;

        position._node->_prev->_next = it._node;
        position._node->_prev = it._node;
    }

    iterator erase(iterator position)
    {
        assert(position != end());

        auto& node = *position._node;

        node._prev->_next = node._next;
        node._next->_prev = node._prev;

        return iterator(node._next);
    }

    void push_front(iterator it)
    {
        insert(begin(), it);
    }

    void push_back(iterator it)
    {
        insert(end(), it);
    }

    template<int Num>
    iterator iterator_for(Elem& elem) const
    {
        auto& node = static_cast<ilist_hook<Elem, Num>&>(elem)._node;
        return iterator(&node);
    }

private:
    ilist_node _root;
};

#endif
