/*******************************************************************\

Module: Variable Mapping HDL<->C

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <set>
#include <sstream>

#include <i2string.h>
#include <config.h>
#include <arith_tools.h>
#include <std_types.h>

#include "gen_interface.h"

class gen_interfacet
{
public:
  gen_interfacet(contextt &_context,
                 std::ostream &_out,
                 std::ostream &_err):
    context(_context), out(_out), err(_err) { }

  void gen_interface(const symbolt &module, bool have_bound);

protected:
  contextt &context;
  std::ostream &out, &err;

  std::set<irep_idt> modules_done;
  std::set<irep_idt> modules_in_progress;

  std::map<std::string, std::string> typedef_map;
  std::stringstream stypedef;

  symbolt &lookup(const irep_idt &identifier);
  std::string gen_declaration(const symbolt &symbol);

  void gen_module(const symbolt &module, std::ostream& os);

  std::string type_to_string(const typet& type);
};

/*******************************************************************\

Function: gen_interfacet::lookup

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

symbolt &gen_interfacet::lookup(const irep_idt &identifier)
{
  contextt::symbolst::iterator it=
    context.symbols.find(identifier);

  if(it==context.symbols.end())
  {
    err << "failed to find identifier " << identifier << std::endl;
    throw 0;
  }

  return it->second;
}

/*******************************************************************\

Function: gen_interfacet::gen_declaration

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

std::string gen_interfacet::gen_declaration(const symbolt &symbol)
{
  std::string result;
  result += type_to_string(symbol.type);
  result += " ";
  result += id2string(symbol.base_name);
  return result;
}

/*******************************************************************\

Function: gen_interfacet::gen_declaration

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

std::string gen_interfacet::type_to_string(const typet& type)
{
  if(type.id()==ID_bool)
  {
    return "_Bool";
  }
  else if(type.id()==ID_unsignedbv || type.id()==ID_signedbv)
  {
    unsigned width=to_unsignedbv_type(type).get_width();
    
    std::string sign=type.id()==ID_unsignedbv?"unsigned ":"signed ";
    
    std::string type_str=sign;
    
    // does integer type fit?
    if(width==config.ansi_c.char_width)
      type_str+="char";
    else if(width==config.ansi_c.short_int_width)
      type_str+="short int";
    else if(width==config.ansi_c.int_width)
      type_str+="int";
    else if(width==config.ansi_c.long_int_width)
      type_str+="long int";
    else if(width==config.ansi_c.long_long_int_width)
      type_str+="long long int";
    else
      type_str+="__CPROVER_bitvector["+i2string(width)+"]";
    
    std::string name="_u"+i2string(width);

    std::map<std::string,std::string>::const_iterator cit =
      typedef_map.find(type_str);
      
    if(cit!=typedef_map.end())
      return cit->second; // it's already there

    typedef_map[type_str]=name;

    stypedef << "typedef " << type_str << " " 
             << name << ";" << std::endl;

    return name;
  }
  else if(type.id()==ID_array)
  {
    const exprt &size_expr=
      to_array_type(type).size();
 
    mp_integer size = 0;
    to_integer(size_expr, size);

    std::string stype_str = type_to_string(type.subtype());
    std::string array_str = "[" + integer2string(size)+"]" ;
    std::string key_str = stype_str + array_str;

    std::map<std::string,std::string>::const_iterator cit =
      typedef_map.find(key_str);
      
    if(cit != typedef_map.end())
    {
      // it's already there
      return cit->second;
    }

    std::string new_type =
      "_array_of_" + integer2string(size) +"_" + stype_str;
    
    typedef_map[key_str] = new_type;
    
    stypedef << "typedef " << stype_str << " " << new_type
             << array_str << ";" << std::endl;
    return new_type;
  }
  else
  {
    // what else can it be?
    err << "Case " << type.id() << " not implemented";
    throw 0;
  }
}

/*******************************************************************\

Function: gen_interfacet::module_struct

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void gen_interfacet::gen_module(
  const symbolt &module,
  std::ostream& os)
{
  if(modules_done.find(module.name)!=modules_done.end())
    return;

  os << std::endl;

  if(modules_in_progress.find(module.name)!=modules_in_progress.end())
  {
    err << "cyclic module instantiation in module " << module.name
        << std::endl;
    throw 0;
  }

  std::set<irep_idt>::iterator
    in_progress_it=modules_in_progress.insert(module.name).first;

  forall_symbol_module_map(it, context.symbol_module_map, module.name)
  {
    const symbolt &symbol=lookup(it->second);

    if(symbol.type.id()==ID_module_instance)
    {
      const symbolt &module_symbol=lookup(symbol.value.get(ID_module));
      gen_module(module_symbol,os);
    }
  }

  os << "/*" << std::endl
      << "  Module " << module.name << std::endl
      << "*/" << std::endl
      << std::endl;

  os << "struct module_" << module.base_name << " {" << std::endl;    

  forall_symbol_module_map(it, context.symbol_module_map, module.name)
  {
    const symbolt &symbol=lookup(it->second);

    if(symbol.type.id()==ID_module_instance)
    {
      const symbolt &module_symbol=lookup(symbol.value.get(ID_module));

      os << "  struct module_"
         << module_symbol.base_name
         << " " << symbol.base_name;

      os << ";" << std::endl;
    }
    else if(symbol.type.id()==ID_module)
    {
    }
    else if(symbol.hierarchy.empty() &&
            symbol.type.id()!=ID_integer &&
            !symbol.theorem)
    {
      os << "  " << gen_declaration(symbol)
         << ";" << std::endl;
    }
  }

  os << "};" << std::endl << std::endl;

  modules_in_progress.erase(in_progress_it);

  modules_done.insert(module.name);
}

/*******************************************************************\

Function: gen_interfacet::gen_interface

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void gen_interfacet::gen_interface(const symbolt &module, bool has_bound)
{
  if(has_bound)
  {
    out << "/* Unwinding Bound */\n"
           "\n"
           "extern const unsigned int bound;\n\n"
           "/* Next Timeframe  */\n"
           "\n"
           "void next_timeframe(void);\n"
           "void set_inputs(void);\n";
  }

  std::stringstream smodule;
  
  gen_module(module, smodule);
  
  out << "\n\n"
      << "/*\n"
      << "  Type declarations\n"
      << "*/\n\n";
      
  out << stypedef.str() << std::endl;
  out << smodule.str() << std::endl;

  out << "\n\n"
      << "/*\n"
      << "  Hierarchy Instantiation\n"
      << "*/\n\n";
      
  out << "extern struct module_" << module.base_name << " "
      << module.base_name << ";" << std::endl;

  out << std::endl;
}

/*******************************************************************\

Function: gen_interfacet::gen_interface

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void gen_interface(
  contextt &context,
  const symbolt &module,
  bool have_bound,
  std::ostream &out,
  std::ostream &err)
{
  gen_interfacet gen_interface(context, out, err);
  gen_interface.gen_interface(module, have_bound);
}
