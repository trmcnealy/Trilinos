#ifndef PANZER_EXPLICIT_TEMPLATE_INSTANTIATION_HPP
#define PANZER_EXPLICIT_TEMPLATE_INSTANTIATION_HPP

#include "Panzer_Traits.hpp"

// TWO template arguments for evaluators
#define PANZER_INSTANTIATE_TEMPLATE_CLASS_RESIDUAL(name) \
  template class name<panzer::Traits::Residual, panzer::Traits>; 

#define PANZER_INSTANTIATE_TEMPLATE_CLASS_JACOBIAN(name) \
  template class name<panzer::Traits::Jacobian, panzer::Traits>; 

#define PANZER_INSTANTIATE_TEMPLATE_CLASS(name) \
  PANZER_INSTANTIATE_TEMPLATE_CLASS_RESIDUAL(name) \
  PANZER_INSTANTIATE_TEMPLATE_CLASS_JACOBIAN(name)

// ONE template argument for fill 
#define PANZER_INSTANTIATE_TEMPLATE_CLASS_RESIDUAL_ONE_T(name) \
  template class name<panzer::Traits::Residual>; 

#define PANZER_INSTANTIATE_TEMPLATE_CLASS_JACOBIAN_ONE_T(name) \
  template class name<panzer::Traits::Jacobian>; 

#define PANZER_INSTANTIATE_TEMPLATE_CLASS_ONE_T(name) \
  PANZER_INSTANTIATE_TEMPLATE_CLASS_RESIDUAL_ONE_T(name) \
  PANZER_INSTANTIATE_TEMPLATE_CLASS_JACOBIAN_ONE_T(name)

#endif
