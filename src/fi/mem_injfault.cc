#include "fi/faultq.hh"
#include "fi/mem_injfault.hh"
#include "fi/fi_system.hh"
#include "arch/alpha/vtophys.hh"
using namespace std;

MemoryInjectedFault::MemoryInjectedFault(std::ifstream &os)
	: CPUInjectedFault(os){
		int k;
		os>>k;
		int reg;
		os>>reg;
		setOffset(k);
		setRegister(reg);
		setFaultType(InjectedFault::MemoryInjectedFault);
		pMem = reinterpret_cast<PhysicalMemory *>(fi_system->find("system.physmem")); 
		fi_system->mainInjectedFaultQueue.insert(this);
}



MemoryInjectedFault::~MemoryInjectedFault()
{
}
const char *
MemoryInjectedFault::description() const
{
    return "MemoryInjectedFault";
}


void MemoryInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "~==MemoryInjectedFault::dump()===\n";
    CPUInjectedFault::dump();
    std::cout << "\tAddress is: " << offset << "\n";
    std::cout << "\tRelative Register is: " << getRegister() << "\n";
    std::cout << "~==MemoryInjectedFault::dump()===\n";
  }
}



int
MemoryInjectedFault::process()
{
  range_map<Addr, AbstractMemory* > *addrMap;
  AbstractMemory *myblock;
  bool isphysical;
  
  DPRINTF(FaultInjection, "===MemoryInjectedFault::process()===\n");
  dump();
   
  Addr physical;
  
  TheISA::IntReg addr = getCPU()->getContext(getTContext())->readIntReg(getRegister()); //find the VA address
  addr+= offset; //calculate the desired VA
  physical= AlphaISA::vtophys(getCPU()->getContext(getTContext()),addr); //find the Physical Address
  isphysical = getCPU()->system->isMemAddr(physical); //check if the address exists
  
  if (DTRACE(FaultInjection)) {
    std::cout<<"\tVirtual  Address is: "<< addr<<"\n";
    std::cout<<"\tPhysical Address is : "<< physical<<"\n";
    std::cout<<"\tis physical mem : "<<isphysical<<"\n";
  }
  
  
  if(isphysical){ //I exist so inject the fault
    addrMap=(getCPU()->system->getPhysMem()).getaddrMap(); // get a map of the memory
    range_map<Addr, AbstractMemory*>::const_iterator m = addrMap->find(physical); //find the injected block
    assert(m != addrMap->end()); 
    myblock=m->second;
    uint8_t *hostAddr = myblock->pmemAddr + physical - myblock->range.start;
    uint8_t memval=*hostAddr;
    int8_t mask = manifest(memval, (uint8_t)getValue(), getValueType()); //alter information of the block
    *hostAddr=mask;
  }else{
    if (DTRACE(FaultInjection)) {
      std::cout<<"I am not going to manifest since the memory does not exist";
    }
  }
  
  check4reschedule();


  return 0;
}

