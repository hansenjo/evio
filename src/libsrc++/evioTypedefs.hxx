#ifndef _evioTypedefs_hxx
#define _evioTypedefs_hxx


#include <string>
#include <list>
#include <map>
#include <memory>
#include <algorithm>


#ifdef vxworks
#include <iostream.h>
#else
#include <iostream>
#include <iomanip>
#endif

#include <evio.h>




//-----------------------------------------------------------------------------
//----------------------------- Typedefs --------------------------------------
//-----------------------------------------------------------------------------

namespace evio {

using namespace std;
using namespace evio;


// evio classes
class evioStreamParserHandler;
class evioStreamParser;
class evioDOMTree;
class evioDOMNode;
class evioDOMContainerNode;
template<typename T> class evioDOMLeafNode;
class evioSerializable;
template <typename T> class evioUtil;
class evioDictionary;
class evioToStringConfig;



//typedef boost::shared_ptr<evioDOMNode> evioDOMNodeP;          /** Node pointer returned by many functions, need to fix all new() calls.*/

typedef evioDOMNode* evioDOMNodeP;                   /**<Pointer to evioDOMNode, only way to access nodes.*/
typedef list<evioDOMNodeP>  evioDOMNodeList;         /**<List of pointers to evioDOMNode.*/
typedef auto_ptr<evioDOMNodeList> evioDOMNodeListP;  /**<auto-ptr of list of evioDOMNode pointers, returned by getNodeList.*/

//typedef boost::shared_ptr<evioDOMNodeList> evioDOMNodeListP;  /** List of node pointers returned by getNodeList.*/


/** Defines the container bank types.*/
typedef enum {
  BANK       = 0xe,  /**<2-word header, 16-bit tag, 8-bit num, 8-bit type.*/
  SEGMENT    = 0xd,  /**<1-word header,  8-bit tag,    no num, 8-bit type.*/
  TAGSEGMENT = 0xc   /**<1-word header, 12-bit tag,    no num, 4-bit type.*/
} ContainerType;

typedef pair<uint16_t,uint8_t> tagNum;     /**<STL pair of tag,num.*/

}

#endif

