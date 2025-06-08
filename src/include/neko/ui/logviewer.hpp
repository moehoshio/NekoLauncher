#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QtGui/QScreen>
#include <QtGui/QTextCharFormat>

namespace neko::ui {

    class LogViewer : public QWidget {
        Q_OBJECT

    private:
        QTextEdit *textEdit;

    public:
        explicit LogViewer(QWidget *parent = nullptr)
            : QWidget(parent), textEdit(new QTextEdit(this)) {
            initialize();
        }

        explicit LogViewer(const QString &filePath, QWidget *parent = nullptr)
            : QWidget(parent), textEdit(new QTextEdit(this)) {
            initialize();
            loadLogFile(filePath);
        }

        void initialize() {
            auto *layout = new QVBoxLayout(this);
            setLayout(layout);
            textEdit->setReadOnly(true);
            textEdit->setLineWrapMode(QTextEdit::NoWrap);
            textEdit->setFontPointSize(10);
            textEdit->setStyleSheet("QTextEdit { background-color: #f0f0f0; }");
            resize(QGuiApplication::primaryScreen()->size() * 0.75);
        }

        void setLayout(QLayout *layout) override {
            if (layout) {
                layout->addWidget(textEdit);
            }
        }

        void loadLogFile(const QString &filePath) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
            QTextStream in(&file);
            QStringList lines;
            while (!in.atEnd())
                lines << in.readLine();
            appendLogLines(lines);
        }

        void appendLogLine(const QString &line) {
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

        void appendLogLines(const QStringList &lines) {
            for (const auto &line : lines)
                appendLogLine(line);
        }
    };

} // namespace neko::ui