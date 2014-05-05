#ifndef __IEW_STAGE_INJECTED_FAULT_HH__
#define __IEW_STAGE_INJECTED_FAULT_HH__

#include "base/types.hh"
#include "fi/faultq.hh"
#include "cpu/o3/cpu.hh"
#include "fi/o3cpu_injfault.hh"


/*
 * Insert a fault during the execute stage.
 * 
 */


class IEWStageInjectedFault : public O3CPUInjectedFault
{

public:

  IEWStageInjectedFault( std::ifstream &os);
  ~IEWStageInjectedFault();

  virtual TheISA::MachInst process(TheISA::MachInst inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  virtual StaticInstPtr process(StaticInstPtr inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  virtual StaticInstPtr process(StaticInstPtr inst , int regNum) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  
  virtual const char *description() const;
  void dump() const;

  template <class T>
  void
  process(void)
  { 
    std::cout << "IEWStageInjectedFault::process(void) -- Should never be called\n";
    getQueue()->remove(this);  
    assert(0);
  }
  
  /*
   * In case that the fault is going to alter a
   * branch instruction the fault is going 
   * to be injected into the condition,
   * thus flipping it.
   */
  bool process(bool v){
     DPRINTF(FaultInjection, "===IEWStageInjectedFault::process(T)===\n");
     DPRINTF(FaultInjection, "===\t\tboolean value===\n");
     #ifdef ALPHA_ISA
      v=!v;
    #endif
    #ifndef ALPHA_ISA
      assert(0);
    #endif
    check4reschedule();
    DPRINTF(FaultInjection, "~==IEWStageInjectedFault::process(T)===\n");
    return v;
  }
  
  /*
   * on the other occasions the fault is going to 
   * alter the give v structure based on the value of what
   */
  
  template <class T> T
  process(T v)
  { 
    T retVal = v;
    
    DPRINTF(FaultInjection, "===IEWStageInjectedFault::process(T)===\n");
    
#ifdef ALPHA_ISA
   if(getValueType() == InjectedFault::FlipBit && getValue() > sizeof(T)*8){ //Make sure that the flipped bit is inside the affected structure!
      setValue(getValue()%(sizeof(T)*8)+1);
      DPRINTF(FaultInjection,"Altered Flip bit location\n");
      dump();
    }
  retVal = manifest(v, getValue(), getValueType());
#endif
#ifndef ALPHA_ISA
    assert(0);
#endif
    
    check4reschedule();
    
    DPRINTF(FaultInjection, "~==IEWStageInjectedFault::process(T)===\n");
    return retVal;
  }

  
};

#endif // __IEW_STAGE_INJECTED_FAULT_HH__
