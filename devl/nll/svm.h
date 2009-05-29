#ifndef NLL_SVM_H_
#define NLL_SVM_H_

//
// code from svmlib
//
namespace nll
{
namespace algorithm
{
//#ifdef __cplusplus
//extern "C" {
//#endif
   struct NLL_API svm_node
   {
	   int index;
	   double value;
   };

   struct NLL_API svm_problem
   {
	   int l;
	   double *y;
	   struct svm_node **x;
   };

   enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
   enum { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED }; /* kernel_type */

   struct NLL_API svm_parameter
   {
	   int svm_type;
	   int kernel_type;
	   int degree;	/* for poly */
	   double gamma;	/* for poly/rbf/sigmoid */
	   double coef0;	/* for poly/sigmoid */

	   /* these are for training only */
	   double cache_size; /* in MB */
	   double eps;	/* stopping criteria */
	   double C;	/* for C_SVC, EPSILON_SVR and NU_SVR */
	   int nr_weight;		/* for C_SVC */
	   int *weight_label;	/* for C_SVC */
	   double* weight;		/* for C_SVC */
	   double nu;	/* for NU_SVC, ONE_CLASS, and NU_SVR */
	   double p;	/* for EPSILON_SVR */
	   int shrinking;	/* use the shrinking heuristics */
	   int probability; /* do probability estimates */
   };

   NLL_API struct svm_model* svm_train(const struct svm_problem *prob, const struct svm_parameter *param);
   void NLL_API svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *target);

   int NLL_API svm_save_model(const char *model_file_name, const struct svm_model *model);
   struct svm_model* svm_load_model(const char *model_file_name);

   int NLL_API svm_get_svm_type(const struct svm_model *model);
   int NLL_API svm_get_nr_class(const struct svm_model *model);
   void NLL_API svm_get_labels(const struct svm_model *model, int *label);
   double NLL_API svm_get_svr_probability(const struct svm_model *model);

   void NLL_API svm_predict_values(const struct svm_model *model, const struct svm_node *x, double* dec_values);
   double NLL_API svm_predict(const struct svm_model *model, const struct svm_node *x);
   double NLL_API svm_predict_probability(const struct svm_model *model, const struct svm_node *x, double* prob_estimates);

   void NLL_API svm_destroy_model(struct svm_model *model);
   void NLL_API svm_destroy_param(struct svm_parameter *param);

   NLL_API const char* svm_check_parameter(const struct svm_problem *prob, const struct svm_parameter *param);
   int NLL_API svm_check_probability_model(const struct svm_model *model);

//#ifdef __cplusplus
//}
//#endif
}
}
#endif
