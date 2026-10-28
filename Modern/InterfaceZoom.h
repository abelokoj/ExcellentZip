#pragma once
#include "../UI_Modern/ExcellentZip.Zoom.h"
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

namespace ExcellentZipZoom
{
    inline winrt::Windows::UI::Xaml::FrameworkElement WrapXaml(
        winrt::Windows::UI::Xaml::FrameworkElement const& Content, int MinimumWidth = 0, int MinimumHeight = 0)
    {
        using namespace winrt::Windows::UI::Xaml;
        using namespace winrt::Windows::UI::Xaml::Controls;
        if (Percent() == 100) return Content;
        const double Factor = Percent() / 100.0;
        Viewbox Box;
        Box.Stretch(Media::Stretch::Fill);
        Content.Width(1);
        Content.Height(1);
        Box.Child(Content);
        ScrollViewer Viewport;
        Viewport.HorizontalScrollBarVisibility(MinimumWidth ? ScrollBarVisibility::Auto : ScrollBarVisibility::Disabled);
        Viewport.VerticalScrollBarVisibility(MinimumHeight ? ScrollBarVisibility::Auto : ScrollBarVisibility::Disabled);
        Viewport.HorizontalContentAlignment(HorizontalAlignment::Left);
        Viewport.VerticalContentAlignment(VerticalAlignment::Top);
        Viewport.Content(Box);
        Viewport.IsTabStop(false);
        Viewport.SizeChanged([Box, Content, Factor, MinimumWidth, MinimumHeight](auto&&, SizeChangedEventArgs const& Args)
        {
            const double Width = (std::max)(static_cast<double>(Args.NewSize().Width), MinimumWidth * Factor);
            const double Height = (std::max)(static_cast<double>(Args.NewSize().Height), MinimumHeight * Factor);
            if (Width <= 0 || Height <= 0) return;
            Content.Width(Width / Factor);
            Content.Height(Height / Factor);
            Box.Width(Width);
            Box.Height(Height);
        });
        return Viewport;
    }

    inline void ApplyPage(winrt::Windows::UI::Xaml::Controls::Page const& Page, int Width = 0, int Height = 0)
    {
        if (Percent() == 100) return;
        auto Content = Page.Content().as<winrt::Windows::UI::Xaml::FrameworkElement>();
        Page.Content(nullptr);
        Page.Content(WrapXaml(Content, Width, Height));
    }
}
