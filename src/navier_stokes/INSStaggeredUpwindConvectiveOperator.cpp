// Filename: INSStaggeredUpwindConvectiveOperator.cpp
// Created on 07 Sep 2012 by Boyce Griffith
//
// Copyright (c) 2002-2014, Boyce Griffith
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of The University of North Carolina nor the names of
//      its contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/////////////////////////////// INCLUDES /////////////////////////////////////

#include <stddef.h>
#include <ostream>
#include <string>
#include <vector>

#include "SAMRAI/pdat/ArrayData.h"
#include "SAMRAI/hier/Box.h"
#include "SAMRAI/geom/CartesianPatchGeometry.h"
#include "SAMRAI/pdat/FaceData.h"
#include "SAMRAI/pdat/FaceIndex.h"
#include "SAMRAI/pdat/FaceIterator.h"
#include "IBAMR_config.h"
#include "SAMRAI/hier/Index.h"
#include "SAMRAI/hier/IntVector.h"

#include "SAMRAI/hier/Patch.h"
#include "SAMRAI/hier/PatchHierarchy.h"
#include "SAMRAI/hier/PatchLevel.h"
#include "SAMRAI/solv/SAMRAIVectorReal.h"
#include "SAMRAI/pdat/SideData.h"
#include "SAMRAI/pdat/SideGeometry.h"
#include "SAMRAI/pdat/SideVariable.h"
#include "SAMRAI/hier/Variable.h"
#include "SAMRAI/hier/VariableContext.h"
#include "SAMRAI/hier/VariableDatabase.h"
#include "boost/array.hpp"
#include "ibamr/ConvectiveOperator.h"
#include "ibamr/INSStaggeredUpwindConvectiveOperator.h"
#include "ibamr/StaggeredStokesPhysicalBoundaryHelper.h"
#include "ibamr/ibamr_enums.h"
#include "ibamr/ibamr_utilities.h"
#include "ibamr/namespaces.h" // IWYU pragma: keep
#include "ibtk/HierarchyGhostCellInterpolation.h"
#include "SAMRAI/tbox/Database.h"

#include "SAMRAI/tbox/Timer.h"
#include "SAMRAI/tbox/TimerManager.h"
#include "SAMRAI/tbox/Utilities.h"

namespace SAMRAI
{
namespace solv
{

class RobinBcCoefStrategy;
} // namespace solv
} // namespace SAMRAI

// FORTRAN ROUTINES
#if (NDIM == 2)
#define ADVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(advect_derivative2d, ADVECT_DERIVATIVE2D)
#define CONVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(convect_derivative2d, CONVECT_DERIVATIVE2D)
#define NAVIER_STOKES_INTERP_COMPS_FC IBAMR_FC_FUNC_(navier_stokes_interp_comps2d, NAVIER_STOKES_INTERP_COMPS2D)
#define SKEW_SYM_DERIVATIVE_FC IBAMR_FC_FUNC_(skew_sym_derivative2d, SKEW_SYM_DERIVATIVE2D)
#endif

#if (NDIM == 3)
#define ADVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(advect_derivative3d, ADVECT_DERIVATIVE3D)
#define CONVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(convect_derivative3d, CONVECT_DERIVATIVE3D)
#define NAVIER_STOKES_INTERP_COMPS_FC IBAMR_FC_FUNC_(navier_stokes_interp_comps3d, NAVIER_STOKES_INTERP_COMPS3D)
#define SKEW_SYM_DERIVATIVE_FC IBAMR_FC_FUNC_(skew_sym_derivative3d, SKEW_SYM_DERIVATIVE3D)
#endif

extern "C" {
void ADVECT_DERIVATIVE_FC(const double*,
#if (NDIM == 2)
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const double*,
                          const double*,
                          const double*,
                          const double*,
                          const int&,
                          const int&,
#endif
#if (NDIM == 3)
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const double*,
                          const double*,
                          const double*,
                          const double*,
                          const double*,
                          const double*,
                          const int&,
                          const int&,
                          const int&,
#endif
                          double*);

void CONVECT_DERIVATIVE_FC(const double*,
#if (NDIM == 2)
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const int&,
                           const int&,
#endif
#if (NDIM == 3)
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const int&,
                           const int&,
                           const int&,
#endif
                           double*);

void NAVIER_STOKES_INTERP_COMPS_FC(
#if (NDIM == 2)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*
#endif
#if (NDIM == 3)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*
#endif
    );

void SKEW_SYM_DERIVATIVE_FC(const double*,
#if (NDIM == 2)
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const int&,
                            const int&,
#endif
#if (NDIM == 3)
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const int&,
                            const int&,
                            const int&,
#endif
                            double*);
}

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBAMR
{
/////////////////////////////// STATIC ///////////////////////////////////////

namespace
{
// NOTE: The number of ghost cells required by the Godunov advection scheme
// depends on the order of the reconstruction.  This value is chosen to work
// with the simple first-order upwind method.
static const int GADVECTG = 2;

// Timers.
static boost::shared_ptr<Timer> t_apply_convective_operator;
static boost::shared_ptr<Timer> t_apply;
static boost::shared_ptr<Timer> t_initialize_operator_state;
static boost::shared_ptr<Timer> t_deallocate_operator_state;
}

/////////////////////////////// PUBLIC ///////////////////////////////////////

INSStaggeredUpwindConvectiveOperator::INSStaggeredUpwindConvectiveOperator(
    const std::string& object_name,
    boost::shared_ptr<Database> input_db,
    const ConvectiveDifferencingType difference_form,
    const std::vector<RobinBcCoefStrategy*>& bc_coefs)
    : ConvectiveOperator(object_name, difference_form), d_bc_coefs(bc_coefs), d_bdry_extrap_type("CONSTANT"),
      d_hierarchy(NULL), d_coarsest_ln(-1), d_finest_ln(-1), d_U_var(NULL), d_U_scratch_idx(-1)
{
    if (d_difference_form != ADVECTIVE && d_difference_form != CONSERVATIVE && d_difference_form != SKEW_SYMMETRIC)
    {
        TBOX_ERROR("INSStaggeredUpwindConvectiveOperator::INSStaggeredUpwindConvectiveOperator():\n"
                   << "  unsupported differencing form: "
                   << enum_to_string<ConvectiveDifferencingType>(d_difference_form) << " \n"
                   << "  valid choices are: ADVECTIVE, CONSERVATIVE, SKEW_SYMMETRIC\n");
    }

    VariableDatabase* var_db = VariableDatabase::getDatabase();
    auto context = var_db->getContext("INSStaggeredUpwindConvectiveOperator::CONTEXT");

    if (input_db)
    {
        if (input_db->keyExists("bdry_extrap_type")) d_bdry_extrap_type = input_db->getString("bdry_extrap_type");
    }

    const std::string U_var_name = "INSStaggeredUpwindConvectiveOperator::U";
    d_U_var = var_db->getVariable(U_var_name);
    if (d_U_var)
    {
        d_U_scratch_idx = var_db->mapVariableAndContextToIndex(d_U_var, context);
    }
    else
    {
        d_U_var = boost::make_shared<SideVariable<double> >(DIM, U_var_name);
        d_U_scratch_idx = var_db->registerVariableAndContext(d_U_var, context, IntVector(DIM, GADVECTG));
    }
    TBOX_ASSERT(d_U_scratch_idx >= 0);

    // Setup Timers.
    IBAMR_DO_ONCE(t_apply_convective_operator = TimerManager::getManager()->getTimer(
                      "IBAMR::INSStaggeredUpwindConvectiveOperator::applyConvectiveOperator()");
                  t_apply =
                      TimerManager::getManager()->getTimer("IBAMR::INSStaggeredUpwindConvectiveOperator::apply()");
                  t_initialize_operator_state = TimerManager::getManager()->getTimer(
                      "IBAMR::INSStaggeredUpwindConvectiveOperator::initializeOperatorState()");
                  t_deallocate_operator_state = TimerManager::getManager()->getTimer(
                      "IBAMR::INSStaggeredUpwindConvectiveOperator::deallocateOperatorState()"););
    return;
} // INSStaggeredUpwindConvectiveOperator

INSStaggeredUpwindConvectiveOperator::~INSStaggeredUpwindConvectiveOperator()
{
    deallocateOperatorState();
    return;
} // ~INSStaggeredUpwindConvectiveOperator

void INSStaggeredUpwindConvectiveOperator::applyConvectiveOperator(const int U_idx, const int N_idx)
{
    IBAMR_TIMER_START(t_apply_convective_operator);

    if (!d_is_initialized)
    {
        TBOX_ERROR("INSStaggeredUpwindConvectiveOperator::applyConvectiveOperator():\n"
                   << "  operator must be initialized prior to call to applyConvectiveOperator\n");
    }
    TBOX_ASSERT(U_idx == d_u_idx);

    // Fill ghost cell values for all components.
    static const bool homogeneous_bc = false;
    typedef HierarchyGhostCellInterpolation::InterpolationTransactionComponent InterpolationTransactionComponent;
    std::vector<InterpolationTransactionComponent> transaction_comps(1);
    transaction_comps[0] = InterpolationTransactionComponent(d_U_scratch_idx,
                                                             U_idx,
                                                             "CONSTANT_REFINE",
                                                             false,
                                                             "CONSERVATIVE_COARSEN",
                                                             d_bdry_extrap_type,
                                                             false,
                                                             d_bc_coefs);
    d_hier_bdry_fill->resetTransactionComponents(transaction_comps);
    d_hier_bdry_fill->setHomogeneousBc(homogeneous_bc);
    StaggeredStokesPhysicalBoundaryHelper::setupBcCoefObjects(d_bc_coefs, NULL, d_U_scratch_idx, -1, homogeneous_bc);
    d_hier_bdry_fill->fillData(d_solution_time);
    StaggeredStokesPhysicalBoundaryHelper::resetBcCoefObjects(d_bc_coefs, NULL);
    //  d_bc_helper->enforceDivergenceFreeConditionAtBoundary(d_U_scratch_idx);
    d_hier_bdry_fill->resetTransactionComponents(d_transaction_comps);

    // Compute the convective derivative.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        auto level =d_hierarchy->getPatchLevel(ln);
        for (auto p = level->begin(); p != level->end(); ++p)
        {
            auto patch =*p;

            auto pgeom = BOOST_CAST<CartesianPatchGeometry>(patch->getPatchGeometry());
            const double* const dx = pgeom->getDx();

            const Box& patch_box = patch->getBox();
            const IntVector& patch_lower = patch_box.lower();
            const IntVector& patch_upper = patch_box.upper();

            boost::shared_ptr<SideData<double> > N_data = patch->getPatchData(N_idx);
            boost::shared_ptr<SideData<double> > U_data = patch->getPatchData(d_U_scratch_idx);

            const IntVector ghosts = IntVector::getOne(DIM);
            std::vector<Box> side_boxes(NDIM, Box(DIM));
            boost::array<boost::shared_ptr<FaceData<double> >, NDIM> U_adv_data;
            boost::array<boost::shared_ptr<FaceData<double> >, NDIM> U_half_data;
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                side_boxes[axis] = SideGeometry::toSideBox(patch_box, axis);
                U_adv_data[axis] = boost::make_shared<FaceData<double>>(side_boxes[axis], 1, ghosts);
                U_half_data[axis] = boost::make_shared<FaceData<double>>(side_boxes[axis], 1, ghosts);
            }
#if (NDIM == 2)
            NAVIER_STOKES_INTERP_COMPS_FC(patch_lower(0),
                                          patch_upper(0),
                                          patch_lower(1),
                                          patch_upper(1),
                                          U_data->getGhostCellWidth()(0),
                                          U_data->getGhostCellWidth()(1),
                                          U_data->getPointer(0),
                                          U_data->getPointer(1),
                                          side_boxes[0].lower(0),
                                          side_boxes[0].upper(0),
                                          side_boxes[0].lower(1),
                                          side_boxes[0].upper(1),
                                          U_adv_data[0]->getGhostCellWidth()(0),
                                          U_adv_data[0]->getGhostCellWidth()(1),
                                          U_adv_data[0]->getPointer(0),
                                          U_adv_data[0]->getPointer(1),
                                          side_boxes[1].lower(0),
                                          side_boxes[1].upper(0),
                                          side_boxes[1].lower(1),
                                          side_boxes[1].upper(1),
                                          U_adv_data[1]->getGhostCellWidth()(0),
                                          U_adv_data[1]->getGhostCellWidth()(1),
                                          U_adv_data[1]->getPointer(0),
                                          U_adv_data[1]->getPointer(1));
#endif
#if (NDIM == 3)
            NAVIER_STOKES_INTERP_COMPS_FC(patch_lower(0),
                                          patch_upper(0),
                                          patch_lower(1),
                                          patch_upper(1),
                                          patch_lower(2),
                                          patch_upper(2),
                                          U_data->getGhostCellWidth()(0),
                                          U_data->getGhostCellWidth()(1),
                                          U_data->getGhostCellWidth()(2),
                                          U_data->getPointer(0),
                                          U_data->getPointer(1),
                                          U_data->getPointer(2),
                                          side_boxes[0].lower(0),
                                          side_boxes[0].upper(0),
                                          side_boxes[0].lower(1),
                                          side_boxes[0].upper(1),
                                          side_boxes[0].lower(2),
                                          side_boxes[0].upper(2),
                                          U_adv_data[0]->getGhostCellWidth()(0),
                                          U_adv_data[0]->getGhostCellWidth()(1),
                                          U_adv_data[0]->getGhostCellWidth()(2),
                                          U_adv_data[0]->getPointer(0),
                                          U_adv_data[0]->getPointer(1),
                                          U_adv_data[0]->getPointer(2),
                                          side_boxes[1].lower(0),
                                          side_boxes[1].upper(0),
                                          side_boxes[1].lower(1),
                                          side_boxes[1].upper(1),
                                          side_boxes[1].lower(2),
                                          side_boxes[1].upper(2),
                                          U_adv_data[1]->getGhostCellWidth()(0),
                                          U_adv_data[1]->getGhostCellWidth()(1),
                                          U_adv_data[1]->getGhostCellWidth()(2),
                                          U_adv_data[1]->getPointer(0),
                                          U_adv_data[1]->getPointer(1),
                                          U_adv_data[1]->getPointer(2),
                                          side_boxes[2].lower(0),
                                          side_boxes[2].upper(0),
                                          side_boxes[2].lower(1),
                                          side_boxes[2].upper(1),
                                          side_boxes[2].lower(2),
                                          side_boxes[2].upper(2),
                                          U_adv_data[2]->getGhostCellWidth()(0),
                                          U_adv_data[2]->getGhostCellWidth()(1),
                                          U_adv_data[2]->getGhostCellWidth()(2),
                                          U_adv_data[2]->getPointer(0),
                                          U_adv_data[2]->getPointer(1),
                                          U_adv_data[2]->getPointer(2));
#endif
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                const ArrayData<double>& U_array_data = U_data->getArrayData(axis);
                for (unsigned int d = 0; d < NDIM; ++d)
                {
                    for (FaceIterator ic(side_boxes[axis], d); ic; ic++)
                    {
                        const FaceIndex& i = ic();
                        const double u_ADV = (*U_adv_data[axis])(i);
                        const double U_lower = U_array_data(i.toCell(0), 0);
                        const double U_upper = U_array_data(i.toCell(1), 0);
                        (*U_half_data[axis])(i) =
                            (u_ADV > 1.0e-8) ? U_lower : (u_ADV < 1.0e-8) ? U_upper : 0.5 * (U_lower + U_upper);
                    }
                }
            }
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                switch (d_difference_form)
                {
                case CONSERVATIVE:
#if (NDIM == 2)
                    CONVECT_DERIVATIVE_FC(dx,
                                          side_boxes[axis].lower(0),
                                          side_boxes[axis].upper(0),
                                          side_boxes[axis].lower(1),
                                          side_boxes[axis].upper(1),
                                          U_adv_data[axis]->getGhostCellWidth()(0),
                                          U_adv_data[axis]->getGhostCellWidth()(1),
                                          U_half_data[axis]->getGhostCellWidth()(0),
                                          U_half_data[axis]->getGhostCellWidth()(1),
                                          U_adv_data[axis]->getPointer(0),
                                          U_adv_data[axis]->getPointer(1),
                                          U_half_data[axis]->getPointer(0),
                                          U_half_data[axis]->getPointer(1),
                                          N_data->getGhostCellWidth()(0),
                                          N_data->getGhostCellWidth()(1),
                                          N_data->getPointer(axis));
#endif
#if (NDIM == 3)
                    CONVECT_DERIVATIVE_FC(dx,
                                          side_boxes[axis].lower(0),
                                          side_boxes[axis].upper(0),
                                          side_boxes[axis].lower(1),
                                          side_boxes[axis].upper(1),
                                          side_boxes[axis].lower(2),
                                          side_boxes[axis].upper(2),
                                          U_adv_data[axis]->getGhostCellWidth()(0),
                                          U_adv_data[axis]->getGhostCellWidth()(1),
                                          U_adv_data[axis]->getGhostCellWidth()(2),
                                          U_half_data[axis]->getGhostCellWidth()(0),
                                          U_half_data[axis]->getGhostCellWidth()(1),
                                          U_half_data[axis]->getGhostCellWidth()(2),
                                          U_adv_data[axis]->getPointer(0),
                                          U_adv_data[axis]->getPointer(1),
                                          U_adv_data[axis]->getPointer(2),
                                          U_half_data[axis]->getPointer(0),
                                          U_half_data[axis]->getPointer(1),
                                          U_half_data[axis]->getPointer(2),
                                          N_data->getGhostCellWidth()(0),
                                          N_data->getGhostCellWidth()(1),
                                          N_data->getGhostCellWidth()(2),
                                          N_data->getPointer(axis));
#endif
                    break;
                case ADVECTIVE:
#if (NDIM == 2)
                    ADVECT_DERIVATIVE_FC(dx,
                                         side_boxes[axis].lower(0),
                                         side_boxes[axis].upper(0),
                                         side_boxes[axis].lower(1),
                                         side_boxes[axis].upper(1),
                                         U_adv_data[axis]->getGhostCellWidth()(0),
                                         U_adv_data[axis]->getGhostCellWidth()(1),
                                         U_half_data[axis]->getGhostCellWidth()(0),
                                         U_half_data[axis]->getGhostCellWidth()(1),
                                         U_adv_data[axis]->getPointer(0),
                                         U_adv_data[axis]->getPointer(1),
                                         U_half_data[axis]->getPointer(0),
                                         U_half_data[axis]->getPointer(1),
                                         N_data->getGhostCellWidth()(0),
                                         N_data->getGhostCellWidth()(1),
                                         N_data->getPointer(axis));
#endif
#if (NDIM == 3)
                    ADVECT_DERIVATIVE_FC(dx,
                                         side_boxes[axis].lower(0),
                                         side_boxes[axis].upper(0),
                                         side_boxes[axis].lower(1),
                                         side_boxes[axis].upper(1),
                                         side_boxes[axis].lower(2),
                                         side_boxes[axis].upper(2),
                                         U_adv_data[axis]->getGhostCellWidth()(0),
                                         U_adv_data[axis]->getGhostCellWidth()(1),
                                         U_adv_data[axis]->getGhostCellWidth()(2),
                                         U_half_data[axis]->getGhostCellWidth()(0),
                                         U_half_data[axis]->getGhostCellWidth()(1),
                                         U_half_data[axis]->getGhostCellWidth()(2),
                                         U_adv_data[axis]->getPointer(0),
                                         U_adv_data[axis]->getPointer(1),
                                         U_adv_data[axis]->getPointer(2),
                                         U_half_data[axis]->getPointer(0),
                                         U_half_data[axis]->getPointer(1),
                                         U_half_data[axis]->getPointer(2),
                                         N_data->getGhostCellWidth()(0),
                                         N_data->getGhostCellWidth()(1),
                                         N_data->getGhostCellWidth()(2),
                                         N_data->getPointer(axis));
#endif
                    break;
                case SKEW_SYMMETRIC:
#if (NDIM == 2)
                    SKEW_SYM_DERIVATIVE_FC(dx,
                                           side_boxes[axis].lower(0),
                                           side_boxes[axis].upper(0),
                                           side_boxes[axis].lower(1),
                                           side_boxes[axis].upper(1),
                                           U_adv_data[axis]->getGhostCellWidth()(0),
                                           U_adv_data[axis]->getGhostCellWidth()(1),
                                           U_half_data[axis]->getGhostCellWidth()(0),
                                           U_half_data[axis]->getGhostCellWidth()(1),
                                           U_adv_data[axis]->getPointer(0),
                                           U_adv_data[axis]->getPointer(1),
                                           U_half_data[axis]->getPointer(0),
                                           U_half_data[axis]->getPointer(1),
                                           N_data->getGhostCellWidth()(0),
                                           N_data->getGhostCellWidth()(1),
                                           N_data->getPointer(axis));
#endif
#if (NDIM == 3)
                    SKEW_SYM_DERIVATIVE_FC(dx,
                                           side_boxes[axis].lower(0),
                                           side_boxes[axis].upper(0),
                                           side_boxes[axis].lower(1),
                                           side_boxes[axis].upper(1),
                                           side_boxes[axis].lower(2),
                                           side_boxes[axis].upper(2),
                                           U_adv_data[axis]->getGhostCellWidth()(0),
                                           U_adv_data[axis]->getGhostCellWidth()(1),
                                           U_adv_data[axis]->getGhostCellWidth()(2),
                                           U_half_data[axis]->getGhostCellWidth()(0),
                                           U_half_data[axis]->getGhostCellWidth()(1),
                                           U_half_data[axis]->getGhostCellWidth()(2),
                                           U_adv_data[axis]->getPointer(0),
                                           U_adv_data[axis]->getPointer(1),
                                           U_adv_data[axis]->getPointer(2),
                                           U_half_data[axis]->getPointer(0),
                                           U_half_data[axis]->getPointer(1),
                                           U_half_data[axis]->getPointer(2),
                                           N_data->getGhostCellWidth()(0),
                                           N_data->getGhostCellWidth()(1),
                                           N_data->getGhostCellWidth()(2),
                                           N_data->getPointer(axis));
#endif
                    break;
                default:
                    TBOX_ERROR("INSStaggeredUpwindConvectiveOperator::applyConvectiveOperator():\n"
                               << "  unsupported differencing form: "
                               << enum_to_string<ConvectiveDifferencingType>(d_difference_form) << " \n"
                               << "  valid choices are: ADVECTIVE, CONSERVATIVE, SKEW_SYMMETRIC\n");
                }
            }
        }
    }

    IBAMR_TIMER_STOP(t_apply_convective_operator);
    return;
} // applyConvectiveOperator

void INSStaggeredUpwindConvectiveOperator::initializeOperatorState(const SAMRAIVectorReal<double>& in,
                                                                   const SAMRAIVectorReal<double>& out)
{
    IBAMR_TIMER_START(t_initialize_operator_state);

    if (d_is_initialized) deallocateOperatorState();

    // Get the hierarchy configuration.
    d_hierarchy = in.getPatchHierarchy();
    d_coarsest_ln = in.getCoarsestLevelNumber();
    d_finest_ln = in.getFinestLevelNumber();
    TBOX_ASSERT(d_hierarchy == out.getPatchHierarchy());
    TBOX_ASSERT(d_coarsest_ln == out.getCoarsestLevelNumber());
    TBOX_ASSERT(d_finest_ln == out.getFinestLevelNumber());

    // Setup the interpolation transaction information.
    typedef HierarchyGhostCellInterpolation::InterpolationTransactionComponent InterpolationTransactionComponent;
    d_transaction_comps.resize(1);
    d_transaction_comps[0] = InterpolationTransactionComponent(d_U_scratch_idx,
                                                               in.getComponentDescriptorIndex(0),
                                                               "CONSTANT_REFINE",
                                                               false,
                                                               "CONSERVATIVE_COARSEN",
                                                               d_bdry_extrap_type,
                                                               false,
                                                               d_bc_coefs);

    // Initialize the interpolation operators.
    d_hier_bdry_fill = boost::make_shared<HierarchyGhostCellInterpolation>();
    d_hier_bdry_fill->initializeOperatorState(d_transaction_comps, d_hierarchy);

    // Initialize the BC helper.
    d_bc_helper = boost::make_shared<StaggeredStokesPhysicalBoundaryHelper>();
    d_bc_helper->cacheBcCoefData(d_bc_coefs, d_solution_time, d_hierarchy);

    // Allocate scratch data.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        auto level =d_hierarchy->getPatchLevel(ln);
        if (!level->checkAllocated(d_U_scratch_idx))
        {
            level->allocatePatchData(d_U_scratch_idx);
        }
    }
    d_is_initialized = true;

    IBAMR_TIMER_STOP(t_initialize_operator_state);
    return;
} // initializeOperatorState

void INSStaggeredUpwindConvectiveOperator::deallocateOperatorState()
{
    if (!d_is_initialized) return;

    IBAMR_TIMER_START(t_deallocate_operator_state);

    // Deallocate scratch data.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        auto level =d_hierarchy->getPatchLevel(ln);
        if (level->checkAllocated(d_U_scratch_idx))
        {
            level->deallocatePatchData(d_U_scratch_idx);
        }
    }

    // Deallocate the communications operators and BC helpers.
    d_hier_bdry_fill.reset();
    d_bc_helper.reset();

    d_is_initialized = false;

    IBAMR_TIMER_STOP(t_deallocate_operator_state);
    return;
} // deallocateOperatorState

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // namespace IBAMR

//////////////////////////////////////////////////////////////////////////////
