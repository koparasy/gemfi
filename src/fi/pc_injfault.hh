#ifndef __PC_INJECTED_FAULT_HH__
#define __PC_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"


//class PCInjectedFaultParams;

/*
 * Alter the value of the PC address
 */

class PCInjectedFault : public CPUInjectedFault
{
public:
  PCInjectedFault( std::ifstream &os);
  ~PCInjectedFault();

  virtual const char *description() const;
  
  void dump() const;
  int process();
};

#endif // __PC_INJECTED_FAULT_HH__
