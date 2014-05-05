#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "fi/fi_system.hh"

using namespace std;


O3CPUInjectedFault::O3CPUInjectedFault( std::ifstream &os)
: InjectedFault(os){
	int t;
	os>>t;
	if(DTRACE(FaultInjection)){
	   std::cout << "O3CPUInjectedFault :: Tcontext:"<< t << "\n";
	}
	setTContext(t);
}


O3CPUInjectedFault::~O3CPUInjectedFault()
{
}


const char *
O3CPUInjectedFault::description() const
{
    return "O3CPUInjectedFault";
}



void
O3CPUInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===O3CPUInjectedFault::dump()===\n";
    InjectedFault::dump();
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==O3CPUInjectedFault::dump()===\n";
  }
}

void 
O3CPUInjectedFault:: check4reschedule(){
  
  
  if(getOccurrence()==1){
    getQueue()->remove(this);
    return;
  }else{
    Tick cycles = getTiming() +  getCPU()->clockEdge(Cycles(1));
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(cycles,insts,addr);
    decreaseOccurrence();
    setManifested(false);
  }

  
}
