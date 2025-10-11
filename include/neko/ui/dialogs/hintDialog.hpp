#pragma once

#include "neko/ui/uiMsg.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <memory>
#include <string_view>
#include <vector>

namespace neko::ui {

    class HintDialog : public QWidget {
        
    private:
        QWidget *centralWidget;
        PixmapWidget *poster;
        QVBoxLayout *centralWidgetLayout;
        QFrame *line;
        std::vector<QPushButton *> buttons;
        QLabel *title;
        QLabel *msg;
        QDialogButtonBox *dialogButton;

    public:
        HintDialog(QWidget *parent = nullptr);

        void showHint(const HintMsg &m);

        void setupFont(QFont font,QFont titleFont);
        void setupStyle(const Theme &theme);

        void resetState();
        void resetButtons();

        void resizeItems(int windowWidth, int windowHeight);
    };
} // namespace neko::ui
