// evioUtil.hxx

//  Author:  Elliott Wolin, JLab, 7-dec-2006


// must do:
//   throw evioException
//   are copy constructors, clone(), and operator= needed?
//   check bufsize in toEVIOBuffer(), parseBank, etc.
//   const correctness
//   Doxygen comments

// should do:
//   standardize on unsigned int
//   getBuffer() and const?
//   mark node in evioDOMTree

//  would like to do:
//   serialize objects
//   cMsg channel
//   exception stack trace if supported on all platforms

// not sure:
//   const vs non-const node access in getNodeList()
//   scheme for exception type codes
//   namespaces?
//   remove auto_ptr<>?
//   AIDA interface?


#ifndef _evioUtil_hxx
#define _evioUtil_hxx


using namespace std;
#include <evio.h>
#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <functional>


// prototypes
class evioDOMTree;
class evioDOMNode;
class evioDOMContainerNode;
template<typename T> class evioDOMLeafNode;
class evioSerializable;




//-----------------------------------------------------------------------------
//----------------------------- Typedefs --------------------------------------
//-----------------------------------------------------------------------------


typedef list<const evioDOMNode*>  evioDOMNodeList;
typedef auto_ptr<evioDOMNodeList> evioDOMNodeListP;
typedef enum {
  BANK_t       = 0xe,
  SEGMENT_t    = 0xd,
  TAGSEGMENT_t = 0xc
} ContainerType;


//-----------------------------------------------------------------------------
//----------------------------- for stream API --------------------------------
//-----------------------------------------------------------------------------


class setEvioArraySize {public: int val; setEvioArraySize(int i) : val(i){} };


//-----------------------------------------------------------------------------
//--------------------------- evio Classes ------------------------------------
//-----------------------------------------------------------------------------


// basic evio exception class
class evioException {

public:
  evioException(int typ = 0, const string &txt = "", const string &aux = "");
  evioException(int typ, const string &txt, const string &file, int line);
  virtual string toString(void) const;


public:
  int type;
  string text;
  string auxText;

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


// evio channel interface
class evioChannel {

public:
  virtual void open(void) throw(evioException) = 0;
  virtual bool read(void) throw(evioException) = 0;
  virtual void write(void) throw(evioException) = 0;
  virtual void write(const unsigned long *myBuf) throw(evioException) = 0;
  virtual void write(const unsigned int* myBuf) throw(evioException) = 0;
  virtual void write(const evioChannel &channel) throw(evioException) = 0;
  virtual void write(const evioChannel *channel) throw(evioException) = 0;
  virtual void write(const evioDOMTree &tree) throw(evioException) = 0;
  virtual void write(const evioDOMTree *tree) throw(evioException) = 0;
  virtual void close(void) throw(evioException) = 0;

  // not sure if these should be part of the interface
  virtual const unsigned long *getBuffer(void) const throw(evioException) = 0;
  virtual int getBufSize(void) const = 0;

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//  wrapper around evio C library
class evioFileChannel : public evioChannel {

public:
  evioFileChannel(const string &fileName, const string &mode = "r", int size = 8192) throw(evioException);
  virtual ~evioFileChannel();

  void open(void) throw(evioException);
  bool read(void) throw(evioException);
  void write(void) throw(evioException);
  void write(const unsigned long *myBuf) throw(evioException);
  void write(const unsigned int *myBuf) throw(evioException);
  void write(const evioChannel &channel) throw(evioException);
  void write(const evioChannel *channel) throw(evioException);
  void write(const evioDOMTree &tree) throw(evioException);
  void write(const evioDOMTree *tree) throw(evioException);
  void close(void) throw(evioException);
  const unsigned long *getBuffer(void) const throw(evioException);
  int getBufSize(void) const;

  void ioctl(const string &request, void *argp) throw(evioException);
  string getFileName(void) const;
  string getMode(void) const;


private:
  string filename;
  string mode;
  int handle;
  unsigned long *buf;
  int bufSize;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


// interface defines node and leaf handlers for stream parsing of evio buffers
class evioStreamParserHandler {

public:
  virtual void *containerNodeHandler(int length, int tag, int contentType, int num, 
                            int depth, void *userArg) = 0;
  virtual void leafNodeHandler(int length, int tag, int contentType, int num, 
                           int depth, const void *data, void *userArg) = 0;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


// stream parser dispatches to handlers when node or leaf reached
class evioStreamParser {

public:
  void *parse(const unsigned long *buf, evioStreamParserHandler &handler, void *userArg)
    throw(evioException);
  
private:
  void *parseBank(const unsigned long *buf, int bankType, int depth, 
                  evioStreamParserHandler &handler, void *userArg) throw(evioException);

};


//--------------------------------------------------------------
//--------------------------------------------------------------


//  represents an evio node in memory
class evioDOMNode {

protected:
  evioDOMNode(evioDOMNode *parent, int tag, int num, int contentType) throw(evioException);
  evioDOMNode(int tag, int num, int contentType) throw(evioException);


public:
  virtual ~evioDOMNode(void);
  virtual evioDOMNode *clone(evioDOMNode *newParent) const = 0;


public:
  static evioDOMNode *createEvioDOMNode(int tag, int num, ContainerType cType=BANK_t) throw(evioException);
  static evioDOMNode *createEvioDOMNode(evioDOMNode *parent, int tag, int num, ContainerType cType=BANK_t) throw(evioException);
  template <typename T> static evioDOMNode *createEvioDOMNode(evioDOMNode *parent, int tag, int num, const vector<T> tVec)
    throw(evioException);
  template <typename T> static evioDOMNode *createEvioDOMNode(int tag, int num, const vector<T> tVec)
    throw(evioException);
  template <typename T> static evioDOMNode *createEvioDOMNode(evioDOMNode *parent, int tag, int num, const T* t, int len) throw(evioException);
  template <typename T> static evioDOMNode *createEvioDOMNode(int tag, int num, const T* t, int len) throw(evioException);


public:
  virtual void addTree(evioDOMTree &tree) throw(evioException);
  virtual void addTree(evioDOMTree *tree) throw(evioException);
  virtual void addNode(evioDOMNode *node) throw(evioException);
  template <typename T> void append(const vector<T> &tVec) throw(evioException);
  template <typename T> void append(T *tBuf, int len) throw(evioException);
  template <typename T> void replace(const vector<T> &tVec) throw(evioException);
  template <typename T> void replace(T *tBuf, int len) throw(evioException);


public:
  evioDOMNode& operator<<(evioDOMNode *node) throw(evioException);
  evioDOMNode& operator<<(evioDOMTree &tree) throw(evioException);
  evioDOMNode& operator<<(evioDOMTree *tree) throw(evioException);
  virtual bool operator==(int tag) const;
  virtual bool operator!=(int tag) const;
  template <typename T> evioDOMNode& operator<<(vector<T> &tVec) throw(evioException);


public:
  virtual const evioDOMNode *getParent(void) const;
  virtual evioDOMNode *getParent(void);
  virtual evioDOMNodeListP getChildList(void) const throw(evioException);
  template <typename T> const vector<T> *getVector(void) const throw(evioException);


public:
  virtual string toString(void) const = 0;
  virtual string getHeader(int depth) const = 0;
  virtual string getFooter(int depth) const = 0;
  bool isContainer(void) const;
  bool isLeaf(void) const;


public:
  evioDOMNode *parent;
  int tag;
  int num;
  int contentType;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//  represents an evio container node in memory
class evioDOMContainerNode : public evioDOMNode {

  friend class evioDOMNode;


private:
  evioDOMContainerNode(evioDOMNode *parent, int tag, int num, ContainerType cType) throw(evioException);
  evioDOMContainerNode(int tag, int num, ContainerType cType) throw(evioException);
  evioDOMContainerNode(const evioDOMContainerNode &cNode) throw (evioException);


public:
  virtual ~evioDOMContainerNode(void);
  evioDOMContainerNode *clone(evioDOMNode *newParent) const;


public:
  void addTree(evioDOMTree &tree) throw(evioException);
  void addTree(evioDOMTree *tree) throw(evioException);
  void addNode(evioDOMNode *node) throw(evioException);
  evioDOMNodeListP getChildList(void) const throw(evioException);



  //evioDOMContainerNode& operator<<(evioDOMNode *pNode) throw(evioException);

  // stream operators for object serializtion
//   evioDOMContainerNode& operator<<(char c) throw(evioException);
//   evioDOMContainerNode& operator<<(unsigned char uc) throw(evioException);
//   evioDOMContainerNode& operator<<(short s) throw(evioException);
//   evioDOMContainerNode& operator<<(unsigned short us) throw(evioException);
//   evioDOMContainerNode& operator<<(long l) throw(evioException);
//   evioDOMContainerNode& operator<<(int l) throw(evioException);
//   evioDOMContainerNode& operator<<(unsigned long ul) throw(evioException);
//   evioDOMContainerNode& operator<<(unsigned int ul) throw(evioException);
//   evioDOMContainerNode& operator<<(long long ll) throw(evioException);
//   evioDOMContainerNode& operator<<(unsigned long long ull) throw(evioException);
//   evioDOMContainerNode& operator<<(float f) throw(evioException);
//   evioDOMContainerNode& operator<<(double d) throw(evioException);
//   evioDOMContainerNode& operator<<(string &s) throw(evioException);
//   evioDOMContainerNode& operator<<(long *lp) throw(evioException);
//   evioDOMContainerNode& operator<<(int *lp) throw(evioException);
//   evioDOMContainerNode& operator<<(evioSerializable &e) throw(evioException);
//   template <typename T> evioDOMContainerNode& operator<<(const vector<T> &v) throw(evioException);


public:
  string toString(void) const;
  string getHeader(int depth) const;
  string getFooter(int depth) const;


public:
  list<evioDOMNode*> childList;

private:
  list<evioDOMNode*>::iterator mark;  // marks current item to stream out
  int streamArraySize;

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//  represents an evio leaf node in memory
template <typename T> class evioDOMLeafNode : public evioDOMNode {

  friend class evioDOMNode;


private:
  evioDOMLeafNode(evioDOMNode *par, int tg, int num, const vector<T> &v) throw(evioException);
  evioDOMLeafNode(int tg, int num, const vector<T> &v) throw(evioException);
  evioDOMLeafNode(evioDOMNode *par, int tg, int num, const T *p, int ndata) throw(evioException);
  evioDOMLeafNode(int tg, int num, const T *p, int ndata) throw(evioException);

  evioDOMLeafNode(const evioDOMLeafNode<T> &lNode) throw(evioException);


public:
  virtual ~evioDOMLeafNode(void);
  evioDOMLeafNode<T>* clone(evioDOMNode *newParent) const;


public:
  //  evioDOMLeafNode<T>& operator=(const evioDOMLeafNode<T> &rhs) throw(evioException);


public:
  string toString(void) const;
  string getHeader(int depth) const;
  string getFooter(int depth) const;


public:
  vector<T> data;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//  manages tree representation of evio event in memory
class evioDOMTree : public evioStreamParserHandler {

public:
  evioDOMTree(const evioChannel &channel, const string &name = "evio") throw(evioException);
  evioDOMTree(const evioChannel *channel, const string &name = "evio") throw(evioException);
  evioDOMTree(const unsigned long *buf, const string &name = "evio") throw(evioException);
  evioDOMTree(const unsigned int *buf, const string &name = "evio") throw(evioException);
  evioDOMTree(evioDOMNode *node, ContainerType rootType=BANK_t, const string &name = "evio") throw(evioException);
  evioDOMTree(int tag, int num, ContainerType cType=BANK_t, ContainerType rootType=BANK_t, const string &name = "evio")
    throw(evioException);

  evioDOMTree(const evioDOMTree &tree) throw(evioException);
  virtual ~evioDOMTree(void);


public:
  void addTree(evioDOMTree &tree) throw(evioException);
  void addTree(evioDOMTree *tree) throw(evioException);
  void addBank(evioDOMNode* node) throw(evioException);
  template <typename T> void addBank(int tag, int num, const vector<T> dataVec) throw(evioException);
  template <typename T> void addBank(int tag, int num, const T* dataBuf, int dataLen) throw(evioException);


public:
  evioDOMTree& operator<<(evioDOMTree &tree) throw(evioException);
  evioDOMTree& operator<<(evioDOMTree *tree) throw(evioException);
  evioDOMTree& operator<<(evioDOMNode *node) throw(evioException);


public:
  void toEVIOBuffer(unsigned long *buf, int size) const throw(evioException);
  void toEVIOBuffer(unsigned int *buf, int size) const throw(evioException);


public:
  evioDOMNodeListP getNodeList(void) const throw(evioException);
  template <class Predicate> evioDOMNodeListP getNodeList(Predicate pred) const throw(evioException);


public:
  string toString(void) const;


private:
  evioDOMNode *parse(const unsigned long *buf) throw(evioException);
  int toEVIOBuffer(unsigned long *buf, const evioDOMNode *pNode, int size) const throw(evioException);
  template <class Predicate> evioDOMNodeList *addToNodeList(const evioDOMNode *pNode,evioDOMNodeList *pList, Predicate pred) const
    throw(evioException);
  
  void toOstream(ostream &os, const evioDOMNode *node, int depth) const throw(evioException);
  void *containerNodeHandler(int length, int tag, int contentType, int num, int depth, void *userArg);
  void leafNodeHandler(int length, int tag, int contentType, int num, int depth, const void *data, void *userArg);


public:
  evioDOMNode *root;
  string name;
  ContainerType rootType;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


// evio interface for generic object serialization
class evioSerializable {

public:
  virtual void serialize(evioDOMNode &cNode) const throw(evioException) = 0;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


// include templates
#include <evioUtilTemplates.hxx>

#endif
