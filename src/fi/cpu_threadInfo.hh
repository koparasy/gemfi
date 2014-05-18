#ifndef __CPU_THREAD_FAULT_INFO__
#define __CPU_THREAD_FAULT_INFO__


#include <map>

#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "debug/FaultInjection.hh"
#include "fi/faultq.hh"
#include "mem/mem_object.hh"
//#include "params/InjectedFault.hh"

/*
 * This file contains the 2 classes which are used for keeping track
 * of the threads/applications which have enabled fault injection
 * 
 */ 



class cpuExecutedTicks ; //forward declaration
class ThreadEnabledFault; 
class InjectedFaultQueue;
extern std::string allcores;
/*
 * class cpuExecutedTicks in reallity is a simple counter class which count how many
 * instructions/ticks has a thread executed on a specific core
 * 
 */

class cpuExecutedTicks {
	private:
		uint64_t instrFetched; //how many instructions has this core fetched for a specific thread
		uint64_t instrExexuted; // how many instructions has this core executed for a specific thread
		uint64_t instrLoadStore;
		/* 
		 * There is no meaning in counting the ticks of fetch,decode,execute seperately since
		 * a thread owns the pipeline even if only one instruction is inside the pipeline
		 */
		uint64_t _ticks; // how many ticks has a thread been executing on this core
		uint64_t mode; 
		std::string _name; // The name of this core (e.g. system.cpu1)
	public:

		cpuExecutedTicks(std:: string name);
		~cpuExecutedTicks();

		void setName(std:: string v){_name = v;} 

		void setInstrFetched(uint64_t v){instrFetched = v;  }
		void setInstrExecuted(uint64_t v){instrExexuted = v; }
		void setInstrLoadStore(uint64_t v){instrLoadStore = v;}

		void setTicks(uint64_t v){_ticks=v;}

		uint64_t getInstrFetched(){return instrFetched;}
		uint64_t getInstrExecuted(){return instrExexuted;}
		uint64_t getInstrLoadStore(){return instrLoadStore;}   

		uint64_t getTicks(){return _ticks;}

		std::string getName() {return _name;}

		uint64_t getMode() {return mode;}
		void setMode(uint64_t mod) {mode = mod;}
		void increaseFetchInstr() { instrFetched++;}
		void increaseExecInstr() {instrExexuted++;}
		void increaseLoadStoreInstr(){instrLoadStore++;}


		void increaseTicks(uint64_t ticks) {_ticks+=ticks;}

		void dump();
};

/*
 * This class represents a software thread (pthread,applciation)
 * Again the class encapsulates some counters
 */


class ThreadEnabledFault {
	friend class InjectedFault;
	private :

	Addr MagicInstVirtualAddr;  // Store the value of the Pc address when the framework is activated
	int threadId; // Given when fi_activate_inst is executed 
	int myId; // different for all threads something like hash id used only for debugging purposes.
	protected :
	std::map<string,cpuExecutedTicks*> cores; // Store all cores which this thread as ever execute an instruction
	std::map<string,cpuExecutedTicks*>::iterator itcores;
	cpuExecutedTicks *currentcore;
	cpuExecutedTicks *all;
	public:
	ThreadEnabledFault( int threadId , std::string name );
	~ThreadEnabledFault();


	void setMagicInstVirtualAddr(Addr v){ MagicInstVirtualAddr  = v; }
	void setThreaId(int v){ threadId  = v; }
	void setMyid(){
		static int my_id_counter = 0;
		myId = my_id_counter++;
	}

	int getMyId(){return myId ;}

	Addr getMagicInstVirtualAddr() { return MagicInstVirtualAddr; }
	int getThreaId(){ return threadId; }

	InjectedFault *copyFault(InjectedFault &source);
	void dump();

	void findThreadFaults(int ThreadsId);


	int increaseFetchedInstr(std:: string curCpu);
	int increaseDecodedInstr(std:: string curCpu);
	int increaseExecutedInstr(std:: string curCpu);
	int increaseLoadStoreInstr(std:: string curCpu);

	int increaseTicks(std :: string curCpu , uint64_t ticks);


	void CalculateFetchedTime(std:: string curCpu , uint64_t *fetched_instr,uint64_t *fetched_time);
	void CalculateDecodedTime(std::string curCpu , uint64_t *fetched_instr, uint64_t *fetched_time  );
	void CalculateExecutedTime(std:: string curCpu , uint64_t *exec_instr ,uint64_t *exec_time);
	void CalculateLoadStoreTime(std:: string curCpu , uint64_t *exec_instr ,uint64_t *exec_time);

	void print_time();

};

#endif //  __CPU_THREAD_FAULT_INFO__
