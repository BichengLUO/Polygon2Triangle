#ifndef _COMPARABLE
#define _COMPARABLE

#include <ostream>

using namespace std;

enum  cmp_t {
	MIN_CMP = -1,
	EQ_CMP = 0,
	MAX_CMP = 1
};


template <class KeyType>
class Comparable
{
private:
	KeyType myKey;

public:
	Comparable(KeyType key) : myKey(key) {}
	virtual ~Comparable() {};

	virtual cmp_t Compare(KeyType key) const
	{
		return (key == myKey) ? EQ_CMP
			: ((key < myKey) ? MIN_CMP : MAX_CMP);
	}

	KeyType Key() const
	{
		return myKey;
	}

	ostream& Print(ostream & os) const
	{
		os << myKey;
		return  os;
	}
};

template <class KeyType>
inline ostream& operator<<(ostream & os, Comparable<KeyType> item)
{
	return  item.Print(os);
}

#endif