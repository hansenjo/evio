//  example of manual evio tree and bank creation

//  ejw, 11-jan-2007


#include <vector>
#include "evioUtil.hxx"

using namespace evio;
using namespace std;


static evioFileChannel *chan = NULL;


uint16_t tag;
uint8_t num;
int len;
ContainerType cType;


vector<uint32_t> uVec1,uVec2;
double dBuf[100];
vector<int32_t> lVec;

/* dummy */
extern "C" {
  int user_frag_select(int tag) {return 1;}
}

//-------------------------------------------------------------------------------i


int main(int argc, char **argv) {
  
  // fill vectors
  for(int i=0; i<10; i++) uVec1.push_back(i),uVec2.push_back(2*i),dBuf[i]=i*10.0,lVec.push_back(100*i);



  try {
    chan = new evioFileChannel("fakeEvents.dat","w");
    chan->open();
    

    // create root container node and build tree using it
    evioDOMNodeP root = evioDOMNode::createEvioDOMNode(tag=1, num=5);
    evioDOMTree tree(root);
    

    // create leaf node contining uint and add to root container node in tree
    evioDOMNodeP ln1 = evioDOMNode::createEvioDOMNode(tag=2, num=6, uVec1); 
    tree.addBank(ln1);


    // create container node of BANKS and add to root node directly
    evioDOMNodeP cn2 = evioDOMNode::createEvioDOMNode(tag=3, num=7, cType=BANK);
    root->addNode(cn2);


    // add some leaf nodes to cn2
    evioDOMNodeP ln3 = evioDOMNode::createEvioDOMNode(tag=4, num=8, dBuf, 10);
    evioDOMNodeP ln4 = evioDOMNode::createEvioDOMNode(tag=5, num=9, lVec);
    cn2->addNode(ln3);
    cn2->addNode(ln4);

    
    // replace the data in some leaf nodes
    ln1->replace(uVec2);
    ln3->replace(dBuf,5);

    
    // write out tree
    chan->write(tree);
    chan->close();

    
  } catch (evioException e) {
    cerr << e.toString() << endl;
  }

  delete(chan);
  cout << "fakeEvents.dat created" << endl;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
