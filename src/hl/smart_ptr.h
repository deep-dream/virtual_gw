#pragma once

template<class T>
class smart_ptr{
	T*	pointer;
public:
	smart_ptr(T* p =0):pointer(p){ if(pointer) pointer->add_ref(); }
	smart_ptr(const smart_ptr& r):pointer(r.pointer){ if(pointer) pointer->add_ref(); }
	void operator=(const smart_ptr& r){
		if(pointer) pointer->release();
		pointer = r.pointer;
		if(pointer) pointer->add_ref();
	}	
	~smart_ptr(){ if(pointer) pointer->release(); }
	T* operator->(){ return pointer; }
	operator T*(){ return pointer; }
	operator bool(){ return pointer != 0; }
};

class intrusive_counter{
	int		count;
public:
	void	add_ref(){ ++count; }
	void	release(){ --count; if( count <= 0 ) delete this; }

protected:
	intrusive_counter():count(0){}
	virtual ~intrusive_counter(){}
};