#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/iew_injfault.hh"
#include "fi/fi_system.hh"

using namespace std;

IEWStageInjectedFault::IEWStageInjectedFault(std::ifstream &os)
  : O3CPUInjectedFault(os)
{
  setFaultType(InjectedFault::ExecutionInjectedFault);
  fi_system->iewStageInjectedFaultQueue.insert(this);
}

IEWStageInjectedFault::~IEWStageInjectedFault()
{
}



const char *
IEWStageInjectedFault::description() const
{
    return "IEWStageInjectedFault";
}

void
IEWStageInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===IEWStageInjectedFault::dump()===\n";
    InjectedFault::dump();
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==IEWStageInjectedFault::dump()===\n";
  }
}


