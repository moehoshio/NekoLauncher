#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QApplication>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QtGui/QScreen>
#include <QtGui/QTextCharFormat>

class LogViewer : public QWidget {
    Q_OBJECT

public:
    explicit LogViewer(const QString &filePath, QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(true);
        layout->addWidget(textEdit);
        setLayout(layout);
        loadLogFile(filePath);
        QSize scrSize = QGuiApplication::primaryScreen()->size();
        resize(scrSize.width() * 0.8, scrSize.height() * 0.75);
    }

    inline void loadLogFile(const QString &filePath) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                appendLogLine(line);
            }
        }
    }

    inline void appendLogLine(const QString &line) {
        QTextCharFormat format;
        if (line.contains(" INFO|")) {
            format.setForeground(Qt::darkBlue);
        } else if (line.contains(" WARN|")) {
            format.setForeground(Qt::darkYellow);
        } else if (line.contains(" ERR|")) {
            format.setForeground(Qt::red);
        } else {
            format.setForeground(Qt::black); 
        }

        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(line + "\n", format);
        textEdit->setTextCursor(cursor);
    }

    QTextEdit *textEdit;
};
