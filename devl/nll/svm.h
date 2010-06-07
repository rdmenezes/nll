#ifndef _LIBSVM_H
#define _LIBSVM_H

#define LIBSVM_VERSION 291

namespace nll
{
namespace algorithm
{

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

   NLL_API struct svm_model *svm_train(const struct svm_problem *prob, const struct svm_parameter *param);
   NLL_API void svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *target);

   NLL_API int svm_save_model(const char *model_file_name, const struct svm_model *model);
   NLL_API struct svm_model *svm_load_model(const char *model_file_name);

   NLL_API int svm_get_svm_type(const struct svm_model *model);
   NLL_API int svm_get_nr_class(const struct svm_model *model);
   NLL_API void svm_get_labels(const struct svm_model *model, int *label);
   NLL_API double svm_get_svr_probability(const struct svm_model *model);

   NLL_API double svm_predict_values(const struct svm_model *model, const struct svm_node *x, double* dec_values);
   NLL_API double svm_predict(const struct svm_model *model, const struct svm_node *x);
   NLL_API double svm_predict_probability(const struct svm_model *model, const struct svm_node *x, double* prob_estimates);

   NLL_API void svm_destroy_model(struct svm_model *model);
   NLL_API void svm_destroy_param(struct svm_parameter *param);

   NLL_API const char *svm_check_parameter(const struct svm_problem *prob, const struct svm_parameter *param);
   NLL_API int svm_check_probability_model(const struct svm_model *model);

   NLL_API void svm_set_print_string_function(void (*print_func)(const char *));

}
}
#endif /* _LIBSVM_H */
