#include "Util.h"
#include <stdlib.h>
#include <memory.h>
#include <time.h>


//*** Element

Element::Element() : container(null), prev(null), next(null) 
{
}

Element::~Element()
{ 
  Free(); 
}


void Element::Free() 
{
	if(container)	container->Detach(this);
}

//*** Container

Container::Container() : elements(null), size(0) 
{
}

Container::~Container()
{
  DetachAll();
}

Element* Container::Elements()
{
  return elements;
}

bool Container::Find(const Element *element) 
{
	Element *c = elements;
	while(c != null) { 
		if(c == element) return true;  
		c = c->next; 
	}

	return false;
}

int Container::GetIndex(const Element *element) 
{
	Element *c = elements;
  int i = 0;
	while(c != null) { 
		if(c == element) return i;  
		c = c->next;
    i ++;
	}

	return -1;
}

bool Container::Attach(Element *element) 
{
	if(!Find(element)) {
		element->Free(); 

		if(elements != null) elements->prev = element;
		
		element->prev = null;
		element->next = elements;
		
		elements = element;
		element->container = this;
		
    size ++;
		return true;
	}

	return false;
}


bool Container::AttachLast(Element *element) 
{
	if(!Find(element)) {
		element->Free();

		if(elements == null)	{
			element->prev = null;
			element->next = elements;
			
			elements = element;
			element->container = this;

		} else {
			Element *last;
			for(last = elements; last->next != null; last = last->next);
			
			last->next = element;
			element->prev = last;
			element->next = null;
			
			element->container = this;
		}

    size ++;
		return true;
	}

	return false;
}


bool Container::AttachAfter(Element *element, Element *after) 
{
	if(!Find(element) && Find(after)) {
		element->Free();

		Element *next = after->next;
    
    element->next = next;
    if(next != null) next->prev = element;

    element->prev = after;
    after->next = element;

		element->container = this;

    size ++;
		return true;
	}

	return false;
}


bool Container::AttachBefore(Element *element, Element *before) 
{
	if(!Find(element) && Find(before)) {
		element->Free();

		Element *prev = before->prev;
    
    element->next = before;
    before->prev = element;

    element->prev = prev;
    if(prev != null) {
      prev->next = element;
    } else
      elements = element;

		element->container = this;

    size ++;
		return true;
	}

	return false;
}

bool Container::AttachAt(Element *element, int index)
{
  if(!Find(element)) {
    Element *before = Get(index);
    if(before != null) {
		  element->Free();

		  Element *prev = before->prev;
      
      element->next = before;
      before->prev = element;

      element->prev = prev;
      if(prev != null) {
        prev->next = element;
      } else
        elements = element;

		  element->container = this;

      size ++;
		  return true;
    }
  }

  return false;
}


bool Container::Detach(Element *element) 
{
	if(Find(element)) {
		if(element->prev == null) elements = element->next;
		else element->prev->next = element->next;

		if(element->next != null)
			element->next->prev = element->prev;

		element->prev = element->next = null;
		element->container = null;

    size --;
		return true;
	}

	return false;
}


void Container::DetachAll()
{
  while(elements != null) Detach(elements);
}


void Container::Empty() 
{ 
	while(elements != null) delete elements; 
}

void Container::ReleaseAll()
{
  /*
  Element *element = elements;
  while(element != null) {
    Element *next = element->next;
    ((Object *)element)->Release();
    element = next;
  }
  */
}

int Container::GetSize()
{
  return size;
}


Element* Container::Get(int number)
{
  int i = 0;
	Element *c = elements;
	while(c != null) {
    if(i == number) return c;
		c = c->next; 
    i ++;
	}

  return null;
}

//*** CleanContainer

CleanContainer::~CleanContainer() 
{
	Empty();
}


//*** MemoryBuffer

int MemoryBuffer::GetSize() 
{ 
  return size; 
}

void* MemoryBuffer::GetBuffer() 
{ 
  return buffer; 
}

// expands the buffer if necessary and returns the pointer to the bytes
void* MemoryBuffer::GetBuffer(int bytesize) 
{
  if(bytesize > size) {
    safe_delete(buffer);
    size = bytesize;
    buffer = new unsigned char[size];
  }
  return buffer;
}

void* MemoryBuffer::Realloc(int bytesize) 
{
  int copysize = min(bytesize,size);
  unsigned char *newbuffer = new unsigned char[bytesize];
  memcpy(newbuffer, buffer, copysize);
  safe_delete_array(buffer);
  buffer = newbuffer;
  size = bytesize;
  return buffer;
}

void MemoryBuffer::Clear() 
{
  safe_delete_array(buffer);
  size = 0;
}

MemoryBuffer::MemoryBuffer() : buffer(null), size(0) 
{
}

MemoryBuffer::MemoryBuffer(int bytesize) 
{
  size = bytesize;
  buffer = new unsigned char[size];
}

MemoryBuffer::~MemoryBuffer() 
{
  Clear();
}

//*** Random

Random Random::initializer;

Random::Random()
{
  srand((unsigned int)time(null));
}

double Random::GetDouble(double min, double max)
{
  double r = (double)rand()/(double)RAND_MAX;
  return (max - min) * r + min;
}

int Random::GetInt(int min, int max)
{
  return rand() % (max - min) + min;
}




