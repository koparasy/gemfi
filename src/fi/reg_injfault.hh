#ifndef __REGISTER_INJECTED_FAULT_HH__
#define __REGISTER_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"


//class RegisterInjectedFaultParams;

/*
 * Insert the fault into a register. The register may be integer,floating point or even
 * misc.
 */

class RegisterInjectedFault : public CPUInjectedFault
{

protected:
  //Type of register to be injected with fault
  typedef short RegisterType;
  static const RegisterType IntegerRegisterFault = 1;
  static const RegisterType FloatRegisterFault   = 2;
  static const RegisterType MiscRegisterFault    = 3;

private:

  int _register;

  RegisterType _regType;

  void setRegister(int v) { _register = v;}
  void setRegType(RegisterType v){_regType = v;}
  
  void setRegType(std::string v) 
  { 
    if (v.compare("int") == 0) {
      _regType = RegisterInjectedFault::IntegerRegisterFault;
    }
    else if (v.compare("float") == 0) {
      _regType = RegisterInjectedFault::FloatRegisterFault;
    }
    else if (v.compare("misc") == 0) {
      _regType = RegisterInjectedFault::MiscRegisterFault;
    }
    else {
      std::cout << "RegisterInjectedFault::setRegType() -- Error parsing setRegType() " <<v <<"\n";
      assert(0);
    }
  }

public:

  RegisterInjectedFault( std::ifstream &os);
  ~RegisterInjectedFault();


  virtual const char *description() const;
  
  void dump() const;

  int process();


  int getRegister() const { return _register;}
  RegisterType getRegType() const { return _regType;}

};

#endif // __REGISTER_INJECTED_FAULT_HH__
