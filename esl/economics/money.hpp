/// \file   money.hpp
///
/// \brief
///
/// \authors    Maarten P. Scholl
/// \date       2018-04-28
/// \copyright  Copyright 2017-2019 The Institute for New Economic Thinking,
///             Oxford Martin School, University of Oxford
///
///             Licensed under the Apache License, Version 2.0 (the "License");
///             you may not use this file except in compliance with the License.
///             You may obtain a copy of the License at
///
///                 http://www.apache.org/licenses/LICENSE-2.0
///
///             Unless required by applicable law or agreed to in writing,
///             software distributed under the License is distributed on an "AS
///             IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
///             express or implied. See the License for the specific language
///             governing permissions and limitations under the License.
///
///             You may obtain instructions to fulfill the attribution
///             requirements in CITATION.cff
///
#ifndef ESL_MONEY_HPP
#define ESL_MONEY_HPP

#include <esl/economics/asset.hpp>
#include <esl/economics/fungibility.hpp>


namespace esl::economics {
    ///
    /// \brief  Money is the idea that an amount of currency is a fungible asset
    ///         and it further specializes into intangible (cash) and tangible
    ///         (physical money < commodity money) forms of assets.
    ///
    struct money
    : public asset
    , public fungible
    {
        currency denomination;

        explicit money(currency denomination)
        : asset(), fungible(), denomination(denomination)
        {}

        virtual ~money() = default;

        virtual std::string name() const  // C++20 constexpr
        {
            std::stringstream stream_;
            stream_ << denomination << " money";
            return stream_.str();
        }

        constexpr inline bool operator==(const money &m) const
        {
            return this->denomination == m.denomination;
        }

        constexpr inline bool operator!=(const money &m) const
        {
            return !(*this == m);
        }
    };

}  // namespace esl::economics

#endif  // ESL_MONEY_HPP
