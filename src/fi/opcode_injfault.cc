#include "base/types.hh"
#include "base/bitfield.hh"
#include "fi/faultq.hh"
#include "fi/opcode_injfault.hh"
#include "fi/fi_system.hh"

using namespace std;



OpCodeInjectedFault::OpCodeInjectedFault(std::ifstream &os)
  : O3CPUInjectedFault(os)
{
  setFaultType(InjectedFault::OpCodeInjectedFault);
  fi_system->fetchStageInjectedFaultQueue.insert(this);
}


OpCodeInjectedFault::~OpCodeInjectedFault()
{
}


const char *
OpCodeInjectedFault::description() const
{
    return "OpCodeInjectedFault";
}

void 
OpCodeInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "OpCodeInjectedFault::dump()\n";
    InjectedFault::dump();
    //std::cout << "CPU module name: " << getCPU()->name() << "\n";
    std::cout << "tcontext: " << getTContext() << "\n";
  }
}


TheISA::MachInst
OpCodeInjectedFault::process(TheISA::MachInst inst)
{
  TheISA::MachInst retInst = 0;

  if (DTRACE(FaultInjection)) {
    std::cout << "===OpCodeInjectedFault::process()===\n";
    dump();
    DPRINTF(FaultInjection, "\n");
  }


  switch (getValueType()) {
  case (InjectedFault::ImmediateValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tImmediateValue\n";
	std::cout << "\tinstruction before FI: "<< inst << "\n";
      }
      retInst = insertBits(inst, 31, 26, getValue());
      if (DTRACE(FaultInjection)) {
	std::cout << "\tinstruction after FI: "<< inst << "\n";
      }
      break;
    }
  case (InjectedFault::MaskValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tMaskValue\n";
	std::cout << "\tinstruction before FI: "<< inst << "\n";
      }
      retInst = insertBits(inst, 31,26, bits(inst, 31,26) ^ getValue());
      if (DTRACE(FaultInjection)) {
	std::cout << "\tinstruction after FI: "<< inst << "\n";
      }
      break;
    }
  default: 
    {
      std::cout << "getValueType Default case Error\n";
      assert(0);
      break;
    }
  }


  check4reschedule();

  if (DTRACE(FaultInjection)) {
    std::cout << "~==OpCodeInjectedFault::process()===\n";
  }
  return retInst; 
}

