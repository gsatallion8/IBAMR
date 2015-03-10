// Filename: QInit.h
// Created on 19 Mar 2004 by Boyce Griffith
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

#ifndef included_QInit
#define included_QInit

/////////////////////////////// INCLUDES /////////////////////////////////////

// IBTK INCLUDES
#include <ibtk/CartGridFunction.h>

// SAMRAI INCLUDES
#include <CartesianGridGeometry.h>

// EIGEN INCLUDES
#include <Eigen/Dense>
typedef Eigen::Matrix<double, 3, 1> VectorNd;

// C++ namespace delcarations
#include <ibamr/app_namespaces.h>

/////////////////////////////// CLASS DEFINITION /////////////////////////////

/*!
 * \brief Method to initialize the value of the advected scalar Q.
 */
class QInit : public CartGridFunction
{
public:
    /*!
     * \brief Constructor.
     */
    QInit(const string& object_name, boost::shared_ptr<GridGeometry > grid_geom, boost::shared_ptr<Database> input_db);

    /*!
     * \brief Destructor.
     */
    ~QInit();

    /*!
     * Indicates whether the concrete CartGridFunction object is time dependent.
     */
    bool isTimeDependent() const
    {
        return true;
    }

    /*!
     * Set the data on the patch interior to the exact answer.
     */
    void setDataOnPatch(int data_idx,
                        boost::shared_ptr<Variable > var,
                        boost::shared_ptr<Patch > patch,
                        double data_time,
                        bool initial_time = false,
                        boost::shared_ptr<PatchLevel > level = NULL);

protected:
private:
    /*!
     * \brief Default constructor.
     *
     * \note This constructor is not implemented and should not be used.
     */
    QInit();

    /*!
     * \brief Copy constructor.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param from The value to copy to this object.
     */
    QInit(const QInit& from);

    /*!
     * \brief Assignment operator.
     *
     * \note This operator is not implemented and should not be used.
     *
     * \param that The value to assign to this object.
     *
     * \return A reference to this object.
     */
    QInit& operator=(const QInit& that);

    /*!
     * Read input values, indicated above, from given database.
     */
    void getFromInput(boost::shared_ptr<Database> db);

    /*
     * The object name is used as a handle to databases stored in restart files
     * and for error reporting purposes.
     */
    string d_object_name;

    /*
     * The grid geometry.
     */
    boost::shared_ptr<CartesianGridGeometry > d_grid_geom;

    /*
     * The center of the initial data.
     */
    VectorNd d_X;

    /*
     * The initialization type.
     */
    string d_init_type;

    /*
     * Parameters for Gaussian initial conditions.
     */
    double d_gaussian_kappa;

    /*
     * Parameters for the Zalesak slotted cylinder.
     */
    double d_zalesak_r;
    double d_zalesak_slot_w;
    double d_zalesak_slot_l;
};

/////////////////////////////// INLINE ///////////////////////////////////////

//#include "QInit.I"

//////////////////////////////////////////////////////////////////////////////

#endif //#ifndef included_QInit
