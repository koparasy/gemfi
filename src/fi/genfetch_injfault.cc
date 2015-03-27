#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/genfetch_injfault.hh"
#include "fi/fi_system.hh"

using namespace std;

GeneralFetchInjectedFault::GeneralFetchInjectedFault(std::ifstream &os)
  : O3CPUInjectedFault(os)
{
  setFaultType(InjectedFault::GeneralFetchInjectedFault);
  fi_system->fetchStageInjectedFaultQueue.insert(this);
}

GeneralFetchInjectedFault::~GeneralFetchInjectedFault()
{
}


const char *
GeneralFetchInjectedFault::description() const
{
    return "GeneralFetchInjectedFault";
}

void GeneralFetchInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===GeneralFetchInjectedFault::dump()===\n";
    O3CPUInjectedFault::dump();
    std::cout << "~==GeneralFetchInjectedFault::dump()===\n";
  }
}

TheISA::MachInst
GeneralFetchInjectedFault::process(TheISA::MachInst inst)
{
  TheISA::MachInst retInst = 0;

  DPRINTF(FaultInjection, "===GeneralFetchStageInjectedFault::process()===\n");
  dump();
  
  retInst = manifest(inst, getValue(), getValueType());

  check4reschedule();

  DPRINTF(FaultInjection, "~==GeneralFetchStageInjectedFault::process()===\n");
  return retInst; 
}

