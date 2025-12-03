#include "neko/ui/windows/logViewerWindow.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QtGui/QScreen>
#include <QtGui/QTextCharFormat>

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
    }
    void LogViewerWindow::setupTheme(const Theme &theme) {
        QString styleSheet =
            QString(
                "QTextEdit {"
                "background-color: %1;"
                "border: none;"
                "}")
                .arg(theme.backgroundColor.data());
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
        if (line.contains(" [Debug]"))
            format.setForeground(Qt::darkGray);
        else if (line.contains(" [Info]"))
            format.setForeground(Qt::darkBlue);
        else if (line.contains(" [Warn]"))
            format.setForeground(Qt::darkYellow);
        else if (line.contains(" [Error]"))
            format.setForeground(Qt::red);
        else
            format.setForeground(Qt::gray);

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
