// utility class definitions
#pragma once

#include <stdlib.h>
#include <stdio.h>

// define NULL just in case it wasn't defined elsewhere
#ifndef NULL
#define NULL 0
#endif

// define a lower case NULL as well (it looks nicer ;)
#define null NULL

// a small fix to MSVC6 and below, makes variables defined 
// inside a for loop have limited scope
#if _MSC_VER < 1300 
#define for if(false) {} else for
#endif // _MSC_VER


// some often used macros
#define safe_delete(x) if(x != null) { delete x; x = null; }
#define safe_delete_array(x) if(x != null) { delete[] x; x = null; }

#define safe_release(x) if(x != null) { x->Release(); x = null; }

class Object
{
protected:
  int refcount;
public:
  int RefCount() { return refcount; }

  int AddRef() { refcount ++; return refcount; }
  int Release() { refcount --; if(refcount <= 0) delete this;  return refcount; }

  Object() : refcount(1) {}
  virtual ~Object() {}
};

// The standard storage system relies on Elements and Containers.
// If any object of a certain class is to be stored, the class should be derived
// from Element. All objects of this class can now be placed in any Container.
// It should be noted, however, that although there are no type checks, each Container object
// should contain Elements of the same base type (that is one level higher than Element).

// The elegancy of such a setup lies in the fact that whenever any object derived from Element is deleted
// (either explicitly or implicitly through a call to Release()) it is also removed from any container(s)
// it may be in.


// in order for an element to be stored in a Container
// it must be derived from this class
class Element
{
protected:
	friend class Container;
	Element *prev, *next;
	Container *container;

	Element();

public:
  // returns the previous element in the Container's linked list
	inline Element* Prev() { return prev; }

  // returns the next element in the Container's linked list
	inline Element* Next() { return next; }

  // removes this element from the container it's in (if any)
	void Free(); 

  // returns a pointer to the Container element
	inline Container* Container() { return container; }

	virtual ~Element() = 0;
};

// the Container class describes a linked list of Elements
class Container
{
protected:
	friend class Element;
	Element *elements;
  int size;

public:

	Container();
  ~Container();

  // returns a pointer to the first element
	Element* Elements();

 
  // returns true if the given element is in this container
  bool Find(const Element *element);

  // returns the index of the element and -1 if the element wasn't found
  int GetIndex(const Element *element);
	  
  // inserts a new element into this container
  // returns true if the element was inserted successfully
  // if the element already existed false is returned
	bool Attach(Element *element);

  // inserts a new element into this container at the beginnning of the linked list
  inline bool AttachFirst(Element *element) { return Attach(element); }

  // inserts a new element into this container at the end of the linked list
	bool AttachLast(Element *element);

  // inserts a new element into this container before the specified element
  bool AttachBefore(Element *element, Element *before);

  // inserts a new element into this container after the specified element
  bool AttachAfter(Element *element, Element *after);

  // inserts a new element at the specified location
  bool AttachAt(Element *element, int index);


  // removes the specified element from this container
	bool Detach(Element *element);

  // removes all elements from this container  
  void DetachAll();

  // deletes all elements in this container
  // be careful when using this function
	void Empty();

  // detaches and releases all elements in this container
  // all elements are assumed to have the Object base type
  void ReleaseAll();

  // returns the number of elements in this container
  int GetSize();

  // returns the element at the given index
  Element* Get(int number);

};

/*
// this macro deletes all the Elements in a Container
#define EmptyContainer(container) { while(container.Elements()) delete container.Elements(); }

// this macro releases all the Elements in a Container
// this is the most commonly used method for emptying a container
#define safe_release_container(classtype, container) { classtype *element = (classtype *)container.Elements(); \
  while(element != null) { classtype *next = (classtype *)element->Next(); element->Release(); element = next; } }
*/

// this macro is used to iterate through the elements in a Container
// e.g.: forEach(Texture, textures, t) t->Destroy();
#define forEach(classtype, container, iterator) \
  for(classtype *iterator = (classtype *)container.Elements(); iterator != null; iterator = (classtype *) iterator->Next())


// this class is identical to Container, the only difference is that
// it deletes all the elements it contains when destroyed
class CleanContainer : public Container
{
public:
	~CleanContainer();
};

// misc stuff follows

// a memory buffer that expands on demand
class MemoryBuffer
{
  int size;
  unsigned char *buffer;

public:
  // returns the size of the buffer in bytes
  int GetSize();

  // returns a pointer to the buffer bytes
  void* GetBuffer();

  // expands the buffer if necessary and returns the pointer to the bytes
  void* GetBuffer(int bytesize);

  // reallocates the buffer, that is changes its size (expands or shrinks) while preserving the contents
  void* Realloc(int bytesize);

  // empties the buffer and frees the memory
  void Clear();

  MemoryBuffer();
  MemoryBuffer(int bytesize);

  ~MemoryBuffer();
};

// random number generator
class Random
{
  static Random initializer;
public:

  static double GetDouble(double min = 0.0, double max = 1.0);
  static int GetInt(int min, int max);
  inline static int GetInt(int max) { return rand() % max; }
  inline static int GetInt() { return rand(); }

  Random();
};

// some commonly used macros

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

