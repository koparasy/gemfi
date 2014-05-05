#ifndef __CPU_INJECTED_FAULT_HH__
#define __CPU_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "cpu/base.hh"

#include <iostream>
#include <fstream>

#include "mem/mem_object.hh"

class CPUInjectedFault : public InjectedFault
{
private:
  BaseCPU *_cpu; //Pointer to the cpu where the fauls is going to be injected
  int _tcontext; // Hardware thread (Currently useless)
  

  
  

public:
  CPUInjectedFault( ifstream &os); //initialize faults from the input fstream
  ~CPUInjectedFault();

  void setTContext(int v) { _tcontext = v;}  //set hardware thread
  virtual void setCPU(BaseCPU *v) { _cpu = v;} // set cput
  virtual void setCPU(BaseO3CPU *v) { assert(0); std::cout <<"this function should never be called from here\n";}
  
  virtual const char *description() const;
 
  void dump() const; //print info of the fault

  virtual int process(){ assert(0);return 0;}; //No fault is going to call this function
  BaseCPU *
  getCPU() const { return _cpu;} 
  int
  getTContext() const { return _tcontext;}
  void check4reschedule();
};

#endif // __CPU_INJECTED_FAULT_HH__
