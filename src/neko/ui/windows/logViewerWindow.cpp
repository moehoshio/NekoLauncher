#include "neko/ui/windows/logViewerWindow.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QtGui/QScreen>
#include <QtGui/QTextCharFormat>
#include <QtGui/QColor>

#include <string_view>

namespace neko::ui::window {

    LogViewerWindow::LogViewerWindow(QWidget *parent)
        : QWidget(parent), textEdit(new QTextEdit(this)) {
        initialize();
    }

    LogViewerWindow::LogViewerWindow(const QString &filePath, QWidget *parent)
        : QWidget(parent), textEdit(new QTextEdit(this)) {
        initialize();
        loadLogFile(filePath);
    }

    void LogViewerWindow::initialize() {
        this->setWindowTitle("NekoLauncher Developer Log Viewer");
        setLayout(new QVBoxLayout(this));
        textEdit->setReadOnly(true);
        textEdit->setLineWrapMode(QTextEdit::NoWrap);
        textEdit->setFontPointSize(10);
        resize(QGuiApplication::primaryScreen()->size() * 0.75);
        setupTheme(ui::getCurrentTheme());
    }
    void LogViewerWindow::setupTheme(const Theme &theme) {
        activeTheme = theme;
        QString styleSheet =
            QString(
                "QTextEdit {"
                "background-color: %1;"
                "color: %2;"
                "border: none;"
                "selection-background-color: %3;"
                "}")
                .arg(theme.colors.surface.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.focus.data());
        textEdit->setStyleSheet(styleSheet);
    }

    void LogViewerWindow::setLayout(QLayout *layout) {
        if (layout) {
            layout->addWidget(textEdit);
        }
    }

    void LogViewerWindow::loadLogFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd()) {
            lines << in.readLine();
        }
        appendLogLines(lines);
    }

    void LogViewerWindow::appendLogLine(const QString &line) {
        QTextCharFormat format;
        auto colorFrom = [](std::string_view hex) {
            return QColor(QString::fromUtf8(hex.data(), static_cast<int>(hex.size())));
        };

        if (line.contains(" [Debug]"))
            format.setForeground(colorFrom(activeTheme.colors.disabled));
        else if (line.contains(" [Info]"))
            format.setForeground(colorFrom(activeTheme.colors.info));
        else if (line.contains(" [Warn]"))
            format.setForeground(colorFrom(activeTheme.colors.warning));
        else if (line.contains(" [Error]"))
            format.setForeground(colorFrom(activeTheme.colors.error));
        else
            format.setForeground(colorFrom(activeTheme.colors.text));

        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(line + '\n', format);
        textEdit->setTextCursor(cursor);
    }

    void LogViewerWindow::appendLogLines(const QStringList &lines) {
        for (const auto &line : lines) {
            appendLogLine(line);
        }
    }

} // namespace neko::ui::window
