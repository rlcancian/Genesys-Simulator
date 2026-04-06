#ifndef GENLIST_H
#define GENLIST_H
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   List.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:55
 */



#include <string>
#include <list>
#include <map>
#include <iterator>
#include <functional>
#include <algorithm>


/*!
 * \brief Lightweight wrapper around \c std::list with convenience helpers used by the kernel.
 *
 * The class centralizes insertion/removal, rank-based access and iterator-like
 * navigation patterns heavily used in simulator internals and plugin infrastructure.
 */
template <typename T>
class List {
public:
	using CompFunct = std::function<bool(const T, const T) >;
public:
	/*! \brief Creates an empty list and positions the internal iterator at the beginning. */
	List();
	/*! \brief Creates a copy of the source list. */
	List(List<T> &origin);
	virtual ~List() = default;
public: // direct access to list
	/*! \brief Returns the number of stored elements. */
	unsigned int size();
	/*! \brief Indicates whether the list is empty. */
	bool empty();
	/*! \brief Removes all elements from the list. */
	void clear();
	/*! \brief Removes the first element from the list. */
	void pop_front();
	template<class Compare>
	/*! \brief Sorts elements using the provided comparator. */
	void sort(Compare comp);
	/*! \brief Returns direct access to the encapsulated \c std::list structure. */
	std::list<T>* list() const;
public: // new methods
	/*! \brief Creates a new default element of type \c T. */
	T create();
	template<typename U>
	/*! \brief Creates a new \c T element using a construction argument. */
	T create(U arg);
	/*! \brief Generates a textual representation of elements for debugging. */
	std::string show();
	/*! \brief Searches for an element and returns an iterator to it (or \c end()). */
	typename std::list<T>::iterator find(T element);
	//int rankOf(T modeldatum); //!< returns the position (1st position=0) of the modeldatum if found, or negative value if not found
public: // improved (easier) methods
	/*! \brief Inserts an element preserving the current ordering policy. */
	void insert(T element);
	/*! \brief Removes all occurrences of the provided element. */
	void remove(T element);
	/*! \brief Replaces (or appends) an element at a specific rank. */
	void setAtRank(unsigned int rank, T element);
	/*! \brief Returns the element at the provided rank. */
	T getAtRank(unsigned int rank);
	/*! \brief Advances the internal iterator and returns the next element. */
	T next();
	/*! \brief Moves to the beginning and returns the first element. */
	T front();
	/*! \brief Moves to the end and returns the last element. */
	T last();
	/*! \brief Moves the internal iterator backward and returns the previous element. */
	T previous();
	/*! \brief Returns the element at the current internal iterator position. */
	T current(); // get current modeldatum on the list (the last used)
	/*! \brief Sets the comparison function used for ordered insertions. */
	void setSortFunc(CompFunct _sortFunc);
	//public: // @TODO: Shoul in a specialized class classed ObservableList
	//	void addObserverHandler();
protected:
	//std::map<Util::identitifcation, T>* _map;
	std::list<T>* _list;
	CompFunct _sortFunc{[](const T, const T) {
			return false;
		}}; //! Default function: insert at the end of the list.
	typename std::list<T>::iterator _it;
};

template <typename T>
List<T>::List() {
	//_map = new std::map<Util::identitifcation, T>();
	_list = new std::list<T>();
	_it = _list->begin();
}

template <typename T>
List<T>::List(List<T> &origin) {
	_list = new std::list<T>(origin);
	_it = _list->begin(); // todo: check. end()? 2210
}

template <typename T>
std::list<T>* List<T>::list() const {
	return _list;
}

template <typename T>
unsigned int List<T>::size() {
	return _list->size();
}

//template <typename T>
//List<T>::List(const List& orig) {
//}

//template <typename T>
//List<T>::~List() {
//}

template <typename T>
std::string List<T>::show() {
	int i = 0;
	std::string text = "{";
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++, i++) {
		text += "[" + std::to_string(i) + "]=(" + (*it)->show() + "),";
	}
	text += "}";
	return text;
}

template <typename T>
void List<T>::insert(T element) {
	_list->insert(std::upper_bound(_list->begin(), _list->end(), element, _sortFunc), element);
}

template <typename T>
bool List<T>::empty() {
	return _list->empty();
}

template <typename T>
void List<T>::pop_front() {
	typename std::list<T>::iterator itTemp = _list->begin();
	_list->pop_front();
	if (_it == itTemp) { /*  @TODO: +: check this */
		_it = _list->begin(); // if it points to the removed modeldatum, then changes to begin
	}
}

template <typename T>
void List<T>::remove(T element) {
	_list->remove(element);
	if ((*_it) == element) { /*  @TODO: +: check this */
		_it = _list->begin(); // if it points to the removed modeldatum, then changes to begin
	}
}

template <typename T>
T List<T>::create() {
	return new T();
}

template <typename T>
void List<T>::clear() {
	_list->clear();
}

template <typename T>
T List<T>::getAtRank(unsigned int rank) {
	unsigned int thisRank = 0;
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
		if (rank == thisRank) {
			return (*it);
		} else {
			thisRank++;
		}
	}
	return 0; /* @TODO: Invalid return depends on T. If T is pointer, nullptr works fine. If T is double, it does not. I just let (*it), but it is not nice*/
}

template <typename T>
void List<T>::setAtRank(unsigned int rank, T element) {
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
T List<T>::next() {
	_it++;
	if (_it != _list->end())
		return (*_it);
	else
		return nullptr;

}

template <typename T>
typename std::list<T>::iterator List<T>::find(const T element) {
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
		if ((*it) == element) {
			return it;
		}
	}
	return _list->end(); /*  @TODO:+-: check nullptr or invalid iterator when not found */
	//return nullptr;
}

/*
template <typename T>
int List<T>::rankOf(T modeldatum) {
	int rank = 0;
	for (typename std::list<T>::iterator it = _list->begin(); it != _list->end(); it++) {
	if ((*it) == modeldatum) {
		return rank;
	} else
		rank++;
	}
	return -1; // not found -> negative rank
}
 */

template <typename T>
T List<T>::front() {
	_it = _list->begin();
	//if (_it != _list->end())
	return (*_it);
	//else
	//return dynamic_cast<T>(nullptr);
}

template <typename T>
T List<T>::last() {
	_it = _list->end();
	_it--;
	//if (_it != _list->end()) // @TODO: CHECK!!!
	return (*_it);
	//else return nullptr;
}

template <typename T>
T List<T>::previous() {
	_it--; // @TODO: CHECK!!!
	return (*_it);
}

template <typename T>
T List<T>::current() {
	/* @TODO: To implement (i thing it's just to check). Must actualize _it on other methods when other elements are accessed */
	return (*_it);
}

template <typename T>
void List<T>::setSortFunc(CompFunct _sortFunc) {
	this->_sortFunc = _sortFunc;
}

template <typename T>
template<typename U>
T List<T>::create(U arg) {
	return T(arg);
}

template <typename T>
template<class Compare>
void List<T>::sort(Compare comp) {
	_list->sort(comp);
}

#endif
