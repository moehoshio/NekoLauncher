#pragma once

#include "neko/ui/theme.hpp"

#include <QtWidgets/QDialog>

#include <string>
#include <vector>

class QLineEdit;
class QComboBox;
class QPushButton;
class QGridLayout;

namespace neko::ui::dialog {

    class ThemeEditorDialog : public QDialog {
        Q_OBJECT
    public:
        explicit ThemeEditorDialog(const Theme &baseTheme, QWidget *parent = nullptr);

        Theme getEditedTheme() const;

    private slots:
        void onPickColor(QLineEdit *target);
        void onAccept();

    private:
        Theme workingTheme;
        QLineEdit *nameEdit;
        QLineEdit *descEdit;
        QLineEdit *authorEdit;
        QComboBox *typeCombo;
        std::vector<std::pair<QString, QLineEdit *>> colorEdits; // label -> line edit

        void buildForm();
        void addColorRow(QGridLayout *layout, int row, const QString &label, const std::string &value);
    };

} // namespace neko::ui::dialog
