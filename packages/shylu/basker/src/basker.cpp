
#include "basker_types.hpp"
#include "basker_decl.hpp"
#include "basker_def.hpp"


#include <Kokkos_Core.hpp>

namespace BaskerNS{


  template class Basker<long int, double, Kokkos::OpenMP>;
  
  template class Basker<int, double, Kokkos::OpenMP>;
  //template class Basker<unsigned long, double, Kokkos::OpenMP>;
  //template class Basker<long long, double, Kokkos::OpenMP>;
 
}
