#ifndef __REGISTER_DECODING_INJECTED_FAULT_HH__
#define __REGISTER_DECODING_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"

/*
 * Insert a fault during decoding stage.
 */


class RegisterDecodingInjectedFault : public O3CPUInjectedFault
{
protected:
  typedef short RegisterDecodingInjectedFaultType;
  static const RegisterDecodingInjectedFaultType SrcRegisterInjectedFault = 1;
  static const RegisterDecodingInjectedFaultType DstRegisterInjectedFault = 2;


private:
  RegisterDecodingInjectedFaultType _srcOrDst;
  int _regToChange;
  int _changeToReg;

  int parseRegDec(std::string s);
  void setSrcOrDst(RegisterDecodingInjectedFaultType v) { _srcOrDst = v;}
  void setRegToChange(std::string v) { _regToChange = atoi(v.c_str());}
  void setChangeToReg(std::string v) { _changeToReg = atoi(v.c_str());}
  void setChangeToReg(int v){_changeToReg = v;}
  void setRegToChange(int v) {_regToChange =v;}


public:

  RegisterDecodingInjectedFault( std::ifstream &os);
  ~RegisterDecodingInjectedFault();

  virtual const char *description() const;
  
  void dump() const;
  
  virtual TheISA::MachInst process(TheISA::MachInst inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  virtual StaticInstPtr process(StaticInstPtr inst , int regNum) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;}; 
  StaticInstPtr process(StaticInstPtr inst); // StaticInstPtr contains the user visible
					      // Destination/source register
					      //so set them to the desired one.
    
  RegisterDecodingInjectedFaultType
  getSrcOrDst() const { return _srcOrDst;}

  int 
  getRegToChange() const { return _regToChange;}
  
  int 
  getChangeToReg() const { return _changeToReg;}

};

#endif // __REGISTER_DECODING_INJECTED_FAULT_HH__
