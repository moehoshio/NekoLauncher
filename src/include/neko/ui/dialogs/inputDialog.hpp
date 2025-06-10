#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui {

    class InputDialog : public QWidget {
    private:
        PixmapWidget *poster;
        QGridLayout *gridLayout;
        QWidget *centralWidget;
        QVBoxLayout *centralWidgetLayout;
        QLabel *title;
        QLabel *msg;
        QDialogButtonBox *dialogButton;
        std::vector<QLineEdit *> lines;

    public:
        InputDialog(QWidget *parent = nullptr);

        void hideInput();
        void showInput(const InputMsg &m);

        void setLines(const std::vector<std::string> &vec);

        /**
         * @brief Retrieve the current data from the input components.
         *
         * @return std::vector<std::string> A vector containing the input from each line.
         * @note Lines without input will be represented as empty strings.
         */
        std::vector<std::string> getLines();

        void resizeItems(int windowWidth, int windowHeight);

        void setupFont(QFont font, QFont titleFont);
        void setupStyle(const Theme &theme);
    };
} // namespace neko::ui
