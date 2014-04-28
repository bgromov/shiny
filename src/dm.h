#ifndef __UCS_DM_H__
#define __UCS_DM_H__

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <list>
#include <functional>
#include <future>

#include "eventhandler.h"

namespace UCS {

	using namespace std;

	// for some events, functors are nice.
	// Q: why functors?
	// A: less clumsy because of value capture.
	// A (another): only applicable where you don't remove the functors, because functor has no reference.

	// Q: can we do this with a sort of type erasure?
	// A: maybe, let's try

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

	class UCSError: public runtime_error {

		public:

			UCSError (const string& errorMessage): runtime_error (errorMessage) { }
			UCSError (const UCSError& rhs): runtime_error (rhs.what()) { }

	};

	template<typename T> class UCSPrimitiveValue: public UCSSerializable {

		private:

			T value;

		public:

			UCSPrimitiveValue () { }
			UCSPrimitiveValue (const T& initValue): value (initValue) { }

			T getValue () const {
				return value;
			}

			void setValue (const T& newValue) {
				if (value != newValue) {
					value = newValue;
					onChanged.invoke (this);
				}
			}

	};

	// we have two containers: namespace and vector. Vector is type-agnostic.
	// could we have typesafe vector? generally yes, but easier to do with dynamic_cast
	// (also, we might decide that generic vector is useful, too)
	// (also, generic vector is useful for things like parameter list)
	// user interface for vector should be typesafe, of course

	class UCSNamespace: public UCSValue, public UCSEventHandler {

		private:

			// Q: why weak ptr?
			// A: because namespace does not own the items. interface objects do.

			map<string, weak_ptr<UCSValue>> values;

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
	
	// ---------- functions are tricky ----------------
	// Q: what's wrong with functions?
	// A: we have both native and managed implementations, and, at the same time, both native and managed callers.
	//    managed caller should be able to invoke native callee, and vice versa.
	// Q: can we wrap native implementation with managed?
	// A: maybe

	// --------- bare minimum for (managed) function -------
	// - name
	// - parameter list (array of types)
	// Q: what about structs as parameters? how typesafe they are?
	// A: no type safety on instantiation. binding will happen at the moment of execution, and that may throw
	// A: wrong, binding will happen at the moment of instantiating function call object, and that may throw;
	//    nevertheless, any structure may be bound to any structure, so.. UCSNamespace
	// Q: do we actually need to store types?
	// A: maybe not because UCSValue will take care of type checking

	// Q: why callbacks?
	// A: because we don't know how to wait for completion. function call may be invoked:
	//    - in the native code, where we would probably block the thread to wait
	//    - in managed code, where we would need to yield the fiber to wait
	//    - as a result of deserialization, where we wouldn't need to wait at all
	//    - in user interface thread, where we can't wait and we need callbacks anyway

	class UCSFunction: public UCSValue {

		private:

			const int numParams;

		public:

			UCSFunction (int p_numParams): numParams (p_numParams) { }

			virtual future<shared_ptr<UCSValue>> execute (const vector<shared_ptr<UCSValue>>& params) = 0;

	};

	// Q: What do we need UCSNativeFunction for, if we could subclass UCSFunction instead?
	// A: yes, but NativeFunction wraps functor or lambda, which provides cleaner syntax.
	//    of course, subclassing UCSFunction is valid use.

	namespace VarIndex {

		template<size_t... Indices>
		struct indices{
		  using next = indices<Indices..., sizeof...(Indices)>;
		};
		template<size_t N>
		struct build_indices{
		  using type = typename build_indices<N-1>::type::next;
		};
		template <>
		struct build_indices<0>{
		  using type = indices<>;
		};
		template<size_t N>
		using IndicesFor = typename build_indices<N>::type;

	}

	template<typename Result>
	class UCSNativeFunction;

	template<typename Result, typename ...Args>
	class UCSNativeFunction<Result(Args...)>: public UCSFunction {

		private:

			function<future<shared_ptr<UCSValue>>(Args...)> func;

			template<typename T, size_t Index>
			T get_param (const vector<shared_ptr<UCSValue>>& params) {
				return T (params[Index]);
			}

			template<size_t N, size_t... Is>
			future<shared_ptr<UCSValue>> invokeFunc (const vector<shared_ptr<UCSValue>>& params, VarIndex::indices<Is...>) {
				return func (get_param<Args,Is> (params)...);
			}

		public:

			UCSNativeFunction (function<future<shared_ptr<UCSValue>> (Args...)> p_func):
				UCSFunction (sizeof...(Args)),
				func (p_func)
				{ }

			future<shared_ptr<UCSValue>> execute (const vector<shared_ptr<UCSValue>>& params) {
				static constexpr size_t nArgs = sizeof...(Args);
				return invokeFunc<nArgs> (params, VarIndex::IndicesFor<nArgs> ());
			}
	};

	template<typename Result>
	class UCSNativeBlockingFunction;

	template<typename Result, typename ...Args>
	class UCSNativeBlockingFunction<Result(Args...)>: public UCSNativeFunction<Result(Args...)> {

		public:

			UCSNativeBlockingFunction (function<Result(Args...)> p_func):
				UCSNativeFunction<Result(Args...)> (
					[p_func] (Args... args) -> future<shared_ptr<UCSValue>> {
						promise<shared_ptr<UCSValue>> promise;
						try {
							Result r = p_func (args...);
							promise.set_value (r.getUCSValue ());
						} catch (...) {
							promise.set_exception(std::current_exception());
						}
						return promise.get_future();
					}
				)
			{ }

	};

	template<typename Result>
	class UCSNativeFunctionCall;

	template<typename Result, typename ...Args>
	class UCSNativeFunctionCall<Result(Args...)> {

		private:

			shared_ptr<UCSFunction> func;

			template<size_t I>
			future<shared_ptr<UCSValue>> execFunc (vector<shared_ptr<UCSValue>>& values) {
				return func -> execute (values);
			}

			template<size_t I, typename Head, typename ...Tail>
			future<shared_ptr<UCSValue>> execFunc (vector<shared_ptr<UCSValue>>& values, Head head, Tail... tail) {
				values[I] = head.getUCSValue ();
				return execFunc<I+1, Tail...> (values, tail...);
			}

		public:

			UCSNativeFunctionCall () { }

			UCSNativeFunctionCall (shared_ptr<UCSFunction> p_func):
				func (p_func) { }

			UCSNativeFunctionCall& operator= (const shared_ptr<UCSFunction>& rhs) {
				func = rhs;
				return *this;
			}

			future<shared_ptr<UCSValue>> executeAsync (Args... params) {
				vector<shared_ptr<UCSValue>> values (sizeof...(Args));
				return execFunc<0, Args...> (values, params...);
			}

			Result call (Args... params) {
				vector<shared_ptr<UCSValue>> values (sizeof...(Args));
				future<shared_ptr<UCSValue>> execFuture = execFunc<0, Args...> (values, params...);
				execFuture.wait ();
				return Result (execFuture.get ());
			}

	};


}


#endif /* __UCS_DM_H__ */


