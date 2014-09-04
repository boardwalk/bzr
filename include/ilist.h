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

template<class Elem, class Tag>
class ilist;

template<class Elem, class Tag, bool IsConst>
class ilist_iterator;

template<class Elem, class Tag>
class ilist_node
{
    ilist_node* _prev;
    ilist_node* _next;

    friend class ilist<Elem, Tag>;
    friend class ilist_iterator<Elem, Tag, false>;
    friend class ilist_iterator<Elem, Tag, true>;
};

template<class Elem, class Tag, bool IsConst>
class ilist_iterator : public iterator<bidirectional_iterator_tag, Elem>
{
public:
    typedef typename conditional<IsConst, ilist_node<Elem, Tag>, const ilist_node<Elem, Tag>>::type node;
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
        operator--();
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
        operator++();
        return tmp;
    }

    elem& operator*()
    {
        return *(elem*)((byte*)_node - offsetof(elem, node::_prev));
    }

    elem* operator->()
    {
        return (elem*)((byte*)_node - offsetof(elem, node::_prev));
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

    friend class ilist<Elem, Tag>;
};

template<class Elem, class Tag>
class ilist
{
public:
    typedef ilist_node<Elem, Tag> node;
    typedef ilist_iterator<Elem, Tag, false> iterator;
    typedef ilist_iterator<Elem, Tag, true> const_iterator;

    ilist()
    {
        _root._prev = &_root;
        _root._next = &_root;
    };

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

    void insert(iterator position, Elem* elem)
    {
        elem->node::_prev = _root._prev;
        elem->node::_next = &_root;
        _root._prev->_next = elem;
        _root._prev = elem;
    }

    iterator erase(iterator position)
    {
        assert(position != end());
        auto n = position._node;
        n->_prev->_next = n->_next;
        n->_next->_prev = n->_prev;
        return iterator(n->_next);
    }

    void push_front(Elem* elem)
    {
        insert(begin(), elem);
    }

    void push_back(Elem* elem)
    {
        insert(end(), elem);
    }

private:
    node _root;
};

#endif