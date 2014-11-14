#ifndef __LOAD_STORE_INJECTED_FAULT_HH__
#define __LOAD_STORE_INJECTED_FAULT_HH__

#include "base/types.hh"
#include "fi/faultq.hh"
#include "cpu/o3/cpu.hh"
#include "fi/o3cpu_injfault.hh"


class LoadStoreInjectedFault : public O3CPUInjectedFault
{
	public:

		LoadStoreInjectedFault(std::ifstream &os);
		~LoadStoreInjectedFault();

		virtual StaticInstPtr process(StaticInstPtr inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
		virtual StaticInstPtr process(StaticInstPtr inst , int regNum) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};


		virtual const char *description() const;
		void dump() const;

		template <class T> 
			T process(T v){
				DPRINTF(FaultInjection,"===LoadStoreFault::Process(T)==\n");

				T retval = v;
				retval = manifest(v,getValue(),getValueType());

				check4reschedule();

				DPRINTF(FaultInjection,"~===LoadStoreFault::Process(T)===\n");
				return retval;
			}

		virtual TheISA::MachInst process(uint64_t v) { 
			DPRINTF(FaultInjection,"===LoadStoreFault::Process(uint64_t)==\n");

			uint64_t retval = v;
			retval = manifest(v,getValue(),getValueType());

			check4reschedule();

			DPRINTF(FaultInjection,"~===LoadStoreFault::Process(uint64_4)===\n");
			return retval;
		}

};


#endif // __LOAD_STORE_INJECTED_FAULT_HH__
