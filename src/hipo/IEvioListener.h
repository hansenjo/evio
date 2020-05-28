//
// Created by Carl Timmer on 2020-05-27.
//

#ifndef EVIO_6_0_IEVIOLISTENER_H
#define EVIO_6_0_IEVIOLISTENER_H

#include <memory>

#include "BaseStructure.h"

namespace evio {


    /**
     * In SAX like behavior, implementors will listen for structures encountered when an event is parsed.
     *
     * @author heddle (Original Java class)
     * @author timmer
     *
     */
    class IEvioListener {

    public:

        /**
         * Called when a structure is read while parsing an event.
         *
         * NOTE: the user should NOT modify the arguments.
         *
         * @param topStructure the evio structure at the top of the search/parse
         * @param structure the full structure, including header
         */
        virtual void gotStructure(std::shared_ptr<BaseStructure> topStructure,
                                  std::shared_ptr<BaseStructure> structure) = 0;

        /**
         * Starting to parse a new evio structure.
         * @param structure the evio structure in question.
         */
        virtual void startEventParse(std::shared_ptr<BaseStructure> structure) = 0;

        /**
         * Done parsing a new evio structure.
         * @param structure the evio structure in question.
         */
        virtual void endEventParse(std::shared_ptr<BaseStructure> structure) = 0;

    };

}

#endif //EVIO_6_0_IEVIOLISTENER_H