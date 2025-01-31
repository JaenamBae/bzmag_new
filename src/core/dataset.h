#ifndef BZMAG_CORE_TYPE_DATASET_H
#define BZMAG_CORE_TYPE_DATASET_H
/**
    @ingroup bzmagCoreType
    @class bzmag::DataSet
    @brief
*/

#include "platform.h"
#include "primitivetype.h"
#include "tuple2.h"
#include <vector>

namespace bzmag
{
    using DataPair = Tuple2<float64>;
    using DataSet = std::vector< DataPair >;
}

#endif // BZMAG_CORE_TYPE_DATASET_H
