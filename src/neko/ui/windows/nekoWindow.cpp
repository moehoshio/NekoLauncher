#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/app/nekoLc.hpp"

#include <QtWidgets/QLabel>

namespace neko::ui::window {
    NekoWindow::NekoWindow(const ClientConfig &config)
        : headBar(new widget::HeadBarWidget(this, this)),
          pixmapWidget(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)) {

        
        headBar->showHeadBar();
        pixmapWidget->lower();

        pixmapWidget->setPixmap(config.main.background);
        this->setCentralWidget(pixmapWidget);

        this->setWindowTitle(lc::AppName.data());
        this->setWindowIcon(QIcon(lc::AppIconPath.data()));
        this->addToolBar(headBar->getToolBar());
    }

    NekoWindow::~NekoWindow() = default;

} // namespace neko::ui::window