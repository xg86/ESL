/// \file   producer.hpp
///
/// \brief
///
/// \authors    Maarten P. Scholl
/// \date       2019-09-26
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
#ifndef ESL_DATA_PRODUCER_HPP
#define ESL_DATA_PRODUCER_HPP

#include <memory>

#include <esl/data/output.hpp>

namespace esl::data {

 struct producer
    {
        template<typename... variable_types_>
        std::shared_ptr<output<variable_types_...>>
        create_output(const std::string &name)
        {
            auto output_ = std::make_shared<output<variable_types_...>>(name);
            outputs.insert(
                {name, std::static_pointer_cast<output_base>(output_)});
            return output_;
        }

        ///
        /// \brief  The observables in (this shard of -) the simulation.
        ///
        std::unordered_map<std::string, std::shared_ptr<output_base>> outputs;
    };
}

#endif  // ESL_DATA_PRODUCER_HPP
