#ifndef DM_INTERFACE_H_
#define DM_INTERFACE_H_

#include "dm.h"

namespace UCS {

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

#endif /* DM_INTERFACE_H_ */
