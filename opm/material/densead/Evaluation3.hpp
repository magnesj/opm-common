// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/
/*!
 * \file
 *
 * \brief This file specializes the dense-AD Evaluation class for 3 derivatives.
 *
 * \attention THIS FILE GETS AUTOMATICALLY GENERATED BY THE "genEvalSpecializations.py"
 *            SCRIPT. DO NOT EDIT IT MANUALLY!
 */
#ifndef OPM_DENSEAD_EVALUATION3_HPP
#define OPM_DENSEAD_EVALUATION3_HPP

#ifndef NDEBUG
#include <opm/material/common/Valgrind.hpp>
#endif

#include <array>
#include <cassert>
#include <iosfwd>
#include <stdexcept>

#include <opm/common/utility/gpuDecorators.hpp>

namespace Opm {
namespace DenseAd {

template <class ValueT>
class Evaluation<ValueT, 3>
{
public:
    //! the template argument which specifies the number of
    //! derivatives (-1 == "DynamicSize" means runtime determined)
    static const int numVars = 3;

    //! field type
    typedef ValueT ValueType;

    //! number of derivatives
    OPM_HOST_DEVICE constexpr int size() const
    { return 3; };

protected:
    //! length of internal data vector
    OPM_HOST_DEVICE constexpr int length_() const
    { return size() + 1; }


    //! position index for value
    OPM_HOST_DEVICE constexpr int valuepos_() const
    { return 0; }
    //! start index for derivatives
    OPM_HOST_DEVICE constexpr int dstart_() const
    { return 1; }
    //! end+1 index for derivatives
    OPM_HOST_DEVICE constexpr int dend_() const
    { return length_(); }

    //! instruct valgrind to check that the value and all derivatives of the
    //! Evaluation object are well-defined.
    OPM_HOST_DEVICE void checkDefined_() const
    {
#ifndef NDEBUG
#if !OPM_IS_INSIDE_DEVICE_FUNCTION // Valgrind not currently supported on GPUs
       for (const auto& v: data_)
           Valgrind::CheckDefined(v);
#endif
#endif
    }

public:
    //! default constructor
    OPM_HOST_DEVICE Evaluation() : data_()
    {}

    //! copy other function evaluation
    Evaluation(const Evaluation& other) = default;


    // create an evaluation which represents a constant function
    //
    // i.e., f(x) = c. this implies an evaluation with the given value and all
    // derivatives being zero.
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation(const RhsValueType& c)
    {
        setValue(c);
        clearDerivatives();

        checkDefined_();
    }

    // create an evaluation which represents a constant function
    //
    // i.e., f(x) = c. this implies an evaluation with the given value and all
    // derivatives being zero.
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation(const RhsValueType& c, int varPos)
    {
        // The variable position must be in represented by the given variable descriptor
        assert(0 <= varPos && varPos < size());

        setValue( c );
        clearDerivatives();

        data_[varPos + dstart_()] = 1.0;

        checkDefined_();
    }

    // set all derivatives to zero
    OPM_HOST_DEVICE void clearDerivatives()
    {
        data_[1] = 0.0;
        data_[2] = 0.0;
        data_[3] = 0.0;
    }

    // create an uninitialized Evaluation object that is compatible with the
    // argument, but not initialized
    //
    // This basically boils down to the copy constructor without copying
    // anything. If the number of derivatives is known at compile time, this
    // is equivalent to creating an uninitialized object using the default
    // constructor, while for dynamic evaluations, it creates an Evaluation
    // object which exhibits the same number of derivatives as the argument.
    OPM_HOST_DEVICE static Evaluation createBlank(const Evaluation&)
    { return Evaluation(); }

    // create an Evaluation with value and all the derivatives to be zero
    OPM_HOST_DEVICE static Evaluation createConstantZero(const Evaluation&)
    { return Evaluation(0.); }

    // create an Evaluation with value to be one and all the derivatives to be zero
    OPM_HOST_DEVICE static Evaluation createConstantOne(const Evaluation&)
    { return Evaluation(1.); }

    // create a function evaluation for a "naked" depending variable (i.e., f(x) = x)
    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createVariable(const RhsValueType& value, int varPos)
    {
        // copy function value and set all derivatives to 0, except for the variable
        // which is represented by the value (which is set to 1.0)
        return Evaluation(value, varPos);
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createVariable(int nVars, const RhsValueType& value, int varPos)
    {
        if (nVars != 3)
            throw std::logic_error("This statically-sized evaluation can only represent objects"
                                   " with 3 derivatives");

        // copy function value and set all derivatives to 0, except for the variable
        // which is represented by the value (which is set to 1.0)
        return Evaluation(nVars, value, varPos);
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createVariable(const Evaluation&, const RhsValueType& value, int varPos)
    {
        // copy function value and set all derivatives to 0, except for the variable
        // which is represented by the value (which is set to 1.0)
        return Evaluation(value, varPos);
    }


    // "evaluate" a constant function (i.e. a function that does not depend on the set of
    // relevant variables, f(x) = c).
    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createConstant(int nVars, const RhsValueType& value)
    {
        if (nVars != 3)
            throw std::logic_error("This statically-sized evaluation can only represent objects"
                                   " with 3 derivatives");
        return Evaluation(value);
    }

    // "evaluate" a constant function (i.e. a function that does not depend on the set of
    // relevant variables, f(x) = c).
    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createConstant(const RhsValueType& value)
    {
        return Evaluation(value);
    }

    // "evaluate" a constant function (i.e. a function that does not depend on the set of
    // relevant variables, f(x) = c).
    template <class RhsValueType>
    OPM_HOST_DEVICE static Evaluation createConstant(const Evaluation&, const RhsValueType& value)
    {
        return Evaluation(value);
    }

    // copy all derivatives from other
    OPM_HOST_DEVICE void copyDerivatives(const Evaluation& other)
    {
        assert(size() == other.size());

        data_[1] = other.data_[1];
        data_[2] = other.data_[2];
        data_[3] = other.data_[3];
    }


    // add value and derivatives from other to this values and derivatives
    OPM_HOST_DEVICE Evaluation& operator+=(const Evaluation& other)
    {
        assert(size() == other.size());

        data_[0] += other.data_[0];
        data_[1] += other.data_[1];
        data_[2] += other.data_[2];
        data_[3] += other.data_[3];

        return *this;
    }

    // add value from other to this values
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation& operator+=(const RhsValueType& other)
    {
        // value is added, derivatives stay the same
        data_[valuepos_()] += other;

        return *this;
    }

    // subtract other's value and derivatives from this values
    OPM_HOST_DEVICE Evaluation& operator-=(const Evaluation& other)
    {
        assert(size() == other.size());

        data_[0] -= other.data_[0];
        data_[1] -= other.data_[1];
        data_[2] -= other.data_[2];
        data_[3] -= other.data_[3];

        return *this;
    }

    // subtract other's value from this values
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation& operator-=(const RhsValueType& other)
    {
        // for constants, values are subtracted, derivatives stay the same
        data_[valuepos_()] -= other;

        return *this;
    }

    // multiply values and apply chain rule to derivatives: (u*v)' = (v'u + u'v)
    OPM_HOST_DEVICE Evaluation& operator*=(const Evaluation& other)
    {
        assert(size() == other.size());

        // while the values are multiplied, the derivatives follow the product rule,
        // i.e., (u*v)' = (v'u + u'v).
        const ValueType u = this->value();
        const ValueType v = other.value();

        // value
        data_[valuepos_()] *= v ;

        //  derivatives
        data_[1] = data_[1] * v + other.data_[1] * u;
        data_[2] = data_[2] * v + other.data_[2] * u;
        data_[3] = data_[3] * v + other.data_[3] * u;

        return *this;
    }

    // m(c*u)' = c*u'
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation& operator*=(const RhsValueType& other)
    {
        data_[0] *= other;
        data_[1] *= other;
        data_[2] *= other;
        data_[3] *= other;

        return *this;
    }

    // m(u*v)' = (vu' - uv')/v^2
    OPM_HOST_DEVICE Evaluation& operator/=(const Evaluation& other)
    {
        assert(size() == other.size());

        // values are divided, derivatives follow the rule for division, i.e., (u/v)' = (v'u -
        // u'v)/v^2.
        ValueType& u = data_[valuepos_()];
        const ValueType& v = other.value();
        data_[1] = (v*data_[1] - u*other.data_[1])/(v*v);
        data_[2] = (v*data_[2] - u*other.data_[2])/(v*v);
        data_[3] = (v*data_[3] - u*other.data_[3])/(v*v);
        u /= v;

        return *this;
    }

    // divide value and derivatives by value of other
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation& operator/=(const RhsValueType& other)
    {
        const ValueType tmp = 1.0/other;

        data_[0] *= tmp;
        data_[1] *= tmp;
        data_[2] *= tmp;
        data_[3] *= tmp;

        return *this;
    }

    // add two evaluation objects
    OPM_HOST_DEVICE Evaluation operator+(const Evaluation& other) const
    {
        assert(size() == other.size());

        Evaluation result(*this);

        result += other;

        return result;
    }

    // add constant to this object
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation operator+(const RhsValueType& other) const
    {
        Evaluation result(*this);

        result += other;

        return result;
    }

    // subtract two evaluation objects
    OPM_HOST_DEVICE Evaluation operator-(const Evaluation& other) const
    {
        assert(size() == other.size());

        Evaluation result(*this);

        result -= other;

        return result;
    }

    // subtract constant from evaluation object
    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation operator-(const RhsValueType& other) const
    {
        Evaluation result(*this);

        result -= other;

        return result;
    }

    // negation (unary minus) operator
    OPM_HOST_DEVICE Evaluation operator-() const
    {
        Evaluation result;

        // set value and derivatives to negative
        result.data_[0] = - data_[0];
        result.data_[1] = - data_[1];
        result.data_[2] = - data_[2];
        result.data_[3] = - data_[3];

        return result;
    }

    OPM_HOST_DEVICE Evaluation operator*(const Evaluation& other) const
    {
        assert(size() == other.size());

        Evaluation result(*this);

        result *= other;

        return result;
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation operator*(const RhsValueType& other) const
    {
        Evaluation result(*this);

        result *= other;

        return result;
    }

    OPM_HOST_DEVICE Evaluation operator/(const Evaluation& other) const
    {
        assert(size() == other.size());

        Evaluation result(*this);

        result /= other;

        return result;
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation operator/(const RhsValueType& other) const
    {
        Evaluation result(*this);

        result /= other;

        return result;
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE Evaluation& operator=(const RhsValueType& other)
    {
        setValue( other );
        clearDerivatives();

        return *this;
    }

    // copy assignment from evaluation
    Evaluation& operator=(const Evaluation& other) = default;

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator==(const RhsValueType& other) const
    { return value() == other; }

    OPM_HOST_DEVICE bool operator==(const Evaluation& other) const
    {
        assert(size() == other.size());

        for (int idx = 0; idx < length_(); ++idx) {
            if (data_[idx] != other.data_[idx]) {
                return false;
            }
        }
        return true;
    }

    OPM_HOST_DEVICE bool operator!=(const Evaluation& other) const
    { return !operator==(other); }

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator!=(const RhsValueType& other) const
    { return !operator==(other); }

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator>(RhsValueType other) const
    { return value() > other; }

    OPM_HOST_DEVICE bool operator>(const Evaluation& other) const
    {
        assert(size() == other.size());

        return value() > other.value();
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator<(RhsValueType other) const
    { return value() < other; }

    OPM_HOST_DEVICE bool operator<(const Evaluation& other) const
    {
        assert(size() == other.size());

        return value() < other.value();
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator>=(RhsValueType other) const
    { return value() >= other; }

    OPM_HOST_DEVICE bool operator>=(const Evaluation& other) const
    {
        assert(size() == other.size());

        return value() >= other.value();
    }

    template <class RhsValueType>
    OPM_HOST_DEVICE bool operator<=(RhsValueType other) const
    { return value() <= other; }

    OPM_HOST_DEVICE bool operator<=(const Evaluation& other) const
    {
        assert(size() == other.size());

        return value() <= other.value();
    }

    // return value of variable
    OPM_HOST_DEVICE const ValueType& value() const
    { return data_[valuepos_()]; }

    // set value of variable
    template <class RhsValueType>
    OPM_HOST_DEVICE void setValue(const RhsValueType& val)
    { data_[valuepos_()] = val; }

    // return varIdx'th derivative
    OPM_HOST_DEVICE const ValueType& derivative(int varIdx) const
    {
        assert(0 <= varIdx && varIdx < size());

        return data_[dstart_() + varIdx];
    }

    // set derivative at position varIdx
    OPM_HOST_DEVICE void setDerivative(int varIdx, const ValueType& derVal)
    {
        assert(0 <= varIdx && varIdx < size());

        data_[dstart_() + varIdx] = derVal;
    }

    template<class Serializer>
    OPM_HOST_DEVICE void serializeOp(Serializer& serializer)
    {
        serializer(data_);
    }

private:
    std::array<ValueT, 4> data_;
};

} // namespace DenseAd
} // namespace Opm

#endif // OPM_DENSEAD_EVALUATION3_HPP
