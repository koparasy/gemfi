#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/loadstore_injfault.hh"
#include "fi/fi_system.hh"
#include "fi/o3cpu_injfault.hh"

using namespace std;

LoadStoreInjectedFault::LoadStoreInjectedFault(std::ifstream &os)
  : O3CPUInjectedFault(os)
{
    setFaultType(InjectedFault::LoadStoreInjectedFault);
    fi_system->LoadStoreInjectedFaultQueue.insert(this);
}

LoadStoreInjectedFault::~LoadStoreInjectedFault()
{
}

const char *
LoadStoreInjectedFault::description() const
{
  return "LoadStoreInjectedFault";
}

void
LoadStoreInjectedFault::dump() const
{
  if(DTRACE(FaultInjection)){
    std::cout<<"===LoadStoreInjectedFault::dump()===\n";
    InjectedFault::dump();
    std::cout<<"\ttcontext: "<<getTContext()<<"\n";
    std::cout<<"~===LoadStoreInjectedFault::dump()===\n";
    
  }
}
