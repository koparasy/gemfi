#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"
#include "fi/fi_system.hh"

#include <iostream>
#include <fstream>

using namespace std;
CPUInjectedFault::CPUInjectedFault(  ifstream &os)
  :InjectedFault(os){
  int t;
  os>>t;
  setTContext(t);
}


CPUInjectedFault::~CPUInjectedFault()
{
}


const char *
CPUInjectedFault::description() const
{
    return "CPUInjectedFault";
}



void CPUInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===CPUInjectedFault::dump()===\n";
    InjectedFault::dump();  
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==CPUInjectedFault::dump()===\n";
  }

}

void 
CPUInjectedFault:: check4reschedule(){
  
  
  /*Is this the last time that the fault is going to be injected? 
  * Yes: Then remove it from the list
  * No: reschedult the instruction for the next cycle/isntruction
  * Doing so (occurrence>1) intermittent/permanent faults are simulated
  */
  
  if(getOccurrence()==1){
    getQueue()->remove(this);
    return;
  }else{
    Tick cycles = getTiming() + getCPU()->clockEdge(Cycles(1));
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(cycles,insts,addr);
    decreaseOccurrence();
    setManifested(false);
  }

  
}

