/// \file   communicator.hpp
///
/// \brief
///
/// \authors    Maarten P. Scholl
/// \date       2018-04-13
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
#ifndef ESL_SIMULATION_COMMUNICATOR_HPP
#define ESL_SIMULATION_COMMUNICATOR_HPP


#include <functional>
#include <map>
#include <memory>
#include <random>

#include <boost/serialization/nvp.hpp>

#include <esl/interaction/header.hpp>


namespace esl::interaction {

    template<typename message_type_, message_code type_code_>
    class message;

    ///
    /// \brief  A communicator can send and receive messages using the provided
    ///         inbox and outbox. It provides a structured way to deal with
    ///         messages through prioritized callbacks.
    ///
    class communicator
    {
    public:
        ///
        /// \brief  A messsage is any class deriving from `header`, which
        ///         contains the minimum amount of information for delivery.
        ///
        typedef std::shared_ptr<header> message_t;

        ///
        /// \brief  A function is any function taking a message and returning
        ///         the time at which the next event is expected.
        ///
        typedef std::function<simulation::time_point(message_t,
                                                     simulation::time_step)>
            callback_t;

        ///
        /// \brief  The inbox stores messages by delivery time.
        ///
        typedef std::multimap<simulation::time_point, message_t> inbox_t;

        ///
        typedef std::vector<message_t> outbox_t;

        ///
        typedef std::int8_t priority_t;

        ///
        inbox_t inbox;

        ///
        outbox_t outbox;

    protected:
        friend class boost::serialization::access;
        ///
        /// \brief  when true, modifying the callbacks data structure results
        ///         in an std::logic_error being throw.
        ///
        /// \details    Because callbacks can't be serialized, we only allow
        ///             setting up callbacks during initialization.
        ///
        bool locked_;

        std::map<message_code, std::multimap<priority_t, callback_t>>
            callbacks_;

    public:
        enum scheduling
        {
            in_order,
            random
        };

    protected:
        scheduling schedule_;

    public:
        ///
        /// \brief  By default, the communicator is unlocked meaning callbacks
        /// can be locked and unlocked
        ///
        communicator(scheduling schedule = random);

        virtual ~communicator() = default;


        ///
        /// \brief  Create a message and queue it for sending.
        ///
        /// \tparam message_type_   The type of message
        /// \tparam recipient_t_    The recipient type, if specific covariant
        ///                         types are required
        /// \tparam constructor_arguments_  The message
        ///                                 constructor's arguments
        ///
        /// \param recipient        The recipient
        /// \param delivery         The time_point at which this message becomes
        ///                         available to the recipient
        /// \param arguments        arguments to the
        ///                         message's constructor
        ///
        /// \return                 shared_ptr to the
        /// message
        template<typename message_type_, typename recipient_t_,
                 typename... constructor_arguments_>
        std::shared_ptr<message_type_>
        create_message(const identity<recipient_t_> &recipient,
                       simulation::time_point delivery,
                       constructor_arguments_... arguments)
        {
            auto result_       = std::make_shared<message_type_>(arguments...);
            result_->recipient = recipient;
            result_->received  = delivery;

            this->outbox().push_back(result_);

            return result_;
        }

        ///
        /// \brief  Registers `callback` to be called when receiving a message
        ///         of type `derived_message_t_`.
        ///
        /// \tparam derived_message_t_
        /// \param callback
        /// \param priority
        template<typename derived_message_t_>
        void register_callback(
            std::function<simulation::time_point(
                std::shared_ptr<derived_message_t_>, simulation::time_step)>
                callback,
            priority_t priority = 0)
        {
            /// the specified message must inherit `header`
            static_assert(std::is_base_of<header, derived_message_t_>::value);

            if(locked_) {
                throw std::logic_error("communicator callback can only be "
                                       "added at initialisation");
            }

            constexpr auto type_code_ = derived_message_t_::code;
            auto iterator_            = callbacks_.find(type_code_);

            if(callbacks_.end() == iterator_) {
                callbacks_.insert({type_code_, {}});
                iterator_ = callbacks_.find(type_code_);
            }

            auto function_ = [callback](message_t m,
                                        simulation::time_step step) {
                auto converted_ =
                    std::dynamic_pointer_cast<derived_message_t_>(m);

                return callback(converted_, step);
            };

            iterator_->second.insert(std::make_pair(priority, function_));
        }

        ///
        /// \brief  Used to lock the callbacks data structure
        ///
        inline void lock_callbacks()
        {
            locked_ = true;
        }

        [[nodiscard]] inline const decltype(callbacks_) &callbacks() const
        {
            return callbacks_;
        }

        ///
        /// \brief  Calls all callbacks registered to receive the given message.
        ///
        /// \param message
        /// \return
        simulation::time_point
        process_message(message_t message, simulation::time_step step) const;

        ///
        /// \brief  Calls all callbacks registered with messages in the inbox.
        ///         Applies the communicator::scheduling method.
        ///
        /// \param start
        simulation::time_point
        process_messages(const simulation::time_step &step,
                         std::seed_seq &seed);

        ///
        /// \tparam archive_t
        /// \param archive
        /// \param file_version
        template<class archive_t>
        void serialize(archive_t &archive, const unsigned int version)
        {
            (void)version;
            archive &BOOST_SERIALIZATION_NVP(inbox);
            archive &BOOST_SERIALIZATION_NVP(outbox);
            archive &BOOST_SERIALIZATION_NVP(locked_);
        }
    };
}  // namespace esl::interaction


#ifdef WITH_MPI
#include <boost/mpi.hpp>

namespace boost { namespace mpi {
    ///
    /// \brief serialization is non-trivial
    ///
    template<>
    struct is_mpi_datatype<esl::interaction::communicator> : public mpl::false_
    {};
}}      // namespace boost::mpi
#endif  // WITH_MPI


#endif  // ESL_SIMULATION_COMMUNICATOR_HPP