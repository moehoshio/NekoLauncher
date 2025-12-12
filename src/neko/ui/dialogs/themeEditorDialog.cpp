#include "neko/ui/dialogs/themeEditorDialog.hpp"

#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace neko::ui::dialog {

    ThemeEditorDialog::ThemeEditorDialog(const Theme &baseTheme, QWidget *parent)
        : QDialog(parent), workingTheme(baseTheme) {
        setWindowTitle(tr("Edit Theme"));
        setModal(true);
        setMinimumWidth(520);
        buildForm();
    }

    void ThemeEditorDialog::buildForm() {
        auto *layout = new QVBoxLayout(this);

        auto *metaGrid = new QGridLayout();
        metaGrid->setHorizontalSpacing(10);
        metaGrid->setVerticalSpacing(8);

        nameEdit = new QLineEdit(QString::fromStdString(workingTheme.info.name), this);
        descEdit = new QLineEdit(QString::fromStdString(workingTheme.info.description), this);
        authorEdit = new QLineEdit(QString::fromStdString(workingTheme.info.author), this);
        typeCombo = new QComboBox(this);
        typeCombo->addItems({"Light", "Dark", "Custom"});
        switch (workingTheme.info.type) {
            case ThemeType::Light: typeCombo->setCurrentText("Light"); break;
            case ThemeType::Dark: typeCombo->setCurrentText("Dark"); break;
            default: typeCombo->setCurrentText("Custom"); break;
        }

        metaGrid->addWidget(new QLabel(tr("Name"), this), 0, 0);
        metaGrid->addWidget(nameEdit, 0, 1);
        metaGrid->addWidget(new QLabel(tr("Description"), this), 1, 0);
        metaGrid->addWidget(descEdit, 1, 1);
        metaGrid->addWidget(new QLabel(tr("Author"), this), 2, 0);
        metaGrid->addWidget(authorEdit, 2, 1);
        metaGrid->addWidget(new QLabel(tr("Type"), this), 3, 0);
        metaGrid->addWidget(typeCombo, 3, 1);

        layout->addLayout(metaGrid);

        auto *colorGrid = new QGridLayout();
        colorGrid->setHorizontalSpacing(10);
        colorGrid->setVerticalSpacing(6);

        int row = 0;
        addColorRow(colorGrid, row++, tr("Primary"), workingTheme.colors.primary);
        addColorRow(colorGrid, row++, tr("Secondary"), workingTheme.colors.secondary);
        addColorRow(colorGrid, row++, tr("Background"), workingTheme.colors.background);
        addColorRow(colorGrid, row++, tr("Canvas"), workingTheme.colors.canvas);
        addColorRow(colorGrid, row++, tr("Text"), workingTheme.colors.text);
        addColorRow(colorGrid, row++, tr("Accent"), workingTheme.colors.accent);
        addColorRow(colorGrid, row++, tr("Success"), workingTheme.colors.success);
        addColorRow(colorGrid, row++, tr("Warning"), workingTheme.colors.warning);
        addColorRow(colorGrid, row++, tr("Error"), workingTheme.colors.error);
        addColorRow(colorGrid, row++, tr("Info"), workingTheme.colors.info);
        addColorRow(colorGrid, row++, tr("Surface"), workingTheme.colors.surface);
        addColorRow(colorGrid, row++, tr("Panel"), workingTheme.colors.panel);
        addColorRow(colorGrid, row++, tr("Disabled"), workingTheme.colors.disabled);
        addColorRow(colorGrid, row++, tr("Hover"), workingTheme.colors.hover);
        addColorRow(colorGrid, row++, tr("Focus"), workingTheme.colors.focus);

        layout->addLayout(colorGrid);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &ThemeEditorDialog::onAccept);
        connect(buttons, &QDialogButtonBox::rejected, this, &ThemeEditorDialog::reject);
        layout->addWidget(buttons);
    }

    void ThemeEditorDialog::addColorRow(QGridLayout *layout, int row, const QString &label, const std::string &value) {
        auto *lbl = new QLabel(label, this);
        auto *edit = new QLineEdit(QString::fromStdString(value), this);
        auto *pick = new QPushButton(tr("Pick"), this);
        pick->setFixedWidth(64);
        connect(pick, &QPushButton::clicked, this, [this, edit]() { onPickColor(edit); });
        layout->addWidget(lbl, row, 0);
        layout->addWidget(edit, row, 1);
        layout->addWidget(pick, row, 2);
        colorEdits.emplace_back(label, edit);
    }

    void ThemeEditorDialog::onPickColor(QLineEdit *target) {
        const QColor initial = QColor(target->text());
        const QColor chosen = QColorDialog::getColor(initial.isValid() ? initial : Qt::white, this, tr("Pick Color"));
        if (chosen.isValid()) {
            target->setText(chosen.name(QColor::HexArgb));
        }
    }

    void ThemeEditorDialog::onAccept() {
        workingTheme.info.name = nameEdit->text().toStdString();
        workingTheme.info.description = descEdit->text().toStdString();
        workingTheme.info.author = authorEdit->text().toStdString();
        const auto typeStr = typeCombo->currentText().toLower();
        if (typeStr == "light") workingTheme.info.type = ThemeType::Light;
        else if (typeStr == "dark") workingTheme.info.type = ThemeType::Dark;
        else workingTheme.info.type = ThemeType::Custom;

        auto setIf = [&](const QString &key, std::string ThemeColors::*field) {
            for (const auto &pair : colorEdits) {
                if (pair.first.compare(key, Qt::CaseInsensitive) == 0) {
                    workingTheme.colors.*field = pair.second->text().toStdString();
                    break;
                }
            }
        };
        setIf("Primary", &ThemeColors::primary);
        setIf("Secondary", &ThemeColors::secondary);
        setIf("Background", &ThemeColors::background);
        setIf("Canvas", &ThemeColors::canvas);
        setIf("Text", &ThemeColors::text);
        setIf("Accent", &ThemeColors::accent);
        setIf("Success", &ThemeColors::success);
        setIf("Warning", &ThemeColors::warning);
        setIf("Error", &ThemeColors::error);
        setIf("Info", &ThemeColors::info);
        setIf("Surface", &ThemeColors::surface);
        setIf("Panel", &ThemeColors::panel);
        setIf("Disabled", &ThemeColors::disabled);
        setIf("Hover", &ThemeColors::hover);
        setIf("Focus", &ThemeColors::focus);

        accept();
    }

    Theme ThemeEditorDialog::getEditedTheme() const {
        return workingTheme;
    }

} // namespace neko::ui::dialog
