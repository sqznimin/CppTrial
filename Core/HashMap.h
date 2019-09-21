#pragma once

#include "General.h"
#include "HashTable.h"

CT_SCOPE_BEGIN

template <typename Key,
          typename Value,
          typename HashFunc = std::hash<Key>,
          typename KeyEqual = typename HashTableInternal::KeyEqual<Key>,
          template <typename T> class Alloc = Allocator>
class HashMap
{
public:
    //typedef std::pair<const Key, Value> PairType;
    typedef std::pair<Key, Value> PairType;
    typedef typename HashTableInternal::MapKeyTraits<PairType> KeyTriats;
    typedef HashTable<PairType, HashFunc, KeyEqual, KeyTriats, Alloc> HashTableType;

public:
    HashMap() = default;
    HashMap(const HashMap &) = default;
    HashMap(HashMap &&) noexcept = default;
    HashMap &operator=(const HashMap &) = default;
    HashMap &operator=(HashMap &&) noexcept = default;
    ~HashMap() = default;

    explicit HashMap(size_t initCapacity) : hashTable(HashTableType(initCapacity))
    {
    }

    HashMap(std::initializer_list<PairType> initList)
    {
        size_t initSize = initList.size();
        hashTable = HashTableType(initSize); // may make waste when has same keys
        for (const PairType &pair : initList)
        {
            hashTable.Put(pair);
        }
    }

    HashMap &operator=(std::initializer_list<PairType> initList)
    {
        HashMap temp(initList);
        Swap(temp);
        return *this;
    }

public:
    size_t Size() const
    {
        return hashTable.Size();
    }

    size_t Capacity() const
    {
        return hashTable.Capacity();
    }

    bool IsEmpty() const
    {
        return hashTable.IsEmpty();
    }

    bool IsFull() const
    {
        return hashTable.IsFull();
    }

    void Swap(HashMap &other)
    {
        hashTable.Swap(other.hashTable);
    }

    void Clear()
    {
        hashTable.Clear();
    }

    void Shrink()
    {
        hashTable.Shrink();
    }

    bool Contains(const Key &key) const
    {
        return hashTable.ContainsKey(key);
    }

    void Put(const Key &key, const Value &value)
    {
        hashTable.Put(std::make_pair(key, value));
    }

    void Put(const Key &key, Value &&value)
    {
        hashTable.Put(std::make_pair(key, std::move(value)));
    }

    void Put(Key &&key, const Value &value)
    {
        hashTable.Put(std::make_pair(key, value));
    }

    void Put(Key &&key, Value &&value)
    {
        hashTable.Put(std::make_pair(key, std::move(value)));
    }

    Key &Get(const Key &key)
    {
        return hashTable.GetByKey(key).first;
    }

    const Key &Get(const Key &key) const
    {
        return hashTable.GetByKey(key).first;
    }

    bool Remove(const Key &key)
    {
        return hashTable.RemoveByKey(key);
    }

    bool operator==(const HashMap &other) const
    {
        return hashTable == other.hashTable;
    }

    bool operator!=(const HashMap &other) const
    {
        return !(*this == other);
    }

    //===================== STL STYLE =========================
public:
    auto begin()
    {
        return hashTable.begin();
    }

    auto begin() const
    {
        return hashTable.begin();
    }

    auto end()
    {
        return hashTable.end();
    }

    auto end() const
    {
        return hashTable.end();
    }

private:
    HashTableType hashTable;
};

CT_SCOPE_END