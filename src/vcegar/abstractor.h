/*******************************************************************\

Module: Abstractor (generates abstract program given a set of predicates)

Author: Himanshu Jain

Date: June 2004 

\*******************************************************************/

#ifndef CPROVER_ABSTRACTOR_H
#define CPROVER_ABSTRACTOR_H

#include <util/message.h>
#include <util/namespace.h>
#include <util/cmdline.h>

#include "concrete_trans.h"
#include "abstract_trans.h"
#include "predicates.h"
#include "partitioning.h"
#include "network_info.h"

class abstractort:public messaget
{
public:
  abstractort(message_handlert &_message_handler,
	      const cmdlinet &_cmdline) : 
    messaget(_message_handler),
    cmdline(_cmdline)
  {
    //some initializations
    max_trans_cluster_size = 0;
    max_init_state_cluster_size = 0;
    num_trans_clusters = 0;
    num_init_clusters = 0;

    if(!cmdline.isset("partition"))
      partitioning_strategy = 6; //default
    else 
      partitioning_strategy=atoi(cmdline.getval("partition"));
     
    #ifdef DEBUG
    std::cout <<"partitioning_strategy is: "<<partitioning_strategy<<"\n";
    #endif

    if (!(partitioning_strategy==1 || 
	  partitioning_strategy==2 || 
	  partitioning_strategy==3 || 
	  partitioning_strategy==4 ||
	  partitioning_strategy==5 ||
	  partitioning_strategy==6 ||
	  partitioning_strategy==7 ||
	  partitioning_strategy==8)) 
	 throw "--partition option takes only 1, 2, 3, 4, 5, 6, 7, 8 as values ";

    if(!cmdline.isset("showcubes"))
      show_cubes = 0; //default
    else 
      show_cubes = 1;

    if(!cmdline.isset("gcr"))
      use_refinement_clusters = 0; //default
    else 
      use_refinement_clusters = 1;

    refinement_clusters.clear();

    if(!cmdline.isset("relate-init"))
      relate_init_predicates = 0; //default
    else 
      relate_init_predicates = 1;

    //for refinement generated clusters
    if(!cmdline.isset("nocache"))
    {
       pred_id_clusters_caching = 1; //default
       init_state_caching = 1;
       trans_rel_caching = 1;
    }
    else
    {
      pred_id_clusters_caching = 0;
      init_state_caching = 0;
      trans_rel_caching = 0;
    }

    pred_id_cache_num_access = 0;
    init_cache_num_access = 0;
    trans_cache_num_access = 0;
     
    pred_id_cache_num_hits = 0;
    init_cache_num_hits = 0;
    trans_cache_num_hits = 0;

    if(!cmdline.isset("noinit")) //do not compute initial states of abstraction
      abs_init_states = 1; //default
    else 
      abs_init_states = 0;

    if (cmdline.isset("verbose"))
      verbose=true;
    else
      verbose=false;
   }

  ~abstractort()
  {
  }

  const cmdlinet &cmdline;
  unsigned abstraction_strategy; //to be implemented
  unsigned partitioning_strategy;
  unsigned max_trans_cluster_size;
  unsigned max_init_state_cluster_size;
  unsigned num_init_clusters;
  unsigned num_trans_clusters;

  unsigned int pred_id_cache_num_access;
  unsigned int init_cache_num_access ;
  unsigned int trans_cache_num_access ;
  unsigned int init_cache_num_hits;
  unsigned int trans_cache_num_hits;
  unsigned int pred_id_cache_num_hits;
  bool pred_id_clusters_caching;
  bool init_state_caching;
  bool trans_rel_caching;

  bool show_cubes;
  bool use_refinement_clusters;
  bool relate_init_predicates;

  bool abs_init_states;

  // Generates the abstract program given a concrete program
  // and a set of predicates. The concrete program will not be touched.
  // The concrete program is assumed not to change between calls.
  void calc_abstraction(
    const predicatest &predicates,
    const concrete_transt &concrete_trans,
    const namespacet &ns,
    const network_infot &network,
    const partitioningt::pred_id_clusterst &pred_id_clusters);

  void abstract_variables(
    const predicatest &predicates,
    abstract_transt::variablest &variables);

  abstract_transt abstract_trans;

  partitioningt::predicate_clusterst pred_clusters, 
    init_pred_clusters;
  
  //These are generated by simulator based upon
  //the unsatisfiable cores for abstract spurious steps.
  partitioningt::predicate_clusterst 
    refinement_clusters;

  void out_stats(std::ostream &out);

protected:
  typedef std::map<partitioningt::pred_id_set_pairt, abstract_transition_relationt> trans_cubes_cachet;
  typedef std::map<partitioningt::pred_id_set_pairt, abstract_initial_statest> states_cubes_cachet;

  trans_cubes_cachet refine_cubes_cache;  
  trans_cubes_cachet trans_cubes_cache;  //cluster transition relation cache 
  states_cubes_cachet init_cubes_cache;  //initial states cache

  static void rename_to_next(exprt &current_pred);



  void calc_abstract_trans_rel
    (const predicatest &cluster, 
     const concrete_transt &concrete_trans,
     const transt &trans, //follow macros is assumed to be done
     abstract_transition_relationt 
     &abstract_transition_relation,
     const namespacet &ns);

  void calc_abstract_initial_states
    (const predicatest &cluster, 
     const concrete_transt &concrete_trans,
     const transt &trans, //follow macros is assumed to be done
     abstract_initial_statest 
     &abstract_initial_states,
     const namespacet &ns);

  void calc_abstraction_with_partition(
    const predicatest &predicates, 
    const concrete_transt &concrete_trans,
    const namespacet &ns,
    const network_infot &network,
    const partitioningt::pred_id_clusterst &pred_id_clusters);

  void relate_initial_state_predicates(
    const predicatest &cluster, 
    const concrete_transt &concrete_trans,
    abstract_transition_relationt 
    &abstract_transition_relation,
    const namespacet &ns);

  void calc_init_states_abstraction(
    const predicatest &predicates, 
    const concrete_transt &concrete_trans,
    const transt &trans,
    const namespacet &ns,
    const network_infot &network);

  void calc_trans_rel_abstraction(
    const predicatest &predicates, 
    const concrete_transt &concrete_trans,
    const transt &trans,
    const namespacet &ns,
    const network_infot &network);

  void refinement_guided_partitioning_abstraction(
    const predicatest &predicates, 
    const concrete_transt &concrete_trans,
    const transt &trans,
    const namespacet &ns,
    const network_infot &network,
    const partitioningt::pred_id_clusterst &pred_id_clusters);

  bool verbose;
};

#endif
