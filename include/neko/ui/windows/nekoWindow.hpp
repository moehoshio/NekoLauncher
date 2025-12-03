#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/headBarWidget.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/app/clientConfig.hpp"

#include <QtWidgets/QMainWindow>

class QLabel;

namespace neko::ui::window
{
    class NekoWindow : public QMainWindow
    {
    private:
        widget::HeadBarWidget* headBar;
        widget::PixmapWidget* pixmapWidget;
    public:
        NekoWindow(const ClientConfig& config);
        ~NekoWindow();
    };
    
} // namespace ui
