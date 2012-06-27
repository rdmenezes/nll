#ifndef CORE_ALGORITHM_LIBLINEAR_H_
# define CORE_ALGORITHM_LIBLINEAR_H_

/*
Copyright (c) 2007-2010 The LIBLINEAR Project.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// NOTES: based on liblinear 1.7
// http://www.csie.ntu.edu.tw/~cjlin/liblinear/
//

# define LIBLINEAR_VERSION 17

namespace nll
{
namespace algorithm
{
	struct NLL_API feature_node
	{
		int index;
		double value;
	};

   struct NLL_API problem
   {
	   int l, n;
	   int *y;
	   struct feature_node **x;
	   double bias;            /* < 0 if no bias term */  
	   double *W;              /* instance weight */
   };

	enum { L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL }; /* solver_type */

	struct NLL_API parameter
	{
		int solver_type;

		/* these are for training only */
		double eps;	        /* stopping criteria */
		double C;
		int nr_weight;
		int *weight_label;
		double* weight;
	};

	struct NLL_API model
	{
		struct parameter param;
		int nr_class;		/* number of classes */
		int nr_feature;
		double *w;
		int *label;		/* label of each class */
		double bias;
	};

	NLL_API struct model* train(const struct problem *prob, const struct parameter *param);
	NLL_API void cross_validation(const struct problem *prob, const struct parameter *param, int nr_fold, int *target);

	NLL_API int predict_values(const struct model *model_, const struct feature_node *x, double* dec_values);
	NLL_API int predict(const struct model *model_, const struct feature_node *x);
	NLL_API int predict_probability(const struct model *model_, const struct feature_node *x, double* prob_estimates);

	NLL_API int save_model(const char *model_file_name, const struct model *model_);
	NLL_API struct model *load_model(const char *model_file_name);

	NLL_API int get_nr_feature(const struct model *model_);
	NLL_API int get_nr_class(const struct model *model_);
	NLL_API void get_labels(const struct model *model_, int* label);

	NLL_API void free_model_content(struct model *model_ptr);
	NLL_API void free_and_destroy_model(struct model **model_ptr_ptr);
	NLL_API void destroy_param(struct parameter *param);

	NLL_API const char *check_parameter(const struct problem *prob, const struct parameter *param);
	NLL_API int check_probability_model(const struct model *model);
	NLL_API void set_print_string_function(void (*print_func) (const char*));
}
}

#endif

