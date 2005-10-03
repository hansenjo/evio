/*
 *  evioUtil.cc
 *
 *   Assorted evio utilities, C++ version
 *
 *   Author:  Elliott Wolin, JLab, 31-aug-2005
*/


#include <evioUtil.hxx>



//--------------------------------------------------------------
//-------------------- local variables -------------------------
//--------------------------------------------------------------

#define BANK 0xe

static bool debug = true;


//--------------------------------------------------------------
//----------------------local utilities ------------------------
//--------------------------------------------------------------


static string getIndent(int depth) {
  string s;
  for(int i=0; i<depth; i++) s+="   ";
  return(s);
}

//--------------------------------------------------------------

template <typename T> static void deleteIt(T *t) {
  delete(t);
}


//--------------------------------------------------------------
//---------------------- evioException -------------------------
//--------------------------------------------------------------


evioException::evioException(int typ, const string &txt, const string &aux) 
  : type(typ), text(txt), auxText(aux) {
}


//--------------------------------------------------------------


evioException::evioException(int typ, const string &txt, const string &file, int line) 
  : type(typ), text(txt) {

  ostringstream oss;
  oss <<  "    evioException occured in file " << file << ", line " << line << ends;
  auxText=oss.str();
}


//--------------------------------------------------------------


string evioException::toString(void) const {
  ostringstream oss;
  oss << "?evioException type = " << type << "    text = " << text << endl << endl << auxText << endl;
  return(oss.str());
}


//-----------------------------------------------------------------------
//---------------------------- evioFile ---------------------------------
//-----------------------------------------------------------------------


evioFile::evioFile(const string &f, const string &m, int size) throw(evioException*) 
  : filename(f), mode(m), bufSize(size), buf(NULL), handle(0) {

  // allocate buffer
  buf = new unsigned long[bufSize];
  if(buf==NULL)throw(new evioException(0,"?evioFile constructor...unable to allocate buffer",__FILE__,__LINE__));
}


//-----------------------------------------------------------------------


evioFile::~evioFile(void) {
  if(buf!=NULL)delete(buf);
}


//-----------------------------------------------------------------------



void evioFile:: open(void) throw(evioException*) {

  if(buf==NULL)throw(new evioException(0,"evioFile::open...null buffer",__FILE__,__LINE__));
  if(evOpen(const_cast<char*>(filename.c_str()),const_cast<char*>(mode.c_str()),&handle)<0)
    throw(new evioException(0,"?evioFile::open...unable to open file",__FILE__,__LINE__));
  if(handle==0)throw(new evioException(0,"?evioFile::open...zero handle",__FILE__,__LINE__));
}


//-----------------------------------------------------------------------


bool evioFile::read(void) throw(evioException*) {
  if(buf==NULL)throw(new evioException(0,"evioFile::read...null buffer",__FILE__,__LINE__));
  return(evRead(handle,&buf[0],bufSize)==0);
}


//-----------------------------------------------------------------------


void evioFile::write(void) throw(evioException*) {
  if(buf==NULL)throw(new evioException(0,"evioFile::write...null buffer",__FILE__,__LINE__));
  if(evWrite(handle,buf)!=0) throw(new evioException(0,"?evioFile::write...unable to write",__FILE__,__LINE__));
}


//-----------------------------------------------------------------------


void evioFile::ioctl(const string &request, void *argp) throw(evioException*) {
  if(evIoctl(handle,const_cast<char*>(request.c_str()),argp)!=0)
    throw(new evioException(0,"?evioFile::ioCtl...error return",__FILE__,__LINE__));
}


//-----------------------------------------------------------------------


void evioFile::close(void) throw(evioException*) {
  evClose(handle);
}


//-----------------------------------------------------------------------


string evioFile::getFileName(void) const {
  return(filename);
}


//-----------------------------------------------------------------------


string evioFile::getMode(void) const {
  return(mode);
}


//-----------------------------------------------------------------------


const unsigned long *evioFile::getBuffer(void) const throw(evioException*) {
  if(buf==NULL)throw(new evioException(0,"evioFile::getbuffer...null buffer",__FILE__,__LINE__));
  return(buf);
}


//-----------------------------------------------------------------------



int evioFile::getBufSize(void) const {
  return(bufSize);
}


//-----------------------------------------------------------------------
//------------------------ evioStreamParser -----------------------------
//-----------------------------------------------------------------------


void *evioStreamParser::parse(const unsigned long *buf, 
                              evioStreamParserHandler &handler, void *userArg) throw(evioException*) {
  
  if(buf==NULL)throw(new evioException(0,"?evioStreamParser::parse...null buffer",__FILE__,__LINE__));

  void *newUserArg = parseBank(buf,BANK,0,handler,userArg);
  return(newUserArg);
}


//--------------------------------------------------------------


void *evioStreamParser::parseBank(const unsigned long *buf, int bankType, int depth, 
                                 evioStreamParserHandler &handler, void *userArg) throw(evioException*) {

  int length,tag,contentType,num,dataOffset,p,bankLen;
  void *newUserArg = userArg;
  const unsigned long *data;


  /* get type-dependent info */
  switch(bankType) {
  case 0xe:
  case 0x10:
    length  	= buf[0]+1;
    tag     	= buf[1]>>16;
    contentType	= (buf[1]>>8)&0xff;
    num     	= buf[1]&0xff;
    dataOffset  = 2;
    break;

  case 0xd:
  case 0x20:
    length  	= (buf[0]&0xffff)+1;
    tag     	= buf[0]>>24;
    contentType = (buf[0]>>16)&0xff;
    num     	= 0;
    dataOffset  = 1;
    break;
    
  case 0xc:
  case 0x40:
    length  	= (buf[0]&0xffff)+1;
    tag     	= buf[0]>>20;
    contentType	= (buf[0]>>16)&0xf;
    num     	= 0;
    dataOffset  = 1;
    break;

  default:
    ostringstream ss;
    ss << hex << showbase << bankType << ends;
    throw(new evioException(0,"?evioStreamParser::parseBank...illegal bank type: " + ss.str(),__FILE__,__LINE__));
  }


  /* 
   * if a leaf node, call leaf handler.
   * if container node, call node handler and then parse contained banks.
   */
  switch (contentType) {

  case 0x0:
  case 0x1:
  case 0x2:
  case 0xb:
    handler.leafHandler(length-dataOffset,tag,contentType,num,depth,
                        &buf[dataOffset],userArg);
    break;

  case 0x3:
  case 0x6:
  case 0x7:
    handler.leafHandler((length-dataOffset)*4,tag,contentType,num,depth,
                        (char*)(&buf[dataOffset]),userArg);
    break;

  case 0x4:
  case 0x5:
    handler.leafHandler((length-dataOffset)*2,tag,contentType,num,depth,
                        (short*)(&buf[dataOffset]),userArg);
    break;

  case 0x8:
  case 0x9:
  case 0xa:
    handler.leafHandler((length-dataOffset)/2,tag,contentType,num,
                        depth,(long long*)(&buf[dataOffset]),userArg);
    break;

  case 0xe:
  case 0x10:
  case 0xd:
  case 0x20:
  case 0xc:
  case 0x40:
    // call node handler and get new userArg
    newUserArg=handler.nodeHandler(length,tag,contentType,num,depth,userArg);


    // parse contained banks
    p       = 0;
    bankLen = length-dataOffset;
    data    = &buf[dataOffset];

    depth++;
    switch (contentType) {
    case 0xe:
    case 0x10:
      while(p<bankLen) {
        parseBank(&data[p],contentType,depth,handler,newUserArg);
        p+=data[p]+1;
      }
      break;

    case 0xd:
    case 0x20:
      while(p<bankLen) {
        parseBank(&data[p],contentType,depth,handler,newUserArg);
        p+=(data[p]&0xffff)+1;
      }
      break;

    case 0xc:
    case 0x40:
      while(p<bankLen) {
        parseBank(&data[p],contentType,depth,handler,newUserArg);
        p+=(data[p]&0xffff)+1;
      }
      break;
    }
    depth--;
    break;


  default:
    ostringstream ss;
    ss << hex << showbase << contentType << ends;
    throw(new evioException(0,"?evioStreamParser::parseBank...illegal content type: " + ss.str(),__FILE__,__LINE__));
    break;

  }  // end main switch(contentType)


  // new user arg is pointer to parent node
  return(newUserArg);
}


//--------------------------------------------------------------
//---------------------- evioDOMTree ---------------------------
//--------------------------------------------------------------


evioDOMTree::evioDOMTree(const evioChannel &channel, const string &n) throw(evioException*) {
  const unsigned long *buf = channel.getBuffer();
  if(buf==NULL)throw(new evioException(0,"?evioDOMTree constructor...channel delivered null buffer",__FILE__,__LINE__));
  name=n;
  root=parse(buf);
}


//-----------------------------------------------------------------------------


evioDOMTree::evioDOMTree(const unsigned long *buf, const string &n) throw(evioException*) {
  if(buf==NULL)throw(new evioException(0,"?evioDOMTree constructor...null buffer",__FILE__,__LINE__));
  name=n;
  root=parse(buf);
}


//-----------------------------------------------------------------------------


evioDOMTree::evioDOMTree(const evioDOMNode *node, const string &n) throw(evioException*) {
  if(node==NULL)throw(new evioException(0,"?evioDOMTree constructor...null evioDOMNode",__FILE__,__LINE__));
  name=n;
  root=node->clone(NULL);
}


//-----------------------------------------------------------------------------


evioDOMTree::evioDOMTree(const evioDOMTree &t) throw(evioException*) {
  name=t.name;
  root=t.getRoot()->clone(NULL);
}


//-----------------------------------------------------------------------------


evioDOMTree &evioDOMTree::operator=(const evioDOMTree &rhs) throw(evioException*) {

  cout << "DOMTree operator= called" << endl;

  if(&rhs!=this) {
    name=rhs.name;
    root=rhs.getRoot()->clone(NULL);
  }

  return(*this);
}


//-----------------------------------------------------------------------------


evioDOMTree::~evioDOMTree(void) {
  delete(root);
}


//-----------------------------------------------------------------------------


evioDOMNode *evioDOMTree::parse(const unsigned long *buf) throw(evioException*) {
  evioStreamParser p;
  return((evioDOMNode*)p.parse(buf,*this,NULL));
}


//-----------------------------------------------------------------------------


void *evioDOMTree::nodeHandler(int length, int tag, int contentType, int num, int depth, 
                               void *userArg) {
    
  if(debug) printf("node   depth %2d  contentType,tag,num,length:  0x%-6x %6d %6d %6d\n",
                   depth,contentType,tag,num,length);
    
    
  // get parent pointer
  evioDOMContainerNode *parent = (evioDOMContainerNode*)userArg;
    

  // create new node
  evioDOMContainerNode *newNode = new evioDOMContainerNode(parent,tag,contentType,num);


  // add new node to parent's list
  if(parent!=NULL)parent->childList.push_back(newNode);


  // return new userArg that points to the new node
  return((void*)newNode);
}
  
  
//-----------------------------------------------------------------------------


void evioDOMTree::leafHandler(int length, int tag, int contentType, int num, int depth, 
                              const void *data, void *userArg) {

  if(debug) printf("leaf   depth %2d  contentType,tag,num,length:  0x%-6x %6d %6d %6d\n",
                   depth,contentType,tag,num,length);


  // get parent pointer
  evioDOMContainerNode *parent = (evioDOMContainerNode*)userArg;


  // create and fill new leaf
  evioDOMNode *newLeaf;
  string s;
  ostringstream os;
  switch (contentType) {

  case 0x0:
  case 0x1:
    newLeaf = new evioDOMLeafNode<unsigned long>(parent,tag,contentType,num,(unsigned long*)data,length);
    break;
      
  case 0x2:
    newLeaf = new evioDOMLeafNode<float>(parent,tag,contentType,num,(float*)data,length);
    break;
      
  case 0x3:
    for(int i=0; i<length; i++) os << ((char *)data)[i];
    os << ends;
    s=os.str();
    newLeaf = new evioDOMLeafNode<string>(parent,tag,contentType,num,&s,1);
    break;

  case 0x4:
    newLeaf = new evioDOMLeafNode<short>(parent,tag,contentType,num,(short*)data,length);
    break;

  case 0x5:
    newLeaf = new evioDOMLeafNode<unsigned short>(parent,tag,contentType,num,(unsigned short*)data,length);
    break;

  case 0x6:
    newLeaf = new evioDOMLeafNode<signed char>(parent,tag,contentType,num,(signed char*)data,length);
    break;

  case 0x7:
    newLeaf = new evioDOMLeafNode<unsigned char>(parent,tag,contentType,num,(unsigned char*)data,length);
    break;

  case 0x8:
    newLeaf = new evioDOMLeafNode<double>(parent,tag,contentType,num,(double*)data,length);
    break;

  case 0x9:
    newLeaf = new evioDOMLeafNode<long long>(parent,tag,contentType,num,(long long*)data,length);
    break;

  case 0xa:
    newLeaf = new evioDOMLeafNode<unsigned long long>(parent,tag,contentType,num,(unsigned long long*)data,length);
    break;

  case 0xb:
    newLeaf = new evioDOMLeafNode<long>(parent,tag,contentType,num,(long*)data,length);
    break;

  default:
    ostringstream ss;
    ss << hex << showbase << contentType<< ends;
    throw(new evioException(0,"?evioDOMTree::leafHandler...illegal content type: " + ss.str(),__FILE__,__LINE__));
    break;
  }


  // add new leaf to parent list
  if(parent!=NULL)parent->childList.push_back(newLeaf);
}


//-----------------------------------------------------------------------------


void evioDOMTree::toEVIOBuffer(unsigned long *buf) const throw(evioException*) {
  toEVIOBuffer(buf,root);
}


//-----------------------------------------------------------------------------


int evioDOMTree::toEVIOBuffer(unsigned long *buf, const evioDOMNode *pNode) const throw(evioException*) {

  int bankLen,bankType,dataOffset;


  if(pNode->getParent()==NULL) {
    bankType=BANK;
  } else {
    bankType=pNode->getParent()->contentType;
  }


  // add bank header word(s)
  switch (bankType) {
  case 0xe:
  case 0x10:
    buf[0]=0;
    buf[1] = (pNode->tag<<16) | (pNode->contentType<<8) | pNode->num;
    dataOffset=2;
    break;
  case 0xd:
  case 0x20:
    buf[0] = (pNode->tag<<24) | ( pNode->contentType<<16);
    dataOffset=1;
    break;
  case 0xc:
  case 0x40:
    buf[0] = (pNode->tag<<20) | ( pNode->contentType<<16);
    dataOffset=1;
    break;
  default:
    ostringstream ss;
    ss << hex << showbase << bankType<< ends;
    throw(new evioException(0,"evioDOMTree::toEVIOBuffer...illegal bank type in boilerplate: " + ss.str(),__FILE__,__LINE__));
    break;
  }


  // set starting length
  bankLen=dataOffset;


  // if container node loop over contained nodes
  const evioDOMContainerNode *c = dynamic_cast<const evioDOMContainerNode*>(pNode);
  if(c!=NULL) {
    list<evioDOMNode*>::const_iterator iter;
    for(iter=c->childList.begin(); iter!=c->childList.end(); iter++) {
      bankLen+=toEVIOBuffer(&buf[bankLen],*iter);
    }


  // leaf node...copy data, and don't forget to pad!
  } else {

    int nword,ndata,i;
    switch (pNode->contentType) {

    case 0x0:
    case 0x1:
    case 0x2:
    case 0xb:
      {
        const evioDOMLeafNode<unsigned long> *leaf = static_cast<const evioDOMLeafNode<unsigned long>*>(pNode);
        ndata = leaf->data.size();
        nword = ndata;
        for(i=0; i<ndata; i++) buf[dataOffset+i]=(unsigned long)(leaf->data[i]);
      }
      break;
      
    case 0x3:
      {
        const evioDOMLeafNode<string> *leaf = static_cast<const evioDOMLeafNode<string>*>(pNode);
        string s = leaf->data[0];
        ndata - s.size();
        nword = (ndata+3)/4;
        unsigned char *c = (unsigned char*)&buf[dataOffset];
        for(i=0; i<ndata; i++) c[i]=(s.c_str())[i];
        for(i=ndata; i<ndata+(4-ndata%4)%4; i++) c[i]='\0';
      }
      break;

    case 0x4:
    case 0x5:
      {
        const evioDOMLeafNode<unsigned short> *leaf = static_cast<const evioDOMLeafNode<unsigned short>*>(pNode);
        ndata = leaf->data.size();
        nword = (ndata+1)/2;
        unsigned short *s = (unsigned short *)&buf[dataOffset];
        for(i=0; i<ndata; i++) s[i]=static_cast<unsigned short>(leaf->data[i]);
        if((ndata%2)!=0)buf[ndata]=0;
      }
      break;

    case 0x6:
    case 0x7:
      {
        const evioDOMLeafNode<unsigned char> *leaf = static_cast<const evioDOMLeafNode<unsigned char>*>(pNode);
        ndata = leaf->data.size();
        nword = (ndata+3)/4;
        unsigned char *c = (unsigned char*)&buf[dataOffset];
        for(i=0; i<ndata; i++) c[i]=static_cast<unsigned char>(leaf->data[i]);
        for(i=ndata; i<ndata+(4-ndata%4)%4; i++) c[i]='\0';
      }
      break;

    case 0x8:
    case 0x9:
    case 0xa:
      {
        const evioDOMLeafNode<unsigned long long> *leaf = static_cast<const evioDOMLeafNode<unsigned long long>*>(pNode);
        ndata = leaf->data.size();
        nword = ndata*2;
        unsigned long long *ll = (unsigned long long*)&buf[dataOffset];
        for(i=0; i<ndata; i++) ll[i]=static_cast<unsigned long long>(leaf->data[i]);
      }
      break;
    default:
      ostringstream ss;
      ss << pNode->contentType<< ends;
      throw(new evioException(0,"?evioDOMTree::toEVOIBuffer...illegal leaf type: " + ss.str(),__FILE__,__LINE__));
      break;
    }

    // increment data count
    bankLen+=nword;
  }


  // finaly set node length in buffer
  switch (bankType) {
  case 0xe:
  case 0x10:
    buf[0]=bankLen-1;
    break;
  case 0xd:
  case 0x20:
  case 0xc:
  case 0x40:
    if((bankLen-1)>0xffff)throw(new evioException(0,"?evioDOMTree::toEVIOVuffer...length too long for segment type",__FILE__,__LINE__));
    buf[0]|=bankLen-1;
    break;
  default: 
    ostringstream ss;
    ss << bankType<< ends;
    throw(new evioException(0,"?evioDOMTree::toEVIOVuffer...illegal bank type setting length: " + ss.str(),__FILE__,__LINE__));
    break;
  }


  // return length of this bank
  return(bankLen);
}


//-----------------------------------------------------------------------------


const evioDOMNode *evioDOMTree::getRoot(void) const {
  return(root);
}


//-----------------------------------------------------------------------------


evioDOMNodeListP evioDOMTree::getNodeList(void) const throw(evioException*) {
  return(evioDOMNodeListP(addToNodeList(root,new evioDOMNodeList)));
}


//-----------------------------------------------------------------------------


evioDOMNodeList *evioDOMTree::addToNodeList(const evioDOMNode *pNode, evioDOMNodeList *pList) const
  throw(evioException*) {

  if(pNode==NULL)return(pList);


  // add this node to list
  pList->push_back(pNode);
  
  
  // add children to list
  const evioDOMContainerNode *c = dynamic_cast<const evioDOMContainerNode*>(pNode);
  if(c!=NULL) {
   list<evioDOMNode*>::const_iterator iter;
    for(iter=c->childList.begin(); iter!=c->childList.end(); iter++) {
      addToNodeList(*iter,pList);
    }
  }


  // return the list
  return(pList);
}


//-----------------------------------------------------------------------------


evioDOMNodeListP evioDOMTree::getLeafNodeList(void) const throw(evioException*) {
  evioDOMNodeListP pList = getNodeList();
  pList->erase(remove_if(pList->begin(),pList->end(),isContainerType()),pList->end());
  return(pList);
}


//-----------------------------------------------------------------------------


evioDOMNodeListP evioDOMTree::getContainerNodeList(void) const throw(evioException*) {
  evioDOMNodeListP pList = getNodeList();
  pList->erase(remove_if(pList->begin(),pList->end(),isLeafType()),pList->end());
  return(pList);
}


//-----------------------------------------------------------------------------


string evioDOMTree::toString(void) const {

  if(root==NULL)return("");

  ostringstream os;
  os << endl << endl << "<!-- Dump of tree: " << name << " -->" << endl << endl;
  toOstream(os,root,0);
  os << endl << endl;
  return(os.str());

}


//-----------------------------------------------------------------------------


void evioDOMTree::toOstream(ostream &os, const evioDOMNode *pNode, int depth) const throw(evioException*) {

  
  if(pNode==NULL)return;


  // get node header
  os << pNode->getHeader(depth);


  // dump contained banks if node is a container
  const evioDOMContainerNode *c = dynamic_cast<const evioDOMContainerNode*>(pNode);
  if(c!=NULL) {
    list<evioDOMNode*>::const_iterator iter;
    for(iter=c->childList.begin(); iter!=c->childList.end(); iter++) {
      toOstream(os,*iter,depth+1);
    }
  }


  // get footer
  os << pNode->getFooter(depth);
}


//-----------------------------------------------------------------------------
//---------------------------- evioDOMNode ------------------------------------
//-----------------------------------------------------------------------------


evioDOMNode::evioDOMNode(evioDOMNode *par, int tg, int content, int n) throw(evioException*)
  : parent(par), tag(tg), contentType(content), num(n)  {
}


//-----------------------------------------------------------------------------


bool evioDOMNode::operator==(int tag) const {
  return(this->tag==tag);
}


//-----------------------------------------------------------------------------


bool evioDOMNode::operator!=(int tag) const {
  return(this->tag!=tag);
}


//-----------------------------------------------------------------------------


const evioDOMNode *evioDOMNode::getParent(void) const {
  return(parent);
}


//-----------------------------------------------------------------------------


bool evioDOMNode::isContainer(void) const {
  return(::isContainer(contentType)==1);
}


//-----------------------------------------------------------------------------


bool evioDOMNode::isLeaf(void) const {
  return(::isContainer(contentType)==0);
}


//-----------------------------------------------------------------------------
//----------------------- evioDOMContainerNode --------------------------------
//-----------------------------------------------------------------------------


evioDOMContainerNode::evioDOMContainerNode(evioDOMNode *par, int tg, int content, int n) throw(evioException*)
  : evioDOMNode(par,tg,content,n) {
}


//-----------------------------------------------------------------------------


evioDOMContainerNode::evioDOMContainerNode(const evioDOMContainerNode &cNode) throw(evioException*) 
  : evioDOMNode(NULL,cNode.tag,cNode.contentType,cNode.num)  {

  cout << "container node copy constructor called" << endl;

  // copy contents of child list
  copy(cNode.childList.begin(),cNode.childList.end(),inserter(childList,childList.begin()));
}


//-----------------------------------------------------------------------------


evioDOMContainerNode &evioDOMContainerNode::operator=(const evioDOMContainerNode &rhs) throw(evioException*) {

  cout << "container node operator= called" << endl;

  if(&rhs!=this) {
    parent=rhs.parent;
    tag=rhs.tag;
    contentType=rhs.contentType;
    num=rhs.num;

    copy(rhs.childList.begin(),rhs.childList.end(),inserter(childList,childList.begin()));
  }

  return(*this);
}


//-----------------------------------------------------------------------------


evioDOMContainerNode *evioDOMContainerNode::clone(evioDOMNode *newParent) const {

  cout << "container node clone called" << endl;
  
  evioDOMContainerNode *c = new evioDOMContainerNode(newParent,tag,contentType,num);

  list<evioDOMNode*>::const_iterator iter;
  for(iter=childList.begin(); iter!=childList.end(); iter++) {
    c->childList.push_back((*iter)->clone(c));
  }
  return(c);
}


//-----------------------------------------------------------------------------

                                   
string evioDOMContainerNode::toString(void) const {
  ostringstream os;
  os << getHeader(0) << getFooter(0);
  return(os.str());
}


//-----------------------------------------------------------------------------

                                   
string evioDOMContainerNode::getHeader(int depth) const {
  ostringstream os;
  os << getIndent(depth)
     <<  "<" << get_typename(parent==NULL?BANK:parent->contentType) << " content=\"" << get_typename(contentType)
     << "\" data_type=\"" << hex << showbase << contentType
     << dec << "\" tag=\""  << tag;
  if((parent==NULL)||((parent->contentType==0xe)||(parent->contentType==0x10))) os << dec << "\" num=\"" << num;
  os << "\">" << endl;
  return(os.str());
}


//-----------------------------------------------------------------------------

                                   
string evioDOMContainerNode::getFooter(int depth) const {
  ostringstream os;
  os << getIndent(depth) << "</" << get_typename(this->parent==NULL?BANK:this->parent->contentType) << ">" << endl;
  return(os.str());
}


//-----------------------------------------------------------------------------

                                   
evioDOMContainerNode::~evioDOMContainerNode(void) {
  if(debug)cout << "deleting container node" << endl;
  for_each(childList.begin(),childList.end(),deleteIt<evioDOMNode>);
}


//-----------------------------------------------------------------------------
//------------------------- evioDOMLeafNode -----------------------------------
//-----------------------------------------------------------------------------

                                   
template <typename T> evioDOMLeafNode<T>::evioDOMLeafNode(evioDOMNode *par, int tg, int content, int n, T *p, int ndata) 
  throw(evioException*) : evioDOMNode(par,tg,content,n) {
  
  // fill vector with data
  for(int i=0; i<ndata; i++) data.push_back(p[i]);
}


//-----------------------------------------------------------------------------


template <typename T> evioDOMLeafNode<T>::evioDOMLeafNode(evioDOMNode *par, int tg, int content, int n, const vector<T> &v)
  throw(evioException*) : evioDOMNode(par,tg,content,n) {

  copy(v.begin(),v.end(),inserter(data,data.begin()));
}


//-----------------------------------------------------------------------------


template <typename T> evioDOMLeafNode<T>::evioDOMLeafNode(const evioDOMLeafNode<T> &lNode) throw(evioException*)
  : evioDOMNode(NULL,lNode.tag,lNode.contentType,lNode.num)  {

  cout << "leaf copy constructor called" << endl;

  copy(lNode.begin(),lNode.end(),inserter(data,data.begin()));
}


//-----------------------------------------------------------------------------

                                   
template <typename T> evioDOMLeafNode<T> *evioDOMLeafNode<T>::clone(evioDOMNode *newParent) const {

  cout << "leaf node clone called" << endl;

  return(new evioDOMLeafNode(newParent,tag,contentType,num,data));
}


//-----------------------------------------------------------------------------


template <typename T> evioDOMLeafNode<T> &evioDOMLeafNode<T>::operator=(const evioDOMLeafNode<T> &rhs) 
  throw(evioException*) {

  cout << "leaf node operator= called" << endl;

  if(&rhs!=this) {
    parent=rhs.parent;
    tag=rhs.tag;
    contentType=rhs.contentType;
    num=rhs.num;
    
    copy(rhs.data.begin(),rhs.data.end(),inserter(data,data.begin()));
  }

  return(*this);
}


//-----------------------------------------------------------------------------


                                  
template <typename T> string evioDOMLeafNode<T>::toString(void) const {
  ostringstream os;
  os << getHeader(0) << getFooter(0);
  return(os.str());
}


//-----------------------------------------------------------------------------

                                   
template <typename T> string evioDOMLeafNode<T>::getHeader(int depth) const {

  ostringstream os;
  string indent = getIndent(depth);
  string indent2 = indent + "    ";

  int wid,swid;
  switch (contentType) {
  case 0x0:
  case 0x1:
  case 0x2:
  case 0xb:
    wid=5;
    swid=10;
    break;
  case 0x4:
  case 0x5:
    wid=8;
    swid=6;
    break;
  case 0x6:
  case 0x7:
    wid=8;
    swid=4;
    break;
  case 0x8:
  case 0x9:
  case 0xa:
    wid=2;
    swid=28;
    break;
  default:
    wid=1;
   swid=30;
    break;
  }


  // dump header
  os << indent
     <<  "<" << get_typename(contentType) 
     << "\" data_type=\"" << hex << showbase << contentType
     << dec << "\" tag=\"" << tag;
  if((parent==NULL)||((parent->contentType==0xe)||(parent->contentType==0x10))) os << dec << "\" num=\"" << num;
  os << "\">" << endl;


  // dump data...odd what has to be done for char types 0x6,0x7 due to bugs in ostream operator<<
  int *j;
  short k;
  typename vector<T>::const_iterator iter;
  for(iter=data.begin(); iter!=data.end();) {

    if(contentType!=0x3)os << indent2;
    for(int j=0; (j<wid)&&(iter!=data.end()); j++) {
      switch (contentType) {
      case 0x0:
      case 0x1:
      case 0x5:
      case 0xa:
        os << hex << showbase << setw(swid) << *iter << "  ";
        break;
      case 0x3:
        os << "<!CDATA[" << endl << *iter << endl << "]]>";
        break;
      case 0x6:
        k = (*((short*)(&(*iter)))) & 0xff;
        if((k&0x80)!=0)k|=0xff00;
        os << setw(swid) << k << "  ";
        break;
      case 0x7:
        os << hex << showbase << setw(swid) << ((*(int*)&(*iter))&0xff) << "  ";
        break;
      case 0x2:
        os << setprecision(6) << fixed << setw(swid) << *iter << "  ";
        break;
      case 0x8:
        os << setw(swid) << setprecision(20) << scientific << *iter << "  ";
        break;
      default:
        os << setw(swid) << *iter << "  ";
        break;
      }
      iter++;
    }
    os << dec << endl;

  }

  return(os.str());
}


//-----------------------------------------------------------------------------


template <typename T> string evioDOMLeafNode<T>::getFooter(int depth) const {
  ostringstream os;
  os << getIndent(depth) << "</" << get_typename(this->contentType) << ">" << endl;
  return(os.str());
}


//-----------------------------------------------------------------------------

                                   
template <typename T> evioDOMLeafNode<T>::~evioDOMLeafNode(void) {
  if(debug)cout << "deleting leaf node" << endl;
}
  

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
