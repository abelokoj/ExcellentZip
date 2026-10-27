#pragma once

#include "AddressBarTemplate.g.h"

namespace winrt::ExcellentZip::Modern::implementation
{
    struct AddressBarTemplate : AddressBarTemplateT<AddressBarTemplate>
    {
        AddressBarTemplate()
        {

        }
    };
}

namespace winrt::ExcellentZip::Modern::factory_implementation
{
    struct AddressBarTemplate : AddressBarTemplateT<
        AddressBarTemplate,
        implementation::AddressBarTemplate>
    {
    };
}
