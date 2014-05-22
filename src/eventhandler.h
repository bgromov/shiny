/*
 * eventhandler.h
 *
 *  Created on: Apr 28, 2014
 *      Author: 0xff
 */

#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include <functional>
#include <list>
#include <memory>

namespace UCS {

using namespace std;

class UCSEventHandler { };

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

}

#endif /* EVENTHANDLER_H_ */
