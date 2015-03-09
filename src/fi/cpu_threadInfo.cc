#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "fi/cpu_threadInfo.hh"
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/fi_system.hh"

std::string allcores = string("all");

//Set all the counters to the correct value
cpuExecutedTicks:: cpuExecutedTicks(std:: string name)
{
  setName(name);
  setInstrFetched(0,0,0,0);
  setInstrDecoded(0,0,0,0);
  setInstrExecuted(0,0,0,0);
  setInstrLoadStore(0,0,0,0);
  setTicks(0);
}

cpuExecutedTicks::~cpuExecutedTicks()
{

}

void cpuExecutedTicks:: dump(){
  if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"CpuExecutedTicks :"<<getName()<<" \t==========================\n"; 
    std::cout << "CpuFetchedInstr: " <<getInstrFetched() <<"\n";
    std::cout << "CpuDecodeInstr: "<<getInstrDecoded()<<"\n";
    std::cout << "CpuExecutedInstr: "<<getInstrExecuted() <<  "\n";
    std::cout << "CpuLoadStoreInstr "<<getInstrLoadStore()<<"\n";
    std::cout << "Ticks: "<<getTicks() << "\n";
    std::cout<<"================\t~CpuExecutedTicks~\t==========================\n";
  }
}

//a new thread has enabled fault injection get the id and set all the information
ThreadEnabledFault::ThreadEnabledFault(int threadId, std::string name)
{
  all = new cpuExecutedTicks(allcores);
  cores.insert(pair<string,cpuExecutedTicks*> (allcores,all));
  isfaulty=0;
  cores.insert(pair<string,cpuExecutedTicks*>(name, new cpuExecutedTicks(name)));
  isInstProtected = 1;
  setThreadId(threadId);
  setMyid();
  setMagicInstVirtualAddr(-1);
}

ThreadEnabledFault::~ThreadEnabledFault()
{
  delete all;
  for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores)
    delete itcores->second;

}



void ThreadEnabledFault::dump(){

  if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"ThreadEnabledFault "<<getMyId()<<" \t==========================\n"; 
    std::cout << "ThreadEnabledInfo  MagicInstVirtualAddr : "<<getMagicInstVirtualAddr()<<" ThreadId :"<<getThreadId() <<"\n";
    std::cout<<"================\t~ThreadEnabledFault~\t==========================\n";
  }
}

//This thread has executed one more cycle increase it

int ThreadEnabledFault:: increaseTicks(std:: string curCpu, uint64_t ticks)
{

  assert(all != NULL);
  all->increaseTicks(ticks);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
    cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
    cores[curCpu]->increaseTicks(ticks);

  return 1;

}


//This thread has fethed one more instruction.


int ThreadEnabledFault:: increaseFetchedInstr(std:: string curCpu)
{
  assert( all != NULL);
  all->increaseFetchInstr((int) isInstProtected);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
    cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
    cores[curCpu]->increaseFetchInstr((int) isInstProtected);
  return 1;

}

int ThreadEnabledFault:: increaseDecodedInstr(std:: string curCpu)
{
  assert( all != NULL);
  all->increaseDecodeInstr((int) isInstProtected);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
    cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
    cores[curCpu]->increaseDecodeInstr( (int) isInstProtected );
  return 1;

}

//This thread has executed one more instruction.

int ThreadEnabledFault:: increaseExecutedInstr(std:: string curCpu)
{
  assert(all != NULL);
  all->increaseExecInstr((int) isInstProtected);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
    cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
    cores[curCpu]->increaseExecInstr((int) isInstProtected);
  return 1;
}


int ThreadEnabledFault:: increaseLoadStoreInstr(std:: string curCpu)
{
  assert(all != NULL);
  all->increaseLoadStoreInstr((int) isInstProtected);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
    cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
    cores[curCpu]->increaseLoadStoreInstr((int) isInstProtected);
  return 1;
}


//how many instruction has this thread fetched  untill now on this core or on all cores?

void ThreadEnabledFault:: CalculateFetchedTime(std::string curCpu , uint64_t *fetched_instr , uint64_t *fetched_time )
{
  uint64_t *vals;
  int i;
  assert( all != NULL);
  if(curCpu.compare("all")==0){
    *fetched_time  =  all->getTicks();
    vals =  all->getInstrFetched(); 
    *fetched_instr = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      *(fetched_instr)+=vals[i];
  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *fetched_time = itcores->second->getTicks();
      vals  = itcores->second->getInstrFetched();
      *fetched_instr = 0;
      for ( i = 0 ; i < CATEGORIES; i++)
        (*fetched_instr)+=vals[i];
    }
    return;
  }
}

void ThreadEnabledFault:: CalculateDecodedTime(std::string curCpu , uint64_t *fetched_instr , uint64_t *fetched_time )
{
  uint64_t *vals;
  int i;
  assert( all != NULL);
  if(curCpu.compare("all")==0){
    *fetched_time  =  all->getTicks();
    vals =  all->getInstrDecoded(); 
    *fetched_instr = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*fetched_instr)+=vals[i];

  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *fetched_time = itcores->second->getTicks();
      vals= itcores->second->getInstrDecoded(); 
    *fetched_instr = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*fetched_instr)+=vals[i];

    }
    return;
  }
}


void ThreadEnabledFault:: CalculateExecutedTime(std::string curCpu  , uint64_t *exec_instr , uint64_t *exec_time)
{
  uint64_t *vals;
  int i;
  assert( all != NULL);
  if(curCpu.compare("all")==0){
    *exec_time  =  all->getTicks();
    vals =  all->getInstrExecuted(); 
    *exec_instr = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*exec_instr)+=vals[i];

  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *exec_time = itcores->second->getTicks();
      vals = itcores->second->getInstrExecuted(); 
    *exec_instr = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*exec_instr) +=vals[i];

    }
  }
  return;
}



void ThreadEnabledFault:: CalculateLoadStoreTime(std::string curCpu  , uint64_t *exec_instr , uint64_t *exec_time)
{
  uint64_t *vals;
  int i;
  assert( all != NULL);
  if(curCpu.compare("all")==0){
    *exec_time  =  all->getTicks();
    vals =  all->getInstrLoadStore();
    (*exec_instr) = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*exec_instr)+=vals[i];
  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *exec_time = itcores->second->getTicks();
      vals = itcores->second->getInstrLoadStore(); 
      (*exec_instr) = 0;
    for ( i = 0 ; i < CATEGORIES; i++)
      (*exec_instr)+=vals[i];
    }
  }
  return;
}


//It is good to know how many instructions i have executed on all cores.
//The more the info the better the results.
void ThreadEnabledFault:: print_time(){
  if (DTRACE(FaultInjection)){
    std::cout<<"THREAD ID: "<<getThreadId()<<"\n";
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      std::cout<<"CORE:"<<itcores->second->getName()<<"\n";
      std::cout<<"Pipeline Stage\t NonProtected\t Protected\t NOP \t Other object file\n";
      std::cout<<"Fetch \t "<< itcores->second->getInstrFetched(0) <<"\t "<< itcores->second->getInstrFetched(1) <<"\t "<< itcores->second->getInstrFetched(2)<<"\t "<< itcores->second->getInstrFetched(3)<< "\n";
      std::cout<<"Decode \t "<< itcores->second->getInstrDecoded(0) <<"\t "<< itcores->second->getInstrDecoded(1) <<"\t "<< itcores->second->getInstrDecoded(2)<<"\t "<< itcores->second->getInstrDecoded(3)<< "\n";
      std::cout<<"IEW \t "<< itcores->second->getInstrExecuted(0) <<"\t "<< itcores->second->getInstrExecuted(1) <<"\t "<< itcores->second->getInstrExecuted(2)<<"\t "<< itcores->second->getInstrExecuted(3)<< "\n";
      std::cout<<"MEM \t "<< itcores->second->getInstrLoadStore(0) <<"\t "<< itcores->second->getInstrLoadStore(1) <<"\t "<< itcores->second->getInstrLoadStore(2)<<"\t "<< itcores->second->getInstrLoadStore(3)<< "\n";

      //     std::cout<<"Decoded Instr: NonProtected: "<< itcores->second->getInstrDecoded(0) <<" Protected: "<< itcores->second->getInstrDecoded(1) <<"\n";
      //     std::cout<<"Executed Instr: NonProtected: "<< itcores->second->getInstrExecuted(0) <<" Protected: "<< itcores->second->getInstrExecuted(1) <<"\n";
      //     std::cout<<"LoadStore Instr: NonProtected: "<< itcores->second->getInstrLoadStore(0) <<" Protected: "<< itcores->second->getInstrLoadStore(1) <<"\n";
      std::cout<<"Ticks : "<< itcores->second->getTicks() <<"\n";
    }

  }
}






