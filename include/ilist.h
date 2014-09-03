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
#ifndef BZR_LIST_H
#define BZR_LIST_H

template<class E, class T>
class ilist;

template<class E, class T>
class ilist_iterator;

template<class E, class T>
class ilist_node
{
private:
    ilist_node* _prev;
    ilist_node* _next;

    friend class ilist<E, T>;
    friend class ilist_iterator<E, T>;
};

template<class E, class T>
class ilist_iterator : public iterator<bidirectional_iterator_tag, E>
{
public:
    typedef ilist_node<E, T> node;

    ilist_iterator(node* n) : _node(n)
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

    E& operator*()
    {
        return *(E*)((uint8_t*)_node - offsetof(E, node::_prev));
    }

    E* operator->()
    {
        return (E*)((uint8_t*)_node - offsetof(E, node::_prev));
    }

    bool operator==(const ilist_iterator& rhs)
    {
        return _node == rhs._node;
    }

    bool operator!=(const ilist_iterator& rhs)
    {
        return  _node != rhs._node;
    }

private:
    node* _node;
};

template<class E, class T>
class ilist
{
public:
    typedef ilist_node<E, T> node;
    typedef ilist_iterator<E, T> iterator;

    ilist()
    {
        _root._prev = &_root;
        _root._next = &_root;
    };

    iterator begin()
    {
        return iterator(&_root);
    }

    iterator end()
    {
        return iterator(&_root);
    }

    void push_front(E* e)
    {
        auto n = static_cast<node_type*>(e);
        n->_prev = &_root;
        n->_next = _root._next;
        _root._next->prev = n;
        _root._next = n;
    }

    void push_back(E* e)
    {
        auto n = static_cast<node_type*>(e);
        n->_prev = _root._prev;
        n->_next = &_root;
        _root._prev->_next = n;
        _root._prev = n;
    };

private:
    node _root;
};

#endif