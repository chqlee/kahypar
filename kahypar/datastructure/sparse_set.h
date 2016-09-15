/***************************************************************************
 *  Copyright (C) 2016 Sebastian Schlag <sebastian.schlag@kit.edu>
 **************************************************************************/

/*
 * Sparse set representation based on
 * Briggs, Preston, and Linda Torczon. "An efficient representation for sparse sets."
 * ACM Letters on Programming Languages and Systems (LOPLAS) 2.1-4 (1993): 59-69.
 */

#pragma once

#include <limits>
#include <memory>
#include <utility>

#include "meta/mandatory.h"
#include "macros.h"

namespace datastructure {
template <typename ValueType = Mandatory,
          typename Derived = Mandatory>
class SparseSetBase {
 public:
  SparseSetBase(const SparseSetBase&) = delete;
  SparseSetBase& operator= (const SparseSetBase&) = delete;

  SparseSetBase& operator= (SparseSetBase&&) = delete;

  ValueType size() const {
    return _size;
  }

  bool contains(const ValueType value) const {
    return static_cast<const Derived*>(this)->containsImpl(value);
  }

  void add(const ValueType value) {
    static_cast<Derived*>(this)->addImpl(value);
  }

  const ValueType* begin() const {
    return _dense;
  }

  const ValueType* end() const {
    return _dense + _size;
  }

  void clear() {
    static_cast<Derived*>(this)->clearImpl();
  }

 protected:
  explicit SparseSetBase(const ValueType k) :
    _size(0),
    _sparse(nullptr),
    _dense(nullptr) {
    ValueType* raw = static_cast<ValueType*>(malloc(((2 * k)) * sizeof(ValueType)));
    for (ValueType i = 0; i < 2 * k; ++i) {
      new(raw + i)ValueType(std::numeric_limits<ValueType>::max());
    }
    _sparse = raw;
    _dense = raw + k;
  }

  ~SparseSetBase() {
    free(_sparse);
  }

  SparseSetBase(SparseSetBase&& other) :
    _size(other._size),
    _sparse(other._sparse),
    _dense(other._dense) {
    other._size = 0;
    other._sparse = nullptr;
    other._dense = nullptr;
  }

  ValueType _size;
  ValueType* _sparse;
  ValueType* _dense;
};

template <typename ValueType = Mandatory>
class SparseSet final : public SparseSetBase<ValueType, SparseSet<ValueType> >{
  using Base = SparseSetBase<ValueType, SparseSet<ValueType> >;
  friend Base;

 public:
  explicit SparseSet(const ValueType k) :
    Base(k) { }

  SparseSet(const SparseSet&) = delete;
  SparseSet(SparseSet&& other) :
    Base(std::move(other)) { }

  SparseSet& operator= (SparseSet&&) = delete;
  SparseSet& operator= (const SparseSet&) = delete;

  void remove(const ValueType value) {
    const ValueType index = _sparse[value];
    if (index < _size && _dense[index] == value) {
      const ValueType e = _dense[--_size];
      _dense[index] = e;
      _sparse[e] = index;
    }
  }

 private:
  bool containsImpl(const ValueType value) const {
    const ValueType index = _sparse[value];
    return index < _size && _dense[index] == value;
  }

  void addImpl(const ValueType value) {
    const ValueType index = _sparse[value];
    if (index >= _size || _dense[index] != value) {
      _sparse[value] = _size;
      _dense[_size++] = value;
    }
  }

  void clearImpl() {
    _size = 0;
  }

  using Base::_sparse;
  using Base::_dense;
  using Base::_size;
};

template <typename ValueType = Mandatory>
class InsertOnlySparseSet final : public SparseSetBase<ValueType, SparseSet<ValueType> >{
  using Base = SparseSetBase<ValueType, SparseSet<ValueType> >;
  friend Base;

 public:
  explicit InsertOnlySparseSet(const ValueType k) :
    Base(k),
    _threshold(0) { }

  InsertOnlySparseSet(const InsertOnlySparseSet&) = delete;
  InsertOnlySparseSet(InsertOnlySparseSet&& other) :
    Base(std::move(other)),
    _threshold(other._threshold) {
    other._threshold = 0;
  }

  InsertOnlySparseSet& operator= (InsertOnlySparseSet&&) = delete;
  InsertOnlySparseSet& operator= (const InsertOnlySparseSet&) = delete;

 private:
  bool containsImpl(const ValueType value) const {
    return _sparse[value] == _threshold;
  }

  void addImpl(const ValueType value) {
    if (!contains(value)) {
      _sparse[value] = _threshold;
      _dense[_size++] = value;
    }
  }

  void clearImpl() {
    _size = 0;
    if (_threshold == std::numeric_limits<ValueType>::max()) {
      for (ValueType i = 0; i < _dense - _sparse; ++i) {
        _sparse[i] = 0;
      }
      _threshold = 0;
    }
    ++_threshold;
  }

  ValueType _threshold;
  using Base::_size;
  using Base::_sparse;
  using Base::_dense;
};
}  // namespace datastructure