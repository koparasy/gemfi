#ifndef __OPCODE_INJECTED_FAULT_HH__
#define __OPCODE_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"


/*
 * Alter the value of the opcode
 */
class OpCodeInjectedFault : public O3CPUInjectedFault
{

public:

  OpCodeInjectedFault(std::ifstream &os);
  ~OpCodeInjectedFault();

  virtual const char *description() const;
  
  void dump() const;

   TheISA::MachInst process(TheISA::MachInst inst);


};

#endif // __OPCODE_INJECTED_FAULT_HH__
