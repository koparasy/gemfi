
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"
using namespace std;


// Insert faults
InjectedFault::InjectedFault(ifstream &os)
{
  std:: string _when, _what, _thread, _where ;
  int _occ;
  os>>_when;
  os>>_what;
  os>>_thread;
  os>>_where;
  os>>_occ;	
  if(DTRACE(FaultInjection)){
    std::cout << "InjectedFault :: when :" << _when << "\n";
    std::cout << "InjectedFault :: where :" << _where << "\n";
    std::cout << "InjectedFault :: what :" << _what  << "\n";
    std::cout << "InjectedFault :: thread :" << _thread << "\n";
    std::cout << "InjectedFault :: occ :" << _occ << "\n";
  }


  setThread(_thread);
  setWhen(_when);
  setWhere(_where);
  setWhat(_what);
  parseWhat(_what);
  parseWhen(_when);
  setFaultID();
  setOccurrence(_occ);
  setManifested(false);

}



InjectedFault::~InjectedFault()
{
  getQueue()->remove(this);
}


const char *
InjectedFault::description() const
{
  return "InjectedFault";
}


  int
InjectedFault::parseWhen(std::string s)
{
  if (s.compare(0,4,"Inst",0,4) == 0) {
    setTimingType(InjectedFault::InstructionTiming);
    setTiming(s.substr(5));
  }
  else if (s.compare(0,4,"Tick",0,4) == 0) {
    setTimingType(InjectedFault::TickTiming);
    setTiming(s.substr(5));
  }
  else if (s.compare(0,4,"Addr",0,4) == 0) {
    setTimingType(InjectedFault::VirtualAddrTiming);
    setTiming(s.substr(5));
  }
  else {
    std::cout << "InjecteFault::parseWhen() - Error Unsupported type " << s << "\n";
    assert(0);
    return 1;
  }

  return 0;
}

  int
InjectedFault::parseWhat(std::string s)
{
  if (s.compare(0,4,"Immd",0,4) == 0) {
    setValueType(InjectedFault::ImmediateValue);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"Mask",0,4) == 0) {
    setValueType(InjectedFault::MaskValue);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"Flip",0,4) == 0) {
    setValueType(InjectedFault::FlipBit);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"All0",0,4) == 0) {
    setValueType(InjectedFault::AllValue);
    setValue(0);
  }
  else if (s.compare(0,4,"All1",0,4) == 0) {
    setValueType(InjectedFault::AllValue);
    setValue(1);
  }
  else {
    std::cout << "InjectedFault::parseWhat() - Error Unsupported type " << s << "\n";
    assert(0);
    return 1;
  }

  return 0;
}

  void 
InjectedFault::increaseTiming(uint64_t cycles, uint64_t insts, uint64_t addr)
{
  switch (getTimingType())
  {
    case (InjectedFault::TickTiming):
      {
        setTiming(cycles);
        break;
      }
    case (InjectedFault::InstructionTiming):
      {
        setTiming(insts);
        break;
      }
    case (InjectedFault::VirtualAddrTiming):
      {
        /*Originally this changed to a different timing type, however, we will keep it at the same type to enable the precise injection of permanent faults (e.g. first value of a macroblock)
        */


        /* Virtual addresses can not be used for non transient faults, that is because in the presence of branches we can not predefine an address for the next fault to occure (will the branch succeed or not).
           For that reason we change the Timing Type either the Tick or Instruction can be used, one can add an option but currently instructions are choosen no thought was put onto that choice.
           */
        //setTimingType(InjectedFault::InstructionTiming);
        //setTiming(insts);

        /*we also remove the fault from the fault queue as it was done for the other types too
        */
        //getQueue()->remove(this);

        break;
      }
    default:
      {
        std::cout << "InjectedFault::increaseTiming() - getTimingType default type error\n";
        assert(0);
        break;
      }
  }
}



void
InjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===InjectedFault::dump()===\n";
    std::cout << "\tWhere: " << getWhere() << "\n";
    std::cout << "\tWhen: " << getWhen() << "\n";
    std::cout << "\tWhat: " << getWhat() << "\n";
    std::cout << "\tthreadID: " << getThread() << "\n";
    std::cout << "\tfaultID: " << getFaultID() << "\n";
    std::cout << "\tfaultType: " << getFaultType() << "\n";
    std::cout << "\ttimingType: " << getTimingType() << "\n";
    std::cout << "\ttiming: " << getTiming() << "\n";
    std::cout << "\tvalueType: " << getValueType() << "\n";
    std::cout << "\tvalue: " << getValue() << "\n";
    std::cout << "\toccurrence: " << getOccurrence() << "\n";
    std::cout << "~==InjectedFault::dump()===\n";
  }
}

InjectedFaultQueue::InjectedFaultQueue()
  :objName(""), head(NULL),tail(NULL)
{
  //
}

  InjectedFaultQueue::InjectedFaultQueue(const string &n)
: objName(n), head(NULL), tail(NULL)
{

}


  void
InjectedFaultQueue::insert(InjectedFault *f)
{
  f->setQueue(this); // when inserting a fault to a queue place a reference from it to the queue

  InjectedFault *p;

  p = head;

  if (empty()) {//queue is empty
    head = f;
    tail = f;
    head->nxt=NULL;
    tail->nxt=NULL;

    head->prv=NULL;
    tail->prv=NULL;
    return;
  }
  else {//queue is not empty
    while ((p!=NULL) && (p->getTiming() < f->getTiming())) {//travel queue elements to find the one before which our element will be inserted
      p = p->nxt;
    }

    if (p==NULL) {//element inserted at the end
      tail->nxt = f;
      f->prv = tail;
      tail = f;
      return;
    }
    else if (p==head) {//element inserted in the beginning
      head->prv = f;
      f->nxt = head;
      head = f;
      return;
    }
    else {//element inserted between two other elements
      p->prv->nxt = f;
      f->prv = p->prv;
      p->prv =f;
      f->nxt = p;
      return;
    }
  }

}

  void
InjectedFaultQueue::remove(InjectedFault *f)
{
  InjectedFault *p;


  if ((head==NULL) & (tail==NULL)) {//queue is empty
    return;
  }

  p = head;
  while ((p!=NULL) && (p!=f)) {//search if event exists
    p = p->nxt;
  }

  if (p==NULL) {//event was not found
    std::cout << "InjectedFaultQueue:remove() -- Fault was not found on the queue\n";
    assert(0);
  }

  if (f->prv==NULL) {//fault to be removed is the first one
    head = f->nxt;
  }
  else {
    f->prv->nxt = f->nxt;
  }

  if (f->nxt==NULL) {//fault to be removed is the last one
    tail = f->prv;
  }
  else {
    f->nxt->prv = f->prv;
  }

  return;

}

// Check if on this cycle/isntruction a fault is going to manifest.

InjectedFault *InjectedFaultQueue::scan(std::string s , ThreadEnabledFault &thisThread , Addr vaddr){

  static uint64_t called=0;
  InjectedFault *p;
  int i;
  uint64_t exec_time = 0;
  uint64_t exec_instr = 0;
  unsigned char flag = 1;
  p = head;
  //pass the list and check if a fault meets the conditions

  while(p && flag){
    if(!p->isManifested()){
      exec_time = 0;
      exec_instr = 0;
      //find how much time do I run.
      if(name().compare("DecodeStageFaultQueue") == 0 ){
        i = fi_system->get_fi_decode_counters( p , thisThread, s , &exec_instr , &exec_time );
        called++;
      }
      else if(name().compare("IEWStageFaultQueue") == 0 ){
        i = fi_system->get_fi_exec_counters( p , thisThread, s , &exec_instr , &exec_time );
      }
      else if (name().compare("LoadStoreFaultQueue") == 0 ){
        i = fi_system->get_fi_loadstore_counters( p , thisThread, s , &exec_instr , &exec_time );
      }
      else{
        i = fi_system->get_fi_fetch_counters( p , thisThread, s , &exec_instr , &exec_time );
      }
      if(i){
        switch( p->getTimingType() ){
          case (InjectedFault ::TickTiming):
            {   
              if(exec_time == p->getTiming()){ //correct time so intend to manifest
                p->setServicedAt(exec_time);
                return(p);
              }
              else if(exec_time > p->getTiming())
                flag = 0;
            }
            break;
          case (InjectedFault ::InstructionTiming):
            {
              if(exec_instr == p->getTiming() ){
                p->setServicedAt(exec_instr);
                return(p);
              }
              else if(exec_instr > p->getTiming())
                flag = 0;
            }
            break;
          case (InjectedFault ::VirtualAddrTiming):
            {
              if(vaddr == p->getTiming() + thisThread.getMagicInstVirtualAddr() ){
                p->setServicedAt(vaddr);
                p->dump();
                return(p);
              }

            }
            break;
          default:
            {
              std::cout << "InjectedFaultQueue::scan() - getTimingType default type error\n";
              assert(0);
              break;
            }


        }
      }
    }
    p = p->nxt;
  }
  return(NULL);

}






void
InjectedFaultQueue::dump() const
{
  InjectedFault *p=head;

  if (DTRACE(FaultInjection)) {
    std::cout << "=====InjectedFaultQueue::dump()=====\n";
    while (p) {
      p->dump();
      p = p->nxt;
    }
    std::cout << "~====InjectedFaultQueue::dump()=====\n";
  }
}
