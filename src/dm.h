#ifndef __UCS_DM_H__
#define __UCS_DM_H__

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <list>
#include <set>
#include <functional>
#include <type_traits>

namespace UCS {

	using namespace std;

	// for some events, functors are nice.
	// Q: why functors?
	// A: less clumsy because of value capture.
	// A (another): only applicable where you don't remove the functors, because functor has no reference.

	// Q: can we do this with a sort of type erasure?
	// A: maybe, let's try

	class UCSEventHandler {	};

	template<typename ...A> class UCSListenerList {

		private:

			list<pair<UCSEventHandler*,function<void(A...)>>> listeners;

		public:

			void addListener (UCSEventHandler *handler, function<void(A...)> listener) {
				listeners.push_back (pair<UCSEventHandler*,function<void(A...)>> (handler, listener));
			}

			void invoke (A... params) {
				for (auto listener: listeners)
					listener.second (params...);
			}

			void removeListeners (UCSEventHandler *handler) {
				// hardcore lambdas for the win. it works!
				listeners.remove_if ([handler] (pair<UCSEventHandler*,function<void(A...)>> element) -> bool {
					return element.first == handler;
				});
			}

	};

	class UCSValue {

		public:

			UCSListenerList<> onDelete;

			virtual ~UCSValue () {
				onDelete.invoke();
			}

		private:

		
	};

	/*
	 * UCSPrimitiveValue is what goes into container. Not to be instantiated directly.
	 *
	 * Q: Why not to put UCSInt into container directly?
	 * A: 1) Tracking references. UCSInt may disappear as client-side interface is freed.
	 *    2) Keeping uniform interface (UCSStruct cannot go into container, too, so why should UCSInt?)
	 *
	 * Q: Why UCSStruct cannot go into container?
	 * A: Because many UCSStruct instances may refer to single struct. See above for tracking references.
	 *
	 * General design rationale:
	 *
	 *    - every managed type (int, struct or whatever) has to be represented with two classes:
	 *       1) "value" type which goes into containers, with generic callbacks for serialization etc
	 *       	- "value" type is listenable for events
	 *       	- "value" type is generally serializable
	 *     	 2) "instance" type like UCSInt or UCSStruct which refers to "value" and provides client-side
	 *     	    interface for instantiation, etc. This also allows for easy reference tracking.
	 *
	 * Events:
	 *
	 *    - we need typesafe events for client-side listeners and generic events for serialization
	 *    - is there any case when client needs non-type-safe event? probably not, so functors possible
	 *    - could generic events be wrapped with functors? no, they have more than one method
	 *    - could typesafe events be wrapped with generic? likely, yes
	 *    - could typesafe events be implemented with functors over generic event? likely, yes
	 *
	 * Tracking references:
	 *
	 *    - we could use our own refcounts, but shared_ptr will do the job just fine. Destructor is our
	 *      callback.
	 *    Variant 1:
	 *    	 - container should not keep the shared_ptr<>, because destroying every interface should
	 *      	remove the value from container.
	 *       - therefore: primitive should know what container it belongs to, to remove itself. clumsy?
	 *    Variant 2:
	 *       - container interface releases references in container value. clumsy?
	 *
	 */

	class UCSSerializable: public UCSValue {

		public:

			UCSListenerList<UCSSerializable *> onChanged;

	};

	template<typename T> class UCSPrimitiveValue: public UCSSerializable {

		class Listener {
			public:
				virtual void onValueChanged (const T& value) = 0;
		};
		
		private:

			T value;
			list<Listener*> listeners;

		public:

			T getValue () const {
				return value;
			}

			void setValue (const T& newValue) {
				if (value != newValue) {
					value = newValue;
					for (auto listener: listeners)
						listener -> onValueChanged (newValue);
					onChanged.invoke (this);
				}
			}

			void addListener (Listener* listener) {
				listeners.push_back (listener);
			}

	};

	// we have two containers: namespace and vector. Vector is type-agnostic.
	// could we have typesafe vector? generally yes, but easier to do with dynamic_cast
	// (also, we might decide that generic vector is useful, too)
	// (also, generic vector is useful for things like parameter list)
	// user interface for vector should be typesafe, of course

	class UCSNamespace: public UCSValue, public UCSEventHandler {

		public:

			class Listener {
				public:
					virtual void onValueAdded (const string& key, shared_ptr<UCSValue> value) = 0;
			};

		private:

			// Q: why weak ptr?
			// A: because namespace does not own the items. interface objects do.

			map<string, weak_ptr<UCSValue>> values;
			list<Listener *> listeners;

			void remove (const string &key) {
				cout << "removing " << key << endl;
				values.erase (key);
			}

		public:

			UCSListenerList<const string &, weak_ptr<UCSValue>> onValueAdded;

			template<class T> shared_ptr<UCSValue> get (const string& key) {

				map<string, weak_ptr<UCSValue>>::const_iterator it = values.find (key);
				if (it != values.end ()) {
					shared_ptr<UCSValue> value = it -> second.lock ();
					if (value)
						return value;
				}

				shared_ptr<typename T::valueType> newValue (new typename T::valueType ());

				// Q: why can't you write "values.erase()" right in the lambda?
				// A: because of "at most one implicit conversion" rule, key, being a closure parameter, won't be
				//    automatically converted to string<basic_char>. we could convert it explicitly or call a function,
				//    the latter being cleaner.

				newValue -> onDelete.addListener (this, [=]() { remove (key); });
				weak_ptr<UCSValue> storedValue (newValue);
				values.insert (pair<string, weak_ptr<UCSValue>> (key, storedValue));
				onValueAdded.invoke (key, storedValue);
				return newValue;
			}

			void addListener (Listener* listener) {
				listeners.push_back (listener);
			}

			~UCSNamespace () {

				// generally, this should never happen. But, let's play nice and clean up after ourselves

				for (auto entry: values) {
					shared_ptr<UCSValue> value = entry.second.lock();
					if (value)
						value -> onDelete.removeListeners(this);
				}
			}

	};

	class UCSVector: public UCSValue {

		class Listener {
			public:
				virtual void onSizeChanged (size_t newSize) = 0;
				virtual void onValueSet (size_t index, shared_ptr<UCSValue> value) = 0;
		};

		private:

			vector<shared_ptr<UCSValue>> elements;
			list<Listener *> listeners;

			void resizeToFit (const size_t& index) {
				if (index >= elements.size ()) {
					size_t newSize = index + 1;
					elements.resize (newSize);
					for (auto listener: listeners)
						listener -> onSizeChanged (newSize);
				}
			}

		public:

			shared_ptr<UCSValue> getValue (const size_t index) {
				if (index < elements.size ())
					return elements[index];
				else
					return shared_ptr<UCSValue> ();
			}

			void putValue (const size_t index, shared_ptr<UCSValue> value) {
				resizeToFit (index);
				elements[index] = value;
				for (auto listener: listeners)
					listener -> onValueSet (index, value);
			}

			size_t size() const {
				return elements.size ();
			}

	};

	// ---- now for client-side interface. note that client-side interfaces are not derived from UCSValue.
	// Q: why?
	// A: client-side interfaces cannot be expected to serialize
	// A: containers should accept only well-defined types, not UCSSupaTroopaStruct
	// A: containers are not typesafe anyway

	template<typename T> class UCSPrimitive {

		public:

			typedef UCSPrimitiveValue<T> valueType;

		private:

			shared_ptr<valueType> value;

		public:

			UCSPrimitive ():
				value (new valueType ()) { }

			// copy constructor copies the reference, not the value
			UCSPrimitive (const valueType& rhs) {
				value = rhs.value;
			}

			UCSPrimitive (shared_ptr<UCSValue> p_value) {
				value = dynamic_pointer_cast<valueType> (p_value);
				if (!value)
					throw runtime_error ("Type mismatch");
			}

			T getValue () const {
				return value -> getValue ();
			}

			void setValue (const T& newValue) {
				value -> setValue (newValue);
			}
	
			UCSPrimitive<T>& operator= (const UCSPrimitive<T>& rhs) {
				setValue (rhs.getValue ());
				return *this;
			}

			UCSPrimitive<T>& operator= (const T& rhs) {
				setValue (rhs);
				return *this;
			}

			operator T () const {
				return getValue ();
			}

			shared_ptr<UCSValue> getUCSValue () {
				return dynamic_pointer_cast<UCSValue> (value);
			}

	};
	
	using UCSInt = UCSPrimitive<int>;
	using UCSString = UCSPrimitive<string>;
	
	class UCSStruct {
		
		public:

			typedef UCSNamespace valueType;

		protected:

			shared_ptr<UCSNamespace> fields;

		public:

			UCSStruct ():
				fields (new UCSNamespace ())
				{ }

			UCSStruct (shared_ptr<UCSValue> value) {
				fields = dynamic_pointer_cast<UCSNamespace> (value);
				if (!fields)
					throw runtime_error ("Type mismatch");
			}

			shared_ptr<UCSValue> getUCSValue () {
				return dynamic_pointer_cast<UCSValue> (fields);
			}
		
	};
	
	// client-side array is typesafe, but there are some quirks.
	// valueT is client-side interface, too, but UCSVector contains UCSValue types.
	// for structs, this problem does not arise because struct instantiates client-side wrappers itself.
	// array should instantiate the array of wrappers with client-side type
	// how about array of structs?
	// all too easy - wrapper type should be able to construct itself from UCSValue or throw.

	template<class valueT> class UCSArray: public UCSVector::Listener {
		
		public:

			typedef UCSVector valueType;

		private:

			shared_ptr<UCSVector> elements;
			vector<shared_ptr<valueT>> values;
	
			virtual void onSizeChanged (size_t newSize) {
				cout << "size changed " << newSize << endl;
			}

			virtual void onValueSet (size_t index, shared_ptr<UCSValue> value) {
				cout << "value set " << index << endl;
			}

		public:

			UCSArray ():
				elements (new UCSVector ())
				{ }

			UCSArray (shared_ptr<UCSValue> value)
			{
				elements = dynamic_pointer_cast<UCSVector> (value);
				if (!elements)
					throw runtime_error ("Type mismatch");

				size_t vectorSize = elements -> size ();
				values.resize (vectorSize);
				for (size_t i = 0; i < vectorSize; i++) {
					shared_ptr<UCSValue> value = elements -> getValue (i);
					if (value)
						values[i] = shared_ptr<valueT> (new valueT (value));
					else {
						shared_ptr<valueT> entry (new valueT ());
						elements -> putValue (i, entry -> getUCSValue ());
						values[i] = entry;
					}
				}
			}

			valueT& operator[] (size_t index) {

				if (index >= values.size ())
					values.resize (index+1);

				shared_ptr<valueT> value = values[index];
				if (value) {
					return *value;
				} else {
					shared_ptr<valueT> entry (new valueT ());
					elements -> putValue (index, entry -> getUCSValue ());
					values[index] = entry;
					return *entry;
				}

			}
			
			size_t size() const {
				return values.size ();
			}

			shared_ptr<UCSValue> getUCSValue () {
				return dynamic_pointer_cast<UCSValue> (elements);
			}

		
	};

	
}


#endif /* __UCS_DM_H__ */


