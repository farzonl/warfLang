/*
 * Copyright (c) 2023 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <vector>

template<class T>
class TemplateList {
protected:
  std::vector<T> vec;

public:
  TemplateList() : vec() {};
  TemplateList(std::vector<T> v) : vec(std::move(v)) {};
  T& operator[](int index) { return vec[index]; }
  const T& operator[](int index) const { return vec[index]; }

  typename std::vector<T>::iterator begin() { return vec.begin(); }
  typename std::vector<T>::iterator end() { return vec.end(); }
  typename std::vector<T>::const_iterator begin() const { return vec.begin(); }
  typename std::vector<T>::const_iterator end() const { return vec.end(); }
  void assign(typename std::vector<T>::const_iterator start,
              typename std::vector<T>::const_iterator end) { vec.assign(start, end); }
  void swap(TemplateList<T> list) { vec.swap(list.vec); }
  bool empty() const {return vec.empty();}
  size_t size() const {return vec.size();}
};