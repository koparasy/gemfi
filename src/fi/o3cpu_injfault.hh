#ifndef __O3CPU_INJECTED_FAULT_HH__
#define __O3CPU_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "cpu/o3/cpu.hh"

/*
 * Usage is the same as the cpu_injfault however 
 * this faults are supposed to hit a more detailed 
 * simulation
 */


class O3CPUInjectedFault : public InjectedFault
{

private:
  BaseO3CPU *_cpu; //Pointer to the cpu where the fauls is going to be injected
  int _tcontext; // Hardware thread (Currently useless)
  
  

public:

  O3CPUInjectedFault(std::ifstream &os);//initialize faults from the input fstream
  ~O3CPUInjectedFault();

  virtual const char *description() const;
  void setTContext(int v) { _tcontext = v;}
  virtual void setCPU(BaseO3CPU *v) {  _cpu = v;}
  virtual void setCPU(BaseCPU *v) { assert(0); std::cout <<"this function should never be called from here\n";}
  void dump() const;

  virtual TheISA::MachInst process(TheISA::MachInst inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  
  virtual StaticInstPtr process(StaticInstPtr inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  virtual StaticInstPtr process(StaticInstPtr inst , int regNum) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  BaseCPU *
  getCPU() const { return _cpu;} 
  int
  getTContext() const { return _tcontext;}
  void check4reschedule();
};


#endif // __O3CPU_INJECTED_FAULT_HH__
