#ifndef __UCS_DM_H__
#define __UCS_DM_H__

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <list>

/*
 * UCSPrimitiveValue<T> 
 * 	
 * 				stores one primitive value of <T> (like int, string)
 * 				not a container!
 * 				fires events on assignment
 */

class UCSValue {

	public:
		
		// FIXME: typeName is useless (returns obfuscated type with gcc)
		
		virtual std::string typeName () const {
			return typeid (*this).name ();
		}
		
		virtual ~UCSValue () { }
	
};

template<typename T> class UCSValueListener {
	
	public:
		
		virtual void onValueChanged (const T& newValue) = 0;
	
};

template<typename T> class UCSPrimitiveValue: public UCSValue {
	
	private:
		
		T value;
		
		// TODO: need better implementation here (this one doesn't get inlined even with -O3)
		
		std::list<std::shared_ptr<UCSValueListener<T>>> listeners;
		
		void invokeListeners () {
			if (!listeners.empty ()) {
				for (auto it = std::begin (listeners); it != std::end (listeners); ++it)
					(*it) -> onValueChanged (value);
			}
		}
	
	public:
		
		UCSPrimitiveValue (): value () { }
		UCSPrimitiveValue (const T& initValue): value (initValue) { }
		UCSPrimitiveValue (const UCSPrimitiveValue<T>& initValue): value (initValue.value) { }

		UCSPrimitiveValue<T>& operator= (const UCSPrimitiveValue<T>& rhs) {
			if (value != rhs.value) {
				value = rhs.value;
				invokeListeners ();
			}
			return *this;
		}
		
		UCSPrimitiveValue<T>& operator= (const T& rhs) {
			if (value != rhs) {
				value = rhs;
				invokeListeners ();
			}
			return *this;
		}
		
		operator T () const { return value; }
		
		void addListener (const std::shared_ptr<UCSValueListener<T>>& listener) {
			listeners.push_back (listener);
		}
		
};

template<typename keyT, typename valueT> class UCSContainer {
	
	protected:
		
		virtual std::shared_ptr<valueT> getValue (const keyT& key) const = 0;
		virtual void setValue (const keyT& key, std::shared_ptr<valueT> value) = 0;

	public:
		
		template<class T> std::shared_ptr<T> get (const keyT& key) {
			std::shared_ptr<valueT> value = getValue (key);
			if (value) {
				std::shared_ptr<T> result = std::dynamic_pointer_cast<T> (value); 
				if (!result)
					throw std::runtime_error ("Type mismatch");
				return result;
			} else {
				std::shared_ptr<T> newEntry (new T());
				setValue (key, std::dynamic_pointer_cast<valueT> (newEntry));
				return newEntry;
			}
		}
	
};

class UCSNamespace: public UCSContainer<std::string,UCSValue> {
	
	private:

		std::map<std::string, std::shared_ptr<UCSValue>> values;

	protected:
		
		std::shared_ptr<UCSValue> getValue (const std::string& key) const {
			std::map<std::string, std::shared_ptr<UCSValue>>::const_iterator it = values.find (key);
			return (it == values.end ()) ? std::shared_ptr<UCSValue> () : it -> second;
		}
		
		void setValue (const std::string& key, std::shared_ptr<UCSValue> value) {
			values.insert (std::pair<std::string, std::shared_ptr<UCSValue>> (key, value));
		}
		
};

template<typename T> class UCSPrimitiveType: public UCSValue {
	
	private:
		
		std::shared_ptr<UCSPrimitiveValue<T>> value;
		
	public:
		
		UCSPrimitiveType (): value (new UCSPrimitiveValue<T> ()) { }
		UCSPrimitiveType (UCSNamespace* nameSpace, const std::string& name): value (nameSpace -> get<UCSPrimitiveValue<T>> (name)) { }
		UCSPrimitiveType (std::shared_ptr<UCSNamespace> nameSpace, const std::string& name): value (nameSpace -> get<UCSPrimitiveValue<T>> (name)) { }
		
		UCSPrimitiveType (const UCSPrimitiveType<T>& rhs) { // copy constructor copies the reference, not the value
			value = rhs.value;
		}
		
		T get () const {
			return (*value);
		}
		
		void set (const T& newValue) {
			(*value) = newValue;
		}

		UCSPrimitiveType<T>& operator= (const UCSPrimitiveType<T>& rhs) {
			(*value) = (*rhs.value);
			return *this;
		}
		
		UCSPrimitiveType<T>& operator= (const T& rhs) {
			(*value) = rhs;
			return *this;
		}
		
		operator T () const {
			return T (*value);
		}

		std::string typeName() const {
			return value -> typeName ();
		}
		
		void addListener (const std::shared_ptr<UCSValueListener<T>>& listener) {
			value -> addListener (listener);
		}
		
};

using UCSInt = UCSPrimitiveType<int>;
using UCSString = UCSPrimitiveType<std::string>;

class UCSStruct: public UCSValue {
	
	protected:
		
		std::shared_ptr<UCSNamespace> fields;
		
	public:
		
		UCSStruct (): 
			fields (new UCSNamespace ()) 
			{ }
			
		UCSStruct (UCSNamespace* nameSpace, const std::string& name):
			fields (nameSpace -> get<UCSStruct> (name) -> fields)
			{ }
	
};

// FIXME: this is clearly over-engineering, need to be simplified. Dynamic arrays are crap for remoting

template<class valueT> class UCSArrayValue: public UCSContainer<std::size_t,valueT> {
	
	private:
		
		std::vector<std::shared_ptr<valueT>> elements;
		
	public:
		
		std::shared_ptr<valueT> getValue (const std::size_t& key) const {
			return (key < elements.size ()) ? elements[key] : std::shared_ptr<valueT> ();
		}
		
		void setValue (const std::size_t& key, std::shared_ptr<valueT> value) {
			resizeToFit (key);
			elements[key] = value;
		}
		
		void resizeToFit (const std::size_t& index) {
			
			if (index >= elements.size ()) {
				std::size_t oldSize = elements.size ();
				elements.resize (index+1);
				for (std::size_t i = oldSize; i < elements.size(); i++) {
					elements[i] = std::shared_ptr<valueT> (new valueT ());
				}
			}
		}
		
		std::size_t size() const {
			return elements.size ();
		}
	
};

template<class valueT> class UCSArray: public UCSValue {
	
	private:
		
		std::shared_ptr<UCSArrayValue<valueT>> arrayValue;
		
	public:
		
		UCSArray (): 
			arrayValue (new UCSArrayValue<valueT> ()) 
			{ }

		UCSArray (UCSNamespace* nameSpace, const std::string& name): 
			arrayValue (nameSpace -> get<UCSArray<valueT>> (name) -> arrayValue) 
			{ }
			
		valueT& operator[] (std::size_t idx) {
			arrayValue -> resizeToFit (idx);
			std::shared_ptr<valueT> value = arrayValue -> getValue ((int) idx);
			if (value)
				return *value;
			else
				throw std::runtime_error ("Array index out of range");
		}
		
		std::size_t size() const {
			return arrayValue -> size ();
		}
	
};


#endif /* __UCS_DM_H__ */


