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

#define NONPROTECTED 0
#define PROTECTED 1
#define NOPINST 2
#define OTHEROBJECT 3
#define CATEGORIES 4


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
		uint64_t instrFetched[CATEGORIES]; //how many instructions has this core fetched for a specific thread
    uint64_t instrDecoded[CATEGORIES];
		uint64_t instrExexuted[CATEGORIES]; // how many instructions has this core executed for a specific thread
		uint64_t instrLoadStore[CATEGORIES];
    
		/* 
		 * There is no meaning in counting the ticks of fetch,decode,execute seperately since
		 * a thread owns the pipeline even if only one instruction is inside the pipeline
		 */
		uint64_t _ticks; // how many ticks has a thread been executing on this core

		std::string _name; // The name of this core (e.g. system.cpu1)
	public:

		cpuExecutedTicks(std:: string name);
		~cpuExecutedTicks();

		void setName(std:: string v){_name = v;} 

		void setInstrFetched(uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){instrFetched[PROTECTED] = v1;  instrFetched[NONPROTECTED] = v2; instrFetched[NOPINST] = v3;  instrFetched[OTHEROBJECT] = v4; }
    void setInstrDecoded(uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){instrDecoded[PROTECTED] = v1; instrDecoded[NONPROTECTED] = v2; instrDecoded[NOPINST] = v3;  instrDecoded[OTHEROBJECT] = v4;}
		void setInstrExecuted(uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){instrExexuted[PROTECTED] = v1;instrExexuted[NONPROTECTED] = v2; instrExexuted[NOPINST] = v3;  instrExexuted[OTHEROBJECT] = v4; }
		void setInstrLoadStore(uint64_t v1, uint64_t v2, uint64_t v3 , uint64_t v4){instrLoadStore[PROTECTED] = v1;instrLoadStore[NONPROTECTED] = v2; instrLoadStore[NOPINST] = v3;  instrLoadStore[OTHEROBJECT] = v4;}

		void setTicks(uint64_t v){_ticks=v;}

		uint64_t* getInstrFetched(){return instrFetched;}
    uint64_t* getInstrDecoded(){return instrDecoded;}
		uint64_t* getInstrExecuted(){return instrExexuted;}
		uint64_t* getInstrLoadStore(){return instrLoadStore;}   


  	uint64_t getInstrFetched(int id){return instrFetched[id];}
    uint64_t getInstrDecoded(int id){return instrDecoded[id];}
		uint64_t getInstrExecuted(int id){return instrExexuted[id];}
		uint64_t getInstrLoadStore(int id){return instrLoadStore[id];}

		uint64_t getTicks(){return _ticks;}

		std::string getName() {return _name;}

		void increaseFetchInstr(int mode) { instrFetched[mode]++;}
    void increaseDecodeInstr(int mode){ instrDecoded[mode]++;}
		void increaseExecInstr(int mode) { instrExexuted[mode]++;}
		void increaseLoadStoreInstr(int mode){ instrLoadStore[mode]++;}


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
	uint64_t mode; 
	Addr MagicInstVirtualAddr;  // Store the value of the Pc address when the framework is activated
	int threadId; // Given when fi_activate_inst is executed 
	int myId; // different for all threads something like hash id used only for debugging purposes.
	uint64_t isfaulty;
  int isInstProtected;
	protected :
	std::map<string,cpuExecutedTicks*> cores; // Store all cores which this thread as ever execute an instruction
	std::map<string,cpuExecutedTicks*>::iterator itcores;
	cpuExecutedTicks *all;
	public:
	ThreadEnabledFault( int threadId , std::string name );
	~ThreadEnabledFault();


	void setMagicInstVirtualAddr(Addr v){ MagicInstVirtualAddr  = v; }
	void setfaulty(uint64_t val){ isfaulty=val;}
	uint64_t getfaulty(){ return isfaulty;}
	void setThreadId(int v){ threadId  = v; }
	void setMyid(){
		static int my_id_counter = 0;
		myId = my_id_counter++;
	}

	int getMyId(){return myId ;}

	Addr getMagicInstVirtualAddr() { return MagicInstVirtualAddr; }
	int getThreadId(){ return threadId; }

	InjectedFault *copyFault(InjectedFault &source);
	void dump();

	void findThreadFaults(int ThreadsId);


	int increaseFetchedInstr(std:: string curCpu);
  int increaseDecodedInstr(std:: string curCpu);
	int increaseExecutedInstr(std:: string curCpu);
	int increaseLoadStoreInstr(std:: string curCpu);

	int increaseTicks(std :: string curCpu , uint64_t ticks);

  void setInstMode(int val){isInstProtected = val;};
  int getInstMode(){return isInstProtected;}


	void CalculateFetchedTime(std:: string curCpu , uint64_t *fetched_instr,uint64_t *fetched_time);
	void CalculateDecodedTime(std::string curCpu , uint64_t *fetched_instr, uint64_t *fetched_time  );
	void CalculateExecutedTime(std:: string curCpu , uint64_t *exec_instr ,uint64_t *exec_time);
	void CalculateLoadStoreTime(std:: string curCpu , uint64_t *exec_instr ,uint64_t *exec_time);

	void print_time();
	
	uint64_t getMode() {return mode;}
	void setMode(uint64_t mod) {mode = mod;}

};

#endif //  __CPU_THREAD_FAULT_INFO__
