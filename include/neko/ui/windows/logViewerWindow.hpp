#pragma once

#include "neko/ui/theme.hpp"

#include <QtWidgets/QWidget>
#include <QtCore/QString>
#include <QtCore/QStringList>

class QTextEdit;
class QLayout;

namespace neko::ui::window {

    class LogViewerWindow : public QWidget {
        Q_OBJECT

    private:
        QTextEdit *textEdit;

    public:
        explicit LogViewerWindow(QWidget *parent = nullptr);
        explicit LogViewerWindow(const QString &filePath, QWidget *parent = nullptr);

        void initialize();
        void setupTheme(const Theme &theme);
        void setLayout(QLayout *layout);
        void loadLogFile(const QString &filePath);
        void appendLogLine(const QString &line);
        void appendLogLines(const QStringList &lines);
    };

} // namespace neko::ui::window