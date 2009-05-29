#ifndef NLLPREPREOCESSING_TYPELIST_CONCATENATE_H_
# define NLLPREPREOCESSING_TYPELIST_CONCATENATE_H_

/**
 @ingroup preprocessing
 @brief Define operation to add preprocessing units one by one
 */
# define TYPELIST_CONCATENATE_TYPE(T1, TYPELIST)                           nll::preprocessing::Typelist<T1, TYPELIST, TYPELIST::Classifier>
# define TYPELIST_CONCATENATE_VAL(T1, TYPELIST, T1_VAL, TYPELIST_VAL)      nll::preprocessing::Typelist<T1, TYPELIST, TYPELIST::Classifier>( (T1_VAL), (TYPELIST_VAL) )

#endif
