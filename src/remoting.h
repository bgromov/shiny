#ifndef __REMOTING_H__
#define __REMOTING_H__

#include <map>
#include <memory>

#include "dm.h"

namespace UCS {

	using namespace std;

	using UCSID = int; // any integral type will do

	class UCSIDStore {

		private:

			UCSID nextId = 1;

			map<UCSID, weak_ptr<UCSValue>> values;

		public:

			UCSIDStore (shared_ptr<UCSValue> rootValue) {
				values.insert (pair<UCSID, weak_ptr<UCSValue>> (getRootID (), rootValue));
			}

			static UCSID getRootID () {
				return 0;
			}

			UCSID getNewID (weak_ptr<UCSValue> value) {

				UCSID newID;

				do {
					newID = (nextId++);
				} while (newID == getRootID() || (values.find (newID) != values.end ()));

				values.insert (pair<UCSID, weak_ptr<UCSValue>> (newID, value));
				return newID;
			}

			shared_ptr<UCSValue> removeID (const UCSID& id) {
				auto iterator = values.find (id);
				if (iterator != values.end ()) {
					shared_ptr<UCSValue> result = iterator -> second.lock();
					values.erase (iterator);
					return result;
				} else
					return shared_ptr<UCSValue> ();
			}

	};

	class UCSRemotingClient: public UCSEventHandler {

		private:

			shared_ptr<UCSIDStore> idStore;
			shared_ptr<UCSNamespace> rootNamespace;

			void trackValue (UCSID id, weak_ptr<UCSValue> weakValue) {

				shared_ptr<UCSValue> value = weakValue.lock();

				// must be careful not to hold the strong reference here somewhere in the captures, because
				// it will prevent the object from being released, ever

				if (value) {

					if (shared_ptr<UCSNamespace> newNamespace = dynamic_pointer_cast <UCSNamespace> (value)) {

						cout << "[id " << id << "] is new namespace" << endl;

						newNamespace -> onValueAdded.addListener(this, [=] (const string& key, weak_ptr<UCSValue> addedValue) {
							UCSID newID = idStore -> getNewID (addedValue);
							cout << "[namespace " << id << "] value (" << key << ") added: " << " id " << newID << endl;
							trackValue (newID, addedValue);
						});

					}

					if (shared_ptr<UCSSerializable> serializable = dynamic_pointer_cast <UCSSerializable> (value)) {
						serializable -> onChanged.addListener (this, [=] (UCSSerializable *p) { localValueChanged (id, p); });
					}

					// Lambdas are so nice, aren't they?
					value -> onDelete.addListener (this, [=] () { untrackValue (id); });
				}

			}

			void localValueChanged (UCSID id, UCSSerializable *p) {
				cout << "[id " << id << "] local value changed" << endl;
			}

			void untrackValue (UCSID id) {
				cout << "[id " << id << "] removing" << endl;
				shared_ptr<UCSValue> value = idStore -> removeID (id);
				if (value) {
					if (shared_ptr<UCSSerializable> serializable = dynamic_pointer_cast <UCSSerializable> (value))
						serializable -> onChanged.removeListeners (this);
					value -> onDelete.removeListeners(this);
				}
			}

		public:

			UCSRemotingClient (shared_ptr<UCSNamespace> nameSpace):
				rootNamespace (nameSpace)
			{
				idStore = shared_ptr<UCSIDStore> (new UCSIDStore (rootNamespace));
				trackValue (idStore -> getRootID(), rootNamespace);
			}

			~UCSRemotingClient ()
			{
				untrackValue (idStore -> getRootID());
			}


	};

}

#endif /* __REMOTING_H__ */

