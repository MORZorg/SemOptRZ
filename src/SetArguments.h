/*
 * SetArguments.h
 *
 *  Created on: 3 Jun 2013
 *      Author: geryo
 */

#ifndef SETARGUMENTS_H_
#define SETARGUMENTS_H_

#include <map>
#include <string>
#include <iterator>
#include <algorithm>
#include <stdexcept>
using namespace std;

class Argument;

/**
 * @brief Class defining a const iterator for the elements of a set of arguments
 */
class SetArgumentsIterator : public map<string, Argument *>::const_iterator
{
public:
	SetArgumentsIterator() : map<string, Argument *>::const_iterator() {};
	SetArgumentsIterator(map<string, Argument *>::const_iterator it) : map<string, Argument *>::const_iterator(it) {};
	Argument ** operator->() {return (Argument ** const)&(map<string, Argument *>::const_iterator::operator->()->second); };
	Argument* operator*() { return map<string, Argument *>::const_iterator::operator*().second; }
	SetArgumentsIterator &operator=(const SetArgumentsIterator &param) { map<string, Argument *>::const_iterator::operator = (param); return *this; }
};



class SetArguments
{
	map<string, Argument *> arguments;
	map<int, string> key_assoc;
public:
	SetArguments();
	void add_Argument(Argument *);
	virtual ~SetArguments();
	Argument *getArgumentByName(string);
	Argument *getArgumentByNumber(int num);
	int cardinality() const;
	bool empty();
	SetArguments* get_attacks() const;
	SetArgumentsIterator begin() const;
	SetArgumentsIterator end() const;
	bool exists(Argument *) const;
	bool is_subset_equal(SetArguments *);
	bool is_subset(SetArguments *);
	void intersect(SetArguments *, SetArguments *);
	void setminus(SetArguments *, SetArguments *);
	void remove(Argument *);
	void clone(SetArguments *);
	bool operator==(const SetArguments &other) const;
};

#include "Argument.h"

ostream& operator<<(ostream& , const SetArguments& );

#endif /* SETARGUMENTS_H_ */
