#include "neko/ui/windows/logViewerWindow.hpp"

namespace neko::ui {

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
    auto *layout = new QVBoxLayout(this);
    setLayout(layout);
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    textEdit->setFontPointSize(10);
    textEdit->setStyleSheet("QTextEdit { background-color: #f0f0f0; }");
    resize(QGuiApplication::primaryScreen()->size() * 0.75);
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
    while (!in.atEnd())
        lines << in.readLine();
    appendLogLines(lines);
}

void LogViewerWindow::appendLogLine(const QString &line) {
    QTextCharFormat format;
    if (line.contains(" INFO|"))
        format.setForeground(Qt::darkBlue);
    else if (line.contains(" WARN|"))
        format.setForeground(Qt::darkYellow);
    else if (line.contains(" ERR|"))
        format.setForeground(Qt::red);
    else
        format.setForeground(Qt::black);

    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(line + '\n', format);
    textEdit->setTextCursor(cursor);
}

void LogViewerWindow::appendLogLines(const QStringList &lines) {
    for (const auto &line : lines)
        appendLogLine(line);
}

} // namespace neko::ui
