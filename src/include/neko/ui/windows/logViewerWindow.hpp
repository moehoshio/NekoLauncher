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

    class LogViewerWindow : public QWidget {
        Q_OBJECT

    private:
        QTextEdit *textEdit;

    public:
        explicit LogViewerWindow(QWidget *parent = nullptr);
        explicit LogViewerWindow(const QString &filePath, QWidget *parent = nullptr);

        void initialize();
        void setLayout(QLayout *layout) override;
        void loadLogFile(const QString &filePath);
        void appendLogLine(const QString &line);
        void appendLogLines(const QStringList &lines);
    };

} // namespace neko::ui