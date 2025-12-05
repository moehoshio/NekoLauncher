#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtCore/QString>
#include <QtGui/QFont>
#include <QtWidgets/QWidget>

#include <vector>

class QDialogButtonBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

namespace neko::ui::dialog {

    class InputDialog : public QWidget {
        Q_OBJECT
    private:
        widget::PixmapWidget *poster;
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
        void setupTheme(const Theme &theme);

    signals:
        void hideInputD();
        void showInputD(const InputMsg &m);
        std::vector<std::string> getLinesD();
    };

} // namespace neko::ui::dialog