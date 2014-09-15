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
#ifndef BZR_BINVALUE_H

class BinSchema
{
public:
    enum Type
    {
        Structure,
        Array,
        UI8,
        SI8,
        UI16,
        SI16,
        UI32,
        SI32,
        UI64,
        SI64
        F32,
        F64
    };

    const string& name() const;
    Type type() const;

private:
    string _name;
    Type _type;
    vector<unique_ptr<BinSchema>> _fields;

};

class BinStructure : public BinSchema
{
private:
    vector<unique_ptr<BinSchema>> _fields;
};

class BinArray : public BinSchema
{
private:
    unique_ptr<BinSchema> _subschema;
};

template<class T>
struct BinTypeTraits

template<>
struct BinTypeTraits<uint8_t> { static const BinSchema::Type type = BinSchema::UI8; };

template<>
struct BinTypeTraits<int8_t> { static const BinSchema::Type type = BinSchema::SI8; };

template<>
struct BinTypeTraits<uint16_t> { static const BinSchema::Type type = BinSchema::UI16; };

template<>
struct BinTypeTraits<int16_t> { static const BinSchema::Type type = BinSchema::SI16; };

template<>
struct BinTypeTraits<uint32_t> { static const BinSchema::Type type = BinSchema::UI32; };

template<>
struct BinTypeTraits<int32_t> { static const BinSchema::Type type = BinSchema::SI32; };

template<>
struct BinTypeTraits<uint64_t> { static const BinSchema::Type type = BinSchema::UI64; };

template<>
struct BinTypeTraits<int64_t> { static const BinSchema::Type type = BinSchema::SI64; };

template<>
struct BinTypeTraits<float> { static const BinSchema::Type type = BinSchema::F32; };

template<>
struct BinTypeTraits<double> { static const BinSchema::Type type = BinSchema::F64; };

class BinValue
{
public:
    const BinSchema& schema() const;

    const BinValue& operator[](const string& key) const;

    const BinValue& operator[](size_t key) const;

    template<class T>
    const T& cast() const
    {
        assert(schema().type() == BinTypeTraits<T>::type);
        return *(const T*)_begin;
    }

private:
    const Schema& _schema;
    const void* _begin;
    size_t _length;
};

#endif