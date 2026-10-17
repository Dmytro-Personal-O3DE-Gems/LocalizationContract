
#pragma once

#include <AzCore/EBus/EBus.h>

namespace LocalizationContract
{
    //! Notifications sent by the active localization implementation.
    //!
    //! The event carries no data on purpose: listeners request the texts they show again
    //! through LocalizationContractRequests (LocalizationContractBus.h).
    class LocalizationContractNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        virtual ~LocalizationContractNotifications() = default;

        //! Sent when localized texts may have changed: the current language changed
        //! or the texts were reloaded. Listeners should request their texts again.
        //! Has an empty default body, so adding new events later does not break existing listeners.
        virtual void OnLocalizedStringsChanged() {}
    };

    using LocalizationContractNotificationBus = AZ::EBus<LocalizationContractNotifications>;

} // namespace LocalizationContract
