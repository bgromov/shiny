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

			map<UCSID, shared_ptr<UCSValue>> values;

		public:

			UCSIDStore (shared_ptr<UCSValue> rootValue) {
				values.insert (std::pair<UCSID, std::shared_ptr<UCSValue>> (getRootID (), rootValue));
			}

			static UCSID getRootID () {
				return 0;
			}

			UCSID getNewID (shared_ptr<UCSValue> value) {

				UCSID newID;

				do {
					newID = (nextId++);
				} while (newID == getRootID() || (values.find (newID) != values.end ()));

				values.insert (std::pair<UCSID, std::shared_ptr<UCSValue>> (newID, value));
				return newID;
			}

	};

	class UCSRemotingClient {

		class NamespaceListener: public UCSNamespace::Listener {

			private:

				UCSRemotingClient *client;
				UCSID namespaceID;

			public:

				NamespaceListener (UCSRemotingClient *p_client, UCSID p_namespaceID):
					client (p_client),
					namespaceID (p_namespaceID)
				{ }

				void onValueAdded (const string& key, shared_ptr<UCSValue> value) {
					UCSID newID = client -> idStore -> getNewID (value);
					cout << "[namespace " << namespaceID << "] value (" << key << ") added: " << value << " id " << newID << endl;

					client -> trackValue (newID, value);
				}

		};

		private:

			shared_ptr<UCSIDStore> idStore;
			shared_ptr<UCSNamespace> rootNamespace;
			NamespaceListener* rootListener;

			void trackValue (UCSID id, shared_ptr<UCSValue> value) {
				if (shared_ptr<UCSNamespace> newNamespace = dynamic_pointer_cast <UCSNamespace> (value)) {
					cout << "[id " << id << "] is new namespace" << endl;
					NamespaceListener *newListener = new NamespaceListener (this, id);
					newNamespace -> addListener (newListener);
				}
			}

		public:

			UCSRemotingClient (shared_ptr<UCSNamespace> nameSpace):
				rootNamespace (nameSpace)
			{
				idStore = shared_ptr<UCSIDStore> (new UCSIDStore (rootNamespace));
				rootListener = new NamespaceListener (this, idStore -> getRootID ());
				nameSpace -> addListener (rootListener);
			}


	};

}

#endif /* __REMOTING_H__ */

