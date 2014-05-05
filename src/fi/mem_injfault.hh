#ifndef __MEM_INJECTED_FAULT_HH__
#define __MEM_INJECTED_FAULT_HH__

#include "mem/physical.hh"
#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"
#include "sim/system.hh"

/*
 * inject faults relative to register value
 * Virtual address = read_int_reg(x)
 * Physical address = virtual_to_phys(Virtual address )
 * block = Physical address + offest
 */


class System;
class MemoryInjectedFault : public CPUInjectedFault
{
private:
  int _register;  
  void setRegister(int v) { _register = v;}
  
public:
  int offset;
  int getRegister() const { return _register;}
  
  PhysicalMemory *pMem;

  MemoryInjectedFault(std::ifstream &os);
  ~MemoryInjectedFault();

  virtual const char *description() const;
  
  void dump() const;


  void setOffset(int v){ offset = v ;}
  int getOffset(){return offset;};
  

  int process();
  
};

#endif // __MEM_INJECTED_FAULT_HH__
