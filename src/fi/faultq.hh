#ifndef __INJECTED_FAULT_QUEUE_HH__
#define __INJECTED_FAULT_QUEUE_HH__

#include <map>
#include <utility> // make_pair
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "debug/FaultInjection.hh"




//#include "fi/cpu_threadInfo.hh"

#include "mem/mem_object.hh"
//#include "params/InjectedFault.hh"
//#include "fi/fi_process.hh"

using namespace std;
using namespace TheISA;

class InjectedFaultQueue; // forward declaration
class InjectedFault; //forward declaration
class ThreadEnabledFault; //forward declaration
class BaseO3CPU;


static const unsigned char singlebit_mask[] = {0x01,
					       0x02,
					       0x04,
					       0x08,
					       0x10,
					       0x20,
					       0x40,
					       0x80};

static const unsigned char all_mask[] = {0x00,
					 0xFF};

class InjectedFault 
{
  friend class InjectedFaultQueue;
  friend class ThreadEnabledFault;
//  friend class Fi_System;

public:
  //Fault Types (WHERE) -- Location of the injection
  typedef uint16_t InjectedFaultType;
  static const InjectedFaultType RegisterInjectedFault         = 1;
  static const InjectedFaultType MemoryInjectedFault           = 2;
  static const InjectedFaultType PCInjectedFault               = 3;
  static const InjectedFaultType GeneralFetchInjectedFault     = 4;
  static const InjectedFaultType OpCodeInjectedFault           = 5;
  static const InjectedFaultType RegisterDecodingInjectedFault = 6;
  static const InjectedFaultType ExecutionInjectedFault        = 7;
  static const InjectedFaultType LoadStoreInjectedFault	= 8;
  InjectedFault *nxt;
  InjectedFault *prv;
protected:
  //Fault Timing Types (WHEN) -- Timing of the injection
  typedef uint16_t InjectedFaultTimingType;
  static const InjectedFaultTimingType TickTiming = 1;
  static const InjectedFaultTimingType InstructionTiming = 2;
  static const InjectedFaultTimingType VirtualAddrTiming = 3;

  //Fault Values Types (WHAT) -- nature of the injection, how it will affect the injected structure's value
  typedef uint16_t InjectedFaultValueType;
  static const InjectedFaultValueType ImmediateValue = 1;
  static const InjectedFaultValueType MaskValue = 2;
  static const InjectedFaultValueType FlipBit = 3;
  static const InjectedFaultValueType AllValue = 4;


protected: 

  uint64_t _faultID;//unique ID value used for distinguishing between fault object
  Tick _servicedAt;

  
  std::string _where;// contains the name of the module that we will inject with the fault (e.g. 'system.cpu')
  std::string _when;// contains information about the timing of the injection
  std::string _what;// contains information about the nature and the value of the injection (how the structures value will be affected)
  std::string _thread; //contains  information about the software thread that the fault will be injected
  
  InjectedFaultQueue  *_queue;//the queue in which the fault has been scheduled

  InjectedFaultType _faultType; //location of the injection (e.g. register, fetch stage, pc)
  InjectedFaultTimingType _timingType;//method used for triggering the injected faults
  InjectedFaultValueType _valueType;//method used for corrupting the injected structures content (XOR, OR, Immediate value)
  uint64_t _timing;// when the fault should manifest
  uint64_t _value;// what value should be used together with the corruption method
  int servicedFrom;

  int _occurrence;//how many times the fault should manifest (1:transient fault, 0: permanent fault, >0: intermittent fault)
  bool manifested;// has the fault manifested at least ones?

  /*
   * pointers on the next and previous fault in the queue
   * used for queue traversal
   */
  


protected:

  /* The setXXX functions are used to assign values at the above described variable
   */
  void setFaultID()
  {
    static uint64_t faultCnt = 0;
    _faultID = faultCnt++; 
  }
  void setFaultID(int id){
    _faultID = id;
  }

  void setServicedAt(Tick v) { _servicedAt = v;}

  void setFaultType(short v) { _faultType = v;}

  void setTimingType(int v) { _timingType = v;}
  void setTiming(int v) { _timing = v;}
  void setTiming(std::string v) { _timing = strtoll(v.c_str(), NULL, 10);}
  void setTiming(uint64_t v) {_timing = v;}

  void setValueType(int v) { _valueType = v;}
  void setValue(int v) { _value = v;}
  void setValue(std::string v) { _value = strtoul(v.c_str(), NULL, 10);}


  void ServicedFrom(int v){ servicedFrom = v ;}
  void setWhen(std::string v) { _when = v;}
  void setWhere(std::string v) { _where = v;}
  void setWhat(std::string v) { _what = v;}
  void setThread(std::string v){_thread = v; }

  void setOccurrence(int v) { _occurrence = v;}
  void setManifested(bool v) { manifested = v;}

  void setQueue(InjectedFaultQueue *q) {_queue = q;}
  
  /* used with intermittent faults
   * increaseTiming increases the timing of the manifestation of the fault, also it may change the type of Timing in case it is of VirtualAddrTiming type.
   * This is done because VirtualAddrTiming can not be used for intermittent fault injection (e.g. branches, loop structures; what should be the next Address that should be used?)
   * Note: right now the next timing value is given as a parameter maybe this should change
   */
  void increaseTiming(uint64_t cycles, uint64_t insts, uint64_t addr);
  void decreaseOccurrence() { _occurrence--;}

  
  
  /*Parse the _when and _what strings to extract information
   */
  int parseWhen(std::string _when);
  int parseWhat(std::string _what);
  
	
	
	
public:

  InjectedFault(ifstream &os);
  ~InjectedFault();
  
  virtual const char *description() const;
  virtual void dump() const;
  virtual void setCPU(BaseCPU *v) { assert(0); std::cout <<"this function should never be called from here\n";}
  virtual void setCPU(BaseO3CPU *v) { assert(0); std::cout <<"this function should never be called from here\n";}
 


  bool isLittleEndian()
  {
      static bool little_endian ;
      static bool endian_checked = false ;
      // runtime endianness check (once only)
      if( !endian_checked )
      {
	  const short endian_test_pattern = 0x00ff ;
	  little_endian = *(reinterpret_cast<const char*>(&endian_test_pattern)) == '\xff' ;
	  endian_checked = true ;
      }
  
      return little_endian ;
  }
 
  template <class T>
  std::string type_to_hex( const T arg )
  {
      std::ostringstream hexstr ;
      const char* addr = reinterpret_cast<const char*>(&arg) ;
  
      hexstr << "0x" ;
      hexstr << std::setw(2) << std::setprecision(2) << std::setfill('0') << std::hex ;
  
      if( isLittleEndian() )
      {
	  for( int b = sizeof(arg) - 1; b >= 0; b-- )
	  {
	      hexstr << static_cast<unsigned>(*(addr+b) & 0xff) ;
	  }
      }
      else
      {
	  for( int b = 0; b < sizeof(arg); b++ )
	  {
	      hexstr << static_cast<unsigned>(*(addr+b) & 0xff) ;
	  }
      }
      return hexstr.str() ;
  }
  
  template <class T> inline T
  XOR(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;
    int j = sizeof(uint64_t) - 1;

    unsigned char *inP = (unsigned char *)&in;
    unsigned char *outP = (unsigned char *)&out;

    while(i>=0) {
	inP[i] = inP[i] ^ outP[j];
	j--;
	i--;
      }

    return in;
  }
 
  //AND's the two given values and returns the result
  template <class T> inline T
  AND(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;
    int j = sizeof(uint64_t) - 1;

    unsigned char *inP = (unsigned char *)&in;
    unsigned char *outP = (unsigned char *)&out;

    while(i>=0) {
	inP[i] = inP[i] & outP[j];
	j--;
	i--;
      }

    return in;
  }

  //OR's the two given values and returns the result
  template <class T> inline T
  OR(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;
    int j = sizeof(uint64_t) - 1;

    unsigned char *inP = (unsigned char *)&in;
    unsigned char *outP = (unsigned char *)&out;

    while(i>=0) {
      inP[i] = inP[i] | outP[j];
	j--;
	i--;
      }

    return in;
  }

  //Set the value of 'out' at 'in'
  template <class T>
  inline
  T
  IMMD(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;
    int j = sizeof(uint64_t) - 1;

    unsigned char *inP = (unsigned char *)&in;
    unsigned char *outP = (unsigned char *)&out;

    while(i>=0) {
	inP[i] = outP[j];
	j--;
	i--;
      }

    return in;
  }

  template <class T> inline T
  FLIP(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;

    int j_quotient;
    int j_remainder;

    if (!out)//we do not flip a bit
      return in;

    j_quotient = (out-1) / 8;
    j_remainder = (out-1) % 8;

    //assert(i >= j_quotient);//this shouldn't be an assertion
    if (j_quotient > i) {
      std::cout << "Structure is not affected by the fault\n";
      return in;
    }

    unsigned char *inP = (unsigned char *)&in;
    
    inP[j_quotient] = inP[j_quotient] ^ singlebit_mask[j_remainder];

    return in;
  }

  template <class T> inline T
  ALL(T in, uint64_t out)
  {
    int i = sizeof(T) - 1;

    unsigned char *inP = (unsigned char *)&in;
    
    assert(out==0 || out==1);

    while(i>=0) {
      inP[i] = all_mask[out];
      i--;
    }
    
    return in;
  }

  template <class T>
  T
  manifest(T in, uint64_t out, InjectedFaultValueType type)
  {
    T retVal;
    if (DTRACE(FaultInjection)) {
      std::cout << "HEX:Value before FI: " << type_to_hex(in) << "\n";
    }
    switch (type)
      {
      case (InjectedFault::ImmediateValue) :
	{
	  if (DTRACE(FaultInjection)) {
	    std::cout <<  "\tImmediate Value\n";
	    std::cout << "Value before FI: " << in << "\n";
	  }
	  retVal = IMMD(in, out);
	  if (DTRACE(FaultInjection)) {
	    std::cout << "Value after FI: " << retVal << "\n";
	  }
	  break;
	}
      case (InjectedFault::MaskValue) :
	{
	  if (DTRACE(FaultInjection)) {
	    std::cout <<  "\t\tMask Value\n";
	    std::cout << "\t\tValue before FI: " << in << "\n";
	  }
	  retVal =XOR(in, out);
	  if (DTRACE(FaultInjection)) {
	    std::cout << "\t\tValue after FI: " << retVal << "\n";
	  }
	  break;
	}
      case (InjectedFault::FlipBit) :
	{
	  if (DTRACE(FaultInjection)) {
	    std::cout <<  "\t\tFlipBit Value\n";
	    std::cout << "\t\tValue before FI: " << in << "\n";
	  }
	  retVal = FLIP(in, out);
	  if (DTRACE(FaultInjection)) {
	    std::cout << "\t\tValue after FI: " << retVal << "\n";
	  }
	  break;
	}
      case (InjectedFault::AllValue) :
	{
	  if (DTRACE(FaultInjection)) {
	    std::cout <<  "\t\tAll Value\n";
	    std::cout << "\t\tValue before FI: " << in << "\n";
	  }
	  retVal = ALL(in, out);
	  if (DTRACE(FaultInjection)) {
	    std::cout << "\t\tValue after FI: " << retVal << "\n";
	  }
	  break;
	}
      default:
	{
	   retVal = in;
	  std::cout << "InjectedFault::manifest -- InjectedFaultValueType Error\n";
	  assert(0);
	  break;
	}
      }
    if (DTRACE(FaultInjection)) {
	std::cout << "HEX:Value after FI: " << type_to_hex(retVal) << "\n";
    }  
    return retVal;
  }
  

  /* The getXXX functions are a compliment to the setXXX functions and are used to get the values of the described variable
   */
  

  
  int 
  getFaultID() const  { return _faultID;} 
  Tick
  getServicedAt() const { return _servicedAt;}
  int
  getServicedFrom() const { return servicedFrom; }
  InjectedFaultType 
  getFaultType() const { return _faultType;}
  InjectedFaultTimingType
  getTimingType() const { return _timingType;}
  uint64_t
  getTiming() const { return _timing;}
  InjectedFaultValueType
  getValueType() const { return _valueType;}
  uint64_t
  getValue() const { return _value;}
  std::string
  getWhen() const { return _when;}
  std::string
  getWhere() const { return _where;}
  std::string
  getWhat() const { return _what;}
  std::string
  getThread() const {return _thread;}
  InjectedFaultQueue *
  getQueue() const {return _queue;}

  int
  getOccurrence() const {return _occurrence;}
  bool
  isManifested() const {return manifested;}
};



class InjectedFaultQueue : public Serializable
{
  friend class ThreadEnabledFault;
private:
  /* Name of the Queue
   */
  std::string objName;
  
protected:
   /*Pointers at the beginning and end of the list (NULL if the list is empty)
   */
  

  
 
   
    
public:
  InjectedFault *head;
  InjectedFault *tail;
  InjectedFaultQueue(const std::string &n);
  InjectedFaultQueue();

  //  InjectedFaultQueue(const InjectedFaultQueue &);
  //  const InjectedFaultQueue &operator=(const InjectedFaultQueue &);


  
  virtual const std::string name() const { return objName; }

  /* Inserts the given fault into the queue
   * The faults are entered in a ascending order based on their timing value, however, we do not make any distiction betweent different timing types (maybe in a later version)
   */
  void insert(InjectedFault *fault);

  /* Remove the given fault from the queue, if it exists.
   */
  void remove(InjectedFault *fault);

  void setName(string v){
    objName=v;
  }

  void setHead(InjectedFault* p) {head=p;}
  void setTail(InjectedFault* p) {tail=p;}
  /* Scan the specific queue to find if any fault matches the provided criteria
   * s: injected module name
   * thisThread: the current thread/application
   * vaddr: current PC address
   */
  virtual InjectedFault *scan(std::string s , ThreadEnabledFault &thisThread , Addr vaddr);
  //virtual InjectedFault * scan(std::string s, int64_t t, uint64_t instCnt, Addr vaddr);
  //virtual InjectedFault * scan(std::string s, int64_t t, uint64_t instCnt, Addr vaddr , ThreadEnabledFault &thisThread);


  
  
  /* returns true if queue is empty
   */
  bool empty() const { return ((head == NULL) && (tail == NULL)); }
  
  /* Dump the contents of the queue
   */
  void dump() const;
};









#endif // __INJECTED_FAULT_QUEUE_HH__
