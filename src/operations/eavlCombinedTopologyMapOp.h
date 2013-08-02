// Copyright 2010-2013 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_COMBINED_TOPOLOGY_MAP_OP_H
#define EAVL_COMBINED_TOPOLOGY_MAP_OP_H

#include "eavlCUDA.h"
#include "eavlCellSet.h"
#include "eavlCellSetExplicit.h"
#include "eavlCellSetAllStructured.h"
#include "eavlDataSet.h"
#include "eavlArray.h"
#include "eavlOpDispatch.h"
#include "eavlOperation.h"
#include "eavlTopology.h"
#include "eavlException.h"
#include <time.h>
#include <omp.h>

#ifndef DOXYGEN

template <class CONN>
struct eavlCombinedTopologyMapOp_CPU
{
    static inline eavlArray::Location location() { return eavlArray::HOST; }
    template <class F, class IN0, class IN1, class OUT>
    static void call(int nitems, CONN &conn,
                     const IN0 s_inputs, const IN1 d_inputs, OUT outputs, F &functor)
    {
        int ids[MAX_LOCAL_TOPOLOGY_IDS];
        for (int index = 0; index < nitems; ++index)
        {
            int nids;
            int shapeType = conn.GetElementComponents(index, nids, ids);

            typename collecttype<IN1>::const_type in_d(collect(index, d_inputs));
            typename collecttype<OUT>::type out(collect(index, outputs));

            out = functor(shapeType, nids, ids, s_inputs, in_d);
        }
    }
};

#if defined __CUDACC__

template <class CONN>
struct eavlCombinedTopologyMapOp_GPU
{
    static inline eavlArray::Location location() { return eavlArray::DEVICE; }
    template <class F, class IN0, class IN1, class OUT>
    static void call(int nitems, CONN &conn,
                     const IN0 s_inputs, const IN1 d_inputs, OUT outputs, F &functor)
    {
        cerr << "IMPLEMENT ME!\n";
        ///\todo: implement!
    }
};


#endif

#endif

// ****************************************************************************
// Class:  eavlCombinedTopologyMapOp
//
// Purpose:
///   Map from one topological element in a mesh to another, with
///   input arrays on both the source and destination topology
///   and with outputs on the destination topology.
//
// Programmer:  Jeremy Meredith
// Creation:    August  1, 2013
//
// Modifications:
// ****************************************************************************
template <class IS, class ID, class O, class F>
class eavlCombinedTopologyMapOp : public eavlOperation
{
  protected:
    eavlCellSet *cells;
    eavlTopology topology;
    IS           s_inputs;
    ID           d_inputs;
    O            outputs;
    F            functor;
  public:
    eavlCombinedTopologyMapOp(eavlCellSet *c, eavlTopology t,
                      IS is, ID id, O o, F f)
        : cells(c), topology(t), s_inputs(is), d_inputs(id), outputs(o), functor(f)
    {
    }
    virtual void GoCPU()
    {
        eavlCellSetExplicit *elExp = dynamic_cast<eavlCellSetExplicit*>(cells);
        eavlCellSetAllStructured *elStr = dynamic_cast<eavlCellSetAllStructured*>(cells);
        int n = outputs.first.array->GetNumberOfTuples();
        if (elExp)
        {
            eavlExplicitConnectivity &conn = elExp->GetConnectivity(topology);
            eavlOpDispatch<eavlCombinedTopologyMapOp_CPU<eavlExplicitConnectivity> >(n, conn, s_inputs, d_inputs, outputs, functor);
        }
        else if (elStr)
        {
            eavlRegularConnectivity conn = eavlRegularConnectivity(elStr->GetRegularStructure(),topology);
            eavlOpDispatch<eavlCombinedTopologyMapOp_CPU<eavlRegularConnectivity> >(n, conn, s_inputs, d_inputs, outputs, functor);
        }
    }
    virtual void GoGPU()
    {
#ifdef HAVE_CUDA
        eavlCellSetExplicit *elExp = dynamic_cast<eavlCellSetExplicit*>(cells);
        eavlCellSetAllStructured *elStr = dynamic_cast<eavlCellSetAllStructured*>(cells);
        int n = outputs.first.array->GetNumberOfTuples();
        if (elExp)
        {
            eavlExplicitConnectivity &conn = elExp->GetConnectivity(topology);

            conn.shapetype.NeedOnDevice();
            conn.connectivity.NeedOnDevice();
            conn.mapCellToIndex.NeedOnDevice();

            eavlOpDispatch<eavlCombinedTopologyMapOp_GPU<eavlExplicitConnectivity> >(n, conn, s_inputs, d_inputs, outputs, functor);

            conn.shapetype.NeedOnHost();
            conn.connectivity.NeedOnHost();
            conn.mapCellToIndex.NeedOnHost();
        }
        else if (elStr)
        {
            eavlRegularConnectivity conn = eavlRegularConnectivity(elStr->GetRegularStructure(),topology);
            eavlOpDispatch<eavlCombinedTopologyMapOp_GPU<eavlRegularConnectivity> >(n, conn, s_inputs, d_inputs, outputs, functor);
        }
#else
        THROW(eavlException,"Executing GPU code without compiling under CUDA compiler.");
#endif
    }
};

// helper function for type deduction
template <class IS, class ID, class O, class F>
eavlCombinedTopologyMapOp<IS,ID,O,F> *new_eavlCombinedTopologyMapOp(eavlCellSet *c, eavlTopology t,
                                                    IS is, ID id, O o, F f) 
{
    return new eavlCombinedTopologyMapOp<IS,ID,O,F>(c,t,is,id,o,f);
}


#endif
