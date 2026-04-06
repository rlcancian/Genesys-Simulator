/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   TManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:55
 */

#ifndef LISTOBSERVABLE_H
#define LISTOBSERVABLE_H

#include <string>
#include <list>
#include <map>
#include <iterator>
#include <functional>
#include <algorithm>
#include "Util.h"
#include "../simulator/ModelElement.h"

#include "List.h"

/*!
 * \brief Observable-oriented list variant used by kernel structures.
 *
 * This template specializes \c List behavior for contexts where list mutations are
 * relevant to external observers/managers, while preserving familiar list helpers.
 */
template <typename T>
class ListObservable : public List {
public:
	using CompFunct = std::function<bool(const T, const T) >;
public:
	/*! \brief Initializes an empty observable list. */
	ListObservable();
	virtual ~ListObservable() = default;
public: // direct access to list
	/*! \brief Removes all elements from the observable list. */
	void clear();
	/*! \brief Removes the first element from the observable list. */
	void pop_front();
public: // improved (easier) methods
	/*! \brief Inserts an element respecting the configured ordering. */
	void insert(T element);
	/*! \brief Removes an element from the observable list. */
	void remove(T element);
	/*! \brief Sets the element at a specific list index. */
	void setAtRank(unsigned int rank, T element);
};

template <typename T>
ListObservable<T>::ListObservable() {
	//_map = new std::map<Util::identitifcation, T>();
	_list = new std::list<T>();
	_it = _list->begin();
}

template <typename T>
std::list<T>* ListObservable<T>::list() const {
	return _list;
}

template <typename T>
unsigned int ListObservable<T>::size() {
	return _list->size();
}

//template <typename T>
//ListObservable<T>::ListObservable(const ListObservable& orig) {
//}

//template <typename T>
//ListObservable<T>::~ListObservable() {
//}

template <typename T>
std::string ListObservable<T>::show() {
	int i = 0;
	std::string text = "{";
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++, i++) {
		text += "[" + std::to_string(i) + "]=(" + (*it)->show() + "),";
	}
	text += "}";
	return text;
}

template <typename T>
void ListObservable<T>::insert(T element) {
	_list->insert(std::upper_bound(_list->begin(), _list->end(), element, _sortFunc), element);
}

template <typename T>
bool ListObservable<T>::empty() {
	return _list->empty();
}

template <typename T>
void ListObservable<T>::pop_front() {
	typename std::list<T>::iterator itTemp = _list->begin();
	_list->pop_front();
	if (_it == itTemp) { /*  \todo: +: check this */
		_it = _list->begin(); // if it points to the removed element, then changes to begin
	}
}

template <typename T>
void ListObservable<T>::remove(T element) {
	_list->remove(element);
	if ((*_it) == element) { /*  \todo: +: check this */
		_it = _list->begin(); // if it points to the removed element, then changes to begin
	}
}

template <typename T>
T ListObservable<T>::create() {
	return new T();
}

template <typename T>
void ListObservable<T>::clear() {
	_list->clear();
}

template <typename T>
T ListObservable<T>::getAtRank(unsigned int rank) {
	unsigned int thisRank = 0;
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
		if (rank == thisRank) {
			return (*it);
		} else {
			thisRank++;
		}
	}
	return 0; /* \todo: Invalid return depends on T. If T is pointer, nullptr works fine. If T is double, it does not. I just let (*it), buut it is not nice*/
}

template <typename T>
void ListObservable<T>::setAtRank(unsigned int rank, T element) {
	if (rank == _list->size()) {
		_list->insert(_list->end(), element);
	} else {
		unsigned int thisRank = 0;
		for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
			if (rank == thisRank) {
				*it = element;
				return;
			} else {
				thisRank++;
			}
		}
	}
}

template <typename T>
T ListObservable<T>::next() {
	_it++;
	if (_it != _list->end())
		return (*_it);
	else
		return nullptr;

}

template <typename T>
typename std::list<T>::iterator ListObservable<T>::find(T element) {
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
		if ((*it) == element) {
			return it;
		}
	}
	return _list->end(); /*  \todo:+-: check nullptr or invalid iterator when not found */
	//return nullptr;
}

/*
template <typename T>
int ListObservable<T>::rankOf(T element) {
	int rank = 0;
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
	if ((*it) == element) {
		return rank;
	} else
		rank++;
	}
	return -1; // not found -> negative rank
}
 */

template <typename T>
T ListObservable<T>::front() {
	_it = _list->begin();
	//if (_it != _list->end())
	return (*_it);
	//else
	//return dynamic_cast<T>(nullptr);
}

template <typename T>
T ListObservable<T>::last() {
	_it = _list->end();
	_it--;
	//if (_it != _list->end()) // \todo: CHECK!!!
	return (*_it);
	//else return nullptr;
}

template <typename T>
T ListObservable<T>::previous() {
	_it--; // \todo: CHECK!!!
	return (*_it);
}

template <typename T>
T ListObservable<T>::current() {
	/* \todo: To implement (i thing it's just to check). Must actualize _it on other methods when other elements are accessed */
	return (*_it);
}

template <typename T>
void ListObservable<T>::setSortFunc(CompFunct _sortFunc) {
	this->_sortFunc = _sortFunc;
}

template <typename T>
template<typename U>
T ListObservable<T>::create(U arg) {
	return T(arg);
}

template <typename T>
template<class Compare>
void ListObservable<T>::sort(Compare comp) {
	_list->sort(comp);
}

#endif /* LISTOBSERVABLE_H */
