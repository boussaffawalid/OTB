/*=========================================================================
 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 Created By:Boussaffa Walid
 Email:	    Boussaffa.walid@outlook.com


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "otbFeatureSelection.h"

#include <boost/smart_ptr.hpp>
#include <exception>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "error.hpp"
#include "global.hpp"
#include "subset.hpp"

#include "data_intervaller.hpp"
#include "data_splitter.hpp"
#include "data_splitter_5050.hpp"
#include "data_splitter_cv.hpp"
//#include "data_splitter_holdout.hpp"
//#include "data_splitter_leave1out.hpp"
//#include "data_splitter_resub.hpp"
#include "data_splitter_randrand.hpp"
//#include "data_splitter_randfix.hpp"
#include "data_scaler.hpp"
#include "data_scaler_void.hpp"
//#include "data_scaler_to01.hpp"
//#include "data_scaler_white.hpp"

#include "data_accessor_splitting_memOTB.hpp"

#include "criterion_normal_bhattacharyya.hpp"
//#include "criterion_normal_gmahalanobis.hpp"
//#include "criterion_normal_divergence.hpp"
//#include "criterion_multinom_bhattacharyya.hpp"
#include "criterion_wrapper.hpp"
//#include "criterion_wrapper_bias_estimate.hpp"
//#include "criterion_subsetsize.hpp"
//#include "criterion_sumofweights.hpp"
//#include "criterion_negative.hpp"

#include "distance_euclid.hpp"
//#include "distance_L1.hpp"
#include "distance_Lp.hpp"
#include "classifier_knn.hpp"
//#include "classifier_normal_bayes.hpp"
//#include "classifier_multinom_naivebayes.hpp"
#include "classifier_svm.hpp"

//#include "search_bif.hpp"
//#include "search_bif_threaded.hpp"


//#include "search_exhaustive.hpp"
//#include "search_exhaustive_threaded.hpp"
#include "branch_and_bound_predictor_averaging.hpp"
//#include "search_branch_and_bound_basic.hpp"
#include "search_branch_and_bound_improved.hpp"
#include "search_branch_and_bound_partial_prediction.hpp"
#include "search_branch_and_bound_fast.hpp"
#include "seq_step_straight.hpp"
//#include "seq_step_straight_threaded.hpp"
//#include "seq_step_hybrid.hpp"
//#include "seq_step_ensemble.hpp"
//#include "search_seq_sfs.hpp"
#include "search_seq_sffs.hpp"
//#include "search_seq_sfrs.hpp"
//#include "search_seq_os.hpp"
#include "search_seq_dos.hpp"
#include "result_tracker_dupless.hpp"
//#include "result_tracker_regularizer.hpp"
#include "result_tracker_feature_stats.hpp"
//#include "result_tracker_stabileval.hpp"

namespace otb
{
namespace Wrapper
{

typedef double RETURNTYPE;
typedef float DATATYPE;
typedef double REALTYPE;
typedef int IDXTYPE;
typedef unsigned int DIMTYPE;
typedef short BINTYPE;
typedef FST::Subset<BINTYPE, DIMTYPE> SUBSET;


typedef FST::Data_Intervaller<std::vector<FST::Data_Interval<IDXTYPE> >,IDXTYPE> INTERVALLER;
typedef boost::shared_ptr<FST::Data_Splitter<INTERVALLER,IDXTYPE> > PSPLITTER;
typedef FST::Data_Splitter_CV<INTERVALLER,IDXTYPE> SPLITTERCV;
typedef FST::Data_Splitter_5050<INTERVALLER,IDXTYPE> SPLITTER5050;
typedef FST::Data_Splitter_RandomRandom<INTERVALLER,IDXTYPE,BINTYPE> SPLITTERRR;
typedef FST::Data_Accessor_Splitting_MemOTB<DATATYPE,IDXTYPE,INTERVALLER> DATAACCESSOR;
typedef FST::Distance_Euclid<DATATYPE,DIMTYPE,SUBSET> DISTANCE;

typedef FST::Classifier_kNN<RETURNTYPE,DATATYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR,DISTANCE> CLASSIFIERKNN;
typedef FST::Classifier_LIBSVM<RETURNTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR> CLASSIFIERSVM;

typedef FST::Criterion_Wrapper<RETURNTYPE,SUBSET,CLASSIFIERKNN,DATAACCESSOR> WRAPPERKNN;
typedef FST::Criterion_Wrapper<RETURNTYPE,SUBSET,CLASSIFIERSVM,DATAACCESSOR> WRAPPERSVM;

typedef FST::Sequential_Step_Straight<RETURNTYPE,DIMTYPE,SUBSET,WRAPPERKNN> EVALUATOR;
typedef FST::Sequential_Step_Straight<RETURNTYPE,DIMTYPE,SUBSET,WRAPPERSVM> EVALUATOR_SVM;


typedef FST::Result_Tracker_Feature_Stats<RETURNTYPE,IDXTYPE,DIMTYPE,SUBSET> TRACKERSTATS;


void FeatureSelection::InitFSTParams()
{
  
   AddChoice ( "method.sffs", "sffs Wrapper-based feature selection with Floating Search" );
   AddParameter ( ParameterType_Choice, "method.sffs.searchdirection", "Sequential Forward Floating Selection search procedure" );
   AddChoice("method.sffs.searchdirection.forward", "forward search ");
   AddChoice("method.sffs.searchdirection.backward", "backward search ");
   SetParameterString("method.sffs.searchdirection", "forward"); 
    

   AddChoice ( "method.dos", "dos Combined feature subset contents, size and SVM parameters optimization.." );
   AddParameter ( ParameterType_Choice, "method.dos.svmkernel", "svm kernel to use " );
   AddChoice("method.dos.svmkernel.linear", "linear kernel");
   AddChoice("method.dos.svmkernel.rbf", "rbf kernel  ");
   AddChoice("method.dos.svmkernel.poly", "poly kernel ");   
   SetParameterString("method.dos.svmkernel", "rbf");     
}


FeatureSelection::ImportanceVector FeatureSelection::getFeatureImportanceDOS ( ListSampleType::Pointer trainingListSample,
        LabelListSampleType::Pointer trainingLabeledListSample )
{

  
          std::cout << "Starting (Missing data substitution) Combined feature subset contents, size and SVM parameters optimization..." << std::endl;
        // randomly sample 50% of data for training and randomly sample (disjunct) 40% for independent testing of final classification performance
        PSPLITTER dsp_outer ( new SPLITTERRR ( 1, 50, 40 ) ); // (there will be one outer randomized split only)
        // in the course of search use the first half of data by 3-fold cross-validation in wrapper FS criterion evaluation
        PSPLITTER dsp_inner ( new SPLITTERCV ( 3 ) );
        // do not scale data
        const DATATYPE missing_value_code=5;
        boost::shared_ptr<FST::Data_Scaler<DATATYPE> > dsc ( new FST::Data_Scaler_void<DATATYPE> ( missing_value_code,1/*to choose correct constructor*/ ) );
        // set-up data access
        boost::shared_ptr<std::vector<PSPLITTER> > splitters ( new std::vector<PSPLITTER> );
        splitters->push_back ( dsp_outer );
        splitters->push_back ( dsp_inner );
        boost::shared_ptr<DATAACCESSOR> da ( new DATAACCESSOR ( trainingListSample,trainingLabeledListSample,splitters,dsc ) );
        da->initialize();
        // initiate access to split data parts
        da->setSplittingDepth ( 0 );
        if ( !da->getFirstSplit() ) throw FST::fst_error ( "50/40 random data split failed." );
        da->setSplittingDepth ( 1 );
        if ( !da->getFirstSplit() ) throw FST::fst_error ( "3-fold cross-validation failure." );
        // initiate the storage for subset to-be-selected + another one as temporary storage
        boost::shared_ptr<SUBSET> sub ( new SUBSET ( da->getNoOfFeatures() ) );
        boost::shared_ptr<SUBSET> sub_temp ( new SUBSET ( da->getNoOfFeatures() ) );
        // set-up SVM (interface to external library LibSVM)
        boost::shared_ptr<CLASSIFIERSVM> csvm ( new CLASSIFIERSVM );
	
        //csvm->set_kernel_type ( RBF ); // (option: LINEAR, RBF, POLY)
        
        std::string svm_kernel  = GetParameterAsString("method.dos.svmkernel");
	
	if( svm_kernel == "linear")
	    csvm->set_kernel_type ( LINEAR );
	   
	else if( svm_kernel == "poly")
	   csvm->set_kernel_type ( POLY );

	else 
	     csvm->set_kernel_type ( RBF );   
        
        csvm->initialize ( da );
        // wrap the SVM classifier to enable its usage as FS criterion (criterion value will be estimated by 3-fold cross-val.)
        boost::shared_ptr<WRAPPERSVM> wsvm ( new WRAPPERSVM );
        wsvm->initialize ( csvm,da );
        // set-up the standard sequential search step object (option: hybrid, ensemble)
        boost::shared_ptr<EVALUATOR_SVM> eval ( new EVALUATOR_SVM );
        // set-up Dynamic Oscillating Search procedure
        FST::Search_DOS<RETURNTYPE,DIMTYPE,SUBSET,WRAPPERSVM,EVALUATOR_SVM> srch ( eval );
        srch.set_delta ( 3 );
        // run the search
        std::cout << "Feature selection setup:" << std::endl << *da << std::endl << srch << std::endl << *wsvm << std::endl << std::endl;
        RETURNTYPE bestcritval_train, critval_train, critval_test;
        sub->select_all();
        csvm->optimize_parameters ( da,sub );
        double best_svm_param_C=csvm->get_parameter_C();
        double best_svm_param_gamma=csvm->get_parameter_gamma();
        double best_svm_param_coef0=csvm->get_parameter_coef0();
        bool stop=false;
        sub->deselect_all();
        if ( !srch.search ( 0,bestcritval_train,sub,wsvm, std::cout ) ) throw FST::fst_error ( "Search not finished." );

        /**
        		sub_temp->stateless_copy(*sub);
        		while(!stop)
        		{
        			csvm->optimize_parameters(da,sub);
        			if(!srch.search(0,critval_train,sub_temp,wsvm,std::cout)) throw FST::fst_error("Search not finished.");
        			if(critval_train>bestcritval_train)
        			{
        				bestcritval_train=critval_train;
        				sub->stateless_copy(*sub_temp);
        				best_svm_param_C=csvm->get_parameter_C();
        				best_svm_param_gamma=csvm->get_parameter_gamma();
        				best_svm_param_coef0=csvm->get_parameter_coef0();
        			} else stop=true;
        		}
        		std::cout << std::endl << "Search result: " << std::endl << *sub << std::endl << "Criterion value=" << bestcritval_train << std::endl << std::endl;
        **/
        // 	// (optionally) validate result by estimating SVM accuracy on selected feature sub-space on independent test data
// 		da->setSplittingDepth(0);
// 		csvm->set_parameter_C(best_svm_param_C);
// 		csvm->set_parameter_gamma(best_svm_param_gamma);
// 		csvm->set_parameter_coef0(best_svm_param_coef0);
// 		csvm->train(da,sub);
// 		csvm->test(critval_test,da);
// 		std::cout << "Validated SVM accuracy=" << critval_test << std::endl << std::endl;
//

ImportanceVector vImp;
        vImp.reserve ( sub->get_n() );
        for ( DIMTYPE d=0; d<sub->get_n(); d++ )
        {
            if ( sub->selected_raw ( d ) )
                vImp.push_back ( std::make_pair ( d+1, 1 ) );
            else
                vImp.push_back ( std::make_pair ( d+1, 0 ) );
        }


  
  return vImp;
}





FeatureSelection::ImportanceVector FeatureSelection::getFeatureImportanceSFFS ( ListSampleType::Pointer trainingListSample,
        LabelListSampleType::Pointer trainingLabeledListSample )
{

    


  // otbAppLogINFO ( "SFFS GetFeatureImporance " );

    std::cout << "Generalized sequential feature subset search..." << std::endl;
        // keep second half of data for independent testing of final classification performance
        PSPLITTER dsp_outer ( new SPLITTER5050() );
        // in the course of search use the first half of data by 3-fold cross-validation in wrapper FS criterion evaluation
        PSPLITTER dsp_inner ( new SPLITTERCV ( 3 ) );
        // do not scale data
        boost::shared_ptr<FST::Data_Scaler<DATATYPE> > dsc ( new FST::Data_Scaler_void<DATATYPE>() );
        // set-up data access
        boost::shared_ptr<std::vector<PSPLITTER> > splitters ( new std::vector<PSPLITTER> );
        splitters->push_back ( dsp_outer );
        splitters->push_back ( dsp_inner );

        //boost::shared_ptr<DATAACCESSOR> da(new DATAACCESSOR("data/speech_15.trn",splitters,dsc));
        boost::shared_ptr<DATAACCESSOR> da ( new DATAACCESSOR ( trainingListSample,trainingLabeledListSample,splitters,dsc ) );

        da->initialize();
        // initiate access to split data parts
        da->setSplittingDepth ( 0 );
        if ( !da->getFirstSplit() ) throw FST::fst_error ( "50/50 data split failed." );
        da->setSplittingDepth ( 1 );
        if ( !da->getFirstSplit() ) throw FST::fst_error ( "3-fold cross-validation failure." );
        // initiate the storage for subset to-be-selected
        boost::shared_ptr<SUBSET> sub ( new SUBSET ( da->getNoOfFeatures() ) );
        sub->deselect_all();
        // set-up 3-Nearest Neighbor classifier based on Euclidean distances
        boost::shared_ptr<CLASSIFIERKNN> cknn ( new CLASSIFIERKNN );
        cknn->set_k ( 5 );
        // wrap the 3-NN classifier to enable its usage as FS criterion (criterion value will be estimated by 3-fold cross-val.)
        boost::shared_ptr<WRAPPERKNN> wknn ( new WRAPPERKNN );
        wknn->initialize ( cknn,da );
        // set-up the standard sequential search step object (option: hybrid, ensemble, etc.)
        boost::shared_ptr<EVALUATOR> eval ( new EVALUATOR );
        // set-up Sequential Forward Floating Selection search procedure
        FST::Search_SFFS<RETURNTYPE,DIMTYPE,SUBSET,WRAPPERKNN,EVALUATOR> srch ( eval );
        
	
	//srch.set_search_direction ( FST::FORWARD ); // try FST::BACKWARD	
      std::string search_method = GetParameterAsString("method.sffs.searchdirection") ;
      
      if(search_method == "backward")
	srch.set_search_direction ( FST::BACKWARD ); 
      else
	srch.set_search_direction ( FST::FORWARD ); 

      
      // set the size of feature groups to be evaluated for inclusion/removal in each sequential step (can be applied to SFS, SFFS, OS, DOS, SFRS)
        srch.set_generalization_level ( 2 );
        // run the search
        std::cout << "Feature selection setup:" << std::endl << *da << std::endl << srch << std::endl << *wknn << std::endl << std::endl;
        RETURNTYPE critval_train, critval_test;
        srch.set_output_detail ( FST::NORMAL ); // set FST::SILENT to disable all text output in the course of search (FST::NORMAL is default)
        if ( !srch.search ( 0,critval_train,sub,wknn,std::cout ) ) throw FST::fst_error ( "Search not finished." );
        // (optionally) validate result by estimating kNN accuracy on selected feature sub-space on independent test data
        //da->setSplittingDepth(0);
        //cknn->train(da,sub);
        //cknn->test(critval_test,da);
        //std::cout << "Validated "<<cknn->get_k()<<"-NN accuracy=" << critval_test << std::endl << std::endl;
        // (optionally) list the best known solutions for each cardinality as recorded throughout the course of search
        //std::cout << "Best recorded solution for subset size:" << std::endl;
        //for(DIMTYPE d=1;d<=sub->get_n();d++)
        //if(srch.get_result(d,critval_train,sub)) std::cout << d << ": val="<< critval_train << ", "<<*sub << std::endl;

	ImportanceVector vImp;

        vImp.reserve ( sub->get_n() );
        for ( DIMTYPE d=0; d<sub->get_n(); d++ )
        {
            if ( sub->selected_raw ( d ) )
                vImp.push_back ( std::make_pair ( d+1, 1 ) );
            else
                vImp.push_back ( std::make_pair ( d+1, 0 ) );
        }
        
    return vImp;
}




} //end namespace wrapper
} //end namespace otb
